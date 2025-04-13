#include "IGCollisionEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKNode.h"

#include <imgui/imgui.h>

void EditorUI::IGCollisionEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	CKSrvCollision* srvcoll = kenv.levelObjects.getFirst<CKSrvCollision>();
	if (!srvcoll)
		return;
	ImGuiMemberListener igml{ kenv, ui };
	igml.setPropertyInfoList(ui.g_encyclo, srvcoll);
	MemberListener* ml = &igml;
	if (ImGui::BeginTabBar("ColliTabBar")) {
		if (ImGui::BeginTabItem("Head")) {
			//ml->reflect(srvcoll->numWhat, "numWhat");
			ml->reflect(srvcoll->huh, "huh");
			//ml->reflect(srvcoll->unk1, "unk1");
			//ml->reflect(srvcoll->unk2, "unk2");
			ml->reflect(srvcoll->activeList, "activeList");
			ml->reflect(srvcoll->inactiveList, "inactiveList");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("objs")) {
			int i = 0;
			for (auto& vec : srvcoll->objs) {
				int j = 0;
				for (auto& ref : vec) {
					std::string str = std::to_string(i) + ',' + std::to_string(j);
					IGObjectSelectorRef(ui, str.c_str(), ref);
					j++;
				}
				ImGui::Separator();
				i++;
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("objs2")) {
			int i = 0;
			for (auto& ref : srvcoll->objs2)
				IGObjectSelectorRef(ui, std::to_string(i++).c_str(), ref);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("bings")) {
			static size_t selectedBing = -1;
			auto getActorName = [&kenv, srvcoll](uint16_t id) -> const char* {
				if (id != 0xFFFF && srvcoll->objs2[id]) {
					if (kenv.version >= kenv.KVERSION_XXL2)
						return kenv.getObjectName(srvcoll->objs2[id].get());
					else
						return srvcoll->objs2[id]->getClassName();
				}
				return "/";
				};
			ImGui::Columns(2);
			static std::vector<int> bingTags;
			if (ImGui::Button("Tag")) {
				bingTags = std::vector<int>(srvcoll->bings.size(), 0);
				std::vector<int> nodes = { srvcoll->activeList };
				std::vector<int> nodes2;
				bingTags[srvcoll->activeList] |= 1;
				while (!nodes.empty()) {
					nodes2.clear();
					for (int n : nodes) {
						for (int a : srvcoll->bings[n].aa) {
							if (a != 0xFFFF && (bingTags[a] & 1) == 0) {
								bingTags[a] |= 1;
								nodes2.push_back(a);
							}
						}
					}
					std::swap(nodes, nodes2);
				}
				nodes = { srvcoll->inactiveList };
				nodes2.clear();
				bingTags[srvcoll->inactiveList] |= 2;
				while (!nodes.empty()) {
					nodes2.clear();
					for (int n : nodes) {
						for (int a : {srvcoll->bings[n].aa[0], srvcoll->bings[n].aa[1]}) {
							if (a != 0xFFFF && (bingTags[a] & 2) == 0) {
								bingTags[a] |= 2;
								nodes2.push_back(a);
							}
						}
					}
					std::swap(nodes, nodes2);
				}
			}
			ImGui::BeginChild("CollBingList");
			int b = 0;
			for (auto& bing : srvcoll->bings) {
				ImGui::PushID(b);
				if (ImGui::Selectable("##sel", b == selectedBing))
					selectedBing = b;
				if (ImGui::IsItemVisible()) {
					ImGui::SameLine();
					int tag = (b < bingTags.size()) ? bingTags[b] : 7;
					ImGui::Text("%i:%i, %s : %s", b, tag, getActorName(bing.b1), getActorName(bing.b2));
				}
				ImGui::PopID();
				b++;
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("CollBingInfo");
			if (selectedBing < srvcoll->bings.size()) {
				auto& bing = srvcoll->bings[selectedBing];
				ImGui::InputScalar("v1", ImGuiDataType_U16, &bing.v1, nullptr, nullptr, "%04X");
				ml->reflect(bing.obj1, "obj1");
				ml->reflect(bing.obj2, "obj2");
				ml->reflect(bing.b1, "b1");
				ImGui::Text("%s", getActorName(bing.b1));
				ml->reflect(bing.b2, "b2");
				ImGui::Text("%s", getActorName(bing.b2));
				ml->reflect(bing.v2, "v2");
				ml->reflect(bing.aa, "aa");
				ImGui::Separator();
				if (bing.obj1) {
					ml->reflect(bing.obj1->cast<CKBoundingShape>()->unk1, "obj1_unk1");
					ml->reflect(bing.obj1->cast<CKBoundingShape>()->unk2, "obj1_unk2");
				}
				if (bing.obj2) {
					ml->reflect(bing.obj2->cast<CKBoundingShape>()->unk1, "obj2_unk1");
					ml->reflect(bing.obj2->cast<CKBoundingShape>()->unk2, "obj2_unk2");
				}
			}
			ImGui::EndChild();
			ImGui::Columns();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
