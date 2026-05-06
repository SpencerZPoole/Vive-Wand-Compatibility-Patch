#include <windows.h>
#include <shobjidl.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr const wchar_t* kInstallerTitle = L"Vive Wand Compatibility Patch OpenVR Installer";
constexpr const wchar_t* kAppId = L"611660";
constexpr const wchar_t* kWorkshopSection = L"Workshop";
constexpr const wchar_t* kItemRotationSpeedKey = L"fItemRotationSpeed";
constexpr const wchar_t* kItemRotationSpeedValue = L"1.0";
constexpr const wchar_t* kItemHoldDistantSpeedKey = L"fItemHoldDistantSpeed";
constexpr const wchar_t* kItemHoldDistantSpeedValue = L"3.0";

struct VersionStrings {
	std::wstring company;
	std::wstring description;
	std::wstring product;
};

struct DetectionResult {
	fs::path gamePath;
	std::wstring source;
	std::wstring error;
};

struct IniUpdateResult {
	std::wstring label;
	fs::path path;
	fs::path backupPath;
	bool success = false;
	bool changed = false;
	std::wstring message;
};

bool g_silent = false;

std::wstring ToLower(std::wstring value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
		return static_cast<wchar_t>(::towlower(ch));
	});
	return value;
}

bool ContainsInsensitive(const std::wstring& haystack, const std::wstring& needle)
{
	return ToLower(haystack).find(ToLower(needle)) != std::wstring::npos;
}

std::string Trim(std::string value)
{
	auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
	value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](char ch) {
		return !isSpace(static_cast<unsigned char>(ch));
	}));
	value.erase(std::find_if(value.rbegin(), value.rend(), [&](char ch) {
		return !isSpace(static_cast<unsigned char>(ch));
	}).base(), value.end());
	return value;
}

std::wstring Utf8ToWide(const std::string& value)
{
	if (value.empty()) {
		return {};
	}

	int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
	UINT codePage = CP_UTF8;
	DWORD flags = MB_ERR_INVALID_CHARS;
	if (count <= 0) {
		codePage = CP_ACP;
		flags = 0;
		count = MultiByteToWideChar(codePage, flags, value.data(), static_cast<int>(value.size()), nullptr, 0);
	}
	if (count <= 0) {
		return {};
	}

	std::wstring result(static_cast<std::size_t>(count), L'\0');
	MultiByteToWideChar(codePage, flags, value.data(), static_cast<int>(value.size()), result.data(), count);
	return result;
}

std::string WideToUtf8(const std::wstring& value)
{
	if (value.empty()) {
		return {};
	}

	int count = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
	if (count <= 0) {
		return {};
	}

	std::string result(static_cast<std::size_t>(count), '\0');
	WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), count, nullptr, nullptr);
	return result;
}

void ShowInfo(const std::wstring& message)
{
	if (!g_silent) {
		MessageBoxW(nullptr, message.c_str(), kInstallerTitle, MB_OK | MB_ICONINFORMATION);
	}
}

void ShowWarning(const std::wstring& message)
{
	if (!g_silent) {
		MessageBoxW(nullptr, message.c_str(), kInstallerTitle, MB_OK | MB_ICONWARNING);
	}
}

void ShowError(const std::wstring& message)
{
	if (!g_silent) {
		MessageBoxW(nullptr, message.c_str(), kInstallerTitle, MB_OK | MB_ICONERROR);
	}
}

fs::path GetModulePath()
{
	std::wstring buffer(MAX_PATH, L'\0');
	for (;;) {
		DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
		if (size == 0) {
			return {};
		}
		if (size < buffer.size() - 1) {
			buffer.resize(size);
			return fs::path(buffer);
		}
		buffer.resize(buffer.size() * 2);
	}
}

bool LooksLikePatchModRoot(const fs::path& path)
{
	return fs::exists(path / L"F4SE") ||
		fs::exists(path / L"meta.ini") ||
		fs::exists(path / L"README.md");
}

fs::path ResolveModRoot(const fs::path& exeDir)
{
	if (LooksLikePatchModRoot(exeDir)) {
		return exeDir;
	}

	fs::path candidate = exeDir;
	if (candidate.filename() == L"dist" &&
		candidate.parent_path().filename() == L"OpenVRInstaller" &&
		candidate.parent_path().parent_path().filename() == L"Source") {
		fs::path modRoot = candidate.parent_path().parent_path().parent_path();
		if (LooksLikePatchModRoot(modRoot)) {
			return modRoot;
		}
	}

	return exeDir;
}

fs::path FindMoRoot(const fs::path& start)
{
	fs::path current = start;
	for (int i = 0; i < 8 && !current.empty(); ++i) {
		if (fs::exists(current / L"ModOrganizer.ini")) {
			return current;
		}
		if (!current.has_parent_path() || current.parent_path() == current) {
			break;
		}
		current = current.parent_path();
	}
	return {};
}

std::string ReadTextFile(const fs::path& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		return {};
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string FindIniValue(const std::string& text, const std::string& key)
{
	std::istringstream stream(text);
	std::string line;
	const std::string prefix = key + "=";
	while (std::getline(stream, line)) {
		line = Trim(line);
		if (line.rfind(prefix, 0) == 0) {
			return Trim(line.substr(prefix.size()));
		}
	}
	return {};
}

std::string DecodeMoIniValue(std::string value)
{
	value = Trim(value);
	if (value.rfind("@ByteArray(", 0) == 0 && value.size() >= 12 && value.back() == ')') {
		value = value.substr(11, value.size() - 12);
	}
	if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
		value = value.substr(1, value.size() - 2);
	}

	std::string decoded;
	decoded.reserve(value.size());
	for (std::size_t i = 0; i < value.size(); ++i) {
		if (value[i] == '\\' && i + 1 < value.size()) {
			char next = value[i + 1];
			if (next == '\\' || next == '/') {
				decoded.push_back(next == '/' ? '/' : '\\');
				++i;
				continue;
			}
		}
		decoded.push_back(value[i]);
	}
	return decoded;
}

bool ReadRegistryString(HKEY root, const wchar_t* subkey, const wchar_t* valueName, std::wstring* out)
{
	HKEY key = nullptr;
	if (RegOpenKeyExW(root, subkey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
		return false;
	}

	DWORD type = 0;
	DWORD size = 0;
	LSTATUS status = RegQueryValueExW(key, valueName, nullptr, &type, nullptr, &size);
	if (status != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ) || size == 0) {
		RegCloseKey(key);
		return false;
	}

	std::wstring value(size / sizeof(wchar_t), L'\0');
	status = RegQueryValueExW(key, valueName, nullptr, &type, reinterpret_cast<LPBYTE>(value.data()), &size);
	RegCloseKey(key);
	if (status != ERROR_SUCCESS) {
		return false;
	}

	value.resize(wcsnlen_s(value.c_str(), value.size()));
	if (type == REG_EXPAND_SZ) {
		std::wstring expanded(32767, L'\0');
		DWORD expandedSize = ExpandEnvironmentStringsW(value.c_str(), expanded.data(), static_cast<DWORD>(expanded.size()));
		if (expandedSize > 0 && expandedSize < expanded.size()) {
			expanded.resize(expandedSize - 1);
			value = expanded;
		}
	}

	*out = value;
	return true;
}

std::vector<fs::path> GetSteamRoots()
{
	std::vector<fs::path> roots;
	std::wstring value;
	if (ReadRegistryString(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath", &value)) {
		std::replace(value.begin(), value.end(), L'/', L'\\');
		roots.emplace_back(value);
	}
	if (ReadRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam", L"InstallPath", &value)) {
		roots.emplace_back(value);
	}
	if (ReadRegistryString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Valve\\Steam", L"InstallPath", &value)) {
		roots.emplace_back(value);
	}

	std::vector<fs::path> common = {
		L"C:\\Program Files (x86)\\Steam",
		L"C:\\Program Files\\Steam",
		L"F:\\Steam",
		L"G:\\SteamLibrary"
	};
	roots.insert(roots.end(), common.begin(), common.end());

	std::sort(roots.begin(), roots.end());
	roots.erase(std::unique(roots.begin(), roots.end()), roots.end());
	return roots;
}

std::vector<fs::path> ParseSteamLibraryFolders(const fs::path& steamRoot)
{
	std::vector<fs::path> libraries;
	libraries.push_back(steamRoot);

	std::string text = ReadTextFile(steamRoot / L"steamapps" / L"libraryfolders.vdf");
	std::istringstream stream(text);
	std::string line;
	while (std::getline(stream, line)) {
		if (line.find("\"path\"") == std::string::npos) {
			continue;
		}

		std::size_t first = line.find('"', line.find("\"path\"") + 6);
		if (first == std::string::npos) {
			continue;
		}
		std::size_t second = line.find('"', first + 1);
		if (second == std::string::npos) {
			continue;
		}
		std::string raw = line.substr(first + 1, second - first - 1);
		libraries.emplace_back(Utf8ToWide(DecodeMoIniValue(raw)));
	}

	std::sort(libraries.begin(), libraries.end());
	libraries.erase(std::unique(libraries.begin(), libraries.end()), libraries.end());
	return libraries;
}

std::wstring ParseSteamManifestInstallDir(const fs::path& manifest)
{
	std::string text = ReadTextFile(manifest);
	std::istringstream stream(text);
	std::string line;
	while (std::getline(stream, line)) {
		if (line.find("\"installdir\"") == std::string::npos) {
			continue;
		}
		std::vector<std::string> quoted;
		for (std::size_t pos = 0; pos < line.size();) {
			std::size_t a = line.find('"', pos);
			if (a == std::string::npos) {
				break;
			}
			std::size_t b = line.find('"', a + 1);
			if (b == std::string::npos) {
				break;
			}
			quoted.push_back(line.substr(a + 1, b - a - 1));
			pos = b + 1;
		}
		if (quoted.size() >= 2) {
			return Utf8ToWide(DecodeMoIniValue(quoted[1]));
		}
	}
	return {};
}

std::wstring QueryVersionString(const fs::path& filePath, const wchar_t* field)
{
	DWORD handle = 0;
	DWORD size = GetFileVersionInfoSizeW(filePath.c_str(), &handle);
	if (size == 0) {
		return {};
	}

	std::vector<BYTE> data(size);
	if (!GetFileVersionInfoW(filePath.c_str(), 0, size, data.data())) {
		return {};
	}

	struct Translation {
		WORD language;
		WORD codePage;
	};

	Translation* translations = nullptr;
	UINT translationBytes = 0;
	WORD language = 0x0409;
	WORD codePage = 0x04B0;
	if (VerQueryValueW(data.data(), L"\\VarFileInfo\\Translation", reinterpret_cast<void**>(&translations), &translationBytes) &&
		translationBytes >= sizeof(Translation)) {
		language = translations[0].language;
		codePage = translations[0].codePage;
	}

	wchar_t query[256];
	swprintf_s(query, L"\\StringFileInfo\\%04x%04x\\%s", language, codePage, field);
	wchar_t* value = nullptr;
	UINT valueLength = 0;
	if (!VerQueryValueW(data.data(), query, reinterpret_cast<void**>(&value), &valueLength) || !value) {
		return {};
	}

	return std::wstring(value, valueLength > 0 ? valueLength - 1 : 0);
}

VersionStrings ReadVersionStrings(const fs::path& dllPath)
{
	VersionStrings info;
	info.company = QueryVersionString(dllPath, L"CompanyName");
	info.description = QueryVersionString(dllPath, L"FileDescription");
	info.product = QueryVersionString(dllPath, L"ProductName");
	return info;
}

bool IsValidFallout4VrGamePath(const fs::path& gamePath, std::wstring* reason)
{
	if (gamePath.empty()) {
		if (reason) {
			*reason = L"No Fallout 4 VR folder was selected or detected.";
		}
		return false;
	}

	if (!fs::exists(gamePath / L"Fallout4VR.exe")) {
		if (reason) {
			*reason = L"Folder does not contain Fallout4VR.exe.";
		}
		return false;
	}

	fs::path openVr = gamePath / L"openvr_api.dll";
	if (!fs::exists(openVr)) {
		if (reason) {
			*reason = L"Folder does not contain openvr_api.dll.";
		}
		return false;
	}

	VersionStrings version = ReadVersionStrings(openVr);
	if (!ContainsInsensitive(version.company, L"Valve") ||
		(!ContainsInsensitive(version.description, L"OpenVR") && !ContainsInsensitive(version.product, L"OpenVR"))) {
		if (reason) {
			*reason = L"openvr_api.dll exists, but it does not look like Valve's OpenVR loader.";
		}
		return false;
	}

	return true;
}

DetectionResult DetectGamePathFromMo(const fs::path& moRoot)
{
	DetectionResult result;
	if (moRoot.empty()) {
		result.error = L"ModOrganizer.ini was not found near the installer.";
		return result;
	}

	std::string text = ReadTextFile(moRoot / L"ModOrganizer.ini");
	std::string raw = FindIniValue(text, "gamePath");
	if (raw.empty()) {
		result.error = L"ModOrganizer.ini did not contain gamePath.";
		return result;
	}

	result.gamePath = fs::path(Utf8ToWide(DecodeMoIniValue(raw)));
	result.source = L"ModOrganizer.ini gamePath";
	return result;
}

DetectionResult DetectGamePathFromSteam()
{
	DetectionResult result;
	for (const fs::path& steamRoot : GetSteamRoots()) {
		if (!fs::exists(steamRoot)) {
			continue;
		}
		for (const fs::path& library : ParseSteamLibraryFolders(steamRoot)) {
			fs::path manifest = library / L"steamapps" / (std::wstring(L"appmanifest_") + kAppId + L".acf");
			fs::path gamePath = library / L"steamapps" / L"common" / L"Fallout 4 VR";
			if (fs::exists(manifest)) {
				std::wstring installDir = ParseSteamManifestInstallDir(manifest);
				if (!installDir.empty()) {
					gamePath = library / L"steamapps" / L"common" / installDir;
				}
			}

			std::wstring reason;
			if (IsValidFallout4VrGamePath(gamePath, &reason)) {
				result.gamePath = gamePath;
				result.source = L"Steam library/appmanifest detection";
				return result;
			}
		}
	}

	result.error = L"Steam registry/library detection did not find Fallout 4 VR.";
	return result;
}

DetectionResult DetectGamePathFromCommonPaths()
{
	DetectionResult result;
	std::vector<fs::path> paths = {
		L"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Fallout 4 VR",
		L"C:\\Program Files\\Steam\\steamapps\\common\\Fallout 4 VR",
		L"F:\\Steam\\steamapps\\common\\Fallout 4 VR",
		L"G:\\SteamLibrary\\steamapps\\common\\Fallout 4 VR"
	};

	for (const fs::path& path : paths) {
		std::wstring reason;
		if (IsValidFallout4VrGamePath(path, &reason)) {
			result.gamePath = path;
			result.source = L"common Steam path fallback";
			return result;
		}
	}

	result.error = L"Common Steam paths did not contain Fallout 4 VR.";
	return result;
}

fs::path BrowseForGameFolder()
{
	if (g_silent) {
		return {};
	}

	IFileDialog* dialog = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
	if (FAILED(hr) || !dialog) {
		return {};
	}

	DWORD options = 0;
	dialog->GetOptions(&options);
	dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
	dialog->SetTitle(L"Select your Fallout 4 VR game folder");

	fs::path selected;
	hr = dialog->Show(nullptr);
	if (SUCCEEDED(hr)) {
		IShellItem* item = nullptr;
		if (SUCCEEDED(dialog->GetResult(&item)) && item) {
			PWSTR path = nullptr;
			if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path) {
				selected = fs::path(path);
				CoTaskMemFree(path);
			}
			item->Release();
		}
	}

	dialog->Release();
	return selected;
}

DetectionResult DetectGamePath(const fs::path& moRoot)
{
	std::vector<DetectionResult> attempts;
	attempts.push_back(DetectGamePathFromMo(moRoot));
	attempts.push_back(DetectGamePathFromSteam());
	attempts.push_back(DetectGamePathFromCommonPaths());

	for (DetectionResult& attempt : attempts) {
		std::wstring reason;
		if (IsValidFallout4VrGamePath(attempt.gamePath, &reason)) {
			return attempt;
		}
		attempt.error = attempt.error.empty() ? reason : attempt.error;
	}

	fs::path browsed = BrowseForGameFolder();
	DetectionResult browseResult;
	browseResult.gamePath = browsed;
	browseResult.source = L"manual folder selection";
	std::wstring reason;
	if (IsValidFallout4VrGamePath(browsed, &reason)) {
		return browseResult;
	}
	browseResult.error = reason;

	std::wostringstream error;
	error << L"Could not locate a valid Fallout 4 VR game folder.\n\n";
	for (const DetectionResult& attempt : attempts) {
		if (!attempt.error.empty()) {
			error << L"- " << attempt.error << L"\n";
		}
	}
	if (!browseResult.error.empty()) {
		error << L"- " << browseResult.error << L"\n";
	}
	browseResult.error = error.str();
	return browseResult;
}

bool FilesEqual(const fs::path& left, const fs::path& right)
{
	if (!fs::exists(left) || !fs::exists(right) || fs::file_size(left) != fs::file_size(right)) {
		return false;
	}

	std::ifstream a(left, std::ios::binary);
	std::ifstream b(right, std::ios::binary);
	if (!a || !b) {
		return false;
	}

	std::vector<char> abuf(64 * 1024);
	std::vector<char> bbuf(64 * 1024);
	while (a && b) {
		a.read(abuf.data(), static_cast<std::streamsize>(abuf.size()));
		b.read(bbuf.data(), static_cast<std::streamsize>(bbuf.size()));
		if (a.gcount() != b.gcount()) {
			return false;
		}
		if (!std::equal(abuf.begin(), abuf.begin() + a.gcount(), bbuf.begin())) {
			return false;
		}
	}

	return true;
}

std::wstring Timestamp()
{
	SYSTEMTIME local{};
	GetLocalTime(&local);
	wchar_t buffer[32];
	swprintf_s(buffer, L"%04u%02u%02u-%02u%02u%02u",
		local.wYear,
		local.wMonth,
		local.wDay,
		local.wHour,
		local.wMinute,
		local.wSecond);
	return buffer;
}

struct ModEntry {
	std::wstring name;
	wchar_t state = L'\0';
	int index = -1;
};

std::vector<ModEntry> ReadModList(const fs::path& modListPath)
{
	std::vector<ModEntry> entries;
	std::string text = ReadTextFile(modListPath);
	std::istringstream stream(text);
	std::string line;
	int index = 0;
	while (std::getline(stream, line)) {
		line = Trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}

		ModEntry entry;
		entry.index = index++;
		if (line[0] == '+' || line[0] == '-') {
			entry.state = static_cast<wchar_t>(line[0]);
			line = line.substr(1);
		}
		entry.name = Utf8ToWide(line);
		entries.push_back(entry);
	}
	return entries;
}

std::wstring FindSelectedProfile(const fs::path& moRoot)
{
	std::string text = ReadTextFile(moRoot / L"ModOrganizer.ini");
	std::string raw = FindIniValue(text, "selected_profile");
	return Utf8ToWide(DecodeMoIniValue(raw));
}

std::wstring CheckPriority(const fs::path& moRoot, const fs::path& modRoot)
{
	if (moRoot.empty()) {
		return L"Could not find ModOrganizer.ini, so priority could not be checked. Make sure Vive Wand Compatibility Patch is below Root Fix / wins conflicts in MO2.";
	}

	std::wstring selectedProfile = FindSelectedProfile(moRoot);
	if (selectedProfile.empty()) {
		return L"Could not determine the active MO2 profile, so priority could not be checked. Make sure Vive Wand Compatibility Patch is below Root Fix / wins conflicts in MO2.";
	}

	fs::path modList = moRoot / L"profiles" / selectedProfile / L"modlist.txt";
	if (!fs::exists(modList)) {
		return L"Could not read the active profile's modlist.txt, so priority could not be checked. Make sure Vive Wand Compatibility Patch is below Root Fix / wins conflicts in MO2.";
	}

	std::vector<ModEntry> entries = ReadModList(modList);
	std::wstring patchName = modRoot.filename().wstring();
	int patchIndex = -1;
	wchar_t patchState = L'\0';
	int rootFixIndex = -1;
	wchar_t rootFixState = L'\0';
	for (const ModEntry& entry : entries) {
		std::wstring lowered = ToLower(entry.name);
		if (lowered == ToLower(patchName) || lowered == L"vive wand compatibility patch") {
			patchIndex = entry.index;
			patchState = entry.state;
		}
		if (lowered == L"root fix") {
			rootFixIndex = entry.index;
			rootFixState = entry.state;
		}
	}

	if (patchIndex < 0) {
		return L"Could not find this mod in the active MO2 profile. Make sure Vive Wand Compatibility Patch is enabled and wins conflicts over Root Fix.";
	}
	if (patchState == L'-') {
		return L"Vive Wand Compatibility Patch appears disabled in the active MO2 profile. Enable it and make it win conflicts over Root Fix.";
	}
	if (rootFixIndex < 0 || rootFixState == L'-') {
		return L"Priority check did not find enabled Root Fix. If Root Fix is enabled under another name, make sure this patch wins conflicts over it.";
	}
	// MO2's modlist.txt stores high-priority/bottom-of-left-pane mods earlier in the file.
	if (patchIndex > rootFixIndex) {
		return L"Vive Wand Compatibility Patch appears lower priority than Root Fix. Move it lower in MO2's left pane / make it win conflicts so this OpenVR DLL wins.";
	}

	return {};
}

fs::path GetActiveProfileFalloutCustomIni(const fs::path& moRoot)
{
	if (moRoot.empty()) {
		return {};
	}

	std::wstring selectedProfile = FindSelectedProfile(moRoot);
	if (selectedProfile.empty()) {
		return {};
	}

	fs::path profileDir = moRoot / L"profiles" / selectedProfile;
	if (!fs::exists(profileDir)) {
		return {};
	}

	return profileDir / L"fallout4custom.ini";
}

std::wstring ReadIniValue(const fs::path& iniPath, const wchar_t* section, const wchar_t* key)
{
	wchar_t buffer[256]{};
	GetPrivateProfileStringW(section, key, L"", buffer, static_cast<DWORD>(std::size(buffer)), iniPath.c_str());
	return buffer;
}

bool IniHasWorkshopValues(const fs::path& iniPath)
{
	return ReadIniValue(iniPath, kWorkshopSection, kItemRotationSpeedKey) == kItemRotationSpeedValue &&
		ReadIniValue(iniPath, kWorkshopSection, kItemHoldDistantSpeedKey) == kItemHoldDistantSpeedValue;
}

bool WriteIniValue(const fs::path& iniPath, const wchar_t* section, const wchar_t* key, const wchar_t* value)
{
	return WritePrivateProfileStringW(section, key, value, iniPath.c_str()) != FALSE;
}

IniUpdateResult UpdateWorkshopIni(const std::wstring& label, const fs::path& iniPath)
{
	IniUpdateResult result;
	result.label = label;
	result.path = iniPath;

	if (iniPath.empty()) {
		result.message = L"No INI path was resolved.";
		return result;
	}

	try {
		const bool existed = fs::exists(iniPath);
		if (existed && IniHasWorkshopValues(iniPath)) {
			result.success = true;
			result.changed = false;
			result.message = L"Already had the requested Workshop values.";
			return result;
		}

		fs::create_directories(iniPath.parent_path());
		if (existed) {
			result.backupPath = iniPath.parent_path() /
				(iniPath.stem().wstring() + L".backup-" + Timestamp() + iniPath.extension().wstring());
			fs::copy_file(iniPath, result.backupPath, fs::copy_options::overwrite_existing);
		}

		const bool wroteRotation = WriteIniValue(iniPath, kWorkshopSection, kItemRotationSpeedKey, kItemRotationSpeedValue);
		const bool wroteDistance = WriteIniValue(iniPath, kWorkshopSection, kItemHoldDistantSpeedKey, kItemHoldDistantSpeedValue);
		WritePrivateProfileStringW(nullptr, nullptr, nullptr, iniPath.c_str());

		if (!wroteRotation || !wroteDistance || !IniHasWorkshopValues(iniPath)) {
			result.message = L"Failed to write or verify the requested Workshop values.";
			return result;
		}

		result.success = true;
		result.changed = true;
		result.message = existed ?
			L"Updated Workshop values and backed up the previous INI." :
			L"Created INI and wrote requested Workshop values.";
	}
	catch (const std::exception& ex) {
		result.success = false;
		result.message = Utf8ToWide(ex.what());
	}

	return result;
}

std::vector<IniUpdateResult> UpdateWorkshopIniTargets(const fs::path& moRoot)
{
	std::vector<IniUpdateResult> results;
	fs::path activeProfileIni = GetActiveProfileFalloutCustomIni(moRoot);
	if (!activeProfileIni.empty()) {
		results.push_back(UpdateWorkshopIni(L"active MO2 profile Fallout4Custom.ini", activeProfileIni));
	}

	return results;
}

bool HasIniWarning(const std::vector<IniUpdateResult>& results)
{
	if (results.empty()) {
		return true;
	}

	for (const IniUpdateResult& result : results) {
		if (!result.success) {
			return true;
		}
	}
	return false;
}

std::wstring FormatIniResults(const std::vector<IniUpdateResult>& results)
{
	if (results.empty()) {
		return L"Workshop INI settings:\nNo active MO2 profile Fallout4Custom.ini target could be resolved.";
	}

	std::wostringstream summary;
	summary << L"Workshop INI settings:\n";
	for (const IniUpdateResult& result : results) {
		summary << L"- " << result.label << L": ";
		if (result.success) {
			summary << (result.changed ? L"updated" : L"already set");
		}
		else {
			summary << L"warning";
		}
		summary << L"\n  " << result.path.wstring();
		if (!result.message.empty()) {
			summary << L"\n  " << result.message;
		}
		if (!result.backupPath.empty()) {
			summary << L"\n  Backup: " << result.backupPath.wstring();
		}
		summary << L"\n";
	}

	return summary.str();
}

void WriteLog(
	const fs::path& modRoot,
	const fs::path& sourceDll,
	const fs::path& destDll,
	const fs::path& backupDll,
	const std::wstring& detectionSource,
	const std::wstring& priorityWarning,
	const std::vector<IniUpdateResult>& iniResults,
	bool success,
	const std::wstring& message)
{
	std::ofstream log(modRoot / L"OpenVRInstall.log", std::ios::binary | std::ios::app);
	if (!log) {
		return;
	}

	log << "==== " << WideToUtf8(Timestamp()) << " ====\n";
	log << "success=" << (success ? "true" : "false") << "\n";
	log << "detectionSource=" << WideToUtf8(detectionSource) << "\n";
	log << "sourceDll=" << WideToUtf8(sourceDll.wstring()) << "\n";
	log << "destDll=" << WideToUtf8(destDll.wstring()) << "\n";
	if (!backupDll.empty()) {
		log << "backupDll=" << WideToUtf8(backupDll.wstring()) << "\n";
	}
	if (!priorityWarning.empty()) {
		log << "priorityWarning=" << WideToUtf8(priorityWarning) << "\n";
	}
	for (const IniUpdateResult& result : iniResults) {
		log << "iniTarget=" << WideToUtf8(result.label) << "\n";
		log << "iniPath=" << WideToUtf8(result.path.wstring()) << "\n";
		log << "iniSuccess=" << (result.success ? "true" : "false") << "\n";
		log << "iniChanged=" << (result.changed ? "true" : "false") << "\n";
		if (!result.backupPath.empty()) {
			log << "iniBackup=" << WideToUtf8(result.backupPath.wstring()) << "\n";
		}
		if (!result.message.empty()) {
			log << "iniMessage=" << WideToUtf8(result.message) << "\n";
		}
	}
	log << "message=" << WideToUtf8(message) << "\n\n";
}

int RunInstaller()
{
	fs::path exePath = GetModulePath();
	fs::path modRoot = ResolveModRoot(exePath.parent_path());
	fs::path moRoot = FindMoRoot(modRoot);

	DetectionResult detected = DetectGamePath(moRoot);
	std::wstring reason;
	if (!IsValidFallout4VrGamePath(detected.gamePath, &reason)) {
		std::wstring error = detected.error.empty() ? reason : detected.error;
		WriteLog(modRoot, {}, {}, {}, detected.source, {}, {}, false, error);
		ShowError(L"Install failed.\n\n" + error + L"\n\nNo files were copied.");
		return 1;
	}

	fs::path sourceDll = detected.gamePath / L"openvr_api.dll";
	fs::path rootDir = modRoot / L"Root";
	fs::path destDll = rootDir / L"openvr_api.dll";
	fs::path backupDll;

	try {
		fs::create_directories(rootDir);
		if (fs::exists(destDll)) {
			backupDll = rootDir / (std::wstring(L"openvr_api.backup-") + Timestamp() + L".dll");
			fs::copy_file(destDll, backupDll, fs::copy_options::overwrite_existing);
		}

		fs::copy_file(sourceDll, destDll, fs::copy_options::overwrite_existing);
		if (!FilesEqual(sourceDll, destDll)) {
			throw std::runtime_error("Copied openvr_api.dll did not match the source file.");
		}
	}
	catch (const std::exception& ex) {
		std::wstring error = L"Could not install openvr_api.dll: " + Utf8ToWide(ex.what());
		WriteLog(modRoot, sourceDll, destDll, backupDll, detected.source, {}, {}, false, error);
		ShowError(L"Install failed.\n\n" + error);
		return 2;
	}

	std::vector<IniUpdateResult> iniResults = UpdateWorkshopIniTargets(moRoot);
	std::wstring priorityWarning = CheckPriority(moRoot, modRoot);
	std::wostringstream success;
	success << L"Vive Wand Compatibility Patch OpenVR install complete.\n\n"
		<< L"Copied vanilla OpenVR from:\n" << sourceDll.wstring() << L"\n\n"
		<< L"Installed to:\n" << destDll.wstring();
	if (!backupDll.empty()) {
		success << L"\n\nBacked up previous file to:\n" << backupDll.wstring();
	}
	if (!priorityWarning.empty()) {
		success << L"\n\nWarning:\n" << priorityWarning;
	}
	else {
		success << L"\n\nMO2 priority check passed.";
	}
	success << L"\n\n" << FormatIniResults(iniResults);

	WriteLog(modRoot, sourceDll, destDll, backupDll, detected.source, priorityWarning, iniResults, true, success.str());
	if (!priorityWarning.empty() || HasIniWarning(iniResults)) {
		ShowWarning(success.str());
	}
	else {
		ShowInfo(success.str());
	}
	return 0;
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR commandLine, int)
{
	std::wstring args = commandLine ? commandLine : L"";
	g_silent = ContainsInsensitive(args, L"--silent") || ContainsInsensitive(args, L"/silent");

	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	int result = RunInstaller();
	if (SUCCEEDED(hr)) {
		CoUninitialize();
	}
	return result;
}
