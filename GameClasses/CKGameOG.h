#pragma once

#include "CKHook.h"
#include "CKGroup.h"
#include "CKComponent.h"
#include "CKUtils.h"

struct CAnimationDictionary;
struct CNode;
struct CSGHotSpot;
struct CKSoundDictionaryID;
struct CParticlesNodeFx;
struct CTrailNodeFx;

using CKQuakeCpnt = CKObject;
using CKQuakeCpntUpdater = CKObject;
using CKProjectileAccessor = CKObject;

namespace GameOG {
	struct CKHkLightRay : CKPartlyUnknown<CKHook, 3> {};
	struct CKHkLightReceptacle : CKPartlyUnknown<CKHook, 4> {};
	struct CKHkFixedMirror : CKPartlyUnknown<CKHook, 5> {};
	struct CKHkOrientableMirror : CKPartlyUnknown<CKHook, 8> {};
	struct CKHkPushStack : CKPartlyUnknown<CKHook, 12> {};
	struct CKHkPressionStoneGroup : CKPartlyUnknown<CKHook, 13> {};
	//struct CKHkPressionStone : CKPartlyUnknown<CKHook, 21> {};
	struct CKCameraBeacon : CKPartlyUnknown<CKHook, 22> {};
	struct CKHkSavePoint : CKPartlyUnknown<CKHook, 24> {};
	//struct CKHkA3Enemy : CKPartlyUnknown<CKHook, 25> {};
	struct CKHkBar : CKPartlyUnknown<CKHook, 29> {};
	struct CKHkPushObstacle : CKPartlyUnknown<CKHook, 33> {};
	struct CKHkPushStackType1 : CKPartlyUnknown<CKHook, 35> {};
	struct CKHkPushStackType2 : CKPartlyUnknown<CKHook, 36> {};
	struct CKHkButterflyBridge : CKPartlyUnknown<CKHook, 45> {};
	struct CKHkPushKillObject : CKPartlyUnknown<CKHook, 53> {};
	struct CKHkTrackerManager : CKPartlyUnknown<CKHook, 61> {};
	struct CKHkLinkedBeacon : CKPartlyUnknown<CKHook, 64> {};
	struct CKHkA3BurriedBonus : CKPartlyUnknown<CKHook, 67> {};
	struct CKHkScrapShower : CKPartlyUnknown<CKHook, 73> {};
	struct CKHkA3Compass : CKPartlyUnknown<CKHook, 77> {};
	struct CKHkLedge : CKPartlyUnknown<CKHook, 86> {};
	struct CKHkHeroBall : CKPartlyUnknown<CKHook, 88> {};
	struct CKHkHeroBallSpawner : CKPartlyUnknown<CKHook, 91> {};
	//struct CKHkDoor : CKPartlyUnknown<CKHook, 101> {};
	//struct CKHkCrumblyZone : CKPartlyUnknown<CKHook, 102> {};
	struct CKHkHeroActivator : CKPartlyUnknown<CKHook, 104> {};
	struct CKHkPxObject : CKPartlyUnknown<CKHook, 107> {};
	//struct CKHkBasicBonus : CKPartlyUnknown<CKHook, 114> {};
	struct CKHkA3Hero : CKPartlyUnknown<CKHook, 117> {};
	struct CKHkA3StepPlatformAirlock : CKPartlyUnknown<CKHook, 118> {};
	struct CKHkA3BirdZone : CKPartlyUnknown<CKHook, 119> {};
	struct CKHkA3Bird : CKPartlyUnknown<CKHook, 120> {};
	struct CKHkA3BirdCage : CKPartlyUnknown<CKHook, 121> {};
	struct CKHkEnemyTarget : CKPartlyUnknown<CKHook, 122> {};
	struct CKHkA3SoundActivator : CKPartlyUnknown<CKHook, 124> {};
	struct CKHkCounterWithDisplay : CKPartlyUnknown<CKHook, 126> {};
	struct CKHkA3PotionSpawner : CKPartlyUnknown<CKHook, 127> {};
	struct CKHkOlympicGameFrog : CKPartlyUnknown<CKHook, 128> {};
	struct CKHkFrogBall : CKPartlyUnknown<CKHook, 129> {};
	struct CKHkCrapombeBall : CKPartlyUnknown<CKHook, 130> {};
	struct CKHkBonusDistributor : CKPartlyUnknown<CKHook, 131> {};
	struct CKHkOlympicGameChar : CKPartlyUnknown<CKHook, 133> {};
	struct CKHkChar : CKPartlyUnknown<CKHook, 134> {};
	struct CKHkOlympicGameArena : CKPartlyUnknown<CKHook, 135> {};
	struct CKHkLaunchQuakeCamera : CKPartlyUnknown<CKHook, 136> {};
	struct CKHkOlympicGameJump : CKPartlyUnknown<CKHook, 137> {};
	struct CKHkMoss : CKPartlyUnknown<CKHook, 138> {};
	struct CKHkA3Moss : CKPartlyUnknown<CKHook, 139> {};
	struct CKHkOlympicGameJavelin : CKPartlyUnknown<CKHook, 140> {};
	struct CKHkOlympicGameRope : CKPartlyUnknown<CKHook, 141> {};
	struct CKHkOlympicGameHammer : CKPartlyUnknown<CKHook, 142> {};
	struct CKHkOlympicGameRace : CKPartlyUnknown<CKHook, 143> {};
	struct CKHkOlympicGameMusicalFight : CKPartlyUnknown<CKHook, 144> {};
	struct CKHkA3SeparationManager : CKPartlyUnknown<CKHook, 145> {};
	struct CKHkA3DDR : CKPartlyUnknown<CKHook, 146> {};
	struct CKHkA3PassManager : CKPartlyUnknown<CKHook, 149> {};
	//struct CKHkTelepher : CKPartlyUnknown<CKHook, 158> {};
	//struct CKHkTelepherTowed : CKPartlyUnknown<CKHook, 159> {};
	//struct CKHkMovableCharacter : CKPartlyUnknown<CKHook, 208> {};
	//struct CKHkCrumblyZoneAnimated : CKPartlyUnknown<CKHook, 230> {};
	//struct CKHkDynamicObject : CKPartlyUnknown<CKHook, 231> {};
	//struct CKHkPlatform : CKPartlyUnknown<CKHook, 233> {};
	//struct CKHkWeatherCenter : CKPartlyUnknown<CKHook, 234> {};
	//struct CKHkParticlesSequencer : CKPartlyUnknown<CKHook, 251> {};
	//struct CKHkRollingBarrel : CKPartlyUnknown<CKHook, 256> {};
	//struct CKHkFoldawayBridge : CKPartlyUnknown<CKHook, 257> {};

	//struct CKGroupRoot : CKPartlyUnknown<CKGroup, 1> {};
	struct CKGrpStorage : CKPartlyUnknown<CKGroup, 8> {};
	struct CKGrpPushStack : CKPartlyUnknown<CKGroup, 9> {};
	struct CKGrpA3BurriedBonus : CKPartlyUnknown<CKGroup, 16> {};
	//struct CKGrpSquad : CKPartlyUnknown<CKGroup, 24> {};
	//struct CKGrpA3BonusPool : CKPartlyUnknown<CKGroup, 31> {};
	struct CKGrpA3Hero : CKPartlyUnknown<CKGroup, 32> {};
	struct CKGrpA3Meca : CKPartlyUnknown<CKGroup, 33> {};
	struct CKGrpA3BirdCage : CKPartlyUnknown<CKGroup, 34> {};
	struct CKGrpA3LevelPotion : CKPartlyUnknown<CKGroup, 35> {};
	//struct CKGrpPoolSquad : CKPartlyUnknown<CKGroup, 44> {};
	//struct CKGrpBonusPool : CKPartlyUnknown<CKGroup, 61> {};
	//struct CKGrpLevelManager : CKPartlyUnknown<CKGroup, 89> {};
	//struct CKGrpBonus : CKPartlyUnknown<CKGroup, 92> {};
	//struct CKGrpA3Enemy : CKPartlyUnknown<CKGroup, 94> {};
	//struct CKGrpFightZone : CKPartlyUnknown<CKGroup, 95> {};
	//struct CKGrpMecaLast : CKPartlyUnknown<CKGroup, 98> {};
	//struct CKCommonBaseGroup : CKPartlyUnknown<CKGroup, 99> {};
	//struct CKFightZoneSectorGrpRoot : CKPartlyUnknown<CKGroup, 100> {};

	struct CKEnemySectorCpnt;

	struct CKHkA3Enemy : CKMRSubclass<CKHkA3Enemy, CKHook, 25> {
		//uint8_t ckhaeUnk0;
		std::vector<kobjref<CKEnemySectorCpnt>> ckhaeEnemySectorCpnts;
		uint8_t ckhaeUnk3 = 1;
		uint16_t ckhaeUnk4 = 1;
		//uint8_t ckhaeUnk5;
		std::vector<uint16_t> ckhaeUnk6;
		uint8_t ckhaeUnk7 = 0;
		//uint8_t ckhaeUnk8;
		std::vector<kobjref<CKSceneNode>> ckhaeBoundingShapes;
		uint8_t ckhaeUnk19 = 4;
		std::array<uint8_t, 4> ckhaeUnk20;
		std::array<uint8_t, 4> ckhaeUnk21;
		std::array<uint8_t, 20> ckhaeUnk22;
		//uint8_t ckhaeUnk23;
		struct Thing1 {
			// divided in 3 segments, each stored in its own vector in the game
			uint16_t ckhaeUnk24;
			uint8_t ckhaeUnk25;
			uint8_t ckhaeUnk26;
			uint8_t ckhaeUnk27;
			//
			float ckhaeUnk32 = 1.0f;
			//
			Vector3 ckhaeUnk33;
		};
		std::vector<Thing1> ckhaeThings1;
		//uint8_t ckhaeUnk34;
		std::vector<kobjref<CParticlesNodeFx>> ckhaeParticleNodes;
		std::array<float, 3> ckhaeUnk41;
		//uint8_t ckhaeUnk42;
		std::vector<kobjref<CTrailNodeFx>> ckhaeTrailNodes;
		uint8_t ckhaeUnk46;
		uint8_t ckhaeUnk47;
		kobjref<CKObject> ckhaeUnk48;
		kobjref<CKObject> ckhaeUnk49;
		Vector3 ckhaeUnk50;
		// [ composed class (TODO which one?)
		float ckhaeUnk51;
		int32_t ckhaeUnk52;
		float ckhaeUnk53;
		float ckhaeUnk54;
		float ckhaeUnk55;
		float ckhaeUnk56;
		// ]
		float ckhaeUnk57;
		float ckhaeUnk58;
		kobjref<CKObject> ckhaeUnk59;
		kobjref<CKShadowCpnt> ckhaeUnk60;
		//uint8_t ckhaeUnk61;
		std::vector<Vector3> ckhaeUnk62;
		// ^ base, v subclass
		kobjref<CKQuakeCpntUpdater> ckhaeUnk63;
		kobjref<CKCameraBeacon> ckhaeUnk64;
		std::array<float, 9> ckhaeUnk65;
		int32_t ckhaeUnk66;
		std::string ckhaeUnkString;
		kobjref<CKProjectileAccessor> ckhaeUnk69;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKEnemySectorCpnt : CKMRSubclass<CKEnemySectorCpnt, CKReflectableComponent, 14> {
		uint8_t numModels = 0;
		std::vector<kobjref<CKSceneNode>> ckescSceneNodes;
		std::vector<KPostponedRef<CAnimationDictionary>> ckescAnimDicts;
		std::vector<kobjref<CKObject>> ckescBlenderControllers;
		//uint8_t ckescUnk4;
		std::vector<KPostponedRef<CNode>> ckescUnk5;
		//uint8_t ckescUnk12;
		std::vector<kobjref<CSGHotSpot>> ckescUnk13;
		//uint8_t ckescUnk32 = 0;
		std::vector<kobjref<CKObject>> ckescUnused1;
		KPostponedRef<CKSoundDictionaryID> ckescSoundDict;
		uint8_t ckescNumDunno = 0;
		std::vector<kobjref<CKObject>> ckescUnused2;
		std::vector<kobjref<CKObject>> ckescUnused3;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};
}