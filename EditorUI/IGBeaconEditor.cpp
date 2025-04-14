#include "IGBeaconEditor.h"

#include <cmath>
#include <numbers>

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "EditorUtils.h"
#include "PropFlagsEditor.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKLogic.h"

#include <imgui/imgui.h>

void EditorUI::IGBeaconEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	const auto getBeaconName = [&ui](int id) {return ui.g_encyclo.getBeaconName(id).c_str(); };
	static const char* bonusNamesX1[] = { "?", "Helmet", "Golden Helmet", "Potion", "Shield", "Ham", "x3 Multiplier", "x10 Multiplier", "Laurel", "Boar", "Retro Coin", "Remaster Coin" };
	static const char* bonusNamesX2[] = { "?", "Potion", "Helmet", "Golden Helmet", "Diamond Helmet", "x3 Multiplier", "x10 Multiplier", "Ham", "Shield" };
	const auto getBonusName = [&ui, &kenv](int bonusId) -> const char* {
		if (bonusId == -1)
			return "/";
		if (kenv.version == kenv.KVERSION_XXL1)
			return bonusNamesX1[bonusId];
		else if (kenv.version == kenv.KVERSION_XXL2)
			return bonusNamesX2[bonusId];
		return "?";
		};
	CKSrvBeacon* srvBeacon = kenv.levelObjects.getFirst<CKSrvBeacon>();
	if (ImGui::Button("Add beacon")) {
		ImGui::OpenPopup("AddBeacon");
	}
	ImGui::SameLine();
	static int spawnSector = -1, spawnPos = 1;
	ImGui::TextUnformatted("Sector:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	ImGui::InputInt("##spawnSector", &spawnSector);
	if (spawnSector < 0) spawnSector = 0;
	if (spawnSector > (int)kenv.numSectors) spawnSector = (int)kenv.numSectors;
	ImGui::SameLine();
	ImGui::TextUnformatted("at:");
	ImGui::SameLine();
	ImGui::RadioButton("Camera", &spawnPos, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Cursor", &spawnPos, 1);

	if (ImGui::Button("New bonus handler")) {
		ImGui::OpenPopup("AddHandler");
	}
	ImGui::SameLine();
	if (ImGui::Button("Update all kluster sphere bounds")) {
		for (CKObject* bk : kenv.levelObjects.getClassType<CKBeaconKluster>().objects)
			srvBeacon->updateKlusterBounds(bk->cast<CKBeaconKluster>());
		for (auto& str : kenv.sectorObjects)
			for (CKObject* bk : str.getClassType<CKBeaconKluster>().objects)
				srvBeacon->updateKlusterBounds(bk->cast<CKBeaconKluster>());
	}
	if (ImGui::BeginPopup("AddBeacon")) {
		for (auto& hs : srvBeacon->handlers) {
			ImGui::PushID(hs.handlerId);
			if (ImGui::MenuItem(getBeaconName(hs.handlerId))) {
				SBeacon beacon;
				if (spawnPos)
					beacon.setPosition(ui.cursorPosition);
				else
					beacon.setPosition(ui.camera.position + ui.camera.direction * 2.5f);

				if (auto* jsBeacon = ui.g_encyclo.getBeaconJson(hs.handlerId); jsBeacon && jsBeacon->is_object() && jsBeacon->contains("defaultParams"))
					beacon.params = (uint16_t)std::stoi(jsBeacon->at("defaultParams").get_ref<const std::string&>(), nullptr, 16);
				else if (hs.object && hs.object->isSubclassOf<CKCrateCpnt>())
					beacon.params = 0b001'010;
				else
					beacon.params = 0;

				int klusterIndex;
				if (kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
					std::tie(klusterIndex, std::ignore) = srvBeacon->addBeaconToNearestKluster(kenv, spawnSector, hs.handlerIndex, beacon);
				}
				else {
					klusterIndex = srvBeacon->addKluster(kenv, spawnSector);
					srvBeacon->addBeacon(spawnSector, klusterIndex, hs.handlerIndex, beacon);
				}
				srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[spawnSector].beaconKlusters[klusterIndex].get());
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(%02X %02X %02X %02X %02X)", hs.unk2a, hs.numBits, hs.handlerIndex, hs.handlerId, hs.persistent);
			ImGui::PopID();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopup("AddHandler")) {
		CKGroup* grpBonus = (kenv.version >= kenv.KVERSION_XXL2) ? (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonusX2>() : (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonus>();
		if (grpBonus) {
			for (CKGroup* grp = grpBonus->childGroup.get(); grp; grp = grp->nextGroup.get()) {
				if (CKGrpBonusPool* pool = grp->dyncast<CKGrpBonusPool>()) {
					int hid = pool->handlerId;
					// do not show bonus pools that already have a handler 
					auto it = std::find_if(srvBeacon->handlers.begin(), srvBeacon->handlers.end(), [hid](const auto& h) {return h.handlerId == hid; });
					if (it != srvBeacon->handlers.end())
						continue;
					if (ImGui::MenuItem(getBeaconName(hid))) {
						srvBeacon->addHandler(pool, 1, hid, 0, 0);
						uint32_t numBonuses = 0;
						for (CKHook* hk = pool->childHook.get(); hk; hk = hk->next.get())
							numBonuses++;
						pool->maxBeaconBonusesOnScreen = std::max(1u, numBonuses / 2u);
					}
				}
			}
		}
		else {
			ImGui::TextUnformatted("No bonus pools, no handlers!");
		}
		ImGui::EndPopup();
	}
	const auto enumBeaconKluster = [&ui, &getBeaconName](CKBeaconKluster* bk) {
		if (ImGui::TreeNode(bk, "Cluster (%f, %f, %f) radius %f", bk->bounds.center.x, bk->bounds.center.y, bk->bounds.center.z, bk->bounds.radius)) {
			ImGui::DragFloat3("Center##beaconKluster", &bk->bounds.center.x, 0.1f);
			ImGui::DragFloat("Radius##beaconKluster", &bk->bounds.radius, 0.1f);
			int nbing = 0;
			for (auto& bing : bk->bings) {
				int boffi = bing.bitIndex;
				if (!bing.beacons.empty())
					ImGui::BulletText("%s %02X %02X %02X %02X %02X %02X %04X %08X", getBeaconName(bing.handlerId), bing.unk2a, bing.numBits, bing.handlerId, bing.sectorIndex, bing.klusterIndex, bing.handlerIndex, bing.bitIndex, bing.unk6);
				int nbeac = 0;
				for (auto& beacon : bing.beacons) {
					ImGui::PushID(&beacon);
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					bool tn_open = ImGui::TreeNodeEx("beacon", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf, "(%i,%i) %f %f %f 0x%04X", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z, beacon.params);
					//if (ImGui::Selectable("##beacon")) {
					if (ImGui::IsItemClicked()) {
						ui.camera.position = pos - ui.camera.direction * 5.0f;
						ui.selBeaconSector = bing.sectorIndex;
						ui.selBeaconKluster = bing.klusterIndex;
						ui.selBeaconBing = nbing;
						ui.selBeaconIndex = nbeac;
					}
					if (tn_open) {
						ImGui::TreePop();
					}
					//ImGui::SameLine();
					//ImGui::Text("(%i,%i) %f %f %f", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z);
					ImGui::PopID();
					boffi += bing.numBits;
					nbeac++;
				}
				nbing++;
			}
			ImGui::TreePop();
		}
		};
	ImGui::Columns(2);
	ImGui::BeginChild("BeaconGraph");
	if (ImGui::TreeNode("Level")) {
		for (CKBeaconKluster* bk = kenv.levelObjects.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
			enumBeaconKluster(bk);
		ImGui::TreePop();
	}
	int i = 1;
	for (auto& str : kenv.sectorObjects) {
		if (ImGui::TreeNode(&str, "Sector %i", i)) {
			if (str.getClassType<CKBeaconKluster>().objects.size())
				for (CKBeaconKluster* bk = str.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
					enumBeaconKluster(bk);
			ImGui::TreePop();
		}
		i++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("BeaconInfo");
	bool removal = false; int remSector, remKluster, remBing, remBeacon;
	if (ui.selBeaconSector != -1) {
		CKBeaconKluster* bk = srvBeacon->beaconSectors[ui.selBeaconSector].beaconKlusters[ui.selBeaconKluster].get();
		CKBeaconKluster::Bing& bing = bk->bings[ui.selBeaconBing];
		SBeacon& beacon = bing.beacons[ui.selBeaconIndex];

		if (ImGui::Button("Remove")) {
			removal = true;
			remSector = bing.sectorIndex;
			remKluster = bing.klusterIndex;
			remBing = ui.selBeaconBing;
			remBeacon = ui.selBeaconIndex;
		}

		ImGui::Text("%s (%02X, %s)", getBeaconName(bing.handlerId), bing.handlerId, bing.handler->getClassName());
		ImGui::Text("Bits:");
		std::vector<bool>::iterator bitIterator = srvBeacon->getBeaconBitIterator(ui.selBeaconSector, ui.selBeaconKluster, ui.selBeaconBing, ui.selBeaconIndex);
		for (int i = 0; i < bing.numBits; i++) {
			ImGui::SameLine();
			ImGui::Text("%i", *(bitIterator + i) ? 1 : 0);
		}
		bool mod = false;
		mod |= ImGui::DragScalarN("Position##beacon", ImGuiDataType_S16, &beacon.posx, 3, 0.1f);
		mod |= ImGui::InputScalar("Params##beacon", ImGuiDataType_U16, &beacon.params, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		if (CKCrateCpnt* crateCpnt = bing.handler->dyncast<CKCrateCpnt>()) {
			ImGui::Separator();
			int cc = beacon.params & 7;
			if (ImGui::InputInt("Num crates", &cc)) {
				cc = std::clamp(cc, 1, 7);
				beacon.params &= ~7;
				beacon.params |= (cc & 7);
				mod = true;
			}
			cc = (beacon.params >> 3) & 7;
			if (ImGui::InputInt("Num bonuses", &cc)) {
				cc = std::clamp(cc, 0, 7);
				beacon.params &= ~(7 << 3);
				beacon.params |= (cc & 7) << 3;
				mod = true;
			}
			cc = (beacon.params >> 6) & 7;
			if (ImGui::InputInt("Arrangement", &cc)) {
				beacon.params &= ~(7 << 6);
				beacon.params |= (cc & 7) << 6;
				mod = true;
			}
			cc = (beacon.params >> 9) & 3;
			if (ImGui::BeginListBox("Bonus", ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing() * 4.0f + ImGui::GetStyle().FramePadding.y * 2.0f))) {
				ImGui::PushItemWidth(-1.0f);
				for (int bon = 0; bon < 4; bon++) {
					ImGui::PushID(bon);
					if (ImGui::RadioButton("##bonusRadio", &cc, bon)) {
						cc = bon;
						beacon.params &= ~(3 << 9);
						beacon.params |= (cc & 3) << 9;
						//mod = true;
					}
					ImGui::SameLine();
					int& bonusId = crateCpnt->bonuses[bon];
					if (ImGui::BeginCombo("##combo", getBonusName(bonusId))) {
						if (ImGui::Selectable("/", bonusId == -1))
							bonusId = -1;
						static std::vector<int> fndBonuses;
						fndBonuses.clear();
						CKGroup* grpBonus = (kenv.version >= kenv.KVERSION_XXL2) ? (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonusX2>() : (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonus>();
						for (CKGroup* grp = grpBonus->childGroup.get(); grp; grp = grp->nextGroup.get())
							if (!grp->isSubclassOf<CKGrpWildBoarPool>()) // sorry, no wild boars inside crates :(
								fndBonuses.push_back(grp->cast<CKGrpBonusPool>()->bonusType);
						std::sort(fndBonuses.begin(), fndBonuses.end());
						for (int bid : fndBonuses)
							if (ImGui::Selectable(getBonusName(bid), bonusId == bid))
								bonusId = bid;
						ImGui::EndCombo();
					}
					ImGui::PopID();
				}
				ImGui::PopItemWidth();
				ImGui::EndListBox();
			}
		}
		else if (auto* jsBeaconInfo = ui.g_encyclo.getBeaconJson(bing.handlerId)) {
			if (jsBeaconInfo->is_object()) {
				bool isOrientable = jsBeaconInfo->is_object() && jsBeaconInfo->value<bool>("orientable", false);
				if (isOrientable) {
					float angle = decode8bitAngle(beacon.params & 255) * 180.0f / std::numbers::pi_v<float>;
					if (ImGui::SliderFloat("Orientation", &angle, 0.0f, 360.0f, u8"%.1f\u00B0")) {
						beacon.params = (beacon.params & 0xFF00) | (uint8_t)std::round(angle * 256.0f / 360.0f);
					}
				}
				if (auto itParams = jsBeaconInfo->find("params"); itParams != jsBeaconInfo->end()) {
					unsigned int modParams = beacon.params;
					if (PropFlagsEditor(modParams, itParams.value()))
						beacon.params = (uint16_t)modParams;
				}
			}
		}
		if (mod) {
			if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
				CKSrvBeacon* srvBeacon = kenv.levelObjects.getFirst<CKSrvBeacon>();
				auto bitIterator = srvBeacon->getBeaconBitIterator(ui.selBeaconSector, ui.selBeaconKluster, ui.selBeaconBing, ui.selBeaconIndex);
				for (int i = 0; i < 6; i++)
					*(bitIterator + i) = beacon.params & (1 << i);
				*(bitIterator + 6) = false;
			}
			if (kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
				SBeacon beaconCopy = beacon;
				srvBeacon->removeBeacon(ui.selBeaconSector, ui.selBeaconKluster, ui.selBeaconBing, ui.selBeaconIndex);
				srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[ui.selBeaconSector].beaconKlusters[ui.selBeaconKluster].get());
				srvBeacon->cleanEmptyKlusters(kenv, ui.selBeaconSector);
				std::tie(ui.selBeaconKluster, ui.selBeaconIndex) = srvBeacon->addBeaconToNearestKluster(kenv, ui.selBeaconSector, ui.selBeaconBing, beaconCopy);
				srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[ui.selBeaconSector].beaconKlusters[ui.selBeaconKluster].get());
			}
			else {
				srvBeacon->updateKlusterBounds(bk);
			}
		}
	}
	ImGui::EndChild();
	ImGui::Columns();

	if (removal) {
		srvBeacon->removeBeacon(remSector, remKluster, remBing, remBeacon);
		if (kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
			srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[remSector].beaconKlusters[remKluster].get());
			srvBeacon->cleanEmptyKlusters(kenv, remSector);
		}
		ui.selBeaconSector = -1;
		ui.rayHits.clear();
		ui.nearestRayHit = nullptr;
	}
}
