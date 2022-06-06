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
#include <filesystem>

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
			auto& destenv = exporting ? copyenv : kenv;
			addToMeshKluster(destenv.levelObjects, dGround);
			for (auto& str : destenv.sectorObjects)
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
	CKHook* clonedHook = doCommon(hook);

	CKGroup* group = HookMemberDuplicator::findGroup(hook, kenv.levelObjects.getFirst<CKGroupRoot>());
	assert(group);
	group->addHook(clonedHook);
}

void HookMemberDuplicator::doExport(CKHook* hook, const std::filesystem::path& path)
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
	copyenv.createAndInitObject<CKSoundDictionary>();
	copyenv.createAndInitObject<CTextureDictionary>();
	CCloneManager* cloneMgr = copyenv.createAndInitObject<CCloneManager>();
	cloneMgr->_team.numBongs = kenv.levelObjects.getFirst<CCloneManager>()->_team.numBongs;
	copyenv.createAndInitObject<CAnimationManager>();

	cloneFunction = [this](CKObject* obj, int sector) -> CKObject* {
		uint32_t fid = obj->getClassFullID();
		CKObject* copy = copyenv.createObject(fid, -1);
		copyenv.factories.at(fid).copy(obj, copy);

		if (CKSoundDictionaryID* ids = copy->dyncast<CKSoundDictionaryID>()) {
			CKSoundDictionary* odict = kenv.levelObjects.getFirst<CKSoundDictionary>();
			CKSoundDictionary* cdict = copyenv.levelObjects.getFirst<CKSoundDictionary>();
			for (auto& ent : ids->soundEntries) {
				uint32_t id = ent.id;
				uint32_t nextid = (uint32_t)cdict->sounds.size();
				cdict->sounds.push_back(odict->sounds[id]);
				cdict->rwSoundDict.list.sounds.push_back(odict->rwSoundDict.list.sounds[id]);
				ent.id = nextid;
			}
		}

		if (CNode* node = copy->dyncast<CNode>()) {
			if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
				CCloneManager* mgr = kenv.levelObjects.getFirst<CCloneManager>();
				CCloneManager* exmgr = copyenv.levelObjects.getFirst<CCloneManager>();
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

			CTextureDictionary* cdict = copyenv.levelObjects.getFirst<CTextureDictionary>();
			for(CKAnyGeometry* geo = node->geometry.get(); geo; geo = geo->nextGeo.get()) {
				if (!geo->clump) continue;
				RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
				if (!rwgeo) continue;
				if (rwgeo->materialList.materials.empty()) continue;
				const std::string& name = rwgeo->materialList.materials.at(0).texture.name;
				if (cdict->piDict.findTexture(name) != -1)
					continue;
				CTextureDictionary* odict = kenv.levelObjects.getFirst<CTextureDictionary>();
				size_t tid = odict->piDict.findTexture(name);
				size_t str = 0;
				while (tid == -1 && str < (size_t)kenv.numSectors) {
					odict = kenv.sectorObjects[str++].getFirst<CTextureDictionary>();
					tid = odict->piDict.findTexture(name);
				}
				if (tid != -1) {
					cdict->piDict.textures.push_back(odict->piDict.textures[tid]);
				}
			}
		}

		if (CAnimationDictionary* cAnimDict = copy->dyncast<CAnimationDictionary>()) {
			CAnimationManager* oAnimMgr = kenv.levelObjects.getFirst<CAnimationManager>();
			CAnimationManager* cAnimMgr = copyenv.levelObjects.getFirst<CAnimationManager>();
			for (uint32_t& ind : cAnimDict->animIndices) {
				if (ind != -1) {
					uint32_t nextIndex = (uint32_t)cAnimMgr->commonAnims.anims.size();
					cAnimMgr->commonAnims.anims.push_back(oAnimMgr->commonAnims.anims[ind]);
					ind = nextIndex;
				}
			}
		}

		return copy;
	};

	CKHook* clonedHook = doCommon(hook);
	
	saveKFab(copyenv, clonedHook, path);
	copyenv.unloadGame();
}

void HookMemberDuplicator::doImport(const std::filesystem::path& path, CKGroup* parent)
{
	KEnvironment kfab;
	CKObject* mainObj = loadKFab(kfab, path);
	
	cloneFunction = [this, &kfab](CKObject* obj, int sector) -> CKObject* {
		uint32_t fid = obj->getClassFullID();

		auto& ktype = kenv.levelObjects.getClassType(fid);
		if (ktype.objects.empty() && ktype.info == 0) {
			auto& ftype = kfab.levelObjects.getClassType(fid);
			ktype.info = ftype.info;
		}

		CKObject* copy = kenv.createObject(fid, -1);
		kenv.factories.at(fid).copy(obj, copy);

		if (CKSoundDictionaryID* ids = copy->dyncast<CKSoundDictionaryID>()) {
			CKSoundDictionary* odict = kenv.levelObjects.getFirst<CKSoundDictionary>();
			CKSoundDictionary* fdict = kfab.levelObjects.getFirst<CKSoundDictionary>();
			for (auto& ent : ids->soundEntries) {
				uint32_t id = ent.id;
				uint32_t nextid = (uint32_t)odict->sounds.size();
				odict->sounds.push_back(fdict->sounds[id]);
				odict->rwSoundDict.list.sounds.push_back(fdict->rwSoundDict.list.sounds[id]);
				ent.id = nextid;
			}
		}

		if (CNode* node = copy->dyncast<CNode>()) {
			if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
				CCloneManager* mgr = kenv.levelObjects.getFirst<CCloneManager>();
				CCloneManager* exmgr = kfab.levelObjects.getFirst<CCloneManager>();
				std::swap(mgr, exmgr);
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

			CTextureDictionary* odict = kenv.levelObjects.getFirst<CTextureDictionary>();
			CTextureDictionary* fdict = kfab.levelObjects.getFirst<CTextureDictionary>();
			for (CKAnyGeometry* geo = node->geometry.get(); geo; geo = geo->nextGeo.get()) {
				if (!geo->clump) continue;
				RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
				if (!rwgeo) continue;
				if (rwgeo->materialList.materials.empty()) continue;
				if (rwgeo->materialList.materials.empty())
					continue;
				const std::string& name = rwgeo->materialList.materials.at(0).texture.name;
				// if odict already has texture, continue
				size_t tid = odict->piDict.findTexture(name);
				if (tid != -1)
					continue;
				// else, if fab has texture, copy
				tid = fdict->piDict.findTexture(name);
				if (tid != -1)
					odict->piDict.textures.push_back(fdict->piDict.textures[tid]);
			}
		}

		if (CAnimationDictionary* cAnimDict = copy->dyncast<CAnimationDictionary>()) {
			CAnimationManager* oAnimMgr = kenv.levelObjects.getFirst<CAnimationManager>();
			CAnimationManager* fAnimMgr = kfab.levelObjects.getFirst<CAnimationManager>();
			for (uint32_t& ind : cAnimDict->animIndices) {
				if (ind != -1) {
					uint32_t nextIndex = (uint32_t)oAnimMgr->commonAnims.anims.size();
					oAnimMgr->commonAnims.anims.push_back(fAnimMgr->commonAnims.anims[ind]);
					ind = nextIndex;
				}
			}
		}

		return copy;
	};
	
	exporting = false;
	CKHook* clonedHook = doCommon(mainObj->cast<CKHook>());
	parent->addHook(clonedHook);
}

CKHook* HookMemberDuplicator::doCommon(CKHook* hook)
{
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
	clone->update();

	if (!exporting) {
		// add collisions
		if (CKSrvCollision* srvCollision = kenv.levelObjects.getFirst<CKSrvCollision>()) {
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

	return clone;
}


void HookMemberDuplicator::saveKFab(KEnvironment& kfab, CKObject* mainObj, const std::filesystem::path& path)
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

CKObject* HookMemberDuplicator::loadKFab(KEnvironment& kfab, const std::filesystem::path& path)
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
