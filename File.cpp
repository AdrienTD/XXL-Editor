#include "File.h"
#include <cassert>
#include <Windows.h>

void IOFile::read(void * out, size_t length) {
	fread(out, length, 1, file);
}

void IOFile::write(const void * out, size_t length) {
	fwrite(out, length, 1, file);
}

void IOFile::seek(size_t pos, int mode) {
	fseek(file, pos, mode);
}

size_t IOFile::tell() {
	return ftell(file);
}

void IOFile::close() {
	if (file) {
		fclose(file);
		file = nullptr;
	}
}

void IOFile::open(const char * name, const char * mode) {
	if (file)
		close();
	assert(!fopen_s(&file, name, mode));
}

void MemFile::read(void * out, size_t length)
{
	memcpy(out, _curptr, length);
	_curptr += length;
}

void MemFile::write(const void * out, size_t length)
{
}

void MemFile::seek(size_t pos, int mode)
{
	if (mode == SEEK_SET)
		_curptr = _startptr + pos;
	else if (mode == SEEK_CUR)
		_curptr += pos;
}

size_t MemFile::tell()
{
	return _curptr - _startptr;
}

File * GetResourceFile(const char * resName)
{
	HMODULE hmod = GetModuleHandleA(NULL);
	HRSRC rs = FindResourceA(hmod, resName, "DATA");
	HGLOBAL gl = LoadResource(hmod, rs);
	return new MemFile(LockResource(gl));
}
