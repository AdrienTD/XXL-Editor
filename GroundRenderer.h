#pragma once

#include <map>
#include <memory>

struct Renderer;
struct RVertexBuffer;
struct RIndexBuffer;
struct CGround;

struct GroundModel {
	Renderer *_gfx;
	RVertexBuffer *vertices;
	RIndexBuffer *groundIndices;
	//RIndexBuffer *wallIndices;
	size_t numGroundTriangles;

	GroundModel(Renderer *gfx, CGround *gnd);
	~GroundModel();
	void draw();
};

struct GroundModelCache {
	Renderer *_gfx;
	std::map<CGround*, std::unique_ptr<GroundModel>> _cache;

	GroundModelCache(Renderer *gfx) : _gfx(gfx) {}
	GroundModel *getModel(CGround *gnd);
	void clear() { _cache.clear(); }
};