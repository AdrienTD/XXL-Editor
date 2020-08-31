#pragma once

#include <vector>
#include "rw.h"
#include "KLocalObject.h"
#include "DynArray.h"

struct Loc_CLocManager : KLocalObjectSub<12, 59> {
	std::vector<std::pair<uint32_t, std::wstring>> trcStrings;
	std::vector<std::wstring> stdStrings;
	uint16_t numLanguages;
	std::vector<uint32_t> langStrIndices, langIDs;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	KLocalObject *clone() override { return new Loc_CLocManager(*this); }
};

struct Loc_CManager2d : KLocalObjectSub<13, 16> {
	RwPITexDict piTexDict;
	std::vector<std::pair<uint32_t, RwFont2D>> fonts;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	KLocalObject *clone() override { return new Loc_CManager2d(*this); }
};