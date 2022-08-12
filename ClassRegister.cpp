#include "ClassRegister.h"
#include "KEnvironment.h"

#include "CKManager.h"
#include "CKHook.h"
#include "CKGroup.h"
#include "CKComponent.h"
#include "CKCamera.h"
#include "CKCinematicNode.h"
#include "CKDictionary.h"
#include "CKGeometry.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKGraphical.h"

void ClassRegister::registerClassesForXXL1PC(KEnvironment& kenv)
{
	// XXL1 PC/GC Original+Romaster

	kenv.addFactory<CKServiceManager>();
	kenv.addFactory<CKGraphic>();
	kenv.addFactory<CKSoundManager>();

	kenv.addFactory<CKServiceLife>();
	kenv.addFactory<CKSrvCollision>();
	kenv.addFactory<CKSrvCamera>();
	kenv.addFactory<CKSrvCinematic>();
	kenv.addFactory<CKSrvEvent>();
	kenv.addFactory<CKSrvPathFinding>();
	kenv.addFactory<CKSrvDetector>();
	kenv.addFactory<CKSrvMarker>();
	kenv.addFactory<CKSrvAvoidance>();
	kenv.addFactory<CKSrvSekensor>();
	kenv.addFactory<CKSrvBeacon>();
	kenv.addFactory<CKSrvShadow>();
	kenv.addFactory<CKSrvProjectiles>();
	kenv.addFactory<CKSrvFx>();

	kenv.addFactory<CKHkPressionStone>();
	kenv.addFactory<CKHkAsterix>();
	kenv.addFactory<CKHkObelix>();
	kenv.addFactory<CKHkIdefix>();
	kenv.addFactory<CKHkMachinegun>();
	kenv.addFactory<CKHkTorch>();
	kenv.addFactory<CKHkHearth>();
	kenv.addFactory<CKHkDrawbridge>();
	kenv.addFactory<CKHkMegaAshtray>();
	kenv.addFactory<CKHkBoat>();
	kenv.addFactory<CKHkCorkscrew>();
	kenv.addFactory<CKHkTurnstile>();
	kenv.addFactory<CKHkLifter>();
	kenv.addFactory<CKHkActivator>();
	kenv.addFactory<CKHkRotaryBeam>();
	kenv.addFactory<CKHkLightPillar>();
	kenv.addFactory<CKHkWind>();
	kenv.addFactory<CKHkJumpingRoman>();
	kenv.addFactory<CKHkWaterJet>();
	kenv.addFactory<CKHkPowderKeg>();
	kenv.addFactory<CKHkTriangularTurtle>();
	kenv.addFactory<CKHkBasicEnemy>();
	kenv.addFactory<CKHkRomanArcher>();
	kenv.addFactory<CKHkAnimatedCharacter>();
	kenv.addFactory<CKHkSwingDoor>();
	kenv.addFactory<CKHkSlideDoor>();
	kenv.addFactory<CKHkCrumblyZone>();
	kenv.addFactory<CKHkHelmetCage>();
	kenv.addFactory<CKHkSquareTurtle>();
	kenv.addFactory<CKHkTeleBridge>();
	kenv.addFactory<CKHkCrate>();
	kenv.addFactory<CKHkBasicBonus>();
	kenv.addFactory<CKHkDonutTurtle>();
	kenv.addFactory<CKHkPyramidalTurtle>();
	kenv.addFactory<CKHkRollingStone>();
	kenv.addFactory<CKHkInterfaceBase>();
	kenv.addFactory<CKHkInterfaceEvolution>();
	kenv.addFactory<CKHkCatapult>();
	kenv.addFactory<CKHkInterfacePause>();
	kenv.addFactory<CKHkInterfaceInGame>();
	kenv.addFactory<CKHkInterfaceOption>();
	kenv.addFactory<CKHkInterfaceMain>();
	kenv.addFactory<CKHkInterfaceLoadSave>();
	kenv.addFactory<CKHkInterfaceCloth>();
	kenv.addFactory<CKHkInterfaceShop>();
	kenv.addFactory<CKHkPushPullAsterix>();
	kenv.addFactory<CKHkBasicEnemyLeader>();
	kenv.addFactory<CKHkTelepher>();
	kenv.addFactory<CKHkTowedTelepher>();
	kenv.addFactory<CKHkBumper>();
	kenv.addFactory<CKHkClueMan>();
	kenv.addFactory<CKHkSky>();
	kenv.addFactory<CKHkRocketRoman>();
	kenv.addFactory<CKHkJetPackRoman>();
	kenv.addFactory<CKHkWildBoar>();
	kenv.addFactory<CKHkAsterixShop>();
	kenv.addFactory<CKHkWater>();
	kenv.addFactory<CKHkMobileTower>();
	kenv.addFactory<CKHkBoss>();
	kenv.addFactory<CKHkInterfaceDemo>();
	kenv.addFactory<CKHkWaterFx>();
	kenv.addFactory<CKHkHighGrass>();
	kenv.addFactory<CKHkWaterFall>();
	kenv.addFactory<CKHkInterfaceGallery>();
	kenv.addFactory<CKHkTrioCatapult>();
	kenv.addFactory<CKHkObelixCatapult>();
	kenv.addFactory<CKHkInterfaceOpening>();
	kenv.addFactory<CKHkAsterixCheckpoint>();
	kenv.addFactory<CKHkBonusSpitter>();
	kenv.addFactory<CKHkLight>();
	kenv.addFactory<CKHkParkourSteleAsterix>();

	kenv.addFactory<CKHkAsterixLife>();
	kenv.addFactory<CKHkBoatLife>();
	kenv.addFactory<CKHkObelixLife>();
	kenv.addFactory<CKHkMecaLife>();
	kenv.addFactory<CKHkIdefixLife>();
	kenv.addFactory<CKHkEnemyLife>();
	kenv.addFactory<CKHkTriangularTurtleLife>();
	kenv.addFactory<CKHkAnimatedCharacterLife>();
	kenv.addFactory<CKHkSquareTurtleLife>();
	kenv.addFactory<CKHkDonutTurtleLife>();
	kenv.addFactory<CKHkPyramidalTurtleLife>();
	kenv.addFactory<CKHkCatapultLife>();
	kenv.addFactory<CKHkSkyLife>();
	kenv.addFactory<CKHkWaterLife>();
	kenv.addFactory<CKHkBossLife>();
	kenv.addFactory<CKHkWaterFxLife>();
	kenv.addFactory<CKHkAsterixCheckpointLife>();
	kenv.addFactory<CKHkWaterFallLife>();

	kenv.addFactory<CKGroupRoot>();
	kenv.addFactory<CKGrpMeca>();
	kenv.addFactory<CKGrpTrio>();
	kenv.addFactory<CKGrpBoat>();
	kenv.addFactory<CKGrpSquadEnemy>();
	kenv.addFactory<CKGrpEnemy>();
	kenv.addFactory<CKGrpPoolSquad>();
	kenv.addFactory<CKGrpWalkingCharacter>();
	kenv.addFactory<CKGrpBonus>();
	kenv.addFactory<CKGrpFrontEnd>();
	kenv.addFactory<CKGrpCatapult>();
	kenv.addFactory<CKGrpMap>();
	kenv.addFactory<CKGrpStorageStd>();
	kenv.addFactory<CKGrpCrate>();
	kenv.addFactory<CKGrpBonusPool>();
	kenv.addFactory<CKGrpAsterixBonusPool>();
	kenv.addFactory<CKGrpSquadJetPack>();
	kenv.addFactory<CKGrpWildBoarPool>();
	kenv.addFactory<CKGrpAsterixCheckpoint>();
	kenv.addFactory<CKGrpBonusSpitter>();
	kenv.addFactory<CKGrpLight>();

	kenv.addFactory<CKGrpTrioLife>();
	kenv.addFactory<CKGrpMecaLife>();
	kenv.addFactory<CKGrpBonusLife>();
	kenv.addFactory<CKGrpMapLife>();
	kenv.addFactory<CKGrpEnemyLife>();
	kenv.addFactory<CKGrpAsterixCheckpointLife>();

	kenv.addFactory<CKGrpMecaCpntAsterix>();
	kenv.addFactory<CKCrateCpnt>();
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.addFactory<CKBasicEnemyLeaderCpnt>();
	kenv.addFactory<CKJumpingRomanCpnt>();
	kenv.addFactory<CKRomanArcherCpnt>();
	kenv.addFactory<CKShadowCpnt>();
	kenv.addFactory<CKRocketRomanCpnt>();
	kenv.addFactory<CKBonusCpnt>();
	kenv.addFactory<CKJetPackRomanCpnt>();
	kenv.addFactory<CKWildBoarCpnt>();
	kenv.addFactory<CKMobileTowerCpnt>();
	kenv.addFactory<CKTriangularTurtleCpnt>();
	kenv.addFactory<CKSquareTurtleCpnt>();
	kenv.addFactory<CKDonutTurtleCpnt>();
	kenv.addFactory<CKPyramidalTurtleCpnt>();

	kenv.addFactory<CKCamera>();
	kenv.addFactory<CKCameraRigidTrack>();
	kenv.addFactory<CKCameraClassicTrack>();
	kenv.addFactory<CKCameraPathTrack>();
	kenv.addFactory<CKCameraFixTrack>();
	kenv.addFactory<CKCameraAxisTrack>();
	kenv.addFactory<CKCameraSpyTrack>();
	kenv.addFactory<CKCameraPassivePathTrack>();

	kenv.addFactory<CKLogicalAnd>();
	kenv.addFactory<CKLogicalOr>();
	kenv.addFactory<CKPlayAnimCinematicBloc>();
	kenv.addFactory<CKPathFindingCinematicBloc>();
	kenv.addFactory<CKFlaggedPathCinematicBloc>();
	kenv.addFactory<CKGroupBlocCinematicBloc>();
	kenv.addFactory<CKAttachObjectsCinematicBloc>();
	kenv.addFactory<CKStreamCinematicBloc>();
	kenv.addFactory<CKRandLogicalDoor>();
	kenv.addFactory<CKParticleCinematicBloc>();
	kenv.addFactory<CKStreamAloneCinematicBloc>();
	kenv.addFactory<CKStreamGroupBlocCinematicBloc>();
	kenv.addFactory<CKManageEventCinematicBloc>();
	kenv.addFactory<CKManagerEventStopCinematicBloc>();
	kenv.addFactory<CKStartDoor>();
	kenv.addFactory<CKSekensorCinematicBloc>();
	kenv.addFactory<CKDisplayPictureCinematicBloc>();
	kenv.addFactory<CKManageCameraCinematicBloc>();
	kenv.addFactory<CKStartEventCinematicBloc>();
	kenv.addFactory<CKSkyCinematicBloc>();
	kenv.addFactory<CKLightningCinematicBloc>();
	kenv.addFactory<CKPlaySoundCinematicBloc>();
	kenv.addFactory<CKPauseCinematicBloc>();
	kenv.addFactory<CKTeleportCinematicBloc>();
	kenv.addFactory<CKEndDoor>();

	kenv.addFactory<CTextureDictionary>();
	kenv.addFactory<CAnimationDictionary>();
	kenv.addFactory<CKSoundDictionary>();
	kenv.addFactory<CKSoundDictionaryID>();

	kenv.addFactory<CKParticleGeometry>();
	kenv.addFactory<CKGeometry>();
	kenv.addFactory<CKSkinGeometry>();

	kenv.addFactory<CSGRootNode>();
	kenv.addFactory<CSGSectorRoot>();
	kenv.addFactory<CNode>();
	kenv.addFactory<CKDynBSphereProjectile>();
	kenv.addFactory<CSGBranch>();
	kenv.addFactory<CGlowNodeFx>();
	kenv.addFactory<CClone>();
	kenv.addFactory<CKBoundingSphere>();
	kenv.addFactory<CKDynamicBoundingSphere>();
	kenv.addFactory<CKAABB>();
	kenv.addFactory<CKOBB>();
	kenv.addFactory<CParticlesNodeFx>();
	kenv.addFactory<CAnimatedNode>();
	kenv.addFactory<CAnimatedClone>();
	kenv.addFactory<CKAACylinder>();
	kenv.addFactory<CSkyNodeFx>();
	kenv.addFactory<CFogBoxNodeFx>();
	kenv.addFactory<CTrailNodeFx>();

	kenv.addFactory<CKPFGraphTransition>();
	kenv.addFactory<CKBundle>();
	kenv.addFactory<CKSector>();
	kenv.addFactory<CKLevel>();
	kenv.addFactory<CKCameraSector>();
	kenv.addFactory<CKCoreManager>();
	kenv.addFactory<CKSpline4>();
	kenv.addFactory<CKChoreoKey>();
	kenv.addFactory<CKPFGraphNode>();
	kenv.addFactory<CKSas>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CDynamicGround>();
	kenv.addFactory<CWall>();
	kenv.addFactory<CKFlaggedPath>();
	kenv.addFactory<CKMsgAction>();
	kenv.addFactory<CKChoreography>();
	kenv.addFactory<CKLine>();
	kenv.addFactory<CKSpline4L>();
	kenv.addFactory<CKCinematicScene>();
	kenv.addFactory<CKCinematicSceneData>();
	kenv.addFactory<CKDefaultGameManager>();
	kenv.addFactory<CKAsterixGameManager>();
	kenv.addFactory<CKAsterixSlideFP>();
	kenv.addFactory<CLocManager>();
	kenv.addFactory<CKSekens>();
	kenv.addFactory<CKMeshKluster>();
	kenv.addFactory<CKBeaconKluster>();
	kenv.addFactory<CKProjectileTypeScrap>();
	kenv.addFactory<CKProjectileTypeAsterixBomb>();
	kenv.addFactory<CKProjectileTypeBallisticPFX>();
	kenv.addFactory<CKFlashNode2dFx>();
	kenv.addFactory<CKElectricArcNodeFx>();
	kenv.addFactory<CKQuadNodeFx>();
	kenv.addFactory<CKLightningObjectNodeFx>();
	kenv.addFactory<CKFilterNode2dFx>();
	kenv.addFactory<CKExplosionNodeFx>();

	kenv.addFactory<CCloneManager>();
	kenv.addFactory<CAnimationManager>();
	kenv.addFactory<CManager2d>();
	kenv.addFactory<CMenuManager>();
	kenv.addFactory<CContainer2d>();
	kenv.addFactory<CScene2d>();
	kenv.addFactory<CMessageBox2d>();
	kenv.addFactory<CText2d>();
	kenv.addFactory<CColorTextButton2d>();
	kenv.addFactory<CBillboard2d>();
	kenv.addFactory<CBillboard2dList>();
	kenv.addFactory<CBillboardButton2d>();
}

void ClassRegister::registerClassesForXXL1Console(KEnvironment& kenv)
{
	// XXL1 PS2

	kenv.addFactory<CKServiceManager>();

	kenv.addFactory<CKSrvCollision>();
	kenv.addFactory<CKSrvCinematic>();
	//kenv.addFactory<CKSrvEvent>();
	kenv.addFactory<CKSrvPathFinding>();
	kenv.addFactory<CKSrvDetector>();
	kenv.addFactory<CKSrvMarker>();
	kenv.addFactory<CKSrvBeacon>();
	kenv.addFactory<CKSrvShadow>();
	kenv.addFactory<CKSrvFx>();

	kenv.addFactory<CKHkPressionStone>();
	kenv.addFactory<CKHkAsterix>();
	kenv.addFactory<CKHkObelix>();
	kenv.addFactory<CKHkIdefix>();
	kenv.addFactory<CKHkMachinegun>();
	kenv.addFactory<CKHkTorch>();
	kenv.addFactory<CKHkHearth>();
	kenv.addFactory<CKHkDrawbridge>();
	kenv.addFactory<CKHkMegaAshtray>();
	kenv.addFactory<CKHkBoat>();
	kenv.addFactory<CKHkCorkscrew>();
	kenv.addFactory<CKHkTurnstile>();
	kenv.addFactory<CKHkLifter>();
	kenv.addFactory<CKHkActivator>();
	kenv.addFactory<CKHkRotaryBeam>();
	kenv.addFactory<CKHkLightPillar>();
	kenv.addFactory<CKHkWind>();
	kenv.addFactory<CKHkJumpingRoman>();
	kenv.addFactory<CKHkWaterJet>();
	kenv.addFactory<CKHkPowderKeg>();
	kenv.addFactory<CKHkTriangularTurtle>();
	kenv.addFactory<CKHkBasicEnemy>();
	kenv.addFactory<CKHkRomanArcher>();
	kenv.addFactory<CKHkAnimatedCharacter>();
	kenv.addFactory<CKHkSwingDoor>();
	kenv.addFactory<CKHkSlideDoor>();
	kenv.addFactory<CKHkCrumblyZone>();
	kenv.addFactory<CKHkHelmetCage>();
	kenv.addFactory<CKHkSquareTurtle>();
	kenv.addFactory<CKHkTeleBridge>();
	kenv.addFactory<CKHkCrate>();
	kenv.addFactory<CKHkBasicBonus>();
	kenv.addFactory<CKHkDonutTurtle>();
	kenv.addFactory<CKHkPyramidalTurtle>();
	kenv.addFactory<CKHkRollingStone>();
	kenv.addFactory<CKHkInterfaceBase>();
	kenv.addFactory<CKHkInterfaceEvolution>();
	kenv.addFactory<CKHkCatapult>();
	kenv.addFactory<CKHkInterfacePause>();
	kenv.addFactory<CKHkInterfaceInGame>();
	kenv.addFactory<CKHkInterfaceOption>();
	kenv.addFactory<CKHkInterfaceMain>();
	kenv.addFactory<CKHkInterfaceLoadSave>();
	kenv.addFactory<CKHkInterfaceCloth>();
	kenv.addFactory<CKHkInterfaceShop>();
	kenv.addFactory<CKHkPushPullAsterix>();
	kenv.addFactory<CKHkBasicEnemyLeader>();
	kenv.addFactory<CKHkTelepher>();
	kenv.addFactory<CKHkTowedTelepher>();
	kenv.addFactory<CKHkBumper>();
	kenv.addFactory<CKHkClueMan>();
	kenv.addFactory<CKHkSky>();
	kenv.addFactory<CKHkRocketRoman>();
	kenv.addFactory<CKHkJetPackRoman>();
	kenv.addFactory<CKHkWildBoar>();
	kenv.addFactory<CKHkAsterixShop>();
	kenv.addFactory<CKHkWater>();
	kenv.addFactory<CKHkMobileTower>();
	kenv.addFactory<CKHkBoss>();
	kenv.addFactory<CKHkInterfaceDemo>();
	kenv.addFactory<CKHkWaterFx>();
	kenv.addFactory<CKHkHighGrass>();
	kenv.addFactory<CKHkWaterFall>();
	kenv.addFactory<CKHkInterfaceGallery>();
	kenv.addFactory<CKHkTrioCatapult>();
	kenv.addFactory<CKHkObelixCatapult>();
	kenv.addFactory<CKHkInterfaceOpening>();
	kenv.addFactory<CKHkAsterixCheckpoint>();
	kenv.addFactory<CKHkBonusSpitter>();
	kenv.addFactory<CKHkLight>();

	kenv.addFactory<CKHkAsterixLife>();
	kenv.addFactory<CKHkBoatLife>();
	kenv.addFactory<CKHkObelixLife>();
	kenv.addFactory<CKHkMecaLife>();
	kenv.addFactory<CKHkIdefixLife>();
	kenv.addFactory<CKHkEnemyLife>();
	kenv.addFactory<CKHkTriangularTurtleLife>();
	kenv.addFactory<CKHkAnimatedCharacterLife>();
	kenv.addFactory<CKHkSquareTurtleLife>();
	kenv.addFactory<CKHkDonutTurtleLife>();
	kenv.addFactory<CKHkPyramidalTurtleLife>();
	kenv.addFactory<CKHkCatapultLife>();
	kenv.addFactory<CKHkSkyLife>();
	kenv.addFactory<CKHkWaterLife>();
	kenv.addFactory<CKHkBossLife>();
	kenv.addFactory<CKHkWaterFxLife>();
	kenv.addFactory<CKHkAsterixCheckpointLife>();
	kenv.addFactory<CKHkWaterFallLife>();

	kenv.addFactory<CKGroupRoot>();
	kenv.addFactory<CKGrpMeca>();
	kenv.addFactory<CKGrpTrio>();
	kenv.addFactory<CKGrpBoat>();
	kenv.addFactory<CKGrpSquadEnemy>();
	kenv.addFactory<CKGrpEnemy>();
	kenv.addFactory<CKGrpPoolSquad>();
	kenv.addFactory<CKGrpWalkingCharacter>();
	kenv.addFactory<CKGrpBonus>();
	kenv.addFactory<CKGrpFrontEnd>();
	kenv.addFactory<CKGrpCatapult>();
	kenv.addFactory<CKGrpMap>();
	kenv.addFactory<CKGrpStorageStd>();
	kenv.addFactory<CKGrpCrate>();
	kenv.addFactory<CKGrpBonusPool>();
	kenv.addFactory<CKGrpAsterixBonusPool>();
	kenv.addFactory<CKGrpSquadJetPack>();
	kenv.addFactory<CKGrpWildBoarPool>();
	kenv.addFactory<CKGrpAsterixCheckpoint>();
	kenv.addFactory<CKGrpBonusSpitter>();
	kenv.addFactory<CKGrpLight>();

	kenv.addFactory<CKGrpTrioLife>();
	kenv.addFactory<CKGrpMecaLife>();
	kenv.addFactory<CKGrpBonusLife>();
	kenv.addFactory<CKGrpMapLife>();
	kenv.addFactory<CKGrpEnemyLife>();
	kenv.addFactory<CKGrpAsterixCheckpointLife>();

	kenv.addFactory<CKCrateCpnt>();
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.addFactory<CKBasicEnemyLeaderCpnt>();
	kenv.addFactory<CKJumpingRomanCpnt>();
	kenv.addFactory<CKRomanArcherCpnt>();
	kenv.addFactory<CKShadowCpnt>();
	kenv.addFactory<CKRocketRomanCpnt>();
	kenv.addFactory<CKJetPackRomanCpnt>();
	kenv.addFactory<CKMobileTowerCpnt>();
	kenv.addFactory<CKTriangularTurtleCpnt>();
	kenv.addFactory<CKSquareTurtleCpnt>();
	kenv.addFactory<CKDonutTurtleCpnt>();
	kenv.addFactory<CKPyramidalTurtleCpnt>();

	//kenv.addFactory<CKCinematicBloc>();
	//kenv.addFactory<CKCinematicDoor>();
	kenv.addFactory<CKLogicalAnd>();
	kenv.addFactory<CKLogicalOr>();
	kenv.addFactory<CKPlayAnimCinematicBloc>();
	kenv.addFactory<CKPathFindingCinematicBloc>();
	kenv.addFactory<CKFlaggedPathCinematicBloc>();
	kenv.addFactory<CKGroupBlocCinematicBloc>();
	kenv.addFactory<CKAttachObjectsCinematicBloc>();
	kenv.addFactory<CKStreamCinematicBloc>();
	kenv.addFactory<CKRandLogicalDoor>();
	kenv.addFactory<CKParticleCinematicBloc>();
	kenv.addFactory<CKStreamAloneCinematicBloc>();
	kenv.addFactory<CKStreamGroupBlocCinematicBloc>();
	kenv.addFactory<CKManageEventCinematicBloc>();
	kenv.addFactory<CKManagerEventStopCinematicBloc>();
	kenv.addFactory<CKStartDoor>();
	kenv.addFactory<CKSekensorCinematicBloc>();
	kenv.addFactory<CKDisplayPictureCinematicBloc>();
	kenv.addFactory<CKManageCameraCinematicBloc>();
	kenv.addFactory<CKStartEventCinematicBloc>();
	kenv.addFactory<CKSkyCinematicBloc>();
	kenv.addFactory<CKLightningCinematicBloc>();
	kenv.addFactory<CKPlaySoundCinematicBloc>();

	kenv.addFactory<CAnimationDictionary>();
	kenv.addFactory<CKSoundDictionaryID>();

	kenv.addFactory<CKPFGraphTransition>();
	kenv.addFactory<CKBundle>();
	kenv.addFactory<CKSector>();
	kenv.addFactory<CKSpline4>();
	kenv.addFactory<CKChoreoKey>();
	kenv.addFactory<CKPFGraphNode>();
	kenv.addFactory<CKSas>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CDynamicGround>();
	kenv.addFactory<CKFlaggedPath>();
	kenv.addFactory<CKMsgAction>();
	kenv.addFactory<CKChoreography>();
	kenv.addFactory<CKLine>();
	kenv.addFactory<CKSpline4L>();
	kenv.addFactory<CKCinematicScene>();
	kenv.addFactory<CKCinematicSceneData>();
	kenv.addFactory<CKMeshKluster>();
	kenv.addFactory<CKBeaconKluster>();
	kenv.addFactory<CKFlashNode2dFx>();
	kenv.addFactory<CKElectricArcNodeFx>();
	kenv.addFactory<CKQuadNodeFx>();
	kenv.addFactory<CKLightningObjectNodeFx>();
	kenv.addFactory<CKFilterNode2dFx>();
	kenv.addFactory<CKExplosionNodeFx>();
}

void ClassRegister::registerClassesForXXL2PlusPC(KEnvironment& kenv)
{
	// XXL2+ PC

	kenv.addFactory<CKSrvCollision>();
	kenv.addFactory<CKSrvCamera>();
	kenv.addFactory<CKSrvCinematic>();
	kenv.addFactory<CKSrvPathFinding>();
	kenv.addFactory<CKSrvBeacon>();
	kenv.addFactory<CKSrvTrigger>();

	kenv.addFactory<CKHkBasicBonus>();

	kenv.addFactory<CKGrpSquadX2>();
	kenv.addFactory<CKGrpPoolSquad>();
	kenv.addFactory<CKGrpA2BonusPool>();
	kenv.addFactory<CKGrpBonusX2>();
	kenv.addFactory<CKGrpA3BonusPool>();

	kenv.addFactory<CKCrateCpnt>();

	kenv.addFactory<CKCamera>();
	kenv.addFactory<CKCameraRigidTrack>();
	//kenv.addFactory<CKCameraClassicTrack>();
	kenv.addFactory<CKCameraPathTrack>();
	kenv.addFactory<CKCameraFixTrack>();
	kenv.addFactory<CKCameraAxisTrack>();
	//kenv.addFactory<CKCameraSpyTrack>();
	kenv.addFactory<CKCameraPassivePathTrack>();
	kenv.addFactory<CKCameraBalistTrack>();
	kenv.addFactory<CKCameraClassicTrack2>();
	kenv.addFactory<CKCameraFirstPersonTrack>();

	kenv.addFactory<CKLogicalAnd>();
	kenv.addFactory<CKLogicalOr>();
	kenv.addFactory<CKPlayAnimCinematicBloc>();
	kenv.addFactory<CKPathFindingCinematicBloc>();
	kenv.addFactory<CKFlaggedPathCinematicBloc>();
	kenv.addFactory<CKGroupBlocCinematicBloc>();
	kenv.addFactory<CKAttachObjectsCinematicBloc>();
	kenv.addFactory<CKParticleCinematicBloc>();
	kenv.addFactory<CKStartDoor>();
	kenv.addFactory<CKSekensorCinematicBloc>();
	kenv.addFactory<CKDisplayPictureCinematicBloc>();
	kenv.addFactory<CKManageCameraCinematicBloc>();
	kenv.addFactory<CKStartEventCinematicBloc>();
	kenv.addFactory<CKLightningCinematicBloc>();
	kenv.addFactory<CKPlaySoundCinematicBloc>();
	kenv.addFactory<CKPauseCinematicBloc>();
	kenv.addFactory<CKTeleportCinematicBloc>();
	kenv.addFactory<CKEndDoor>();
	kenv.addFactory<CKPlayVideoCinematicBloc>();
	kenv.addFactory<CKFlashUICinematicBloc>();
	kenv.addFactory<CKLockUnlockCinematicBloc>();

	kenv.addFactory<CAnimationDictionary>();
	kenv.addFactory<CTextureDictionary>();
	kenv.addFactory<CKSoundDictionary>();

	kenv.addFactory<CKParticleGeometry>();
	kenv.addFactory<CKGeometry>();
	kenv.addFactory<CKSkinGeometry>();

	kenv.addFactory<CSGRootNode>();
	kenv.addFactory<CSGSectorRoot>();
	kenv.addFactory<CNode>();
	kenv.addFactory<CKDynBSphereProjectile>();
	kenv.addFactory<CSGLeaf>();
	kenv.addFactory<CSGBranch>();
	kenv.addFactory<CGlowNodeFx>();
	kenv.addFactory<CClone>();
	kenv.addFactory<CKBoundingSphere>();
	kenv.addFactory<CKDynamicBoundingSphere>();
	kenv.addFactory<CKAABB>();
	kenv.addFactory<CKOBB>();
	kenv.addFactory<CParticlesNodeFx>();
	kenv.addFactory<CAnimatedNode>();
	kenv.addFactory<CAnimatedClone>();
	kenv.addFactory<CKAACylinder>();
	kenv.addFactory<CSkyNodeFx>();
	kenv.addFactory<CFogBoxNodeFx>();
	kenv.addFactory<CTrailNodeFx>();
	kenv.addFactory<CSGLight>();
	kenv.addFactory<CCloudsNodeFx>();
	kenv.addFactory<CZoneNode>();
	kenv.addFactory<CSpawnNode>();
	kenv.addFactory<CSpawnAnimatedNode>();

	kenv.addFactory<CSGAnchor>();
	kenv.addFactory<CSGBkgRootNode>();

	kenv.addFactory<CKPFGraphTransition>();
	kenv.addFactory<CKSector>();
	kenv.addFactory<CKSpline4>();
	kenv.addFactory<CKChoreoKey>();
	kenv.addFactory<CKPFGraphNode>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CDynamicGround>();
	kenv.addFactory<CKFlaggedPath>();
	kenv.addFactory<CKChoreography>();
	kenv.addFactory<CKLine>();
	kenv.addFactory<CKSpline4L>();
	kenv.addFactory<CKCinematicScene>();
	kenv.addFactory<CKCinematicSceneData>();
	kenv.addFactory<CLocManager>();
	kenv.addFactory<CKMeshKluster>();
	kenv.addFactory<CKBeaconKluster>();
	kenv.addFactory<CKCombiner>();
	kenv.addFactory<CKComparator>();
	kenv.addFactory<CKComparedData>();
	kenv.addFactory<CKTrigger>();
	kenv.addFactory<CKDetectorBase>();
	kenv.addFactory<CKSectorDetector>();
	kenv.addFactory<CMultiGeometry>();
	kenv.addFactory<CKDetectorEvent>();
	kenv.addFactory<CKDetectedMovable>();
	kenv.addFactory<CKTriggerDomain>();
	kenv.addFactory<CMaterial>();
	kenv.addFactory<CKDetectorMusic>();
	kenv.addFactory<CKA2GameState>();
	kenv.addFactory<CKCameraFogDatas>();
	kenv.addFactory<CKA3GameState>();
	kenv.addFactory<CKTriggerSynchro>();

	kenv.addFactory<CCloneManager>();
	kenv.addFactory<CAnimationManager>();
	kenv.addFactory<CBillboard2d>();
	kenv.addFactory<CManager2d>();
	kenv.addFactory<CSectorAnimation>();
}

void ClassRegister::registerClassesForXXL2PlusConsole(KEnvironment& kenv)
{
	// XXL2+ console

	kenv.addFactory<CKSrvCinematic>();
	kenv.addFactory<CKSrvPathFinding>();
	kenv.addFactory<CKSrvBeacon>();
	kenv.addFactory<CKSrvTrigger>();

	//kenv.addFactory<CKHkBasicBonus>();

	//kenv.addFactory<CKGrpSquadX2>();
	//kenv.addFactory<CKGrpPoolSquad>();
	//kenv.addFactory<CKGrpA2BonusPool>();

	//kenv.addFactory<CKCrateCpnt>();

	kenv.addFactory<CKLogicalAnd>();
	kenv.addFactory<CKLogicalOr>();
	kenv.addFactory<CKPlayAnimCinematicBloc>();
	kenv.addFactory<CKPathFindingCinematicBloc>();
	kenv.addFactory<CKFlaggedPathCinematicBloc>();
	kenv.addFactory<CKGroupBlocCinematicBloc>();
	kenv.addFactory<CKAttachObjectsCinematicBloc>();
	kenv.addFactory<CKParticleCinematicBloc>();
	kenv.addFactory<CKStartDoor>();
	kenv.addFactory<CKSekensorCinematicBloc>();
	kenv.addFactory<CKDisplayPictureCinematicBloc>();
	kenv.addFactory<CKManageCameraCinematicBloc>();
	kenv.addFactory<CKStartEventCinematicBloc>();
	kenv.addFactory<CKLightningCinematicBloc>();
	kenv.addFactory<CKPlaySoundCinematicBloc>();
	kenv.addFactory<CKPauseCinematicBloc>();
	kenv.addFactory<CKTeleportCinematicBloc>();
	kenv.addFactory<CKEndDoor>();
	kenv.addFactory<CKPlayVideoCinematicBloc>();
	kenv.addFactory<CKFlashUICinematicBloc>();
	kenv.addFactory<CKLockUnlockCinematicBloc>();

	kenv.addFactory<CAnimationDictionary>();
	//kenv.addFactory<CTextureDictionary>();
	//kenv.addFactory<CKSoundDictionary>();

	//kenv.addFactory<CKParticleGeometry>();
	//kenv.addFactory<CKGeometry>();
	//kenv.addFactory<CKSkinGeometry>();

	//kenv.addFactory<CSGRootNode>();
	//kenv.addFactory<CSGSectorRoot>();
	//kenv.addFactory<CNode>();
	//kenv.addFactory<CKDynBSphereProjectile>();
	//kenv.addFactory<CSGLeaf>();
	//kenv.addFactory<CSGBranch>();
	//kenv.addFactory<CGlowNodeFx>();
	//kenv.addFactory<CClone>();
	//kenv.addFactory<CKBoundingSphere>();
	//kenv.addFactory<CKDynamicBoundingSphere>();
	//kenv.addFactory<CKAABB>();
	//kenv.addFactory<CKOBB>();
	//kenv.addFactory<CParticlesNodeFx>();
	//kenv.addFactory<CAnimatedNode>();
	//kenv.addFactory<CAnimatedClone>();
	//kenv.addFactory<CKAACylinder>();
	//kenv.addFactory<CSkyNodeFx>();
	//kenv.addFactory<CFogBoxNodeFx>();
	//kenv.addFactory<CTrailNodeFx>();
	//kenv.addFactory<CSGLight>();
	//kenv.addFactory<CCloudsNodeFx>();
	//kenv.addFactory<CZoneNode>();
	//kenv.addFactory<CSpawnNode>();
	//kenv.addFactory<CSpawnAnimatedNode>();

	//kenv.addFactory<CSGAnchor>();
	//kenv.addFactory<CSGBkgRootNode>();

	kenv.addFactory<CKPFGraphTransition>();
	kenv.addFactory<CKSector>();
	kenv.addFactory<CKSpline4>();
	kenv.addFactory<CKChoreoKey>();
	kenv.addFactory<CKPFGraphNode>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CDynamicGround>();
	kenv.addFactory<CKFlaggedPath>();
	kenv.addFactory<CKChoreography>();
	kenv.addFactory<CKLine>();
	kenv.addFactory<CKSpline4L>();
	kenv.addFactory<CKCinematicScene>();
	kenv.addFactory<CKCinematicSceneData>();
	kenv.addFactory<CLocManager>();
	kenv.addFactory<CKMeshKluster>();
	kenv.addFactory<CKBeaconKluster>();
	kenv.addFactory<CKCombiner>();
	kenv.addFactory<CKComparator>();
	kenv.addFactory<CKComparedData>();
	kenv.addFactory<CKTrigger>();
	kenv.addFactory<CKDetectorBase>();
	kenv.addFactory<CKSectorDetector>();
	kenv.addFactory<CMultiGeometry>();
	kenv.addFactory<CKDetectorEvent>();
	kenv.addFactory<CKDetectedMovable>();
	kenv.addFactory<CKTriggerDomain>();
	kenv.addFactory<CKDetectorMusic>();
	kenv.addFactory<CKA2GameState>();
	kenv.addFactory<CKA3GameState>();
	kenv.addFactory<CKTriggerSynchro>();

	//kenv.addFactory<CCloneManager>();
	//kenv.addFactory<CAnimationManager>();
	kenv.addFactory<CBillboard2d>();
	kenv.addFactory<CManager2d>();
	//kenv.addFactory<CSectorAnimation>();
}

void ClassRegister::registerClasses(KEnvironment& kenv, int gameVersion, int gamePlatform, bool isRemaster)
{
	if (gameVersion <= KEnvironment::KVERSION_XXL1 && (gamePlatform == KEnvironment::PLATFORM_PC || gamePlatform == KEnvironment::PLATFORM_GCN)) {
		registerClassesForXXL1PC(kenv);
	}
	else if (gameVersion <= KEnvironment::KVERSION_XXL1) {
		registerClassesForXXL1Console(kenv);
	}
	else if (gamePlatform == KEnvironment::PLATFORM_PC) {
		registerClassesForXXL2PlusPC(kenv);
	}
	else {
		registerClassesForXXL2PlusConsole(kenv);
		if (gameVersion <= KEnvironment::KVERSION_OLYMPIC) {
			kenv.addFactory<CKGrpSquadX2>();
			kenv.addFactory<CKGrpPoolSquad>();

			kenv.addFactory<CKHkBasicBonus>();
			kenv.addFactory<CKGrpSquadX2>();
			kenv.addFactory<CKGrpPoolSquad>();
			kenv.addFactory<CKGrpA2BonusPool>();
			kenv.addFactory<CKGrpBonusX2>();
			kenv.addFactory<CKGrpA3BonusPool>();
			kenv.addFactory<CKCrateCpnt>();
		}
		if (gamePlatform == KEnvironment::PLATFORM_WII || gamePlatform == KEnvironment::PLATFORM_X360) {
			kenv.addFactory<CKParticleGeometry>();
			kenv.addFactory<CKGeometry>();
			kenv.addFactory<CKSkinGeometry>();
			kenv.addFactory<CTextureDictionary>();

			kenv.addFactory<CSGRootNode>();
			kenv.addFactory<CSGSectorRoot>();
			kenv.addFactory<CNode>();
			kenv.addFactory<CKDynBSphereProjectile>();
			kenv.addFactory<CSGLeaf>();
			kenv.addFactory<CSGBranch>();
			kenv.addFactory<CGlowNodeFx>();
			kenv.addFactory<CClone>();
			kenv.addFactory<CKBoundingSphere>();
			kenv.addFactory<CKDynamicBoundingSphere>();
			kenv.addFactory<CKAABB>();
			kenv.addFactory<CKOBB>();
			kenv.addFactory<CParticlesNodeFx>();
			kenv.addFactory<CAnimatedNode>();
			kenv.addFactory<CAnimatedClone>();
			kenv.addFactory<CKAACylinder>();
			kenv.addFactory<CSkyNodeFx>();
			kenv.addFactory<CFogBoxNodeFx>();
			kenv.addFactory<CTrailNodeFx>();
			kenv.addFactory<CSGLight>();
			kenv.addFactory<CCloudsNodeFx>();
			kenv.addFactory<CZoneNode>();
			kenv.addFactory<CSpawnNode>();
			kenv.addFactory<CSpawnAnimatedNode>();
			//kenv.addFactory<CFogBoxNodeFx>();
			kenv.addFactory<CKPartlyUnknown<CNodeFx, 26>>();
			//kenv.addFactory<CTrailNodeFx>();
			kenv.addFactory<CKPartlyUnknown<CNodeFx, 27>>();

			kenv.addFactory<CSGAnchor>();
			kenv.addFactory<CSGBkgRootNode>();
			kenv.addFactory<CKPartlyUnknown<CNode, 28>>();

			kenv.addFactory<CCloneManager>();
		}
	}
}
