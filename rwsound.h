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
	static constexpr uint32_t NONNULL_POINTER = 0xFFFFFFFF; // a pointer value that can be anything but 0!

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
		fin_j = 0,
		subSampleRate = 0,
		fin_l = 0x45F4E8;
	uint32_t fin_m = 0;
	uint8_t fin_n1 = 4, subNumChannels = 1, fin_n3 = 0, fin_n4 = 0;
	uint32_t fin_o = 0,
		fin_p = 0;
	uint32_t fin_q = 0,
		fin_r = 0xEF386593,
		fin_s = 0x432DB611,
		fin_t = 0x1AA77F95;
	uint32_t fin_u = 0x7A2244DE,
		fin_v = 0,
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
