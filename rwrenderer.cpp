#include "rwrenderer.h"
#include "CoreClasses/CKDictionary.h"

uint32_t ProTexDict::globalVersion = 0;

ProTexDict::ProTexDict(Renderer * gfx, CTextureDictionary * ctd) {
	_gfx = gfx;
	reset(ctd);
}

ProTexDict::~ProTexDict() {
	for (auto &e : dict) {
		_gfx->deleteTexture(e.second);
	}
}

std::pair<bool, texture_t> ProTexDict::find(const std::string & name) {
	auto it = dict.find(name);
	if (it != dict.end())
		return std::make_pair(true, it->second);
	if (_next)
		return _next->find(name);
	return std::make_pair(false, (texture_t)0);
}

void ProTexDict::reset(CTextureDictionary * ctd)
{
	globalVersion++;
	for (auto &e : dict) {
		_gfx->deleteTexture(e.second);
	}
	dict.clear();
	for (auto &tex : ctd->piDict.textures) {
		dict[tex.texture.name.c_str()] = _gfx->createTexture(tex.images[0]);
	}
}

ProGeometry::ProGeometry(Renderer * gfx, RwGeometry * geo, ProTexDict * proTexDict)
{
	_gfx = gfx;
	_proTexDict = proTexDict;
	numTris = geo->tris.size();
	vbuf.reset(gfx->createVertexBuffer(geo->numVerts));
	ibuf.reset(gfx->createIndexBuffer(geo->tris.size() * 3));
	RVertex *verts = vbuf->lock();
	for (size_t i = 0; i < geo->numVerts; i++) {
		verts[i].x = geo->verts[i].x;
		verts[i].y = geo->verts[i].y;
		verts[i].z = geo->verts[i].z;
		if (!geo->colors.empty())
			verts[i].color = geo->colors[i];
		else
			verts[i].color = 0xFFFFFFFF;
		if (!geo->texSets.empty() && !geo->texSets[0].empty()) {
			auto &guv = geo->texSets[0][i];
			verts[i].u = guv[0];
			verts[i].v = guv[1];
		}
		else {
			verts[i].u = 0.0f;
			verts[i].v = 0.0f;
		}
	}
	vbuf->unlock();
	uint16_t *index = ibuf->lock();
	for (auto &tri : geo->tris) {
		for (uint16_t ix : tri.indices)
			*(index++) = ix;
	}
	ibuf->unlock();

	auto &matvec = geo->materialList.materials;
	if (!matvec.empty())
		if (matvec[0].isTextured)
			textureName = matvec[0].texture.name;
}

void ProGeometry::draw(bool showTextures) {
	if (numTris == 0) return;
	_gfx->setVertexBuffer(vbuf.get());
	_gfx->setIndexBuffer(ibuf.get());
	texture_t texid = nullptr;
	if (ptdVersion == ProTexDict::globalVersion)
		texid = ptdTexId;
	else {
		auto mt = _proTexDict->find(textureName);
		if (mt.first)
			texid = mt.second;
		ptdTexId = texid;
		ptdVersion = ProTexDict::globalVersion;
	}
	if (texid && showTextures)
		_gfx->bindTexture(0, texid);
	else
		_gfx->unbindTexture(0);
	_gfx->drawBuffer(0, numTris * 3);
}

ProGeoCache::ProGeoCache(Renderer * gfx)
{
	_gfx = gfx;
}

ProGeometry * ProGeoCache::getPro(RwGeometry *geo, ProTexDict *proTexDict)
{
	auto it = dict.find(geo);
	if (it != dict.end())
		return it->second.get();
	ProGeometry *prog = new ProGeometry(_gfx, geo, proTexDict);
	dict[geo] = std::unique_ptr<ProGeometry>(prog);
	return prog;
}
