#include "IGLevelInfoEditor.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"
#include "CoreClasses/CKManager.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"
#include "GameClasses/CKGameX1.h"
#include "GameClasses/CKGameX2.h"

#include <imgui/imgui.h>
#include <fmt/format.h>

namespace {
	void addNewSector(KEnvironment& kenv) {
		int strNumber = kenv.sectorObjects.size();
		auto& str = kenv.sectorObjects.emplace_back();
		kenv.sectorObjNames.emplace_back();
		int clcat = 0;
		for (auto& cat : str.categories) {
			cat.type.resize(kenv.levelObjects.categories[clcat].type.size());
			int clid = 0;
			for (auto& kcl : cat.type) {
				auto& lvltype = kenv.levelObjects.categories[clcat].type[clid];
				kcl.startId = (uint16_t)lvltype.objects.size();
				if (lvltype.info != 2) {
					for (int p = 0; p < strNumber; p++)
						kcl.startId += (uint16_t)kenv.sectorObjects[p].categories[clcat].type[clid].objects.size();
				}
				clid++;
			}
			clcat++;
		}
		kenv.numSectors++;

		// CKSector
		CKSector* ksector = kenv.createObject<CKSector>(-1);
		ksector->sgRoot = kenv.createObject<CSGSectorRoot>(strNumber);
		ksector->strId = strNumber + 1;
		ksector->unk1 = 2;
		ksector->soundDictionary = kenv.createObject<CKSoundDictionary>(strNumber);
		ksector->soundDictionary->cast<CKSoundDictionary>()->inactive = strNumber + 1;
		ksector->meshKluster = kenv.createObject<CKMeshKluster>(strNumber);

		// beacons
		auto& bs = kenv.levelObjects.getFirst<CKSrvBeacon>()->beaconSectors.emplace_back();

		// sgroot
		CTextureDictionary* texDict = kenv.createObject<CTextureDictionary>(strNumber);
		ksector->sgRoot->cast<CSGSectorRoot>()->texDictionary = texDict;
		texDict->piDict.nativeVersionPlatform = kenv.levelObjects.getFirst<CTextureDictionary>()->piDict.nativeVersionPlatform;
		ksector->sgRoot->cast<CSGSectorRoot>()->sectorNum = strNumber + 1;

		// Lvl
		CKLevel* klevel = kenv.levelObjects.getFirst<CKLevel>();
		klevel->sectors.emplace_back(ksector);

		// SoundManager
		kenv.levelObjects.getFirst<CKSoundManager>()->ksndmgrSndDicts.push_back(CKSoundDictionary::FULL_ID | ((strNumber + 1) << 17));

		// XXL2
		if (kenv.version >= KEnvironment::KVERSION_XXL2) {
			ksector->x2sectorDetector = kenv.createAndInitObject<CKSectorDetector>();

			CBackgroundManager* bgndMgr = kenv.levelObjects.getFirst<CBackgroundManager>();
			bgndMgr->sectorBackgrounds.emplace_back();

			CNode* bgndNode = kenv.createObject<CNode>(strNumber);
			bgndMgr->sectorBackgrounds.back().node = bgndNode;
			ksector->sgRoot->cast<CSGSectorRoot>()->insertChild(bgndNode);
			kenv.setObjectName(bgndNode, "Sector Background node");

			for (CKObject* obj : kenv.levelObjects.getClassType<CKSpawnPool>().objects) {
				CKSpawnPool* spawnPool = obj->cast<CKSpawnPool>();
				spawnPool->ckspUnk1.push_back(spawnPool->ckspUnk1.front());
			}

			// squads
			GameX2::CKGrpA2Enemy* grpEnemy = kenv.levelObjects.getFirst<GameX2::CKGrpA2Enemy>();
			GameX2::CKFightZoneSectorGrpRoot* sectorGrpRoot = kenv.createAndInitObject<GameX2::CKFightZoneSectorGrpRoot>();
			grpEnemy->addGroup(sectorGrpRoot);
			grpEnemy->fightZoneGroups.emplace_back(sectorGrpRoot);
			kenv.setObjectName(sectorGrpRoot, fmt::format("Zone(s) secteur {:02}", strNumber + 1));
		}
	}
}

void EditorUI::IGLevelInfoEditor(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	bool doDynGroundFix = false;
	if (kenv.version <= kenv.KVERSION_XXL2 && ImGui::Button("Add new sector")) {
		addNewSector(kenv);
		doDynGroundFix = true;
		// editor
		ui.progeocache.clear();
		ui.gndmdlcache.clear();
		ui.prepareLevelGfx();
	}
	ImGui::SameLine();
	ImGui::Text("%i sectors", kenv.numSectors);

	if (ImGui::Button("Fix last sector's dyngrounds") || doDynGroundFix) {
		// add common dynamic grounds in MeshKluster
		for (auto& str : kenv.sectorObjects) {
			auto& strGrounds = str.getFirst<CKMeshKluster>()->grounds;
			const auto& lvlGrounds = kenv.levelObjects.getFirst<CKMeshKluster>()->grounds;
			for (auto& ref : lvlGrounds) {
				if (ref && ref->isSubclassOf<CDynamicGround>()) {
					if (kenv.sectorObjects.size() >= 1) {
						auto& firstGrounds = kenv.sectorObjects[0].getFirst<CKMeshKluster>()->grounds;
						if (std::find(firstGrounds.begin(), firstGrounds.end(), ref) == firstGrounds.end())
							continue;
					}
					if (std::find(strGrounds.begin(), strGrounds.end(), ref) == strGrounds.end())
						strGrounds.push_back(ref);
				}
			}
		}
	}

	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::CollapsingHeader("Level Start")) {
		using namespace GameX1;
		CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
		static int cheatIndex = 0;
		if (!kenv.isRemaster) cheatIndex = 0;

		auto getCheatDesc = [level](int index) -> std::string {
			return std::to_string(index) + ": " + level->lvlRemasterCheatSpawnNames[index];
			};

		if (kenv.isRemaster) {
			if (ImGui::BeginCombo("Cheat", getCheatDesc(cheatIndex).c_str())) {
				for (int i = 0; i < 20; ++i) {
					ImGui::PushID(i);
					if (ImGui::Selectable("##cheatentry")) {
						cheatIndex = i;
					}
					ImGui::SameLine();
					ImGui::TextUnformatted(getCheatDesc(i).c_str());
					ImGui::PopID();
				}
				ImGui::EndCombo();
			}
			IGStringInput("Cheat name", level->lvlRemasterCheatSpawnNames[cheatIndex]);
		}
		ImGui::InputScalar("Initial sector", ImGuiDataType_U32, &level->initialSector[cheatIndex]);
		CKHkHero* heroes[3] = { kenv.levelObjects.getFirst<CKHkAsterix>(), kenv.levelObjects.getFirst<CKHkObelix>(), kenv.levelObjects.getFirst<CKHkIdefix>() };
		static constexpr const char* heroNames[3] = { "Asterix", "Obelix", "Dogmatix" };
		if (heroes[0] && heroes[1] && heroes[2]) {
			for (size_t i = 0; i < 3; ++i) {
				ImGui::InputFloat3(heroNames[i], &heroes[i]->heroUnk53[cheatIndex].x);
			}
			if (ImGui::Button("Teleport heroes to cursor and update start positions")) {
				Vector3 oriAstPos = heroes[0]->node->transform.getTranslationVector();
				Vector3 vec = ui.cursorPosition - oriAstPos;
				for (size_t i = 0; i < 3; ++i) {
					CKHkHero* hero = heroes[i];
					auto& mat = hero->node->transform;
					mat.setTranslation(mat.getTranslationVector() + vec);
					heroes[i]->heroUnk53[cheatIndex] = heroes[i]->node->transform.getTranslationVector();
				}
			}
			if (ImGui::Button("Update hero start positions from nodes")) {
				for (size_t i = 0; i < 3; ++i) {
					heroes[i]->heroUnk53[cheatIndex] = heroes[i]->node->transform.getTranslationVector();
				}
			}
		}
	}

	if (ImGui::CollapsingHeader("Sases")) {
		auto doBox = [&](AABoundingBox& box) {
			ImGui::PushID(&box);
			Vector3 boxSize = box.highCorner - box.lowCorner;
			bool mod = false;
			mod |= ImGui::DragFloat3("Position", &box.lowCorner.x, 1.0f);
			mod |= ImGui::DragFloat3("Size", &boxSize.x, 1.0f, 0.0f, FLT_MAX);
			if (mod) {
				box.highCorner = box.lowCorner + boxSize;
			}
			ImGui::PopID();
			};
		
		auto& sasObjList = kenv.levelObjects.getClassType<CKSas>();
		CKSas* removeSas = nullptr;
		for (CKObject* obj : sasObjList.objects) {
			CKSas* sas = (CKSas*)obj;
			ImGui::PushID(sas);
			ImGui::AlignTextToFramePadding();
			ImGui::BulletText("Sector %u to %u", sas->sector[0], sas->sector[1]);
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				removeSas = sas;
			}
			ImGui::SetItemTooltip("Remove");
			ImGui::Indent();
			ImGui::Text("%u -> %u:", sas->sector[0], sas->sector[1]);
			doBox(sas->boxes[0]);
			ImGui::Text("%u -> %u:", sas->sector[1], sas->sector[0]);
			doBox(sas->boxes[1]);
			ImGui::Unindent();
			ImGui::PopID();
		}

		if (removeSas) {
			CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
			auto pred = [&](const kobjref<CKSas>& ref) {return ref.get() == removeSas; };
			std::erase_if(level->sectors[removeSas->sector[0]]->sases, pred);
			std::erase_if(level->sectors[removeSas->sector[1]]->sases, pred);
			kenv.removeObject(removeSas);
		}

		if (ImGui::Button("Add new sas")) {
			ImGui::OpenPopup("NewSasPopup");
		}
		if (ImGui::BeginPopup("NewSasPopup")) {
			static int newFrom = 1;
			static int newTo = 1;
			ImGui::PushItemWidth(64.0f);
			ImGui::InputInt("##From", &newFrom, 0);
			ImGui::SameLine();
			ImGui::TextUnformatted("->");
			ImGui::SameLine();
			ImGui::InputInt("##To", &newTo, 0);

			bool valid = newFrom > 0 && newTo > 0 && newFrom != newTo && newFrom <= kenv.numSectors && newTo <= kenv.numSectors;
			if (valid) {
				for (CKObject* obj : sasObjList.objects) {
					CKSas* sas = (CKSas*)obj;
					if ((sas->sector[0] == newFrom && sas->sector[1] == newTo)
						|| (sas->sector[0] == newTo && sas->sector[1] == newFrom)) {
						valid = false;
						break;
					}
				}
			}
			ImGui::BeginDisabled(!valid);
			if (ImGui::Button("Add")) {
				CKSas* sas = kenv.createAndInitObject<CKSas>();
				sas->sector[0] = newFrom;
				sas->sector[1] = newTo;
				sas->boxes[0] = AABoundingBox(ui.cursorPosition + Vector3(-10, 20, 20), ui.cursorPosition + Vector3(-30, 0, -20));
				sas->boxes[1] = AABoundingBox(ui.cursorPosition + Vector3(30, 20, 20), ui.cursorPosition + Vector3(10, 0, -20));
				CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
				level->sectors[newFrom]->sases.push_back(sas);
				level->sectors[newTo]->sases.push_back(sas);
			}
			ImGui::EndDisabled();
			ImGui::EndPopup();
		}
	}

	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::CollapsingHeader("Sky colors")) {
		if (CKHkSkyLife* hkSkyLife = kenv.levelObjects.getFirst<CKHkSkyLife>()) {
			ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(hkSkyLife->skyColor);
			ImGui::ColorEdit4("Sky color", &c1.x);
			hkSkyLife->skyColor = ImGui::ColorConvertFloat4ToU32(c1);
			ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(hkSkyLife->cloudColor);
			ImGui::ColorEdit4("Cloud color", &c2.x);
			hkSkyLife->cloudColor = ImGui::ColorConvertFloat4ToU32(c2);
		}
	}
	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::CollapsingHeader("Level-handled objects")) {
		CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
		ImGui::PushID("LevelObjs");
		int i = 0;
		for (auto& kref : level->objs) {
			IGObjectSelectorRef(ui, std::to_string(i++).c_str(), kref);
		}
		if (ImGui::Button("Add"))
			level->objs.emplace_back();
		ImGui::PopID();
	}
}
