#pragma once

#include "KObject.h"

struct CKSceneNode;

struct CKHook : CKCategory<2> {
	objref<CKHook> next;
	uint32_t unk1 = 0;
	objref<CKObject> life;
	objref<CKSceneNode> node;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKHookLife : CKCategory<3> {
	objref<CKHook> hook;
	objref<CKHookLife> nextLife;
	uint32_t unk1 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};