#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <string_view>

struct Window;
struct ImGuiInputTextCallbackData;

namespace GuiUtils {
	enum class MsgBoxIcon {
		Empty,
		Information,
		Warning,
		Error
	};
	enum class MsgBoxButton {
		Unknown,
		Yes,
		No
	};

	// ImGui InputCallback for std::string
	int IGStdStringInputCallback(ImGuiInputTextCallbackData* data);

	void MsgBox_Ok(Window* window, const char* message, MsgBoxIcon flags = MsgBoxIcon::Empty);
	MsgBoxButton MsgBox_YesNo(Window* window, const char* message, MsgBoxIcon flags = MsgBoxIcon::Empty);
	std::filesystem::path OpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::vector<std::filesystem::path> MultiOpenDialogBox(Window* window, const char* filter, const char* defExt);
	std::filesystem::path SaveDialogBox(Window* window, const char* filter, const char* defExt, const std::filesystem::path& defName = {});
	std::filesystem::path SelectFolderDialogBox(Window* window, const char* text = nullptr);
	std::string latinToUtf8(std::string_view text);
	std::string utf8ToLatin(std::string_view text);
	std::string wcharToUtf8(std::wstring_view text);
	std::wstring utf8ToWchar(std::string_view text);
	errno_t fsfopen_s(FILE** streamptr, const std::filesystem::path& filename, const char* mode);
}