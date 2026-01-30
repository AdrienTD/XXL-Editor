#include "IGSekensEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "LocaleEditor.h"
#include "GuiUtils.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include "rwsound.h"
#include "WavDocument.h"

#include <imgui/imgui.h>
#include <fmt/format.h>
#include "adpcm-xq/adpcm-lib.h"

namespace {
	void ExportSekensStreamFiles(EditorUI::EditorInterface& ui, int& selectedSekensIndex)
	{
		auto& kenv = ui.kenv;
		CKSrvSekensor* srvSekensor = kenv.levelObjects.getFirst<CKSrvSekensor>();
		const auto& selectedSekens = srvSekensor->sekens[selectedSekensIndex];

		const int numLanguages = ui.g_localeEditor->documents.front().locpack.get<Loc_CLocManager>()->numLanguages;

		for (int langIndex = 0; langIndex < numLanguages; ++langIndex) {
			auto* locSekensor = ui.g_localeEditor->documents.at(langIndex).lvlLocpacks.at(ui.levelNum).get<Loc_CKSrvSekensor>();
			locSekensor->locSekens.resize(srvSekensor->sekens.size());
			auto* locSekens = &locSekensor->locSekens.at(selectedSekensIndex);
			*locSekens = Loc_CKSrvSekensor::LocalizedSekens();

			int streamSampleRate = 0;

			RwStream rws;
			rws.info.basicSectorSize = 0x800;
			rws.info.streamSectorSize = 0x1000;
			rws.info.basicSectorSize2 = 0x800;
			strcpy_s(rws.info.streamName.data(), rws.info.streamName.size(), "Stream0");

			rws.info.samplesPerFrame = 0x40;
			rws.info.subSectorSize = 0x1000;
			rws.info.channelInterleaveSize = 0x24;
			rws.info.audioFrameSize = 0x24; // 36 size of XBOX ADPCM block
			rws.info.repeatChannels = 0x39; // 57 = 0x804 / 36, num of XBOX ADPCM blocks per sector
			rws.info.subSectorUsedSize = 0x804;
			strcpy_s(rws.info.subName.data(), rws.info.subName.size(), "SubStream0");

			rws.info.waveFormat.bitsPerSample = 4;
			rws.info.waveFormat.numChannels = 1;
			rws.info.waveFormat.uuid = RWA_FORMAT_ID_XBOX_ADPCM;

			const int adpcmSamplesPerBlock = 64; // in XBOX ADPCM, only 4 bits is used in the last byte
			const int adpcmBlockSize = 36;
			const int adpcmBlocksPerSector = 57;
			const int samplesPerSector = adpcmSamplesPerBlock * adpcmBlocksPerSector;

			bool showInconsistentSamplerateWarning = true;

			for (int segment = 0; segment < selectedSekens->sekLines.size(); ++segment) {
				auto wavPath = std::filesystem::u8path(kenv.gamePath) / fmt::format("Speech/{}/{:04}.wav", langIndex, selectedSekens->sekLines[segment].mUnk0);
				if (!std::filesystem::exists(wavPath)) {
					locSekens->locLines.emplace_back().duration = 0.0f;
					continue;
				}

				std::vector<int16_t> input;
				int segmentSampleRate = 0;
				{
					IOFile wavFile(wavPath.c_str(), "rb");
					WavDocument	doc;
					doc.read(&wavFile);
					if (streamSampleRate == 0) {
						streamSampleRate = doc.samplesPerSec;
						rws.info.waveFormat.sampleRate = streamSampleRate;
					}
					segmentSampleRate = doc.samplesPerSec;
					WavSampleReader wavReader(&doc);

					while (wavReader.available()) {
						input.push_back((int16_t)(wavReader.getSample(0) * 32767.0f));
						wavReader.nextSample();
					}
				}
				int numSamples = input.size();

				// resample segment if the wav sample rate is different than the first one
				if (segmentSampleRate != streamSampleRate) {
					if (showInconsistentSamplerateWarning) {
						GuiUtils::MsgBox_Ok(ui.g_window,
							"The samplerate of the segments' WAV files used for this Sekens are inconsistent.\n"
							"All segments will be resampled to the samplerate of the first one.\n"
							"The resampling might not be perfect.", GuiUtils::MsgBoxIcon::Warning);
						showInconsistentSamplerateWarning = false;
					}
					std::vector<int16_t> resampledInput;
					float oldSamplePos = 0.0;
					float oldSampleIncrement = (float)segmentSampleRate / (float)streamSampleRate;
					while ((int)oldSamplePos < numSamples) {
						resampledInput.push_back(input[(size_t)oldSamplePos]);
						oldSamplePos += oldSampleIncrement;
					}
					numSamples = resampledInput.size();
					input = std::move(resampledInput);
				}

				// align input to samples per sector to avoid garbage noice played at the end
				input.resize((input.size() + samplesPerSector - 1) / samplesPerSector * samplesPerSector);
				std::fill(input.begin() + numSamples, input.end(), 0);

				const int sectorSize = 0x1000;
				int segmentUsedSize = 0;

				const int numSectors = (numSamples + samplesPerSector - 1) / samplesPerSector;
				const int numAdpcmBlocks = (numSamples + adpcmSamplesPerBlock - 1) / adpcmSamplesPerBlock;

				auto& segInfo = rws.info.segments.emplace_back();
				rws.info.numSegments += 1;
				segInfo.dataOffset = rws.data.size();

				void* codec = adpcm_create_context(1, streamSampleRate, 1, 1);
				std::vector<uint8_t> output(0x1000);
				size_t outSize;
				for (int sector = 0; sector < numSectors; ++sector) {
					const int remainingBlocks = std::min(adpcmBlocksPerSector, numAdpcmBlocks - sector * adpcmBlocksPerSector);
					for (int block = 0; block < remainingBlocks; ++block) {
						const int sampleIndex = samplesPerSector * sector + adpcmSamplesPerBlock * block;
						adpcm_encode_block(codec, output.data() + block * adpcmBlockSize, &outSize, input.data() + sampleIndex, adpcmSamplesPerBlock);
						segmentUsedSize += outSize;
					}
					rws.data.insert(rws.data.end(), output.begin(), output.end());
				}

				segInfo.dataSize = segmentUsedSize;
				segInfo.dataAlignedSize = sectorSize * numSectors;
				sprintf_s(segInfo.name.data(), segInfo.name.size(), "Segment%i", segment);

				adpcm_free_context(codec);

				auto& locLine = locSekens->locLines.emplace_back();
				locLine.duration = (float)numSamples / (float)streamSampleRate;
				locSekens->totalTime += locLine.duration;
				locSekens->numVoiceLines += 1;
			}

			auto outDir = std::filesystem::u8path(kenv.outGamePath);
			auto outFilePath = outDir / fmt::format("LVL{0:03}/WINAS/SPEECH/{1}/{1}_WIN{2}.RWS", ui.levelNum, langIndex, selectedSekensIndex);
			std::filesystem::create_directories(outFilePath.parent_path());
			IOFile outFile(outFilePath.c_str(), "wb");
			rws.serialize(&outFile);
		}
	}
}

void EditorUI::IGSekensEditor(EditorInterface& ui)
{
	static KWeakRef<CKSekens> selectedSekens;

	auto& kenv = ui.kenv;
	CKSrvSekensor* srvSekensor = kenv.levelObjects.getFirst<CKSrvSekensor>();
	CLocManager* locManager = kenv.getGlobal<CLocManager>();
	bool mainTableOpen = ImGui::BeginTable("SekensEditorMainTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail());
	if (!mainTableOpen)
		return;
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	if (kenv.version <= KEnvironment::KVERSION_XXL2 && ImGui::Button("New sekens")) {
		CKSekens* newSekens = kenv.createAndInitObject<CKSekens>();
		srvSekensor->sekens.emplace_back(newSekens);
	}
	ImGui::BeginChild("SekensList");
	int sekIndex = 0;
	for (auto& sek : srvSekensor->sekens) {
		ImGui::PushID(sek.get());
		if (ImGui::Selectable("##SekensSel", sek.get() == selectedSekens.get()))
			selectedSekens = sek.get();
		ImGui::SameLine();
		ImGui::Text("%2i: %s", sekIndex++, kenv.getObjectName(sek.get()));
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::TableNextColumn();
	ImGui::BeginChild("SekensInfo");
	if (selectedSekens) {
		auto selectedSekensIterator = std::find_if(srvSekensor->sekens.begin(), srvSekensor->sekens.end(), [](kanyobjref& ref) {return ref.get() == selectedSekens.get(); });
		int selectedSekensIndex = selectedSekensIterator - srvSekensor->sekens.begin();
		if (ImGui::BeginTabBar("SekensBar")) {
			if (ImGui::BeginTabItem("Simple")) {
				static int selectedLanguage = 0;

				ImGui::BeginDisabled(!ui.g_localeEditor);
				if (kenv.platform == KEnvironment::PLATFORM_PC && kenv.version <= KEnvironment::KVERSION_XXL2 && ImGui::Button("Update stream files (.RWS)")) {
					ExportSekensStreamFiles(ui, selectedSekensIndex);
				}
				ImGui::EndDisabled();

				ImGui::InputInt("Language", &selectedLanguage);
				IGObjectNameInput("Name", selectedSekens.get(), kenv);
				if (kenv.version <= KEnvironment::KVERSION_XXL2) {
					bool skippableLines = selectedSekens->sekSkippable;
					if (ImGui::Checkbox("Skippable lines", &skippableLines))
						selectedSekens->sekSkippable = skippableLines ? 1 : 0;
				}

				ImGui::Separator();

				static int selectedLine = -1;
				if (kenv.version <= KEnvironment::KVERSION_XXL2) {
					bool selectedLineValid = selectedLine >= 0 && selectedLine < selectedSekens->sekLines.size();
					if (ImGui::Button("Add")) {
						selectedSekens->sekLines.emplace_back();
					}
					ImGui::SameLine();
					ImGui::BeginDisabled(!selectedLineValid);
					if (ImGui::Button("Remove") && selectedLineValid) {
						selectedSekens->sekLines.erase(selectedSekens->sekLines.begin() + selectedLine);
					}
					ImGui::EndDisabled();
				}

				static int movingLine = -1;
				if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
					movingLine = -1;

				if (ImGui::BeginTable("LineTable", 5, ImGuiTableFlags_Borders)) {
					ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 32.0f);
					ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("LocDuration", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("Text ID", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("Localized text");
					ImGui::TableHeadersRow();
					size_t numLines = (kenv.version < kenv.KVERSION_OLYMPIC) ? selectedSekens->sekLines.size() : selectedSekens->ogLines.size();
					float totalLocDuration = 0.0f;
					int ogStdTextIndex = 0;
					for (size_t i = 0; i < numLines; ++i) {
						int* pIndex = nullptr;
						float* pDuration = nullptr;
						if (kenv.version < kenv.KVERSION_OLYMPIC) {
							pIndex = (int*)&selectedSekens->sekLines[i].mUnk0;
							pDuration = &selectedSekens->sekLines[i].mUnk1;
						}
						else if (kenv.version >= kenv.KVERSION_OLYMPIC) {
							pDuration = &selectedSekens->ogLines[i]->skbkUnk1;
							if (auto* block = selectedSekens->ogLines[i]->dyncast<CKSekensBlock>()) {
								pIndex = &ogStdTextIndex;
								auto& strList = locManager->stdStringRefs;
								if (auto it = std::find(strList.begin(), strList.end(), block->skbkTextRef); it != strList.end())
									ogStdTextIndex = it - strList.begin();
								else
									ogStdTextIndex = -1;
							}
						}
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::PushID(i);
						ImGui::AlignTextToFramePadding();
						if (ImGui::Selectable("##SekLineSel", selectedLine == i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
							selectedLine = i;
						}
						if (movingLine >= 0 && i != movingLine && ImGui::IsItemHovered()) {
							if (kenv.version <= KEnvironment::KVERSION_XXL2) {
								auto temp = std::move(selectedSekens->sekLines[movingLine]);
								selectedSekens->sekLines.erase(selectedSekens->sekLines.begin() + movingLine);
								selectedSekens->sekLines.insert(selectedSekens->sekLines.begin() + i, std::move(temp));
								movingLine = i;
							}
						}
						if (ImGui::IsItemActivated()) {
							movingLine = i;
						}
						ImGui::SameLine();
						ImGui::Text("%2i", (int)i);
						ImGui::TableNextColumn();
						ImGui::SetNextItemWidth(48.0f);
						ImGui::InputScalar("##duration", ImGuiDataType_Float, pDuration);
						ImGui::TableNextColumn();
						if (ui.g_localeEditor) {
							try {
								float locDuration = ui.g_localeEditor->documents.at(selectedLanguage).lvlLocpacks.at(ui.levelNum).get<Loc_CKSrvSekensor>()->locSekens.at(selectedSekensIndex).locLines.at(i).duration;
								ImGui::Text("%.3f", locDuration);
								totalLocDuration += locDuration;
							}
							catch (const std::out_of_range&) {
								ImGui::TextUnformatted("/");
							}
						}
						ImGui::TableNextColumn();
						if (pIndex) {
							ImGui::SetNextItemWidth(48.0f);
							bool modified = ImGui::InputScalar("##LineID", ImGuiDataType_S32, pIndex);
							if (modified && kenv.version >= kenv.KVERSION_OLYMPIC) {
								CKObject* newPtr = (0 <= *pIndex && (size_t)*pIndex < locManager->stdStringRefs.size()) ? locManager->stdStringRefs[*pIndex].get() : nullptr;
								selectedSekens->ogLines[i]->dyncast<CKSekensBlock>()->skbkTextRef = newPtr;
							}
						}
						ImGui::TableNextColumn();
						if (pIndex) {
							const char* text = "Please open the Localization window first.";
							if (*pIndex == -1) {
								text = "/";
							}
							else if (ui.g_localeEditor) {
								try {
									text = ui.g_localeEditor->documents.at(selectedLanguage).stdTextU8.at(*pIndex).c_str();
								}
								catch (const std::out_of_range&) {
									text = "(not found)";
								}
							}
							ImGui::TextUnformatted(text);
						}
						ImGui::PopID();
					}
					ImGui::EndTable();
					ImGui::Text("Total localized duration: %f", totalLocDuration);
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Advanced")) {
				ImGuiMemberListener igml{ kenv, ui };
				igml.setPropertyInfoList(ui.g_encyclo, selectedSekens.get());
				selectedSekens->virtualReflectMembers(igml, &kenv);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::EndChild();
	ImGui::EndTable();
}
