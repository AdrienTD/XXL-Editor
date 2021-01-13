#include "GuiUtils.h"
#include "window.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include "imgui/imgui.h"

// ImGui InputCallback for std::string
int GuiUtils::IGStdStringInputCallback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
		std::string* str = (std::string*)data->UserData;
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->data();
	}
	return 0;
}

std::string GuiUtils::OpenDialogBox(Window* window, const char* filter, const char* defExt)
{
	char filepath[MAX_PATH + 1] = "\0";
	OPENFILENAME ofn = {};
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)window->getNativeWindow();
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = defExt;
	if (GetOpenFileNameA(&ofn))
		return filepath;
	return std::string();
}

std::string GuiUtils::SaveDialogBox(Window* window, const char* filter, const char* defExt, const char* defName)
{
	char filepath[MAX_PATH + 1] = "\0";
	if (defName)
		strcpy_s(filepath, defName);
	OPENFILENAME ofn = {};
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)window->getNativeWindow();
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filepath;
	ofn.nMaxFile = sizeof(filepath);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = defExt;
	if (GetSaveFileNameA(&ofn))
		return filepath;
	return std::string();
}

std::string GuiUtils::latinToUtf8(const char* text)
{
	// latin -> UTF-16
	int widesize = MultiByteToWideChar(1252, 0, text, -1, NULL, 0);
	wchar_t* widename = (wchar_t*)alloca(2 * widesize);
	MultiByteToWideChar(1252, 0, text, -1, widename, widesize);

	// UTF-16 -> UTF-8
	int u8size = WideCharToMultiByte(CP_UTF8, 0, widename, widesize, NULL, 0, NULL, NULL);
	char* u8name = (char*)alloca(u8size);
	WideCharToMultiByte(CP_UTF8, 0, widename, widesize, u8name, u8size, NULL, NULL);

	return std::string(u8name);
}

std::string GuiUtils::wcharToUtf8(const wchar_t* text)
{
	// UTF-16 -> UTF-8
	int u8size = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
	char* u8name = (char*)alloca(u8size);
	WideCharToMultiByte(CP_UTF8, 0, text, -1, u8name, u8size, NULL, NULL);
	return std::string(u8name);
}

std::wstring GuiUtils::utf8ToWchar(const char* text)
{
	int widesize = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
	wchar_t* widename = (wchar_t*)alloca(2 * widesize);
	MultiByteToWideChar(CP_UTF8, 0, text, -1, widename, widesize);
	return std::wstring(widename);
}
