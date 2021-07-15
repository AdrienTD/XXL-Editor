#pragma once

#include <string>
#include <vector>
#include <filesystem>

struct Window;
struct ImGuiInputTextCallbackData;

namespace GuiUtils {
	// ImGui InputCallback for std::string
	int IGStdStringInputCallback(ImGuiInputTextCallbackData* data);

	std::filesystem::path OpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::vector<std::filesystem::path> MultiOpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::filesystem::path SaveDialogBox(Window* window, const char* filter, const char* defExt, const std::filesystem::path& defName = {});
	std::filesystem::path SelectFolderDialogBox(Window* window, const char* text = nullptr);
	std::string latinToUtf8(const char* text);
	std::string wcharToUtf8(const wchar_t* text);
	std::wstring utf8ToWchar(const char* text);
	errno_t fsfopen_s(FILE** streamptr, const std::filesystem::path& filename, const char* mode);
}