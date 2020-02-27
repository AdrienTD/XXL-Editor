#pragma once

#include "KObject.h"
#include <vector>

struct CKService;

struct CKManager : CKCategory<0> {};

struct CKServiceManager : CKSubclass<CKManager, 1> {
	std::vector<objref<CKService>> services;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(File *file) override;
};