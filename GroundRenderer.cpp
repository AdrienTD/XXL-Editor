#include "GroundRenderer.h"
#include "renderer.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKNode.h"

std::optional<GroundGeo> GroundGeo::generateGroundGeo(CGround* gnd, bool hasInfinites)
{
	static constexpr Vector3 lightDir = { 1.0f, 1.5f, 2.0f };

	std::optional<Matrix> transform;
	if (auto* dynGround = gnd->dyncast<CDynamicGround>()) {
		transform = dynGround->getTransform();
	}

	GroundGeo ggeo;
	uint16_t startIndex = 0;
	for (auto& tri : gnd->triangles) {
		std::array<Vector3, 3> tv;
		for (int i = 0; i < 3; i++)
			tv[i] = gnd->vertices[tri.indices[2 - i]];
		if (transform) {
			for (auto& vec : tv)
				vec = vec.transform(*transform);
		}
		Vector3 crs = (tv[2] - tv[0]).cross(tv[1] - tv[0]).normal();
		float dp = crs.dot(lightDir.normal()) + 1.0f;
		uint8_t ll = (uint8_t)(dp * 255.0f);
		uint32_t clr = 0xFF000000 + ll * 0x010101;

		ggeo.positions.insert(ggeo.positions.end(), tv.begin(), tv.end());
		ggeo.colors.insert(ggeo.colors.end(), 3, clr);
		ggeo.triangles.push_back({ startIndex, (uint16_t)(startIndex + 1), (uint16_t)(startIndex + 2) });
		startIndex += 3;
	}
	auto onWall = [&](const std::array<uint16_t, 2>& baseIndices, const std::array<float, 2>& heights) {
		bool flip = heights[0] < 0.0f && heights[1] < 0.0f;

		std::array<Vector3, 4> tv;
		for (int i = 0; i < 2; i++) {
			tv[i] = gnd->vertices[baseIndices[i]];
			tv[2 + i] = gnd->vertices[baseIndices[i]];
		}
		if (transform) {
			for (auto& vec : tv)
				vec = vec.transform(*transform);
		}
		for (int i = 0; i < 2; i++)
			tv[2 + i] += Vector3{ 0.0f, heights[i], 0.0f };
		Vector3 crs = (tv[2] - tv[0]).cross(tv[1] - tv[0]).normal();
		float dp = crs.dot(lightDir.normal()) + 1.0f;
		uint8_t ll = (uint8_t)(dp * 255.0f);
		uint32_t clr = 0xFFFF0000 + ll * 0x000100;

		ggeo.positions.insert(ggeo.positions.end(), tv.begin(), tv.end());
		ggeo.colors.insert(ggeo.colors.end(), 4, clr);

		std::array<uint16_t, 3> tri = { startIndex, (uint16_t)(startIndex + 1), (uint16_t)(startIndex + 2) };
		if (flip) std::swap(tri[1], tri[2]);
		ggeo.triangles.push_back(std::move(tri));

		tri = { (uint16_t)(startIndex + 2), (uint16_t)(startIndex + 1), (uint16_t)(startIndex + 3) };
		if (flip) std::swap(tri[1], tri[2]);
		ggeo.triangles.push_back(std::move(tri));

		startIndex += 4;
	};
	for (auto& wall : gnd->finiteWalls) {
		onWall(wall.baseIndices, wall.heights);
	}
	ggeo.numTrisWithoutInfiniteWalls = ggeo.triangles.size();
	if (hasInfinites) {
		for (auto& wall : gnd->infiniteWalls) {
			static const std::array<float, 2> infiniteHeights = { 100.0f , 100.0f };
			onWall(wall.baseIndices, infiniteHeights);
		}
	}
	ggeo.numTrisWithInfiniteWalls = ggeo.triangles.size();

	if (ggeo.triangles.empty() || ggeo.positions.empty())
		return {};

	return std::move(ggeo);
}

GroundModel::GroundModel(Renderer * gfx, CGround * gnd)
{
	_gfx = gfx;

	auto ggeo = GroundGeo::generateGroundGeo(gnd, true);

	vertices = gfx->createVertexBuffer((int)ggeo->positions.size());
	RVertex *rv = vertices->lock();
	for (size_t i = 0; i < ggeo->positions.size(); ++i) {
		rv[i].x = ggeo->positions[i].x;
		rv[i].y = ggeo->positions[i].y;
		rv[i].z = ggeo->positions[i].z;
		rv[i].color = ggeo->colors[i];
		rv[i].u = 0.0f;
		rv[i].v = 0.0f;
	}
	vertices->unlock();
	numGroundTriangles = gnd->triangles.size();
	numFinWallTris = gnd->finiteWalls.size() * 2;
	numInfWallTris = gnd->infiniteWalls.size() * 2;
	assert(ggeo->triangles.size() == numGroundTriangles + numFinWallTris + numInfWallTris);
	groundIndices = gfx->createIndexBuffer((numGroundTriangles + numFinWallTris + numInfWallTris) * 3);
	uint16_t *gi = groundIndices->lock();
	for (size_t i = 0; i < ggeo->triangles.size(); ++i) {
		*gi++ = ggeo->triangles[i][0];
		*gi++ = ggeo->triangles[i][1];
		*gi++ = ggeo->triangles[i][2];
	}
	groundIndices->unlock();

	if (auto* dynGround = gnd->dyncast<CDynamicGround>()) {
		_usedTransform = dynGround->getTransform();
	}
	else if (gnd->editing) {
		_usedTransform = gnd->editing->transform;
	}
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
		return _cache.emplace(std::make_pair(gnd, std::make_unique<GroundModel>(_gfx, gnd))).first->second.get();
	}
	else {
		if (auto* dynGround = gnd->dyncast<CDynamicGround>()) {
			if (dynGround->getTransform() != it->second->usedTransform()) {
				it->second = std::make_unique<GroundModel>(_gfx, gnd);
			}
		}
		else if (gnd->editing) {
			if (gnd->editing->transform != it->second->usedTransform()) {
				it->second = std::make_unique<GroundModel>(_gfx, gnd);
			}
		}
		return it->second.get();
	}
}
