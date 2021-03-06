#pragma once

#include <vector>
#include <array>
#include <memory>
#include "vecmat.h"
#include "File.h"
#include "DynArray.h"

struct RwsHeader {
	uint32_t type, length, rwVersion;
	RwsHeader() {}
	RwsHeader(uint32_t type, uint32_t length, uint32_t rwVersion) : type(type), length(length), rwVersion(rwVersion) {}
};

struct HeaderWriter {
	uint32_t headpos;
	static uint32_t rwver;
	void begin(File *file, int type) {
		headpos = file->tell();
		file->writeUint32(type);
		file->writeUint32(0);
		file->writeUint32(rwver);
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
	std::vector<RwExtension*> exts;
	void read(File *file, void *parent);
	void write(File *file);
	RwExtension *find(uint32_t type);
	const RwExtension *find(uint32_t type) const;
	~RwsExtHolder();
	RwsExtHolder() {}
	RwsExtHolder(const RwsExtHolder &orig);
	RwsExtHolder(RwsExtHolder &&old) noexcept { exts = std::move(old.exts); }
	void operator=(const RwsExtHolder &orig);
	void operator=(RwsExtHolder &&old) noexcept { exts = std::move(old.exts); }
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

	enum {
		RWGEOFLAG_TRISTRIP =	0x01,
		RWGEOFLAG_POSITIONS =	0x02,
		RWGEOFLAG_TEXTURED =	0x04,
		RWGEOFLAG_PRELIT =		0x08,
		RWGEOFLAG_NORMALS =		0x10,
		RWGEOFLAG_LIGHT =		0x20,
		RWGEOFLAG_MODULATECOLOR = 0x40,
		RWGEOFLAG_TEXTURED2 =	0x80,
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

	void merge(const RwGeometry &other);
	std::vector<std::unique_ptr<RwGeometry>> splitByMaterial();
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
	DynArray<uint8_t> pixels;
	DynArray<uint32_t> palette;

	void deserialize(File *file);
	void serialize(File *file);

	RwImage convertToRGBA32() const;
	static RwImage loadFromFile(const char *filename);
};

struct RwPITexDict {
	uint16_t flags = 1;
	struct PITexture {
		uint32_t type = 1;
		RwImage image;
		RwTexture texture;
	};
	std::vector<PITexture> textures;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwFont2D {
	struct Glyph {
		std::array<float, 4> coords;
		float glUnk1;
		uint8_t texIndex;
	};

	uint32_t flags;
	uint32_t fntUnk1;
	float glyphHeight;
	float fntUnk3;
	uint32_t fntUnk4;
	uint32_t fntUnk5;
	uint32_t firstWideChar;
	//uint32_t numGlyphs;
	//uint32_t numWideChars;

	std::vector<uint16_t> wideGlyphTable;
	std::array<uint16_t, 128> charGlyphTable;

	std::vector<Glyph> glyphs;
	std::vector<std::string> texNames;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwBrush2D {
	uint32_t rbUnk1 = 1;
	std::array<float, 24> rbFloats = { 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 255, 0, 1, 0, 0, 0, 0, 1, 0 };
	uint32_t rbUnk2 = 2;
	float rbUnk3 = 0.5f;
	uint32_t rbUnk4 = 0;

	void deserialize(File *file);
	void serialize(File *file);
};