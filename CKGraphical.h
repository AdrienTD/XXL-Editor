#pragma once

#include "KObject.h"
#include <vector>
#include "rw.h"

struct CClone;
struct CSGBranch;

struct CKGraphical : CKCategory<13> {

};

struct CCloneManager : CKSubclass<CKGraphical, 3> {
	// For XXL2+ : Same values as for the beginning of CGeometry:
	kobjref<CKObject> x2_unkobj1;
	kobjref<CKObject> x2_lightSet;
	uint32_t x2_flags;

	uint32_t _numClones, _unk1, _unk2, _unk3, _unk4;
	std::vector<kobjref<CSGBranch>> _clones;
	RwTeamDictionary _teamDict;
	RwTeam _team;
	std::vector<std::array<float, 4>> flinfos;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};