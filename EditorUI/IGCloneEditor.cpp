#include "IGCloneEditor.h"

#include "EditorInterface.h"

#include "KEnvironment.h"
#include "CoreClasses/CKGraphical.h"

#include "GeoUtils.h"
#include "GuiUtils.h"

#include <imgui/imgui.h>

void EditorUI::IGCloneEditor(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;

	CCloneManager* cloneMgr = kenv.levelObjects.getFirst<CCloneManager>();
	if (!cloneMgr) return;
	if (cloneMgr->_numClones == 0) return;

	ImGui::DragFloat3("Preview pos", &ui.selgeoPos.x, 0.1f);
	if (ImGui::Button("Move preview to front"))
		ui.selgeoPos = ui.camera.position + ui.camera.direction * 3;

	if (!ui.selClones.empty()) {
		ImGui::SameLine();
		if (ImGui::Button("Import DFF")) {
			auto filepath = OpenDialogBox(ui.g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				std::unique_ptr<RwClump> impClump = GeoUtils::LoadDFF(filepath);
				std::vector<std::unique_ptr<RwGeometry>> impGeos = impClump->geoList.geometries[0]->splitByMaterial();

				std::vector<uint32_t> newIndices;
				for (size_t p = 0; p < impGeos.size(); ++p) {
					if (p < ui.selClones.size()) {
						const uint32_t bingIndex = ui.selClones[p];
						auto& tdGeo = cloneMgr->_teamDict._bings[ui.selClones[p]]._clump.atomic.geometry;
						tdGeo = std::move(impGeos[p]);
						newIndices.push_back(bingIndex);
					}
					else {
						const int prevBingSomeNum = ui.selClones.empty() ? 1 : cloneMgr->_teamDict._bings[ui.selClones[0]]._someNum;
						const uint32_t bingIndex = (uint32_t)cloneMgr->_teamDict._bings.size();
						auto& bing = cloneMgr->_teamDict._bings.emplace_back();
						bing._someNum = prevBingSomeNum;
						bing._clump.atomic.flags = 5;
						bing._clump.atomic.unused = 0;
						bing._clump.atomic.geometry = std::move(impGeos[p]);
						newIndices.push_back(bingIndex);
					}
				}
				ui.selGeometry = nullptr;

				for (auto& dong : cloneMgr->_team.dongs) {
					if (dong.bongs == ui.selClones) {
						dong.bongs = newIndices;
					}
				}
				ui.selClones = newIndices;

				ui.progeocache.clear();
				ui.prepareLevelGfx();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Export DFF")) {
			auto filepath = SaveDialogBox(ui.g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				RwTeam::Dong* seldong = nullptr;
				for (auto& dong : cloneMgr->_team.dongs)
					if (dong.bongs == ui.selClones)
						seldong = &dong;

				if (!seldong) {
					MsgBox(ui.g_window, "Sorry, I couldn't find back the team entry with the selected team dict indices :(", 16);
				}
				else {
					RwFrameList* framelist = &seldong->clump.frameList;
					RwExtHAnim* hanim = (RwExtHAnim*)framelist->extensions[1].find(0x11E);	// null if not found

					// merge clone geos
					auto sharedMergedGeo = std::make_shared<RwGeometry>();
					RwGeometry& mergedGeo = *sharedMergedGeo; bool first = true;
					for (auto td : seldong->bongs) {
						const RwGeometry& tdgeo = *cloneMgr->_teamDict._bings[td]._clump.atomic.geometry.get();
						if (first) {
							mergedGeo = tdgeo;
							first = false;
						}
						else {
							mergedGeo.merge(tdgeo);
						}
					}

					RwClump clump = GeoUtils::CreateClumpFromGeo(sharedMergedGeo, hanim);
					IOFile out(filepath.c_str(), "wb");
					clump.serialize(&out);
					out.close();
				}
			}
		}
	}

	ImGui::BeginChild("CloneList");
	for (auto& clone : ui.cloneSet) {
		std::string lol;
		for (size_t i = 0; i < clone.size(); i++) {
			uint32_t de = clone[i];
			std::string texname = "?";
			const auto& matlist = cloneMgr->_teamDict._bings[de]._clump.atomic.geometry->materialList.materials;
			if (!matlist.empty())
				texname = matlist[0].texture.name;
			if (i != 0) lol.append(", ");
			lol.append(texname);
		}
		ImGui::PushID(&clone);
		if (ImGui::Selectable(lol.c_str(), ui.selClones == clone))
			ui.selClones = clone;
		ImGui::PopID();
	}
	ImGui::EndChild();
}
