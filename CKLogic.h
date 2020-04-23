#pragma once

#include "KObject.h"
#include <vector>
#include <array>
#include "vecmat.h"

struct CKHook;
struct CKHookLife;
struct CKGroup;
struct CKGroupLife;
struct CKSceneNode;
struct CSGSectorRoot;

struct CKLogic : CKCategory<12> {};

struct CKBundle : CKSubclass<CKLogic, 3> {
	kobjref<CKBundle> next;
	uint8_t flags;
	kobjref<CKGroupLife> grpLife;
	kobjref<CKHookLife> firstHookLife, otherHookLife;
};

struct CKSector : CKSubclass<CKLogic, 4> {
	kobjref<CKObject> sgRoot;
	uint16_t strId, unk1;
	//uint32_t numSas;
	std::vector<kobjref<CKObject>> sases;
	kobjref<CKObject> soundDictionary;
	kobjref<CKObject> beaconKluster;
	kobjref<CKObject> meshKluster;
	std::array<float, 6> boundaries;
	uint32_t unk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSas : CKSubclass<CKLogic, 17> {
	struct SAABB { Vector3 highCorner, lowCorner; };
	std::array<uint32_t, 2> sector;
	std::array<SAABB, 2> boxes;

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

struct CDynamicGround : CKSubclass<CGround, 19> {
	Vector3 mpos, mrot;
	kobjref<CKSceneNode> node; uint32_t nodeId;
	Matrix transform;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment *kenv) override;
};

struct CKMeshKluster : CKSubclass<CKLogic, 66> {
	std::array<float, 6> aabb;
	uint16_t numGrounds, unk1, unk2;
	std::vector<kobjref<CGround>> grounds;
	std::vector<kobjref<CKObject>> walls;
	kobjref<CKObject> unkRef;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBeaconKluster : CKSubclass<CKLogic, 73> {
	struct Beacon {
		int16_t posx, posy, posz; uint16_t params;
		Vector3 getPosition() { return Vector3(posx, posy, posz) * 0.1f; }
		void setPosition(const Vector3 &ts) { posx = ts.x * 10; posy = ts.y * 10; posz = ts.z * 10; }
	};
	struct Bing {
		bool active;
		uint32_t numBeacons;
		uint16_t unk2, unk3, unk4, unk5;
		kobjref<CKObject> handler;
		uint32_t unk6;
		std::vector<Beacon> beacons;
	};
	kobjref<CKBeaconKluster> nextKluster;
	std::array<float, 5> bounds;
	uint16_t numUsedBings;
	std::vector<Bing> bings;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};