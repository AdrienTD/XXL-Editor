#pragma once

#include "CKHook.h"
#include "CKGroup.h"
#include "CKUtils.h"

namespace GameX2 {
	struct CKHkShoppingArea : CKPartlyUnknown<CKHook, 9> {};
	struct CKHkBonusSpitter : CKPartlyUnknown<CKHook, 14> {};
	struct CKHkPressionStone : CKPartlyUnknown<CKHook, 21> {};
	struct CKHkActivator : CKPartlyUnknown<CKHook, 52> {};
	struct CKHkDoor : CKPartlyUnknown<CKHook, 101> {};
	struct CKHkCrumblyZone : CKPartlyUnknown<CKHook, 102> {};
	struct CKHkCrate : CKPartlyUnknown<CKHook, 112> {};
	struct CKHkBasicBonus : CKPartlyUnknown<CKHook, 114> {};
	struct CKHkTelepher : CKPartlyUnknown<CKHook, 158> {};
	struct CKHkTelepherTowed : CKPartlyUnknown<CKHook, 159> {};
	struct CKHkA2JetPackEnemy : CKPartlyUnknown<CKHook, 167> {};
	struct CKHkMovableCharacter : CKPartlyUnknown<CKHook, 208> {};
	struct CKHkA2Hero : CKPartlyUnknown<CKHook, 218> {};
	struct CKHkLockMachineGun : CKPartlyUnknown<CKHook, 220> {};
	struct CKHkA2PotionStone : CKPartlyUnknown<CKHook, 227> {};
	struct CKHkA2Enemy : CKPartlyUnknown<CKHook, 228> {};
	struct CKHkCrumblyZoneAnimated : CKPartlyUnknown<CKHook, 230> {};
	struct CKHkDynamicObject : CKPartlyUnknown<CKHook, 231> {};
	struct CKHkPlatform : CKPartlyUnknown<CKHook, 233> {};
	struct CKHkWeatherCenter : CKPartlyUnknown<CKHook, 234> {};
	struct CKHkEnemyTarget : CKPartlyUnknown<CKHook, 235> {};
	struct CKHkEnemyTargetPit : CKPartlyUnknown<CKHook, 236> {};
	struct CKHkWaterWork : CKPartlyUnknown<CKHook, 237> {};
	struct CKHkSwitch : CKPartlyUnknown<CKHook, 238> {};
	struct CKHkCounter : CKPartlyUnknown<CKHook, 241> {};
	struct CKHkA2InvincibleEnemy : CKPartlyUnknown<CKHook, 244> {};
	struct CKHkCorridorEnemy : CKPartlyUnknown<CKHook, 245> {};
	struct CKHkTelepherAuto : CKPartlyUnknown<CKHook, 246> {};
	struct CKHkA2ArcherEnemy : CKPartlyUnknown<CKHook, 247> {};
	struct CKHkPushBomb : CKPartlyUnknown<CKHook, 249> {};
	struct CKHkMovableBloc : CKPartlyUnknown<CKHook, 250> {};
	struct CKHkParticlesSequencer : CKPartlyUnknown<CKHook, 251> {};
	struct CKHkA2TurtleEnemy : CKPartlyUnknown<CKHook, 252> {};
	struct CKHkCatapult : CKPartlyUnknown<CKHook, 253> {};
	struct CKHkA2Boss : CKPartlyUnknown<CKHook, 254> {};
	struct CKHkRollingBarrel : CKPartlyUnknown<CKHook, 256> {};
	struct CKHkFoldawayBridge : CKPartlyUnknown<CKHook, 257> {};
	struct CKHkBumper : CKPartlyUnknown<CKHook, 258> {};
	struct CKHkToll : CKPartlyUnknown<CKHook, 259> {};
	struct CKHkSlotMachine : CKPartlyUnknown<CKHook, 260> {};
	struct CKHkA2BossTrap : CKPartlyUnknown<CKHook, 261> {};
	struct CKHkCheckPoint : CKPartlyUnknown<CKHook, 262> {};
	struct CKHkA2CrumblyZone : CKPartlyUnknown<CKHook, 263> {};
	struct CKHkA2MarioEnemy : CKPartlyUnknown<CKHook, 265> {};
	struct CKHkA2DeathFx : CKPartlyUnknown<CKHook, 266> {};
	struct CKHkBonusHolder : CKPartlyUnknown<CKHook, 268> {};

	//struct CKGroupRoot : CKPartlyUnknown<CKGroup, 1> {};
	struct CKGrpA2Boss : CKPartlyUnknown<CKGroup, 4> {};
	struct CKGrpMeca : CKPartlyUnknown<CKGroup, 11> {};
	struct CKGrpSquad : CKPartlyUnknown<CKGroup, 24> {};
	struct CKGrpPoolSquad : CKPartlyUnknown<CKGroup, 44> {};
	struct CKGrpCrate : CKPartlyUnknown<CKGroup, 60> {};
	struct CKGrpBonusPool : CKPartlyUnknown<CKGroup, 61> {};
	struct CKGrpA2Hero : CKPartlyUnknown<CKGroup, 86> {};
	struct CKGrpA2LevelPotion : CKPartlyUnknown<CKGroup, 88> {};
	struct CKGrpLevelManager : CKSubclass<CKGroup, 89> {};
	struct CKGrpA2BonusPool : CKPartlyUnknown<CKGroup, 91> {};
	//struct CKGrpBonus : CKPartlyUnknown<CKGroup, 92> {};
	struct CKGrpA2Enemy : CKPartlyUnknown<CKGroup, 94> {};
	struct CKGrpFightZone : CKPartlyUnknown<CKGroup, 95> {};
	struct CKGrpMecaLast : CKSubclass<CKGroup, 98> {};
	struct CKCommonBaseGroup : CKPartlyUnknown<CKGroup, 99> {};
	struct CKFightZoneSectorGrpRoot : CKSubclass<CKGroup, 100> {};
	struct CKGrpA2FoodBasket : CKPartlyUnknown<CKGroup, 103> {};
}