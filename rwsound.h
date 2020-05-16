#pragma once

#include "rw.h" // for stuff like RwsHeader, might need to move this into something like "rwcore.h" because we don't need graphics there
#include "DynArray.h"
#include <array>
#include <string>
#include <vector>

struct File;

template<int N> struct RwStruct {
	static const uint32_t tagID = N;
};

struct RwSoundData : RwStruct<0x804> {
	DynArray<uint8_t> data;

	void deserialize(File *file, uint32_t size);
	void serialize(File *file);
};

struct RwSoundInfo : RwStruct<0x803> {
	uint32_t unk1;
	struct Ding {
		uint32_t sampleRate;
		uint32_t dunk1;
		uint32_t dataSize;
		std::array<uint8_t, 32> rest;
	};
	std::vector<Ding> dings;
	std::array<uint8_t, 20> rest;
	DynArray<uint8_t> name;
	//std::string name;

	void deserialize(File *file, uint32_t size);
	void serialize(File *file);
};

struct RwSound : RwStruct<0x802> {
	RwSoundInfo info;
	RwSoundData data;
	
	void deserialize(File *file);
	void serialize(File *file);
};

struct RwSoundList : RwStruct<0x80C> {
	std::vector<RwSound> sounds;

	void deserialize(File *file);
	void serialize(File *file);
};

struct RwSoundDictionaryInfo : RwStruct<0x80A> {
	DynArray<uint8_t> bytes;

	void deserialize(File *file, uint32_t size);
	void serialize(File *file);
};

struct RwSoundDictionary : RwStruct<0x809> {
	RwSoundDictionaryInfo info;
	RwSoundList list;

	void deserialize(File *file);
	void serialize(File *file);
};