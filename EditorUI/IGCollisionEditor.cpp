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
			ml->reflect(srvcoll->numProjectilesUsed, "numProjectilesUsed");
			//ml->reflect(srvcoll->unk1, "unk1");
			//ml->reflect(srvcoll->unk2, "unk2");
			ml->reflect(srvcoll->activeList, "activeList");
			ml->reflect(srvcoll->inactiveList, "inactiveList");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("projectileTargets")) {
			int i = 0;
			for (auto& vec : srvcoll->projectileTargets) {
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
		if (ImGui::BeginTabItem("collidingLifes")) {
			int i = 0;
			for (auto& ref : srvcoll->collidingLifes)
				IGObjectSelectorRef(ui, std::to_string(i++).c_str(), ref);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("collisionTests")) {
			static size_t selectedBing = -1;
			auto getActorName = [&kenv, srvcoll](uint16_t id) -> const char* {
				if (id != 0xFFFF && srvcoll->collidingLifes[id]) {
					if (kenv.version >= kenv.KVERSION_XXL2)
						return kenv.getObjectName(srvcoll->collidingLifes[id].get());
					else
						return srvcoll->collidingLifes[id]->getClassName();
				}
				return "/";
				};
			ImGui::Columns(2);
			static std::vector<int> bingTags;
			if (ImGui::Button("Tag")) {
				bingTags = std::vector<int>(srvcoll->collisionTests.size(), 0);
				std::vector<int> nodes = { srvcoll->activeList };
				std::vector<int> nodes2;
				bingTags[srvcoll->activeList] |= 1;
				while (!nodes.empty()) {
					nodes2.clear();
					for (int n : nodes) {
						for (int a : srvcoll->collisionTests[n].aa) {
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
						for (int a : {srvcoll->collisionTests[n].aa[0], srvcoll->collisionTests[n].aa[1]}) {
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
			for (auto& bing : srvcoll->collisionTests) {
				ImGui::PushID(b);
				if (ImGui::Selectable("##sel", b == selectedBing))
					selectedBing = b;
				if (ImGui::IsItemVisible()) {
					ImGui::SameLine();
					int tag = (b < bingTags.size()) ? bingTags[b] : 7;
					ImGui::Text("%i:%i, %s : %s", b, tag, getActorName(bing.lifeIndex1), getActorName(bing.lifeIndex2));
				}
				ImGui::PopID();
				b++;
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("CollBingInfo");
			if (selectedBing < srvcoll->collisionTests.size()) {
				auto& bing = srvcoll->collisionTests[selectedBing];
				ImGui::InputScalar("flags", ImGuiDataType_U16, &bing.flags, nullptr, nullptr, "%04X");
				ml->reflect(bing.shapeNode1, "shapeNode1");
				ml->reflect(bing.shapeNode2, "shapeNode2");
				ml->reflect(bing.lifeIndex1, "lifeIndex1");
				ImGui::Text("%s", getActorName(bing.lifeIndex1));
				ml->reflect(bing.lifeIndex2, "lifeIndex2");
				ImGui::Text("%s", getActorName(bing.lifeIndex2));
				ml->reflect(bing.priority, "priority");
				ml->reflect(bing.aa, "aa");
				ImGui::Separator();
				if (bing.shapeNode1) {
					ml->reflect(bing.shapeNode1->cast<CKBoundingShape>()->unk1, "obj1_unk1");
					ml->reflect(bing.shapeNode1->cast<CKBoundingShape>()->unk2, "obj1_unk2");
				}
				if (bing.shapeNode2) {
					ml->reflect(bing.shapeNode2->cast<CKBoundingShape>()->unk1, "obj2_unk1");
					ml->reflect(bing.shapeNode2->cast<CKBoundingShape>()->unk2, "obj2_unk2");
				}
			}
			ImGui::EndChild();
			ImGui::Columns();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
