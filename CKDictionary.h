#pragma once

#include "KObject.h"
#include "rw.h"
#include "rwsound.h"
#include "CKUtils.h"

struct CKDictionary : CKCategory<9> {

};

struct CAnimationDictionary : CKSubclass<CKDictionary, 1> {
	uint32_t numAnims = 0, numSets = 1;
	std::vector<uint32_t> animIndices;

	// Arthur+:
	int32_t arSector = 0;
	int8_t arUnk = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CTextureDictionary : CKSubclass<CKDictionary, 2> {
	//struct Texture {
	//	char name[33];
	//	uint32_t unk1, unk2, unk3;
	//	RwImage image;
	//};

	//std::vector<Texture> textures;
	RwPITexDict piDict;
	//RwNTTexDict nativeDict; // for consoles

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSoundDictionary : CKSubclass<CKDictionary, 3> {
	uint8_t inactive = 0xFD;
	//uint32_t numSounds;
	struct Sound {
		uint32_t id1; float unk2, unk3; uint8_t unk4;
		float unk5; uint16_t sampleRate; uint32_t unk7;
		uint8_t unk8, unk9, unkA;
		uint32_t id2;
		kobjref<CKObject> waveObj; // Arthur+
		std::string hdPath; // Romaster
	};
	std::vector<Sound> sounds;
	RwSoundDictionary rwSoundDict;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSoundDictionaryID : CKSubclass<CKDictionary, 4> {
	struct SoundEntry {
		bool active = false;
		uint32_t id = 0, flags = 0;
		kobjref<CKObject> obj; Vector3 refalt;
		float unk1 = 1.0f, unk2 = 1.0f, unk3 = 0.0f, unk4 = 0.0f;
		Vector3 boxHigh = { 15.0f, 15.0f, 15.0f }, boxLow = { -15.0f, -15.0f, -15.0f };
		uint8_t unk6 = 0;
	};
	std::vector<SoundEntry> soundEntries;

	// XXL2+:
	std::vector<KPostponedRef<CKObject>> x2Sounds;
	uint32_t x2Sector = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment* kenv) override;
};