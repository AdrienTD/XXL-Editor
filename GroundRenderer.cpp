#include "GroundRenderer.h"
#include "renderer.h"
#include "CKLogic.h"

GroundModel::GroundModel(Renderer * gfx, CGround * gnd)
{
	_gfx = gfx;

	std::vector<Vector3> norms(gnd->vertices.size(), Vector3(0,0,0));
	std::vector<int> tripervtx(gnd->vertices.size(), 0);
	for (auto &tri : gnd->triangles) {
		auto &gv = gnd->vertices;
		auto &ti = tri.indices;
		Vector3 norm = (gv[ti[1]] - gv[ti[0]]).cross(gv[ti[2]] - gv[ti[0]]).normal();
		for (uint16_t ix : ti) {
			norms[ix] += norm;
			tripervtx[ix]++;
		}
	}
	for (size_t i = 0; i < gnd->vertices.size(); i++)
		norms[i] /= tripervtx[i];

	vertices = gfx->createVertexBuffer(gnd->vertices.size());
	RVertex *rv = vertices->lock();
	for (size_t i = 0; i < gnd->vertices.size(); i++) {
		Vector3 &gv = gnd->vertices[i];
		rv[i].x = gv.x; rv[i].y = gv.y; rv[i].z = gv.z;
		float f = norms[i].dot(Vector3(1, 1, 1).normal());
		uint8_t n = (f > 0) ? (f * 255) : 0;
		uint32_t c = 0x00000000;
		if (gnd->param2 & 8) c |= 0xFF;
		rv[i].color = (0xFF000000 | (n * 0x010101)) & ~c;
		rv[i].u = rv[i].v = 0.0f;
	}
	vertices->unlock();
	numGroundTriangles = gnd->triangles.size();
	groundIndices = gfx->createIndexBuffer(gnd->triangles.size() * 3);
	uint16_t *gi = groundIndices->lock();
	for (size_t i = 0; i < gnd->triangles.size(); i++)
		for (int j : {0, 2, 1})
			*(gi++) = gnd->triangles[i].indices[j];
	groundIndices->unlock();
}

GroundModel::~GroundModel()
{
	delete vertices;
	delete groundIndices;
}

void GroundModel::draw()
{
	_gfx->setVertexBuffer(vertices);
	_gfx->setIndexBuffer(groundIndices);
	_gfx->drawBuffer(0, 3*numGroundTriangles);
}

GroundModel * GroundModelCache::getModel(CGround * gnd)
{
	auto it = _cache.find(gnd);
	if (it == _cache.end()) {
		return _cache.emplace(std::make_pair(gnd, new GroundModel(_gfx, gnd))).first->second.get();
	}
	else
		return it->second.get();
}
