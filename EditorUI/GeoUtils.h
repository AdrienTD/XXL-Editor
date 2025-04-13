#pragma once

#include <filesystem>
#include <memory>

struct RwClump;
struct RwGeometry;
struct RwExtHAnim;
struct KEnvironment;
struct CNode;

namespace GeoUtils
{
	std::unique_ptr<RwClump> LoadDFF(const std::filesystem::path& filename);
	RwClump CreateClumpFromGeo(std::shared_ptr<RwGeometry> rwgeo, RwExtHAnim* hanim = nullptr);
	RwGeometry createEmptyGeo();
	void ChangeNodeGeometry(KEnvironment& kenv, CNode* geonode, RwGeometry** rwgeos, size_t numRwgeos); // TODO use span
}