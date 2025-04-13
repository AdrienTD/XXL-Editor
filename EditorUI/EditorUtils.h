#pragma once

struct KEnvironment;
struct RwSound;

namespace EditorUI
{
	void InitSnd(int freq, bool byteSwapped);
	void PlaySnd(KEnvironment& kenv, RwSound& snd);
}