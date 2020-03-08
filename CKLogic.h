#pragma once

#include "KObject.h"
#include <vector>
#include <array>
#include "vecmat.h"

struct CSGSectorRoot;

struct CKLogic : CKCategory<12> {};

struct CKSector : CKSubclass<CKLogic, 4> {
	objref<CKObject> sgRoot;
	uint16_t strId, unk1;
	//uint32_t numSas;
	std::vector<objref<CKObject>> sases;
	objref<CKObject> soundDictionary;
	objref<CKObject> beaconKluster;
	objref<CKObject> meshKluster;
	std::array<float, 6> boundaries;
	uint32_t unk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CGround : CKSubclass<CKLogic, 18> {
	struct Triangle {
		std::array<uint16_t, 3> indices;
	};
	struct InfiniteWall {
		std::array<uint16_t, 2> baseIndices;
	};
	struct FiniteWall {
		std::array<uint16_t, 2> baseIndices;
		std::array<float, 2> heights;
	};

	uint32_t numa;
	std::vector<Triangle> triangles;
	std::vector<Vector3> vertices;
	std::array<float, 6> aabb;
	uint16_t param1, param2;
	// ... new stuff from XXL2/OG
	std::vector<InfiniteWall> infiniteWalls;
	std::vector<FiniteWall> finiteWalls;
	float param3, param4;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKMeshKluster : CKSubclass<CKLogic, 66> {
	std::array<float, 6> aabb;
	uint16_t numGrounds, unk1, unk2;
	std::vector<objref<CGround>> grounds;
	objref<CKObject> unkRef;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};