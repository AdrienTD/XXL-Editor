#pragma once

#include <vector>
#include <array>
#include <memory>
#include "vecmat.h"
#include "File.h"

struct RwsHeader {
	uint32_t type, length, rwVersion;
	RwsHeader() {}
	RwsHeader(uint32_t type, uint32_t length, uint32_t rwVersion) : type(type), length(length), rwVersion(rwVersion) {}
};

struct HeaderWriter {
	uint32_t headpos;
	void begin(File *file, int type) {
		headpos = file->tell();
		file->writeUint32(type);
		file->writeUint32(0);
		file->writeUint32(0x1803FFFF);
	}
	void end(File *file) {
		uint32_t endpos = file->tell();
		file->seek(headpos+4, SEEK_SET);
		file->writeUint32(endpos - headpos - 12);
		file->seek(endpos, SEEK_SET);
	}
};

RwsHeader rwReadHeader(File *file);
RwsHeader rwFindHeader(File *file, uint32_t type);
uint32_t rwCheckHeader(File *file, uint32_t type);
void rwCheckAndSkipHeader(File *file, uint32_t type);
void rwWriteString(File *file, const std::string &str);

struct RwExtension;

struct RwsExtHolder {
	//void *_ptr = nullptr;
	//uint32_t _length = 0;
	std::vector<RwExtension*> exts;
	void read(File *file);
	void write(File *file);
	//~RwsExtHolder();
	//~RwsExtHolder() { if (_ptr) free(_ptr); }
	RwsExtHolder() {}
	RwsExtHolder(const RwsExtHolder &orig);
	RwsExtHolder(RwsExtHolder &&old) { exts = std::move(old.exts); }
	void operator=(const RwsExtHolder &orig);
	void operator=(RwsExtHolder &&old) { exts = std::move(old.exts); }
};

struct RwFrame {
	float matrix[4][3];
	uint32_t index, flags;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwFrameList {
	std::vector<RwFrame> frames;
	std::vector<RwsExtHolder> extensions;
	void deserialize(File *file);
	void serialize(File *file);
};

struct RwTexture {
	uint8_t filtering, uAddr, vAddr;
	bool usesMips;
	std::string name, alphaName;
	RwsExtHolder extensions;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwMaterial {
	uint32_t flags;
	uint32_t color;
	uint32_t unused, isTextured;
	float ambient, specular, diffuse;
	RwTexture texture;
	RwsExtHolder extensions;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwMaterialList {
	std::vector<uint32_t> slots;
	std::vector<RwMaterial> materials;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwGeometry {
	struct Triangle {
		std::array<uint16_t, 3> indices;
		uint16_t materialId;
	};
	uint32_t flags;
	uint32_t numTris, numVerts, numMorphs;
	std::vector<uint32_t> colors;
	std::vector<std::vector<std::array<float, 2>>> texSets;
	std::vector<Triangle> tris;

	Vector3 spherePos; float sphereRadius;
	uint32_t hasVertices, hasNormals;
	std::vector<Vector3> verts;
	std::vector<Vector3> norms;

	//uint32_t numMaterials;
	//std::vector<uint32_t> matIndices;
	RwMaterialList materialList;
	RwsExtHolder extensions;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwAtomic {
	uint32_t frameIndex, geoIndex, flags, unused;
	std::unique_ptr<RwGeometry> geometry;
	RwsExtHolder extensions;

	void deserialize(File *file, bool hasGeo = true);
	void serialize(File *file);
};

struct RwMiniClump {
	RwFrameList frameList;
	RwAtomic atomic;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwGeometryList {
	std::vector<RwGeometry*> geometries;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwClump {
	RwFrameList frameList;
	RwGeometryList geoList;
	std::vector<RwAtomic*> atomics;
	RwsExtHolder extensions;

	void deserialize(File *file);
	void serialize(File *file);
};

template<int N> struct FixedBuffer {
	uint8_t buf[N];
	void deserialize(File *file) { file->read(buf, N); }
	void serialize(File *file) { file->write(buf, N); }
};

struct RwTeamDictionary {
	struct Bing {
		uint32_t _someNum;
		std::unique_ptr<RwMiniClump> _clump;
	};
	uint32_t _numDings, _unk1;
	std::vector<uint32_t> _dings;
	std::vector<Bing> _bings;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwTeam {
	struct Dong {
		FixedBuffer<16> head3;
		FixedBuffer<12> head4;
		std::vector<uint32_t> bongs;
		RwClump clump;
	};

	uint32_t numBongs, numDongs;
	FixedBuffer<8> head2;
	std::vector<Dong> dongs;
	FixedBuffer<20> end;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwImage {
	uint32_t width, height, bpp, pitch;
	void *pixels = nullptr; uint32_t *palette = nullptr;

	void deserialize(File *file);
	void serialize(File *file);

	RwImage() {}
	RwImage(RwImage &&img) {
		width = img.width;
		height = img.height;
		bpp = img.bpp;
		pitch = img.pitch;
		pixels = img.pixels;
		palette = img.palette;
		img.pixels = nullptr;
		img.palette = nullptr;
	}

	~RwImage() { if (pixels) free(pixels); if (palette) free(palette); }
};