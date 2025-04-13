#include "IGEventEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "GuiUtils.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"

#include <imgui/imgui.h>

void EditorUI::IGEventEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	CKSrvEvent* srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>();
	if (!srvEvent) return;

	if (ImGui::Button("Find targets...")) {
		ImGui::OpenPopup("EvtFindTargets");
	}
	if (ImGui::BeginPopup("EvtFindTargets")) {
		static int tgCat = 0, tgId = 0;
		ImGui::InputInt("Category", &tgCat);
		ImGui::InputInt("ID", &tgId);
		if (ImGui::Button("Find")) {
			size_t ev = 0, s = 0;
			for (auto& bee : srvEvent->sequences) {
				for (int i = 0; i < bee.numActions; i++) {
					if ((srvEvent->objs[ev + i].id & 0x1FFFF) == (tgCat | (tgId << 6)))
						printf("class (%i, %i) found at seq %i ev 0x%04X\n", tgCat, tgId, s, srvEvent->objInfos[ev + i]);
				}
				ev += bee.numActions;
				s++;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Export TSV")) {
		auto filename = GuiUtils::SaveDialogBox(ui.g_window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
		if (!filename.empty()) {
			std::map<std::pair<int, int>, std::set<uint16_t>> evtmap;
			size_t ev = 0, s = 0;
			for (auto& bee : srvEvent->sequences) {
				for (int i = 0; i < bee.numActions; i++) {
					auto& obj = srvEvent->objs[ev + i];
					evtmap[std::make_pair(obj.id & 63, (obj.id >> 6) & 2047)].insert(srvEvent->objInfos[ev + i]);
				}
				ev += bee.numActions;
				s++;
			}
			FILE* file;
			GuiUtils::fsfopen_s(&file, filename, "wt");
			for (auto& mapentry : evtmap) {
				for (uint16_t evt : mapentry.second) {
					fprintf(file, "%i\t%i\t%04X\n", mapentry.first.first, mapentry.first.second, evt);
				}
			}
			fclose(file);
		}
	}
	ImGui::SameLine();
	ImGui::Text("Decoded: %i/%i", std::count_if(srvEvent->sequences.begin(), srvEvent->sequences.end(), [](CKSrvEvent::EventSequence& bee) {return bee.userFound; }), srvEvent->sequences.size());

	ImGui::Columns(2);
	if (ImGui::BeginTabBar("EvtSeqTypeBar")) {
		size_t ev = 0, i = 0;
		for (int et = 0; et < 3; ++et) {
			static const char* etNameArr[3] = { "Or", "And", "Simultaneous" };
			if (ImGui::BeginTabItem(etNameArr[et])) {
				if (ImGui::Button("New")) {
					srvEvent->sequences.emplace(srvEvent->sequences.begin() + i);
					srvEvent->evtSeqIDs.insert(srvEvent->evtSeqIDs.begin() + i, srvEvent->nextSeqID++);
					srvEvent->evtSeqNames.emplace(srvEvent->evtSeqNames.begin() + i, "New sequence");
					srvEvent->numSeqs[et] += 1;
				}
				ImGui::SameLine();
				bool pleaseRemove = false;
				if (ImGui::Button("Remove")) {
					pleaseRemove = true;
				}
				ImGui::BeginChild("EventSeqList");
				for (int j = 0; j < srvEvent->numSeqs[et]; ++j) {
					if (pleaseRemove && i == ui.selectedEventSequence) {
						size_t actFirst = ev, actLast = ev + srvEvent->sequences[i].numActions;
						srvEvent->objs.erase(srvEvent->objs.begin() + actFirst, srvEvent->objs.begin() + actLast);
						srvEvent->objInfos.erase(srvEvent->objInfos.begin() + actFirst, srvEvent->objInfos.begin() + actLast);
						srvEvent->sequences.erase(srvEvent->sequences.begin() + i);
						srvEvent->evtSeqIDs.erase(srvEvent->evtSeqIDs.begin() + i);
						srvEvent->evtSeqNames.erase(srvEvent->evtSeqNames.begin() + i);
						srvEvent->numSeqs[et] -= 1;
						pleaseRemove = false;
						continue; // do not increment i nor ev
					}
					auto& bee = srvEvent->sequences[i];
					ImGui::PushID(i);
					if (ImGui::Selectable("##EventSeqEntry", i == ui.selectedEventSequence, 0, ImVec2(0, ImGui::GetTextLineHeight() * 2.0f)))
						ui.selectedEventSequence = i;
					if (ImGui::BeginDragDropSource()) {
						const auto& seqid = srvEvent->evtSeqIDs[i];
						ImGui::SetDragDropPayload("EventSeq", &seqid, sizeof(seqid));
						ImGui::Text("Event sequence\n%s", srvEvent->evtSeqNames[i].c_str());
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EventNodeX1")) {
							EventNodeX1* node = *(EventNodeX1**)payload->Data;
							node->seqIndex = srvEvent->evtSeqIDs[i];
							node->bit = 0; // TODO: Take available free bit
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					ImGui::BulletText("%s (%i, %02X)\nUsed by %s", srvEvent->evtSeqNames[i].c_str(), bee.numActions, bee.bitMask, bee.users.size() ? bee.users[0]->getClassName() : "?");
					ImGui::PopID();
					ev += bee.numActions;
					i++;
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			else {
				for (int j = 0; j < srvEvent->numSeqs[et]; ++j) {
					ev += srvEvent->sequences[i].numActions;
					i++;
				}
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::NextColumn();

	ImGui::BeginChild("EventEditor");
	if (ui.selectedEventSequence >= 0 && ui.selectedEventSequence < (int)srvEvent->sequences.size()) {
		auto& bee = srvEvent->sequences[ui.selectedEventSequence];
		int ev = 0;
		for (int i = 0; i < ui.selectedEventSequence; i++) {
			ev += srvEvent->sequences[i].numActions;
		}
		ImGui::Text("Used by %s", bee.users.size() ? bee.users[0]->getClassName() : "?");
		for (size_t u = 1; u < bee.users.size(); u++) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), ", %s", bee.users[u]->getClassName());
		}
		auto& name = srvEvent->evtSeqNames[ui.selectedEventSequence];
		ImGui::InputText("Name", (char*)name.c_str(), name.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &name);
		ImGui::InputScalar("Bitmask", ImGuiDataType_U8, &bee.bitMask, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
		char checkboxName[] = "?##BitmaskBit";
		for (int i = 0; i < 8; ++i) {
			uint8_t flag = (uint8_t)(1 << i);
			if (i != 0) ImGui::SameLine();
			bool b = (bee.bitMask & flag) != 0;
			checkboxName[0] = (char)('0' + i);
			if (ImGui::Checkbox(checkboxName, &b))
				bee.bitMask = (bee.bitMask & ~flag) | (uint8_t)((b ? 1 : 0) << i);
		}
		if (ImGui::Button("Add")) {
			auto it = srvEvent->objs.emplace(srvEvent->objs.begin() + ev + bee.numActions);
			it->bound = true;
			srvEvent->objInfos.emplace(srvEvent->objInfos.begin() + ev + bee.numActions);
			bee.numActions += 1;
		}
		int deleteEvent = -1;
		for (uint8_t i = 0; i < bee.numActions; i++) {
			ImGui::PushID(ev + i);

			if (srvEvent->objs[ev + i].bound) {
				ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
				IGObjectSelectorRef(ui, "##evtTargetObj", srvEvent->objs[ev + i].ref);
			}
			else {
				uint32_t& encref = srvEvent->objs[ev + i].id;
				uint32_t clcat = encref & 63;
				uint32_t clid = (encref >> 6) & 2047;
				uint32_t objnb = encref >> 17;
				ImGui::PushItemWidth(48.0f);
				ImGui::InputScalar("##EvtObjClCat", ImGuiDataType_U32, &clcat); ImGui::SameLine();
				ImGui::InputScalar("##EvtObjClId", ImGuiDataType_U32, &clid); ImGui::SameLine();
				ImGui::InputScalar("##EvtObjNumber", ImGuiDataType_U32, &objnb); //ImGui::SameLine();
				encref = clcat | (clid << 6) | (objnb << 17);
				ImGui::PopItemWidth();
			}
			ImGui::SameLine();
			if (ImGui::Button("X##EvtRemove")) {
				deleteEvent = ev + i;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Remove");

			ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
			IGEventMessageSelector(ui, "##EventMessage", srvEvent->objInfos[ev + i], srvEvent->objs[ev + i].ref.get());

			ImGui::PopID();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		}
		if (deleteEvent >= 0) {
			srvEvent->objs.erase(srvEvent->objs.begin() + deleteEvent);
			srvEvent->objInfos.erase(srvEvent->objInfos.begin() + deleteEvent);
			bee.numActions -= 1;
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}
