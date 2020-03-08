#pragma once

#include "KObject.h"
#include <vector>

struct CClone;

struct CKGraphical : CKCategory<13> {

};

struct CCloneManager : CKSubclass<CKGraphical, 3> {
	uint32_t _numClones, _unk1, _unk2, _unk3, _unk4;
	std::vector<objref<CClone>> _clones;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};