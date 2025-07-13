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

static constexpr uint32_t NONNULL_POINTER = 0xFFFFFFFF; // a pointer value that can be anything but 0!

static constexpr std::array<uint8_t, 16> RWA_FORMAT_ID_XBOX_ADPCM = {
	0x93, 0x65, 0x38, 0xEF, 0x11, 0xB6, 0x2D, 0x43, 0x95, 0x7F, 0xA7, 0x1A, 0xDE, 0x44, 0x22, 0x7A
};

struct RwaWaveFormat {
	uint32_t sampleRate = 0;
	uint32_t ptrDataTypeUuid = NONNULL_POINTER;
	uint32_t dataSize = 0;
	uint8_t bitsPerSample = 0;
	uint8_t numChannels = 0;
	uint8_t padding1 = 0;
	uint8_t padding2 = 0;
	uint32_t ptrMiscData = 0;
	uint32_t miscDataSize = 0;
	uint8_t flags = 0;
	uint8_t reserved = 0;
	uint8_t padding3 = 0;
	uint8_t padding4 = 0;
	std::array<uint8_t, 16> uuid;

	void deserialize(File* file, bool isBigEndian);
	void serialize(File* file, bool isBigEndian) const;
};

struct RwSoundData : RwStruct<0x804> {
	DynArray<uint8_t> data;

	void deserialize(File *file, uint32_t size);
	void serialize(File *file);
};

struct RwSoundInfo : RwStruct<0x803> {
	uint32_t unk1;
	RwaWaveFormat format;
	RwaWaveFormat targetFormat;
	std::array<uint8_t, 20> rest;
	DynArray<uint8_t> name;
	//std::string name;
	bool isBigEndian = false;

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
	bool isBigEndian = false;

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

struct RwStreamInfo : RwStruct<0x80E> {
	uint32_t head_a = 0x14, head_b = 0x10, head_c = 0x24, head_d = 7;

	uint32_t str_a = NONNULL_POINTER,
		str_b = NONNULL_POINTER,
		str_c = 0,
		numSegments = 0;
	uint32_t str_e = NONNULL_POINTER,
		numSubstreams = 1,
		str_g = NONNULL_POINTER,
		basicSectorSize = 0;
	uint32_t streamSectorSize = 0,
		basicSectorSize2 = 0,
		str_k = 0,
		str_l = 0;
	uint32_t str_m = 0, str_n = 0, str_o = 0;
	std::array<char, 16> streamName = { 0 };

	struct Segment {
		uint32_t segVal_1a = NONNULL_POINTER, segVal_1b = NONNULL_POINTER, segVal_1c = 0, segVal_1d = NONNULL_POINTER;
		uint32_t segVal_1e = 0, segVal_1f = 0, dataAlignedSize = 0, dataOffset = 0;
		uint32_t dataSize = 0;
		uint32_t segVal_3a = 0, segVal_3b = 0, segVal_3c = 0, segVal_3d = 0;
		std::array<char, 16> name = { 0 };
	};
	std::vector<Segment> segments;

	uint32_t fin_a = NONNULL_POINTER,
		fin_b = NONNULL_POINTER,
		fin_c = 0,
		sub_d = 0;
	uint32_t subSectorSize = 0,
		fin_f = NONNULL_POINTER,
		sub_g = 0,
		sub_h = 0;
	uint32_t subSectorUsedSize = 0,
		fin_j = 0;
	RwaWaveFormat waveFormat;
	uint32_t fin_v = 0,
		fin_w = 0,
		fin_x = 0;
	uint32_t fin_y = 0,
		fin_z = 0;
	std::array<char, 16> subName = { 0 };

	void deserialize(File* file);
	void serialize(File* file);
};

struct RwStream : RwStruct<0x80D> {
	RwStreamInfo info;
	std::vector<uint8_t> data;

	void deserialize(File* file);
	void serialize(File* file);
};
