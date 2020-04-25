#pragma once

#include "KObject.h"
#include <array>

struct CKSceneNode;
struct CKGrpBonusPool;
struct CKGrpWildBoarPool;
struct CAnimatedClone;

struct CKHook : CKCategory<2> {
	kobjref<CKHook> next;
	uint32_t unk1 = 0;
	kobjref<CKObject> life;
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

struct CKHkBasicEnemy : CKSubclass<CKHook, 93> {
	kobjref<CAnimatedClone> _1;

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