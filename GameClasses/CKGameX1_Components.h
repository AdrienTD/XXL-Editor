#pragma once

#include "CoreClasses/CKComponent.h"
#include "vecmat.h"

struct CKSoundDictionaryID;


// =================== XXL1 Component classes ===================

namespace GameX1 {

	struct CKGrpMecaCpntAsterix : CKMRSubclass<CKGrpMecaCpntAsterix, CKComponent, 2> {
		kobjref<CKObject> cpmecWoodenCrateCpnt;
		kobjref<CKObject> cpmecMetalCrateCpnt;
		kobjref<CKObject> cpmecPark1CrateCpnt;
		kobjref<CKObject> cpmecPark3CrateCpnt;
		kobjref<CKObject> cpmecPark5CrateCpnt;
		std::array<kobjref<CKObject>, 17> cpmecOtherRefs;
		std::array<float, 13> cpmecUnk6;
		kobjref<CKObject> cpmecSndDictID;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKSquadEnemyCpnt : CKMRSubclass<CKSquadEnemyCpnt, CKEnemyCpnt, 7> {
		void reflectMembers2(MemberListener& r, KEnvironment* kenv) { CKEnemyCpnt::reflectMembers2(r, kenv); }
	};

	struct CKSeizableEnemyCpnt : CKMRSubclass<CKSeizableEnemyCpnt, CKEnemyCpnt, 8> {
		float seUnk1 = 1.5f;
		uint8_t seUnk2 = 1;
		Vector3 seUnk3 = { 0.0f, 0.0f, 0.0f };
		float seBBoxSize = 0.525267f;
		uint8_t seFortitude = 1;
		Vector3 seUnk6 = { 0.0f, 0.0f, 0.094667f };
		float seBBoxOffset = 0.418867f;
		uint8_t seUnk8 = 3;
		Vector3 seUnk9 = { 0.0f, 0.222667f, 0.0f };
		float seColScale = 0.5f;
		float seUnk11 = 0.8206f;
		uint8_t seUnk12 = 0;
		Vector3 seColOffset = { 0.0f, 0.274f, 0.247333f };
		float seStunTime = 1.5f;
		float seUnk15 = 2.2f;
		float seKnockback = 2.5f;
		float seUnk17 = 10.0f;
		float seUnk18 = 1.5f;
		float seUnk19 = 4.0f;
		float seUnk20 = 0.7f;
		float seUnk21 = 10.0f;
		float seUnk22 = 10.0f;
		float seUnk23 = 1.6f;
		float seComboStunTime = 5.0f;
		float seDeathSpeed = 30.0f;
		float seDeathFlySpeed = 9.81f;
		uint8_t seShieldPoints = 0;
		uint8_t seUnk28 = 0;
		float seCoverTime = 1.0f;
		float seKnockbackSpeed = 5.0f;
		float seKnockbackResistance = 0.5f;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKSquadSeizableEnemyCpnt : CKMRSubclass<CKSquadSeizableEnemyCpnt, CKSeizableEnemyCpnt, 9> {
		float sqseUnk1 = 12.0f;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKBasicEnemyCpnt : CKMRSubclass<CKBasicEnemyCpnt, CKSquadSeizableEnemyCpnt, 10> {
		Vector3 beUnk1 = { 0.1f, 0.1f, 0.604667f };
		uint8_t beUnk2 = 8;
		Vector3 beUnk3 = { -0.15f, -0.05f, 0.8f };
		float beRange = 2.8f;
		float beUnk5 = 1.0f;
		float beChargeDuration = 1.5f;
		float beAttackTime1 = 10.0f;
		float beAttackTime2 = 10.0f;
		float beAttackTime3 = 10.0f;
		float beAttackTime4 = 10.0f;
		float beAttackTime5 = 0.52f;
		float beUnk12 = 5.0f;
		float beUnk13 = 8.0f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKBasicEnemyLeaderCpnt : CKMRSubclass<CKBasicEnemyLeaderCpnt, CKBasicEnemyCpnt, 11> {
		void reflectMembers2(MemberListener& r, KEnvironment* kenv) { CKBasicEnemyCpnt::reflectMembers2(r, kenv); }
	};

	struct CKJumpingRomanCpnt : CKMRSubclass<CKJumpingRomanCpnt, CKSquadSeizableEnemyCpnt, 12> {
		float jrUnk1 = 0.5f;
		float jrUnk2 = 0.0f;
		Vector3 jrUnk3 = { 0.2f, -0.4f, -1.467333f };
		float jrUnk4 = 2.0f;
		float jrUnk5 = 1.0f;
		float jrUnk6 = 2.0f;
		float jrUnk7 = 0.3f;
		float jrUnk8 = 18.0f;
		float jrUnk9 = 55.0f;
		float jrUnk10 = 5.0f;
		float jrUnk11 = 5.0f;
		float jrUnk12 = 10.0f;
		float jrUnk13 = 15.0f;
		float jrUnk14 = 30.0f;
		float jrUnk15 = 25.0f;
		float jrUnk16 = 1.0f;
		float jrUnk17 = 0.0f;
		float jrUnk18 = 1.0f;
		float jrUnk19 = 12.0f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKRomanArcherCpnt : CKMRSubclass<CKRomanArcherCpnt, CKSquadSeizableEnemyCpnt, 17> {
		float raUnk1 = 0.5f;
		float raUnk2 = 28.0f;
		float raUnk3 = 3.0f;
		float raUnk4 = 25.0f;
		float raUnk5 = 3.141593f;
		float raUnk6 = 4.0f;
		float raUnk7 = 3.0f;
		uint8_t raNumArrowsPerAttack = 6;
		float raArrowTimeInterval = 0.2f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKRocketRomanCpnt : CKMRSubclass<CKRocketRomanCpnt, CKBasicEnemyCpnt, 19> {
		float rrCylinderRadius = 0, rrCylinderHeight = 0;
		Vector3 rrUnk3 = Vector3(0, 0, 0);
		uint8_t rrUnk4 = 0;
		float rrFireDistance = 0;
		uint8_t rrUnk6 = 0;
		float rrFlySpeed = 0, rrRomanAimFactor = 0;
		kobjref<CKObject> rrUnk9;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKJetPackRomanCpnt : CKMRSubclass<CKJetPackRomanCpnt, CKSquadEnemyCpnt, 23> {
		float jpUnk1 = 5.0f;
		float jpUnk2 = 10.0f;
		float jpUnk3 = 15.0f;
		float jpUnk4 = 1.0f;
		float jpUnk5 = 3.0f;
		float jpUnk6 = 1.0f;
		float jpUnk7 = 0.5f;
		float jpUnk8 = 3.0f;
		float jpUnk9 = 1.0f;
		float jpUnk10 = 0.8f;
		float jpUnk11 = 1.5f;
		uint8_t jpUnk12 = 3;
		uint8_t jpUnk13 = 1;
		float jpUnk14 = 3.0f;
		uint8_t jpUnk15 = 10;
		float jpUnk16 = 1.5f;
		float jpUnk17 = 1.0f;
		float jpUnk18 = 5.0f;
		float jpUnk19 = 1.0f;
		float jpUnk20 = 5.0f;
		float jpUnk21 = 5.0f;
		float jpUnk22 = 3.5f;
		float jpUnk23 = 0.2f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKWildBoarCpnt : CKMRSubclass<CKWildBoarCpnt, CKComponent, 25> {
		float ckwbcUnk0 = 4.0f;
		kobjref<CKSoundDictionaryID> ckwbcUnk1;
		float ckwbcUnk2 = 15.0f;
		float ckwbcUnk3 = 7.0f;
		float ckwbcUnk4 = 4.0f;
		float ckwbcUnk5 = 0.3f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKMobileTowerCpnt : CKMRSubclass<CKMobileTowerCpnt, CKSquadEnemyCpnt, 26> {
		uint8_t mtUnk1 = 3;
		float mtUnk2 = 6.0f;
		float mtUnk3 = 15.0f;
		float mtUnk4 = 8.0f;
		float mtUnk5 = 7.0f;
		float mtUnk6 = 0.5f;
		float mtUnk7 = 0.4f;
		float mtUnk8 = 10.0f;
		float mtUnk9 = 1000.0f;
		float mtUnk10 = 600.0f;
		float mtUnk11 = 10.0f;
		float mtUnk12 = 22.0f;
		float mtUnk13 = 10.0f;
		float mtUnk14 = 12.0f;
		float mtUnk15 = 12.0f;
		uint8_t mtUnk16 = 1;
		float mtUnk17 = 10.0f;
		uint8_t mtUnk18 = 100;
		uint8_t mtUnk19 = 3;
		uint8_t mtUnk20 = 2;
		uint8_t mtUnk21 = 5;
		uint8_t mtUnk22 = 0;
		float mtUnk23 = 300.0f;
		uint8_t mtUnk24 = 0;
		uint8_t mtUnk25 = 0;
		uint8_t mtUnk26 = 0;
		uint8_t mtUnk27 = 0;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKTurtleCpnt : CKMRSubclass<CKTurtleCpnt, CKSquadEnemyCpnt, 27> {
		uint8_t ttUnk1 = 0;
		float ttUnk2 = 3.5f;
		uint8_t ttUnk3 = 2;
		float ttUnk4 = 15.0f;
		uint8_t ttUnk5 = 20;
		uint8_t ttUnk6 = 10;
		float ttUnk7 = 2.0f;
		float ttUnk8 = 5.0f;
		//uint16_t ttUnk9;

		struct SpearState {
			float delay;
			uint16_t sides;
			void reflectMembers(MemberListener& r) {
				r.reflect(delay, "delay");
				r.reflect(sides, "sides");
			}
		};
		std::vector<SpearState> ttSpearStates = { {4.0f, 0}, {4.0f, 1} };

		uint16_t ttNumSpearSides = 4;
		float ttUnk12 = 1.0f;
		float ttUnk13 = 0.12f;
		uint8_t ttUnk14 = 3;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKTriangularTurtleCpnt : CKMRSubclass<CKTriangularTurtleCpnt, CKTurtleCpnt, 28> {
		void reflectMembers2(MemberListener& r, KEnvironment* kenv) { CKTurtleCpnt::reflectMembers2(r, kenv); }
	};
	struct CKSquareTurtleCpnt : CKMRSubclass<CKSquareTurtleCpnt, CKTurtleCpnt, 29> {
		void reflectMembers2(MemberListener& r, KEnvironment* kenv) { CKTurtleCpnt::reflectMembers2(r, kenv); }
	};
	struct CKDonutTurtleCpnt : CKMRSubclass<CKDonutTurtleCpnt, CKTurtleCpnt, 30> {
		void reflectMembers2(MemberListener& r, KEnvironment* kenv) { CKTurtleCpnt::reflectMembers2(r, kenv); }
	};
	struct CKPyramidalTurtleCpnt : CKMRSubclass<CKPyramidalTurtleCpnt, CKTurtleCpnt, 31> {
		void reflectMembers2(MemberListener& r, KEnvironment* kenv) { CKTurtleCpnt::reflectMembers2(r, kenv); }
	};

};