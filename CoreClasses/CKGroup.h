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

struct CKGroup : CKMRSubclass<CKGroup, CKMemberReflectable<CKCategory<4>>, 0> {
	kobjref<CKGroup> nextGroup;
	kobjref<CKGroup> parentGroup;
	kobjref<CKGroupLife> life;
	kobjref<CKBundle> bundle;
	uint32_t unk2 = 0;
	kobjref<CKGroup> childGroup;
	kobjref<CKHook> childHook;

	// XXL2+:
	uint32_t x2UnkA;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);

	void addHook(CKHook* hook);
	void addGroup(CKGroup* group);
	void removeGroup(CKGroup* group);
};

// TO REMOVE
template <class D, class T, int N> using CKReflectableGroupSubclass = CKMRSubclass<D, T, N>;

struct CKGroupLife : CKCategory<5> {
	kobjref<CKObject> unk;
	kobjref<CKGroup> group;
	uint32_t unk2 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};


// =================== Common Group classes ===================


struct CKGroupRoot : CKSubclass<CKGroup, 1> {};

struct CKGrpBaseSquad : CKReflectableGroupSubclass<CKGrpBaseSquad, CKGroup, 18> {
	uint32_t bsUnk1 = 0;
	kobjref<CKMsgAction> msgAction;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpSquad : CKReflectableGroupSubclass<CKGrpSquad, CKGrpBaseSquad, 24> {
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
	uint16_t sqUnk7 = 0;
	uint8_t sqUnk8 = 255;
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

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

// Fight data either stored in the squad (XXL2), or in the fight zone (Arthur+)
struct X2FightData {
	struct PoolEntry {
		kobjref<CKGrpPoolSquad> pool;
		uint8_t componentIndex = 0;
		uint16_t numEnemies = 0;
		uint8_t numInitiallySpawned = 0; // XXL2 only
	};
	struct SpawnPoint {
		Vector3 position, direction;
		uint8_t flags = 0;
	};
	struct ConstrainedSpawnPoint {
		Vector3 position, direction;
		uint8_t flags = 0, poolIndex = 0, numMembersToRespawn = 0, isConstrained = 1;
		kobjref<CKObject> squadGroup; // Arthur+
	};

	std::vector<PoolEntry> pools;
	std::vector<SpawnPoint> spawnPoints;
	std::vector<ConstrainedSpawnPoint> constrainedSpawnPoints;
};

struct CKGrpSquadX2 : CKReflectableGroupSubclass<CKGrpSquadX2, CKGroup, 24> {
	//uint32_t numPhases;
	struct Phase {
		Matrix mat = Matrix::getIdentity();
		// Arthur+:
		uint8_t ogpuUnk0 = 0;
		kobjref<CKObject> ogpuUnkObj1;
		// Spyro+:
		std::array<uint8_t, 4> spUnk1;
		kobjref<CKObject> spSpawnCamera;
		uint8_t spUnk3;
		std::array<uint32_t, 6> spUnk4;
		std::vector<uint32_t> spUnk5;
		// XXL2+:
		uint8_t followLeader = 0, lookAtLeader = 0, autoRespawn = 1;
		uint32_t orientationTarget = 0;
		Vector3 orientationTargetVector;
		kobjref<CKObject> orientationTargetHook;
		uint8_t behavior = 0;
		kobjref<CKChoreography> choreography;
		// Arthur+:
		uint8_t ogpuUnkA = 0;
		float ogpuUnkB = 0.0f;
		void reflectMembers(MemberListener& r, KEnvironment *kenv);
	};
	std::vector<Phase> phases;
	X2FightData fightData;
	//uint32_t numVecs;
	std::vector<Vector3> anchors;
	float angularSpeed = 1.571f;
	float timerDuration = 0.0f;
	float reinitDuration = 0.0f;
	float originalReinitDuration = -1.0f;
	EventNode evtStart, evtEnemyDead, evtSquadDead;

	// Spyro+:
	EventNode evtPhaseChanged, alEventUnk2;
	uint32_t alUnk3;
	uint8_t alUnk4;
	// Alice+:
	uint8_t alEnd1;
	kobjref<CKObject> alEnd2;
	Vector3 alEnd3;
	kobjref<CKObject> alEnd4;
	uint8_t alEnd5;

	struct OgThing {
		uint8_t ogt1, ogt2;
		std::vector<float> ogt3;
		std::vector<float> alt4;
		uint32_t alt5;
	};
	std::vector<std::vector<OgThing>> ogThings;
	std::vector<uint8_t> ogBytes;
	uint32_t ogVeryUnk = 1;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpSquadEnemy : CKReflectableGroupSubclass<CKGrpSquadEnemy, CKGrpSquad, 26> {
	float seUnk1 = 10.0f, seUnk2 = 50.0f;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpEnemy : CKSubclass<CKGroup, 39> {};

struct CKGrpPoolSquad : CKReflectableGroupSubclass<CKGrpPoolSquad, CKGroup, 44> {
	// XXL1
	uint32_t somenum;
	kobjref<CKObject> shadowCpnt;

	// XXL2+
	std::vector<kobjref<CKObject>> components;
	uint8_t enemyType = 0;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpBonus : CKSubclass<CKGroup, 48> {};

struct CKGrpCrate : CKSubclass<CKGroup, 60> {};

struct CKGrpBonusPool : CKReflectableGroupSubclass<CKGrpBonusPool, CKGroup, 61> {
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

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpAsterixBonusPool : CKSubclass<CKGrpBonusPool, 63> {};

struct CKGrpSquadJetPack : CKReflectableGroupSubclass<CKGrpSquadJetPack, CKGrpSquadEnemy, 64> {
	std::vector<kobjref<CKHook>> hearths;
	float sjpUnk1 = 5.0f;
	uint8_t sjpUnk2 = 2;
	uint8_t sjpUnk3 = 0;
	std::array<kobjref<CKSceneNode>, 3> particleNodes;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpWildBoarPool : CKSubclass<CKGrpBonusPool, 66> {};

struct CKGrpLight : CKReflectableGroupSubclass<CKGrpLight, CKGroup, 77> {
	kobjref<CKSceneNode> node;
	std::string texname;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGrpA2BonusPool : CKSubclass<CKGrpBonusPool, 91> {};
struct CKGrpBonusX2 : CKSubclass<CKGroup, 92> {};
struct CKGrpArBonusPool : CKSubclass<CKGrpBonusPool, 15> {};
struct CKGrpA3BonusPool : CKSubclass<CKGrpBonusPool, 31> {};

