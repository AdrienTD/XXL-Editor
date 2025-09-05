#include "Image.h"

#include <cassert>
#include <cstring>
#include <squish.h>
#include <stb_image.h>
#include <unordered_map>

Image Image::convertToRGBA32() const
{
	if (bpp == 32)
		return Image(*this);
	Image img;
	img.width = width;
	img.height = height;
	img.bpp = 32;
	img.pitch = width * 4;
	img.pixels.resize(img.pitch * img.height);
	if (bpp <= 8) {
		assert(width == pitch);
		uint8_t* oldpix = (uint8_t*)pixels.data();
		uint32_t* newpix = (uint32_t*)img.pixels.data();
		size_t oldsize = pitch * height;
		for (size_t i = 0; i < oldsize; i++)
			newpix[i] = palette[oldpix[i]];
	}
	else if (bpp == Format::ImageFormat_DXT1) {
		squish::DecompressImage(img.pixels.data(), width, height, pixels.data(), squish::kDxt1);
	}
	else if (bpp == Format::ImageFormat_DXT2) {
		squish::DecompressImage(img.pixels.data(), width, height, pixels.data(), squish::kDxt3);
	}
	else if (bpp == Format::ImageFormat_DXT4) {
		squish::DecompressImage(img.pixels.data(), width, height, pixels.data(), squish::kDxt5);
	}
	else {
		assert(false && "unsupported format for RGBA32 conversion");
	}
	return img;
}

std::optional<Image> Image::palettize() const
{
	// convert image to 32bpp if it was in another format
	std::optional<Image> cvtImage;
	if (this->bpp != Format::ImageFormat_RGBA8888) {
		cvtImage = this->convertToRGBA32();
	}
	const Image& image32 = cvtImage ? *cvtImage : *this;

	int numUniqueColors = 0;
	std::unordered_map<uint32_t, uint8_t> colorIndexMap;

	const size_t numPixels = image32.width * image32.height;
	DynArray<uint8_t> palPixels(numPixels);

	for (size_t i = 0; i < numPixels; ++i) {
		uint32_t color = *(uint32_t*)(image32.pixels.data() + 4 * i);
		auto [iter, inserted] = colorIndexMap.try_emplace(color, (uint8_t)numUniqueColors);
		if (inserted) {
			numUniqueColors += 1;
			if (numUniqueColors > 256)
				return std::nullopt;
		}
		palPixels[i] = iter->second;
	}

	DynArray<uint32_t> palette((numUniqueColors <= 16) ? 16 : 256);
	for (auto [color, index] : colorIndexMap) {
		palette[index] = color;
	}
	std::fill(palette.begin() + numUniqueColors, palette.end(), 0);

	Image image8;
	image8.width = image32.width;
	image8.height = image32.height;
	image8.bpp = (numUniqueColors <= 16) ? 4 : 8;
	image8.pitch = image8.width;
	image8.pixels = std::move(palPixels);
	image8.palette = std::move(palette);
	return image8;
}

Image Image::loadFromFile(const char* filename)
{
	Image img;
	int sizx, sizy, origBpp;
	void* pix = stbi_load(filename, &sizx, &sizy, &origBpp, 4);
	assert(pix && "Failed to load image file\n");
	img.width = sizx;
	img.height = sizy;
	img.bpp = 32;
	img.pitch = img.width * 4;
	img.pixels.resize(img.pitch * img.height);
	memcpy(img.pixels.data(), pix, img.pixels.size());
	stbi_image_free(pix);
	return img;
}

Image Image::loadFromFile(const wchar_t* filename)
{
	FILE* file;
	_wfopen_s(&file, filename, L"rb");
	Image img = loadFromFile(file);
	fclose(file);
	return img;
}

Image Image::loadFromFile(FILE* file)
{
	Image img;
	int sizx, sizy, origBpp;
	void* pix = stbi_load_from_file(file, &sizx, &sizy, &origBpp, 4);
	assert(pix && "Failed to load image file\n");
	img.width = sizx;
	img.height = sizy;
	img.bpp = 32;
	img.pitch = img.width * 4;
	img.pixels.resize(img.pitch * img.height);
	memcpy(img.pixels.data(), pix, img.pixels.size());
	stbi_image_free(pix);
	return img;
}

Image Image::loadFromMemory(void* ptr, size_t len) {
	Image img;
	int sizx, sizy, origBpp;
	void* pix = stbi_load_from_memory((uint8_t*)ptr, (int)len, &sizx, &sizy, &origBpp, 4);
	assert(pix && "Failed to load image from memory\n");
	img.width = sizx;
	img.height = sizy;
	img.bpp = 32;
	img.pitch = img.width * 4;
	img.pixels.resize(img.pitch * img.height);
	memcpy(img.pixels.data(), pix, img.pixels.size());
	stbi_image_free(pix);
	return img;
}
