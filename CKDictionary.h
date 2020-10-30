#pragma once

#include "KObject.h"
#include "rw.h"
#include "rwsound.h"

struct CKDictionary : CKCategory<9> {

};

struct CAnimationDictionary : CKSubclass<CKDictionary, 1> {
	std::vector<uint32_t> animIndices, secondAnimIndices;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
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
		uint32_t id, flags;
		kobjref<CKObject> obj; std::array<uint32_t,3> refalt;
		float unk1, unk2, unk3, unk4;
		std::array<float, 6> flar;
		uint8_t unk6;
	};
	std::vector<SoundEntry> soundEntries;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};