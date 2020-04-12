#pragma once

#include "KObject.h"
#include <memory>
#include <array>
#include <vector>

struct RwMiniClump;

struct CKAnyGeometry : CKCategory<10> {
	kobjref<CKAnyGeometry> nextGeo;
	uint32_t flags;
	RwMiniClump *clump = nullptr;
	std::vector<RwMiniClump*> costumes;
	kobjref<CKAnyGeometry> sameGeo;
	uint32_t flags2;
	std::array<uint32_t, 7> unkarea;

	~CKAnyGeometry();
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKParticleGeometry : CKSubclass<CKAnyGeometry, 1> {
	void *extra = nullptr; size_t extraSize = 0;
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGeometry : CKSubclass<CKAnyGeometry, 2> {};
struct CKSkinGeometry : CKSubclass<CKAnyGeometry, 3> {};