#pragma once

#include "KObject.h"
#include <array>
#include <vector>
#include "vecmat.h"
#include "CKPartlyUnknown.h"
#include "Events.h"
#include "CKUtils.h"

struct CKHook;
struct CKGroupLife;
struct CKBundle;
struct CKSceneNode;

struct CKHkBasicBonus;
struct CKGrpPoolSquad;
struct CKEnemyCpnt;

struct CKMsgAction;
struct CKChoreoKey;
struct CKChoreography;

struct CKGroup : CKCategory<4> {
	kobjref<CKGroup> nextGroup;
	kobjref<CKGroup> parentGroup;
	kobjref<CKGroupLife> life;
	kobjref<CKBundle> bundle;
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
	kobjref<CKMsgAction> msgAction;

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
	uint8_t sqRomasterValue = 0;
	EventNode sqUnkC;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpSquadX2 : CKMRSubclass<CKGrpSquadX2, CKMemberReflectable<CKGroup>, 24> {
	//uint32_t numPhases;
	struct Phase {
		Matrix mat;
		//
		uint8_t ogpuUnk0;
		kobjref<CKObject> ogpuUnkObj1;
		//
		uint8_t pu1, pu2, pu3;
		uint32_t pu4;
		uint32_t pu5, pu6, pu7;
		kobjref<CKObject> puUnkObj;
		uint8_t pu8;
		kobjref<CKChoreography> choreography;
		//
		uint8_t ogpuUnkA;
		float ogpuUnkB;
		void reflectMembers(MemberListener& r, KEnvironment *kenv) {
			r.reflect(mat, "mat");
			if (kenv->version >= kenv->KVERSION_ARTHUR) {
				r.reflect(ogpuUnk0, "ogpuUnk0");
				r.reflect(ogpuUnkObj1, "ogpuUnkObj1");
			}
			r.reflect(pu1, "pu1");
			r.reflect(pu2, "pu2");
			r.reflect(pu3, "pu3");
			r.reflect(pu4, "pu4");
			r.reflect(pu5, "pu5");
			r.reflect(pu6, "pu6");
			r.reflect(pu7, "pu7");
			r.reflect(puUnkObj, "puUnkObj");
			r.reflect(pu8, "pu8");
			r.reflect(choreography, "choreography");
			if (kenv->version >= kenv->KVERSION_ARTHUR) {
				r.reflect(ogpuUnkA, "ogpuUnkA");
				r.reflect(ogpuUnkB, "ogpuUnkB");
			}
		}
	};
	std::vector<Phase> phases;
	//uint8_t numPoolEntries;
	struct PoolEntry {
		kobjref<CKGrpPoolSquad> pool;
		uint8_t u1;
		uint16_t numEnemies = 0;
		uint8_t u2;
		void reflectMembers(MemberListener& r) {
			r.reflect(pool, "pool");
			r.reflect(u1, "u1");
			r.reflect(numEnemies, "numEnemies");
			r.reflect(u2, "u2");
		}
	};
	std::vector<PoolEntry> pools;
	//uint32_t numSlots;
	struct Slot {
		Vector3 pos, dir;
		uint8_t index;
		void reflectMembers(MemberListener& r) {
			r.reflect(pos, "pos");
			r.reflect(dir, "dir");
			r.reflect(index, "index");
		}
	};
	std::vector<Slot> slots;
	//uint32_t numSlots2;
	struct Slot2 {
		Vector3 pos, dir;
		uint8_t us1, us2, us3, us4;
		void reflectMembers(MemberListener& r) {
			r.reflect(pos, "pos");
			r.reflect(dir, "dir");
			r.reflect(us1, "us1");
			r.reflect(us2, "us2");
			r.reflect(us3, "us3");
			r.reflect(us4, "us4");
		}
	};
	std::vector<Slot2> slots2;
	//uint32_t numVecs;
	std::vector<Vector3> vecVec;
	float x2sqUnk1, x2sqUnk2, x2sqUnk3, x2sqUnk4;
	std::vector<kobjref<CKObject>> x2sqObjList1, x2sqObjList2, x2sqObjList3;

	struct OgThing {
		uint8_t ogt1, ogt2;
		std::vector<float> ogt3;
		void reflectMembers(MemberListener& r) {
			r.reflect(ogt1, "ogt1");
			r.reflect(ogt2, "ogt2");
			r.reflectSize<uint32_t>(ogt3, "size_ogt3");
			r.reflectContainer(ogt3, "ogt3");
		}
	};
	std::vector<std::vector<OgThing>> ogThings;
	std::vector<uint8_t> ogBytes;
	uint32_t ogVeryUnk;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpSquadEnemy : CKSubclass<CKGrpSquad, 26> {
	float seUnk1, seUnk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpEnemy : CKSubclass<CKGroup, 39> {};

struct CKGrpPoolSquad : CKSubclass<CKGroup, 44> {
	// XXL1
	uint32_t somenum;
	kobjref<CKObject> shadowCpnt;

	// XXL2+
	std::vector<kobjref<CKObject>> components;
	uint8_t enemyType = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpWalkingCharacter : CKSubclass<CKGroup, 45> {};
struct CKGrpBonus : CKSubclass<CKGroup, 48> {};
struct CKGrpStorageStd : CKSubclass<CKGroup, 59> {};
struct CKGrpCrate : CKSubclass<CKGroup, 60> {};

struct CKGrpBonusPool : CKSubclass<CKGroup, 61> {
	uint32_t bonusType;
	uint32_t handlerId, maxBeaconBonusesOnScreen;
	float x2UnkFlt = 110.0f; // XXL2+, in Arthur+ it's -1 (different context?)
	uint8_t arUnkByte = 1; // Arthur+
	uint32_t unk3, unk4; // unk3 and unk4 might be objrefs?
	kobjref<CKHkBasicBonus> nextBonusHook;
	kobjref<CKObject> bonusCpnt;
	kobjref<CKSceneNode> particleNode1, particleNode2;
	kobjref<CKObject> secondBonusCpnt; // only XXL1, removed in XXL2+
	kobjref<CKObject> ogSekensLauncherCpnt; // OG+

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
struct CKGrpBonusX2 : CKSubclass<CKGroup, 92> {};
struct CKGrpA3BonusPool : CKSubclass<CKGrpBonusPool, 31> {};

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
