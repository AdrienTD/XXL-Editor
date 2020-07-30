#pragma once

#include "KObject.h"
#include <array>
#include "vecmat.h"
#include "CKPartlyUnknown.h"
#include "CKUtils.h"

struct CKHookLife;
struct CKGrpBonusPool;
struct CKGrpWildBoarPool;
struct CAnimationDictionary;
struct CKSceneNode;
struct CKBoundingShape;
struct CAnimatedClone;

struct CKHook : CKCategory<2> {
	kobjref<CKHook> next;
	uint32_t unk1 = 0;
	kobjref<CKHookLife> life;
	KPostponedRef<CKSceneNode> node;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment *kenv) override;
};

struct CKHookLife : CKCategory<3> {
	kobjref<CKHook> hook;
	kobjref<CKHookLife> nextLife;
	uint32_t unk1 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkEnemy : CKSubclass<CKHook, 80> {
	uint32_t unk1;
	Vector3 unk2;
	float unk3;
	uint8_t unk4, unk5;
	kobjref<CKObject> squad;
	Vector3 unk7;
	float unk8;
	kobjref<CKObject> unk9;
	kobjref<CKObject> unkA;
	kobjref<CKObject> shadowCpnt;
	kobjref<CKObject> hkWaterFx;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkSeizableEnemy : CKSubclass<CKHkEnemy, 84> {
	uint32_t sunk1;
	uint8_t sunk2, sunk3, sunk4;
	std::array<kobjref<CKBoundingShape>, 4> boundingShapes;
	kobjref<CKSceneNode> particlesNodeFx1, particlesNodeFx2, particlesNodeFx3;
	kobjref<CKSceneNode> fogBoxNode;
	uint32_t sunused;
	kobjref<CKHook> hero;
	kobjref<CKSceneNode> romanAnimatedClone;
	uint8_t sunk5;
	std::array<float, 7> sunk6;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkSquadSeizableEnemy : CKSubclass<CKHkSeizableEnemy, 92> {
	std::array<float, 9> matrix33;
	uint32_t sunk7;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkBasicEnemy : CKSubclass<CKHkSquadSeizableEnemy, 93> {
	kobjref<CKSceneNode> beClone1, beClone2, beClone3, beClone4;
	kobjref<CKSceneNode> beParticleNode1, beParticleNode2, beParticleNode3, beParticleNode4;
	kobjref<CAnimationDictionary> beAnimDict;
	kobjref<CKObject> beSoundDict;
	kobjref<CKBoundingShape> beBoundNode;

	kobjref<CAnimatedClone> romanAnimatedClone2;
	uint8_t beUnk1;
	std::array<float, 7> beUnk2;
	kobjref<CAnimatedClone> romanAnimatedClone3;
	uint8_t beUnk3;
	std::array<float, 7> beUnk4;
	float beUnk5, beUnk6;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkAnimatedCharacter : CKSubclass<CKHook, 97> {
	kobjref<CAnimationDictionary> animDict;
	kobjref<CKObject> shadowCpnt;
	kobjref<CKObject> unkRef1;
	Matrix matrix;
	Vector3 position;
	Vector3 orientation;
	std::array<float, 7> unkFloatArray;
	float unkFloat1, unkFloat2, unkFloat3, unkFloat4;
	uint8_t sector;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkRocketRoman : CKSubclass<CKHkBasicEnemy, 164> {
	kobjref<CKObject> rrAnimDict, rrParticleNode, rrCylinderNode, rrSoundDictID;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkBasicBonus : CKSubclass<CKHook, 114> {
	kobjref<CKHkBasicBonus> nextBonus;
	kobjref<CKGrpBonusPool> pool;
	kobjref<CKObject> cpnt;
	kobjref<CKObject> hero;
	std::array<float, 7> somenums;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkWildBoar : CKSubclass<CKHook, 171> {
	kobjref<CKHkWildBoar> nextBoar;
	kobjref<CKSceneNode> boundingSphere;
	kobjref<CKObject> animationDictionary;
	kobjref<CKObject> cpnt;
	kobjref<CKGrpWildBoarPool> pool;
	std::array<float, 4> somenums;
	kobjref<CKObject> shadowCpnt;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

// Unknown hook classes. To implement later!
struct CKHkPressionStone : CKPartlyUnknown<CKHook, 21> {};
struct CKHkAsterix : CKPartlyUnknown<CKHook, 28> {};
struct CKHkObelix : CKPartlyUnknown<CKHook, 29> {};
struct CKHkIdefix : CKPartlyUnknown<CKHook, 30> {};
struct CKHkMachinegun : CKPartlyUnknown<CKHook, 31> {};
struct CKHkTorch : CKPartlyUnknown<CKHook, 32> {};
struct CKHkHearth : CKPartlyUnknown<CKHook, 33> {};
struct CKHkDrawbridge : CKPartlyUnknown<CKHook, 34> {};
struct CKHkMegaAshtray : CKPartlyUnknown<CKHook, 37> {};
struct CKHkBoat : CKPartlyUnknown<CKHook, 39> {};
struct CKHkCorkscrew : CKPartlyUnknown<CKHook, 44> {};
struct CKHkTurnstile : CKPartlyUnknown<CKHook, 45> {};
struct CKHkLifter : CKPartlyUnknown<CKHook, 47> {};
struct CKHkActivator : CKPartlyUnknown<CKHook, 52> {};
struct CKHkRotaryBeam : CKPartlyUnknown<CKHook, 57> {};
struct CKHkLightPillar : CKPartlyUnknown<CKHook, 60> {};
struct CKHkWind : CKPartlyUnknown<CKHook, 73> {};
struct CKHkJumpingRoman : CKPartlyUnknown<CKHook, 75> {};
struct CKHkWaterJet : CKPartlyUnknown<CKHook, 76> {};
struct CKHkPowderKeg : CKPartlyUnknown<CKHook, 77> {};
struct CKHkTriangularTurtle : CKPartlyUnknown<CKHook, 90> {};
struct CKHkRomanArcher : CKPartlyUnknown<CKHook, 95> {};
//struct CKHkAnimatedCharacter : CKPartlyUnknown<CKHook, 97> {}; ////////
struct CKHkSwingDoor : CKPartlyUnknown<CKHook, 98> {};
struct CKHkSlideDoor : CKPartlyUnknown<CKHook, 100> {};
struct CKHkCrumblyZone : CKPartlyUnknown<CKHook, 102> {};
struct CKHkHelmetCage : CKPartlyUnknown<CKHook, 108> {};
struct CKHkSquareTurtle : CKPartlyUnknown<CKHook, 110> {};
struct CKHkTeleBridge : CKPartlyUnknown<CKHook, 111> {};
struct CKHkCrate : CKPartlyUnknown<CKHook, 112> {};
struct CKHkDonutTurtle : CKPartlyUnknown<CKHook, 124> {};
struct CKHkPyramidalTurtle : CKPartlyUnknown<CKHook, 125> {};
struct CKHkRollingStone : CKPartlyUnknown<CKHook, 126> {};
struct CKHkInterfaceBase : CKPartlyUnknown<CKHook, 128> {};
struct CKHkInterfaceEvolution : CKPartlyUnknown<CKHook, 129> {};
struct CKHkCatapult : CKPartlyUnknown<CKHook, 130> {};
struct CKHkInterfacePause : CKPartlyUnknown<CKHook, 131> {};
struct CKHkInterfaceInGame : CKPartlyUnknown<CKHook, 132> {};
struct CKHkInterfaceOption : CKPartlyUnknown<CKHook, 133> {};
struct CKHkInterfaceMain : CKPartlyUnknown<CKHook, 136> {};
struct CKHkInterfaceLoadSave : CKPartlyUnknown<CKHook, 138> {};
struct CKHkInterfaceCloth : CKPartlyUnknown<CKHook, 141> {};
struct CKHkInterfaceShop : CKPartlyUnknown<CKHook, 144> {};
struct CKHkPushPullAsterix : CKPartlyUnknown<CKHook, 147> {};
struct CKHkBasicEnemyLeader : CKPartlyUnknown<CKHook, 148> {};
struct CKHkTelepher : CKPartlyUnknown<CKHook, 158> {};
struct CKHkTowedTelepher : CKPartlyUnknown<CKHook, 159> {};
struct CKHkBumper : CKPartlyUnknown<CKHook, 160> {};
struct CKHkClueMan : CKPartlyUnknown<CKHook, 161> {};
struct CKHkSky : CKPartlyUnknown<CKHook, 163> {};
struct CKHkJetPackRoman : CKPartlyUnknown<CKHook, 167> {};
struct CKHkAsterixShop : CKPartlyUnknown<CKHook, 172> {};
struct CKHkWater : CKPartlyUnknown<CKHook, 173> {};
struct CKHkMobileTower : CKPartlyUnknown<CKHook, 176> {};
struct CKHkBoss : CKPartlyUnknown<CKHook, 177> {};
struct CKHkInterfaceDemo : CKPartlyUnknown<CKHook, 179> {};
struct CKHkWaterFx : CKPartlyUnknown<CKHook, 180> {};
struct CKHkHighGrass : CKPartlyUnknown<CKHook, 183> {};
struct CKHkWaterFall : CKPartlyUnknown<CKHook, 185> {};
struct CKHkInterfaceGallery : CKPartlyUnknown<CKHook, 187> {};
struct CKHkTrioCatapult : CKPartlyUnknown<CKHook, 190> {};
struct CKHkObelixCatapult : CKPartlyUnknown<CKHook, 191> {};
struct CKHkInterfaceOpening : CKPartlyUnknown<CKHook, 192> {};
struct CKHkAsterixCheckpoint : CKPartlyUnknown<CKHook, 193> {};
struct CKHkBonusSpitter : CKPartlyUnknown<CKHook, 194> {};
struct CKHkLight : CKPartlyUnknown<CKHook, 195> {};

///--- Hook life classes ---///

struct CKHkAsterixLife : CKSubclass<CKHookLife, 27> {};

struct CKHkBoatLife : CKSubclass<CKHookLife, 55> {
	kobjref<CKHook> boatHook;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkObelixLife : CKSubclass<CKHookLife, 57> {};
struct CKHkMecaLife : CKSubclass<CKHookLife, 59> {};
struct CKHkIdefixLife : CKSubclass<CKHookLife, 66> {};
struct CKHkEnemyLife : CKSubclass<CKHookLife, 93> {};
struct CKHkTriangularLife : CKSubclass<CKHookLife, 95> {};

struct CKHkAnimatedCharacterLife : CKSubclass<CKHookLife, 99> {};
struct CKHkSquareTurtleLife : CKSubclass<CKHookLife, 102> {};
struct CKHkDonutTurtleLife : CKSubclass<CKHookLife, 104> {};
struct CKHkPyramidalTurtleLife : CKSubclass<CKHookLife, 105> {};
struct CKHkCatapultLife : CKSubclass<CKHookLife, 108> {};

struct CKHkSkyLife : CKSubclass<CKHookLife, 112> {
	uint32_t skyColor, cloudColor;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkWaterLife : CKSubclass<CKHookLife, 115> {};
struct CKHkBossLife : CKSubclass<CKHookLife, 118> {};
struct CKHkWaterFxLife : CKSubclass<CKHookLife, 120> {};
struct CKHkAsterixCheckpointLife : CKSubclass<CKHookLife, 124> {};
struct CKHkWaterFallLife : CKSubclass<CKHookLife, 125> {};
