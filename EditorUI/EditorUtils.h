#pragma once

#include <cstdint>

struct KEnvironment;
struct RwSound;

namespace EditorUI
{
	void InitSnd(int freq, bool byteSwapped);
	void PlaySnd(KEnvironment& kenv, RwSound& snd);

	float decode8bitAngle(uint8_t byte);
}