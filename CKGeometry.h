#pragma once

#include "KObject.h"
#include <memory>
#include <array>
#include <vector>
#include "vecmat.h"
#include <string>
#include "IKRenderable.h"

struct RwMiniClump;
struct CMaterial;

struct CKAnyGeometry : CKSubclass<IKRenderable, 0, 10> {
	// Common :
	kobjref<CKAnyGeometry> nextGeo;
	//uint32_t flags;
	std::shared_ptr<RwMiniClump> clump;

	// XXL1 :
	std::vector<std::shared_ptr<RwMiniClump>> costumes;
	//kobjref<CKAnyGeometry> sameGeo = this;
	uint32_t flags2 = 6;
	std::array<uint32_t, 2> unkarea; std::string unkstring;
	uint32_t unkloner;

	// XXL2+ :
	//kobjref<CKObject> unkobj1; uint32_t spUnk1 = 0;
	//kobjref<CKObject> lightSet;
	kobjref<CMaterial> material;
	uint32_t color = 0xFFFFFFFF;
	kobjref<CKAnyGeometry> duplicateGeo;

	// XXL1/2 Remaster:
	std::string hdKifPath, hdMatName;
	int32_t hdUnk1 = -1;

	// Arthur/OG+ :
	kobjref<CKObject> ogUnkObj;
	uint8_t ogLastByte;

	// Spyro+:
	uint8_t spLastByte2, spLastByte3, spLastByte4;

	// Alice+:
	std::array<uint8_t, 5> alBytes = { 0,0,0,0,0 };

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKParticleGeometry : CKSubclass<CKAnyGeometry, 1> {
	// XXL1 :
	uint32_t pgHead1, pgHead2, pgHead3;
	std::array<float, 4> pgSphere;
	std::vector<Vector3> pgPoints;

	// Common :
	void *extra = nullptr; size_t extraSize = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGeometry : CKSubclass<CKAnyGeometry, 2> {};
struct CKSkinGeometry : CKSubclass<CKAnyGeometry, 3> {};