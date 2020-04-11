#pragma once

#include "KObject.h"
#include "CKService.h"
#include <vector>

struct CKManager : CKCategory<0> {};

struct CKServiceManager : CKSubclass<CKManager, 1> {
	std::vector<kobjref<CKService>> services;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};