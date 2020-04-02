#pragma once

#include "KObject.h"
#include <array>

struct CKSceneNode;

struct CKComponent : CKCategory<6> {

};

struct CKCrateCpnt : CKSubclass<CKComponent, 5> {
	objref<CKObject> group;
	objref<CKSceneNode> particleNode;
	objref<CKObject> soundIds;
	objref<CKObject> projectiles;
	objref<CKSceneNode> crateNode;
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