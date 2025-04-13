#include "IGDetectorEditor.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"

#include "imgui/imgui.h"

void EditorUI::IGDetectorEditorXXL1(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	CKSrvDetector* srvDetector = kenv.levelObjects.getFirst<CKSrvDetector>();
	if (!srvDetector) return;
	auto coloredTreeNode = [](const char* label, const ImVec4& color = ImVec4(1, 1, 1, 1)) {
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		bool res = ImGui::TreeNode(label);
		ImGui::PopStyleColor();
		return res;
		};
	auto enumdctlist = [&ui, &coloredTreeNode](std::vector<CKSrvDetector::Detector>& dctlist, const char* name, const ImVec4& color = ImVec4(1, 1, 1, 1), bool isInsideDetector = false, int filterShape = -1) {
		if (filterShape != -1)
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (coloredTreeNode(name, color)) {

			for (size_t i = 0; i < dctlist.size(); ++i) {
				auto& dct = dctlist[i];
				if (filterShape != -1 && (int)dct.shapeIndex != filterShape)
					continue;
				ImGui::PushID(&dct);
				ImGui::BulletText("#%zi", i);
				if (filterShape == -1)
					ImGui::InputScalar("Shape index", ImGuiDataType_U16, &dct.shapeIndex);
				ImGui::InputScalar("Node index", ImGuiDataType_U16, &dct.nodeIndex);
				ImGui::InputScalar("Flags", ImGuiDataType_U16, &dct.flags, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
				unsigned int modflags = dct.flags;
				bool flagsModified = false;
				flagsModified |= ImGui::CheckboxFlags("Enabled on start", &modflags, 2);
				modflags = (modflags & ~3) | ((modflags & 2) ? 3 : 0);
				if (flagsModified)
					dct.flags = (uint16_t)modflags;
				IGEventSelector(ui, "Event sequence", dct.eventSeqIndex);
				ImGui::Separator();
				ImGui::PopID();
			}
			if (ImGui::Button("New")) {
				dctlist.emplace_back();
				dctlist.back().flags = isInsideDetector ? 0x0133 : 0x010F;
				if (filterShape != -1)
					dctlist.back().shapeIndex = filterShape;
			}

			ImGui::TreePop();
		}
		};

	Vector3 creationPosition = ui.cursorPosition;

	if (ImGui::BeginTabBar("DetectorTabBar")) {
		if (ImGui::BeginTabItem("Shapes")) {
			ImGui::Columns(2);
			ImGui::BeginChild("DctShapesList");
			if (coloredTreeNode("Bounding boxes", ImVec4(0, 1, 0, 1))) {
				int i = 0;
				for (auto& bb : srvDetector->aaBoundingBoxes) {
					bool selected = ui.selectedShapeType == 0 && ui.selectedShapeIndex == i;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0);
					if (ImGui::TreeNodeEx(&bb, flags, "%3i: %s (%.2f,%.2f,%.2f) - (%.2f,%.2f,%.2f)", i, srvDetector->aabbNames[i].c_str(), bb.highCorner.x, bb.highCorner.y, bb.highCorner.z, bb.lowCorner.x, bb.lowCorner.y, bb.lowCorner.z))
						ImGui::TreePop();
					if (ImGui::IsItemActivated()) {
						ui.selectedShapeType = 0; ui.selectedShapeIndex = i;
					}
					++i;
				}
				if (ImGui::Button("New")) {
					srvDetector->aaBoundingBoxes.emplace_back(creationPosition + Vector3(1, 1, 1), creationPosition - Vector3(1, 1, 1));
					srvDetector->aabbNames.emplace_back("New bounding box");
				}
				ImGui::TreePop();
			}
			if (coloredTreeNode("Spheres", ImVec4(1, 0.5f, 0, 1))) {
				int i = 0;
				for (auto& sph : srvDetector->spheres) {
					bool selected = ui.selectedShapeType == 1 && ui.selectedShapeIndex == i;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0);
					if (ImGui::TreeNodeEx(&sph, flags, "%3i: %s c=(%.2f,%.2f,%.2f) r=%f", i, srvDetector->sphNames[i].c_str(), sph.center.x, sph.center.y, sph.center.z, sph.radius))
						ImGui::TreePop();
					if (ImGui::IsItemActivated()) {
						ui.selectedShapeType = 1; ui.selectedShapeIndex = i;
					}
					++i;
				}
				if (ImGui::Button("New")) {
					srvDetector->spheres.emplace_back(creationPosition, 1.0f);
					srvDetector->sphNames.emplace_back("New sphere");
				}
				ImGui::TreePop();
			}
			if (coloredTreeNode("Rectangles", ImVec4(1, 0, 1, 1))) {
				int i = 0;
				for (auto& rect : srvDetector->rectangles) {
					bool selected = ui.selectedShapeType == 2 && ui.selectedShapeIndex == i;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0);
					if (ImGui::TreeNodeEx(&rect, flags, "%3i: %s c=(%.2f,%.2f,%.2f) dir=%i", i, srvDetector->rectNames[i].c_str(), rect.center.x, rect.center.y, rect.center.z, rect.direction))
						ImGui::TreePop();
					if (ImGui::IsItemActivated()) {
						ui.selectedShapeType = 2; ui.selectedShapeIndex = i;
					}
					++i;
				}
				if (ImGui::Button("New")) {
					srvDetector->rectangles.emplace_back();
					srvDetector->rectangles.back().center = creationPosition;
					srvDetector->rectNames.emplace_back("New rectangle");
				}
				ImGui::TreePop();
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("DctShapeInfo");
			if (ui.selectedShapeType == 0 && ui.selectedShapeIndex >= 0 && ui.selectedShapeIndex < srvDetector->aaBoundingBoxes.size()) {
				auto& bb = srvDetector->aaBoundingBoxes[ui.selectedShapeIndex];
				IGStringInput("Name", srvDetector->aabbNames[ui.selectedShapeIndex]);
				ImGui::DragFloat3("High corner", &bb.highCorner.x, 0.1f);
				ImGui::DragFloat3("Low corner", &bb.lowCorner.x, 0.1f);
				if (ImGui::Button("See OvO"))
					ui.camera.position = Vector3(bb.highCorner.x, ui.camera.position.y, bb.highCorner.z);
				enumdctlist(srvDetector->aDetectors, "On Enter Box", ImVec4(0, 1, 0, 1), false, ui.selectedShapeIndex);
				enumdctlist(srvDetector->dDetectors, "While Inside Box", ImVec4(0, 1, 0, 1), true, ui.selectedShapeIndex);
			}
			else if (ui.selectedShapeType == 1 && ui.selectedShapeIndex >= 0 && ui.selectedShapeIndex < srvDetector->spheres.size()) {
				auto& sph = srvDetector->spheres[ui.selectedShapeIndex];
				IGStringInput("Name", srvDetector->sphNames[ui.selectedShapeIndex]);
				ImGui::DragFloat3("Center", &sph.center.x, 0.1f);
				ImGui::DragFloat("Radius", &sph.radius, 0.1f);
				if (ImGui::Button("See OvO"))
					ui.camera.position = Vector3(sph.center.x, ui.camera.position.y, sph.center.z);
				enumdctlist(srvDetector->bDetectors, "On Enter Sphere", ImVec4(1, 0.5f, 0, 1), false, ui.selectedShapeIndex);
				enumdctlist(srvDetector->eDetectors, "While Inside Sphere", ImVec4(1, 0.5f, 0, 1), true, ui.selectedShapeIndex);
			}
			else if (ui.selectedShapeType == 2 && ui.selectedShapeIndex >= 0 && ui.selectedShapeIndex < srvDetector->rectangles.size()) {
				auto& rect = srvDetector->rectangles[ui.selectedShapeIndex];
				IGStringInput("Name", srvDetector->rectNames[ui.selectedShapeIndex]);
				ImGui::DragFloat3("Center", &rect.center.x, 0.1f);
				ImGui::DragFloat("Length 1", &rect.length1, 0.1f);
				ImGui::DragFloat("Length 2", &rect.length2, 0.1f);
				ImGui::InputScalar("Direction", ImGuiDataType_U8, &rect.direction);
				if (ImGui::Button("See OvO"))
					ui.camera.position = Vector3(rect.center.x, ui.camera.position.y, rect.center.z);
				enumdctlist(srvDetector->cDetectors, "On Cross Rectangle", ImVec4(1, 0, 1, 1), false, ui.selectedShapeIndex);
			}
			ImGui::EndChild();
			ImGui::Columns(1);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Checklist")) {
			ImGui::BeginChild("DetectorChecklist");
			ImGui::PushID("checklist");
			enumdctlist(srvDetector->aDetectors, "Entering Bounding boxes", ImVec4(0, 1, 0, 1), false);
			enumdctlist(srvDetector->bDetectors, "Entering Spheres", ImVec4(1, 0.5f, 0, 1), false);
			enumdctlist(srvDetector->cDetectors, "Crossing Rectangles", ImVec4(1, 0, 1, 1), false);
			enumdctlist(srvDetector->dDetectors, "Being in Bounding boxes", ImVec4(0, 1, 0, 1), true);
			enumdctlist(srvDetector->eDetectors, "Being in Spheres", ImVec4(1, 0.5f, 0, 1), true);
			ImGui::PopID();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Scene Nodes")) {
			ImGui::BeginChild("DetectorSceneNodes");
			int i = 0;
			ImGui::PushItemWidth(-32.0f);
			for (auto& node : srvDetector->nodes) {
				IGObjectSelectorRef(ui, std::to_string(i++).c_str(), node);
			}
			ImGui::PopItemWidth();
			if (ImGui::Button("New")) {
				srvDetector->nodes.emplace_back();
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void EditorUI::IGDetectorEditorXXL2Plus(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	if (kenv.version < kenv.KVERSION_XXL2)
		return;
	int strid = 0;
	ImGui::Columns(2);
	ImGui::BeginChild("DtcList");
	for (CKObject* osector : kenv.levelObjects.getClassType<CKSectorDetector>().objects) {
		CKSectorDetector* sector = osector->cast<CKSectorDetector>();
		if (ImGui::TreeNode(sector, "Sector %i", strid)) {
			for (auto& detector : sector->sdDetectors) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
				if (ui.selectedX2Detector.get() == detector.get())
					flags |= ImGuiTreeNodeFlags_Selected;
				if (ImGui::TreeNodeEx(detector.get(), flags, "%s", kenv.getObjectName(detector.get())))
					ImGui::TreePop();
				if (ImGui::IsItemClicked())
					ui.selectedX2Detector = detector.get();
				IGObjectDragDropSource(ui, detector.get());
			}
			CKDetectorBase* addedDetector = nullptr;
			if (ImGui::Button("Add Event detector")) {
				addedDetector = kenv.createAndInitObject<CKDetectorEvent>();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add Music detector")) {
				addedDetector = kenv.createAndInitObject<CKDetectorMusic>();
			}
			if (addedDetector) {
				addedDetector->dbFlags = 113;
				addedDetector->dbMovable = kenv.levelObjects.getFirst<CKDetectedMovable>();
				addedDetector->dbGeometry = kenv.createAndInitObject<CMultiGeometry>();
				addedDetector->dbSectorIndex = strid;
				addedDetector->dbGeometry->mgShape.emplace<AABoundingBox>(ui.cursorPosition + Vector3(1.0f, 1.0f, 1.0f), ui.cursorPosition - Vector3(1.0f, 1.0f, 1.0f));
				sector->sdDetectors.emplace_back(addedDetector);
			}
			ImGui::TreePop();
		}
		++strid;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("DtcProps");
	if (CKDetectorBase* detector = ui.selectedX2Detector.get()) {
		IGObjectNameInput("Name", detector, kenv);
		ImGuiMemberListener igml(kenv, ui);
		igml.setPropertyInfoList(ui.g_encyclo, detector);
		detector->virtualReflectMembers(igml, &kenv);
		if (ImGui::CollapsingHeader("Geometry") && detector->dbGeometry) {
			auto& mgShape = detector->dbGeometry->mgShape;
			static const char* shapeTypeNames[3] = { "Box", "Sphere", "Rectangle" };
			int shapeType = mgShape.index();
			if (ImGui::Combo("Shape Type", &shapeType, shapeTypeNames, 3)) {
				// obtain center from old shape
				Vector3 center;
				if (auto* aabb = std::get_if<AABoundingBox>(&mgShape)) {
					center = (aabb->highCorner + aabb->lowCorner) * 0.5f;
				}
				else if (auto* sph = std::get_if<BoundingSphere>(&mgShape)) {
					center = sph->center;
				}
				else if (auto* rect = std::get_if<AARectangle>(&mgShape)) {
					center = rect->center;
				}
				// then create new shape
				if (shapeType == 0)
					mgShape.emplace<AABoundingBox>(center + Vector3(1.0f, 1.0f, 1.0f), center - Vector3(1.0f, 1.0f, 1.0f));
				else if (shapeType == 1)
					mgShape.emplace<BoundingSphere>(center, 1.0f);
				else if (shapeType == 2)
					mgShape.emplace<AARectangle>(center);
			}
			detector->dbGeometry->reflectMembers2(igml, &kenv);
			ImGui::Text("%i references", detector->dbGeometry->getRefCount());
		}
		if (ImGui::CollapsingHeader("Movables") && detector->dbMovable) {
			detector->dbMovable->reflectMembers2(igml, &kenv);
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}
