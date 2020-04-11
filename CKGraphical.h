#pragma once

#include "KObject.h"
#include <vector>
#include "rw.h"

struct CClone;

struct CKGraphical : CKCategory<13> {

};

struct CCloneManager : CKSubclass<CKGraphical, 3> {
	uint32_t _numClones, _unk1, _unk2, _unk3, _unk4;
	std::vector<kobjref<CClone>> _clones;
	RwTeamDictionary _teamDict;
	RwTeam _team;
	std::vector<std::array<float, 4>> flinfos;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};