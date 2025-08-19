#pragma once

#include "KObject.h"
#include "CKUtils.h"
#include "Events.h"

struct CKHookLife;
struct CKSceneNode;

struct CKHook : CKMRSubclass<CKHook, CKMemberReflectable<CKCategory<2>>, 0> {
	kobjref<CKHook> next; // next hook in a CKGroup
	uint32_t unk1 = 0;
	kobjref<CKHookLife> life;
	KPostponedRef<CKSceneNode> node;

	// XXL2+:
	kobjref<CKHook> x2nextLife; // next life in a CKBundle active/inactive hook life list
	uint32_t x2UnkA, x2Sector = 0;

	// Addendum:
	int activeSector = -2;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment *kenv) override;
	int getAddendumVersion() override;
	void deserializeAddendum(KEnvironment* kenv, File* file, int version) override;
	void serializeAddendum(KEnvironment* kenv, File* file) override;

	virtual void update() {}
};

struct CKHookLife : CKCategory<3> {
	kobjref<CKHook> hook;
	kobjref<CKHookLife> nextLife;
	uint32_t unk1 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

// TO REMOVE
template <class D, class T, int N> using CKHookSubclass = CKMRSubclass<D, T, N>;

// ===== Common hooks =====

struct CKGrpBonusPool;
struct CKCrateCpnt;
struct CDynamicGround;

struct CKHkCrate : CKHookSubclass<CKHkCrate, CKHook, 112> {
	kobjref<CDynamicGround> ckhcUnk0;
	kobjref<CKCrateCpnt> ckhcUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKHkBasicBonus : CKHookSubclass<CKHkBasicBonus, CKHook, 114> {
	kobjref<CKHkBasicBonus> nextBonus;
	kobjref<CKGrpBonusPool> pool;
	kobjref<CKObject> cpnt;
	kobjref<CKObject> hero;
	std::array<float, 7> somenums;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKHkLight : CKHookSubclass<CKHkLight, CKHook, 195> {
	kobjref<CKObject> lightGrpLight;
	EventNode lightEvtSeq1;
	EventNode lightEvtSeq2;
	EventNode lightEvtSeq3;
	EventNode lightEvtSeq4;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKHkSkyLife : CKSubclass<CKHookLife, 112> {
	uint32_t skyColor, cloudColor;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};
