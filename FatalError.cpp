#include "FatalError.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdarg>

void FatalError(const wchar_t* message)
{
	MessageBoxW(NULL, message, L"XXL Editor Failure", 16);
#ifndef XEC_RELEASE
	DebugBreak();
#endif
	exit(-3);
}

void FatalError(const std::wstring& message)
{
	FatalError(message.c_str());
}

void FatalErrorCFormat(const wchar_t* message, ...)
{
	va_list args;
	va_start(args, message);
	wchar_t buffer[1024];
	vswprintf_s(buffer, message, args);
	FatalError(buffer);
	va_end(args);
}
