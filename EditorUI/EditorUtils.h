#pragma once

#include <cstdint>

struct RwSound;
struct WavDocument;

namespace EditorUI
{
	void InitSnd(int freq, int numChannels, bool byteSwapped);
	void PlaySnd(const RwSound& snd);
	void PlaySnd(const WavDocument& wav);

	float decode8bitAngle(uint8_t byte);
}