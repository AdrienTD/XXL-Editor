#include "IGTriggerEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

#include <imgui/imgui.h>

void EditorUI::IGTriggerEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	CKSrvTrigger* srvTrigger = kenv.levelObjects.getFirst<CKSrvTrigger>();
	if (!srvTrigger) return;
	ImGui::Columns(2);
	ImGui::BeginChild("TriggerTree");
	auto enumDomain = [&ui, &kenv](CKTriggerDomain* domain, const auto& rec) -> void {
		bool open = ImGui::TreeNode(domain, "%s", kenv.getObjectName(domain));
		if (open) {
			for (const auto& subdom : domain->subdomains)
				rec(subdom.get(), rec);
			for (const auto& trigger : domain->triggers) {
				bool open = ImGui::TreeNodeEx(trigger.get(), ImGuiTreeNodeFlags_Leaf, "%s", kenv.getObjectName(trigger.get()));
				if (ImGui::IsItemClicked())
					ui.selectedTrigger = trigger.get();
				if (open)
					ImGui::TreePop();
			}
			if (ImGui::SmallButton("Add")) {
				domain->triggers.emplace_back(kenv.createAndInitObject<CKTrigger>());
			}
			ImGui::TreePop();
		}
		};
	enumDomain(srvTrigger->rootDomain.get(), enumDomain);
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("TriggerView");
	if (ui.selectedTrigger) {
		static const char* combinerNames[5] = {
			"All (and)",
			"Not all (nand)",
			"At least one (ori)",
			"None (nori)",
			"Only one (xor)"
		};
		static const char* comparatorNames[6] = {
			"<=  Less than or equal",
			"<   Less than",
			">=  Greater than or equal",
			">   Greater than",
			"==  Equal",
			"!=  Different"
		};

		IGObjectNameInput("Name", ui.selectedTrigger.get(), kenv);
		static ImDrawListSplitter splitter;
		splitter.Split(ImGui::GetWindowDrawList(), 3);
		splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 2);

		auto trimPath = [](const char* name) -> std::string {
			std::string str = name;
			size_t pathIndex = str.find("(/Domaine racine");
			if (pathIndex != str.npos) str.resize(pathIndex);
			return str;
			};
		auto addButton = [&ui, &kenv]() -> CKConditionNode* {
			if (ImGui::Button("Add")) {
				ImGui::OpenPopup("AddCondNodeMenu");
			}
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EventNodeX2")) {
					auto& [eventNode, name] = *(const EventNodePayload*)payload->Data;
					CKComparator* cmp = kenv.createAndInitObject<CKComparator>();
					cmp->condNodeType = 8; // equal
					kenv.setObjectName(cmp, comparatorNames[4]);

					cmp->leftCmpData = kenv.createAndInitObject<CKComparedData>();
					cmp->leftCmpData->cmpdatType = 2 << 2;
					auto& cden = cmp->leftCmpData->cmpdatValue.emplace<CKComparedData::CmpDataEventNode>();
					cden.cmpdatT2Trigger = ui.selectedTrigger.get();
					kenv.setObjectName(cmp->leftCmpData.get(), name);

					cmp->rightCmpData = kenv.createAndInitObject<CKComparedData>();
					cmp->rightCmpData->cmpdatType = 1 << 2;
					auto& cdc = cmp->rightCmpData->cmpdatValue.emplace<CKComparedData::CmpDataConstant>();
					cdc.value.emplace<uint8_t>(1);
					kenv.setObjectName(cmp->rightCmpData.get(), "True");

					eventNode->datas.emplace_back(cmp->leftCmpData.get());

					ImGui::EndDragDropTarget();
					return cmp;
				}
				ImGui::EndDragDropTarget();
			}
			if (ImGui::BeginPopup("AddCondNodeMenu")) {
				int addComparator = -1, addCombiner = -1;
				for (int i = 0; i < (int)std::size(comparatorNames); ++i) {
					if (ImGui::Selectable(comparatorNames[i])) {
						addComparator = i;
					}
				}
				ImGui::Separator();
				for (int i = 0; i < (int)std::size(combinerNames); ++i) {
					if (ImGui::Selectable(combinerNames[i])) {
						addCombiner = i;
					}
				}
				ImGui::EndPopup();
				if (addComparator != -1) {
					CKComparator* cmp = kenv.createAndInitObject<CKComparator>();
					cmp->condNodeType = addComparator << 1;
					kenv.setObjectName(cmp, comparatorNames[addComparator]);
					return cmp;
				}
				else if (addCombiner != -1) {
					CKCombiner* comb = kenv.createAndInitObject<CKCombiner>();
					comb->condNodeType = addCombiner << 1;
					kenv.setObjectName(comb, combinerNames[addCombiner]);
					return comb;
				}
			}
			return nullptr;
			};
		auto walkCondNode = [&kenv, trimPath, addButton](CKConditionNode* node, auto rec) -> void {
			const char* name = "?";
			uint32_t boxColor = 0;
			int boxChannel = 2;
			if (CKCombiner* c = node->dyncast<CKCombiner>()) {
				name = combinerNames[c->condNodeType >> 1];
				boxColor = 0x40FF4020;
				boxChannel = 0;
			}
			else if (CKComparator* c = node->dyncast<CKComparator>()) {
				name = comparatorNames[c->condNodeType >> 1];
				boxColor = 0xFF008000;
				boxChannel = 1;
			}
			const ImVec2 boxStart = ImGui::GetCursorScreenPos();
			if (ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_DefaultOpen, "%s", name)) { //trimPath(kenv.getObjectName(node)).c_str()
				if (CKCombiner* comb = node->dyncast<CKCombiner>()) {
					for (auto& child : comb->condNodeChildren)
						rec(child.get(), rec);
					if (CKConditionNode* added = addButton())
						comb->condNodeChildren.emplace_back(added);
				}
				else if (CKComparator* cmp = node->dyncast<CKComparator>()) {
					//ImGuiMemberListener ml{ kenv, *this };
					ImGui::Bullet(); ImGui::TextUnformatted(trimPath(kenv.getObjectName(cmp->leftCmpData.get())).c_str());
					//cmp->leftCmpData->virtualReflectMembers(ml, &kenv);
					ImGui::Bullet(); ImGui::TextUnformatted(trimPath(kenv.getObjectName(cmp->rightCmpData.get())).c_str());
					//cmp->rightCmpData->virtualReflectMembers(ml, &kenv);
				}
				ImGui::TreePop();
			}
			const ImVec2 boxEnd = ImVec2(ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x, ImGui::GetCursorScreenPos().y - 2.0f);
			splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), boxChannel);
			ImGui::GetWindowDrawList()->AddRectFilled(boxStart, boxEnd, boxColor, 4.0f);
			splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 2);
			};
		auto removeCondNode = [&kenv](CKConditionNode* node, const auto& rec) -> void {
			if (CKCombiner* comb = node->dyncast<CKCombiner>()) {
				for (auto& ref : comb->condNodeChildren) {
					CKConditionNode* sub = ref.get();
					ref = nullptr;
					rec(sub, rec);
				}
			}
			else if (CKComparator* cmp = node->dyncast<CKComparator>()) {
				CKComparedData* left = cmp->leftCmpData.get();
				CKComparedData* right = cmp->rightCmpData.get();
				cmp->leftCmpData = nullptr;
				cmp->rightCmpData = nullptr;
				kenv.removeObject(left);
				kenv.removeObject(right);
			}
			kenv.removeObject(node);
			};
		if (ImGui::Button("Clear condition")) {
			if (CKConditionNode* cnd = ui.selectedTrigger->condition.get()) {
				ui.selectedTrigger->condition = nullptr;
				removeCondNode(cnd, removeCondNode);
			}
		}
		if (ui.selectedTrigger->condition) {
			walkCondNode(ui.selectedTrigger->condition.get(), walkCondNode);
		}
		else {
			if (CKConditionNode* added = addButton()) {
				ui.selectedTrigger->condition = added;
			}
		}
		splitter.Merge(ImGui::GetWindowDrawList());

		int acttodelete = -1;
		for (size_t actindex = 0; actindex < ui.selectedTrigger->actions.size(); actindex++) {
			auto& act = ui.selectedTrigger->actions[actindex];
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::PushID(&act);

			bool updateParameter = false;
			
			ImGui::SetNextItemWidth(-32.0f);
			if (IGObjectSelectorRef(ui, "##EventObj", act.target)) {
				updateParameter = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				acttodelete = actindex;
			}
			ImGui::SetItemTooltip("Delete");

			ImGui::SetNextItemWidth(-32.0f);
			if (IGEventMessageSelector(ui, "##EventID", act.event, act.target.get())) {
				updateParameter = true;
			}

			static const char* typeNames[4] = { "bool", "int", "float", "ref" };

			const char* knownParameterName = "Parameter";
			std::string eventKnownDataTypeName;
			int eventKnownDataTypeIndex = -1;

			if (act.target.bound && act.target.get()) {
				auto* jsEvent = ui.g_encyclo.getEventJson(act.target->getClassHierarchy(), act.event);
				if (jsEvent) {
					if (auto itParameter = jsEvent->find("parameter"); itParameter != jsEvent->end()) {
						if (itParameter->is_string()) {
							knownParameterName = itParameter->get_ref<const std::string&>().c_str();
						}
						else if (itParameter->is_object()) {
							if (auto itName = itParameter->find("name"); itName != itParameter->end()) {
								knownParameterName = itName->get_ref<const std::string&>().c_str();
							}
						}
					}

					eventKnownDataTypeName = jsEvent->value<std::string>("dataType", {});
					if (auto itName = std::ranges::find(typeNames, eventKnownDataTypeName); itName != std::end(typeNames)) {
						eventKnownDataTypeIndex = int(itName - std::begin(typeNames));
					}
				}
			}

			bool showParameter = eventKnownDataTypeName != "none";
			bool showDatatypeCombobox = eventKnownDataTypeIndex == -1;

			if (updateParameter) {
				if (!showParameter) {
					act.value.emplace<0>(0);
				}
				else if (eventKnownDataTypeIndex != -1) {
					changeVariantType(act.value, eventKnownDataTypeIndex);
				}
			}

			if (showParameter) {
				if (showDatatypeCombobox) {
					int valType = (int)act.value.index();
					ImGui::SetNextItemWidth(64.0f);
					if (ImGui::Combo("##ValueType", &valType, typeNames, 4))
						changeVariantType(act.value, (size_t)valType);
					ImGui::SameLine();
				}

				ImGui::SetNextItemWidth(-32.0f - 80.0f);
				switch (act.value.index()) {
				case 0: {
					bool b = std::get<uint8_t>(act.value) != 0;
					if (ImGui::Checkbox(knownParameterName, &b))
						act.value = b ? uint8_t(1) : uint8_t(0);
					break;
				}
				case 1: ImGui::InputScalar(knownParameterName, ImGuiDataType_U32, &std::get<uint32_t>(act.value)); break;
				case 2: ImGui::InputScalar(knownParameterName, ImGuiDataType_Float, &std::get<float>(act.value)); break;
				case 3: IGObjectSelectorRef(ui, knownParameterName, std::get<KPostponedRef<CKObject>>(act.value)); break;
				}
			}
			ImGui::PopID();
		}
		if (acttodelete != -1)
			ui.selectedTrigger->actions.erase(ui.selectedTrigger->actions.begin() + acttodelete);
		if (ImGui::Button("New action")) {
			ui.selectedTrigger->actions.emplace_back();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}
