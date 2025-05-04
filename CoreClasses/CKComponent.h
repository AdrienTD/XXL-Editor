#pragma once

#include "KObject.h"
#include <array>
#include "vecmat.h"
#include "CKUtils.h"

struct CKSceneNode;
struct CKSoundDictionaryID;

struct CKComponent : CKMemberReflectable<CKCategory<6>> {

};

struct CKCrateCpnt : CKMRSubclass<CKCrateCpnt, CKComponent, 5> {
	struct CameraQuakeDatas { // TODO: Move to own class CKCameraQuakeDatas
		std::vector<float> data1, data2;
		float fnFloat;
	};
	kobjref<CKObject> group;
	kobjref<CKSceneNode> particleNode;
	kobjref<CKObject> soundIds;
	kobjref<CKObject> projectiles;
	kobjref<CKSceneNode> crateNode;
	// -- XXL2+: --
	std::array<CameraQuakeDatas, 2> x2CamQuakeDatas;
	float x2UnkFlt;
	// ----
	std::array<float, 6> unk1;
	uint8_t unk7;
	std::array<float, 56 * 2> pack1;
	std::array<float, 56> pack2;
	std::array<int, 4> bonuses;
	uint16_t unk8;
	uint8_t unk9;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKEnemyCpnt : CKMRSubclass<CKEnemyCpnt, CKComponent, 6> {
	// Some names proposed by Spork
	uint32_t flags = 0x3007ea9;
	uint8_t health = 3, damage = 1;
	float unk1 = 0.3f;
	float walkAnimSpeed = 7.0f;
	float coverAnimSpeed = 2.7f;
	float speed1 = 12.0f;
	float runSpeed1 = 9.0f;
	float speed2 = 2.5f;
	float speed3 = 24.0f;
	float runSpeed2 = 12.0f;
	float speed4 = 8.0f;
	float speed5 = 6.283185f;
	float unkf1 = 3.14f;
	float unkf2 = 6.28f;
	float unkf3 = 0.05f;
	float coverRange = 0.5f;
	float unkf5 = 0.392699f;
	float attackCooldown = 2.0f;
	float unkf7 = 3.0f;
	float unkf8 = 2.0f;
	float unkf9 = 1.0f;
	float targeting = 1.0f;
	float unkLast = 1.0f;

	// New Romaster Values:
	uint8_t ecRoma1 = 3;
	uint8_t ecRoma2 = 1;
	float ecRoma3 = 0.1f;
	float ecRoma4 = 0.0f;
	float ecRoma5 = 0.1f;
	float ecRoma6 = 0.0f;
	uint8_t ecRoma7 = 8;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKShadowCpnt : CKMRSubclass<CKShadowCpnt, CKComponent, 18> {
	std::array<float, 9> scpValues;
	std::array<uint8_t, 4> scpBytes;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKBonusCpnt : CKMRSubclass<CKBonusCpnt, CKComponent, 22> {
	float ckbcUnk0;
	kobjref<CKSoundDictionaryID> ckbcUnk1;
	float ckbcUnk2;
	float ckbcUnk3;
	float ckbcUnk4;
	float ckbcUnk5;
	float ckbcUnk6;
	float ckbcUnk7;
	float ckbcUnk8;
	float ckbcUnk9;
	float ckbcUnk10;
	float ckbcUnk11;
	float ckbcUnk12;
	float ckbcUnk13;
	uint8_t ckbcUnk14;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
