#include "IGCameraEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "ImGuiMemberListener.h"

#include "KEnvironment.h"
#include "CKUtils.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKCamera.h"
#include "CoreClasses/CKLogic.h"

#include <imgui/imgui.h>

void EditorUI::IGCameraEditor(EditorInterface& ui)
{
	static KWeakRef<CKCameraBase> selectedCamera;
	
	auto& kenv = ui.kenv;

	CKSrvCamera* srvCamera = kenv.levelObjects.getFirst<CKSrvCamera>();
	auto viewCamera = [&ui](CKCameraBase* kcamera) {
		ui.camera.position = kcamera->kcamPosition;
		Vector3 newDir = (kcamera->kcamLookAt - ui.camera.position).normal();
		float newAngleX = std::asin(newDir.y);
		Vector3 newOri;
		float cosAX = std::cos(newAngleX);
		float mar = 1.5707f;
		if (!(-mar <= newAngleX && newAngleX <= mar))
			newOri = { std::clamp(newAngleX, -mar + 0.0005f, mar - 0.0005f), 0, 0 };
		else
			newOri = { newAngleX, std::atan2(-newDir.x / cosAX, newDir.z / cosAX) };
		ui.camera.orientation = newOri;
		};
	if (ImGui::BeginTable("CameraTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if (ImGui::Button("New camera")) {
			ImGui::OpenPopup("NewCamera");
		}
		if (ImGui::BeginPopup("NewCamera")) {
			int toadd = -1;
			auto cls = [&toadd](int id, const char* name) {
				if (ImGui::Selectable(name))
					toadd = id;
				};
			cls(CKCamera::FULL_ID, "CKCamera");
			cls(CKCameraRigidTrack::FULL_ID, "CKCameraRigidTrack");
			if (kenv.version == KEnvironment::KVERSION_XXL1)
				cls(CKCameraClassicTrack::FULL_ID, "CKCameraClassicTrack");
			cls(CKCameraPathTrack::FULL_ID, "CKCameraPathTrack");
			cls(CKCameraFixTrack::FULL_ID, "CKCameraFixTrack");
			cls(CKCameraAxisTrack::FULL_ID, "CKCameraAxisTrack");
			cls(CKCameraSpyTrack::FULL_ID, "CKCameraSpyTrack");
			cls(CKCameraPassivePathTrack::FULL_ID, "CKCameraPassivePathTrack");
			if (kenv.version >= KEnvironment::KVERSION_XXL2) {
				cls(CKCameraBalistTrack::FULL_ID, "CKCameraBalistTrack");
				cls(CKCameraClassicTrack2::FULL_ID, "CKCameraClassicTrack2");
				cls(CKCameraFirstPersonTrack::FULL_ID, "CKCameraFirstPersonTrack");
			}
			if (toadd != -1) {
				kenv.levelObjects.getClassType(toadd).instantiation = KInstantiation::LevelUnique;
				CKCameraBase* added = kenv.createObject((uint32_t)toadd, -1)->cast<CKCameraBase>();
				added->init(&kenv);
				added->kcamNextCam = srvCamera->scamCam;
				srvCamera->scamCam = added;
			}
			ImGui::EndPopup();
		}
		ImGui::BeginChild("CameraList");
		for (CKCameraBase* camera = srvCamera->scamCam.get(); camera; camera = camera->kcamNextCam.get()) {
			ImGui::PushID(camera);
			if (ImGui::Selectable("##CamSel", selectedCamera == camera)) {
				selectedCamera = camera;
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					viewCamera(camera);
				}
			}
			ImGui::SameLine();
			ImGui::Text("%s %s", camera->getClassName(), kenv.getObjectName(camera));
			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::TableNextColumn();
		if (CKCameraBase* kcamera = selectedCamera.get()) {
			ImGui::BeginChild("CameraProps");
			IGObjectNameInput("Name", kcamera, kenv);
			bool mod = false;
			mod |= ImGui::DragFloat("FOV", &kcamera->kcamFOV);
			mod |= ImGui::DragFloat3("Position", &kcamera->kcamPosition.x);
			mod |= ImGui::DragFloat3("Look at", &kcamera->kcamLookAt.x);
			mod |= ImGui::DragFloat3("Up vector", &kcamera->kcamUpVector.x);
			if (ImGui::Button("View")) {
				viewCamera(kcamera);
			}
			ImGui::SameLine();
			if (ImGui::Button("Set")) {
				kcamera->kcamPosition = ui.camera.position;
				kcamera->kcamLookAt = ui.camera.position + ui.camera.direction;
				mod = true;
			}
			if (mod) {
				kcamera->kcamFarDistance_dup = kcamera->kcamFarDistance;
				kcamera->kcamUnk3_dup = kcamera->kcamUnk3;
				kcamera->kcamFOV_dup = kcamera->kcamFOV;
				kcamera->kcamPosition_dup = kcamera->kcamPosition;
				kcamera->kcamLookAt_dup = kcamera->kcamLookAt;
				kcamera->kcamUpVector_dup = kcamera->kcamUpVector;
			}
			if (kcamera->ogFogData && ImGui::CollapsingHeader("Fog data")) {
				CKCameraFogDatas* fogData = kcamera->ogFogData.get();
				IGU32Color("Color 1", fogData->color1);
				ImGui::InputFloat("unk2", &fogData->unk2);
				ImGui::InputFloat("unk3", &fogData->unk3);
				IGU32Color("Color 2", fogData->color2);
				ImGui::InputFloat("unk4", &fogData->unk4);
				ImGui::InputFloat("unk5", &fogData->unk5);
				ImGui::InputFloat("unk6", &fogData->unk6);
			}
			struct CameraEditMemberListener : FilterMemberListener<CameraEditMemberListener, ImGuiMemberListener> {
				using FilterMemberListener::FilterMemberListener;
				bool _allow = true;
				bool cond(const char* name) {
					return _allow;
				}
				virtual void message(const char* msg) override {
					if (msg == std::string_view("End of CKCamera")) {
						_allow = true;
					}
				}
			};
			bool showBaseMembers = false;
			if (ImGui::CollapsingHeader("Base members"))
				showBaseMembers = true;
			CameraEditMemberListener iml{ kenv, ui };
			iml.setPropertyInfoList(ui.g_encyclo, kcamera);
			iml._allow = showBaseMembers;
			kcamera->virtualReflectMembers(iml, &kenv);
			ImGui::EndChild();
		}
		ImGui::EndTable();
	}

}
