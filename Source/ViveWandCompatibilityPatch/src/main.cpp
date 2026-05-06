#include <windows.h>
#include <shlobj.h>

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "openvr.h"

using UInt32 = std::uint32_t;
using PluginHandle = UInt32;

namespace {

constexpr UInt32 kPluginInfoVersion = 1;
constexpr UInt32 kPluginVersion = 213;
constexpr PluginHandle kPluginHandleInvalid = 0xFFFFFFFF;
constexpr int kInterfaceMessaging = 1;

constexpr float kDialogueScrollAxisDeadzone = 0.20f;
constexpr float kDialogueScrollStepAxis = 0.35f;
constexpr float kDialogueCenterTouchAxisThreshold = 0.40f;
constexpr int kScaleformElementSearchMaxDepth = 16;
constexpr int kScaleformElementSearchMaxNodes = 512;
constexpr int kScaleformElementSearchMaxChildren = 128;

constexpr std::uintptr_t kUiPtrRva = 0x05932320;
constexpr std::uintptr_t kUiMenuStackOffset = 0x190;
constexpr std::uintptr_t kMenuMovieOffset = 0x40;
constexpr std::uintptr_t kMenuNameOffset = 0x50;
constexpr std::uintptr_t kMovieRootOffset = 0x18;
constexpr std::uintptr_t kGFxValueObjectInvokeRva = 0x0213BB80;
constexpr std::uintptr_t kGFxValueObjectGetMemberRva = 0x02131EC0;
constexpr std::uintptr_t kGFxValueObjectReleaseManagedRva = 0x02143AE0;
constexpr std::size_t kGFxMovieRootGetVariableVtableIndex = 50;

PluginHandle g_pluginHandle = kPluginHandleInvalid;

struct F4SEInterface
{
	UInt32 f4seVersion;
	UInt32 runtimeVersion;
	UInt32 editorVersion;
	UInt32 isEditor;
	void* (*QueryInterface)(UInt32 id);
	PluginHandle (*GetPluginHandle)();
	UInt32 (*GetReleaseIndex)();
};

struct PluginInfo
{
	UInt32 infoVersion;
	const char* name;
	UInt32 version;
};

struct F4SEMessagingInterface
{
	struct Message
	{
		const char* sender;
		UInt32 type;
		UInt32 dataLen;
		void* data;
	};

	using EventCallback = void (*)(Message* message);

	UInt32 interfaceVersion;
	bool (*RegisterListener)(PluginHandle listener, const char* sender, EventCallback handler);
	bool (*Dispatch)(PluginHandle sender, UInt32 messageType, void* data, UInt32 dataLen, const char* receiver);
	void* (*GetEventDispatcher)(UInt32 dispatcherId);

	enum : UInt32
	{
		kMessagePostLoad = 0,
		kMessagePostPostLoad = 1,
		kMessageGameLoaded = 9,
	};
};

using GetControllerStateCB = bool (*)(
	vr::TrackedDeviceIndex_t controllerDeviceIndex,
	const vr::VRControllerState_t* controllerState,
	std::uint32_t controllerStateSize,
	vr::VRControllerState_t* outputControllerState);

using WaitGetPosesCB = vr::EVRCompositorError (*)(
	vr::TrackedDevicePose_t* renderPoseArray,
	std::uint32_t renderPoseArrayCount,
	vr::TrackedDevicePose_t* gamePoseArray,
	std::uint32_t gamePoseArrayCount);

class OpenVRHookManagerAPI
{
public:
	virtual unsigned int GetVersion() = 0;
	virtual bool IsInitialized() = 0;
	virtual void RegisterControllerStateCB(GetControllerStateCB callback) = 0;
	virtual void RegisterGetPosesCB(WaitGetPosesCB callback) = 0;
	virtual void UnregisterControllerStateCB(GetControllerStateCB callback) = 0;
	virtual void UnregisterGetPosesCB(WaitGetPosesCB callback) = 0;
	virtual vr::IVRSystem* GetVRSystem() const = 0;
	virtual vr::IVRCompositor* GetVRCompositor() const = 0;
};

using GetVRHookManagerFn = OpenVRHookManagerAPI* (*)();

struct BSFixedStringLite
{
	void* data;
};

struct StringCacheEntryLite
{
	StringCacheEntryLite* next;
	std::uint32_t state;
	std::uint32_t length;
	StringCacheEntryLite* externData;
	char data[1];
};

struct TArrayPointerLite
{
	void** entries;
	std::uint32_t capacity;
	std::uint32_t pad0C;
	std::uint32_t count;
	std::uint32_t pad14;
};

struct GFxValueLite
{
	enum Type : std::uint32_t
	{
		kTypeUndefined = 0,
		kTypeNull = 1,
		kTypeBool = 2,
		kTypeInt = 3,
		kTypeUInt = 4,
		kTypeNumber = 5,
		kTypeString = 6,
		kTypeObject = 8,
		kTypeArray = 9,
		kTypeDisplayObject = 10,
		kTypeFlagManaged = 1 << 6,
		kMaskType = 0x8F,
	};

	union Data
	{
		std::uint32_t u32;
		std::int32_t s32;
		double number;
		bool boolean;
		const char* string;
		const char** managedString;
		void* obj;
	};

	void* objectInterface;
	std::uint32_t type;
	std::uint32_t pad0C;
	Data data;
	void* unk18;
};

using GFxMovieRootGetVariableFn = bool (*)(void* movieRoot, GFxValueLite* result, const char* variablePath);
using GFxValueObjectGetMemberFn = bool (*)(void* objectInterface, void* data, const char* name, GFxValueLite* result, bool isDisplayObject);
using GFxValueObjectInvokeFn = bool (*)(void* objectInterface, void* data, GFxValueLite* result, const char* name, GFxValueLite* args, std::uint64_t argCount, std::uint8_t isDisplayObject);
using GFxValueObjectReleaseManagedFn = void (*)(void* objectInterface, GFxValueLite* value, void* data);

enum class PadDirection
{
	Center,
	Up,
	Down,
};

OpenVRHookManagerAPI* g_hookManager = nullptr;
bool g_callbackRegistered = false;
bool g_dialogueScrollSessionActive = false;
bool g_dialogueScrollStartedNearCenter = false;
float g_dialogueScrollLastY = 0.0f;
float g_dialogueScrollAccumulator = 0.0f;
void* g_activeDialogueMenu = nullptr;
std::uint32_t g_lastDialogueScrollInvokePacket = 0;
PadDirection g_lastDialogueScrollInvokeDirection = PadDirection::Center;
int g_dialogueScrollLogBudget = 80;

float AbsFloat(float value)
{
	return value < 0.0f ? -value : value;
}

bool IsDialogueCenterTouch(float x, float y)
{
	return AbsFloat(x) <= kDialogueCenterTouchAxisThreshold &&
		AbsFloat(y) <= kDialogueCenterTouchAxisThreshold;
}

const char* PadDirectionName(PadDirection direction)
{
	switch (direction) {
	case PadDirection::Up:
		return "up";
	case PadDirection::Down:
		return "down";
	case PadDirection::Center:
	default:
		return "center";
	}
}

std::string GetLogPath()
{
	char documentsPath[MAX_PATH]{};
	if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, SHGFP_TYPE_CURRENT, documentsPath))) {
		return std::string(documentsPath) + "\\My Games\\Fallout4VR\\F4SE\\ViveWandCompatibilityPatch.log";
	}

	char tempPath[MAX_PATH]{};
	if (GetTempPathA(MAX_PATH, tempPath) > 0) {
		return std::string(tempPath) + "ViveWandCompatibilityPatch.log";
	}

	return "ViveWandCompatibilityPatch.log";
}

void Log(const char* format, ...)
{
	char message[1024]{};
	va_list args;
	va_start(args, format);
	vsnprintf_s(message, sizeof(message), _TRUNCATE, format, args);
	va_end(args);

	std::ofstream log(GetLogPath(), std::ios::app);
	if (log.is_open()) {
		log << message << '\n';
	}

	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

std::uintptr_t GetFallout4VRBase()
{
	return reinterpret_cast<std::uintptr_t>(GetModuleHandleA("Fallout4VR.exe"));
}

const char* GetFixedStringText(const BSFixedStringLite& value)
{
	if (!value.data) {
		return nullptr;
	}

	auto entry = reinterpret_cast<StringCacheEntryLite*>(value.data);
	__try {
		while (entry && (entry->state & 0x4000) != 0) {
			entry = entry->externData;
		}

		return entry ? entry->data : nullptr;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

void* GetUI()
{
	const std::uintptr_t base = GetFallout4VRBase();
	if (!base) {
		return nullptr;
	}

	auto uiSlot = reinterpret_cast<void**>(base + kUiPtrRva);
	return uiSlot ? *uiSlot : nullptr;
}

bool MenuNameEquals(void* menu, const char* expectedName)
{
	if (!menu || !expectedName) {
		return false;
	}

	__try {
		auto menuName = reinterpret_cast<BSFixedStringLite*>(reinterpret_cast<std::uint8_t*>(menu) + kMenuNameOffset);
		const char* actualName = GetFixedStringText(*menuName);
		return actualName && std::strcmp(actualName, expectedName) == 0;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

void* FindDialogueMenu()
{
	static bool loggedFault = false;
	static bool loggedMissingStack = false;

	void* ui = GetUI();
	if (!ui) {
		return nullptr;
	}

	__try {
		auto menuStack = reinterpret_cast<TArrayPointerLite*>(reinterpret_cast<std::uint8_t*>(ui) + kUiMenuStackOffset);
		if (!menuStack || !menuStack->entries || menuStack->count == 0 || menuStack->count > 64) {
			if (!loggedMissingStack) {
				Log(
					"DialogueMenu lookup could not inspect UI menu stack; entries=%p count=%u",
					menuStack ? menuStack->entries : nullptr,
					menuStack ? menuStack->count : 0);
				loggedMissingStack = true;
			}
			return nullptr;
		}

		for (std::int32_t i = static_cast<std::int32_t>(menuStack->count) - 1; i >= 0; --i) {
			void* menu = menuStack->entries[i];
			if (MenuNameEquals(menu, "DialogueMenu")) {
				return menu;
			}
		}
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		if (!loggedFault) {
			Log("DialogueMenu lookup faulted while walking UI menu stack");
			loggedFault = true;
		}
	}

	return nullptr;
}

void* GetMenuMovieRoot(void* menu)
{
	if (!menu) {
		return nullptr;
	}

	__try {
		void* movie = *reinterpret_cast<void**>(reinterpret_cast<std::uint8_t*>(menu) + kMenuMovieOffset);
		if (!movie) {
			return nullptr;
		}
		return *reinterpret_cast<void**>(reinterpret_cast<std::uint8_t*>(movie) + kMovieRootOffset);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return nullptr;
	}
}

GFxValueLite MakeGFxInt(std::int32_t value)
{
	GFxValueLite result{};
	result.type = GFxValueLite::kTypeInt;
	result.data.s32 = value;
	return result;
}

std::uint32_t GetGFxType(const GFxValueLite& value)
{
	return value.type & GFxValueLite::kMaskType;
}

bool IsGFxManaged(const GFxValueLite& value)
{
	return (value.type & GFxValueLite::kTypeFlagManaged) != 0;
}

bool IsGFxDisplayObject(const GFxValueLite& value)
{
	return GetGFxType(value) == GFxValueLite::kTypeDisplayObject;
}

bool IsGFxObjectLike(const GFxValueLite& value)
{
	const std::uint32_t type = GetGFxType(value);
	return type == GFxValueLite::kTypeObject ||
		type == GFxValueLite::kTypeArray ||
		type == GFxValueLite::kTypeDisplayObject;
}

void CleanGFxValue(GFxValueLite& value)
{
	if (!IsGFxManaged(value) || !value.objectInterface) {
		value = {};
		return;
	}

	static bool loggedFault = false;
	const std::uintptr_t base = GetFallout4VRBase();
	if (!base) {
		value = {};
		return;
	}

	auto releaseManaged = reinterpret_cast<GFxValueObjectReleaseManagedFn>(base + kGFxValueObjectReleaseManagedRva);
	__try {
		releaseManaged(value.objectInterface, &value, value.data.obj);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		if (!loggedFault) {
			Log("DialogueMenu Scaleform bridge faulted while releasing managed GFxValue");
			loggedFault = true;
		}
	}

	value = {};
}

bool GetScaleformVariable(void* movieRoot, const char* variablePath, GFxValueLite* result)
{
	static bool loggedFault = false;

	if (!movieRoot || !variablePath || !result) {
		return false;
	}

	*result = {};
	__try {
		auto vtable = *reinterpret_cast<void***>(movieRoot);
		if (!vtable) {
			return false;
		}

		auto getVariable = reinterpret_cast<GFxMovieRootGetVariableFn>(vtable[kGFxMovieRootGetVariableVtableIndex]);
		if (!getVariable) {
			return false;
		}

		return getVariable(movieRoot, result, variablePath);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		if (!loggedFault) {
			Log("DialogueMenu Scaleform bridge faulted while reading variable %s", variablePath);
			loggedFault = true;
		}
		*result = {};
		return false;
	}
}

bool GFxObjectInvoke(const GFxValueLite& object, const char* methodName, GFxValueLite* args, std::uint64_t argCount)
{
	static bool loggedFault = false;

	if (!IsGFxObjectLike(object) || !object.objectInterface || !methodName) {
		return false;
	}

	const std::uintptr_t base = GetFallout4VRBase();
	if (!base) {
		return false;
	}

	GFxValueLite result{};
	auto invoke = reinterpret_cast<GFxValueObjectInvokeFn>(base + kGFxValueObjectInvokeRva);
	bool invoked = false;
	__try {
		invoked = invoke(
			object.objectInterface,
			object.data.obj,
			&result,
			methodName,
			args,
			argCount,
			IsGFxDisplayObject(object) ? 1 : 0);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		if (!loggedFault) {
			Log("DialogueMenu Scaleform bridge faulted while invoking object member %s", methodName);
			loggedFault = true;
		}
		invoked = false;
	}

	CleanGFxValue(result);
	return invoked;
}

bool GFxObjectInvokeResult(
	const GFxValueLite& object,
	const char* methodName,
	GFxValueLite* result,
	GFxValueLite* args,
	std::uint64_t argCount)
{
	static bool loggedFault = false;

	if (result) {
		*result = {};
	}

	if (!IsGFxObjectLike(object) || !object.objectInterface || !methodName) {
		return false;
	}

	const std::uintptr_t base = GetFallout4VRBase();
	if (!base) {
		return false;
	}

	auto invoke = reinterpret_cast<GFxValueObjectInvokeFn>(base + kGFxValueObjectInvokeRva);
	bool invoked = false;
	__try {
		invoked = invoke(
			object.objectInterface,
			object.data.obj,
			result,
			methodName,
			args,
			argCount,
			IsGFxDisplayObject(object) ? 1 : 0);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		if (!loggedFault) {
			Log("DialogueMenu Scaleform bridge faulted while invoking object member %s for result", methodName);
			loggedFault = true;
		}
		if (result) {
			*result = {};
		}
		invoked = false;
	}

	return invoked;
}

bool GFxObjectGetMember(const GFxValueLite& object, const char* name, GFxValueLite* result)
{
	static bool loggedFault = false;

	if (result) {
		*result = {};
	}

	if (!IsGFxObjectLike(object) || !object.objectInterface || !name || !result) {
		return false;
	}

	const std::uintptr_t base = GetFallout4VRBase();
	if (!base) {
		return false;
	}

	auto getMember = reinterpret_cast<GFxValueObjectGetMemberFn>(base + kGFxValueObjectGetMemberRva);
	bool gotMember = false;
	__try {
		gotMember = getMember(
			object.objectInterface,
			object.data.obj,
			name,
			result,
			IsGFxDisplayObject(object));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		if (!loggedFault) {
			Log("DialogueMenu Scaleform bridge faulted while reading object member %s", name);
			loggedFault = true;
		}
		*result = {};
		gotMember = false;
	}

	return gotMember;
}

bool TryReadGFxIntLike(const GFxValueLite& value, int* valueOut)
{
	if (!valueOut) {
		return false;
	}

	switch (GetGFxType(value)) {
	case GFxValueLite::kTypeBool:
		*valueOut = value.data.boolean ? 1 : 0;
		return true;
	case GFxValueLite::kTypeInt:
		*valueOut = value.data.s32;
		return true;
	case GFxValueLite::kTypeUInt:
		*valueOut = static_cast<int>(value.data.u32);
		return true;
	case GFxValueLite::kTypeNumber:
		*valueOut = static_cast<int>(value.data.number);
		return true;
	default:
		return false;
	}
}

bool TryReadGFxIntMember(const GFxValueLite& object, const char* name, int* valueOut)
{
	GFxValueLite value{};
	const bool gotMember = GFxObjectGetMember(object, name, &value);
	const bool read = gotMember && TryReadGFxIntLike(value, valueOut);
	CleanGFxValue(value);
	return read;
}

const char* GetGFxStringValue(const GFxValueLite& value)
{
	if (GetGFxType(value) != GFxValueLite::kTypeString) {
		return nullptr;
	}

	if (IsGFxManaged(value)) {
		return value.data.managedString ? *value.data.managedString : nullptr;
	}

	return value.data.string;
}

bool GFxObjectNameEquals(const GFxValueLite& object, const char* expectedName)
{
	if (!expectedName) {
		return false;
	}

	GFxValueLite nameValue{};
	const bool gotName = GFxObjectGetMember(object, "name", &nameValue);
	const char* actualName = gotName ? GetGFxStringValue(nameValue) : nullptr;
	const bool matches = actualName && std::strcmp(actualName, expectedName) == 0;
	CleanGFxValue(nameValue);
	return matches;
}

bool InvokeScaleformPathMethod(void* movieRoot, const char* objectPath, const char* methodName)
{
	if (!movieRoot || !objectPath || !methodName) {
		return false;
	}

	GFxValueLite object{};
	const bool gotObject = GetScaleformVariable(movieRoot, objectPath, &object) && IsGFxObjectLike(object);
	const bool invoked = gotObject && GFxObjectInvoke(object, methodName, nullptr, 0);
	CleanGFxValue(object);
	return invoked;
}

bool ShouldPassDialogueScrollAxis(float axisX, float axisY)
{
	const float absX = AbsFloat(axisX);
	const float absY = AbsFloat(axisY);

	if (absY < kDialogueScrollAxisDeadzone) {
		return false;
	}

	return absY >= absX * 0.65f;
}

void ResetDialogueScrollSession()
{
	g_dialogueScrollSessionActive = false;
	g_dialogueScrollStartedNearCenter = false;
	g_dialogueScrollLastY = 0.0f;
	g_dialogueScrollAccumulator = 0.0f;
}

PadDirection UpdateDialogueScrollSession(
	bool padTouched,
	bool padPressed,
	float axisX,
	float axisY,
	std::uint32_t packetNum)
{
	if (!padTouched || padPressed) {
		if (g_dialogueScrollSessionActive && g_dialogueScrollLogBudget > 0) {
			Log(
				"DialogueMenu right-pad touch-scroll session ended; touched=%d pressed=%d packet=%u remaining=%d",
				padTouched ? 1 : 0,
				padPressed ? 1 : 0,
				packetNum,
				--g_dialogueScrollLogBudget);
		}
		ResetDialogueScrollSession();
		return PadDirection::Center;
	}

	if (!g_dialogueScrollSessionActive) {
		g_dialogueScrollSessionActive = true;
		g_dialogueScrollStartedNearCenter = IsDialogueCenterTouch(axisX, axisY);
		g_dialogueScrollLastY = axisY;
		g_dialogueScrollAccumulator = 0.0f;
		if (g_dialogueScrollLogBudget > 0) {
			Log(
				"DialogueMenu right-pad touch-scroll session started; nearCenter=%d axis0=(%.2f, %.2f) packet=%u remaining=%d",
				g_dialogueScrollStartedNearCenter ? 1 : 0,
				axisX,
				axisY,
				packetNum,
				--g_dialogueScrollLogBudget);
		}
		return PadDirection::Center;
	}

	if (!g_dialogueScrollStartedNearCenter) {
		if (IsDialogueCenterTouch(axisX, axisY)) {
			g_dialogueScrollStartedNearCenter = true;
			g_dialogueScrollLastY = axisY;
			g_dialogueScrollAccumulator = 0.0f;
			if (g_dialogueScrollLogBudget > 0) {
				Log("DialogueMenu right-pad touch-scroll armed after returning to center; packet=%u remaining=%d", packetNum, --g_dialogueScrollLogBudget);
			}
		} else {
			g_dialogueScrollLastY = axisY;
		}
		return PadDirection::Center;
	}

	const float deltaY = axisY - g_dialogueScrollLastY;
	g_dialogueScrollLastY = axisY;

	if (!ShouldPassDialogueScrollAxis(axisX, axisY)) {
		g_dialogueScrollAccumulator = 0.0f;
		return PadDirection::Center;
	}

	g_dialogueScrollAccumulator += deltaY;
	if (g_dialogueScrollAccumulator >= kDialogueScrollStepAxis) {
		g_dialogueScrollAccumulator -= kDialogueScrollStepAxis;
		return PadDirection::Up;
	}
	if (g_dialogueScrollAccumulator <= -kDialogueScrollStepAxis) {
		g_dialogueScrollAccumulator += kDialogueScrollStepAxis;
		return PadDirection::Down;
	}

	return PadDirection::Center;
}

bool TryInvokeDialogueMoveOnObject(
	GFxValueLite& object,
	PadDirection direction,
	const char* targetLabel,
	const char** targetOut,
	const char** methodOut)
{
	if (!IsGFxObjectLike(object) || (direction != PadDirection::Up && direction != PadDirection::Down)) {
		return false;
	}

	const bool moveUp = direction == PadDirection::Up;
	const char* methods[] = {
		moveUp ? "moveSelectionUp" : "moveSelectionDown",
		moveUp ? "ScrollUp" : "ScrollDown",
	};

	for (const char* method : methods) {
		if (GFxObjectInvoke(object, method, nullptr, 0)) {
			if (targetOut) {
				*targetOut = targetLabel;
			}
			if (methodOut) {
				*methodOut = method;
			}
			return true;
		}
	}

	return false;
}

bool TryInvokeDialogueMoveAtKnownPath(
	void* movieRoot,
	PadDirection direction,
	const char** targetOut,
	const char** methodOut)
{
	if (!movieRoot || (direction != PadDirection::Up && direction != PadDirection::Down)) {
		return false;
	}

	const bool moveUp = direction == PadDirection::Up;
	const char* methods[] = {
		moveUp ? "moveSelectionUp" : "moveSelectionDown",
		moveUp ? "ScrollUp" : "ScrollDown",
	};
	const char* paths[] = {
		"root.DialogueList",
		"root.List_mc",
		"root.Menu_mc.DialogueList",
		"root.Menu_mc.List_mc",
		"root.DialogueMenu_mc.DialogueList",
		"root.DialogueMenu_mc.List_mc",
	};

	for (const char* path : paths) {
		for (const char* method : methods) {
			if (InvokeScaleformPathMethod(movieRoot, path, method)) {
				if (targetOut) {
					*targetOut = path;
				}
				if (methodOut) {
					*methodOut = method;
				}
				return true;
			}
		}
	}

	return false;
}

bool FindAndInvokeDialogueMove(
	GFxValueLite& element,
	PadDirection direction,
	int depth,
	int* nodesVisited,
	const char** targetOut,
	const char** methodOut)
{
	if (!IsGFxObjectLike(element) || depth > kScaleformElementSearchMaxDepth) {
		return false;
	}

	if (nodesVisited) {
		++(*nodesVisited);
		if (*nodesVisited > kScaleformElementSearchMaxNodes) {
			return false;
		}
	}

	const char* listNames[] = {
		"DialogueList",
		"List_mc",
	};
	for (const char* listName : listNames) {
		if (GFxObjectNameEquals(element, listName) &&
			TryInvokeDialogueMoveOnObject(element, direction, listName, targetOut, methodOut)) {
			return true;
		}
	}

	int childCount = 0;
	if (!TryReadGFxIntMember(element, "numChildren", &childCount) || childCount <= 0) {
		return false;
	}
	if (childCount > kScaleformElementSearchMaxChildren) {
		childCount = kScaleformElementSearchMaxChildren;
	}

	for (int index = 0; index < childCount; ++index) {
		GFxValueLite child{};
		GFxValueLite indexArg = MakeGFxInt(index);
		const bool gotChild = GFxObjectInvokeResult(element, "getChildAt", &child, &indexArg, 1);
		bool invoked = false;
		if (gotChild && IsGFxObjectLike(child)) {
			invoked = FindAndInvokeDialogueMove(child, direction, depth + 1, nodesVisited, targetOut, methodOut);
		}
		CleanGFxValue(child);
		if (invoked) {
			return true;
		}
	}

	return false;
}

bool InvokeDialogueMenuSelectionMove(
	void* menu,
	PadDirection direction,
	std::uint32_t packetNum,
	const char** targetOut,
	const char** methodOut)
{
	static bool loggedMissingRoot = false;
	static bool loggedInvokeFailure = false;

	if (targetOut) {
		*targetOut = nullptr;
	}
	if (methodOut) {
		*methodOut = nullptr;
	}

	if (!menu || (direction != PadDirection::Up && direction != PadDirection::Down)) {
		return false;
	}

	if (
		packetNum != 0 &&
		g_lastDialogueScrollInvokePacket == packetNum &&
		g_lastDialogueScrollInvokeDirection == direction) {
		if (targetOut) {
			*targetOut = "duplicate-packet";
		}
		if (methodOut) {
			*methodOut = "deduped";
		}
		return true;
	}

	void* movieRoot = GetMenuMovieRoot(menu);
	if (!movieRoot) {
		if (!loggedMissingRoot) {
			Log("DialogueMenu touch-scroll bridge could not resolve movie root; menu=%p", menu);
			loggedMissingRoot = true;
		}
		return false;
	}

	bool invoked = TryInvokeDialogueMoveAtKnownPath(movieRoot, direction, targetOut, methodOut);

	GFxValueLite root{};
	if (!invoked && GetScaleformVariable(movieRoot, "root", &root) && IsGFxObjectLike(root)) {
		int nodesVisited = 0;
		invoked = FindAndInvokeDialogueMove(root, direction, 0, &nodesVisited, targetOut, methodOut);
	}
	CleanGFxValue(root);

	if (invoked) {
		g_lastDialogueScrollInvokePacket = packetNum;
		g_lastDialogueScrollInvokeDirection = direction;
	} else if (!loggedInvokeFailure) {
		Log(
			"DialogueMenu touch-scroll bridge could not invoke moveSelection/Scroll for dir=%s; menu=%p root=%p",
			PadDirectionName(direction),
			menu,
			movieRoot);
		loggedInvokeFailure = true;
	}

	return invoked;
}

bool IsRightHandController(vr::IVRSystem* vrSystem, vr::TrackedDeviceIndex_t controllerDeviceIndex)
{
	if (!vrSystem || controllerDeviceIndex == vr::k_unTrackedDeviceIndexInvalid) {
		return false;
	}

	return vrSystem->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand) == controllerDeviceIndex;
}

void SuppressRightTouchpadForDialogue(vr::VRControllerState_t* outputControllerState, std::uint64_t touchpadMask)
{
	if (!outputControllerState) {
		return;
	}

	outputControllerState->rAxis[0].x = 0.0f;
	outputControllerState->rAxis[0].y = 0.0f;
	outputControllerState->ulButtonPressed &= ~touchpadMask;
	outputControllerState->ulButtonTouched &= ~touchpadMask;
}

bool DialogueMenuControllerStateCallback(
	vr::TrackedDeviceIndex_t controllerDeviceIndex,
	const vr::VRControllerState_t* controllerState,
	std::uint32_t controllerStateSize,
	vr::VRControllerState_t* outputControllerState)
{
	if (!controllerState || !outputControllerState || controllerStateSize < sizeof(vr::VRControllerState_t)) {
		return true;
	}

	if (!g_hookManager) {
		return true;
	}

	vr::IVRSystem* vrSystem = g_hookManager->GetVRSystem();
	if (!IsRightHandController(vrSystem, controllerDeviceIndex)) {
		// FO4VRTools invokes this callback for both hands. Left-hand samples must
		// not reset the active right-hand drag session between right-hand packets.
		return true;
	}

	void* menu = FindDialogueMenu();
	if (!menu) {
		if (g_dialogueScrollSessionActive && g_dialogueScrollLogBudget > 0) {
			Log("DialogueMenu right-pad touch-scroll session reset after DialogueMenu closed; remaining=%d", --g_dialogueScrollLogBudget);
		}
		ResetDialogueScrollSession();
		g_activeDialogueMenu = nullptr;
		return true;
	}

	if (menu != g_activeDialogueMenu) {
		ResetDialogueScrollSession();
		g_activeDialogueMenu = menu;
		if (g_dialogueScrollLogBudget > 0) {
			Log("DialogueMenu detected for touch-scroll bridge; menu=%p remaining=%d", menu, --g_dialogueScrollLogBudget);
		}
	}

	const std::uint64_t touchpadMask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad);
	const bool padTouched = (controllerState->ulButtonTouched & touchpadMask) != 0;
	const bool padPressed = (controllerState->ulButtonPressed & touchpadMask) != 0;
	const float rawAxisX = controllerState->rAxis[0].x;
	const float rawAxisY = controllerState->rAxis[0].y;
	const PadDirection movementDirection = UpdateDialogueScrollSession(
		padTouched,
		padPressed,
		rawAxisX,
		rawAxisY,
		controllerState->unPacketNum);

	if (movementDirection != PadDirection::Center) {
		const char* target = nullptr;
		const char* method = nullptr;
		const bool invoked = InvokeDialogueMenuSelectionMove(
			menu,
			movementDirection,
			controllerState->unPacketNum,
			&target,
			&method);
		if (g_dialogueScrollLogBudget > 0) {
			Log(
				"DialogueMenu right-pad touch-scroll step; dir=%s invoked=%d target=%s method=%s axis0=(%.2f, %.2f) packet=%u remaining=%d",
				PadDirectionName(movementDirection),
				invoked ? 1 : 0,
				target ? target : "<none>",
				method ? method : "<none>",
				rawAxisX,
				rawAxisY,
				controllerState->unPacketNum,
				--g_dialogueScrollLogBudget);
		}
	}

	SuppressRightTouchpadForDialogue(outputControllerState, touchpadMask);
	return true;
}

OpenVRHookManagerAPI* RequestFO4VRToolsHookManager()
{
	HMODULE toolsModule = GetModuleHandleA("FO4VRTools.dll");
	if (!toolsModule) {
		toolsModule = LoadLibraryA("FO4VRTools.dll");
	}

	if (!toolsModule) {
		Log("FO4VRTools.dll is not loaded or could not be loaded");
		return nullptr;
	}

	auto getHookManager = reinterpret_cast<GetVRHookManagerFn>(GetProcAddress(toolsModule, "GetVRHookManager"));
	if (!getHookManager) {
		Log("FO4VRTools.dll does not export GetVRHookManager");
		return nullptr;
	}

	return getHookManager();
}

bool RegisterDialogueMenuBridge()
{
	if (g_callbackRegistered) {
		return true;
	}

	g_hookManager = RequestFO4VRToolsHookManager();
	if (!g_hookManager) {
		return false;
	}

	g_hookManager->RegisterControllerStateCB(DialogueMenuControllerStateCallback);
	g_callbackRegistered = true;
	Log("Registered Vive Wand DialogueMenu touch-scroll bridge with FO4VRTools controller-state callback");

	return true;
}

void OnF4SEMessage(F4SEMessagingInterface::Message* message)
{
	if (!message) {
		return;
	}

	if (message->type == F4SEMessagingInterface::kMessagePostLoad ||
		message->type == F4SEMessagingInterface::kMessagePostPostLoad ||
		message->type == F4SEMessagingInterface::kMessageGameLoaded) {
		RegisterDialogueMenuBridge();
	}
}

} // namespace

extern "C" __declspec(dllexport) bool F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info)
{
	if (!info) {
		return false;
	}

	info->infoVersion = kPluginInfoVersion;
	info->name = "ViveWandCompatibilityPatch";
	info->version = kPluginVersion;

	if (!f4se) {
		return false;
	}

	if (f4se->isEditor) {
		Log("Refusing to load in editor");
		return false;
	}

	Log("Query passed; pluginVersion=%u runtimeVersion=0x%08X f4seVersion=0x%08X", kPluginVersion, f4se->runtimeVersion, f4se->f4seVersion);
	return true;
}

extern "C" __declspec(dllexport) bool F4SEPlugin_Load(const F4SEInterface* f4se)
{
	if (!f4se) {
		return false;
	}

	g_pluginHandle = f4se->GetPluginHandle ? f4se->GetPluginHandle() : kPluginHandleInvalid;
	Log("Loading; pluginHandle=0x%08X", g_pluginHandle);

	auto messaging = reinterpret_cast<F4SEMessagingInterface*>(
		f4se->QueryInterface ? f4se->QueryInterface(kInterfaceMessaging) : nullptr);

	if (messaging && g_pluginHandle != kPluginHandleInvalid) {
		if (messaging->RegisterListener(g_pluginHandle, "F4SE", OnF4SEMessage)) {
			Log("Registered F4SE message listener");
		} else {
			Log("Failed to register F4SE message listener; will try immediate callback registration");
		}
	}

	RegisterDialogueMenuBridge();
	return true;
}
