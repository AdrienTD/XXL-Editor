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

struct CKGrpMecaCpntAsterix;
struct CNode;
struct CParticlesNodeFx;
struct CKSoundDictionaryID;
struct CKGrpBonusSpitter;
struct CKGrpEnemy;
struct CKBoundingSphere;
struct CKHkSkyLife;
struct CKCameraSector;
struct CKShadowCpnt;

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

	void addHook(CKHook* hook);
	void addGroup(CKGroup* group);
};

struct CKReflectableGroup : CKMRSubclass<CKReflectableGroup, CKMemberReflectable<CKGroup>, 0xEA5E> {
	void reflectMembers2(MemberListener& r, KEnvironment* kenv) {}
};

template <class D, class T, int N> struct CKReflectableGroupSubclass : CKMRSubclass< D, T, N> {
	static_assert(std::is_base_of<CKReflectableGroup, T>::value, "T is not a reflectable group");

	void serialize(KEnvironment* kenv, File* file) override {
		CKGroup::serialize(kenv, file);
		WritingMemberListener r(file, kenv);
		((D*)this)->reflectMembers2(r, kenv);
	}

	void deserialize(KEnvironment* kenv, File* file, size_t length) override {
		CKGroup::deserialize(kenv, file, length);
		ReadingMemberListener r(file, kenv);
		((D*)this)->reflectMembers2(r, kenv);
	}
};

struct CKGroupLife : CKCategory<5> {
	kobjref<CKObject> unk;
	kobjref<CKGroup> group;
	uint32_t unk2 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGroupRoot : CKSubclass<CKGroup, 1> {};

struct CKGrpMeca : CKReflectableGroupSubclass<CKGrpMeca, CKReflectableGroup, 11> {
	int32_t ckgmUnk5;
	std::array<kobjref<CKGrpMecaCpntAsterix>, 2> ckgmUnk6;
	kobjref<CParticlesNodeFx> ckgmUnk7;
	std::array<kobjref<CNode>, 15> ckgmUnk8;
	kobjref<CNode> ckgmUnk9;
	kobjref<CNode> ckgmUnk10;
	std::array<kobjref<CParticlesNodeFx>, 2> ckgmUnk11;
	std::array<kobjref<CNode>, 2> ckgmUnk12;
	kobjref<CKGrpBonusSpitter> ckgmUnk13;
	kobjref<CKSoundDictionaryID> ckgmUnk14;
	float ckgmUnk15;
	float ckgmUnk16;
	float ckgmUnk17;
	float ckgmUnk18;
	float ckgmUnk19;
	float ckgmUnk20;
	float ckgmUnk21;
	float ckgmUnk22;
	float ckgmUnk23;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpTrio : CKReflectableGroupSubclass<CKGrpTrio, CKReflectableGroup, 12> {
	uint8_t ckgtUnk5;
	uint8_t ckgtUnk6;
	std::array<kobjref<CKObject>, 3> ckgtUnk7;
	std::array<kobjref<CKObject>, 3> ckgtUnk8;
	float ckgtUnk9;
	kobjref<CKGrpEnemy> ckgtUnk10;
	float ckgtUnk11;
	float ckgtUnk12;
	float ckgtUnk13;
	float ckgtUnk14;
	float ckgtUnk15;
	float ckgtUnk16;
	float ckgtUnk17;
	float ckgtUnk18;
	float ckgtUnk19;
	float ckgtUnk20;
	float ckgtUnk21;
	float ckgtUnk22;
	float ckgtUnk23;
	float ckgtUnk24;
	float ckgtUnk25;
	float ckgtUnk26;
	float ckgtUnk27;
	float ckgtUnk28;
	kobjref<CNode> ckgtUnk29;
	float ckgtUnk30;
	float ckgtUnk31;
	float ckgtUnk32;
	float ckgtUnk33;
	float ckgtUnk34;
	float ckgtUnk35;
	float ckgtUnk36;
	float ckgtUnk37;
	kobjref<CKBoundingSphere> ckgtUnk38;
	kobjref<CNode> ckgtUnk39;
	kobjref<CParticlesNodeFx> ckgtUnk40;
	kobjref<CNode> ckgtUnk41;
	kobjref<CNode> ckgtUnk42;
	kobjref<CNode> ckgtUnk43;
	kobjref<CNode> ckgtUnk44;
	float ckgtUnk45;
	float ckgtUnk46;
	kobjref<CKHkSkyLife> ckgtUnk47;
	float ckgtUnk48;
	int32_t ckgtUnk49;
	int32_t ckgtUnk50;
	float ckgtUnk51;
	float ckgtUnk52;
	float ckgtUnk53;
	float ckgtUnk54;
	float ckgtUnk55;
	float ckgtUnk56;
	uint8_t ckgtUnk57;
	float ckgtUnk58;
	int32_t ckgtUnk59;
	float ckgtUnk60;
	float ckgtUnk61;
	float ckgtUnk62;
	uint8_t ckgtUnk63;
	float ckgtUnk64;
	int32_t ckgtUnk65;
	float ckgtUnk66;
	float ckgtUnk67;
	std::array<float, 2> ckgtUnk68;
	std::string ckgtUnk69;
	int32_t ckgtUnk70;
	kobjref<CKCameraSector> ckgtUnk71;
	float ckgtUnk72;
	float ckgtUnk73;
	float ckgtUnk74;
	float ckgtUnk75;
	float ckgtUnk76;
	float ckgtUnk77;
	float ckgtUnk78;
	EventNode ckgtUnk79;
	EventNode ckgtUnk80;
	EventNode ckgtUnk81;
	EventNode ckgtUnk82;
	EventNode ckgtUnk83;
	EventNode ckgtUnk84;
	EventNode ckgtUnk85;
	EventNode ckgtUnk86;
	EventNode ckgtUnk87;
	EventNode ckgtUnk88;
	EventNode ckgtUnk89;
	EventNode ckgtUnk90;
	EventNode ckgtUnk91;
	EventNode ckgtUnk92;
	EventNode ckgtUnk93;
	EventNode ckgtUnk94;
	EventNode ckgtUnk95;
	EventNode ckgtUnk96;
	EventNode ckgtUnk97;
	//uint8_t ckgtUnk98;
	std::vector<MarkerIndex> ckgtUnk99;
	//uint8_t ckgtUnk106;
	std::vector<MarkerIndex> ckgtUnk107;
	//uint8_t ckgtUnk108;
	std::vector<MarkerIndex> ckgtUnk109;
	float ckgtUnk111;
	float ckgtUnk112;
	std::array<float, 2> ckgtUnk113;
	std::string ckgtUnk114;
	int32_t ckgtUnk115;
	kobjref<CParticlesNodeFx> ckgtUnk116;
	std::array<float, 2> ckgtUnk117;
	std::array<float, 4> ckgtUnk118;
	int32_t ckgtUnk119;
	kobjref<CNode> ckgtUnk120;
	std::array<float, 6> ckgtUnk121;
	float ckgtUnk122;
	std::array<float, 2> ckgtUnk123;
	std::array<kobjref<CKObject>, 2> ckgtUnk124;
	std::array<float, 3> ckgtUnk125;
	uint8_t ckgtUnk126;
	kobjref<CKObject> ckgtUnk127;
	kobjref<CKObject> ckgtUnk128;
	std::array<float, 2> ckgtUnk129;
	kobjref<CKObject> ckgtUnk130;
	float ckgtUnk131;
	float ckgtUnk132;
	int32_t ckgtUnk133;
	int32_t ckgtUnk134;
	float ckgtUnk135;
	float ckgtUnk136;
	std::array<float, 2> ckgtUnk137;
	std::string ckgtUnk138;
	int32_t ckgtUnk139;
	float ckgtUnk140;
	float ckgtUnk141;
	float ckgtUnk142;
	float ckgtUnk143;
	float ckgtUnk144;
	float ckgtUnk145;
	uint8_t ckgtUnk146;
	float ckgtUnk147;
	int32_t ckgtUnk148;
	kobjref<CKObject> ckgtUnk149;
	float ckgtUnk150;
	float ckgtUnk151;
	float ckgtUnk152;
	float ckgtUnk153;
	float ckgtUnk154;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};


struct CKGrpBoat : CKSubclass<CKGroup, 16> {};

struct CKGrpBaseSquad : CKSubclass<CKGroup, 18> {
	uint32_t bsUnk1;
	kobjref<CKMsgAction> msgAction;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpSquad : CKSubclass<CKGrpBaseSquad, 24> {
	Matrix mat1 = Matrix::getIdentity(), mat2 = Matrix::getIdentity();
	float sqUnk1 = 3.0f;
	Vector3 sqUnk2;
	kobjref<CKObject> sqBizObj1; MarkerIndex sqBizMarker1;
	kobjref<CKObject> sqBizObj2; MarkerIndex sqBizMarker2;
	std::array<Vector3, 2> sqUnk3, sqUnk4;
	uint32_t sqUnk5 = 0;
	//uint32_t numChoreographies;
	std::vector<kobjref<CKChoreography>> choreographies;
	//uint32_t numChoreoKeys;
	std::vector<kobjref<CKChoreoKey>> choreoKeys;
	struct Bing {
		MarkerIndex markerIndex; uint8_t b = 0;
	};
	std::vector<Bing> guardMarkers, spawnMarkers;
	std::vector<uint32_t> fings; // seems to be always empty
	std::array<float, 3> sqUnk6 = { 5.0f, 1.570796f, 0.5f };
	uint32_t sqUnk6b = 0; // useless
	uint16_t sqUnk7;
	uint8_t sqUnk8;
	struct PoolEntry {
		kobjref<CKGrpPoolSquad> pool;
		kobjref<CKEnemyCpnt> cpnt;
		uint8_t u1 = 0;
		uint16_t numEnemies = 0; //(DRM!!!)
		uint8_t u2 = 0;
		kobjref<CKObject> u3;
	};
	std::vector<PoolEntry> pools;
	EventNode sqUnkA;
	float sqUnkB = 300.0f;
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

struct CKGrpFrontEnd : CKReflectableGroupSubclass<CKGrpFrontEnd, CKReflectableGroup, 53> {
	kobjref<CKSoundDictionaryID> ckgfeSoundDict;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpCatapult : CKReflectableGroupSubclass<CKGrpCatapult, CKReflectableGroup, 54> {
	kobjref<CKShadowCpnt> ckgcShadowCpnt;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpMap : CKReflectableGroupSubclass<CKGrpMap, CKReflectableGroup, 56> {
	std::array<kobjref<CKObject>, 2> ckgmUnk5;
	std::array<kobjref<CKObject>, 21> ckgmUnk6;
	std::array<kobjref<CKObject>, 6> ckgmUnk7;
	std::array<kobjref<CKObject>, 2> ckgmUnk8;
	std::array<kobjref<CKObject>, 3> ckgmUnk9;
	std::array<kobjref<CKObject>, 6> ckgmUnk10;
	kobjref<CKSoundDictionaryID> ckgmUnk11;
	std::array<kobjref<CKObject>, 2> ckgmUnk12;
	std::array<kobjref<CKObject>, 6> ckgmUnk13;
	std::array<kobjref<CKObject>, 5> ckgmUnk14;
	std::array<kobjref<CKObject>, 6> ckgmUnk15;
	std::array<kobjref<CKObject>, 3> ckgmUnk16;
	std::array<kobjref<CKObject>, 2> ckgmUnk17;
	std::array<int32_t, 4> ckgmUnk18;
	std::array<int32_t, 2> ckgmUnk19;
	std::array<int32_t, 13> ckgmUnk20;
	std::array<int32_t, 4> ckgmUnk21;
	std::array<float, 8> ckgmUnk22;
	std::array<float, 18> ckgmUnk23;
	kobjref<CKObject> ckgmUnk24;
	std::array<float, 2> ckgmUnk25;
	std::array<uint16_t, 27> ckgmUnk26;
	std::array<kobjref<CKObject>, 2> ckgmUnk27;
	std::array<float, 2> ckgmUnk28;
	uint16_t ckgmUnk29;
	std::array<kobjref<CKObject>, 3> ckgmUnk30;
	kobjref<CKObject> ckgmUnk31;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpStorageStd : CKSubclass<CKGroup, 59> {};
struct CKGrpCrate : CKSubclass<CKGroup, 60> {};

struct CKGrpBonusPool : CKSubclass<CKGroup, 61> {
	uint32_t bonusType;
	uint32_t handlerId, maxBeaconBonusesOnScreen;
	float x2UnkFlt = 110.0f; // XXL2+, in Arthur+ it's -1 (different context?)
	uint8_t arUnkByte = 1; // Arthur+
	kobjref<CKObject> unk3, unk4; // always null references
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
	float sjpUnk1;
	uint8_t sjpUnk2;
	uint8_t sjpUnk3;
	std::array<kobjref<CKSceneNode>, 3> particleNodes;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpWildBoarPool : CKSubclass<CKGrpBonusPool, 66> {};

struct CKGrpAsterixCheckpoint : CKReflectableGroupSubclass<CKGrpAsterixCheckpoint, CKReflectableGroup, 75> {
	float astCheckpointValue = 4.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpBonusSpitter : CKReflectableGroupSubclass<CKGrpBonusSpitter, CKReflectableGroup, 76> {
	kobjref<CKSoundDictionaryID> ckgbsUnk5;
	kobjref<CParticlesNodeFx> ckgbsUnk6;
	float ckgbsUnk7 = 0.2f;
	float ckgbsUnk8 = 0.4f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpLight : CKSubclass<CKGroup, 77> {
	kobjref<CKSceneNode> node;
	std::string texname;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpA2BonusPool : CKSubclass<CKGrpBonusPool, 91> {};
struct CKGrpBonusX2 : CKSubclass<CKGroup, 92> {};
struct CKGrpA3BonusPool : CKSubclass<CKGrpBonusPool, 31> {};

///--- Group life classes ---///

struct CKGrpTrioLife : CKSubclass<CKGroupLife, 6> {};
struct CKGrpMecaLife : CKSubclass<CKGroupLife, 10> {};
struct CKGrpBonusLife : CKSubclass<CKGroupLife, 19> {};
struct CKGrpMapLife : CKSubclass<CKGroupLife, 22> {};
struct CKGrpEnemyLife : CKSubclass<CKGroupLife, 24> {};
struct CKGrpAsterixCheckpointLife : CKSubclass<CKGroupLife, 29> {};
