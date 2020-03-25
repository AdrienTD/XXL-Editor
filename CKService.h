#pragma once

#include "KObject.h"
#include <vector>

struct CKService : CKCategory<1> {};

struct CKSrvEvent : CKSubclass<CKService, 5>
{
	struct StructB {
		uint8_t _1, _2;
	};
	uint16_t numA, numB, numC, numObjs;
	std::vector<StructB> bees;
	std::vector<objref<CKObject>> objs;
	std::vector<uint16_t> objInfos;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};