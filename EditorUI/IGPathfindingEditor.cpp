#include "IGPathfindingEditor.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "GuiUtils.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include "imgui/imgui.h"
#include <fmt/format.h>

namespace {
	AABoundingBox getDefaultDoorBoxForPFNode(const CKPFGraphNode* pfNode) {
		Vector3 middle = (pfNode->boundingBox.lowCorner + pfNode->boundingBox.highCorner) * 0.5f;
		AABoundingBox box;
		box.lowCorner = Vector3(middle.x - 1.0f, pfNode->boundingBox.lowCorner.y, middle.z - 1.0f);
		box.highCorner = Vector3(middle.x + 1.0f, pfNode->boundingBox.highCorner.y, middle.z + 1.0f);
		return box;
	}
}

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
	ImGui::SameLine();
	ImGui::BeginDisabled(!ui.selectedPFGraphNode.get());
	if (ImGui::Button("Remove")) {
		auto* pfNode = ui.selectedPFGraphNode.get();
		if (pfNode->getRefCount() > 1) {
			GuiUtils::MsgBox(ui.g_window, "Cannot delete the PF node since it is being referenced (by another PF node for example).", 48);
		}
		else {
			const bool transReferenced =
				std::ranges::any_of(pfNode->transitions, [](const kobjref<CKPFGraphTransition>& ref) {return ref->getRefCount() > 1; });
			if (transReferenced) {
				GuiUtils::MsgBox(ui.g_window, "Cannot delete the PF node since one of its transition objects is being referenced.\n\nThis could be the case if for example there is an event sequence that enables/disables the transition.", 48);
			}
			else {
				for (auto& ref : pfNode->transitions) {
					auto* pfTrans = ref.get();
					ref = nullptr;
					kenv.removeObject(pfTrans);
				}
				std::erase_if(srvpf->nodes, [pfNode](const auto& ref) {return ref.get() == pfNode; });
				kenv.removeObject(pfNode);
			}
		}
	}
	ImGui::EndDisabled();

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
		bool boundingBoxUpdated = false;
		if (ImGui::DragFloat3("BB Low", &pfnode->boundingBox.lowCorner.x, 0.1f)) {
			pfnode->boundingBox.highCorner = pfnode->boundingBox.lowCorner + Vector3(pfnode->numCellsX * oldcw, oldHeight, pfnode->numCellsZ * oldch);
			boundingBoxUpdated = true;
		}
		if (ImGui::DragFloat("BB Height", &oldHeight)) {
			pfnode->boundingBox.highCorner.y = pfnode->boundingBox.lowCorner.y + oldHeight;
			boundingBoxUpdated = true;
		}
		//ImGui::DragFloat3("BB High", &pfnode->boundingBox.highCorner.x, 0.1f);
		if (boundingBoxUpdated) {
			for (auto& pfTrans : pfnode->transitions) {
				for (auto& door : pfTrans->doors) {
					door.sourceBox.lowCorner.y = pfnode->boundingBox.lowCorner.y;
					door.sourceBox.highCorner.y = pfnode->boundingBox.highCorner.y;
				}
			}
		}
		if (ImGui::Button("Place camera there")) {
			ui.camera.position.x = (pfnode->boundingBox.lowCorner.x + pfnode->boundingBox.highCorner.x) * 0.5f;
			ui.camera.position.z = (pfnode->boundingBox.lowCorner.z + pfnode->boundingBox.highCorner.z) * 0.5f;
		}

		const auto transitionHeaderName = fmt::format("Transitions ({})###Transitions", pfnode->transitions.size());
		if (ImGui::CollapsingHeader(transitionHeaderName.c_str())) {
			int tid = 0;
			int removeTransition = -1;
			for (auto& pftrans : pfnode->transitions) {
				const bool open = ImGui::TreeNode(pftrans.get(), "To %i: %s", tid, kenv.getObjectName(pftrans->node.get()));
				IGObjectDragDropSource(ui, pftrans.get());
				if (open) {
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
					if (ImGui::Button("Add door")) {
						auto& door = pftrans->doors.emplace_back();
						door.sourceBox = getDefaultDoorBoxForPFNode(pfnode);
						door.destinationBox = getDefaultDoorBoxForPFNode(pftrans->node ? pftrans->node.get() : pfnode);
					}
					ImGui::SameLine();
					if (ImGui::Button("Remove transition")) {
						removeTransition = tid;
					}
					ImGui::TreePop();
				}
				tid++;
			}
			if (removeTransition != -1) {
				auto* pfTrans = pfnode->transitions[removeTransition].get();
				if (pfTrans->getRefCount() > 1) {
					GuiUtils::MsgBox(ui.g_window, "Cannot delete the transition since it is being referenced.\n\nThis could be the case if for example there is an event sequence that enables/disables the transition.", 48);
				}
				else {
					pfnode->transitions.erase(pfnode->transitions.begin() + removeTransition);
					kenv.removeObject(pfTrans);
				}
			}
			if (ImGui::Button("Add transition")) {
				ImGui::OpenPopup("Add transition popup");
			}
			if (ImGui::BeginPopup("Add transition popup")) {
				int nid = 0;
				for (auto& targetNode : srvpf->nodes) {
					if (targetNode.get() != pfnode) {
						ImGui::PushID(nid);
						if (ImGui::Selectable("##PFNodeEntry")) {
							auto* newTransition = kenv.createAndInitObject<CKPFGraphTransition>();
							pfnode->transitions.emplace_back(newTransition);
							newTransition->node = targetNode;

							auto& door = newTransition->doors.emplace_back();
							if (auto optIntersection = pfnode->boundingBox.intersectionWith(targetNode->boundingBox)) {
								door.sourceBox = *optIntersection;
								door.destinationBox = *optIntersection;
							}
							else {
								door.sourceBox = getDefaultDoorBoxForPFNode(pfnode);
								door.destinationBox = getDefaultDoorBoxForPFNode(targetNode.get());
							}
						}
						ImGui::SameLine();
						ImGui::Text("%3i: %s", nid, kenv.getObjectName(targetNode.get()));
						ImGui::PopID();
					}
					nid++;
				}
				ImGui::EndPopup();
			}
		}

		const auto othersHeaderName = fmt::format("Alternative nodes ({})###Others", pfnode->others.size());
		if (ImGui::CollapsingHeader(othersHeaderName.c_str())) {
			ImGui::PushID("Other nodes");
			if (kenv.version == KEnvironment::KVERSION_XXL1) {
				bool hasAlt = !pfnode->others.empty();
				if (ImGui::Checkbox("Has alternative", &hasAlt)) {
					pfnode->others.resize(hasAlt ? 1 : 0);
				}
			}
			else {
				int numOthers = (int)pfnode->others.size();
				ImGui::InputInt("Num alternatives", &numOthers);
				if (ImGui::IsItemDeactivatedAfterEdit())
					if (numOthers >= 0 && numOthers < 100)
						pfnode->others.resize(numOthers);
			}

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

			static const std::tuple<uint8_t, const char*> knownCellTypes[] = {
				{1, "Free cell\nEveryone can pass"},
				{4, "Forbidden for Enemies\nTracking Heroes can still pass"},
				{7, "Forbidden for everyone"},
				{0, "Forbidden for Tracking Heroes\nEnemies can still pass"}
			};
			static uint8_t paintval = 7;
			for (const auto& [val, description] : knownCellTypes) {
				char buf[8];
				sprintf_s(buf, "%X", val);
				const bool active = paintval == val;
				if (active)
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
				ImGui::PushStyleColor(ImGuiCol_Text, getPFCellColor(val));
				if (ImGui::Button(buf))
					paintval = val;
				ImGui::SetItemTooltip("%s", description);
				ImGui::PopStyleColor();
				if (active)
					ImGui::PopStyleColor();
				ImGui::SameLine();
			}
			//ImGui::SameLine();
			ImGui::InputScalar("Value", ImGuiDataType_U8, &paintval);
			paintval &= 15;

			const int cellSize = 16;

			int c = 0;
			ImGui::BeginChild("PFGrid", ImVec2(0, 16 * 0), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::InvisibleButton("PFGridButton", ImVec2(cellSize * pfnode->numCellsX, cellSize * pfnode->numCellsZ), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 drawOffset = ImGui::GetItemRectMin();
			if (ImGui::IsItemActive()) {
				auto mousePos = ImGui::GetMousePos();
				int x = (int)(mousePos.x - drawOffset.x) / cellSize;
				int y = (int)(mousePos.y - drawOffset.y) / cellSize;
				if (x >= 0 && y >= 0 && x < pfnode->numCellsX && y < pfnode->numCellsZ) {
					uint8_t& val = pfnode->cells[y * pfnode->numCellsX + x];
					if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
						val = paintval;
					else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
						paintval = val;
				}
			}
			// Draw Grid cells
			for (int y = 0; y < pfnode->numCellsZ; y++) {
				for (int x = 0; x < pfnode->numCellsX; x++) {
					uint8_t& val = pfnode->cells[c++];
					const uint32_t colorInt = ImGui::ColorConvertFloat4ToU32(getPFCellColor(val));

					static const char hexchars[17] = "0123456789ABCDEF";
					char text[2] = { hexchars[val % 16], 0 };

					const float rect_x = drawOffset.x + cellSize * x;
					const float rect_y = drawOffset.y + cellSize * y;
					drawList->AddRectFilled(ImVec2(rect_x, rect_y), ImVec2(rect_x + cellSize - 1, rect_y + cellSize - 1), colorInt);
					drawList->AddText(ImVec2(rect_x + 4, rect_y + 1), 0xFF000000, text, text + 1);
					drawList->AddText(ImVec2(rect_x + 3, rect_y), (colorInt == 0xFFFFFFFF) ? 0xFFC0C0C0 : 0xFFFFFFFF, text, text + 1);
				}
			}
			ImGui::EndChild();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}
