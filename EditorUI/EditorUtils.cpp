#include "EditorUtils.h"
#include <cassert>
#include <numbers>
#include <SDL2/SDL_audio.h>
#include "KEnvironment.h"
#include "rwsound.h"
#include "WavDocument.h"

namespace EditorUI
{
	static bool audioInitDone = false;
	static SDL_AudioDeviceID audiodevid;
	static int audioLastFreq = 0;
	static int audioLastNumChannels = 0;

	void InitSnd(int freq, int numChannels, bool byteSwapped) {
		if (audioInitDone && audioLastFreq == freq && audioLastNumChannels == numChannels) {
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
		spec.channels = numChannels;
		spec.samples = 4096;
		audiodevid = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);
		assert(audiodevid);
		SDL_PauseAudioDevice(audiodevid, 0);
		audioInitDone = true;
		audioLastFreq = freq;
		audioLastNumChannels = numChannels;
	}

	void PlaySnd(const KEnvironment& kenv, const RwSound& snd) {
		InitSnd(snd.info.format.sampleRate, 1, kenv.platform == KEnvironment::PLATFORM_X360 || kenv.platform == KEnvironment::PLATFORM_PS3);
		SDL_QueueAudio(audiodevid, snd.data.data.data(), snd.data.data.size());
	}

	void PlaySnd(const WavDocument& wav) {
		InitSnd(wav.samplesPerSec, wav.numChannels, false);
		SDL_QueueAudio(audiodevid, wav.data.data(), wav.data.size());
	}

	float decode8bitAngle(uint8_t byte) {
		return byte * std::numbers::pi_v<float> / 128.0f;
	}
}