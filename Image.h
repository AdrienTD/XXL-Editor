#pragma once

#include "DynArray.h"
#include <cstdint>
#include <cstdio>

struct Image {
	uint32_t width, height, bpp, pitch;
	DynArray<uint8_t> pixels;
	DynArray<uint32_t> palette;

	enum Format {
		// bpp <= 8 -> 8-bit palettized with 2^bpp palette colors
		ImageFormat_P8 = 8,
		ImageFormat_RGBA8888 = 32,
		ImageFormat_DXT1 = 0x101,
		ImageFormat_DXT2 = 0x102,
		ImageFormat_DXT4 = 0x103,
	};

	Image convertToRGBA32() const;
	static Image loadFromFile(const char* filename);
	static Image loadFromFile(const wchar_t* filename);
	static Image loadFromFile(FILE* file);
	static Image loadFromMemory(void* ptr, size_t len);
};
