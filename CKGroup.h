#pragma once

#include "KObject.h"
#include <array>
#include <vector>
#include "vecmat.h"

struct CKHook;
struct CKGroupLife;
struct CKSceneNode;

struct CKHkBasicBonus;
struct CKGrpPoolSquad;

struct CKGroup : CKCategory<4> {
	kobjref<CKGroup> nextGroup;
	kobjref<CKGroup> parentGroup;
	kobjref<CKGroupLife> life;
	kobjref<CKObject> bundle;
	uint32_t unk2 = 0;
	kobjref<CKGroup> childGroup;
	kobjref<CKHook> childHook;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGroupLife : CKCategory<5> {
	kobjref<CKObject> unk;
	kobjref<CKGroup> group;
	uint32_t unk2 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGroupRoot : CKSubclass<CKGroup, 1> {};

struct CKGrpBaseSquad : CKSubclass<CKGroup, 18> {
	uint32_t bsUnk1;
	kobjref<CKObject> msgAction;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpSquad : CKSubclass<CKGrpBaseSquad, 24> {
	std::array<float, 12> mat1, mat2;
	float sqUnk1;
	Vector3 sqUnk2;
	std::array<kobjref<CKObject>, 4> refs;
	std::array<float, 6> sqUnk3, sqUnk4;
	uint32_t sqUnk5;
	//uint32_t numChoreographies;
	std::vector<kobjref<CKObject>> choreographies;
	//uint32_t numChoreoKeys;
	std::vector<kobjref<CKObject>> choreoKeys;
	struct Bing {
		uint32_t a; uint8_t b;
	};
	std::vector<Bing> bings, dings;
	std::vector<uint32_t> fings;
	std::array<float, 4> sqUnk6;
	uint16_t sqUnk7;
	uint8_t sqUnk8;
	struct PoolEntry {
		kobjref<CKGrpPoolSquad> pool;
		kobjref<CKObject> cpnt;
		uint8_t u1;
		uint16_t numEnemies = 0; //(DRM!!!)
		uint8_t u2;
		kobjref<CKObject> u3;
	};
	std::vector<PoolEntry> pools;
	uint16_t sqUnkA;
	float sqUnkB;
	uint16_t sqUnkC;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpSquadEnemy : CKSubclass<CKGrpSquad, 26> {
	float seUnk1, seUnk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpPoolSquad : CKSubclass<CKGroup, 44> {
	uint32_t somenum;
	kobjref<CKObject> shadowCpnt;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpWalkingCharacter : CKSubclass<CKGroup, 45> {};
struct CKGrpStorageStd : CKSubclass<CKGroup, 59> {};

struct CKGrpBonusPool : CKSubclass<CKGroup, 61> {
	uint32_t bonusType;
	uint32_t handlerId, unk2, unk3, unk4; // unk3 and unk4 might be objrefs?
	kobjref<CKHkBasicBonus> nextBonusHook;
	kobjref<CKObject> bonusCpnt;
	kobjref<CKSceneNode> particleNode1, particleNode2;
	kobjref<CKObject> secondBonusCpnt;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpAsterixBonusPool : CKSubclass<CKGrpBonusPool, 63> {};
struct CKGrpWildBoarPool : CKSubclass<CKGrpBonusPool, 66> {};