#pragma once

#include "KObject.h"
#include <array>
#include <vector>
#include "vecmat.h"
#include "CKPartlyUnknown.h"
#include "Events.h"

struct CKHook;
struct CKGroupLife;
struct CKSceneNode;

struct CKHkBasicBonus;
struct CKGrpPoolSquad;
struct CKEnemyCpnt;

struct CKChoreoKey;
struct CKChoreography;

struct CKGroup : CKCategory<4> {
	kobjref<CKGroup> nextGroup;
	kobjref<CKGroup> parentGroup;
	kobjref<CKGroupLife> life;
	kobjref<CKObject> bundle;
	uint32_t unk2 = 0;
	kobjref<CKGroup> childGroup;
	kobjref<CKHook> childHook;

	// XXL2+:
	uint32_t x2UnkA;

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

struct CKGrpBoat : CKSubclass<CKGroup, 16> {};

struct CKGrpBaseSquad : CKSubclass<CKGroup, 18> {
	uint32_t bsUnk1;
	kobjref<CKObject> msgAction;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpSquad : CKSubclass<CKGrpBaseSquad, 24> {
	Matrix mat1, mat2;
	float sqUnk1;
	Vector3 sqUnk2;
	std::array<kobjref<CKObject>, 4> refs;
	std::array<float, 6> sqUnk3, sqUnk4;
	uint32_t sqUnk5;
	//uint32_t numChoreographies;
	std::vector<kobjref<CKChoreography>> choreographies;
	//uint32_t numChoreoKeys;
	std::vector<kobjref<CKChoreoKey>> choreoKeys;
	struct Bing {
		uint32_t markerIndex; uint8_t b;
	};
	std::vector<Bing> guardMarkers, spawnMarkers;
	std::vector<uint32_t> fings; // seems to be always empty
	std::array<float, 4> sqUnk6;
	uint16_t sqUnk7;
	uint8_t sqUnk8;
	struct PoolEntry {
		kobjref<CKGrpPoolSquad> pool;
		kobjref<CKEnemyCpnt> cpnt;
		uint8_t u1;
		uint16_t numEnemies = 0; //(DRM!!!)
		uint8_t u2;
		kobjref<CKObject> u3;
	};
	std::vector<PoolEntry> pools;
	EventNode sqUnkA;
	float sqUnkB;
	EventNode sqUnkC;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpSquadEnemy : CKSubclass<CKGrpSquad, 26> {
	float seUnk1, seUnk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpEnemy : CKSubclass<CKGroup, 39> {};

struct CKGrpPoolSquad : CKSubclass<CKGroup, 44> {
	uint32_t somenum;
	kobjref<CKObject> shadowCpnt;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpWalkingCharacter : CKSubclass<CKGroup, 45> {};
struct CKGrpBonus : CKSubclass<CKGroup, 48> {};
struct CKGrpStorageStd : CKSubclass<CKGroup, 59> {};
struct CKGrpCrate : CKSubclass<CKGroup, 60> {};

struct CKGrpBonusPool : CKSubclass<CKGroup, 61> {
	uint32_t bonusType;
	uint32_t handlerId, unk2;
	float x2UnkFlt = 110.0f; // XXL2+
	uint32_t unk3, unk4; // unk3 and unk4 might be objrefs?
	kobjref<CKHkBasicBonus> nextBonusHook;
	kobjref<CKObject> bonusCpnt;
	kobjref<CKSceneNode> particleNode1, particleNode2;
	kobjref<CKObject> secondBonusCpnt; // only XXL1, removed in XXL2+

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpAsterixBonusPool : CKSubclass<CKGrpBonusPool, 63> {};

struct CKGrpSquadJetPack : CKSubclass<CKGrpSquadEnemy, 64> {
	std::vector<kobjref<CKHook>> hearths;
	uint32_t sjpUnk1;
	uint8_t sjpUnk2;
	uint8_t sjpUnk3;
	std::array<kobjref<CKSceneNode>, 3> particleNodes;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpWildBoarPool : CKSubclass<CKGrpBonusPool, 66> {};

struct CKGrpLight : CKSubclass<CKGroup, 77> {
	kobjref<CKSceneNode> node;
	std::string texname;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpA2BonusPool : CKSubclass<CKGrpBonusPool, 91> {};

// Unknown group classes. To implement later!
struct CKGrpMeca : CKPartlyUnknown<CKGroup, 11> {};
struct CKGrpTrio : CKPartlyUnknown<CKGroup, 12> {};
struct CKGrpFrontEnd : CKPartlyUnknown<CKGroup, 53> {};
struct CKGrpCatapult : CKPartlyUnknown<CKGroup, 54> {};
struct CKGrpMap : CKPartlyUnknown<CKGroup, 56> {};
struct CKGrpAsterixCheckpoint : CKPartlyUnknown<CKGroup, 75> {};
struct CKGrpBonusSpitter : CKPartlyUnknown<CKGroup, 76> {};
//struct CKGrpLight : CKPartlyUnknown<CKGroup, 77> {};

///--- Group life classes ---///

struct CKGrpTrioLife : CKSubclass<CKGroupLife, 6> {};
struct CKGrpMecaLife : CKSubclass<CKGroupLife, 10> {};
struct CKGrpBonusLife : CKSubclass<CKGroupLife, 19> {};
struct CKGrpMapLife : CKSubclass<CKGroupLife, 22> {};
struct CKGrpEnemyLife : CKSubclass<CKGroupLife, 24> {};
struct CKGrpAsterixCheckpointLife : CKSubclass<CKGroupLife, 29> {};
