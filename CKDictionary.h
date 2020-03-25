#pragma once

#include "KObject.h"
#include "rw.h"

struct CKDictionary : CKCategory<9> {

};

struct CTextureDictionary : CKSubclass<CKDictionary, 2> {
	struct Texture {
		char name[33];
		uint32_t unk1, unk2, unk3;
		RwImage image;
	};

	std::vector<Texture> textures;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};