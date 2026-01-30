#include "IGCinematicEditor.h"
#include "EditorInterface.h"
#include "GuiUtils.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKCinematicNode.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"

#include "imgui/imgui.h"
#include "imgui/imnodes.h"
#include <fmt/format.h>

namespace {
	void InitImNodes() {
		static bool ImNodesInitialized = false;
		if (!ImNodesInitialized) {
			ImNodes::CreateContext();
			ImNodes::GetIO().AltMouseButton = ImGuiMouseButton_Right;
			ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = &ImGui::GetIO().KeyAlt;
			ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
			ImNodesInitialized = true;
		}
	}
}

void EditorUI::IGCinematicEditor(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;

	CKSrvCinematic* srvCine = kenv.levelObjects.getFirst<CKSrvCinematic>();
	static int selectedCinematicSceneIndex = -1;
	static KWeakRef<CKCinematicNode> selectedCineNode;
	auto getSceneName = [&kenv, srvCine](int index) -> std::string {
		if (index >= 0 && index < (int)srvCine->cineScenes.size())
			return std::to_string(index) + ": " + kenv.getObjectName(srvCine->cineScenes[index].get());
		return "?";
		};
	if (ImGui::BeginCombo("Cinematic Scene", getSceneName(selectedCinematicSceneIndex).c_str())) {
		for (int i = 0; i < (int)srvCine->cineScenes.size(); ++i)
			if (ImGui::Selectable(getSceneName(i).c_str()))
				selectedCinematicSceneIndex = i;
		ImGui::EndCombo();
	}
	if (ImGui::Button("New scene")) {
		CKCinematicScene* newScene = kenv.createAndInitObject<CKCinematicScene>();
		srvCine->cineScenes.emplace_back(newScene);
		selectedCinematicSceneIndex = (int)srvCine->cineScenes.size() - 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Corrupt")) {
		for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
			for (CKObject* obj : cncls.objects) {
				CKCinematicNode* knode = obj->cast<CKCinematicNode>();
				if (knode->isSubclassOf<CKCinematicDoor>())
					knode->cast<CKCinematicDoor>()->cnFlags &= 0xFA;
				else
					knode->cast<CKCinematicBloc>()->cnFlags &= 0xFFFA;
			}
		}
	}
	if (selectedCinematicSceneIndex >= 0 && selectedCinematicSceneIndex < (int)srvCine->cineScenes.size()) {
		CKCinematicScene* scene = srvCine->cineScenes[selectedCinematicSceneIndex].get();

		ImGui::SameLine();
		if (ImGui::Button("Export TGF")) {
			auto filename = SaveDialogBox(ui.g_window, "Trivial Graph Format (*.tgf)\0*.TGF\0", "tgf");
			if (!filename.empty()) {
				std::map<CKCinematicNode*, int> gfNodes;
				std::map<std::array<CKCinematicNode*, 2>, std::string> gfEdges;
				int nextNodeId = 1;

				// Find all nodes from the scene
				for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject* obj : cncls.objects) {
						CKCinematicNode* knode = obj->cast<CKCinematicNode>();
						if (knode->cnScene.get() == scene)
							gfNodes.insert({ knode, nextNodeId++ });
					}
				}

				// Constructing edges
				std::function<void(CKCinematicNode* node)> visit;
				visit = [&scene, &gfNodes, &gfEdges, &visit](CKCinematicNode* node) {
					uint16_t start = (node->cnStartOutEdge != 0xFFFF) ? node->cnStartOutEdge : node->cnFinishOutEdge;
					for (int i = 0; i < node->cnNumOutEdges; i++) {
						CKCinematicNode* subnode = scene->cineNodes[start + i].get();
						gfEdges[{ node, subnode }].append(std::to_string(i)).append(",");
					}
					if (node->cnFinishOutEdge != 0xFFFF) {
						for (int i = node->cnFinishOutEdge; i < start + node->cnNumOutEdges; ++i) {
							CKCinematicNode* subnode = scene->cineNodes[i].get();
							gfEdges[{ node, subnode }].append("cond,");
						}
					}
					if (CKGroupBlocCinematicBloc* grpbloc = node->dyncast<CKGroupBlocCinematicBloc>()) {
						gfEdges[{node, grpbloc->gbFirstNode.get()}].append("grpHead,");
					}
					};
				//visit(scene->startDoor.get());
				for (auto& gnode : gfNodes) {
					visit(gnode.first);
				}

				//// Node labeling
				//for (auto &edge : gfEdges) {
				//	for (CKCinematicNode *node : edge.first) {
				//		if (gfNodes[node] == 0)
				//			gfNodes[node] = nextNodeId++;
				//	}
				//}

				FILE* tgf;
				fsfopen_s(&tgf, filename, "wt");
				for (auto& gnode : gfNodes) {
					fprintf(tgf, "%i %s (%p)\n", gnode.second, gnode.first->getClassName(), gnode.first);
				}
				fprintf(tgf, "#\n");
				for (auto& edge : gfEdges) {
					fprintf(tgf, "%i %i %s\n", gfNodes[edge.first[0]], gfNodes[edge.first[1]], edge.second.c_str());
				}
				fclose(tgf);
			}
		}

		ImGui::Columns(2);

		if (ImGui::BeginTabBar("GraphBar")) {
			if (ImGui::BeginTabItem("Edge list")) {
				ImGui::BeginChild("CineNodes");

				bool b = ImGui::TreeNodeEx("Start door", ImGuiTreeNodeFlags_Leaf | ((selectedCineNode == scene->startDoor.get()) ? ImGuiTreeNodeFlags_Selected : 0));
				if (ImGui::IsItemClicked()) {
					selectedCineNode = scene->startDoor.get();
				}
				if (b) ImGui::TreePop();

				struct CineNodeEnumerator {
					static void enumNode(CKCinematicNode* node, int i) {
						bool isGroup = node->isSubclassOf<CKGroupBlocCinematicBloc>();
						ImGuiTreeNodeFlags tflags = 0;
						if (selectedCineNode == node) tflags |= ImGuiTreeNodeFlags_Selected;
						if (!isGroup) tflags |= ImGuiTreeNodeFlags_Leaf;
						bool b = ImGui::TreeNodeEx(node, tflags, "%i: %s", i++, node->getClassName());
						if (ImGui::IsItemClicked()) {
							selectedCineNode = node;
						}
						if (b) {
							if (isGroup) {
								int i = 0;
								for (auto& sub : node->cast<CKGroupBlocCinematicBloc>()->gbSubnodes)
									enumNode(sub.get(), i++);
							}
							ImGui::TreePop();
						}
					}
				};

				int i = 0;
				for (auto& node : scene->cineNodes) {
					CineNodeEnumerator::enumNode(node.get(), i++);
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Block graph", nullptr, ImGuiTabItemFlags_Leading)) {
				static ImNodesEditorContext* cineNodesContext = nullptr;
				if (!cineNodesContext) {
					InitImNodes();
					cineNodesContext = ImNodes::EditorContextCreate();
				}
				ImNodes::EditorContextSet(cineNodesContext);

				static int ImNodesCurrentScene = -1;
				static std::unordered_map<CKCinematicNode*, int> blockIdMap;
				static std::unordered_map<int, CKCinematicNode*> idBlockMap;
				static int nextId = 0;

				if (ImNodesCurrentScene != selectedCinematicSceneIndex) {
					ImNodes::ClearNodeSelection();
					ImNodes::ClearLinkSelection();

					// Compute the layout
					std::set<CKCinematicNode*> reachedSet;
					std::map<CKGroupBlocCinematicBloc*, std::vector<std::vector<CKCinematicNode*>>> layout;
					auto visit = [scene, &reachedSet, &layout](CKCinematicNode* node, int depth, const auto& rec) -> void {
						if (reachedSet.count(node))
							return;
						reachedSet.insert(node);

						auto& layoutGroup = layout[node->cnGroupBloc.get()];
						if (depth >= (int)layoutGroup.size())
							layoutGroup.resize(depth + 1);
						layoutGroup[depth].push_back(node);

						uint16_t start = (node->cnStartOutEdge != 0xFFFF) ? node->cnStartOutEdge : node->cnFinishOutEdge;
						uint16_t end = (node->cnFinishOutEdge != 0xFFFF) ? node->cnFinishOutEdge : (start + node->cnNumOutEdges);
						for (int i = start; i < end; i++) {
							CKCinematicNode* subnode = scene->cineNodes[i].get();
							rec(subnode, depth + 1, rec);
						}
						};

					// Visit from the Start Door, as well as Group Starting Node
					visit(scene->startDoor.get(), 0, visit);
					for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
						for (CKObject* obj : cncls.objects) {
							if (CKGroupBlocCinematicBloc* group = obj->dyncast<CKGroupBlocCinematicBloc>()) {
								if (group->cnScene.get() == scene) {
									visit(group->gbFirstNode.get(), 0, visit);
								}
							}
						}
					}

					for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
						for (CKObject* obj : cncls.objects) {
							CKCinematicNode* knode = obj->cast<CKCinematicNode>();
							if (knode->cnScene.get() == scene) {
								int id;
								if (auto it = blockIdMap.find(knode); it != blockIdMap.end()) {
									id = it->second;
								}
								else {
									blockIdMap.insert_or_assign(knode, nextId);
									idBlockMap.insert_or_assign(nextId, knode);
									id = nextId;
									++nextId;
								}
							}
						}
					}
					static constexpr float nodeWidth = 240.0f;
					static constexpr float nodeHeight = 150.0f;
					std::vector<int> groupPosY(layout.size(), 0);
					size_t lg = 0;
					for (auto& [group, layoutGroup] : layout) {
						int rows = 0;
						for (int x = 0; x < (int)layoutGroup.size(); ++x) {
							rows = std::max(rows, (int)layoutGroup[x].size());
						}

						groupPosY[lg] = rows;
						if (lg > 0)
							groupPosY[lg] += groupPosY[lg - 1];
						lg += 1;
					}
					lg = 0;
					for (auto& [group, layoutGroup] : layout) {
						int startRow = (lg > 0) ? groupPosY[lg - 1] : 0;
						lg += 1;
						for (int x = 0; x < (int)layoutGroup.size(); ++x) {
							for (int y = 0; y < (int)layoutGroup[x].size(); ++y) {
								ImNodes::SetNodeGridSpacePos(blockIdMap.at(layoutGroup[x][y]), ImVec2(x * nodeWidth, (startRow + y) * nodeHeight));
							}
						}
					}
					ImNodesCurrentScene = selectedCinematicSceneIndex;
				}
				if (ImGui::Button("Add node")) {
					ImGui::OpenPopup("AddCineNode");
				}
				if (ImGui::BeginPopup("AddCineNode")) {
					int toadd = -1;
					auto door = [&toadd](int id, const char* name) {
						if (ImGui::Selectable(name))
							toadd = id;
						};
					door(CKLogicalAnd::FULL_ID, "CKLogicalAnd");
					door(CKLogicalOr::FULL_ID, "CKLogicalOr");
					if (kenv.version == KEnvironment::KVERSION_XXL1)
						door(CKRandLogicalDoor::FULL_ID, "CKRandLogicalDoor");
					if (kenv.version >= KEnvironment::KVERSION_XXL2 || kenv.isRemaster)
						door(CKEndDoor::FULL_ID, "CKEndDoor");

					ImGui::Separator();

					auto block = [&toadd](int id, const char* name) {
						if (ImGui::Selectable(name))
							toadd = id;
						};
					block(CKPlayAnimCinematicBloc::FULL_ID, "CKPlayAnimCinematicBloc");
					block(CKPathFindingCinematicBloc::FULL_ID, "CKPathFindingCinematicBloc");
					block(CKFlaggedPathCinematicBloc::FULL_ID, "CKFlaggedPathCinematicBloc");
					block(CKGroupBlocCinematicBloc::FULL_ID, "CKGroupBlocCinematicBloc");
					block(CKAttachObjectsCinematicBloc::FULL_ID, "CKAttachObjectsCinematicBloc");
					block(CKParticleCinematicBloc::FULL_ID, "CKParticleCinematicBloc");
					block(CKSekensorCinematicBloc::FULL_ID, "CKSekensorCinematicBloc");
					block(CKDisplayPictureCinematicBloc::FULL_ID, "CKDisplayPictureCinematicBloc");
					block(CKManageCameraCinematicBloc::FULL_ID, "CKManageCameraCinematicBloc");
					block(CKStartEventCinematicBloc::FULL_ID, "CKStartEventCinematicBloc");
					block(CKLightningCinematicBloc::FULL_ID, "CKLightningCinematicBloc");
					block(CKPlaySoundCinematicBloc::FULL_ID, "CKPlaySoundCinematicBloc");
					ImGui::Separator();
					if (kenv.version >= KEnvironment::KVERSION_XXL2 || kenv.isRemaster) {
						block(CKPauseCinematicBloc::FULL_ID, "CKPauseCinematicBloc");
						block(CKTeleportCinematicBloc::FULL_ID, "CKTeleportCinematicBloc");
					}
					if (kenv.version == KEnvironment::KVERSION_XXL1) {
						block(CKStreamCinematicBloc::FULL_ID, "CKStreamCinematicBloc");
						block(CKStreamAloneCinematicBloc::FULL_ID, "CKStreamAloneCinematicBloc");
						block(CKStreamGroupBlocCinematicBloc::FULL_ID, "CKStreamGroupBlocCinematicBloc");
						block(CKManageEventCinematicBloc::FULL_ID, "CKManageEventCinematicBloc");
						block(CKManagerEventStopCinematicBloc::FULL_ID, "CKManagerEventStopCinematicBloc");
						block(CKSkyCinematicBloc::FULL_ID, "CKSkyCinematicBloc");
					}
					else if (kenv.version >= KEnvironment::KVERSION_XXL2) {
						block(CKPlayVideoCinematicBloc::FULL_ID, "CKPlayVideoCinematicBloc");
						block(CKFlashUICinematicBloc::FULL_ID, "CKFlashUICinematicBloc");
						if (kenv.version >= KEnvironment::KVERSION_OLYMPIC) {
							block(CKLockUnlockCinematicBloc::FULL_ID, "CKLockUnlockCinematicBloc");
						}
					}

					if (toadd != -1) {
						kenv.levelObjects.getClassType(toadd).instantiation = KInstantiation::LevelUnique;
						CKCinematicNode* added = kenv.createObject((uint32_t)toadd, -1)->cast<CKCinematicNode>();
						added->init(&kenv);
						added->cnScene = scene;
						if (kenv.version >= KEnvironment::KVERSION_XXL2) {
							added->cnFlags = 0x12;
						}
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();
				ImGui::BeginDisabled(ImNodes::NumSelectedNodes() <= 0);
				if (ImGui::Button("Remove nodes")) {
					int numSelNodes = ImNodes::NumSelectedNodes();
					if (numSelNodes > 0) {
						std::vector<int> selNodes;
						selNodes.resize(numSelNodes);
						ImNodes::GetSelectedNodes(selNodes.data());
						bool someImpossible = false;
						for (int id : selNodes) {
							CKCinematicNode* node = idBlockMap.at(id);
							int refCount = node->getRefCount();
							if (refCount == 0 && node->cnNumOutEdges == 0) {
								kenv.removeObject(node);
								idBlockMap.erase(id);
								blockIdMap.erase(node);
							}
							else
								someImpossible = true;
						}
						ImNodes::ClearNodeSelection();
						if (someImpossible)
							MsgBox_Ok(ui.g_window, "Some nodes could not be removed as they are still referenced or have edges.", MsgBoxIcon::Warning);
					}
				}
				ImGui::EndDisabled();

				ImNodes::BeginNodeEditor();
				// Nodes
				for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject* obj : cncls.objects) {
						CKCinematicNode* knode = obj->cast<CKCinematicNode>();
						if (knode->cnScene.get() == scene) {
							int id;
							if (auto it = blockIdMap.find(knode); it != blockIdMap.end()) {
								id = it->second;
							}
							else {
								blockIdMap.insert_or_assign(knode, nextId);
								idBlockMap.insert_or_assign(nextId, knode);
								id = nextId;
								++nextId;
							}
							ImNodes::BeginNode(id);
							ImNodes::BeginNodeTitleBar();
							ImGui::TextUnformatted(knode->getClassName());
							ImGui::SetNextItemWidth(128.0f);
							IGObjectNameInput("##Name", knode, kenv);
							ImNodes::EndNodeTitleBar();
							ImNodes::BeginInputAttribute(3 * id);
							ImGui::TextUnformatted("Start");
							ImNodes::EndInputAttribute();
							ImNodes::BeginInputAttribute(3 * id + 1);
							ImGui::TextUnformatted("Finish");
							ImNodes::EndInputAttribute();
							ImNodes::BeginOutputAttribute(3 * id + 2);
							ImGui::TextUnformatted("Done");
							ImNodes::EndOutputAttribute();
							ImNodes::EndNode();
						}
					}
				}
				// Edges
				for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject* obj : cncls.objects) {
						CKCinematicNode* knodeSrc = obj->cast<CKCinematicNode>();
						if (knodeSrc->cnScene.get() == scene) {
							int srcId = blockIdMap.at(knodeSrc);
							uint16_t start = (knodeSrc->cnStartOutEdge != 0xFFFF) ? knodeSrc->cnStartOutEdge : knodeSrc->cnFinishOutEdge;
							uint16_t end = start + knodeSrc->cnNumOutEdges;
							uint16_t trans = (knodeSrc->cnFinishOutEdge != 0xFFFF) ? knodeSrc->cnFinishOutEdge : end;
							for (int i = start; i < end; i++) {
								bool isFinish = i >= trans;
								int destId = blockIdMap.at(scene->cineNodes[i].get());
								ImNodes::PushColorStyle(ImNodesCol_Link, isFinish ? 0xFF0000FF : 0xFF00FF00);
								ImNodes::Link(i, 3 * srcId + 2, 3 * destId + (isFinish ? 1 : 0));
								ImNodes::PopColorStyle();
							}
						}
					}
				}
				ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
				ImNodes::EndNodeEditor();

				int numSelNodes = ImNodes::NumSelectedNodes();
				if (numSelNodes > 0) {
					std::vector<int> selNodes;
					selNodes.resize(numSelNodes);
					ImNodes::GetSelectedNodes(selNodes.data());
					selectedCineNode = idBlockMap.at(selNodes[0]);
				}
				int creaSrcNode, creaSrcAttrib, creaDestNode, creaDestAttrib;
				if (ImNodes::IsLinkCreated(&creaSrcNode, &creaSrcAttrib, &creaDestNode, &creaDestAttrib)) {
					scene->addEdge(idBlockMap.at(creaSrcNode), idBlockMap.at(creaDestNode), (creaDestAttrib % 3) != 0, &kenv);
				}
				int destroyedLink;
				if (ImNodes::IsLinkDestroyed(&destroyedLink)) {
					auto [src, dest, fin] = scene->getEdgeInfo(destroyedLink, &kenv);
					scene->removeEdge(src, dest, fin, &kenv);
				}

				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::NextColumn();
		if (ImGui::BeginTabBar("CineRight")) {
			if (ImGui::BeginTabItem("Node")) {
				ImGui::Text("(%i refs)", selectedCineNode->getRefCount());
				if (selectedCineNode) {
					ImGui::BeginChild("CineSelectedNode");
					uint32_t& flags = selectedCineNode->cnFlags;
					for (uint32_t i = 0; i < 16; ++i) {
						ImGui::CheckboxFlags(fmt::format("{:X}", i).c_str(), &flags, 1 << i);
						if ((i % 8) != 7)
							ImGui::SameLine();
					}
					struct CineBlocMemberListener : ImGuiMemberListener {
						using ImGuiMemberListener::ImGuiMemberListener;
						void message(const char* msg) {
							static const std::string_view target = "End of Cinematic Node Base";
							if (msg == target)
								ImGui::Separator();
						}
					};
					CineBlocMemberListener ml(kenv, ui);
					ml.setPropertyInfoList(ui.g_encyclo, selectedCineNode.get());
					selectedCineNode->virtualReflectMembers(ml, &kenv);

					if (CKDisplayPictureCinematicBloc* dp = selectedCineNode->dyncast<CKDisplayPictureCinematicBloc>()) {
						IGStringInput("Texture", dp->ckdpcbBillboard->texture);
					}

					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Scene")) {
				ImGui::BeginChild("CineSceneProperties");
				IGObjectNameInput("Name", scene, kenv);
				ImGuiMemberListener iml{ kenv, ui };
				iml.setPropertyInfoList(ui.g_encyclo, scene);
				MemberListener& ml = iml;
				ml.reflect(scene->csFlags, "csFlags");
				ml.reflect(scene->csUnk2, "csUnk2");
				ml.reflect(scene->csBarsColor, "csBarsColor");
				ml.reflect(scene->csUnk4, "csUnk4");
				ml.reflect(scene->csUnk5, "csUnk5");
				ml.reflect(scene->csUnk6, "csUnk6");
				ml.reflect(scene->csUnk7, "csUnk7");
				ml.reflect(scene->csUnk8, "csUnk8");
				ml.reflect(scene->csUnk9, "csUnk9");
				ml.reflect(scene->csUnkA, "csUnkA");
				ml.reflect(scene->onSceneEnded, "onSceneEnded", nullptr);
				ml.reflect(scene->groups, "groups");
				ml.reflect(scene->sndDict, "sndDict");
				ml.reflect(scene->csUnkF, "csUnkF");
				if (kenv.version >= KEnvironment::KVERSION_XXL2) {
					ml.reflect(scene->arcsUnk1a, "arcsUnk1a");
					ml.reflect(scene->ogOnSceneStart, "ogOnSceneStart", nullptr);
					ml.reflect(scene->spyroOnSceneSkipped, "spyroOnSceneSkipped", nullptr);
					ml.reflect(scene->x2CameraEndDuration, "x2CameraEndDuration");
					ml.reflect(scene->arthurOnlyByte, "arthurOnlyByte");
					ml.reflect(scene->spyroSkipScene, "spyroSkipScene");
				}
				else if (kenv.isRemaster) {
					ml.reflect(scene->otherUnkFromRomaster, "otherUnkFromRomaster");
				}
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::CollapsingHeader("Scene Data")) {
					for (auto& dataRef : scene->cineDatas) {
						CKCinematicSceneData* data = dataRef.get();
						ImGui::PushID(data);
						ml.reflect(data->hook, "hook");
						ml.reflect(data->animDict, "animDict");
						ml.reflect(data->csdUnkA, "csdUnkA");
						ml.reflect(data->csdUnkB, "csdUnkB");
						ImGui::PopID();
						if (&dataRef != &scene->cineDatas.back()) ImGui::Separator();
					}
					if (ImGui::Button("Add data")) {
						CKCinematicSceneData* data = kenv.createAndInitObject<CKCinematicSceneData>();
						data->animDict = kenv.createAndInitObject<CAnimationDictionary>();
						scene->cineDatas.emplace_back(data);
					}
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Columns();
	}
}
