#include "IGObjectList.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"

#include <imgui/imgui.h>

void EditorUI::IGObjectList(EditorInterface& ui)
{
	auto& kenv = ui.kenv;

	static const char* catnames[15] = { "Managers", "Services", "Hooks",
		"Hook Lives", "Groups", "Group Lives", "Components", "Camera",
		"Cinematic blocs", "Dictionaries", "Geometries", "Scene nodes",
		"Logic stuff", "Graphical stuff", "Errors"
	};
	auto handleObjTreeNode = [&ui](CKObject* obj) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			ui.selectedInspectorObjectRef = obj;
			ui.wndShowObjInspector = true;
		}
		IGObjectDragDropSource(ui, obj);
		};
	auto enumObjList = [&ui, &kenv, &handleObjTreeNode](KObjectList& objlist) {
		for (int i = 0; i < 15; i++) {
			if (ImGui::TreeNode(catnames[i])) {
				for (auto& cl : objlist.categories[i].type) {
					if (!cl.objects.empty()) {
						CKObject* first = cl.objects[0];
						if (ImGui::TreeNode(&cl, "%s (%i, %i), %zu objects", first->getClassName(), first->getClassCategory(), first->getClassID(), cl.objects.size())) {
							int n = 0;
							for (CKObject* obj : cl.objects) {
								bool b = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%i, refCount=%i, %s", n, obj->getRefCount(), kenv.getObjectName(obj));
								handleObjTreeNode(obj);
								if (b)
									ImGui::TreePop();
								n++;
							}
							ImGui::TreePop();
						}
					}
				}
				ImGui::TreePop();
			}
		}
		};
	if (ImGui::TreeNode("Global (GAME)")) {
		for (CKObject* obj : kenv.globalObjects) {
			bool b = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%s (%i, %i), refCount=%i, %s", obj->getClassName(), obj->getClassCategory(), obj->getClassID(), obj->getRefCount(), kenv.getObjectName(obj));
			handleObjTreeNode(obj);
			if (b)
				ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Level (LVL)")) {
		enumObjList(kenv.levelObjects);
		ImGui::TreePop();
	}
	int i = 0;
	for (auto& str : kenv.sectorObjects) {
		if (ImGui::TreeNode(&str, "Sector %i (STR %02i)", i + 1, i)) {
			enumObjList(str);
			ImGui::TreePop();
		}
		i++;
	}
}