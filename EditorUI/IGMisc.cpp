#include "IGMisc.h"
#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "GuiUtils.h"

#include "CoreClasses/CKManager.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKCinematicNode.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKNode.h"
#include "GameClasses/CKGameX1.h"
#include "GameClasses/CKGameX2.h"

#include <imgui/imgui.h>
#include <fmt/format.h>

using namespace GuiUtils;

namespace {
	void GimmeTheRocketRomans(KEnvironment& kenv)
	{
		using namespace GameX1;
		std::map<CKHkBasicEnemy*, CKHkRocketRoman*> hkmap;
		for (CKObject* obj : kenv.levelObjects.getClassType<CKHkBasicEnemy>().objects) {
			CKHkBasicEnemy* hbe = obj->cast<CKHkBasicEnemy>();
			CKHkRocketRoman* hrr = kenv.createObject<CKHkRocketRoman>(-1);
			hkmap[hbe] = hrr;
			for (auto& ref : hbe->boundingShapes)
				ref->object = hrr;
			hbe->beBoundNode->object = hrr;
			hbe->life->hook = hrr;

			// copy
			*static_cast<CKHkBasicEnemy*>(hrr) = *hbe;

			// Rocket-specific values
			CKAACylinder* rrsphere = kenv.createObject<CKAACylinder>(-1);
			rrsphere->transform = kenv.levelObjects.getFirst<CSGSectorRoot>()->transform;
			rrsphere->radius = 2.0f;
			rrsphere->cylinderHeight = 2.0f;
			rrsphere->cylinderRadius = 2.0f;
			assert(hrr->romanAnimatedClone == hrr->romanAnimatedClone2 && hrr->romanAnimatedClone3 == hrr->node && hrr->node == hrr->romanAnimatedClone);
			hrr->romanAnimatedClone2->insertChild(rrsphere);
			//hrr->rrCylinderNode = rrsphere;

			hrr->rrCylinderNode = hrr->boundingShapes[3]->cast<CKAACylinder>();
			hrr->boundingShapes[3] = rrsphere;

			CKSoundDictionaryID* sdid = kenv.createObject<CKSoundDictionaryID>(-1);
			sdid->soundEntries.resize(32); // add 32 default (empty) sounds
			int sndid = 0;
			for (auto& se : sdid->soundEntries) {
				se.active = true;
				se.id = sndid++;
				se.flags = 16;
				se.obj = hrr->node.get();
			}
			hrr->rrSoundDictID = sdid;

			hrr->rrParticleNode = kenv.levelObjects.getFirst<CKCrateCpnt>()->particleNode.get();

			CAnimationDictionary* animDict = kenv.createAndInitObject<CAnimationDictionary>();
			hrr->rrAnimDict = animDict;
			animDict->numAnims = 4;
			animDict->animIndices.resize(animDict->numAnims);
			for (int i = 0; i < animDict->numAnims; ++i)
				animDict->animIndices[i] = hrr->beAnimDict->animIndices[i];
		}
		for (CKObject* obj : kenv.levelObjects.getClassType<CKHkBasicEnemy>().objects) {
			CKHkBasicEnemy* hbe = obj->cast<CKHkBasicEnemy>();
			CKHkRocketRoman* hrr = hkmap[hbe];
			if (hbe->next.get())
				hrr->next = hkmap[(CKHkBasicEnemy*)hbe->next.get()];
			hbe->next.reset();
		}
		CKSrvCollision* col = kenv.levelObjects.getFirst<CKSrvCollision>();
		for (auto& ref : col->objs2)
			if (ref->getClassFullID() == CKHkBasicEnemy::FULL_ID)
				ref = hkmap[ref->cast<CKHkBasicEnemy>()];
		for (CKObject* obj : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
			CKGrpSquadEnemy* gse = obj->cast<CKGrpSquadEnemy>();
			for (auto& pe : gse->pools) {
				if (pe.cpnt->getClassFullID() == CKBasicEnemyCpnt::FULL_ID) {
					CKBasicEnemyCpnt* becpnt = pe.cpnt->cast<CKBasicEnemyCpnt>();
					CKRocketRomanCpnt* rrcpnt = kenv.createObject<CKRocketRomanCpnt>(-1);
					*(CKBasicEnemyCpnt*)rrcpnt = *becpnt;
					//....
					rrcpnt->rrCylinderRadius = 1.0f;
					rrcpnt->rrCylinderHeight = 1.0f;
					rrcpnt->rrUnk3 = Vector3(1.0f, 1.0f, 1.0f);
					rrcpnt->rrUnk4 = 0;
					rrcpnt->rrFireDistance = 3.0f;
					rrcpnt->rrUnk6 = 0;
					rrcpnt->rrFlySpeed = 5.0f;
					rrcpnt->rrRomanAimFactor = 10.0f;
					rrcpnt->rrUnk9 = kenv.levelObjects.getClassType(2, 28).objects[0]; // Asterix Hook
					//
					pe.cpnt = rrcpnt;
					kenv.removeObject(becpnt);
				}
			}
		}
		for (CKObject* obj : kenv.levelObjects.getClassType<CKGrpPoolSquad>().objects) {
			CKGrpPoolSquad* pool = obj->cast<CKGrpPoolSquad>();
			if (pool->childHook.get())
				if (pool->childHook->getClassFullID() == CKHkBasicEnemy::FULL_ID)
					pool->childHook = hkmap[pool->childHook->cast<CKHkBasicEnemy>()];
		}
		for (auto& ent : hkmap) {
			if (ent.first)
				kenv.removeObject(ent.first);
		}
		//col->objs.clear();
		//col->objs2.clear();
		//col->bings.clear();
		//col->unk1 = 0;
		//col->unk2 = 0;
		kenv.levelObjects.getClassType<CKHkRocketRoman>().instantiation = kenv.levelObjects.getClassType<CKHkBasicEnemy>().instantiation;
		kenv.levelObjects.getClassType<CKHkBasicEnemy>().instantiation = KInstantiation::Globally;
		kenv.levelObjects.getClassType<CKRocketRomanCpnt>().instantiation = kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().instantiation;
		kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().instantiation = KInstantiation::Globally;
	}

	void GimmeX2Turtles(KEnvironment& kenv)
	{
		using namespace GameX2;
		using CKHkA2EnemyToConvert = CKHkA2InvincibleEnemy;
		using CKA2EnemyCpntToConvert = CKA2InvincibleEnemyCpnt;

		std::map<CKHkA2EnemyBase*, CKHkA2TurtleEnemy*> hookMap;
		for (CKObject* obj : kenv.levelObjects.getClassType<CKHkA2EnemyToConvert>().objects) {
			CKHkA2EnemyBase* hkBase = obj->cast<CKHkA2EnemyBase>();
			CKHkA2TurtleEnemy* hkTurtle = kenv.createObject<CKHkA2TurtleEnemy>(-1);

			// copy of base members
			*static_cast<CKHkA2EnemyBase*>(hkTurtle) = *hkBase;

			for (auto& nodeRef : hkTurtle->ckhaieUnk6) {
				auto* boundShape = nodeRef->cast<CKBoundingShape>();
				boundShape->object = hkTurtle;
			}

			for (auto& dynGroundRef : hkTurtle->ckhaieUnk4) {
				kenv.levelObjNames.getObjInfoRef(dynGroundRef.get()).user = hkTurtle;
			}

			kenv.levelObjNames.getObjInfoRef(hkTurtle->ckhaieUnk65.get()).user = hkTurtle;
			kenv.levelObjNames.getObjInfoRef(hkTurtle->node.get()).user2 = hkTurtle;

			hookMap[hkBase] = hkTurtle;
		}

		for (CKObject* obj : kenv.levelObjects.getClassType<CKHkA2EnemyToConvert>().objects) {
			CKHkA2EnemyBase* hkBase = obj->cast<CKHkA2EnemyBase>();
			CKHkA2TurtleEnemy* hkTurtle = hookMap.at(hkBase);

			if (hkTurtle->next)
				hkTurtle->next = hookMap.at(hkTurtle->next->cast<CKHkA2EnemyBase>());
			if (hkTurtle->x2next)
				hkTurtle->x2next = hookMap.at(hkTurtle->x2next->cast<CKHkA2EnemyBase>());
			hkBase->next = nullptr;
			hkBase->x2next = nullptr;
		}

		CKSrvCollision* col = kenv.levelObjects.getFirst<CKSrvCollision>();
		for (auto& ref : col->objs2)
			if (ref->getClassFullID() == CKHkA2EnemyToConvert::FULL_ID)
				ref = hookMap.at(ref->cast<CKHkA2EnemyBase>());

		for (CKObject* obj : kenv.levelObjects.getClassType<CKGrpPoolSquad>().objects) {
			CKGrpPoolSquad* pool = obj->cast<CKGrpPoolSquad>();
			if (pool->bundle->otherHook)
				if (pool->bundle->otherHook->getClassFullID() == CKHkA2EnemyToConvert::FULL_ID)
					pool->bundle->otherHook = hookMap.at(pool->bundle->otherHook->cast<CKHkA2EnemyBase>());
			if (pool->childHook.get())
				if (pool->childHook->getClassFullID() == CKHkA2EnemyToConvert::FULL_ID)
					pool->childHook = hookMap.at(pool->childHook->cast<CKHkA2EnemyBase>());
			for (auto& compRef : pool->components) {
				if (compRef->getClassFullID() == CKA2EnemyCpntToConvert::FULL_ID) {
					auto* oldCpnt = compRef->cast<GameX2::CKEnemyCpnt>();
					auto* turtleCpnt = kenv.createAndInitObject<CKA2TurtleEnemyCpnt>();
					*static_cast<GameX2::CKEnemyCpnt*>(turtleCpnt) = *oldCpnt;
					turtleCpnt->enemyDeadExplosionFxData = oldCpnt->cast<CKA2EnemyCpntToConvert>()->explosionFx1;
					compRef = turtleCpnt;
					kenv.removeObject(oldCpnt);
				}
			}
		}

		for (auto& entry : hookMap) {
			if (entry.first)
				kenv.removeObject(entry.first);
		}

		kenv.levelObjects.getClassType<CKHkA2TurtleEnemy>().instantiation = kenv.levelObjects.getClassType<CKHkA2EnemyToConvert>().instantiation;
		kenv.levelObjects.getClassType<CKHkA2EnemyToConvert>().instantiation = KInstantiation::Globally;
		kenv.levelObjects.getClassType<CKA2TurtleEnemyCpnt>().instantiation = kenv.levelObjects.getClassType<CKA2EnemyCpntToConvert>().instantiation;
		kenv.levelObjects.getClassType<CKA2EnemyCpntToConvert>().instantiation = KInstantiation::Globally;
	}

	void ConvertRomasterToOriginal(KEnvironment& kenv)
	{
		// Truncate CParticlesNodeFx for compatibility with original DONE
		// Only keep one index per animation slot in the AnimDictionary
		for (CKObject* obj : kenv.levelObjects.getClassType<CAnimationDictionary>().objects) {
			CAnimationDictionary* dict = obj->cast<CAnimationDictionary>();
			for (int i = 0; i < dict->numAnims; ++i)
				dict->animIndices[i] = dict->animIndices[2 * i];
			dict->animIndices.resize(dict->numAnims);
			dict->numSets = 1;
		}
		// Remove all events sent to parkour stele hooks
		if (CKSrvEvent* srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>()) {
			int ev = 0;
			for (auto& bee : srvEvent->sequences) {
				for (int j = 0; j < bee.numActions; j++) {
					if (CKObject* obj = srvEvent->objs[ev].get())
						if (obj->isSubclassOf<GameX1::CKHkParkourSteleAsterix>()) {
							srvEvent->objs.erase(srvEvent->objs.begin() + ev);
							srvEvent->objInfos.erase(srvEvent->objInfos.begin() + ev);
							bee.numActions -= 1;
							j -= 1; // Don't increment j for next iteration
							ev -= 1;
						}
					ev++;
				}
			}
		}
		// remove the parkour hooks
		if (auto* grpMeca = kenv.levelObjects.getFirst<GameX1::CKGrpMeca>()) {
			CKHook* prev = nullptr, * next;
			for (CKHook* hook = grpMeca->childHook.get(); hook; hook = next) {
				next = hook->next.get();
				if (hook->isSubclassOf<GameX1::CKHkParkourSteleAsterix>()) {
					if (prev)
						prev->next = hook->next;
					else // meaning the parkour hook is the first child of the group
						grpMeca->childHook = hook->next;
					auto* life = hook->life.get();

					// remove life
					CKBundle* bundle = grpMeca->bundle.get();
					CKHookLife* prevlife = nullptr, * nextlife;
					for (CKHookLife* cndpnt = bundle->firstHookLife.get(); cndpnt; cndpnt = nextlife) {
						nextlife = cndpnt->nextLife.get();
						if (cndpnt == life) {
							if (prevlife)
								prevlife->nextLife = cndpnt->nextLife;
							else
								bundle->firstHookLife = cndpnt->nextLife;
							hook->life = nullptr;
							kenv.removeObject(life);
							break;
						}
						else
							prevlife = cndpnt;
					}

					kenv.removeObject(hook);
				}
				else
					prev = hook;
			}
		}
		// remove romaster-specific cinematic nodes, substitute them with NOP ones
		for (CKObject* obj : kenv.levelObjects.getClassType<CKCinematicScene>().objects) {
			CKCinematicScene* scene = obj->cast<CKCinematicScene>();
			for (auto& noderef : scene->cineNodes) {
				CKCinematicNode* node = noderef.get();
				if (node->isSubclassOf<CKPauseCinematicBloc>() || node->isSubclassOf<CKTeleportCinematicBloc>()) {
					CKCinematicBloc* bloc = node->cast<CKCinematicBloc>();
					CKStartEventCinematicBloc* sub = kenv.createObject<CKStartEventCinematicBloc>(-1);
					*(CKCinematicBloc*)sub = *bloc;
					noderef = sub;
					kenv.removeObject(node);
				}
				else if (node->isSubclassOf<CKEndDoor>()) {
					CKCinematicDoor* door = node->cast<CKCinematicDoor>();
					CKLogicalAnd* sub = kenv.createObject<CKLogicalAnd>(-1);
					*(CKCinematicDoor*)sub = *door;
					noderef = sub;
					kenv.removeObject(node);
				}
			}
		}
		// remove remaining romaster-specific cinematic nodes that were not referenced
		for (int clid : {CKPauseCinematicBloc::FULL_ID, CKTeleportCinematicBloc::FULL_ID, CKEndDoor::FULL_ID}) {
			auto& cls = kenv.levelObjects.getClassType(clid);
			auto veccopy = cls.objects;
			for (CKObject* obj : veccopy)
				kenv.removeObject(obj);
			cls.instantiation = KInstantiation::Globally;
		}
		// shorten class list for hooks + logic misc
		kenv.levelObjects.categories[CKHook::CATEGORY].type.resize(208);
		kenv.levelObjects.categories[CKLogic::CATEGORY].type.resize(133);
		for (auto& str : kenv.sectorObjects) {
			str.categories[CKHook::CATEGORY].type.resize(208);
			str.categories[CKLogic::CATEGORY].type.resize(133);
		}
		kenv.isRemaster = false;
	}

	void ConvertXXL2HDToOriginal(KEnvironment& kenv, CKGameState* startState)
	{
		// Recreating CKParticleGeometry objects
		RwMiniClump rclump;
		IOFile file("X2HD2O_ParticleClump.rws", "rb");
		rclump.deserialize(&file);
		file.close();
		auto getStrObjects = [&](int str) -> KObjectList& {return (str == -1) ? kenv.levelObjects : kenv.sectorObjects[str]; };
		for (int str = -1; str < (int)kenv.numSectors; ++str) {
			std::set<CKParticleGeometry*> pgeoTreated;
			for (auto& cls : getStrObjects(str).categories[CKSceneNode::CATEGORY].type) {
				for (CKObject* obj : cls.objects) {
					if (CNode* node = obj->dyncast<CNode>()) {
						if (node->geometry) {
							CKParticleGeometry* pgeo = node->geometry->dyncast<CKParticleGeometry>();
							if (pgeo) {
								pgeoTreated.insert(pgeo);
								*pgeo = {};
								if (CParticlesNodeFx* pfx = node->dyncast<CParticlesNodeFx>()) {
									pgeo->flags = 0x410e;
									pgeo->color = 0xFFFFFFFF;
									pgeo->x2Head1 = 0;
									pgeo->clump = std::make_shared<RwMiniClump>(rclump);
								}
								else if (CGlowNodeFx* glow = node->dyncast<CGlowNodeFx>()) {
									pgeo->flags = 0x610E;
									pgeo->color = 0xFFFFFFFF;
									pgeo->x2Head1 = 0;
									pgeo->pgHead1 = 0x10080027;
									pgeo->pgHead2 = glow->cgnfUnk0;
									pgeo->pgHead3 = glow->cgnfUnk0;
									pgeo->x2TexName = glow->cgnfUnk4;
								}
								else if (CFogBoxNodeFx* fog = node->dyncast<CFogBoxNodeFx>()) {
									pgeo->flags = 0x4610E;
									pgeo->color = 0xFFFFFFFF;
									pgeo->x2Head1 = 0;
									pgeo->pgHead1 = 0x10080003;
									pgeo->pgHead2 = fog->fogUnk10;
									pgeo->pgHead3 = fog->fogUnk09;
									pgeo->x2TexName = fog->fogUnk02;
								}
								else if (CCloudsNodeFx* cloud = node->dyncast<CCloudsNodeFx>()) {
									pgeo->flags = 0x460A0;
									pgeo->color = 0xFFFFFFFF;
									pgeo->x2Head1 = 0x20000;
									pgeo->pgHead1 = 0x10000087;
									pgeo->pgHead2 = 0;
									pgeo->pgHead3 = 1000;
									pgeo->x2TexName = "a_sfx_nuages01";
								}
								else {
									fmt::println("Non-FX node: {}", kenv.getObjectName(node));
									// the shadow node likely
									pgeo->flags = 0x460AA;
									pgeo->color = 0xFFFFFFFF;
									pgeo->x2Head1 = 0x20000;
									pgeo->pgHead1 = 0x1000008A;
									pgeo->pgHead2 = 0;
									pgeo->pgHead3 = 30;
									pgeo->x2TexName = "a_sfx_ombre";
								}
							}
						}
					}
				}
			}
			fmt::println("STR {} Particle geos treated: {} / {}", str, pgeoTreated.size(), getStrObjects(str).getClassType<CKParticleGeometry>().objects.size());
		}

		// Assigning materials
		for (int str = -1; str < (int)kenv.numSectors; ++str) {
			std::map<CKParticleGeometry*, CMaterial*> pgeosWithMat;
			for (CKObject* obj : getStrObjects(str).getClassType<CMaterial>().objects) {
				CMaterial* mat = obj->cast<CMaterial>();
				if (mat->geometry && mat->geometry->isSubclassOf<CKParticleGeometry>()) {
					pgeosWithMat[mat->geometry->cast<CKParticleGeometry>()] = mat;
				}
			}
			int treated = 0;
			for (CKObject* obj : getStrObjects(str).getClassType<CKParticleGeometry>().objects) {
				CKParticleGeometry* pgeo = obj->cast<CKParticleGeometry>();
				if (pgeosWithMat.count(pgeo)) {
					pgeo->material = pgeosWithMat.at(pgeo);
					treated += 1;
				}
			}
			fmt::println("STR {} Materials treated: {} / {}", str, treated, pgeosWithMat.size());
		}

		// Copy CKInput data from original
		IOFile inpDataFile("X2HD2O_GAME_CKInput.bin", "rb");
		inpDataFile.seek(0, SEEK_END);
		auto inpSize = inpDataFile.tell();
		inpDataFile.seek(0, SEEK_SET);
		CKInput* kinput = kenv.getGlobal<CKInput>();
		kinput->data.resize(inpSize);
		inpDataFile.read(kinput->data.data(), kinput->data.size());
		inpDataFile.close();

		// Set the initial game state when launching the game
		GameX2::CKA2GameStructure* gameStruct = kenv.getGlobal<GameX2::CKA2GameStructure>();
		gameStruct->someGameState = startState;

		kenv.isRemaster = false;
		kenv.isXXL2Demo = false;
	}
}

void EditorUI::IGMisc(EditorInterface& ui)
{
#ifndef XEC_RELEASE
	ImGui::Checkbox("Show ImGui Demo", &ui.showImGuiDemo);
#endif
	auto& kenv = ui.kenv;
	if (kenv.version == kenv.KVERSION_XXL1) {
		if (ImGui::Button("Rocket Romans \\o/"))
			GimmeTheRocketRomans(kenv);
		ImGui::SetItemTooltip("Transform all Basic Enemies to Rocket Romans");
	}
	if (kenv.version == kenv.KVERSION_XXL2) {
		if (ImGui::Button("Turtles! ^o^"))
			GimmeX2Turtles(kenv);
		ImGui::SetItemTooltip("Transform all Basic XXL2 Enemies to Turtle Enemies");
	}

	if (kenv.version == kenv.KVERSION_XXL1 && kenv.isRemaster && ImGui::Button("Convert Romaster -> Original")) {
		ConvertRomasterToOriginal(kenv);
	}

	static kobjref<CKGameState> startState = nullptr;
	if (kenv.version == KEnvironment::KVERSION_XXL2) {
		if (ImGui::Button("Convert XXL2 HD -> Original")) {
			ConvertXXL2HDToOriginal(kenv, startState.get());
		}
		IGObjectSelectorRef(ui, "Start state", startState);
	}

	if (ImGui::Button("Reload Encyclopedia"))
		ui.g_encyclo.clear();

	if (ImGui::Button("Save GAME file"))
		kenv.saveGameFile();

	if (ImGui::CollapsingHeader("Ray Hits")) {
		ImGui::Columns(2);
		for (auto& hit : ui.rayHits) {
			ImGui::BulletText("%f", (ui.camera.position - hit->hitPosition).len3());
			ImGui::NextColumn();
			// TODO REFACTOR
			//if (hit->is<NodeSelection>()) {
			//	NodeSelection* ns = (NodeSelection*)hit.get();
			//	ImGui::Text("%i %p %s", hit->getTypeID(), ns->node, ns->node->getClassName());
			//}
			//else
				ImGui::Text("%i", hit->getTypeID());
			ImGui::NextColumn();
		}
		ImGui::Columns();
	}
	if (ImGui::CollapsingHeader("Unknown classes")) {
		for (auto& cl : CKUnknown::hits) {
			ImGui::BulletText("%i %i", cl.first, cl.second);
		}
	}
}
