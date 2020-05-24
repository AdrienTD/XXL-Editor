#pragma once

#include "KObject.h"
#include <array>
#include "vecmat.h"

struct CKSceneNode;

struct CKComponent : CKCategory<6> {

};

struct CKCrateCpnt : CKSubclass<CKComponent, 5> {
	kobjref<CKObject> group;
	kobjref<CKSceneNode> particleNode;
	kobjref<CKObject> soundIds;
	kobjref<CKObject> projectiles;
	kobjref<CKSceneNode> crateNode;
	std::array<float, 6> unk1;
	uint8_t unk7;
	std::array<float, 56 * 2> pack1;
	std::array<float, 56> pack2;
	std::array<int, 4> bonuses;
	uint16_t unk8;
	uint8_t unk9;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBasicEnemyCpnt : CKSubclass<CKComponent, 10> {
	std::array<uint8_t, 0x129> data;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKRocketRomanCpnt : CKSubclass<CKBasicEnemyCpnt, 19> {
	float rrCylinderRadius = 0, rrCylinderHeight = 0;
	Vector3 runk3 = Vector3(0,0,0);
	uint8_t runk4 = 0;
	float rrFireDistance = 0;
	uint8_t runk6 = 0;
	float rrFlySpeed = 0, rrRomanAimFactor = 0;
	kobjref<CKObject> runk9;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};