#include "IGSquadEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "GuiUtils.h"
#include "ImGuiMemberListener.h"
#include "PropFlagsEditor.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKLogic.h"
#include "GameClasses/CKGameX1.h"
#include "GameClasses/CKGameX2.h"

#include "Duplicator.h"

#include <imgui/imgui.h>

namespace
{
	using namespace EditorUI;

	void IGComponentEditor(EditorInterface& ui, CKEnemyCpnt* cpnt)
	{
		ImGui::PushItemWidth(130.0f);

		ImGuiMemberListener igml(ui.kenv, ui);
		igml.setPropertyInfoList(ui.g_encyclo, cpnt);
		cpnt->virtualReflectMembers(igml, &ui.kenv);

		ImGui::PopItemWidth();
	}
}

void EditorUI::IGSquadEditorXXL1(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;

	ImGui::Columns(2);
	ImGui::BeginChild("SquadList");
	int wantToAdd = -1;
	if (ImGui::Button("New"))
		wantToAdd = CKGrpSquadEnemy::FULL_ID;
	ImGui::SameLine();
	if (ImGui::Button("New JetPack"))
		wantToAdd = CKGrpSquadJetPack::FULL_ID;
	if (wantToAdd != -1) {
		CKGrpSquadEnemy* squad = (CKGrpSquadEnemy*)kenv.createObject((uint32_t)wantToAdd, -1);
		squad->bundle = kenv.createObject<CKBundle>(-1);
		squad->msgAction = kenv.createObject<CKMsgAction>(-1);

		CKGrpEnemy* grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>();
		grpEnemy->addGroup(squad);

		squad->bundle->priority = 0x7F;
		CKServiceLife* svcLife = kenv.levelObjects.getFirst<CKServiceLife>();
		svcLife->addBundle(squad->bundle.get());

		squad->msgAction->states = { CKMsgAction::MAState{ { CKMsgAction::MAMessage{0xD16, { {14, {} } } } } } };
		squad->mat1 = Matrix::getTranslationMatrix(ui.cursorPosition);
		squad->mat2 = squad->mat1;
		squad->sqUnk2 = ui.cursorPosition;
		squad->sqUnk3 = { ui.cursorPosition, {20.0f, 10.0f, 20.0f} };
		squad->sqUnk4 = { ui.cursorPosition, {20.0f, 10.0f, 20.0f} };

		CKChoreography* choreo = kenv.createObject<CKChoreography>(-1);
		choreo->numKeys = 1;
		CKChoreoKey* key = kenv.createObject<CKChoreoKey>(-1);
		squad->choreographies = { choreo };
		squad->choreoKeys = { key };
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!ui.selectedSquad);
	if (ImGui::Button("Duplicate") && ui.selectedSquad) {
		CKGrpSquadEnemy* squad = ui.selectedSquad.get();
		CKGrpSquadEnemy* clone;
		if (CKGrpSquadJetPack* jpsquad = squad->dyncast<CKGrpSquadJetPack>()) {
			CKGrpSquadJetPack* jpclone = kenv.createObject<CKGrpSquadJetPack>(-1);
			*jpclone = *jpsquad;
			clone = jpclone;
		}
		else {
			clone = kenv.createObject<CKGrpSquadEnemy>(-1);
			*clone = *squad;
		}

		CKGrpEnemy* grpEnemy = squad->parentGroup->cast<CKGrpEnemy>();
		grpEnemy->addGroup(clone);

		clone->bundle = kenv.createObject<CKBundle>(-1);
		clone->bundle->priority = 0x7F;
		CKServiceLife* svcLife = kenv.levelObjects.getFirst<CKServiceLife>();
		//clone->bundle->next = svcLife->firstBundle;
		//svcLife->firstBundle = clone->bundle;
		svcLife->addBundle(clone->bundle.get());

		clone->msgAction = kenv.createObject<CKMsgAction>(-1);
		*clone->msgAction = *squad->msgAction;

		for (auto& ref : clone->choreographies) {
			CKChoreography* oriChoreo = ref.get();
			ref = kenv.createObject<CKChoreography>(-1);
			*ref = *oriChoreo;
		}
		for (auto& ref : clone->choreoKeys) {
			CKChoreoKey* ori = ref.get();
			ref = kenv.createObject<CKChoreoKey>(-1);
			*ref = *ori;
		}
		for (auto& pool : clone->pools) {
			pool.cpnt = kenv.cloneObject(pool.cpnt.get());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete") && ui.selectedSquad) {
		CKGrpSquadEnemy* squad = ui.selectedSquad.get();
		if (squad->getRefCount() > 1) {
			MsgBox(ui.g_window, "The squad is still being referenced.", 16);
		}
		else {
			auto removeObj = [&kenv](kanyobjref& ref) {
				CKObject* obj = ref.get();
				ref.anyreset();
				kenv.removeObject(obj);
				};
			removeObj(squad->msgAction);
			for (auto& ref : squad->choreographies)
				removeObj(ref);
			for (auto& ref : squad->choreoKeys)
				removeObj(ref);
			for (auto& pool : squad->pools)
				removeObj(pool.cpnt);
			squad->parentGroup->removeGroup(squad);
			kenv.levelObjects.getFirst<CKServiceLife>()->removeBundle(squad->bundle.get());
			removeObj(squad->bundle);
			kenv.removeObject(squad);
		}
	}
	ImGui::EndDisabled();
	auto enumSquad = [&ui, &kenv](CKObject* osquad, int si, bool jetpack) {
		CKGrpSquadEnemy* squad = osquad->cast<CKGrpSquadEnemy>();
		int numEnemies = 0;
		for (auto& pool : squad->pools) {
			numEnemies += pool.numEnemies;
		}
		ImGui::PushID(squad);
		if (ImGui::SmallButton("View")) {
			ui.camera.position = squad->mat1.getTranslationVector() - ui.camera.direction * 15.0f;
		}
		ImGui::SameLine();
		if (ImGui::Selectable("##SquadItem", ui.selectedSquad == squad)) {
			ui.selectedSquad = squad;
		}
		IGObjectDragDropSource(ui, squad);
		ImGui::SameLine();
		ImGui::Text("%s %i (%i): %s", jetpack ? "JetPack Squad" : "Squad", si, numEnemies, kenv.getObjectName(squad));
		ImGui::PopID();
		};
	int si = 0;
	for (CKObject* osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
		enumSquad(osquad, si++, false);
	}
	si = 0;
	for (CKObject* osquad : kenv.levelObjects.getClassType<CKGrpSquadJetPack>().objects) {
		enumSquad(osquad, si++, true);
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	if (ui.selectedSquad) {
		CKGrpSquadEnemy* squad = ui.selectedSquad.get();
		if (ImGui::BeginTabBar("SquadInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::BeginChild("SquadMain");
				IGObjectNameInput("Name", squad, kenv);
				ImGuiMemberListener ml(kenv, ui);
				ml.setPropertyInfoList(ui.g_encyclo, squad);
				if (ImGui::Button("Fix all position vectors")) {
					Vector3 pos = squad->mat1.getTranslationVector();
					squad->mat2 = squad->mat1;
					squad->sqUnk2 = pos;
					squad->sqUnk3[0] = pos;
					squad->sqUnk4[0] = pos;
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("according to mat1");
				ml.reflect(squad->bsUnk1, "bsUnk1");
				ml.reflect(*(Vector3*)&squad->mat1._41, "mat1");
				ml.reflect(*(Vector3*)&squad->mat2._41, "mat2");
				ml.reflect(squad->sqUnk1, "sqUnk1");
				ml.reflect(squad->sqUnk2, "sqUnk2");
				ml.reflectContainer(squad->sqUnk3, "sqUnk3");
				ml.reflectContainer(squad->sqUnk4, "sqUnk4");
				ml.reflect(squad->sqUnk5, "sqUnk5");
				ml.reflectContainer(squad->fings, "fings");
				ml.reflectContainer(squad->sqUnk6, "sqUnk6");
				ml.reflect(squad->sqUnk6b, "sqUnk6b");
				ml.reflect(squad->sqUnk7, "sqUnk7");
				ml.reflect(squad->sqUnk8, "sqUnk8");
				ml.reflect(squad->sqUnkB, "sqUnkB");
				ml.reflect(squad->sqUnkA, "Event 1", squad);
				ml.reflect(squad->sqUnkC, "Event 2", squad);
				ml.reflect(squad->seUnk1, "seUnk1");
				ml.reflect(squad->seUnk2, "seUnk2");
				if (kenv.isRemaster) {
					bool isChallenge = squad->sqRomasterValue != 0;
					ImGui::Checkbox("Part of Romaster Challenge", &isChallenge);
					squad->sqRomasterValue = isChallenge ? 1 : 0;
				}
				if (CKGrpSquadJetPack* jpsquad = squad->dyncast<CKGrpSquadJetPack>()) {
					ImGui::Spacing();
					ml.reflectSize<uint16_t>(jpsquad->hearths, "Num hearths");
					ml.reflectContainer(jpsquad->hearths, "hearths");
					ml.reflect(jpsquad->sjpUnk1, "sjpUnk1");
					ml.reflect(jpsquad->sjpUnk2, "sjpUnk2");
					ml.reflect(jpsquad->sjpUnk3, "sjpUnk3");
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Markers")) {
				ImGui::BeginChild("MarkersChild");
				for (auto [name, list] : { std::make_pair("Spawning points", &squad->spawnMarkers), std::make_pair("Guard points",&squad->guardMarkers) }) {
					ImGui::PushID(name);
					if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
						if (ImGui::Button("Add")) {
							list->emplace_back();
						}
						ImGui::SameLine();
						if (ImGui::Button("Clear")) {
							list->clear();
						}
						for (auto& pnt : *list) {
							ImGui::Separator();
							ImGui::PushID(&pnt);
							IGMarkerSelector(ui, "Marker Index", pnt.markerIndex);
							ImGui::InputScalar("Byte", ImGuiDataType_U8, &pnt.b);
							// TODO: Modify marker properties directly here
							ImGui::PopID();
						}
					}
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("MsgAction")) {
				static std::map<int, nlohmann::json> actionMap;
				static bool actionMapRead = false;
				if (!actionMapRead) {
					auto [smaPtr, smaSize] = GetResourceContent("SquadMsgActions.json");
					assert(smaPtr && smaSize > 0);
					nlohmann::json actionJson = nlohmann::json::parse((const char*)smaPtr, (const char*)smaPtr + smaSize);
					for (auto& elem : actionJson.at("actions")) {
						actionMap.insert_or_assign(elem.at("number").get<int>(), std::move(elem));
					}
					actionMapRead = true;
				}
				auto getActionName = [](int num) -> std::string {
					auto it = actionMap.find(num);
					if (it != actionMap.end()) {
						return it->second.at("name").get<std::string>();
					}
					else {
						return "Unknown action " + std::to_string(num);
					}
					};
				auto getParameterJson = [](int action, int param) -> const nlohmann::json* {
					auto it = actionMap.find(action);
					if (it != actionMap.end()) {
						auto& plist = it->second.at("parameters");
						if (plist.is_array() && param >= 0 && param < (int)plist.size())
							return &plist.at(param);
					}
					return nullptr;
					};

				CKMsgAction* msgAction = squad->msgAction->cast<CKMsgAction>();
				static size_t stateIndex = 0;
				if (ImGui::Button("New state"))
					msgAction->states.emplace_back();
				ImGui::SameLine();
				if (ImGui::Button("Duplicate") && stateIndex < msgAction->states.size())
					msgAction->states.push_back(msgAction->states[stateIndex]);
				ImGui::SameLine();
				if (ImGui::ArrowButton("StateDown", ImGuiDir_Down) && stateIndex + 1 < msgAction->states.size()) {
					std::swap(msgAction->states[stateIndex], msgAction->states[stateIndex + 1]);
					++stateIndex;
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move selected state down");
				ImGui::SameLine();
				if (ImGui::ArrowButton("StateUp", ImGuiDir_Up) && stateIndex >= 1 && stateIndex < msgAction->states.size()) {
					std::swap(msgAction->states[stateIndex], msgAction->states[stateIndex - 1]);
					--stateIndex;
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move selected state up");
				ImGui::SameLine();
				if (ImGui::Button("Remove") && stateIndex < msgAction->states.size())
					msgAction->states.erase(msgAction->states.begin() + stateIndex);
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
					msgAction->states.clear();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::BeginListBox("##StateList", ImVec2(0.0f, 80.0f))) {
					int i = 0;
					for (auto& a : msgAction->states) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##StateEntry", stateIndex == (size_t)i)) {
							stateIndex = i;
						}
						ImGui::SameLine();
						ImGui::Text("%i: %s (%zi Msgs)", i, a.name.c_str(), a.messageHandlers.size());
						ImGui::PopID();
						++i;
					}
					ImGui::EndListBox();
				}
				if (stateIndex >= 0 && stateIndex < (int)msgAction->states.size()) {
					auto& a = msgAction->states[stateIndex];
					IGStringInput("Name", a.name);
					if (ImGui::Button("New message handler"))
						a.messageHandlers.emplace_back();
					ImGui::BeginChild("MsgActionWnd");
					CKMsgAction::MAMessage* removeMsg = nullptr;
					CKMsgAction::MAAction* removeAct = nullptr;
					for (auto& b : a.messageHandlers) {
						bool wannaKeepMessage = true;
						ImGui::PushID(&b);
						ImGui::SetNextItemOpen(true, ImGuiCond_Once);
						bool hopen = ImGui::CollapsingHeader("##MessageHeader", &wannaKeepMessage);
						ImGui::SameLine();
						const auto squadClassIds = kenv.factories.at(squad->getClassFullID()).hierarchy;
						ImGui::Text("Message %s", ui.g_encyclo.getEventName(ui.g_encyclo.getEventJson(squadClassIds, b.event), b.event).c_str());
						if (hopen) {
							ImGui::Indent();
							uint16_t msg = (uint16_t)b.event;
							if (IGEventMessageSelector(ui, "Message", msg, squad, true))
								b.event = msg;

							if (ImGui::Button("New action"))
								b.actions.emplace_back().num = 14;
							for (auto& c : b.actions) {
								//if (&c != &b.actions.front())
								ImGui::Separator();
								ImGui::PushID(&c);
								auto itActInfo = actionMap.find(c.num);
								if (ImGui::BeginCombo("Action", getActionName(c.num).c_str())) {
									for (auto& [num, info] : actionMap) {
										if (ImGui::Selectable(getActionName(num).c_str(), c.num == num)) {
											c.num = num;
											c.parameters.clear();
											auto it = actionMap.find(c.num);
											if (it != actionMap.end()) {
												for (auto& param : it->second.at("parameters")) {
													auto& typestr = param.at("type").get_ref<std::string&>();
													if (typestr == "int0") c.parameters.emplace_back(std::in_place_index<0>);
													else if (typestr == "int1") c.parameters.emplace_back(std::in_place_index<1>);
													else if (typestr == "float") c.parameters.emplace_back(std::in_place_index<2>);
													else if (typestr == "ref") c.parameters.emplace_back(std::in_place_index<3>);
													else if (typestr == "marker") c.parameters.emplace_back(std::in_place_index<4>);

													if (auto it = param.find("content"); it != param.end() && *it == "comparatorFloat") {
														std::get<1>(c.parameters.back()) = 2;
													}
												}
											}
										}
									}
									ImGui::EndCombo();
								}
								ImGui::PushItemWidth(ImGui::CalcItemWidth() - 32.0f);
								int i = 0;
								for (auto& d : c.parameters) {
									const nlohmann::json* paramJson = getParameterJson(c.num, i);
									assert(paramJson);
									const std::string& paramName = paramJson->at("name").get_ref<const std::string&>();
									const char* tbuf = paramName.c_str();
									ImGui::PushID(&d);
									ImGui::SetNextItemWidth(32.0f);
									int type = (int)d.index();
									static const std::array typeAbbr = { "Int0", "Int1", "Flt", "Ref", "Mark" };
									ImGui::LabelText("##Type", typeAbbr.at(type));
									ImGui::SameLine();
									switch (d.index()) {
									case 0: {
										uint32_t& num0 = std::get<0>(d);
										if (auto it = paramJson->find("content"); it != paramJson->end() && it->get_ref<const std::string&>() == "event") {
											uint16_t msg = (uint16_t)num0;
											if (IGEventMessageSelector(ui, tbuf, msg, GameX1::CKHkEnemy::FULL_ID))
												num0 = msg;
										}
										else {
											ImGui::InputInt(tbuf, (int*)&num0);
										}
										break;
									}
									case 1: {
										uint32_t& num1 = std::get<1>(d);
										std::span<const std::pair<int, const char*>> comboEnums;
										if (auto it = paramJson->find("content"); it != paramJson->end()) {
											const std::string& content = it->get_ref<const std::string&>();
											if (content == "comparatorInt") {
												static const std::pair<int, const char*> cmpIntEnums[] = {
													{0, "=="}, {1, "!="}, {2, "<"}, {3, "<="}, {4, ">"}, {5, ">="}
												};
												comboEnums = cmpIntEnums;
											}
											else if (content == "comparatorFloat") {
												static const std::pair<int, const char*> cmpFloatEnums[] = {
													{2, "<"}, {4, ">"}
												};
												comboEnums = cmpFloatEnums;
											}
										}
										if (!comboEnums.empty()) {
											const char* preview = "?";
											auto itEnum = std::ranges::find(comboEnums, (int)num1, [](const auto& pair) {return pair.first; });
											if (itEnum != comboEnums.end())
												preview = itEnum->second;
											if (ImGui::BeginCombo(tbuf, preview)) {
												for (const auto& [enumNum, enumName] : comboEnums) {
													if (ImGui::Selectable(enumName, num1 == enumNum))
														num1 = enumNum;
												}
												ImGui::EndCombo();
											}
										}
										else {
											ImGui::InputInt(tbuf, (int*)&num1);
										}
										break;
									}
									case 2:
										ImGui::InputFloat(tbuf, &std::get<2>(d)); break;
									case 3:
										IGObjectSelectorRef(ui, tbuf, std::get<kobjref<CKObject>>(d)); break;
									case 4:
										IGMarkerSelector(ui, tbuf, std::get<MarkerIndex>(d)); break;
									}
									ImGui::PopID();
									++i;
								}
								ImGui::PopItemWidth();
								if (ImGui::SmallButton("Remove")) {
									removeMsg = &b; removeAct = &c;
								}
								ImGui::PopID();
							}
							ImGui::Unindent();
							ImGui::Spacing();
						}
						if (!wannaKeepMessage)
							removeMsg = &b;
						ImGui::PopID();
					}
					if (removeAct)
						removeMsg->actions.erase(removeMsg->actions.begin() + (removeAct - removeMsg->actions.data()));
					else if (removeMsg)
						a.messageHandlers.erase(a.messageHandlers.begin() + (removeMsg - a.messageHandlers.data()));

					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Choreographies")) {
				ImGui::BeginChild("SquadChoreos");
				ImGui::Text("Num choreo: %i, Num choreo keys: %i", squad->choreographies.size(), squad->choreoKeys.size());
				auto getChoreo = [squad](int key) -> int {
					int cindex = 0, kindex = 0;
					for (auto& choreo : squad->choreographies) {
						if (key >= kindex && key < kindex + choreo->numKeys) {
							return cindex;
						}
						kindex += choreo->numKeys;
						cindex++;
					}
					return -1;
					};
				auto getFirstKey = [squad](int choreoIndex) -> int {
					int cindex = 0, kindex = 0;
					for (auto& choreo : squad->choreographies) {
						if (cindex == choreoIndex) {
							return kindex;
						}
						kindex += choreo->numKeys;
						cindex++;
					}
					return -1;
					};
				static int removeChoreography = -1;
				bool pleaseOpenRemoveChoreoPopup = false;
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::BeginListBox("##ChoreoList")) {
					int cindex = 0;
					int kindex = 0;
					for (auto& choreo : squad->choreographies) {
						ImGui::PushID(choreo.get());
						ImGui::BulletText("Choreo %i: %s", cindex, kenv.getObjectName(choreo.get()));
						ImGui::SameLine();
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::GetTextLineHeightWithSpacing());
						if (ImGui::SmallButton("X")) {
							pleaseOpenRemoveChoreoPopup = true;
							removeChoreography = cindex;
						}
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Remove");
						for (int k = kindex; k < kindex + choreo->numKeys; ++k) {
							ImGui::PushID(k);
							ImGui::Indent();
							if (ImGui::Selectable("##ChoreoEntry", ui.showingChoreoKey == k))
								ui.showingChoreoKey = k;
							ImGui::SameLine();
							ImGui::Text("Key %i: %s", k, kenv.getObjectName(squad->choreoKeys[k].get()));
							ImGui::Unindent();
							ImGui::PopID();
						}
						ImGui::PopID();
						cindex++;
						kindex += choreo->numKeys;
					}
					ImGui::EndListBox();
				}

				if (pleaseOpenRemoveChoreoPopup)
					ImGui::OpenPopup("DeleteChoreography");
				if (ImGui::BeginPopup("DeleteChoreography")) {
					CKChoreography* kchoreo = squad->choreographies[removeChoreography].get();
					ImGui::Text("Remove choreography %i\nand its %i keys?", removeChoreography, kchoreo->numKeys);
					if (ImGui::Button("Yes")) {
						int firstindex = getFirstKey(removeChoreography);
						auto rembegin = squad->choreoKeys.begin() + firstindex;
						auto remend = rembegin + kchoreo->numKeys;
						for (auto it = rembegin; it != remend; ++it) {
							CKChoreoKey* key = it->get();
							*it = nullptr;
							kenv.removeObject(key);
						}
						squad->choreoKeys.erase(rembegin, remend);

						squad->choreographies.erase(squad->choreographies.begin() + removeChoreography);
						kenv.removeObject(kchoreo);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				bool keyInRange = ui.showingChoreoKey >= 0 && ui.showingChoreoKey < (int)squad->choreoKeys.size();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Choreo key:  ");
				ImGui::SameLine();
				if (ImGui::Button("New##Choreokey") && !squad->choreographies.empty()) {
					CKChoreoKey* newkey = kenv.createObject<CKChoreoKey>(-1);
					ui.selectedSquad->choreoKeys.push_back(newkey);
					ui.selectedSquad->choreographies.back()->numKeys += 1;
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate##Choreokey") && keyInRange) {
					int cindex = getChoreo(ui.showingChoreoKey);
					CKChoreoKey* original = ui.selectedSquad->choreoKeys[ui.showingChoreoKey].get();
					CKChoreoKey* clone = kenv.cloneObject(original);
					ui.selectedSquad->choreoKeys.emplace(ui.selectedSquad->choreoKeys.begin() + ui.showingChoreoKey + 1, clone);
					ui.selectedSquad->choreographies[cindex]->numKeys += 1;
					kenv.setObjectName(clone, std::string("Copy of ") + kenv.getObjectName(original));
				}
				ImGui::SameLine();
				if (ImGui::ArrowButton("MoveChoreoKeyDown", ImGuiDir_Down) && keyInRange) {
					int kindex = ui.showingChoreoKey;
					int cindex = getChoreo(kindex);
					int firstkeyindex = getFirstKey(cindex);
					int lastkeyindex = firstkeyindex + squad->choreographies[cindex]->numKeys - 1;
					if (kindex == lastkeyindex) {
						if (cindex < (int)squad->choreographies.size() - 1) {
							squad->choreographies[cindex]->numKeys -= 1;
							squad->choreographies[cindex + 1]->numKeys += 1;
						}
					}
					else {
						if (kindex < (int)squad->choreoKeys.size() - 1) {
							std::swap(squad->choreoKeys[kindex], squad->choreoKeys[kindex + 1]);
							ui.showingChoreoKey += 1;
						}
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Move selected Choreokey down");
				ImGui::SameLine();
				if (ImGui::ArrowButton("MoveChoreoKeyUp", ImGuiDir_Up) && keyInRange) {
					int kindex = ui.showingChoreoKey;
					int cindex = getChoreo(kindex);
					int firstkeyindex = getFirstKey(cindex);
					if (kindex == firstkeyindex) {
						if (cindex >= 1) {
							squad->choreographies[cindex]->numKeys -= 1;
							squad->choreographies[cindex - 1]->numKeys += 1;
						}
					}
					else {
						if (kindex >= 1) {
							std::swap(squad->choreoKeys[kindex], squad->choreoKeys[kindex - 1]);
							ui.showingChoreoKey -= 1;
						}
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Move selected Choreokey up");
				ImGui::SameLine();
				if (ImGui::Button("X") && keyInRange) {
					int cindex = getChoreo(ui.showingChoreoKey);
					CKChoreoKey* key = squad->choreoKeys[ui.showingChoreoKey].get();
					squad->choreoKeys.erase(squad->choreoKeys.begin() + ui.showingChoreoKey);
					squad->choreographies[cindex]->numKeys -= 1;
					kenv.removeObject(key);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Remove selected Choreokey");

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Choreography:");
				ImGui::SameLine();
				if (ImGui::Button("New##Choreography")) {
					CKChoreography* ref = kenv.createObject<CKChoreography>(-1);
					ui.selectedSquad->choreographies.push_back(ref);
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate##ChoreoGraphy") && keyInRange) {
					int cindex = getChoreo(ui.showingChoreoKey);
					int firstkey = getFirstKey(cindex);
					CKChoreography* ori = ui.selectedSquad->choreographies[cindex].get();;
					CKChoreography* ref = kenv.createObject<CKChoreography>(-1);
					*ref = *ori;
					ui.selectedSquad->choreographies.push_back(ref);
					kenv.setObjectName(ref, std::string("Copy of ") + kenv.getObjectName(ori));

					for (int currentKey = 0; currentKey < int(ori->numKeys); currentKey++) {
						CKChoreoKey* ori = ui.selectedSquad->choreoKeys[firstkey + currentKey].get();
						CKChoreoKey* ref = kenv.createObject<CKChoreoKey>(-1);
						*ref = *ori;
						ui.selectedSquad->choreoKeys.push_back(ref);
						kenv.setObjectName(ref, std::string("Copy of ") + kenv.getObjectName(ori));
					}
				}

				//ImGui::InputInt("ChoreoKey", &ui.showingChoreoKey);
				int ckeyindex = ui.showingChoreoKey;
				if (ckeyindex >= 0 && ckeyindex < (int)squad->choreoKeys.size()) {
					CKChoreoKey* ckey = squad->choreoKeys[ckeyindex].get();
					ImGui::Separator();
					IGObjectNameInput("Choreo name", squad->choreographies[getChoreo(ckeyindex)].get(), kenv);
					IGObjectNameInput("Key name", ckey, kenv);
					ImGui::DragFloatRange2("Duration min/max", &ckey->unk1, &ckey->unk2, 0.2f, 0.0f, 10000.0f);
					float percentage = ckey->unk3 * 100.0f;
					if (ImGui::DragFloat("Enemies needed at position to start timer", &percentage, 0.2f, 0.0f, 100.0f, "%.1f%%"))
						ckey->unk3 = percentage / 100.0f;

					unsigned int iflags = ckey->flags;
					if (const auto* jsChoreo = ui.g_encyclo.getClassJson(CKChoreoKey::FULL_ID)) {
						try {
							const auto& bitFlags = jsChoreo->at("properties").at("flags").at("bitFlags");
							if (PropFlagsEditor(iflags, bitFlags))
								ckey->flags = (uint16_t)iflags;
						}
						catch (const nlohmann::json::exception&) {}
					}

					if (ImGui::Button("Add spot")) {
						ckey->slots.emplace_back();
					}
					ImGui::SameLine();
					if (ImGui::Button("Randomize directions")) {
						for (auto& slot : ckey->slots) {
							float angle = (rand() & 255) * 3.1416f / 128.0f;
							slot.direction = Vector3(cos(angle), 0, sin(angle));
						}
					}
					ImGui::SameLine();
					ImGui::Text("%zu spots", ckey->slots.size());

					ImGui::BeginChild("ChoreoSlots", ImVec2(0, 0), true);
					for (auto& slot : ckey->slots) {
						ImGui::PushID(&slot);
						ImGui::DragFloat3("Position", &slot.position.x, 0.1f);
						ImGui::DragFloat3("Direction", &slot.direction.x, 0.1f);
						ImGui::InputScalar("Enemy pool", ImGuiDataType_S16, &slot.enemyGroup);
						ImGui::PopID();
						ImGui::Separator();
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Pools")) {
				static size_t currentPoolInput = 0;
				if (ImGui::Button("Add")) {
					ImGui::OpenPopup("AddPool");
					//CKGrpSquadEnemy::PoolEntry pe;
				}
				if (ImGui::BeginPopup("AddPool")) {
					CKGrpEnemy* grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>();
					for (CKGroup* grp = grpEnemy->childGroup.get(); grp; grp = grp->nextGroup.get()) {
						if (CKGrpPoolSquad* pool = grp->dyncast<CKGrpPoolSquad>()) {
							CKHook* enemyHook = pool->childHook.get();
							if (enemyHook) {
								ImGui::PushID(pool);
								if (ImGui::Selectable("##AddPoolEntry")) {
									// find corresponding component class
									using namespace GameX1;
									static constexpr std::pair<int, int> hookCpntMap[] = {
										{CKHkBasicEnemy::FULL_ID, CKBasicEnemyCpnt::FULL_ID},
										{CKHkBasicEnemyLeader::FULL_ID, CKBasicEnemyLeaderCpnt::FULL_ID},
										{CKHkJumpingRoman::FULL_ID, CKJumpingRomanCpnt::FULL_ID},
										{CKHkRomanArcher::FULL_ID, CKRomanArcherCpnt::FULL_ID},
										{CKHkRocketRoman::FULL_ID, CKRocketRomanCpnt::FULL_ID},
										{CKHkJetPackRoman::FULL_ID, CKJetPackRomanCpnt::FULL_ID},
										{CKHkMobileTower::FULL_ID, CKMobileTowerCpnt::FULL_ID},
										{CKHkTriangularTurtle::FULL_ID, CKTriangularTurtleCpnt::FULL_ID},
										{CKHkSquareTurtle::FULL_ID, CKSquareTurtleCpnt::FULL_ID},
										{CKHkDonutTurtle::FULL_ID, CKDonutTurtleCpnt::FULL_ID},
										{CKHkPyramidalTurtle::FULL_ID, CKPyramidalTurtleCpnt::FULL_ID}
									};
									int ehookClassFid = (int)enemyHook->getClassFullID();
									auto it = std::find_if(std::begin(hookCpntMap), std::end(hookCpntMap), [ehookClassFid](auto& p) {return p.first == ehookClassFid; });
									assert(it != std::end(hookCpntMap));
									int cpntClassFid = it->second;

									auto& pe = squad->pools.emplace_back();
									pe.pool = pool;
									pe.cpnt = kenv.createObject((uint32_t)cpntClassFid, -1)->cast<CKEnemyCpnt>();
									pe.u1 = 1;
									pe.numEnemies = 1;
									pe.u2 = 1;
									kenv.levelObjects.getClassType(cpntClassFid).instantiation = KInstantiation::LevelUnique;
								}
								ImGui::SameLine();
								ImGui::Text("%s (%s)", kenv.getObjectName(pool), enemyHook->getClassName());
								ImGui::PopID();
							}
							else {
								ImGui::PushID(pool);
								ImGui::Selectable("##AddPoolEntry");
								ImGui::SameLine();
								ImGui::Text("%s (empty)", kenv.getObjectName(pool));
								ImGui::PopID();
							}
						}
					}
					ImGui::EndPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate")) {
					if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
						CKGrpSquadEnemy::PoolEntry duppe = squad->pools[currentPoolInput];
						duppe.cpnt = kenv.cloneObject(duppe.cpnt.get());
						squad->pools.push_back(duppe);
					}
				}
				ImGui::SameLine();
				if (ImGui::ArrowButton("PoolDown", ImGuiDir_Down)) {
					if (currentPoolInput >= 0 && currentPoolInput + 1 < squad->pools.size()) {
						std::swap(squad->pools[currentPoolInput], squad->pools[currentPoolInput + 1]);
						currentPoolInput += 1;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move pool down");
				ImGui::SameLine();
				if (ImGui::ArrowButton("PoolUp", ImGuiDir_Up)) {
					if (currentPoolInput >= 1 && currentPoolInput < squad->pools.size()) {
						std::swap(squad->pools[currentPoolInput - 1], squad->pools[currentPoolInput]);
						currentPoolInput -= 1;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move pool up");
				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
						auto& cpntref = squad->pools[currentPoolInput].cpnt;
						CKEnemyCpnt* cpnt = cpntref.get();
						cpntref.reset();
						kenv.removeObject(cpnt);
						squad->pools.erase(squad->pools.begin() + currentPoolInput);
					}
				}
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::BeginListBox("##PoolList")) {
					for (int i = 0; i < (int)squad->pools.size(); i++) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
							currentPoolInput = i;
						ImGui::SameLine();
						auto& pe = squad->pools[i];
						ImGui::Text("%s %u %u", pe.cpnt->getClassName(), pe.numEnemies, pe.u1);
						ImGui::PopID();
					}
					ImGui::EndListBox();
				}
				if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
					auto& pe = squad->pools[currentPoolInput];
					ImGui::BeginChild("SquadPools");
					ImGui::BulletText("%s %u %u %u", pe.cpnt->getClassName(), pe.u1, pe.u2, pe.u3.get() ? 1 : 0);
					IGObjectSelectorRef(ui, "Pool", pe.pool);
					ImGui::InputScalar("Enemy Count", ImGuiDataType_U16, &pe.numEnemies);
					ImGui::InputScalar("U1", ImGuiDataType_U8, &pe.u1);
					ImGui::InputScalar("U2", ImGuiDataType_U8, &pe.u2);
					if (pe.cpnt->isSubclassOf<CKEnemyCpnt>()) {
						CKEnemyCpnt* cpnt = pe.cpnt->cast<CKEnemyCpnt>();
						if (ImGui::Button("Import")) {
							auto path = OpenDialogBox(ui.g_window, "Enemy Component file\0*.XEC-ENM-CPNT\0", "xec-enm-cpnt");
							if (!path.empty()) {
								KEnvironment kfab;
								CKObject* obj = KFab::loadKFab(kfab, path);
								CKEnemyCpnt* impCpnt = obj->dyncast<CKEnemyCpnt>();
								if (!impCpnt) {
									MsgBox(ui.g_window, "This is not an enemy component!", 16);
								}
								else if (impCpnt->getClassFullID() != cpnt->getClassFullID()) {
									std::string msg = std::string("The components types don't match!\nThe selected pool's component is:\n ") + cpnt->getClassName() + "\nbut the imported file's component is:\n " + impCpnt->getClassName();
									MsgBox(ui.g_window, msg.c_str(), 16);
								}
								else {
									kenv.factories.at(impCpnt->getClassFullID()).copy(impCpnt, cpnt);
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Export")) {
							auto path = SaveDialogBox(ui.g_window, "Enemy Component file\0*.XEC-ENM-CPNT\0", "xec-enm-cpnt");
							if (!path.empty()) {
								int fid = (int)cpnt->getClassFullID();
								KEnvironment kfab = KFab::makeSimilarKEnv(kenv);
								CKObject* clone = kfab.createObject((int)fid, -1);
								kfab.factories.at(fid).copy(cpnt, clone);
								if (GameX1::CKRocketRomanCpnt* rockman = clone->dyncast<GameX1::CKRocketRomanCpnt>())
									rockman->rrUnk9 = nullptr;
								KFab::saveKFab(kfab, clone, path);
							}
						}
						ImGui::SameLine(0.0f, 16.0f);
						static KWeakRef<CKEnemyCpnt> cpntToCopy;
						if (ImGui::Button("Copy")) {
							cpntToCopy = cpnt;
						}
						ImGui::SameLine();
						if (ImGui::Button("Paste") && cpntToCopy) {
							if (cpntToCopy->getClassFullID() != cpnt->getClassFullID()) {
								std::string msg = std::string("The components types don't match!\nThe selected pool's component is:\n ") + cpnt->getClassName() + "\nbut the copied component is:\n " + cpntToCopy->getClassName();
								MsgBox(ui.g_window, msg.c_str(), 16);
							}
							else {
								kenv.factories.at(cpnt->getClassFullID()).copy(cpntToCopy.get(), cpnt);
							}
						}
						IGComponentEditor(ui, cpnt);
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::Columns();
}

void EditorUI::IGSquadEditorXXL2Plus(EditorInterface& ui)
{
	using namespace GameX2;
	auto& kenv = ui.kenv;

	auto* grpEnemy = ui.getX2PlusEnemyGroup();
	if (!grpEnemy) return;

	ImGui::Columns(2);
	ImGui::BeginChild("SquadList");
	auto enumSquad = [&ui, &kenv](CKObject* osquad, int si, bool jetpack) {
		CKGrpSquadX2* squad = osquad->cast<CKGrpSquadX2>();
		int numEnemies = 0;
		for (auto& pool : squad->fightData.pools) { // only works with XXL2
			numEnemies += pool.numEnemies;
		}
		ImGui::PushID(squad);
		if (ImGui::SmallButton("View")) {
			ui.camera.position = squad->phases[0].mat.getTranslationVector() - ui.camera.direction * 15.0f;
		}
		ImGui::SameLine();
		if (ImGui::Selectable("##SquadItem", ui.selectedX2Squad == squad)) {
			ui.selectedX2Squad = squad;
			ui.viewFightZoneInsteadOfSquad = false;
		}
		ImGui::SameLine();
		ImGui::Text("[%i] (%i) %s", si, numEnemies, kenv.getObjectName(squad));
		ImGui::PopID();
		};
	auto createNewSquadPhase = [&](CKGrpSquadX2* squad) {
		auto& newPhase = squad->phases.emplace_back();
		newPhase.choreography = kenv.createAndInitObject<CKChoreography>();
		newPhase.mat.setTranslation(ui.cursorPosition);

		newPhase.choreography->keys.push_back(kenv.createAndInitObject<CKChoreoKey>());
		newPhase.choreography->numKeys = 1;

		ui.showingChoreography = (int)squad->phases.size() - 1;
		ui.showingChoreoKey = 0;
		};
	auto enumFightZone = [&](CKGrpFightZone* zone) {
		bool open = ImGui::TreeNodeEx(zone, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick, "Zone %s", kenv.getObjectName(zone));
		if (ImGui::IsItemActivated()) {
			ui.selectedX2FightZone = zone;
			ui.viewFightZoneInsteadOfSquad = true;
		}
		if (open) {
			int si = 0;
			if (ImGui::SmallButton("New squad")) {
				auto* newSquad = kenv.createAndInitObject<CKGrpSquadX2>();
				newSquad->x2UnkA = 5;
				newSquad->bundle = kenv.createAndInitObject<CKBundle>();
				newSquad->bundle->x2Group = newSquad;
				newSquad->bundle->priority = 0x80000013;
				auto* srvLife = kenv.levelObjects.getFirst<CKServiceLife>();
				srvLife->addBundle(newSquad->bundle.get());
				zone->addGroup(newSquad);
				createNewSquadPhase(newSquad);
			}
			for (CKGroup* osquad = zone->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
				enumSquad(osquad, si++, false);
			}
			ImGui::TreePop();
		}
		};
	auto enumSectorRoot = [&](CKFightZoneSectorGrpRoot* fightZoneRoot) {
		bool open = ImGui::TreeNodeEx(fightZoneRoot,
			ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen,
			"Root %s", kenv.getObjectName(fightZoneRoot));
		if (open) {
			if (ImGui::SmallButton("New fight zone")) {
				auto* newZone = kenv.createAndInitObject<CKGrpFightZone>();
				newZone->bundle = kenv.createAndInitObject<CKBundle>();
				newZone->bundle->x2Group = newZone;
				fightZoneRoot->addGroup(newZone);
				auto* srvLife = kenv.levelObjects.getFirst<CKServiceLife>();
				srvLife->addBundle(newZone->bundle.get());
			}
			for (CKGroup* ozone = fightZoneRoot->childGroup.get(); ozone; ozone = ozone->nextGroup.get()) {
				enumFightZone(ozone->cast<CKGrpFightZone>());
			}
			ImGui::TreePop();
		}
		};
	for (const auto& fightZoneGroup : grpEnemy->fightZoneGroups) {
		enumSectorRoot(fightZoneGroup.get());
	}
	ImGui::EndChild();
	ImGui::NextColumn();

	auto poolEditor = [&](X2FightData& fightData) {
		static size_t currentPoolInput = 0;
		if (ImGui::Button("Add")) {
			ImGui::OpenPopup("AddPool");
		}
		if (ImGui::BeginPopup("AddPool")) {
			for (CKGroup* poolBase = grpEnemy->poolGroup->childGroup.get(); poolBase; poolBase = poolBase->nextGroup.get()) {
				ImGui::PushID(poolBase);
				auto* pool = poolBase->cast<CKGrpPoolSquad>();
				if (ImGui::Selectable(kenv.getObjectName(pool))) {
					auto& memberType = fightData.pools.emplace_back();
					memberType.pool = pool;
				}
				ImGui::PopID();
			}
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Duplicate")) {
			if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
				const X2FightData::PoolEntry& duppe = fightData.pools[currentPoolInput];
				fightData.pools.push_back(duppe);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
				fightData.pools.erase(fightData.pools.begin() + currentPoolInput);
			}
		}
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginListBox("##PoolList")) {
			for (int i = 0; i < (int)fightData.pools.size(); i++) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
					currentPoolInput = i;
				ImGui::SameLine();
				auto& pe = fightData.pools[i];
				ImGui::Text("%s Cpnt%u %u", kenv.getObjectName(pe.pool.get()), pe.componentIndex, pe.numEnemies);
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
			auto& pe = fightData.pools[currentPoolInput];
			ImGui::BeginChild("SquadPools");
			//ImGui::BulletText("%s %u %u", "TODO", pe.u1, pe.u2);
			IGObjectSelectorRef(ui, "Pool", pe.pool);
			ImGui::InputScalar("Component Index", ImGuiDataType_U8, &pe.componentIndex);
			ImGui::InputScalar("Enemy Total Count", ImGuiDataType_U16, &pe.numEnemies);
			ImGui::InputScalar("Enemy Initial Count", ImGuiDataType_U8, &pe.numInitiallySpawned);
			ImGui::EndChild();
		}
		};

	if (!ui.viewFightZoneInsteadOfSquad && ui.selectedX2Squad) {
		CKGrpSquadX2* squad = ui.selectedX2Squad.get();
		//if (ImGui::Button("Duplicate")) {
		//	// TODO
		//}
		if (ImGui::BeginTabBar("SquadInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::BeginChild("SquadReflection");
				ImGuiMemberListener ml(kenv, ui);
				ml.setPropertyInfoList(ui.g_encyclo, squad);
				squad->reflectMembers2(ml, &kenv);
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Phases")) {
				ImGui::BeginChild("SquadChoreos");

				ImGui::Text("Num phases: %i", squad->phases.size());
				ImGui::InputInt("Squad Phase", &ui.showingChoreography);
				if (ImGui::Button("Add phase")) {
					createNewSquadPhase(squad);
				}
				ImGui::SameLine();
				ImGui::BeginDisabled(!(ui.showingChoreography >= 0 && ui.showingChoreography < (int)squad->phases.size()));
				if (ImGui::Button("Remove phase")) {
					CKChoreography* choreo = squad->phases[ui.showingChoreography].choreography.get();
					squad->phases[ui.showingChoreography].choreography = nullptr;
					kenv.removeObject(choreo);
					squad->phases.erase(squad->phases.begin() + ui.showingChoreography);
					ui.showingChoreography = std::min(ui.showingChoreography, (int)squad->phases.size() - 1);
				}
				ImGui::EndDisabled();

				if (ui.showingChoreography >= 0 && ui.showingChoreography < (int)squad->phases.size()) {
					auto& phase = squad->phases[ui.showingChoreography];

					ImGui::Separator();
					ImGui::DragFloat3("Position", &phase.mat._41, 0.1f);
					ImGui::InputFloat("Unkfloat", &phase.choreography->unkfloat);
					ImGui::InputScalar("Unkflags", ImGuiDataType_U8, &phase.choreography->unk2);

					ImGui::Separator();
					ImGui::InputInt("ChoreoKey", &ui.showingChoreoKey);
					if (ImGui::Button("Add key")) {
						phase.choreography->keys.push_back(kenv.createAndInitObject<CKChoreoKey>());
						phase.choreography->numKeys = (uint8_t)phase.choreography->keys.size();
						ui.showingChoreoKey = (int)phase.choreography->keys.size() - 1;
					}
					ImGui::SameLine();
					ImGui::BeginDisabled(!(ui.showingChoreoKey >= 0 && ui.showingChoreoKey < (int)phase.choreography->keys.size()));
					if (ImGui::Button("Remove key")) {
						CKChoreoKey* key = phase.choreography->keys[ui.showingChoreoKey].get();
						phase.choreography->keys[ui.showingChoreoKey] = nullptr;
						kenv.removeObject(key);
						phase.choreography->keys.erase(phase.choreography->keys.begin() + ui.showingChoreoKey);
						phase.choreography->numKeys = (uint8_t)phase.choreography->keys.size();
						ui.showingChoreoKey = std::min(ui.showingChoreoKey, (int)phase.choreography->keys.size() - 1);
					}
					ImGui::EndDisabled();

					const int& ckeyindex = ui.showingChoreoKey;
					if (ckeyindex >= 0 && ckeyindex < (int)phase.choreography->keys.size()) {
						CKChoreoKey* ckey = phase.choreography->keys[ckeyindex].get();
						ImGui::Separator();
						ImGui::DragFloat("Duration", &ckey->x2unk1);
						ImGui::InputScalar("Flags", ImGuiDataType_U16, &ckey->flags, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);

						if (ImGui::Button("Add spot")) {
							auto& slot = ckey->slots.emplace_back();
							slot.enemyGroup = 0;
						}
						ImGui::SameLine();
						if (ImGui::Button("Randomize orientations")) {
							for (auto& slot : ckey->slots) {
								float angle = (rand() & 255) * 3.1416f / 128.0f;
								slot.direction = Vector3(cos(angle), 0, sin(angle));
							}
						}
						ImGui::BeginChild("ChoreoSlots", ImVec2(0, 0), true);
						for (auto& slot : ckey->slots) {
							ImGui::PushID(&slot);
							ImGui::DragFloat3("Position", &slot.position.x, 0.1f);
							ImGui::DragFloat3("Direction", &slot.direction.x, 0.1f);
							ImGui::InputScalar("Enemy pool", ImGuiDataType_S16, &slot.enemyGroup);
							ImGui::PopID();
							ImGui::Separator();
						}
						ImGui::EndChild();
					}
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if ((kenv.version == kenv.KVERSION_XXL2) && ImGui::BeginTabItem("Pools")) {
				poolEditor(squad->fightData);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	else if (ui.viewFightZoneInsteadOfSquad && ui.selectedX2FightZone) {
		CKGrpFightZone* zone = ui.selectedX2FightZone.get();
		if (ImGui::BeginTabBar("FightZoneInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::BeginChild("FightZoneReflection");
				ImGuiMemberListener ml(kenv, ui);
				ml.setPropertyInfoList(ui.g_encyclo, zone);
				zone->reflectMembers2(ml, &kenv);
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (kenv.version >= kenv.KVERSION_ARTHUR && ImGui::BeginTabItem("Pools")) {
				poolEditor(zone->fightData);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::Columns();
}
