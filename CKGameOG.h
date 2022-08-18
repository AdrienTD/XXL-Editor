#pragma once

#include "CKHook.h"
#include "CKGroup.h"
#include "CKUtils.h"

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
	struct CKHkA3Enemy : CKPartlyUnknown<CKHook, 25> {};
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
}