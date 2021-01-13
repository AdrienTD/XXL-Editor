#pragma once

#include <string>

struct Window;
struct ImGuiInputTextCallbackData;

namespace GuiUtils {
	// ImGui InputCallback for std::string
	int IGStdStringInputCallback(ImGuiInputTextCallbackData* data);

	std::string OpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::string SaveDialogBox(Window* window, const char* filter, const char* defExt, const char* defName = nullptr);
	std::string latinToUtf8(const char* text);
	std::string wcharToUtf8(const wchar_t* text);
	std::wstring utf8ToWchar(const char* text);
}