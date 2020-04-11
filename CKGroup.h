#pragma once

#include "KObject.h"

struct CKHook;
struct CKGroupLife;
struct CKSceneNode;

struct CKHkBasicBonus;

struct CKGroup : CKCategory<4> {
	kobjref<CKGroup> nextGroup;
	kobjref<CKGroup> parentGroup;
	kobjref<CKGroupLife> life;
	kobjref<CKObject> bundle;
	uint32_t unk2 = 0;
	kobjref<CKGroup> childGroup;
	kobjref<CKHook> childHook;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGroupLife : CKCategory<5> {
	kobjref<CKObject> unk;
	kobjref<CKGroup> group;
	uint32_t unk2 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGrpAsterixBonusPool : CKSubclass<CKGroup, 63> {
	uint32_t bonusType;
	uint32_t unk1, unk2, unk3, unk4; // unk3 and unk4 might be objrefs?
	kobjref<CKHkBasicBonus> nextBonusHook;
	kobjref<CKObject> bonusCpnt;
	kobjref<CKSceneNode> particleNode1, particleNode2;
	kobjref<CKObject> secondBonusCpnt;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};