#include "IGMusicEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKManager.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include <imgui/imgui.h>

void EditorUI::IGMusicEditor(EditorInterface& ui)
{
	static KWeakRef<CKMusicPlayList> selectedPlayList;
	static int selectedTrackIndex = -1;

	auto& kenv = ui.kenv;
	CKSrvMusic* srvMusic = kenv.levelObjects.getFirst<CKSrvMusic>();
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
		if (ImGui::Button("New stream")) {
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
		if (ImGui::Button("Delete stream")) {
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
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginListBox("##PlayListTrackList")) {
			for (int i = 0; i < numStreams; ++i) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##Track", selectedTrackIndex == i))
					selectedTrackIndex = i;
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
