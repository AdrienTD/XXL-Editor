#pragma once

#include "KObject.h"
#include <array>
#include "vecmat.h"

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
	kobjref<CKSceneNode> node;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
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
	uint8_t unkByte;
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
