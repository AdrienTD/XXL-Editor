#include "EditorUtils.h"
#include <cassert>
#include <SDL2/SDL_audio.h>
#include "KEnvironment.h"
#include "rwsound.h"

namespace EditorUI
{
	static bool audioInitDone = false;
	static SDL_AudioDeviceID audiodevid;
	static int audioLastFreq = 0;

	void InitSnd(int freq, bool byteSwapped) {
		if (audioInitDone && audioLastFreq == freq) {
			SDL_ClearQueuedAudio(audiodevid);
			return;
		}
		if (audioInitDone) {
			SDL_ClearQueuedAudio(audiodevid);
			SDL_CloseAudioDevice(audiodevid);
		}
		SDL_AudioSpec spec, have;
		memset(&spec, 0, sizeof(spec));
		spec.freq = freq;
		spec.format = byteSwapped ? AUDIO_S16MSB : AUDIO_S16LSB;
		spec.channels = 1;
		spec.samples = 4096;
		audiodevid = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);
		assert(audiodevid);
		SDL_PauseAudioDevice(audiodevid, 0);
		audioInitDone = true;
		audioLastFreq = freq;
	}

	void PlaySnd(KEnvironment& kenv, RwSound& snd) {
		InitSnd(snd.info.dings[0].sampleRate, kenv.platform == KEnvironment::PLATFORM_X360 || kenv.platform == KEnvironment::PLATFORM_PS3);
		SDL_QueueAudio(audiodevid, snd.data.data.data(), snd.data.data.size());
	}
}