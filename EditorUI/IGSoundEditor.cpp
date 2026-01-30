#include "IGSoundEditor.h"

#include "EditorInterface.h"
#include "EditorUtils.h"
#include "GuiUtils.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKDictionary.h"

#include "rwsound.h"
#include "WavDocument.h"

#include <imgui/imgui.h>

void EditorUI::IGSoundEditor(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;

	static auto massByteSwap = [](void* data, size_t numBytes) {
		assert((numBytes & 1) == 0);
		size_t numShorts = numBytes / 2;
		uint16_t* data16 = (uint16_t*)data;
		for (size_t i = 0; i < numShorts; ++i)
			data16[i] = ((data16[i] & 255) << 8) | (data16[i] >> 8);
		};
	static auto exportSound = [](RwSound& snd, const std::filesystem::path& path, KEnvironment& kenv) {
		WavDocument wav;
		wav.formatTag = 1;
		wav.numChannels = 1;
		wav.samplesPerSec = snd.info.format.sampleRate;
		wav.avgBytesPerSec = wav.samplesPerSec * 2;
		wav.pcmBitsPerSample = 16;
		wav.blockAlign = ((wav.pcmBitsPerSample + 7) / 8) * wav.numChannels;
		wav.data = snd.data.data;
		if (snd.info.isBigEndian) {
			massByteSwap(wav.data.data(), wav.data.size());
		}
		IOFile out = IOFile(path.c_str(), "wb");
		wav.write(&out);
		};
	auto enumDict = [&](CKSoundDictionary* sndDict, int strnum) {
		if (sndDict->sounds.empty() || sndDict->rwSoundDict.list.sounds.empty())
			return;
		const bool formatSupported = sndDict->rwSoundDict.list.sounds[0].info.format.uuid == RWA_FORMAT_ID_PCM;
		if (ImGui::TreeNode(sndDict, (strnum == 0) ? "Level" : "Sector %i", strnum)) {
			if (formatSupported && ImGui::Button("Export all")) {
				auto dirpath = GuiUtils::SelectFolderDialogBox(ui.g_window, "Export all the sounds to folder:");
				if (!dirpath.empty()) {
					for (auto& snd : sndDict->rwSoundDict.list.sounds) {
						const char* np = strrchr((const char*)snd.info.name.data(), '\\');
						np = np ? np + 1 : (const char*)snd.info.name.data();
						exportSound(snd, dirpath / np, kenv);
					}
				}
			}
			for (int sndid = 0; sndid < (int)sndDict->sounds.size(); sndid++) {
				auto& snd = sndDict->rwSoundDict.list.sounds[sndid];
				ImGui::PushID(sndid);
				ImGui::AlignTextToFramePadding();
				ImGui::BeginGroup();
				ImGui::Text("%3i", sndid);
				if (formatSupported) {
					ImGui::SameLine();
					if (ImGui::ArrowButton("PlaySound", ImGuiDir_Right))
						PlaySnd(snd);
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Play");
					ImGui::SameLine();
					if (ImGui::Button("I")) {
						auto filepath = OpenDialogBox(ui.g_window, "WAV audio file\0*.WAV\0\0", "wav");
						if (!filepath.empty()) {
							IOFile wf = IOFile(filepath.c_str(), "rb");
							WavDocument wav;
							wav.read(&wf);
							WavSampleReader wsr(&wav);
							if (wav.formatTag == 1 || wav.formatTag == 3) {
								if (wav.numChannels != 1) {
									MsgBox_Ok(ui.g_window, "The WAV contains multiple channels (e.g. stereo).\nOnly the first channel will be imported.", MsgBoxIcon::Warning);
								}

								size_t numSamples = wav.getNumSamples();
								auto& ndata = snd.data.data;
								ndata.resize(numSamples * 2);
								int16_t* pnt = (int16_t*)ndata.data();
								for (size_t i = 0; i < numSamples; i++) {
									*(pnt++) = (int16_t)(wsr.getSample(0) * 32767);
									wsr.nextSample();
								}
								if (snd.info.isBigEndian)
									massByteSwap(ndata.data(), ndata.size());

								for (auto* format : { &snd.info.format, &snd.info.targetFormat }) {
									format->sampleRate = wav.samplesPerSec;
									format->dataSize = ndata.size();
								}
								sndDict->sounds[sndid].sampleRate = wav.samplesPerSec;
							}
							else {
								MsgBox_Ok(ui.g_window, "The WAV file doesn't contain uncompressed mono 8/16-bit PCM wave data.\nPlease convert it to this format first.", MsgBoxIcon::Warning);
							}
						}
					}
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import");
					ImGui::SameLine();
					if (ImGui::Button("E")) {
						const char* name = strrchr((const char*)snd.info.name.data(), '\\');
						if (name) name++;
						auto filepath = SaveDialogBox(ui.g_window, "WAV audio file\0*.WAV\0\0", "wav", name);
						if (!filepath.empty()) {
							exportSound(snd, filepath, kenv);
						}
					}
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export");
				}
				ImGui::SameLine();
				const char* name = (const char*)snd.info.name.data();
				ImGui::Text("%s", latinToUtf8(name).c_str());
				//ImGui::Text("%u %u", snd.info.dings[0].sampleRate, snd.info.dings[1].sampleRate);
				ImGui::EndGroup();
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
					int32_t fullIndex = (strnum << 24) | sndid;
					ImGui::SetDragDropPayload("SoundIndex", &fullIndex, sizeof(fullIndex));
					ImGui::Text("Sound Sector %i ID %i", strnum, sndid);
					ImGui::EndDragDropSource();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		};
	enumDict(kenv.levelObjects.getFirst<CKSoundDictionary>(), 0);
	for (int i = 0; i < (int)kenv.numSectors; i++)
		enumDict(kenv.sectorObjects[i].getFirst<CKSoundDictionary>(), i + 1);
}
