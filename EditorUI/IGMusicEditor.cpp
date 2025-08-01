#include "IGMusicEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "EditorUtils.h"
#include "ImGuiMemberListener.h"
#include "GuiUtils.h"

#include "KEnvironment.h"
#include "CoreClasses/CKManager.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include "WavDocument.h"
#include "rwsound.h"

#include <imgui/imgui.h>
#include <adpcm-xq/adpcm-lib.h>
#include <fmt/format.h>

namespace {
	WavDocument ConvertAudioStreamToWav(const std::filesystem::path& rwsPath)
	{
		IOFile file(rwsPath.c_str(), "rb");
		RwStream rws;
		auto header = rwReadHeader(&file);
		assert(header.type == 0x80D);
		rws.deserialize(&file);

		const int usedSize = rws.info.segments[0].dataSize;
		const int sectorSize = rws.info.streamSectorSize;
		const int usedSectorSize = rws.info.subSectorUsedSize;
		const int numSectors = (usedSize + usedSectorSize - 1) / usedSectorSize;
		const int numChannels = rws.info.waveFormat.numChannels;

		WavDocument wav;
		wav.formatTag = 1;
		wav.numChannels = numChannels;
		wav.samplesPerSec = rws.info.waveFormat.sampleRate;
		wav.avgBytesPerSec = wav.samplesPerSec * numChannels * 2;
		wav.blockAlign = numChannels * 2;
		wav.pcmBitsPerSample = 16;

		const int blockSize = 36 * numChannels;
		std::vector<int16_t> decBlock(65 * numChannels);

		std::vector<int16_t> decWave;
		for (int sector = 0; sector < numSectors; ++sector) {
			const int offset = sector * sectorSize;
			const int usedThisSectorSize = std::min(usedSize - sector * usedSectorSize, usedSectorSize);
			for (int byte = 0; byte < usedThisSectorSize; byte += blockSize) {
				adpcm_decode_block(decBlock.data(), rws.data.data() + offset + byte, blockSize, numChannels);
				decWave.insert(decWave.end(), decBlock.begin(), decBlock.begin() + 64 * numChannels);
			}
		}
		wav.data.resize(decWave.size() * 2);
		memcpy(wav.data.data(), decWave.data(), decWave.size() * 2);

		return wav;
	}

	void ConvertWavToAudioStream(const WavDocument& wav, const std::filesystem::path& rwsPath)
	{
		std::vector<int16_t> input;
		const int segmentSampleRate = wav.samplesPerSec;
		WavSampleReader wavReader(&wav);
		while (wavReader.available()) {
			for (int ch = 0; ch < wav.numChannels; ++ch) {
				input.push_back((int16_t)(wavReader.getSample(ch) * 32767.0f));
			}
			wavReader.nextSample();
		}

		const int numChannels = wav.numChannels;
		const int numSamples = input.size() / numChannels;
		const int streamSampleRate = wav.samplesPerSec;
		const int sectorSize = 0x8800;
		const int usedSectorSize = 0x8040;

		RwStream rws;

		rws.info.basicSectorSize = 0x800;
		rws.info.streamSectorSize = sectorSize;
		rws.info.basicSectorSize2 = 0x800;
		strcpy_s(rws.info.streamName.data(), rws.info.streamName.size(), "Stream0");

		rws.info.samplesPerFrame = 7; // unused/overwritten
		rws.info.subSectorSize = sectorSize;
		rws.info.channelInterleaveSize = 4; // unused/overwritten
		rws.info.audioFrameSize = 4; // unused/overwritten
		rws.info.repeatChannels = usedSectorSize / (4 * numChannels); //4104
		rws.info.subSectorUsedSize = usedSectorSize;

		rws.info.waveFormat.sampleRate = streamSampleRate;
		rws.info.waveFormat.bitsPerSample = 4;
		rws.info.waveFormat.numChannels = numChannels;
		rws.info.waveFormat.uuid = RWA_FORMAT_ID_XBOX_ADPCM;

		strcpy_s(rws.info.subName.data(), rws.info.subName.size(), "SubStream0");

		const int adpcmSamplesPerBlock = 64; // in XBOX ADPCM, only 4 bits is used in the last byte
		const int adpcmBlockSize = 36 * numChannels;
		const int adpcmBlocksPerSector = usedSectorSize / adpcmBlockSize;
		const int samplesPerSector = adpcmSamplesPerBlock * adpcmBlocksPerSector;
		const int coordsPerSector = samplesPerSector * numChannels;

		// align input to samples per sector to avoid garbage noice played at the end
		input.resize((input.size() + coordsPerSector - 1) / coordsPerSector * coordsPerSector);
		std::fill(input.begin() + numSamples * numChannels, input.end(), 0);

		int segmentUsedSize = 0;

		const int numSectors = (numSamples + samplesPerSector - 1) / samplesPerSector;
		const int numAdpcmBlocks = (numSamples + adpcmSamplesPerBlock - 1) / adpcmSamplesPerBlock;

		auto& segInfo = rws.info.segments.emplace_back();
		rws.info.numSegments += 1;
		segInfo.dataOffset = rws.data.size();

		void* codec = adpcm_create_context(numChannels, streamSampleRate, 1, 1);
		std::vector<uint8_t> output(sectorSize);
		size_t outSize;
		for (int sector = 0; sector < numSectors; ++sector) {
			const int remainingBlocks = std::min(adpcmBlocksPerSector, numAdpcmBlocks - sector * adpcmBlocksPerSector);
			for (int block = 0; block < remainingBlocks; ++block) {
				const int sampleIndex = samplesPerSector * sector + adpcmSamplesPerBlock * block;
				adpcm_encode_block(codec, output.data() + block * adpcmBlockSize, &outSize, input.data() + sampleIndex * numChannels, adpcmSamplesPerBlock);
				segmentUsedSize += outSize;
			}
			rws.data.insert(rws.data.end(), output.begin(), output.end());
		}

		segInfo.dataSize = segmentUsedSize;
		segInfo.dataAlignedSize = sectorSize * numSectors;
		strcpy_s(segInfo.name.data(), segInfo.name.size(), "Segment0");

		adpcm_free_context(codec);

		IOFile file(rwsPath.c_str(), "wb");
		rws.serialize(&file);
	}

	std::string GetAudioStreamPath(const EditorUI::EditorInterface& ui, int levelIndex, int streamIndex)
	{
		std::string path;
		if (ui.kenv.version == KEnvironment::KVERSION_ARTHUR) {
			CKSoundManager* soundManager = ui.kenv.levelObjects.getFirst<CKSoundManager>();
			auto& tune = soundManager->ksndmgrDings[streamIndex];
			path = fmt::format("MUSIC/{:02}/WIN_{:03}.RWS", tune.arValue1, tune.arValue2);
		}
		else {
			path = fmt::format("LVL{:03}/WINAS/WINAS{}.RWS", levelIndex, streamIndex);
		}
		return path;
	}

	std::filesystem::path GetInputAudioStreamPath(const EditorUI::EditorInterface& ui, int levelIndex, int streamIndex, bool showErrorMessage = true, bool hasToExist = true)
	{
		std::filesystem::path path = std::filesystem::u8path(ui.kenv.outGamePath) / GetAudioStreamPath(ui, levelIndex, streamIndex);
		if (hasToExist && !std::filesystem::exists(path)) {
			if (showErrorMessage) {
				GuiUtils::MsgBox(ui.g_window, fmt::format("The stream file {} could not be found.", path.string()).c_str(), 16);
			}
			path.clear();
		}
		return path;
	}

	std::filesystem::path GetOutputAudioStreamPath(const EditorUI::EditorInterface& ui, int levelIndex, int streamIndex)
	{
		std::filesystem::path path = std::filesystem::u8path(ui.kenv.outGamePath) / GetAudioStreamPath(ui, levelIndex, streamIndex);
		std::filesystem::create_directories(path.parent_path());
		return path;
	}
}

void EditorUI::IGMusicEditor(EditorInterface& ui)
{
	if (ui.kenv.version == KEnvironment::KVERSION_XXL1) {
		IGAudioStreamEditor(ui);
	}
	else if (ui.kenv.version <= KEnvironment::KVERSION_ARTHUR) {
		if (ImGui::BeginTabBar("MusicTabBar")) {
			if (ImGui::BeginTabItem("Play list")) {
				IGPlayListEditor(ui);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Streams")) {
				IGAudioStreamEditor(ui);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	else {
		IGPlayListEditor(ui);
	}
}

void EditorUI::IGPlayListEditor(EditorInterface& ui)
{
	static KWeakRef<CKMusicPlayList> selectedPlayList;
	static int selectedTrackIndex = -1;

	auto& kenv = ui.kenv;

	if (!(kenv.hasClass<CKSrvMusic>() && kenv.hasClass<CKMusicPlayList>())) {
		ImGui::Text("Playlist editing not supported for this platform.");
		return;
	}

	CKSrvMusic* srvMusic = kenv.levelObjects.getFirst<CKSrvMusic>();
	CKSoundManager* soundManager = kenv.levelObjects.getFirst<CKSoundManager>();

	if (ImGui::Button("New playlist")) {
		srvMusic->playLists.emplace_back(kenv.createAndInitObject<CKMusicPlayList>());
	}
	ImGui::SetNextItemWidth(-1.0f);
	if (ImGui::BeginListBox("##PlayListListBox")) {
		for (auto& playList : srvMusic->playLists) {
			ImGui::PushID(playList.get());
			if (ImGui::Selectable("##PlayListSelect", playList.get() == selectedPlayList.get()))
				selectedPlayList = playList.get();
			ImGui::SameLine();
			ImGui::TextUnformatted(kenv.getObjectName(playList.get()));
			ImGui::PopID();
		}
		ImGui::EndListBox();
	}
	ImGui::Separator();
	bool streamsSeparated = kenv.version >= KEnvironment::KVERSION_OLYMPIC;
	if (selectedPlayList) {
		IGObjectNameInput("Playlist Name", selectedPlayList.get(), kenv);
		if (ImGui::Button("Add stream")) {
			if (streamsSeparated)
				selectedPlayList->ogStreams.emplace_back(kenv.createAndInitObject<CKStreamObject>(), 0);
			else
				selectedPlayList->x2Streams.emplace_back();
		}
		int numStreams = (int)(streamsSeparated ? selectedPlayList->ogStreams.size() : selectedPlayList->x2Streams.size());
		ImGui::SameLine();
		ImGui::BeginDisabled(!(0 <= selectedTrackIndex && selectedTrackIndex < numStreams));
		if (ImGui::ArrowButton("TrackDown", ImGuiDir_Down)) {
			if (selectedTrackIndex < numStreams - 1) {
				if (streamsSeparated)
					std::swap(selectedPlayList->ogStreams[selectedTrackIndex], selectedPlayList->ogStreams[selectedTrackIndex + 1]);
				else
					std::swap(selectedPlayList->x2Streams[selectedTrackIndex], selectedPlayList->x2Streams[selectedTrackIndex + 1]);
				selectedTrackIndex += 1;
			}

		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move stream down");
		ImGui::SameLine();
		if (ImGui::ArrowButton("TrackUp", ImGuiDir_Up)) {
			if (selectedTrackIndex > 0) {
				if (streamsSeparated)
					std::swap(selectedPlayList->ogStreams[selectedTrackIndex], selectedPlayList->ogStreams[selectedTrackIndex - 1]);
				else
					std::swap(selectedPlayList->x2Streams[selectedTrackIndex], selectedPlayList->x2Streams[selectedTrackIndex - 1]);
				selectedTrackIndex -= 1;
			}
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move stream up");
		ImGui::SameLine();
		if (ImGui::Button("Remove stream")) {
			if (0 <= selectedTrackIndex && selectedTrackIndex < numStreams) {
				if (streamsSeparated) {
					CKObject* obj = selectedPlayList->ogStreams[selectedTrackIndex].first.get();
					selectedPlayList->ogStreams.erase(selectedPlayList->ogStreams.begin() + selectedTrackIndex);
					if (obj)
						kenv.removeObject(obj);
				}
				else {
					selectedPlayList->x2Streams.erase(selectedPlayList->x2Streams.begin() + selectedTrackIndex);
				}
				numStreams -= 1;
			}
		}
		ImGui::EndDisabled();
		const bool canPreview = kenv.version <= KEnvironment::KVERSION_ARTHUR && kenv.platform == KEnvironment::PLATFORM_PC;
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginListBox("##PlayListTrackList")) {
			for (int i = 0; i < numStreams; ++i) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##Track", selectedTrackIndex == i))
					selectedTrackIndex = i;
				if (canPreview && ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					const int streamIndex = selectedPlayList->x2Streams[i].streamIndex;
					if (streamIndex >= 0 && streamIndex < soundManager->ksndmgrDings.size()) {
						auto path = GetInputAudioStreamPath(ui, ui.levelNum, streamIndex, false);
						if (!path.empty()) {
							PlaySnd(ConvertAudioStreamToWav(path));
						}
					}
				}
				ImGui::SameLine();
				if (streamsSeparated)
					ImGui::Text("%2i: %s", i, kenv.getObjectName(selectedPlayList->ogStreams[i].first.get()));
				else
					ImGui::Text("%2i: Stream %2i", i, selectedPlayList->x2Streams[i].streamIndex);
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if (0 <= selectedTrackIndex && selectedTrackIndex < numStreams) {
			ImGuiMemberListener ml{ kenv, ui };
			if (streamsSeparated) {
				CKStreamObject* stream = selectedPlayList->ogStreams[selectedTrackIndex].first.get();
				ml.setPropertyInfoList(ui.g_encyclo, stream);
				IGObjectNameInput("Entry Name", stream, kenv);
				IGObjectSelectorRef(ui, "track", stream->streamPointer);
				if (stream->streamPointer)
					ImGui::Text("File %s, %f seconds", stream->streamPointer->wavePath.c_str(), stream->streamPointer->waveDurationSec);
				ml.reflect(stream->param1, "param1");
				ml.reflect(stream->param2, "param2");
				ml.reflect(stream->param3, "param3");
				ml.reflect(stream->param4, "param4");
				ml.reflect(stream->ogUnk1, "ogUnk1");
				ml.reflect(stream->ogUnk2, "ogUnk2");
			}
			else {
				auto& stream = selectedPlayList->x2Streams[selectedTrackIndex];
				ml.reflect(stream.streamIndex, "streamIndex");
				if (kenv.version == KEnvironment::KVERSION_ARTHUR) {
					CKSoundManager* sndMgr = kenv.levelObjects.getFirst<CKSoundManager>();
					if (sndMgr && stream.streamIndex >= 0 && stream.streamIndex < (int)sndMgr->ksndmgrDings.size()) {
						const auto& tune = sndMgr->ksndmgrDings[stream.streamIndex];
						ImGui::Text("File MUSIC\\%i\\STRA%i.RWS, %f seconds", tune.arValue1, tune.arValue2, tune.duration);
					}
				}
				ml.reflect(stream.param1, "param1");
				ml.reflect(stream.param2, "param2");
				ml.reflect(stream.param3, "param3");
				ml.reflect(stream.param4, "param4");
				ml.reflect(stream.x2Unk1, "x2Unk1");
			}
		}
	}
}

void EditorUI::IGAudioStreamEditor(EditorInterface& ui)
{
	CKSoundManager* soundMgr = ui.kenv.levelObjects.getFirst<CKSoundManager>();
	if (!soundMgr) return;

	if (ImGui::Button("New stream entry")) {
		soundMgr->ksndmgrDings.emplace_back();
	}

	const bool canDoOperations = ui.kenv.platform == KEnvironment::PLATFORM_PC && !ui.kenv.isRemaster;
	int numColumns = 2;
	if (canDoOperations)
		numColumns += 1;
	if (ui.kenv.version == KEnvironment::KVERSION_ARTHUR || ui.kenv.isRemaster)
		numColumns += 1;

	if (ImGui::BeginTable("StreamTable", numColumns)) {
		ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 32.0f);
		if (canDoOperations)
			ImGui::TableSetupColumn("Ops", ImGuiTableColumnFlags_WidthFixed, 70.0f);
		ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		if (ui.kenv.version == KEnvironment::KVERSION_ARTHUR)
			ImGui::TableSetupColumn("Music file index");
		if (ui.kenv.isRemaster)
			ImGui::TableSetupColumn("Music file path");

		ImGui::TableHeadersRow();
		for (int i = 0; i < soundMgr->ksndmgrDings.size(); ++i) {
			auto& tune = soundMgr->ksndmgrDings[i];
			ImGui::TableNextRow();
			ImGui::PushID(i);
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%2i", i);
			if (canDoOperations) {
				ImGui::TableNextColumn();
				if (ImGui::ArrowButton("Play", ImGuiDir_Right)) {
					const auto path = GetInputAudioStreamPath(ui, ui.levelNum, i);
					if (!path.empty()) {
						const auto wav = ConvertAudioStreamToWav(path);
						PlaySnd(wav);
					}
				}
				ImGui::SetItemTooltip("Play");
				ImGui::SameLine();
				if (ImGui::Button("I")) {
					const auto wavPath = GuiUtils::OpenDialogBox(ui.g_window, "WAV audio file\0*.WAV\0\0", "wav");
					if (!wavPath.empty()) {
						IOFile wavFile(wavPath.c_str(), "rb");
						WavDocument wav;
						wav.read(&wavFile);
						const auto rwsPath = GetOutputAudioStreamPath(ui, ui.levelNum, i);
						ConvertWavToAudioStream(wav, rwsPath);
						const int bytesPerCombinedSample = wav.pcmBitsPerSample * wav.numChannels / 8;
						tune.duration = (float)(wav.data.size() / bytesPerCombinedSample) / wav.samplesPerSec;
					}
				}
				ImGui::SetItemTooltip("Import");
				ImGui::SameLine();
				if (ImGui::Button("E")) {
					auto path = GetInputAudioStreamPath(ui, ui.levelNum, i);
					if (!path.empty()) {
						const auto filepath = GuiUtils::SaveDialogBox(ui.g_window, "WAV audio file\0*.WAV\0\0", "wav", std::to_string(i));
						if (!filepath.empty()) {
							IOFile file(filepath.c_str(), "wb");
							auto wav = ConvertAudioStreamToWav(path);
							wav.write(&file);
						}
					}
				}
				ImGui::SetItemTooltip("Export");
			}
			ImGui::TableNextColumn();
			ImGui::Text("%f", tune.duration);
			if (ui.kenv.version == KEnvironment::KVERSION_ARTHUR) {
				ImGui::TableNextColumn();
				ImGui::SetNextItemWidth(64.0f);
				if (ImGui::InputScalar("##MusicFileIndex", ImGuiDataType_U32, &tune.arValue2)) {
					tune.arValue1 = tune.arValue2 / 10;
				}
			}
			if (ui.kenv.isRemaster) {
				ImGui::TableNextColumn();
				ImGui::GetColumnWidth();
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				IGStringInput("##RemasterFilePath", tune.remasterPath);
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}