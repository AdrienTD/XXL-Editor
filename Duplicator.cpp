#include "Duplicator.h"
#include "KEnvironment.h"
#include "ClassRegister.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"
#include "EditorInterface.h"
#include <filesystem>
#include "GameClasses/CKGameX1.h"
#include "CoreClasses/CKManager.h"
#include <span>

using namespace GameX1; // TEMP

static constexpr std::array g_singletonFidsX1 = {
	CKHkWaterFx::FULL_ID,
	CKHkAsterix::FULL_ID,
	CKHkObelix::FULL_ID,
	CKHkIdefix::FULL_ID,
	CKSrvCollision::FULL_ID,
	CKSrvCamera::FULL_ID,
	CKServiceLife::FULL_ID,
	CKGrpTrio::FULL_ID,
};

static constexpr std::array g_singletonFidsX2 = {
	CKSrvCollision::FULL_ID,
	CKSrvCamera::FULL_ID,
	CKServiceLife::FULL_ID,
};

static std::span<const int> getSingletonFids(int version) {
	if (version == KEnvironment::KVERSION_XXL1)
		return std::span(g_singletonFidsX1);
	else
		return std::span(g_singletonFidsX2);
}

static constexpr int g_toNullifyFids[] = {
	CKGrpBaseSquad::FULL_ID
};

CKSceneNode* Duplicator::cloneNode(CKSceneNode* original, bool recursive)
{
	printf("Cloning node %s\n", original->getClassName());
	CKSceneNode* clone = cloneWrap(original, -1);
	cloneMap[original] = clone;
	clone->next = clone->parent = nullptr;

	CSGBranch* oSGB = original->dyncast<CSGBranch>();
	CSGBranch* dSGB = clone->dyncast<CSGBranch>();
	if (oSGB) {
		dSGB->child = nullptr;
	}

	CNode* oNode = original->dyncast<CNode>();
	CNode* dNode = clone->dyncast<CNode>();
	if (oNode && oNode->geometry) {
		CKAnyGeometry* prev = nullptr;
		for (CKAnyGeometry* ogeo = oNode->geometry.get(); ogeo; ogeo = ogeo->nextGeo.get()) {
			CKAnyGeometry* dgeo = cloneWrap(ogeo, -1);
			if (ogeo->material) {
				dgeo->material = cloneWrap(ogeo->material.get(), -1);
				dgeo->material->geometry = dgeo;
			}
			if (prev)
				prev->nextGeo = dgeo;
			else
				dNode->geometry = dgeo;
			prev = dgeo;
		}
	}

	CSGBranch* oClone = original->dyncast<CSGBranch>();
	CSGBranch* dClone = clone->dyncast<CSGBranch>();
	if (oClone && (oClone->isSubclassOf<CClone>() || oClone->isSubclassOf<CAnimatedClone>())) {
	}

	if (CKBoundingShape* dShape = clone->dyncast<CKBoundingShape>()) {
		CKBoundingShape* oShape = original->cast<CKBoundingShape>();
		if (oShape->object)
			dShape->object = cloneMap.at(oShape->object.get());
	}

	if (recursive) {
		CSGBranch* oBranch = original->dyncast<CSGBranch>();
		CSGBranch* dBranch = clone->dyncast<CSGBranch>();
		if (oBranch && oBranch->child) {
			CKSceneNode* prev = nullptr;
			for (CKSceneNode* sub = oBranch->child.get(); sub; sub = sub->next.get()) {
				CKSceneNode* subcopy = cloneNode(sub, true);
				subcopy->parent = dBranch;
				if (prev)
					prev->next = subcopy;
				else
					dBranch->child = subcopy;
				prev = subcopy;
			}
		}

		CAnyAnimatedNode* oAnim = original->dyncast<CAnyAnimatedNode>();
		CAnyAnimatedNode* dAnim = clone->dyncast<CAnyAnimatedNode>();
		if (oAnim && oAnim->branchs) {
			CKSceneNode* prev = nullptr;
			for (CKSceneNode* sub = oAnim->branchs.get(); sub; sub = sub->next.get()) {
				CKSceneNode* subcopy = cloneNode(sub, true);
				subcopy->parent = dAnim;
				if (prev)
					prev->next = subcopy;
				else
					dAnim->branchs = subcopy;
				prev = subcopy;
			}
		}
	}

	return clone;
}

CKGroup* Duplicator::findGroup(CKHook* hook, CKGroup* root)
{
	for (CKHook* cand = root->childHook.get(); cand; cand = cand->next.get()) {
		if (cand == hook)
			return root;
	}
	for (CKGroup* subgrp = root->childGroup.get(); subgrp; subgrp = subgrp->nextGroup.get()) {
		if (CKGroup* cand = findGroup(hook, subgrp))
			return cand;
	}
	return nullptr;
}

void Duplicator::reflectAnyRef(kanyobjref& ref, int clfid, const char* name)
{
	if ((int)currentFlags & (int)MemberFlags::MF_DUPLICATOR_IGNORED)
		return;
	if (!ref)
		return;
	CKObject* cloned = nullptr;
	clfid = ref.get()->getClassFullID();
	if (auto it = cloneMap.find(ref.get()); it != cloneMap.end()) {
		cloned = it->second;
		printf("for kobjref<(%i,%i)> %s the object %s is already cloned\n", clfid & 63, clfid >> 6, name, ref.get()->getClassName());
	}
	else {
		static constexpr int compositions[] = {
			CAnimationDictionary::FULL_ID,
			CKShadowCpnt::FULL_ID,
			CKLine::FULL_ID,
			CKSpline4::FULL_ID,
			CKSpline4L::FULL_ID
		};
		const auto& singletons = getSingletonFids(srcEnv->version);
		if (std::find(std::begin(compositions), std::end(compositions), clfid) != std::end(compositions)) {
			cloned = cloneWrap(ref.get());
		}
		else if (clfid == CKSoundDictionaryID::FULL_ID) {
			cloned = cloneWrap(ref.get());
			if (srcEnv->version >= KEnvironment::KVERSION_XXL2) {
				for (auto& ref : cloned->cast<CKSoundDictionaryID>()->x2Sounds) {
					CKSound* oldSound = ref->cast<CKSound>();
					ref = cloneWrap(oldSound);
					ref->cast<CKSound>()->sndSector = 0;
					cloneMap[oldSound] = ref.get();
				}
				cloned->cast<CKSoundDictionaryID>()->x2Sector = 0;
			}
		}
		else if ((clfid & 63) == CKSceneNode::CATEGORY) {
			CKSceneNode* node = cloneNode((CKSceneNode*)ref.get(), true);
			CSGSectorRoot* sroot = destEnv->levelObjects.getFirst<CSGSectorRoot>();
			sroot->insertChild(node);
			cloned = node;
		}
		else if (clfid == CGround::FULL_ID || clfid == CDynamicGround::FULL_ID) {
			auto addToMeshKluster = [](KObjectList& objlist, CGround* ground) {
				CKMeshKluster* kluster = objlist.getFirst<CKMeshKluster>();
				if(kluster)
					kluster->grounds.emplace_back(ground);
			};
			CGround* dGround = (CGround*)cloneWrap(ref.get());
			cloned = dGround;
			if (clfid == CDynamicGround::FULL_ID) {
				CDynamicGround* dDynGround = dGround->dyncast<CDynamicGround>();
				if (dDynGround->node)
					dDynGround->node = (CKSceneNode*)cloneMap.at(dDynGround->node.get());
			}
			addToMeshKluster(destEnv->levelObjects, dGround);
			for (auto& str : destEnv->sectorObjects)
				addToMeshKluster(str, dGround);
		}
		else if (CKProjectileTypeBase* ptb = ref.get()->dyncast<CKProjectileTypeBase>()) {
			if (destEnv != srcEnv) {
				cloned = cloneWrap(ref.get());
				cloned->cast<CKProjectileTypeBase>()->virtualReflectMembers(*this, destEnv);
			}
		}
		else if (clfid == CKFlaggedPath::FULL_ID) {
			cloned = cloneWrap(ref.get());
			auto& kref = cloned->cast<CKFlaggedPath>()->line;
			kref = cloneWrap(kref.get());
		}
		else if (clfid == CSGHotSpot::FULL_ID) {
			CSGHotSpot* hotSpot = (CSGHotSpot*)cloneWrap(ref.get());
			cloned = hotSpot;
			hotSpot->csghsUnk0 = (CKSceneNode*)cloneMap.at(ref.get()->cast<CSGHotSpot>()->csghsUnk0.get());
		}
		else if (std::find(std::begin(singletons), std::end(singletons), clfid) != std::end(singletons)) {
			cloned = destEnv->levelObjects.getClassType(clfid).objects[0];
		}
		else if (std::find_if(std::begin(g_toNullifyFids), std::end(g_toNullifyFids), [&ref](int fid) { return ref.get()->isSubclassOfID((uint32_t)fid); }) != std::end(g_toNullifyFids)) {
			cloned = nullptr;
			ref.anyreset(nullptr);
		}
		else {
			printf("!! I don't know what to do with kobjref<(%i,%i)> %s containing %s\n", clfid & 63, clfid >> 6, name, ref ? ref.get()->getClassName() : "nothing");
		}
	}
	if (cloned) {
		cloneMap[ref.get()] = cloned;
		ref.anyreset(cloned);
	}
}

void Duplicator::reflect(uint8_t& ref, const char* name) {}
void Duplicator::reflect(uint16_t& ref, const char* name) {}
void Duplicator::reflect(uint32_t& ref, const char* name) {}
void Duplicator::reflect(float& ref, const char* name) {}
void Duplicator::reflect(Vector3& ref, const char* name) {}
void Duplicator::reflect(EventNode& ref, const char* name, CKObject* user) {}
void Duplicator::reflect(MarkerIndex& ref, const char* name) {}
void Duplicator::reflect(std::string& ref, const char* name) {}

void Duplicator::setNextFlags(MemberFlags flags) { currentFlags = flags; }

void Duplicator::doClone(CKObject* object)
{
	srcEnv = &kenv;
	destEnv = &kenv;
	cloneFunction = [this](CKObject* obj, int sector) -> CKObject* {
		CKObject* clone = kenv.cloneObject(obj, sector);
		if (CNode* dClone = clone->dyncast<CNode>()) {
			if (dClone->isSubclassOf<CClone>() || dClone->isSubclassOf<CAnimatedClone>()) {
				CCloneManager* mgr = kenv.levelObjects.getFirst<CCloneManager>();
				CCloneManager* exmgr = mgr;
				int clIndex = 0;
				auto it = std::find_if(mgr->_clones.begin(), mgr->_clones.end(), [obj](auto& ref) {return ref.get() == obj; });
				assert(it != mgr->_clones.end());
				clIndex = it - mgr->_clones.begin();
				int dupIndex = exmgr->_numClones;
				exmgr->_clones.emplace_back(dClone);
				exmgr->flinfos.emplace_back(mgr->flinfos[clIndex]);
				exmgr->_team.dongs.emplace_back(mgr->_team.dongs[clIndex]);
				exmgr->_numClones += 1;
				ui->nodeCloneIndexMap[dClone] = dupIndex;
			}
		}
		return clone;
	};
	CKObject* clonedHook = doCommon(object);

	if (CKHook* hook = object->dyncast<CKHook>()) {
		CKGroup* group = Duplicator::findGroup(hook, kenv.levelObjects.getFirst<CKGroupRoot>());
		assert(group);
		group->addHook(clonedHook->dyncast<CKHook>());
	}
}

void Duplicator::doExport(CKObject* object, const std::filesystem::path& path)
{
	KEnvironment copyenv = KFab::makeSimilarKEnv(kenv);
	CSGBranch* copyNodeRoot = copyenv.createAndInitObject<CSGSectorRoot>(-1);
	copyenv.createAndInitObject<CKSrvEvent>(-1);
	copyenv.createAndInitObject<CKSoundDictionary>();
	copyenv.createAndInitObject<CTextureDictionary>();
	CCloneManager* cloneMgr = copyenv.createAndInitObject<CCloneManager>();
	cloneMgr->_team.numBongs = kenv.levelObjects.getFirst<CCloneManager>()->_team.numBongs;
	copyenv.createAndInitObject<CAnimationManager>();
	for (int fid : getSingletonFids(copyenv.version)) {
		const auto& srcClassType = kenv.levelObjects.getClassType(fid);
		if (srcClassType.objects.empty())
			continue;

		CKObject* stclone = copyenv.createObject(fid, -1);
		stclone->init(&copyenv);
		cloneMap[srcClassType.objects[0]] = stclone;
	}

	CKObject* clonedObject = doTransfer(object, &kenv, &copyenv);
	
	KFab::saveKFab(copyenv, clonedObject, path);
	copyenv.unloadGame();
}

void Duplicator::doImport(const std::filesystem::path& path, CKGroup* parentGroup)
{
	KEnvironment kfab;
	CKObject* mainObj = KFab::loadKFab(kfab, path);
		
	CKObject* clonedObj = doTransfer(mainObj, &kfab, &kenv);

	if (CKHook* clonedHook = clonedObj->dyncast<CKHook>()) {
		parentGroup->addHook(clonedHook);
	}
	else if (CKGroup* clonedGroup = clonedObj->dyncast<CKGroup>()) {
		parentGroup->addGroup(clonedGroup);
	}
}

CKHook* Duplicator::cloneHook(CKHook* hook)
{
	CKHook* clone = cloneWrap(hook);
	clone->next = nullptr;
	clone->x2next = nullptr;
	clone->x2Sector = 0;
	clone->activeSector = -1;
	cloneMap[hook] = clone;

	if (hook->node.get()) {
		CKSceneNode* clonedNode = cloneNode(hook->node.get(), true);
		CSGSectorRoot* sroot = destEnv->levelObjects.getFirst<CSGSectorRoot>();
		sroot->insertChild(clonedNode);
		cloneMap[hook->node.get()] = clonedNode;
		clone->node = clonedNode;
	}

	clone->virtualReflectMembers(*this, destEnv);

	CKHookLife* life = nullptr;
	if (hook->life) {
		life = cloneWrap(hook->life.get());
		life->unk1 = 0;
		life->hook = clone; life->nextLife = nullptr;
	}

	clone->life = life;
	clone->next = nullptr;
	clone->update();

	return clone;
}

CKGroup* Duplicator::cloneGroup(CKGroup* group)
{
	CKGroup* clone = cloneWrap(group);

	clone->nextGroup = nullptr;
	clone->parentGroup = nullptr;
	clone->life = nullptr;
	clone->bundle = nullptr;

	// clone grouplife
	if (group->life) {
		clone->life = cloneWrap(group->life.get());
		clone->life->group = clone;
	}

	// clone bundle
	bool hasEnabledHooks = false;
	bool hasDisabledHooks = false;
	if (group->bundle) {
		hasEnabledHooks = group->bundle->firstHookLife.get();
		hasDisabledHooks = group->bundle->otherHookLife.get();
		clone->bundle = cloneWrap(group->bundle.get());
		clone->bundle->next = nullptr;
		clone->bundle->grpLife = clone->life;
		clone->bundle->firstHookLife = nullptr;
		clone->bundle->otherHookLife = nullptr;

		CKServiceLife* srvLife = destEnv->levelObjects.getFirst<CKServiceLife>();
		clone->bundle->next = srvLife->firstBundle;
		srvLife->firstBundle = clone->bundle;
	}

	std::vector<CKGroup*> subGroups;
	std::vector<CKHook*> subHooks;
	for (CKGroup* subGroup = group->childGroup.get(); subGroup; subGroup = subGroup->nextGroup.get()) {
		subGroups.push_back(subGroup);
	}
	for (CKHook* subHook = group->childHook.get(); subHook; subHook = subHook->next.get()) {
		subHooks.push_back(subHook);
	}

	// erase the original children from the cloned group
	clone->childGroup = nullptr;
	clone->childHook = nullptr;

	// clone the children recursively
	for (CKGroup* subGroup : std::views::reverse(subGroups)) {
		CKGroup* clonedSubGroup = cloneGroup(subGroup);
		clone->addGroup(clonedSubGroup);
	}
	for (CKHook* subHook : std::views::reverse(subHooks)) {
		CKHook* clonedSubHook = cloneHook(subHook);
		clone->addHook(clonedSubHook);
	}

	clone->virtualReflectMembers(*this, destEnv);

	return clone;
}

CKObject* Duplicator::doCommon(CKObject* object)
{
	CKObject* clone = nullptr;
	if (CKHook* hook = object->dyncast<CKHook>())
		clone = cloneHook(hook);
	else if (CKGroup* group = object->dyncast<CKGroup>())
		clone = cloneGroup(group);

	if (srcEnv == destEnv) {
		// add collisions
		if (CKSrvCollision* srcCollision = srcEnv->levelObjects.getFirst<CKSrvCollision>()) {
			CKSrvCollision* destCollision = destEnv->levelObjects.getFirst<CKSrvCollision>();
			std::vector<CKSrvCollision::Bing> dupColls;
			std::map<uint16_t, uint16_t> clonedCollMap;
			auto substituteIfCloned = [this](CKObject* obj) {
				if (auto it = cloneMap.find(obj); it != cloneMap.end()) {
					return it->second;
				}
				return obj;
			};
			uint16_t index = 0;
			for (auto& coll : srcCollision->bings) {
				// for now, skip collisions with the boss, when exporting hooks other than the boss itself.
				//if (!hook->isSubclassOf<CKHkBoss>() && (
				//	(coll.b1 != 0xFFFF && srcCollision->objs2[coll.b1]->isSubclassOf<CKHkBoss>()) ||
				//	(coll.b2 != 0xFFFF && srcCollision->objs2[coll.b2]->isSubclassOf<CKHkBoss>())))
				//{
				//	continue;
				//}
				if (cloneMap.count(coll.obj1.get()) || cloneMap.count(coll.obj2.get())) {
					uint16_t cIndex = (uint16_t)dupColls.size();
					auto& dup = dupColls.emplace_back(coll);
					dup.obj1 = substituteIfCloned(dup.obj1.get());
					dup.obj2 = substituteIfCloned(dup.obj2.get());
					if (coll.b1 != 0xFFFF) {
						CKObject* hand1 = substituteIfCloned(srcCollision->objs2[coll.b1].get());
						dup.b1 = destCollision->addOrGetHandler(hand1);
					}
					if (coll.b2 != 0xFFFF) {
						CKObject* hand2 = substituteIfCloned(srcCollision->objs2[coll.b2].get());
						dup.b2 = destCollision->addOrGetHandler(hand2);
					}
					dup.aa[0] = 0xFFFF;
					dup.aa[1] = 0xFFFF;
					clonedCollMap[index] = cIndex;
				}
				++index;
			}
			uint16_t originalSize = (uint16_t)destCollision->bings.size();
			for (auto& dup : dupColls) {
				if (dup.aa[2] != 0xFFFF && clonedCollMap.count(dup.aa[2])) {
					dup.aa[2] = originalSize + clonedCollMap.at(dup.aa[2]);
				}
				dup.v1 &= 0xF; // disable it
				uint16_t added = (uint16_t)destCollision->bings.size();
				dup.aa[0] = destCollision->inactiveList;
				destCollision->inactiveList = added;
				destCollision->bings.push_back(std::move(dup));
			}
		}

		//// For animated characters, add their cloned hook to CKLevel's list if it was there
		//if (CKHkAnimatedCharacter* animChar = clone->dyncast<CKHkAnimatedCharacter>()) {
		//	CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
		//	if (std::find_if(level->objs.begin(), level->objs.end(), [hook](auto& ref) {return ref.get() == hook; }) != level->objs.end())
		//		level->objs.emplace_back(clone);
		//}
	}

	// fix sound dict references
	for (auto& [originalObj, clonedObj] : cloneMap) {
		if (CKSoundDictionaryID* snddict = clonedObj->dyncast<CKSoundDictionaryID>()) {
			for (auto& entry : snddict->soundEntries) {
				if (entry.obj) {
					entry.obj = cloneMap.at(entry.obj.get());
				}
			}
		}
		if (CKSound* snd = clonedObj->dyncast<CKSound>()) {
			if (auto* ref = std::get_if<1>(&snd->sndPosition)) {
				if (ref->get())
					*ref = (CKSceneNode*)cloneMap.at(ref->get());
			}
		}
	}

	return clone;
}


CKObject* Duplicator::doTransfer(CKObject* object, KEnvironment* _srcEnv, KEnvironment* _destEnv)
{
	srcEnv = _srcEnv;
	destEnv = _destEnv;
	cloneFunction = [this](CKObject* obj, int sector) -> CKObject* {
		uint32_t fid = obj->getClassFullID();

		auto& ktype = destEnv->levelObjects.getClassType(fid);
		if (ktype.objects.empty() && ktype.info == 0) {
			auto& ftype = srcEnv->levelObjects.getClassType(fid);
			ktype.info = ftype.info;
		}

		CKObject* copy = destEnv->createObject(fid, -1);
		destEnv->factories.at(fid).copy(obj, copy);

		if (CKSoundDictionaryID* ids = copy->dyncast<CKSoundDictionaryID>()) {
			CKSoundDictionary* odict = srcEnv->levelObjects.getFirst<CKSoundDictionary>();
			CKSoundDictionary* cdict = destEnv->levelObjects.getFirst<CKSoundDictionary>();
			for (auto& ent : ids->soundEntries) {
				uint32_t id = ent.id;
				uint32_t nextid = (uint32_t)cdict->sounds.size();
				cdict->sounds.push_back(odict->sounds[id]);
				cdict->rwSoundDict.list.sounds.push_back(odict->rwSoundDict.list.sounds[id]);
				ent.id = nextid;
			}
		}

		if (CKSound* snd = copy->dyncast<CKSound>()) {
			bool slotActive = (snd->sndIndex & 0xFFFFFF) != 0xFFFFFF;
			if (slotActive) {
				int sectorIndex = snd->sndIndex >> 24;
				int soundId = snd->sndIndex & 0xFFFFFF;

				CKSoundDictionary* odict = srcEnv->getObjectList(sectorIndex - 1).getFirst<CKSoundDictionary>();
				CKSoundDictionary* cdict = destEnv->levelObjects.getFirst<CKSoundDictionary>();
				uint32_t nextid = (uint32_t)cdict->sounds.size();
				cdict->sounds.push_back(odict->sounds[soundId]);
				cdict->rwSoundDict.list.sounds.push_back(odict->rwSoundDict.list.sounds[soundId]);
				snd->sndIndex = nextid;
			}
		}

		if (CNode* node = copy->dyncast<CNode>()) {
			if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
				CCloneManager* mgr = srcEnv->levelObjects.getFirst<CCloneManager>();
				CCloneManager* exmgr = destEnv->levelObjects.getFirst<CCloneManager>();
				int clIndex = 0;
				auto it = std::find_if(mgr->_clones.begin(), mgr->_clones.end(), [obj](auto& ref) {return ref.get() == obj; });
				assert(it != mgr->_clones.end());
				clIndex = it - mgr->_clones.begin();
				int dupIndex = exmgr->_numClones;
				exmgr->_clones.emplace_back(node);
				exmgr->flinfos.emplace_back(mgr->flinfos[clIndex]);
				auto& dong = exmgr->_team.dongs.emplace_back(mgr->_team.dongs[clIndex]);
				exmgr->_numClones += 1;
				ui->nodeCloneIndexMap[node] = dupIndex;
				// export teamdict things
				for (uint32_t& tde : dong.bongs) {
					if (tde != -1) {
						auto& val2 = mgr->_teamDict._bings[tde];
						uint32_t nextid = (uint32_t)exmgr->_teamDict._bings.size();
						exmgr->_teamDict._bings.push_back(val2);
						tde = nextid;
					}
				}
			}

			CTextureDictionary* cdict = destEnv->levelObjects.getFirst<CTextureDictionary>();
			for (CKAnyGeometry* geo = node->geometry.get(); geo; geo = geo->nextGeo.get()) {
				if (!geo->clump) continue;
				RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
				if (!rwgeo) continue;
				if (rwgeo->materialList.materials.empty()) continue;
				const std::string& name = rwgeo->materialList.materials.at(0).texture.name;
				// if src dict already has texture, continue
				if (cdict->piDict.findTexture(name) != -1)
					continue;
				// find texture in dest dict
				CTextureDictionary* odict = srcEnv->levelObjects.getFirst<CTextureDictionary>();
				size_t tid = odict->piDict.findTexture(name);
				size_t str = 0;
				while (tid == -1 && str < (size_t)srcEnv->numSectors) {
					odict = srcEnv->sectorObjects[str++].getFirst<CTextureDictionary>();
					tid = odict->piDict.findTexture(name);
				}
				// if dest dict has texture, copy
				if (tid != -1) {
					cdict->piDict.textures.push_back(odict->piDict.textures[tid]);
				}
			}
		}

		if (CAnimationDictionary* cAnimDict = copy->dyncast<CAnimationDictionary>()) {
			CAnimationManager* oAnimMgr = srcEnv->levelObjects.getFirst<CAnimationManager>();
			CAnimationManager* cAnimMgr = destEnv->levelObjects.getFirst<CAnimationManager>();
			for (uint32_t& ind : cAnimDict->animIndices) {
				if (ind != -1) {
					uint32_t nextIndex = (uint32_t)cAnimMgr->commonAnims.anims.size();
					cAnimMgr->commonAnims.anims.push_back(oAnimMgr->commonAnims.anims[ind]);
					ind = nextIndex;
				}
			}
		}

		if (CKProjectileTypeBase* projectileType = copy->dyncast<CKProjectileTypeBase>()) {
			auto* srvProjectiles = destEnv->levelObjects.getFirst<CKSrvProjectiles>();
			if (srvProjectiles) {
				srvProjectiles->projectiles.push_back(projectileType);
			}
		}

		return copy;
	};
	return doCommon(object);
}

void KFab::saveKFab(KEnvironment& kfab, CKObject* mainObj, const std::filesystem::path& path)
{
	IOFile file{ path.c_str(), "wb" };
	file.writeString("XEC-HOOK");
	file.writeInt32(0); // file version
	file.writeInt32(0); // flags
	file.writeInt32(kfab.version);
	file.writeInt32(kfab.platform);
	file.writeInt32(kfab.isRemaster);

	for (auto& cat : kfab.levelObjects.categories) {
		file.writeUint16((uint16_t)cat.type.size());
		for (auto& kcl : cat.type) {
			file.writeUint16(kcl.totalCount);
			file.writeUint16((uint16_t)kcl.objects.size());
			file.writeUint8(kcl.info);
		}
	}

	kfab.prepareSavingMap();

	kfab.writeObjID(&file, mainObj);

	for (int clcat = 0; clcat < 15; ++clcat) {
		for (size_t clid = 0; clid < kfab.levelObjects.categories[clcat].type.size(); clid++) {
			auto& cltype = kfab.levelObjects.categories[clcat].type[clid];
			for (CKObject* obj : cltype.objects) {
				uint32_t noo = (uint32_t)file.tell();
				file.writeUint32(0);
				obj->serialize(&kfab, &file);
				uint32_t eoo = (uint32_t)file.tell();
				file.seek(noo, SEEK_SET);
				file.writeInt32(eoo);
				file.seek(eoo, SEEK_SET);
			}
		}
	}
}

CKObject* KFab::loadKFab(KEnvironment& kfab, const std::filesystem::path& path)
{
	IOFile file{ path.c_str(), "rb" };
	auto header = file.readString(8);
	assert(header == "XEC-HOOK");
	int fileVersion = file.readInt32();
	int flags = file.readInt32();
	int gameVersion = file.readInt32();
	int gamePlatform = file.readInt32();
	int gameIsRemaster = file.readInt32();
	kfab.version = gameVersion;
	kfab.platform = gamePlatform;
	kfab.isRemaster = gameIsRemaster != 0;

	ClassRegister::registerClasses(kfab, gameVersion, gamePlatform, gameIsRemaster != 0);

	int catid = 0;
	for (auto& cat : kfab.levelObjects.categories) {
		int clid = 0;
		cat.type.resize(file.readUint16());
		for (auto& kcl : cat.type) {
			kcl.totalCount = file.readUint16();
			kcl.objects.resize(file.readUint16());
			for (auto& obj : kcl.objects)
				obj = kfab.constructObject(catid, clid);
			kcl.info = file.readUint8();
			++clid;
		}
		++catid;
	}

	CKObject* mainObject = kfab.readObjPnt(&file);

	for (int clcat = 0; clcat < 15; ++clcat) {
		for (size_t clid = 0; clid < kfab.levelObjects.categories[clcat].type.size(); clid++) {
			auto& cltype = kfab.levelObjects.categories[clcat].type[clid];
			for (CKObject* obj : cltype.objects) {
				uint32_t nextObjOffset = file.readUint32();
				obj->deserialize(&kfab, &file, nextObjOffset - file.tell());
				obj->onLevelLoaded(&kfab);
				obj->onLevelLoaded2(&kfab);
				assert(file.tell() == nextObjOffset);
			}
		}
	}

	for (auto& cat : kfab.levelObjects.categories)
		for (auto& kcl : cat.type)
			for (CKObject* obj : kcl.objects)
				obj->onLevelLoaded(&kfab);
	for (auto& cat : kfab.levelObjects.categories)
		for (auto& kcl : cat.type)
			for (CKObject* obj : kcl.objects)
				obj->onLevelLoaded2(&kfab);

	return mainObject;
}

KEnvironment KFab::makeSimilarKEnv(const KEnvironment& kenv)
{
	KEnvironment kfab;
	ClassRegister::registerClasses(kfab, kenv.version, kenv.platform, kenv.isRemaster);
	kfab.loadGame(kenv.gamePath.c_str(), kenv.version, kenv.platform, kenv.isRemaster);
	kfab.outGamePath = "C:\\Users\\Adrien\\Desktop\\kthings\\xecexport";
	kfab.numSectors = 0;
	for (size_t cat = 0; cat < 15; ++cat) {
		auto& kCats = kenv.levelObjects.categories[cat];
		auto& cCats = kfab.levelObjects.categories[cat];
		cCats.type.resize(kCats.type.size());
		for (size_t cl = 0; cl < kCats.type.size(); ++cl) {
			const KObjectList::ClassType& kty = kCats.type[cl];
			KObjectList::ClassType tty;
			tty.startId = 0;
			tty.totalCount = 0;
			tty.info = kty.info;
			cCats.type[cl] = std::move(tty);
		}
	}
	return kfab;
}
