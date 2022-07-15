#include "GuiUtils.h"
#include "window.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include "imgui/imgui.h"
#include <shlobj_core.h>

static std::wstring simpleCharToWcharConvert(const char* str) {
	if (!str) return {};
	size_t len = strlen(str);
	return { str, str + len };
}

static std::wstring filterCharToWcharConvert(const char* str) {
	if (!str) return {};
	const char* ptr = str;
	// find a double null char
	while (*ptr || *(ptr + 1)) ptr++;
	return { str, ptr + 1 };
}

// ImGui InputCallback for std::string
int GuiUtils::IGStdStringInputCallback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		std::string* str = (std::string*)data->UserData;
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->data();
	}
	return 0;
}

void GuiUtils::MsgBox(Window* window, const char* message, int flags)
{
	MessageBoxA((HWND)window->getNativeWindow(), message, "XXL Editor", (UINT)flags);
}

std::filesystem::path GuiUtils::OpenDialogBox(Window* window, const char* filter, const char* defExt)
{
	wchar_t filepath[MAX_PATH + 1] = L"\0";
	OPENFILENAMEW ofn = {};
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = (HWND)window->getNativeWindow();
	ofn.hInstance = GetModuleHandle(NULL);
	std::wstring wFilter = filterCharToWcharConvert(filter);
	ofn.lpstrFilter = wFilter.c_str();
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	std::wstring wExt = simpleCharToWcharConvert(defExt);
	ofn.lpstrDefExt = wExt.c_str();
	if (GetOpenFileNameW(&ofn))
		return filepath;
	return {};
}

std::vector<std::filesystem::path> GuiUtils::MultiOpenDialogBox(Window* window, const char* filter, const char* defExt)
{
	wchar_t filepath[1025] = L"\0";
	OPENFILENAMEW ofn = {};
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = (HWND)window->getNativeWindow();
	ofn.hInstance = GetModuleHandle(NULL);
	std::wstring wFilter = filterCharToWcharConvert(filter);
	ofn.lpstrFilter = wFilter.c_str();
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	std::wstring wExt = simpleCharToWcharConvert(defExt);
	ofn.lpstrDefExt = wExt.c_str();
	if (GetOpenFileNameW(&ofn)) {
		std::wstring folder = filepath;
		const wchar_t* nextfile = filepath + folder.size() + 1;
		if (*nextfile) {
			std::vector<std::filesystem::path> list;
			while (*nextfile) {
				size_t len = wcslen(nextfile);
				list.emplace_back((folder + L'\\').append(std::wstring_view(nextfile, len)));
				nextfile += len + 1;
			}
			return list;
		}
		else {
			return { std::move(folder) };
		}
	}
	return {};
}

std::filesystem::path GuiUtils::SaveDialogBox(Window* window, const char* filter, const char* defExt, const std::filesystem::path& defName)
{
	wchar_t filepath[MAX_PATH + 1] = L"\0";
	if (!defName.empty())
		wcscpy_s(filepath, defName.c_str());
	OPENFILENAMEW ofn = {};
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)window->getNativeWindow();
	ofn.hInstance = GetModuleHandle(NULL);
	std::wstring wFilter = filterCharToWcharConvert(filter);
	ofn.lpstrFilter = wFilter.c_str();
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	std::wstring wExt = simpleCharToWcharConvert(defExt);
	ofn.lpstrDefExt = wExt.c_str();
	if (GetSaveFileNameW(&ofn))
		return filepath;
	return {};
}

std::filesystem::path GuiUtils::SelectFolderDialogBox(Window* window, const char* text)
{
	wchar_t dirname[MAX_PATH + 1];
	BROWSEINFOW bri;
	memset(&bri, 0, sizeof(bri));
	bri.hwndOwner = (HWND)window->getNativeWindow();
	bri.pszDisplayName = dirname;
	std::wstring wText(text, text + strlen(text));
	bri.lpszTitle = wText.c_str();
	bri.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
	PIDLIST_ABSOLUTE pid = SHBrowseForFolderW(&bri);
	if (pid != NULL) {
		SHGetPathFromIDListW(pid, dirname);
		return dirname;
	}
	return {};
}

std::string GuiUtils::latinToUtf8(std::string_view text)
{
	// latin -> UTF-16
	int widesize = MultiByteToWideChar(1252, 0, text.data(), text.size(), NULL, 0);
	wchar_t* widename = (wchar_t*)alloca(2 * widesize);
	MultiByteToWideChar(1252, 0, text.data(), text.size(), widename, widesize);

	// UTF-16 -> UTF-8
	int u8size = WideCharToMultiByte(CP_UTF8, 0, widename, widesize, NULL, 0, NULL, NULL);
	char* u8name = (char*)alloca(u8size);
	WideCharToMultiByte(CP_UTF8, 0, widename, widesize, u8name, u8size, NULL, NULL);

	return std::string(u8name, u8size);
}

std::string GuiUtils::utf8ToLatin(std::string_view text)
{
	// UTF-8 -> UTF-16
	int widesize = MultiByteToWideChar(CP_UTF8, 0, text.data(), text.size(), NULL, 0);
	wchar_t* widename = (wchar_t*)alloca(2 * widesize);
	MultiByteToWideChar(CP_UTF8, 0, text.data(), text.size(), widename, widesize);

	// UTF-16 -> latin
	int latinsize = WideCharToMultiByte(1252, 0, widename, widesize, NULL, 0, NULL, NULL);
	char* latinname = (char*)alloca(latinsize);
	WideCharToMultiByte(1252, 0, widename, widesize, latinname, latinsize, "?", NULL);

	return std::string(latinname, latinsize);
}


std::string GuiUtils::wcharToUtf8(std::wstring_view text)
{
	// UTF-16 -> UTF-8
	int u8size = WideCharToMultiByte(CP_UTF8, 0, text.data(), text.size(), NULL, 0, NULL, NULL);
	char* u8name = (char*)alloca(u8size);
	WideCharToMultiByte(CP_UTF8, 0, text.data(), text.size(), u8name, u8size, NULL, NULL);
	return std::string(u8name, u8size);
}

std::wstring GuiUtils::utf8ToWchar(std::string_view text)
{
	int widesize = MultiByteToWideChar(CP_UTF8, 0, text.data(), text.size(), NULL, 0);
	wchar_t* widename = (wchar_t*)alloca(2 * widesize);
	MultiByteToWideChar(CP_UTF8, 0, text.data(), text.size(), widename, widesize);
	return std::wstring(widename, widesize);
}

errno_t GuiUtils::fsfopen_s(FILE** streamptr, const std::filesystem::path& filename, const char* mode)
{
	return _wfopen_s(streamptr, filename.c_str(), std::wstring(mode, mode + strlen(mode)).c_str());
}
