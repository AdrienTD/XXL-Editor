#include "IGHookEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"
#include "PropFlagsEditor.h"

#include "KEnvironment.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"

#include "Duplicator.h"
#include "GuiUtils.h"

#include <imgui/imgui.h>

namespace
{
	void IGEnumGroup(EditorUI::EditorInterface& ui, CKGroup* group)
	{
		using namespace EditorUI;
		auto& kenv = ui.kenv;

		if (!group)
			return;
		bool gopen = ImGui::TreeNodeEx(group, (ui.selectedGroup == group && ui.viewGroupInsteadOfHook) ? ImGuiTreeNodeFlags_Selected : 0, "%s %s", group->getClassName(), kenv.getObjectName(group));
		IGObjectDragDropSource(ui, group);
		if (ImGui::IsItemClicked()) {
			ui.selectedGroup = group;
			ui.viewGroupInsteadOfHook = true;
		}
		if (gopen) {
			IGEnumGroup(ui, group->childGroup.get());
			for (CKHook* hook = group->childHook.get(); hook; hook = hook->next.get()) {
				bool b = ImGui::TreeNodeEx(hook, ImGuiTreeNodeFlags_Leaf | ((ui.selectedHook == hook && !ui.viewGroupInsteadOfHook) ? ImGuiTreeNodeFlags_Selected : 0), "%s %s", hook->getClassName(), kenv.getObjectName(hook));
				IGObjectDragDropSource(ui, hook);
				if (ImGui::IsItemClicked()) {
					ui.selectedHook = hook;
					ui.viewGroupInsteadOfHook = false;
				}
				if (b)
					ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		IGEnumGroup(ui, group->nextGroup.get());
	}
}

void EditorUI::IGHookEditor(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;

	ImGui::Columns(2);
	ImGui::BeginChild("HookTree");
	IGEnumGroup(ui, kenv.levelObjects.getFirst<CKGroupRoot>());
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("HookInfo");
	CKHook* selectedHook = ui.selectedHook.get();
	CKGroup* selectedGroup = ui.selectedGroup.get();
	if (selectedHook && !ui.viewGroupInsteadOfHook) {
		ImGui::Text("%p %s", selectedHook, selectedHook->getClassName());
		IGObjectDragDropSource(ui, selectedHook);
		ImGui::Separator();
		IGObjectNameInput("Name", selectedHook, kenv);
		ImGui::InputScalar("Hook flags", ImGuiDataType_U32, &selectedHook->unk1, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		if (selectedHook->life) {
			// NOTE: Currently modifying life sector is dangerous
			// (e.g. makes PostRef decoding fail since it relies on the
			// life sectors)
			ImGui::LabelText("Life value", "%08X", selectedHook->life->unk1);
			int lifeSector = selectedHook->life->unk1 >> 2;
			int lifeFlags = selectedHook->life->unk1 & 3;
			bool lifeChanged = false;
			lifeChanged |= ImGui::InputInt("Life sector", &lifeSector);
			lifeChanged |= ImGui::InputInt("Life flags", &lifeFlags);
			if (lifeChanged) {
				selectedHook->life->unk1 = (lifeFlags & 3) | (lifeSector << 2);
			}
		}
		if (auto* node = selectedHook->node.get()) {
			ImGui::BeginGroup();
			ImGui::AlignTextToFramePadding();
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - 2 * ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
			ImGui::LabelText("##Node", "%s %s (%p)", node->getClassName(), kenv.getObjectName(node), node);
			ImGui::SameLine();
			if (ImGui::ArrowButton("GoToNode", ImGuiDir_Right)) {
				ui.selNode = node;
				ui.camera.position = node->getGlobalMatrix().getTranslationVector() - ui.camera.direction * 5.0f;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Select and show me where it is");
			}
			ImGui::SameLine();
			ImGui::Text("Node");
			ImGui::EndGroup();
			IGObjectDragDropSource(ui, node);
		}
		ImGui::Separator();
		const auto* clsInfo = ui.g_encyclo.getClassJson(selectedHook->getClassFullID());
		if (clsInfo) {
			if (auto it = clsInfo->find("hookFlags"); it != clsInfo->end()) {
				PropFlagsEditor(selectedHook->unk1, it.value());
			}
		}
		ImGuiMemberListener ml(kenv, ui);
		ml.setPropertyInfoList(ui.g_encyclo, selectedHook);
		selectedHook->virtualReflectMembers(ml, &kenv);

		if (ImGui::Button("Duplicate (UNSTABLE!)")) {
			Duplicator hmd{ kenv, &ui };
			hmd.doClone(selectedHook);
		}
		if (ImGui::Button("Export (UNSTABLE!)")) {
			auto fpath = SaveDialogBox(ui.g_window, "Hook file (*.xechook)\0*.XECHOOK\0", "xechook");
			if (!fpath.empty()) {
				Duplicator hmd{ kenv, &ui };
				hmd.doExport(selectedHook, fpath);
			}
		}

		if (ImGui::Button("Update"))
			selectedHook->update();
	}
	else if (selectedGroup && ui.viewGroupInsteadOfHook) {
		if (ImGui::Button("Import Hook/Group (UNSTABLE!)")) {
			auto fpath = OpenDialogBox(ui.g_window,
				"Either hook or group\0*.XECHOOK;*.XECGROUP\0"
				"Hook (*.xechook)\0*.XECHOOK\0"
				"Group (*.xecgroup)\0*.XECGROUP\0", "xechook");
			if (!fpath.empty()) {
				Duplicator hmd{ kenv, &ui };
				hmd.doImport(fpath, selectedGroup);
				ui.protexdict.reset(kenv.levelObjects.getFirst<CTextureDictionary>());
			}
		}
		if (ImGui::Button("Export Group")) {
			auto fpath = SaveDialogBox(ui.g_window, "Group (*.xecgroup)\0*.XECGROUP\0", "xecgroup");
			if (!fpath.empty()) {
				Duplicator hmd{ kenv, &ui };
				hmd.doExport(selectedGroup, fpath);
			}
		}
		IGObjectNameInput("Name", selectedGroup, kenv);
		if (CKGroup* rgroup = selectedGroup->dyncast<CKGroup>()) {
			ImGuiMemberListener ml(kenv, ui);
			ml.setPropertyInfoList(ui.g_encyclo, rgroup);
			rgroup->virtualReflectMembers(ml, &kenv);
		}
		if (CKGrpLight* grpLight = selectedGroup->dyncast<CKGrpLight>()) {
			CKParticleGeometry* geo = grpLight->node->dyncast<CNode>()->geometry->dyncast<CKParticleGeometry>();
			if (ImGui::Button("Add light")) {
				CKHkLight* light = kenv.createAndInitObject<CKHkLight>();
				light->lightGrpLight = selectedGroup;
				light->activeSector = -1; // TEMP
				selectedGroup->addHook(light);
				geo->pgPoints.emplace(geo->pgPoints.begin(), ui.cursorPosition);
				geo->pgHead2 += 1; geo->pgHead3 += 1;
			}
			int index = 0;
			for (CKHook* objLight = grpLight->childHook.get(); objLight; objLight = objLight->next.get()) {
				ImGui::PushID(objLight);
				ImGui::DragFloat3("##LightPos", &geo->pgPoints[index].x, 0.1f);
				ImGui::PopID();
				++index;
			}
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}
