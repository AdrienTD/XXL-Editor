#include "IGSceneNodeEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"

#include "GeoUtils.h"
#include "GuiUtils.h"
#include "rw.h"

#include <imgui/imgui.h>
#include <imgui/ImGuizmo.h>
#include <fmt/format.h>

namespace {
	using namespace EditorUI;

	void IGEnumNode(EditorInterface& ui, CKSceneNode* node, const char* description = nullptr, bool isAnimBranch = false)
	{
		if (!node)
			return;
		bool hassub = false;
		if (CSGBranch* branch = node->dyncast<CSGBranch>())
			hassub |= (bool)branch->child;
		if (CAnyAnimatedNode* anyanimnode = node->dyncast<CAnyAnimatedNode>())
			hassub |= (bool)anyanimnode->branchs;
		if (isAnimBranch) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
		bool open = ImGui::TreeNodeEx(node, (hassub ? 0 : ImGuiTreeNodeFlags_Leaf) | ((ui.selNode == node) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick,
			"%s %s", node->getClassName(), description ? description : ui.kenv.getObjectName(node));
		if (isAnimBranch) ImGui::PopStyleColor();
		if (ImGui::IsItemClicked()) {
			ui.selNode = node;
		}
		if (!node->isSubclassOf<CSGRootNode>() && !node->isSubclassOf<CSGSectorRoot>()) {
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				ImGui::SetDragDropPayload("SceneGraphNode", &node, sizeof(node));
				ImGui::Text("%s", node->getClassName());
				ImGui::EndDragDropSource();
			}
		}
		if (CSGBranch* branch = node->dyncast<CSGBranch>()) {
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneGraphNode")) {
					CKSceneNode* sub = *(CKSceneNode**)payload->Data;
					// don't allow target nodes parented to source node
					bool parentedToSource = false;
					for (CKSceneNode* pc = node->parent.get(); pc; pc = pc->parent.get())
						parentedToSource |= pc == sub;
					if (!parentedToSource) {
						sub->parent->cast<CSGBranch>()->removeChild(sub);
						branch->insertChild(sub);
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
		if (open) {
			if (hassub) {
				if (CSGBranch* branch = node->dyncast<CSGBranch>()) {
					IGEnumNode(ui, branch->child.get());
					if (CAnyAnimatedNode* anyanimnode = node->dyncast<CAnyAnimatedNode>())
						IGEnumNode(ui, anyanimnode->branchs.get(), nullptr, true);
				}
			}
			ImGui::TreePop();
		}
		IGEnumNode(ui, node->next.get(), nullptr, isAnimBranch);
	}
}

void EditorUI::IGSceneNodeEditor(EditorInterface& ui)
{
	if (ImGui::BeginTable("SceneGraphTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::BeginChild("SceneNodeTree");
		IGSceneGraph(ui);
		ImGui::EndChild();
		ImGui::TableNextColumn();
		ImGui::BeginChild("SceneNodeProperties");
		IGSceneNodeProperties(ui);
		ImGui::EndChild();
		ImGui::EndTable();
	}
}

void EditorUI::IGSceneGraph(EditorInterface& ui)
{
	auto& kenv = ui.kenv;

	if (ImGui::Button("Add Node")) {
		CKSceneNode* par = ui.selNode.get();
		while (par && !par->isSubclassOf<CSGSectorRoot>())
			par = par->parent.get();
		if (par) {
			int sector = kenv.getObjectSector(par);
			CNode* node = kenv.createAndInitObject<CNode>(sector);
			par->cast<CSGSectorRoot>()->insertChild(node);
			node->transform.setTranslation(ui.cursorPosition);
			kenv.setObjectName(node, "New_Node");
		}
		else {
			GuiUtils::MsgBox(ui.g_window, "Select the sector root node (or any of its children) where you want to add the node.", 48);
		}
	}
	CSGSectorRoot* lvlroot = kenv.levelObjects.getObject<CSGSectorRoot>(0);
	IGEnumNode(ui, lvlroot, "Common sector");
	for (int i = 0; i < (int)kenv.numSectors; i++) {
		CSGSectorRoot* strroot = kenv.sectorObjects[i].getObject<CSGSectorRoot>(0);
		char buf[40];
		sprintf_s(buf, "Sector %i", i + 1);
		IGEnumNode(ui, strroot, buf);
	}
}

void EditorUI::IGSceneNodeProperties(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;
	
	CKSceneNode* selNode = ui.selNode.get();
	if (!selNode) {
		ImGui::Text("No node selected!");
		return;
	}

	ImGui::Text("%p %s : %s", selNode, selNode->getClassName(), kenv.getObjectName(selNode));
	IGObjectDragDropSource(ui, selNode);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
			if (payload->IsDataType("CKObject")) {
				CKObject* obj = *(CKObject**)payload->Data;
				if (obj->isSubclassOf<CKSceneNode>())
					if (const ImGuiPayload* acceptedPayload = ImGui::AcceptDragDropPayload("CKObject"))
						ui.selNode = *(CKSceneNode**)payload->Data;
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Separator();

	Matrix globalMat = selNode->getGlobalMatrix();
	if (ImGui::Button("Place camera there")) {
		Matrix& m = globalMat;
		ui.camera.position = Vector3(m._41, m._42, m._43) - ui.camera.direction * 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Find hook")) {
		ui.wndShowHooks = true;
		for (auto& hkclass : kenv.levelObjects.categories[CKHook::CATEGORY].type) {
			for (CKObject* obj : hkclass.objects) {
				CKHook* hook = obj->cast<CKHook>();
				if (hook->node.bound) {
					if (hook->node.get() == selNode) {
						ui.selectedHook = hook;
						ui.viewGroupInsteadOfHook = false;
					}
				}
			}
		}
	}

	IGObjectNameInput("Name", selNode, kenv);

	Vector3 locTranslation, locRotation, locScale;
	bool recompose = false;
	ImGuizmo::DecomposeMatrixToComponents(selNode->transform.v, &locTranslation.x, &locRotation.x, &locScale.x);
	ImGui::BeginDisabled();
	if (ImGui::DragFloat3("Global Position", &globalMat._41, 0.1f)) {
		//selNode->transform = globalMat * (selNode->transform.getInverse4x3() * globalMat).getInverse4x3();
	}
	ImGui::EndDisabled();
	ImGui::DragFloat3("Local Position", &selNode->transform._41, 0.1f);
	recompose |= ImGui::DragFloat3("Local Rotation (deg)", &locRotation.x);
	recompose |= ImGui::DragFloat3("Local Scale", &locScale.x, 0.1f);
	if (recompose) {
		ImGuizmo::RecomposeMatrixFromComponents(&locTranslation.x, &locRotation.x, &locScale.x, selNode->transform.v);
	}
	ImGui::InputScalar("Flags1", ImGuiDataType_U32, &selNode->unk1, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputScalar("Flags2", ImGuiDataType_U8, &selNode->unk2, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
	if (selNode->isSubclassOf<CNode>()) {
		CNode* geonode = selNode->cast<CNode>();
		if (ImGui::Button("Import geometry from DFF")) {
			auto filepath = OpenDialogBox(ui.g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				std::unique_ptr<RwClump> impClump = GeoUtils::LoadDFF(filepath);
				std::vector<RwGeometry*> rwgeos;
				for (RwAtomic& atom : impClump->atomics) {
					RwGeometry* rwgeotot = impClump->geoList.geometries[atom.geoIndex].get();
					rwgeos.push_back(rwgeotot);
				}
				GeoUtils::ChangeNodeGeometry(kenv, geonode, rwgeos.data(), rwgeos.size());

				ui.progeocache.clear();
			}
		}
		if (!geonode->geometry) {
			ImGui::Text("No geometry");
		}
		else {
			if (ImGui::Button("Export geometry to DFF")) {
				auto filepath = SaveDialogBox(ui.g_window, "Renderware Clump\0*.DFF\0\0", "dff");
				if (!filepath.empty()) {
					CKAnyGeometry* wgeo = geonode->geometry.get();
					CKAnyGeometry* kgeo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
					auto sharedRwgeo = std::make_shared<RwGeometry>(*kgeo->clump->atomic.geometry.get());
					RwGeometry& rwgeo = *sharedRwgeo;
					rwgeo.nativeVersion.reset();

					auto flushClump = [&]()
						{
							RwExtHAnim* hanim = nullptr;
							if (geonode->isSubclassOf<CAnimatedNode>()) {
								RwFrameList* framelist = geonode->cast<CAnimatedNode>()->frameList.get();
								hanim = (RwExtHAnim*)framelist->extensions[0].find(0x11E);
								assert(hanim);
							}

							RwClump clump = GeoUtils::CreateClumpFromGeo(sharedRwgeo, hanim);

							IOFile dff(filepath.c_str(), "wb");
							clump.serialize(&dff);
						};

					int splitPartNumber = 1;
					const auto baseName = filepath.stem().u8string();
					while (wgeo = wgeo->nextGeo.get()) {
						CKAnyGeometry* kgeo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
						if (rwgeo.numVerts + kgeo->clump->atomic.geometry->numVerts <= 65536) {
							rwgeo.merge(*kgeo->clump->atomic.geometry);
						}
						else {
							// too many vertices, we need to split
							filepath = filepath.parent_path() / std::filesystem::u8path(fmt::format("{}_Part{}{}", baseName, splitPartNumber, filepath.extension().u8string()));
							flushClump();
							splitPartNumber += 1;
							rwgeo = *kgeo->clump->atomic.geometry;
							filepath = filepath.parent_path() / std::filesystem::u8path(fmt::format("{}_Part{}{}", baseName, splitPartNumber, filepath.extension().u8string()));
						}
					}
					flushClump();
				}
			}
			CKAnyGeometry* kgeo = geonode->geometry.get();
			if (geonode->geometry->flags & 8192) {
				static const char* const costumeNames[4] = { "Gaul", "Roman", "Pirate", "Swimsuit" };
				geonode->geometry->costumes;
				if (ImGui::BeginListBox("Costume", ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 5.0f))) {
					for (size_t i = 0; i < kgeo->costumes.size(); i++) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##costumeEntry", kgeo->clump == kgeo->costumes[i])) {
							for (CKAnyGeometry* gp = kgeo; gp; gp = gp->nextGeo.get())
								gp->clump = gp->costumes[i];
						}
						ImGui::SameLine();
						if (i >= 4) ImGui::TextUnformatted("???");
						else ImGui::TextUnformatted(costumeNames[i]);
						ImGui::PopID();
					}
					ImGui::EndListBox();
				}
			}
			if (ImGui::CollapsingHeader("Materials")) {
				for (CKAnyGeometry* wgeo = geonode->geometry.get(); wgeo; wgeo = wgeo->nextGeo.get()) {
					CKAnyGeometry* geo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
					if (!geo->clump) {
						ImGui::BulletText("(Geometry with no clump)");
						continue;
					}
					const char* matname = "(no texture)";
					RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
					if (!rwgeo->materialList.materials.empty())
						if (rwgeo->materialList.materials[0].isTextured)
							matname = geo->clump->atomic.geometry->materialList.materials[0].texture.name.c_str();
					ImGui::PushID(geo);
					ImGui::BulletText("%s%s", matname, (geo != wgeo) ? " (duplicated geo)" : "");
					ImGui::InputScalar("Flags 1", ImGuiDataType_U32, &geo->flags, 0, 0, "%X", ImGuiInputTextFlags_CharsHexadecimal);
					ImGui::InputScalar("Flags 2", ImGuiDataType_U32, &geo->flags2, 0, 0, "%X", ImGuiInputTextFlags_CharsHexadecimal);
					if (geo->clump) {
						auto& flags = geo->clump->atomic.geometry->flags;
						ImGui::LabelText("RwGeo Flags", "%08X", flags);
						ImGui::CheckboxFlags("Rw Light", &flags, RwGeometry::RWGEOFLAG_LIGHT); ImGui::SameLine();
						ImGui::CheckboxFlags("Rw Modulate Colors", &flags, RwGeometry::RWGEOFLAG_MODULATECOLOR);
					}
					if (geo->material && kenv.hasClass<CMaterial>()) {
						ImGuiMemberListener ml{ kenv, ui };
						ml.setPropertyInfoList(ui.g_encyclo, geo->material.get());
						geo->material->reflectMembers2(ml, &kenv);
					}
					ImGui::PopID();
					ImGui::Separator();
				}
			}
		}
	}
	if (CFogBoxNodeFx* fogbox = selNode->dyncast<CFogBoxNodeFx>()) {
		if (ImGui::CollapsingHeader("Fog box")) {
			ImGuiMemberListener ml(kenv, ui);
			ml.setPropertyInfoList(ui.g_encyclo, fogbox);
			fogbox->reflectFog(ml, &kenv);
		}
	}
}
