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
		norms[i] /= (float)tripervtx[i];

	vertices = gfx->createVertexBuffer(gnd->vertices.size() + 2*gnd->finiteWalls.size() + 2*gnd->infiniteWalls.size());
	RVertex *rv = vertices->lock();
	for (size_t i = 0; i < gnd->vertices.size(); i++) {
		Vector3 &gv = gnd->vertices[i];
		rv[i].x = gv.x; rv[i].y = gv.y; rv[i].z = gv.z;
		float f = norms[i].dot(Vector3(1, 1, 1).normal());
		uint8_t n = (f > 0.0f) ? (uint8_t)(f * 255) : 0;
		uint32_t c = 0x00000000;
		//if (gnd->param2 & 8) c |= 0xFF;
		rv[i].color = (0xFF000000 | (n * 0x010101)) & ~c;
		rv[i].u = rv[i].v = 0.0f;
	}
	int i = gnd->vertices.size();
	for (auto &fw : gnd->finiteWalls) {
		Vector3 norm = (gnd->vertices[fw.baseIndices[0]] - gnd->vertices[fw.baseIndices[1]]).cross(Vector3(0, 1, 0)).normal();
		for (size_t j = 0; j < 2; j++) {
			Vector3 &gv = gnd->vertices[fw.baseIndices[j]];
			rv[i].x = gv.x; rv[i].y = gv.y + fw.heights[j]; rv[i].z = gv.z;
			float f = norm.dot(Vector3(1, 1, 1).normal());
			uint8_t n = (f > 0.0f) ? (uint8_t)(f * 255) : 0;
			uint32_t c = 0x00000000;
			//if (gnd->param2 & 8) c |= 0xFF;
			c |= 0xFF;
			rv[i].color = (0xFFFF0000 | (n * 0x010101)) & ~c;
			rv[i].u = rv[i].v = 0.0f;
			i++;
		}
	}
	for (auto &fw : gnd->infiniteWalls) {
		Vector3 norm = (gnd->vertices[fw.baseIndices[0]] - gnd->vertices[fw.baseIndices[1]]).cross(Vector3(0, 1, 0)).normal();
		for (size_t j = 0; j < 2; j++) {
			Vector3 &gv = gnd->vertices[fw.baseIndices[j]];
			rv[i].x = gv.x; rv[i].y = 1000.0f; rv[i].z = gv.z;
			float f = norm.dot(Vector3(1, 1, 1).normal());
			uint8_t n = (f > 0.0f) ? (uint8_t)(f * 255) : 0;
			uint32_t c = 0x00000000;
			//if (gnd->param2 & 8) c |= 0xFF;
			c |= 0xFF;
			rv[i].color = (0xFFFF0000 | (n * 0x010101)) & ~c;
			rv[i].u = rv[i].v = 0.0f;
			i++;
		}
	}
	vertices->unlock();
	numGroundTriangles = gnd->triangles.size();
	numFinWallTris = gnd->finiteWalls.size() * 2;
	numInfWallTris = gnd->infiniteWalls.size() * 2;
	groundIndices = gfx->createIndexBuffer((numGroundTriangles + numFinWallTris + numInfWallTris) * 3);
	uint16_t *gi = groundIndices->lock();
	for (size_t i = 0; i < gnd->triangles.size(); i++)
		for (int j : {0, 2, 1})
			*(gi++) = gnd->triangles[i].indices[j];
	for (size_t i = 0; i < gnd->finiteWalls.size(); i++) {
		auto &fw = gnd->finiteWalls[i];
		uint16_t topIndex0 = (uint16_t)(gnd->vertices.size() + 2 * i);
		uint16_t topIndex1 = (uint16_t)(gnd->vertices.size() + 2 * i + 1);
		if (fw.heights[0] >= 0.0f || fw.heights[1] >= 0.0f) {
			*(gi++) = fw.baseIndices[0];
			*(gi++) = fw.baseIndices[1];
			*(gi++) = topIndex0;
			*(gi++) = fw.baseIndices[1];
			*(gi++) = topIndex1;
			*(gi++) = topIndex0;
		}
		else {
			*(gi++) = fw.baseIndices[0];
			*(gi++) = topIndex0;
			*(gi++) = fw.baseIndices[1];
			*(gi++) = fw.baseIndices[1];
			*(gi++) = topIndex0;
			*(gi++) = topIndex1;
		}
	}
	for (size_t i = 0; i < gnd->infiniteWalls.size(); i++) {
		auto &fw = gnd->infiniteWalls[i];
		*(gi++) = fw.baseIndices[0];
		*(gi++) = fw.baseIndices[1];
		*(gi++) = (uint16_t)(gnd->vertices.size() + 2 * gnd->finiteWalls.size() + 2 * i);
		*(gi++) = fw.baseIndices[1];
		*(gi++) = (uint16_t)(gnd->vertices.size() + 2 * gnd->finiteWalls.size() + 2 * i + 1);
		*(gi++) = (uint16_t)(gnd->vertices.size() + 2 * gnd->finiteWalls.size() + 2 * i);
	}

	groundIndices->unlock();
}

GroundModel::~GroundModel()
{
	delete vertices;
	delete groundIndices;
}

void GroundModel::draw(bool showInfiniteWalls)
{
	_gfx->setVertexBuffer(vertices);
	_gfx->setIndexBuffer(groundIndices);
	_gfx->drawBuffer(0, 3*(numGroundTriangles + numFinWallTris + (showInfiniteWalls ? numInfWallTris : 0)) );
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
