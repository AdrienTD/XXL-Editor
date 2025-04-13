#include "IGMarkerEditor.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"

#include <imgui/imgui.h>

void EditorUI::IGMarkerEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	CKSrvMarker* srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>();
	if (!srvMarker) return;
	ImGui::Columns(2);
	ImGui::BeginChild("MarkerTree");
	int lx = 0;
	for (auto& list : srvMarker->lists) {
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode(&list, "List %i", lx)) {
			if (ImGui::Button("Add")) {
				auto& marker = list.emplace_back();
				marker.position = ui.cursorPosition;
			}
			int mx = 0;
			for (auto& marker : list) {
				ImGui::PushID(&marker);
				if (ImGui::Selectable("##MarkerEntry", ui.selectedMarkerIndex == mx)) {
					ui.selectedMarkerIndex = mx;
				}
				ImGui::SameLine();
				ImGui::Text("%3i: %s (%u)", mx, marker.name.c_str(), marker.val3);
				ImGui::PopID();
				mx++;
			}
			ImGui::TreePop();
		}
		lx++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("MarkerInfo");
	if (!srvMarker->lists.empty() && ui.selectedMarkerIndex >= 0 && ui.selectedMarkerIndex < srvMarker->lists[0].size()) {
		CKSrvMarker::Marker& marker = srvMarker->lists[0][ui.selectedMarkerIndex];
		if (ImGui::Button("Place camera there")) {
			ui.camera.position = marker.position - ui.camera.direction * 5.0f;
		}
		IGStringInput("Name", marker.name);
		ImGui::DragFloat3("Position", &marker.position.x, 0.1f);
		ImGui::InputScalar("Orientation 1", ImGuiDataType_U8, &marker.orientation1);
		ImGui::InputScalar("Orientation 2", ImGuiDataType_U8, &marker.orientation2);
		ImGui::InputScalar("Val3", ImGuiDataType_U16, &marker.val3);
	}
	ImGui::EndChild();
	ImGui::Columns();
}
