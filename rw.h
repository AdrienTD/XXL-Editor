#pragma once

#include <vector>
#include <array>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include "vecmat.h"
#include "DynArray.h"

struct File;

struct RwsHeader {
	uint32_t type, length, rwVersion;
	RwsHeader() {}
	RwsHeader(uint32_t type, uint32_t length, uint32_t rwVersion) : type(type), length(length), rwVersion(rwVersion) {}
};

struct HeaderWriter {
	uint32_t headpos;
	static uint32_t rwver;
	void begin(File *file, int type);
	void end(File *file);
};

RwsHeader rwReadHeader(File *file);
void rwWriteHeader(File *file, uint32_t type, uint32_t length);
RwsHeader rwFindHeader(File *file, uint32_t type);
uint32_t rwCheckHeader(File *file, uint32_t type);
void rwCheckAndSkipHeader(File *file, uint32_t type);
void rwWriteString(File *file, const std::string &str);

struct RwExtension;

struct RwsExtHolder {
	std::vector<std::unique_ptr<RwExtension>> exts;
	void read(File *file, void *parent, uint32_t parentType = 0);
	void write(File *file);
	RwExtension *find(uint32_t type);
	const RwExtension *find(uint32_t type) const;
	~RwsExtHolder();
	RwsExtHolder();
	RwsExtHolder(const RwsExtHolder &orig);
	RwsExtHolder(RwsExtHolder&& old) noexcept;
	RwsExtHolder& operator=(const RwsExtHolder &orig);
	RwsExtHolder& operator=(RwsExtHolder&& old) noexcept;
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
		RWGEOFLAG_NATIVE = 0x01000000,
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

	// used on consoles for 1:1 back serialization
	std::shared_ptr<RwGeometry> nativeVersion;

	void deserialize(File *file);
	void serialize(File *file);

	void merge(const RwGeometry &other);
	std::vector<std::unique_ptr<RwGeometry>> splitByMaterial();
	RwGeometry convertToPI();
	RwGeometry convertToPI_GCN();
	RwGeometry convertToPI_X360();
};

struct RwAtomic {
	uint32_t frameIndex, geoIndex, flags, unused;
	std::shared_ptr<RwGeometry> geometry;
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
	std::vector<std::shared_ptr<RwGeometry>> geometries;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwClump {
	RwFrameList frameList;
	RwGeometryList geoList;
	std::vector<RwAtomic> atomics;
	RwsExtHolder extensions;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwTeamDictionary {
	struct Bing {
		uint32_t _ding = 1;
		uint32_t _someNum;
		RwMiniClump _clump;
	};
	uint32_t _unk1;
	std::vector<Bing> _bings;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwTeam {
	template<int N> using FixedBuffer = std::array<uint8_t, N>;

	struct Dong {
		FixedBuffer<16> head3;
		FixedBuffer<12> head4;
		std::vector<uint32_t> bongs;
		RwClump clump;
	};

	//uint32_t numBongs;
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

	enum Format {
		// bpp <= 8 -> 8-bit palettized with 2^bpp palette colors
		ImageFormat_P8 = 8,
		ImageFormat_RGBA8888 = 32,
		ImageFormat_DXT1 = 0x101,
		ImageFormat_DXT2 = 0x102,
		ImageFormat_DXT4 = 0x103,
	};

	void deserialize(File *file);
	void serialize(File *file);

	RwImage convertToRGBA32() const;
	static RwImage loadFromFile(const char* filename);
	static RwImage loadFromFile(const wchar_t* filename);
	static RwImage loadFromFile(FILE* file);
	static RwImage loadFromMemory(void *ptr, size_t len);
	static std::vector<RwImage> loadDDS(const void* ddsData);
	static int computeDDSSize(const void* ddsData);
};

struct RwRaster;

struct RwPITexDict {
	uint16_t flags = 1;
	struct PITexture {
		//uint32_t numMipmaps = 1;
		std::vector<RwImage> images;
		RwTexture texture;
		std::shared_ptr<const RwRaster> nativeVersion; // used for console 1:1 back serialization
	};
	std::vector<PITexture> textures;

	uint16_t nativeVersionPlatform = 0; // platform of native versions

	void deserialize(File *file);
	void serialize(File *file);
	size_t findTexture(const std::string& name) const;
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

	uint16_t* createGlyphSlot(uint16_t charId);
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

struct RwAnimAnimation {
	struct HAnimKeyFrame {
		float time;
		float quaternion[4];
		Vector3 translation;
		int32_t prevFrame;
	};
	struct CompressedKeyFrame {
		float time;
		int16_t quaternion[4];
		int16_t translation[3];
		int32_t prevFrame;
	};
	int32_t version = 0x100;
	int32_t schemeId = 1;
	//int32_t numFrames = 0;
	uint32_t flags = 0;
	uint32_t extra = 0;
	float duration = 0.0f;
	std::variant<std::vector<HAnimKeyFrame>, std::vector<CompressedKeyFrame>> frames;
	Vector3 compressedTranslationOffset, compressedTranslationScale;

	void deserialize(File* file);
	void serialize(File* file);

	size_t numFrames() const;
	HAnimKeyFrame decompressFrame(int frameIndex) const;
	float frameTime(int frameIndex) const;
	std::span<Matrix> interpolateNodeTransforms(int numNodes, float time) const;
	int guessNumNodes() const;

	static constexpr float decompressFloat(uint16_t compressedValue);
};

struct RwRaster {
	std::vector<uint8_t> data;
	std::vector<uint8_t> furtherSegs; // PS2 rasters contain more than one "type 1" chunk
	//RwsExtHolder extensions;

	void deserialize(File* file, uint32_t len);
	void serialize(File* file);

	RwPITexDict::PITexture convertToPI() const;
	RwPITexDict::PITexture convertToPI_GCN() const;
	RwPITexDict::PITexture convertToPI_X360() const;
	static RwRaster createFromPI(const RwPITexDict::PITexture& pit);
};

struct RwNTTexDict {
	//uint16_t numTextures;
	uint16_t platform;
	std::vector<RwRaster> textures;
	RwsExtHolder extensions;

	void deserialize(File* file);
	void serialize(File* file);

	RwPITexDict convertToPI();
	static RwNTTexDict createFromPI(const RwPITexDict& pi);
};