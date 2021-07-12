#pragma once

#include <string>
#include <vector>

struct Window;
struct ImGuiInputTextCallbackData;

namespace GuiUtils {
	// ImGui InputCallback for std::string
	int IGStdStringInputCallback(ImGuiInputTextCallbackData* data);

	std::string OpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::vector<std::string> MultiOpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::string SaveDialogBox(Window* window, const char* filter, const char* defExt, const char* defName = nullptr);
	std::string SelectFolderDialogBox(Window* window, const char* text = nullptr);
	std::string latinToUtf8(const char* text);
	std::string wcharToUtf8(const wchar_t* text);
	std::wstring utf8ToWchar(const char* text);
}