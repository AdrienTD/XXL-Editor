#pragma once

#include "KObject.h"

struct CKHook;
struct CKGroupLife;
struct CKSceneNode;

struct CKHkBasicBonus;

struct CKGroup : CKCategory<4> {
	objref<CKGroup> nextGroup;
	objref<CKGroup> parentGroup;
	objref<CKGroupLife> life;
	objref<CKObject> bundle;
	uint32_t unk2 = 0;
	objref<CKGroup> childGroup;
	objref<CKHook> childHook;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGroupLife : CKCategory<5> {
	objref<CKObject> unk;
	objref<CKGroup> group;
	uint32_t unk2 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpAsterixBonusPool : CKSubclass<CKGroup, 63> {
	uint32_t bonusType;
	uint32_t unk1, unk2, unk3, unk4; // unk3 and unk4 might be objrefs?
	objref<CKHkBasicBonus> nextBonusHook;
	objref<CKObject> bonusCpnt;
	objref<CKSceneNode> particleNode1, particleNode2;
	uint32_t unk5; // objref maybe?

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};