#pragma once

#include <map>
#include <memory>
#include <optional>
#include <vector>
#include "vecmat.h"

struct Renderer;
struct RVertexBuffer;
struct RIndexBuffer;
struct CGround;

struct GroundGeo {
	std::vector<Vector3> positions;
	std::vector<std::array<uint16_t, 3>> triangles;
	std::vector<uint32_t> colors;
	size_t numTrisWithoutInfiniteWalls = 0, numTrisWithInfiniteWalls = 0;

	static std::optional<GroundGeo> generateGroundGeo(CGround* gnd, bool hasInfinites);
};

class GroundModel {
private:
	Renderer *_gfx;
	RVertexBuffer *vertices;
	RIndexBuffer *groundIndices;
	//RIndexBuffer *wallIndices;
	size_t numGroundTriangles, numFinWallTris, numInfWallTris;
	std::optional<Matrix> _dynamicGroundTransform;

public:
	GroundModel(Renderer *gfx, CGround *gnd);
	~GroundModel();
	void draw(bool showInfiniteWalls = false);
	const std::optional<Matrix>& dynamicGroundTransform() const { return _dynamicGroundTransform; }
};

class GroundModelCache {
private:
	Renderer *_gfx;
	std::map<CGround*, std::unique_ptr<GroundModel>> _cache;

public:
	GroundModelCache(Renderer *gfx) : _gfx(gfx) {}
	GroundModel *getModel(CGround *gnd);
	void clear() { _cache.clear(); }
};