#pragma once

#include "KObject.h"
#include <array>

struct CKSceneNode;
struct CKGrpAsterixBonusPool;

struct CKHook : CKCategory<2> {
	objref<CKHook> next;
	uint32_t unk1 = 0;
	objref<CKObject> life;
	objref<CKSceneNode> node;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHookLife : CKCategory<3> {
	objref<CKHook> hook;
	objref<CKHookLife> nextLife;
	uint32_t unk1 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHkBasicBonus : CKSubclass<CKHook, 114> {
	objref<CKHkBasicBonus> nextBonus;
	objref<CKGrpAsterixBonusPool> pool;
	objref<CKObject> cpnt;
	objref<CKObject> hero;
	std::array<float, 7> somenums;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};