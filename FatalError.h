#pragma once

#include <string>

[[noreturn]] void FatalError(const wchar_t* message);
[[noreturn]] void FatalError(const std::wstring& message);
[[noreturn]] void FatalErrorCFormat(const wchar_t* message, ...);
