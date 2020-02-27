#include "File.h"
#include <cassert>

void IOFile::read(void * out, size_t length) {
	fread(out, length, 1, file);
}

void IOFile::write(void * out, size_t length) {
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
