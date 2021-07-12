#pragma once

#include "KObject.h"
#include <array>
#include <string>
#include <vector>
#include "CKPartlyUnknown.h"
#include "vecmat.h"

struct File;

namespace GamePatcher {
	extern File* g_drmValues;

	struct CKHook : CKCategory<2> {};
	struct CKComponent : CKCategory<6> {};
	struct CKLogic : CKCategory<12> {};

	struct CKGroup : CKCategory<4> {
		kobjref<CKGroup> nextGroup;
		kobjref<CKGroup> parentGroup;
		kobjref<CKObject> life;
		kobjref<CKObject> bundle;
		uint32_t unk2 = 0;
		kobjref<CKGroup> childGroup;
		kobjref<CKHook> childHook;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
	};

	struct CKAsterixGameManager : CKSubclass<CKLogic, 51> {
		std::array<uint8_t, 0x111> part1;
		std::array<uint8_t, 0x22> part2;
		std::string texture;
		std::array<uint8_t, 0x5B> part3;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
	};

	struct CKEnemyCoreCpnt : CKSubclass<CKComponent, 6969> {
		uint32_t firstVal;
		uint8_t health;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
	};

	struct CKEnemyCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 6> {};
	struct CKSquadEnemyCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 7> {};
	struct CKSeizableEnemyCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 8> {};
	struct CKSquadSeizableEnemyCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 9> {};
	struct CKBasicEnemyCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 10> {};
	struct CKBasicEnemyLeaderCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 11> {};
	struct CKJumpingRomanCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 12> {};
	struct CKRomanArcherCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 17> {};
	struct CKRocketRomanCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 19> {};
	struct CKJetPackRomanCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 23> {};
	struct CKMobileTowerCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 26> {};
	struct CKTriangularTurtleCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 28> {};
	struct CKSquareTurtleCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 29> {};
	struct CKDonutTurtleCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 30> {};
	struct CKPyramidalTurtleCpnt : CKPartlyUnknown<CKEnemyCoreCpnt, 31> {};

	struct CKGrpBaseSquad : CKSubclass<CKGroup, 18> {
		uint32_t bsUnk1;
		kobjref<CKObject> msgAction;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
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
			kobjref<CKObject> pool;
			kobjref<CKObject> cpnt;
			uint8_t u1;
			uint16_t numEnemies; //(DRM!!!)
			uint8_t u2;
			kobjref<CKObject> u3;
		};
		std::vector<PoolEntry> pools;
		uint16_t sqUnkA;
		float sqUnkB;
		uint16_t sqUnkC;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
	};

	struct CKGrpSquadEnemy : CKSubclass<CKGrpSquad, 26> {
		float seUnk1, seUnk2;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
	};

	struct CKGrpSquadJetPack : CKPartlyUnknown<CKGrpSquadEnemy, 64> {};

	struct CKHkTorchCore : CKSubclass<CKHook, 6969> {
		std::array<uint8_t, 0x18> part1;
		float timer;

		void deserialize(KEnvironment* kenv, File* file, size_t length) override;
		void serialize(KEnvironment* kenv, File* file) override;
	};

	struct CKHkTorch : CKPartlyUnknown<CKHkTorchCore, 32> {};
};