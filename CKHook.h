#pragma once

#include "KObject.h"
#include <array>

struct CKSceneNode;
struct CKGrpAsterixBonusPool;

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

struct CKHkBasicBonus : CKSubclass<CKHook, 114> {
	kobjref<CKHkBasicBonus> nextBonus;
	kobjref<CKGrpAsterixBonusPool> pool;
	kobjref<CKObject> cpnt;
	kobjref<CKObject> hero;
	std::array<float, 7> somenums;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};