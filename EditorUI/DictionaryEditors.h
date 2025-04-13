#pragma once

struct CAnimationDictionary;
struct CKSoundDictionaryID;

namespace EditorUI
{
	struct EditorInterface;

	void AnimDictEditor(EditorInterface& ui, CAnimationDictionary* animDict, bool showHeader = true);
	void SoundDictIDEditor(EditorInterface& ui, CKSoundDictionaryID* sndDictID, bool showHeader = true);
}