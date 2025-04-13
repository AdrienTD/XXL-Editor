#include "IGCounterEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include <imgui/imgui.h>

void EditorUI::IGCounterEditor(EditorInterface& ui)
{
	static KWeakRef<CKObject> selectedCounter;

	auto& kenv = ui.kenv;
	CKSrvCounter* srvCounter = kenv.levelObjects.getFirst<CKSrvCounter>();
	if (ImGui::BeginTable("CountersTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("New integer")) {
			kenv.levelObjects.getClassType<CKIntegerCounter>().info = 1;
			srvCounter->integerCounters.emplace_back(kenv.createAndInitObject<CKIntegerCounter>());
		}
		ImGui::SameLine();
		if (ImGui::Button("New timer")) {
			kenv.levelObjects.getClassType<CKTimeCounter>().info = 1;
			srvCounter->timeCounters.emplace_back(kenv.createAndInitObject<CKTimeCounter>());
		}
		ImGui::BeginChild("CounterList");
		for (auto& ref : srvCounter->integerCounters) {
			CKIntegerCounter* intCounter = ref.get();
			ImGui::PushID(intCounter);
			if (ImGui::Selectable("##sel", selectedCounter == intCounter))
				selectedCounter = intCounter;
			ImGui::SameLine();
			ImGui::Text("Integer %s", kenv.getObjectName(intCounter));
			ImGui::PopID();
		}
		ImGui::Separator();
		for (auto& ref : srvCounter->timeCounters) {
			CKTimeCounter* timeCounter = ref.get();
			ImGui::PushID(timeCounter);
			if (ImGui::Selectable("##sel", selectedCounter == timeCounter))
				selectedCounter = timeCounter;
			ImGui::SameLine();
			ImGui::Text("Timer %s", kenv.getObjectName(timeCounter));
			ImGui::PopID();
		}
		ImGui::EndChild();

		ImGui::TableNextColumn();

		if (selectedCounter) {
			IGObjectNameInput("Name", selectedCounter.get(), kenv);
			ImGuiMemberListener ml{ kenv, ui };
			ml.setPropertyInfoList(ui.g_encyclo, selectedCounter.get());
			if (CKIntegerCounter* intCounter = selectedCounter->dyncast<CKIntegerCounter>()) {
				intCounter->reflectMembers2(ml, &kenv);
			}
			else if (CKTimeCounter* timeCounter = selectedCounter->dyncast<CKTimeCounter>()) {
				timeCounter->reflectMembers2(ml, &kenv);
			}
		}

		ImGui::EndTable();
	}
}
