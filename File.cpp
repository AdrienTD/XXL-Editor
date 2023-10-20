#include "File.h"
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "zlib.h"
#include "KEnvironment.h"
#include "FatalError.h"

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
	if (file) close();
	fopen_s(&file, name, mode);
	if (!file) FatalErrorCFormat(L"Failed to open the file %S", name);
}

void IOFile::open(const wchar_t* name, const char* mode)
{
	if (file) close();
	_wfopen_s(&file, name, std::wstring(mode, mode+strlen(mode)).c_str());
	if (!file) FatalErrorCFormat(L"Failed to open the file %s", name);
}

void MemFile::read(void * out, size_t length)
{
	memcpy(out, _curptr, length);
	_curptr += length;
}

void MemFile::write(const void * out, size_t length)
{
	memcpy(_curptr, out, length);
	_curptr += length;
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
	auto [ptr, size] = GetResourceContent(resName);
	return new MemFile(ptr);
}

std::pair<void*, size_t> GetResourceContent(const char* resName)
{
	static bool firstTime = true;
	static bool useExternalDir = false;
	static const wchar_t* extDirName = L"xec_resources";
	if (firstTime) {
		DWORD attr = GetFileAttributesW(extDirName);
		if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
			useExternalDir = true;
		}
		firstTime = false;
	}
	// First check for file presence in customized resources directory
	if (useExternalDir) {
		wchar_t path[MAX_PATH];
		swprintf_s(path, L"%s\\%S", extDirName, resName);
		FILE* file = nullptr;
		_wfopen_s(&file, path, L"rb");
		if (file) {
			fseek(file, 0, SEEK_END);
			size_t len = ftell(file);
			fseek(file, 0, SEEK_SET);

			static void* buffer = nullptr;
			if (buffer) free(buffer);
			if (buffer = malloc(len)) {
				fread(buffer, len, 1, file);
				fclose(file);
				return { buffer, len };
			}

			fclose(file);
		}
	}
	// Else check in the executable's resources
	HMODULE hmod = GetModuleHandleA(NULL);
	HRSRC rs = FindResourceA(hmod, resName, "DATA");
	if (!rs) return { nullptr, 0 };
	HGLOBAL gl = LoadResource(hmod, rs);
	if (!gl) return { nullptr, 0 };
	return { LockResource(gl), SizeofResource(hmod, rs) };
}

std::unique_ptr<File> GetGzipFile(const wchar_t* path, const char* mode, KEnvironment& kenv)
{
	struct GzipFile : File {
		gzFile _gzf;
		GzipFile(const wchar_t* path, const char* mode) {
			_gzf = gzopen_w(path, mode);
			if (!_gzf) FatalErrorCFormat(L"Failed to open the gzip-compressed file %s", path);
		}
		~GzipFile() {
			gzclose(_gzf);
		}
		virtual void read(void* out, size_t length) override { gzread(_gzf, out, length); }
		virtual void write(const void* out, size_t length) override { gzwrite(_gzf, out, length); }
		virtual void seek(size_t pos, int mode) override { gzseek(_gzf, pos, mode); }
		virtual size_t tell() override { return gztell(_gzf); }
	};
	if (kenv.version >= KEnvironment::KVERSION_ALICE) {
		return std::make_unique<GzipFile>(path, mode);
	}
	else {
		return std::make_unique<IOFile>(path, mode);
	}
}
