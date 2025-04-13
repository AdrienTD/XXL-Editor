#include "IGLineEditor.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"
#include "CoreClasses/CKLogic.h"

#include <imgui/imgui.h>

void EditorUI::IGLineEditor(EditorInterface& ui)
{
	static KWeakRef<CKLogic> lineObject;
	kobjref<CKLogic> lineTempRef = lineObject.get();
	IGObjectSelectorRef(ui, "Line", lineTempRef);
	lineObject = lineTempRef.get();
	if (!lineObject) return;
	if (CKLine* line = lineObject->dyncast<CKLine>()) {
		bool update = false;
		for (size_t i = 0; i < line->points.size(); ++i) {
			ImGui::PushID((int)i);
			ImGui::DragFloat3("##LinePoint", &line->points[i].x, 0.1f);
			ImGui::SameLine();
			if (ImGui::Button("D")) {
				line->points.insert(line->points.begin() + i, line->points[i]);
				update = true;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate");
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				line->points.erase(line->points.begin() + i);
				update = true;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove");
			ImGui::PopID();
		}
		if (update) {
			line->numSegments = (uint8_t)(line->points.size() - 1);
			line->segmentLengths.resize(line->numSegments);
			float total = 0.0f;
			for (size_t i = 0; i < (size_t)line->numSegments; ++i) {
				line->segmentLengths[i] = (line->points[i + 1] - line->points[i]).len3();
				total += line->segmentLengths[i];
			}
			line->totalLength = total;
		}
	}
	if (CKFlaggedPath* path = lineObject->dyncast<CKFlaggedPath>()) {
		IGObjectSelectorRef(ui, "Path's line", path->line);
		for (size_t i = 0; i < path->numPoints; ++i) {
			ImGui::PushID((int)i);
			ImGui::SetNextItemWidth(64.0f);
			ImGui::DragFloat("##PathElemValue", &path->pntValues[i], 0.1f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(128.0f);
			IGEventSelector(ui, "", path->pntEvents[i]);
			ImGui::SameLine();
			if (ImGui::Button("D")) {
				path->pntValues.insert(path->pntValues.begin() + i, path->pntValues[i]);
				path->pntEvents.insert(path->pntEvents.begin() + i, path->pntEvents[i]);
				path->numPoints += 1;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate");
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				path->pntValues.erase(path->pntValues.begin() + i);
				path->pntEvents.erase(path->pntEvents.begin() + i);
				path->numPoints -= 1;
				--i;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove");
			ImGui::PopID();
		}
	}
}
