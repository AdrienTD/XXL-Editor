#include "IGPathfindingEditor.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include "imgui/imgui.h"
#include <fmt/format.h>

ImVec4 EditorUI::getPFCellColor(uint8_t val) {
	ImVec4 color(1, 0, 1, 1);
	switch (val) {
	case 0: color = ImVec4(0, 1, 1, 1); break; // enemy
	case 1: color = ImVec4(1, 1, 1, 1); break; // partner + enemy
	case 4: color = ImVec4(1, 1, 0, 1); break; // partner
	case 7: color = ImVec4(1, 0, 0, 1); break; // wall
	}
	return color;
}

void EditorUI::IGPathfindingEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	CKSrvPathFinding* srvpf = kenv.levelObjects.getFirst<CKSrvPathFinding>();
	if (!srvpf) return;
	if (ImGui::Button("New PF node")) {
		CKPFGraphNode* pfnode = kenv.createObject<CKPFGraphNode>(-1);
		srvpf->nodes.emplace_back(pfnode);
		pfnode->numCellsX = 20;
		pfnode->numCellsZ = 20;
		pfnode->cells = std::vector<uint8_t>(pfnode->numCellsX * pfnode->numCellsZ, 1);
		pfnode->boundingBox.lowCorner = ui.cursorPosition;
		pfnode->boundingBox.highCorner = pfnode->boundingBox.lowCorner + Vector3((float)pfnode->numCellsX * 2.0f, 50.0f, (float)pfnode->numCellsZ * 2.0f);
	}

	ImGui::Columns(2);
	ImGui::BeginChild("PFNodeList");
	int nid = 0;
	for (auto& pfnode : srvpf->nodes) {
		ImGui::PushID(&pfnode);
		if (ImGui::Selectable("##PFNodeEntry", ui.selectedPFGraphNode == pfnode.get())) {
			ui.selectedPFGraphNode = pfnode.get();
		}
		ImGui::SameLine();
		ImGui::Text("%3i: %s (%u*%u)", nid, kenv.getObjectName(pfnode.get()), pfnode->numCellsX, pfnode->numCellsZ);
		ImGui::PopID();
		nid++;
	}
	ImGui::EndChild();

	ImGui::NextColumn();
	ImGui::BeginChild("PFNodeInfo");
	if (CKPFGraphNode* pfnode = ui.selectedPFGraphNode.get()) {
		IGObjectNameInput("Name", pfnode, kenv);
		float oldcw = pfnode->getCellWidth();
		float oldch = pfnode->getCellHeight();
		float oldHeight = pfnode->boundingBox.highCorner.y - pfnode->boundingBox.lowCorner.y;
		if (ImGui::DragFloat3("BB Low", &pfnode->boundingBox.lowCorner.x, 0.1f)) {
			pfnode->boundingBox.highCorner = pfnode->boundingBox.lowCorner + Vector3(pfnode->numCellsX * oldcw, oldHeight, pfnode->numCellsZ * oldch);
		}
		if (ImGui::DragFloat("BB Height", &oldHeight)) {
			pfnode->boundingBox.highCorner.y = pfnode->boundingBox.lowCorner.y + oldHeight;
		}
		//ImGui::DragFloat3("BB High", &pfnode->boundingBox.highCorner.x, 0.1f);
		if (ImGui::Button("Place camera there")) {
			ui.camera.position.x = (pfnode->boundingBox.lowCorner.x + pfnode->boundingBox.highCorner.x) * 0.5f;
			ui.camera.position.z = (pfnode->boundingBox.lowCorner.z + pfnode->boundingBox.highCorner.z) * 0.5f;
		}

		const auto transitionHeaderName = fmt::format("Transitions ({})###Transitions", pfnode->transitions.size());
		if (ImGui::CollapsingHeader(transitionHeaderName.c_str())) {
			int tid = 0;
			for (auto& pftrans : pfnode->transitions) {
				if (ImGui::TreeNode(&pftrans, "Transition %i", tid)) {
					IGObjectSelectorRef(ui, "Target node", pftrans->node);
					ImGui::InputScalar("unk1", ImGuiDataType_U32, &pftrans->unk1, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
					ImGui::InputScalar("unk2", ImGuiDataType_U32, &pftrans->unk2, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
					for (auto& door : pftrans->doors) {
						ImGui::PushID(&door);
						ImGui::Bullet();
						ImGui::Indent();
						ImGui::DragFloat3("Source Low", &door.sourceBox.lowCorner.x);
						ImGui::DragFloat3("Source High", &door.sourceBox.highCorner.x);
						ImGui::DragFloat3("Destination Low", &door.destinationBox.lowCorner.x);
						ImGui::DragFloat3("Destination High", &door.destinationBox.highCorner.x);
						ImGui::InputScalar("Flags", ImGuiDataType_U32, &door.unk);
						ImGui::Unindent();
						ImGui::PopID();
					}
					if (ImGui::Button("Add")) {
						auto& door = pftrans->doors.emplace_back();
						Vector3 middle = (pfnode->boundingBox.lowCorner + pfnode->boundingBox.highCorner) * 0.5f;
						door.sourceBox.lowCorner = Vector3(middle.x - 1.0f, pfnode->boundingBox.lowCorner.y, middle.z - 1.0f);
						door.sourceBox.highCorner = Vector3(middle.x + 1.0f, pfnode->boundingBox.highCorner.y, middle.z + 1.0f);
					}
					ImGui::TreePop();
				}
				tid++;
			}
			if (ImGui::Button("Add")) {
				auto* newTransition = kenv.createAndInitObject<CKPFGraphTransition>();
				pfnode->transitions.emplace_back(newTransition);
			}
		}

		const auto othersHeaderName = fmt::format("Alternative nodes ({})###Others", pfnode->others.size());
		if (ImGui::CollapsingHeader(othersHeaderName.c_str())) {
			ImGui::PushID("Other nodes");
			int numOthers = (int)pfnode->others.size();
			ImGui::InputInt("Num alternatives", &numOthers);
			if (ImGui::IsItemDeactivatedAfterEdit())
				if (numOthers >= 0 && numOthers < 100)
					pfnode->others.resize(numOthers);
			for (size_t i = 0; i < pfnode->others.size(); ++i) {
				IGObjectSelectorRef(ui, std::to_string(i).c_str(), pfnode->others[i]);
			}
			ImGui::PopID();
		}

		if (ImGui::CollapsingHeader("Grid", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Grid size: %u * %u", pfnode->numCellsX, pfnode->numCellsZ);
			ImGui::Text("Cell size: %f * %f", pfnode->getCellWidth(), pfnode->getCellHeight());

			static uint8_t resizeX, resizeZ;
			static float recellWidth, recellHeight;
			if (ImGui::Button("Resize")) {
				ImGui::OpenPopup("PFGridResize");
				resizeX = pfnode->numCellsX;
				resizeZ = pfnode->numCellsZ;
				recellWidth = pfnode->getCellWidth();
				recellHeight = pfnode->getCellHeight();
			}
			if (ImGui::BeginPopup("PFGridResize")) {
				ImGui::InputScalar("Grid Width", ImGuiDataType_U8, &resizeX);
				ImGui::InputScalar("Grid Height", ImGuiDataType_U8, &resizeZ);
				ImGui::InputFloat("Cell Width", &recellWidth);
				ImGui::InputFloat("Cell Height", &recellHeight);
				if (ImGui::Button("OK")) {
					std::vector<uint8_t> res(resizeX * resizeZ, 1);
					int cx = std::min(pfnode->numCellsX, resizeX);
					int cz = std::min(pfnode->numCellsZ, resizeZ);
					for (uint8_t x = 0; x < cx; x++)
						for (uint8_t z = 0; z < cz; z++)
							res[z * resizeX + x] = pfnode->cells[z * pfnode->numCellsX + x];
					pfnode->numCellsX = resizeX;
					pfnode->numCellsZ = resizeZ;
					pfnode->cells = res;
					pfnode->boundingBox.highCorner = pfnode->boundingBox.lowCorner + Vector3(recellWidth * resizeX, 50, recellHeight * resizeZ);
				}
				ImGui::EndPopup();
			}

			static uint8_t paintval = 7;
			for (uint8_t val : {1, 4, 7, 0}) {
				char buf[8];
				sprintf_s(buf, "%X", val);
				ImGui::PushStyleColor(ImGuiCol_Text, getPFCellColor(val));
				if (ImGui::Button(buf))
					paintval = val;
				ImGui::PopStyleColor();
				ImGui::SameLine();
			}
			//ImGui::SameLine();
			ImGui::InputScalar("Value", ImGuiDataType_U8, &paintval);
			paintval &= 15;

			int c = 0;
			ImGui::BeginChild("PFGrid", ImVec2(0, 16 * 0), true, ImGuiWindowFlags_NoMove);
			for (int y = 0; y < pfnode->numCellsZ; y++) {
				for (int x = 0; x < pfnode->numCellsX; x++) {
					uint8_t& val = pfnode->cells[c++];
					ImVec4 color = getPFCellColor(val);

					ImGui::TextColored(color, "%X", val);
					//ImGui::Image(nullptr, ImVec2(8, 8), ImVec2(0, 0), ImVec2(0, 0), color);
					//ImVec2 curpos = ImGui::GetCursorScreenPos();
					//ImGui::GetWindowDrawList()->AddRectFilled(curpos, ImVec2(curpos.x + 8, curpos.y + 8), ImGui::GetColorU32(color));
					//ImGui::Dummy(ImVec2(8, 8));

					if (ImGui::IsItemHovered()) {
						if (ImGui::IsMouseDown(0))
							val = paintval;
						else if (ImGui::IsMouseDown(1))
							paintval = val;
					}
					ImGui::SameLine();
				}
				ImGui::NewLine();
			}
			ImGui::EndChild();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}
