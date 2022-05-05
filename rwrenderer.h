#pragma once

#include <map>
#include <memory>
#include <unordered_map>
#include "renderer.h"

struct Renderer;
struct CTextureDictionary;
struct RwGeometry;
struct CKAnyGeometry;

struct ProTexDict {
	Renderer *_gfx;
	ProTexDict *_next = nullptr;
	std::unordered_map<std::string, texture_t> dict;
	static uint32_t globalVersion;

	ProTexDict(Renderer *gfx) : _gfx(gfx) {}
	ProTexDict(Renderer *gfx, CTextureDictionary *ctd);
	ProTexDict(ProTexDict &&ptd) = default;
	~ProTexDict();

	std::pair<bool, texture_t> find(const std::string &name);
	void reset(CTextureDictionary *ctd);
};

struct ProGeometry {
	Renderer *_gfx;
	ProTexDict *_proTexDict;
	std::unique_ptr<RVertexBuffer> vbuf;
	std::unique_ptr<RIndexBuffer> ibuf;
	size_t numTris;
	std::string textureName;

	texture_t ptdTexId = nullptr;
	uint32_t ptdVersion = 0;

	//ProGeometry(Renderer *gfx, RVertexBuffer *vbuf, RIndexBuffer *ibuf, size_t numTris, const std::string &textureName, ProTexDict *proTexDict) :
	//	gfx(gfx), vbuf(vbuf), ibuf(ibuf), numTris(numTris), textureName(textureName), proTexDict(proTexDict);

	ProGeometry(Renderer *gfx, RwGeometry *geo, ProTexDict *proTexDict);

	void draw(bool showTextures = true);
};

struct ProGeoCache {
	Renderer *_gfx;
	std::unordered_map<RwGeometry*, std::unique_ptr<ProGeometry>> dict;

	ProGeoCache(Renderer *gfx);

	ProGeometry* getPro(RwGeometry* geo, ProTexDict* proTexDict);
	ProGeometry* getPro(const std::shared_ptr<RwGeometry>& geo, ProTexDict* proTexDict) { return getPro(geo.get(), proTexDict); }
	void clear() { dict.clear(); }
};