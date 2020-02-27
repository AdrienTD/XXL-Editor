#pragma once

#include <vector>
#include <array>
#include "vecmat.h"
#include "File.h"

uint32_t rwCheckHeader(File *file, uint32_t type);

struct RwFrame {
	float matrix[3][4];
	uint32_t index, flags;

	void deserialize(File *file);
};

struct RwFrameList {
	std::vector<RwFrame> frames;
	void deserialize(File *file);
};

struct RwGeometry {
	struct Triangle {
		std::array<uint16_t, 3> indices;
		uint16_t materialId;
	};
	uint32_t flags;
	uint32_t numTris, numVerts, numMorphs;
	std::vector<uint32_t> colors;
	std::vector<std::pair<float, float>> texCoords;
	std::vector<Triangle> tris;

	Vector3 spherePos; float sphereRadius;
	uint32_t hasVertices, hasNormals;
	std::vector<Vector3> verts;
	std::vector<Vector3> norms;

	uint32_t numMaterials;
	std::vector<uint32_t> matIndices;

	void deserialize(File *file);
};

struct RwTexture {
	uint8_t filtering, uAddr, vAddr;
	bool usesMips;
	std::string name, alphaName;

	void deserialize(File *file);
};

struct RwMaterial {
	uint32_t flags;
	uint32_t color;
	uint32_t unused, isTextured;
	float ambient, specular, diffuse;
	std::unique_ptr<RwTexture> texture;

	void deserialize(File *file);
};