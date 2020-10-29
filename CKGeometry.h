#pragma once

#include "KObject.h"
#include <memory>
#include <array>
#include <vector>
#include "vecmat.h"

struct RwMiniClump;

struct CKAnyGeometry : CKCategory<10> {
	// Common :
	kobjref<CKAnyGeometry> nextGeo;
	uint32_t flags;
	RwMiniClump *clump = nullptr;

	// XXL1 :
	std::vector<RwMiniClump*> costumes;
	kobjref<CKAnyGeometry> sameGeo;
	uint32_t flags2;
	std::array<uint32_t, 7> unkarea;
	uint32_t unkloner;

	// XXL2+ :
	kobjref<CKObject> unkobj1;
	kobjref<CKObject> lightSet;
	kobjref<CKObject> material;
	uint32_t color = 0xFFFFFFFF;
	kobjref<CKAnyGeometry> duplicateGeo;

	// XXL1/2 Remaster:
	std::string hdKifPath, hdMatName;
	int32_t hdUnk1;

	// Arthur/OG+ :
	kobjref<CKObject> ogUnkObj;
	uint8_t ogLastByte;

	~CKAnyGeometry();
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