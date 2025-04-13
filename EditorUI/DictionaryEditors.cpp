#include "DictionaryEditors.h"
#include "EditorInterface.h"
#include "KEnvironment.h"
#include "GuiUtils.h"
#include "EditorUtils.h"
#include "ImGuiMemberListener.h"

#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"

#include <imgui/imgui.h>

void EditorUI::AnimDictEditor(EditorInterface& ui, CAnimationDictionary* animDict, bool showHeader) {
	CAnimationManager* animMgr = ui.kenv.levelObjects.getFirst<CAnimationManager>();
	ImGui::PushID(animDict);
	ImGui::Indent();
	if (!showHeader || ImGui::CollapsingHeader("Animation Dictionary")) {
		ImGui::BeginChild("AnimDictEdit", ImVec2(0, 250.0f), true);
		ImGui::Columns(animDict->numSets); // TODO: correct order for Arthur sets
		for (size_t i = 0; i < animDict->animIndices.size(); ++i) {
			ImGui::PushID(i);
			ImGui::AlignTextToFramePadding();

			uint32_t animFullIndex = animDict->animIndices[i];
			uint32_t animSector = animFullIndex >> 24;
			uint32_t animIndex = animFullIndex & 0xFFFFFF;

			CSectorAnimation* secAnim = (animFullIndex == -1) ? nullptr : (ui.kenv.version < KEnvironment::KVERSION_ARTHUR) ? &animMgr->commonAnims : animMgr->arSectors[animSector].get();
			if ((i % animDict->numSets) == 0) {
				ImGui::Text("%2i:", (uint32_t)i / animDict->numSets);
				ImGui::SameLine();
			}
			ImGui::BeginDisabled(animFullIndex == -1);
			if (ImGui::ArrowButton("Play", ImGuiDir_Right)) {
				ui.selectedAnimationIndex = animIndex;
				ui.selectedAnimationSector = animSector;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show in Anim viewer");
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("A")) {
				auto anmpath = GuiUtils::OpenDialogBox(ui.g_window, "Renderware Animation (*.anm)\0*.ANM\0\0", "anm");
				if (!anmpath.empty()) {
					IOFile file = IOFile(anmpath.c_str(), "rb");
					RwAnimAnimation rwAnim;
					auto rwVerBackup = HeaderWriter::rwver; // TODO: Remove hack
					rwCheckHeader(&file, 0x1B);
					rwAnim.deserialize(&file);
					HeaderWriter::rwver = rwVerBackup;
					int32_t newIndex = animMgr->addAnimation(rwAnim, animDict->arSector);
					animDict->animIndices[i] = newIndex;
				}
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import from .ANM (unique)");
			ImGui::SameLine();
			ImGui::BeginDisabled(animFullIndex == -1);
			if (ImGui::Button("I")) {
				auto anmpath = GuiUtils::OpenDialogBox(ui.g_window, "Renderware Animation (*.anm)\0*.ANM\0\0", "anm");
				if (!anmpath.empty()) {
					IOFile file = IOFile(anmpath.c_str(), "rb");
					RwAnimAnimation& rwAnim = secAnim->anims[animIndex].rwAnim;
					rwAnim = {};
					auto rwVerBackup = HeaderWriter::rwver; // TODO: Remove hack
					rwCheckHeader(&file, 0x1B);
					rwAnim.deserialize(&file);
					HeaderWriter::rwver = rwVerBackup;
				}
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import from .ANM (shared)");
			ImGui::SameLine();
			if (ImGui::Button("E")) {
				auto anmpath = GuiUtils::SaveDialogBox(ui.g_window, "Renderware Animation (*.anm)\0*.ANM\0\0", "anm");
				if (!anmpath.empty()) {
					IOFile file = IOFile(anmpath.c_str(), "wb");
					RwAnimAnimation& rwAnim = secAnim->anims[animIndex].rwAnim;
					rwAnim.serialize(&file);
				}
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export to .ANM");
			ImGui::SameLine();
			if (animFullIndex != -1)
				ImGui::Text("Sec %2u, Index %3u", animSector, animIndex);
			else
				ImGui::TextUnformatted("None");
			ImGui::EndDisabled();
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		if (ImGui::Button("New slot")) {
			for (uint32_t i = 0; i < animDict->numSets; ++i) {
				animDict->animIndices.emplace_back(-1);
			}
			animDict->numAnims += 1;
		}
		ImGui::EndChild();
	}
	ImGui::Unindent();
	ImGui::PopID();
}

void EditorUI::SoundDictIDEditor(EditorInterface& ui, CKSoundDictionaryID* sndDictID, bool showHeader) {
	if (ui.kenv.version >= KEnvironment::KVERSION_XXL2 && !ui.kenv.hasClass<CKSound>())
		return;
	ImGui::PushID(sndDictID);
	ImGui::Indent();
	if (!showHeader || ImGui::CollapsingHeader("Sound ID Dictionary")) {
		ImGui::BeginChild("SndDictIDEdit", ImVec2(0, 500.0f/*120.0f*/), true);
		ImGui::Text("Ref count: %i", sndDictID->getRefCount());
		int numSlots = (ui.kenv.version >= KEnvironment::KVERSION_XXL2) ? sndDictID->x2Sounds.size() : sndDictID->soundEntries.size();
		for (int i = 0; i < numSlots; ++i) {
			bool slotActive;
			int sectorIndex = 0;
			int soundId = -1;
			if (ui.kenv.version >= KEnvironment::KVERSION_ARTHUR) {
				CKSound* snd = sndDictID->x2Sounds[i]->cast<CKSound>();
				slotActive = snd->sndWaveObj.get();
				if (slotActive) {
					// we need to find the CKWave's corresponding sector+id
					CKLevel* level = ui.kenv.levelObjects.getFirst<CKLevel>();
					for (int str = 0; str < level->sectors.size() && soundId == -1; ++str) {
						CKSoundDictionary* dict = level->sectors[str]->soundDictionary.get() ? level->sectors[str]->soundDictionary->cast<CKSoundDictionary>() : nullptr;
						if (dict) {
							for (int id = 0; id < dict->sounds.size() && soundId == -1; ++id) {
								if (dict->sounds[id].waveObj.get() == snd->sndWaveObj.get()) {
									// found
									sectorIndex = str;
									soundId = id;
								}
							}
						}
					}
					assert(soundId != -1);
				}
			}
			else if (ui.kenv.version >= KEnvironment::KVERSION_XXL2) {
				CKSound* snd = sndDictID->x2Sounds[i]->cast<CKSound>();
				slotActive = (snd->sndIndex & 0xFFFFFF) != 0xFFFFFF;
				if (slotActive) {
					sectorIndex = snd->sndIndex >> 24;
					soundId = snd->sndIndex & 0xFFFFFF;
				}
			}
			else {
				auto& snd = sndDictID->soundEntries[i];
				slotActive = snd.active;
				sectorIndex = snd.id >> 24;
				soundId = snd.id & 0xFFFFFF;
			}
			CKSoundDictionary* sndDict = nullptr;
			if (slotActive)
				sndDict = ui.kenv.levelObjects.getFirst<CKLevel>()->sectors[sectorIndex]->soundDictionary->cast<CKSoundDictionary>();
			ImGui::PushID(i);
			ImGui::AlignTextToFramePadding();
			ImGui::BeginGroup();
			bool enabled = slotActive && soundId >= 0 && soundId < sndDict->sounds.size();
			ImGui::BeginDisabled(!enabled);
			if (ImGui::ArrowButton("PlaySound", ImGuiDir_Right) && enabled) {
				PlaySnd(ui.kenv, sndDict->rwSoundDict.list.sounds[soundId]);
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::Text("Slot %2i:", i);
			ImGui::SameLine();
			if (slotActive) {
				ImGui::Text("Sector %i ID %i", sectorIndex, soundId);
			}
			else {
				ImGui::TextDisabled("No sound");
			}
			ImGui::EndGroup();
			if (ImGui::BeginDragDropTarget()) {
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SoundIndex");
				if (payload) {
					int32_t fullIndex = *(int32_t*)payload->Data;
					if (ui.kenv.version >= KEnvironment::KVERSION_ARTHUR) {
						int sector = fullIndex >> 24;
						int id = fullIndex & 0xFFFFFF;
						sndDictID->x2Sounds[i]->cast<CKSound>()->sndWaveObj = ui.kenv.levelObjects.getFirst<CKLevel>()->sectors[sector]->soundDictionary->cast<CKSoundDictionary>()->sounds[id].waveObj.get();
					}
					else if (ui.kenv.version >= KEnvironment::KVERSION_XXL2)
						sndDictID->x2Sounds[i]->cast<CKSound>()->sndIndex = fullIndex;
					else
						sndDictID->soundEntries[i].id = fullIndex;
				}
				ImGui::EndDragDropTarget();
			}
			ImGui::SameLine();
			if (ImGui::TreeNode("More")) {
				ImGuiMemberListener igml(ui.kenv, ui);
				MemberListener& ml = igml;
				if (ui.kenv.version >= KEnvironment::KVERSION_XXL2) {
					igml.setPropertyInfoList(ui.g_encyclo, sndDictID->x2Sounds[i].get());
					sndDictID->x2Sounds[i]->cast<CKSound>()->virtualReflectMembers(ml, &ui.kenv);
				}
				else {
					auto& e = sndDictID->soundEntries[i];
					ImGui::Checkbox("Active", &e.active);
					ml.reflect(e.id, "id");
					ml.reflect(e.flags, "flags");
					bool onSceneNode = e.flags & 16;
					ImGui::CheckboxFlags("Playing", &e.flags, 8);
					ImGui::TextUnformatted("Play on:");
					ImGui::SameLine();
					if (ImGui::RadioButton("Position", !onSceneNode))
						e.flags &= ~16;
					ImGui::SameLine();
					if (ImGui::RadioButton("Scene node", onSceneNode))
						e.flags |= 16;
					if (e.flags & 16)
						ml.reflect(e.obj, "obj");
					else
						ml.reflect(e.refalt, "position");
					ml.reflect(e.volume, "volume");
					ml.reflect(e.speed, "speed");
					ml.reflect(e.replayAfterMin, "replayAfterMin");
					ml.reflect(e.replayAfterMax, "replayAfterMax");
					ml.reflect(e.boxHigh, "boxHigh");
					ml.reflect(e.boxLow, "boxLow");
					bool pil = e.playInLoop;
					if (ImGui::Checkbox("Play in loop", &pil))
						e.playInLoop = pil;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (true) {
			if (ImGui::Button("New slot")) {
				if (ui.kenv.version == KEnvironment::KVERSION_XXL1) {
					sndDictID->soundEntries.emplace_back();
				}
				else {
					CKSound* ksound = ui.kenv.createAndInitObject<CKSound>();
					sndDictID->x2Sounds.emplace_back() = ksound;
				}
			}
		}
		ImGui::EndChild();
	}
	ImGui::Unindent();
	ImGui::PopID();
}
