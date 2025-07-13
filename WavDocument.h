#pragma once

#include <cstdint>
#include "DynArray.h"

struct File;

struct WavDocument {
	uint16_t formatTag;
	uint16_t numChannels;
	uint32_t samplesPerSec;
	uint32_t avgBytesPerSec;
	uint16_t blockAlign;
	uint16_t pcmBitsPerSample;
	DynArray<uint8_t> data;

	void read(File *file);
	void write(File *file);

	size_t getNumSamples() { return data.size() / blockAlign; }
};

struct WavSampleReader {
	const WavDocument *_wav;
	const uint8_t *_pnt;

	WavSampleReader(const WavDocument *wav) { _wav = wav; _pnt = wav->data.data(); }

	void nextSample();
	float getSample(int channelIndex);
	bool available();
};