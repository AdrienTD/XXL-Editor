#pragma once

#include <vector>
#include "rw.h"
#include "KLocalObject.h"
#include "DynArray.h"

struct Loc_CLocManager : KLocalObjectSub<12, 59> {
	std::vector<std::pair<uint32_t, std::wstring>> trcStrings;
	std::vector<std::wstring> stdStrings;
	uint16_t numThings;
	std::vector<uint32_t> thingTable1, thingTable2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct Loc_CManager2d : KLocalObjectSub<13, 16> {
	RwPITexDict piTexDict;
	DynArray<uint8_t> rest;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};