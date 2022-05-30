#include "Duplicator.h"
#include "KEnvironment.h"
#include "ClassRegister.h"
#include "CKService.h"
#include "CKHook.h"
#include "CKGroup.h"
#include "CKComponent.h"
#include "CKNode.h"
#include "CKDictionary.h"
#include "CKLogic.h"
#include "CKGraphical.h"
#include "EditorInterface.h"

CKSceneNode* HookMemberDuplicator::cloneNode(CKSceneNode* original, bool recursive)
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
		CCloneManager* mgr = kenv.levelObjects.getFirst<CCloneManager>();
		int clIndex = 0;
		auto it = std::find_if(mgr->_clones.begin(), mgr->_clones.end(), [oClone](auto& ref) {return ref.get() == oClone; });
		assert(it != mgr->_clones.end());
		clIndex = it - mgr->_clones.begin();
		int dupIndex = mgr->_numClones;
		mgr->_clones.emplace_back(dClone);
		mgr->flinfos.emplace_back(mgr->flinfos[clIndex]);
		mgr->_team.dongs.emplace_back(mgr->_team.dongs[clIndex]);
		mgr->_team.numDongs += 1;
		mgr->_numClones += 1;
		ui->nodeCloneIndexMap[dClone] = dupIndex;
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
					dAnim->branchs = subcopy->cast<CSGBranch>();
				prev = subcopy;
			}
		}
	}

	return clone;
}

CKGroup* HookMemberDuplicator::findGroup(CKHook* hook, CKGroup* root)
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

void HookMemberDuplicator::reflectAnyRef(kanyobjref& ref, int clfid, const char* name)
{
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
			CKSoundDictionaryID::FULL_ID,
			CAnimationDictionary::FULL_ID,
			CKShadowCpnt::FULL_ID
		};
		if (std::find(std::begin(compositions), std::end(compositions), clfid) != std::end(compositions)) {
			cloned = cloneWrap(ref.get());
		}
		else if ((clfid & 63) == CKSceneNode::CATEGORY) {
			CKSceneNode* node = cloneNode((CKSceneNode*)ref.get(), true);
			CSGSectorRoot* sroot = (exporting ? copyenv : kenv).levelObjects.getFirst<CSGSectorRoot>();
			sroot->insertChild(node);
			cloned = node;
		}
		else if (clfid == CGround::FULL_ID || clfid == CDynamicGround::FULL_ID) {
			auto addToMeshKluster = [](KObjectList& objlist, CGround* ground) {
				CKMeshKluster* kluster = objlist.getFirst<CKMeshKluster>();
				kluster->grounds.emplace_back(ground);
			};
			CGround* dGround = (CGround*)cloneWrap(ref.get());
			cloned = dGround;
			if (clfid == CDynamicGround::FULL_ID) {
				CDynamicGround* dDynGround = dGround->dyncast<CDynamicGround>();
				if (dDynGround->node)
					dDynGround->node = (CKSceneNode*)cloneMap.at(dDynGround->node.get());
			}
			addToMeshKluster(kenv.levelObjects, dGround);
			for (auto& str : kenv.sectorObjects)
				addToMeshKluster(str, dGround);
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

void HookMemberDuplicator::reflect(uint8_t& ref, const char* name) {}
void HookMemberDuplicator::reflect(uint16_t& ref, const char* name) {}
void HookMemberDuplicator::reflect(uint32_t& ref, const char* name) {}
void HookMemberDuplicator::reflect(float& ref, const char* name) {}
void HookMemberDuplicator::reflect(Vector3& ref, const char* name) {}
void HookMemberDuplicator::reflect(EventNode& ref, const char* name, CKObject* user) {}
void HookMemberDuplicator::reflect(std::string& ref, const char* name) {}

void HookMemberDuplicator::doClone(CKHook* hook)
{
	exporting = false;
	cloneFunction = [this](CKObject* obj, int sector) -> CKObject* {
		return kenv.cloneObject(obj, sector);
	};
	doCommon(hook);
}

void HookMemberDuplicator::doExport(CKHook* hook)
{
	exporting = true;
	copyenv = {};
	ClassRegister::registerClasses(copyenv, kenv.version, kenv.platform, kenv.isRemaster);
	copyenv.loadGame(kenv.gamePath.c_str(), kenv.version, kenv.platform, kenv.isRemaster);
	copyenv.outGamePath = "C:\\Users\\Adrien\\Desktop\\kthings\\xecexport";
	copyenv.numSectors = 0;
	for (size_t cat = 0; cat < 15; ++cat) {
		auto& kCats = kenv.levelObjects.categories[cat];
		auto& cCats = copyenv.levelObjects.categories[cat];
		cCats.type.resize(kCats.type.size());
		for (size_t cl = 0; cl < kCats.type.size(); ++cl) {
			KObjectList::ClassType& kty = kCats.type[cl];
			KObjectList::ClassType tty;
			tty.startId = 0;
			tty.totalCount = 0;
			tty.info = kty.info;
			cCats.type[cl] = std::move(tty);
		}
	}
	CSGBranch* copyNodeRoot = copyenv.createAndInitObject<CSGSectorRoot>(-1);
	copyenv.createAndInitObject<CKSrvEvent>(-1);

	cloneFunction = [this](CKObject* obj, int sector) -> CKObject* {
		uint32_t fid = obj->getClassFullID();
		auto* copy = copyenv.createObject(fid, -1);
		copyenv.factories.at(fid).copy(obj, copy);
		return (decltype(obj))copy;
	};

	doCommon(hook);
	
	copyenv.saveLevel(0);
	copyenv.unloadGame();
}

void HookMemberDuplicator::doCommon(CKHook* hook)
{
	CKGroup* group = HookMemberDuplicator::findGroup(hook, kenv.levelObjects.getFirst<CKGroupRoot>());
	assert(group);
	CKHook* clone = cloneWrap(hook);
	clone->next = nullptr;
	clone->activeSector = -1;
	cloneMap[hook] = clone;
	clone->virtualReflectMembers(*this, &kenv);
	CKHookLife* life = nullptr;
	if (hook->life) {
		life = cloneWrap(hook->life.get());
		life->unk1 = 0;
		life->hook = clone; life->nextLife = nullptr;
	}
	clone->life = life; clone->next = nullptr;
	if (!exporting)
		group->addHook(clone);
	clone->update();

	if (!exporting) {
		// add collisions
		CKSrvCollision* srvCollision = kenv.levelObjects.getFirst<CKSrvCollision>();
		std::vector<CKSrvCollision::Bing> dupColls;
		std::map<uint16_t, uint16_t> clonedCollMap;
		auto substituteIfCloned = [this](CKObject* obj) {
			if (auto it = cloneMap.find(obj); it != cloneMap.end()) {
				return it->second;
			}
			return obj;
		};
		uint16_t index = 0;
		for (auto& coll : srvCollision->bings) {
			if (cloneMap.count(coll.obj1.get()) || cloneMap.count(coll.obj2.get())) {
				uint16_t cIndex = (uint16_t)dupColls.size();
				auto& dup = dupColls.emplace_back(coll);
				dup.obj1 = substituteIfCloned(dup.obj1.get());
				dup.obj2 = substituteIfCloned(dup.obj2.get());
				if (coll.b1 != 0xFFFF) {
					CKObject* hand1 = substituteIfCloned(srvCollision->objs2[coll.b1].get());
					dup.b1 = srvCollision->addOrGetHandler(hand1);
				}
				if (coll.b2 != 0xFFFF) {
					CKObject* hand2 = substituteIfCloned(srvCollision->objs2[coll.b2].get());
					dup.b2 = srvCollision->addOrGetHandler(hand2);
				}
				dup.aa[0] = 0xFFFF;
				dup.aa[1] = 0xFFFF;
				clonedCollMap[index] = cIndex;
			}
			++index;
		}
		uint16_t originalSize = (uint16_t)srvCollision->bings.size();
		for (auto& dup : dupColls) {
			if (dup.aa[2] != 0xFFFF && clonedCollMap.count(dup.aa[2])) {
				dup.aa[2] = originalSize + clonedCollMap.at(dup.aa[2]);
			}
			dup.v1 &= 0xF; // disable it
			uint16_t added = (uint16_t)srvCollision->bings.size();
			dup.aa[0] = srvCollision->inactiveList;
			srvCollision->inactiveList = added;
			srvCollision->bings.push_back(std::move(dup));
		}

		// For animated characters, add their cloned hook to CKLevel's list if it was there
		if (CKHkAnimatedCharacter* animChar = clone->dyncast<CKHkAnimatedCharacter>()) {
			CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
			if (std::find_if(level->objs.begin(), level->objs.end(), [hook](auto& ref) {return ref.get() == hook; }) != level->objs.end())
				level->objs.emplace_back(clone);
		}
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
	}
}
