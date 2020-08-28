#include "KEnvironment.h"
#include "File.h"
#include "KObject.h"
#include <cassert>
#include <stack>
#include <algorithm>

const char * KEnvironment::platformExt[5] = { "K", "KWN", "KP2", "KGC", "KPP" };

void KEnvironment::loadGame(const char * path, int version, int platform)
{
	this->gamePath = this->outGamePath = path;
	this->version = version;
	this->platform = platform;

	if (version < KVERSION_XXL2)
		clcatReorder = { 0,9,1,2,3,4,5,6,7,8,10,11,12,13,14 };
	else
		clcatReorder = { 9,0,1,2,3,4,5,6,7,8,10,11,12,13,14 };

	char gamefn[300];
	snprintf(gamefn, sizeof(gamefn), "%s/GAME.%s", gamePath.c_str(), platformExt[platform]);

	IOFile gameFile(gamefn, "rb");
	if (version < KVERSION_XXL2) {
		uint32_t numGameObjects = gameFile.readUint32();
		uint32_t gameManagerId = gameFile.readUint32();
		this->globalObjects.reserve(numGameObjects);
		for (uint32_t i = 0; i < numGameObjects; i++) {
			uint32_t clcat = gameFile.readUint32();
			uint32_t clid = gameFile.readUint32();
			uint32_t nextoff = gameFile.readUint32();
			CKObject *obj = new CKUnknown(clcat, clid);
			obj->deserialize(this, &gameFile, nextoff - gameFile.tell());
			assert(nextoff == gameFile.tell());
			this->globalObjects.push_back(obj);
		}
	}
	else {
		uint32_t numGameObjects = gameFile.readUint32();
		gameFile.read(this->gameManagerUuid.data(), 16);
		uint32_t gameManagerId = gameFile.readUint32();
		this->globalObjects.reserve(numGameObjects);
		uint32_t i = 0;
		while (i < numGameObjects) {
			uint32_t clfid = gameFile.readUint32();
			uint32_t count = gameFile.readUint32();
			uint8_t hasUuid = gameFile.readUint8();
			for (uint32_t j = 0; j < count; j++) {
				CKObject *obj = constructObject(clfid);
				this->globalObjects.push_back(obj);
				if (hasUuid) {
					kuuid uid;
					gameFile.read(uid.data(), 16);
					this->globalUuidMap[uid] = obj;
				}
			}
			i += count;
		}
		for (CKObject *obj : this->globalObjects) {
			uint32_t nextoff = gameFile.readUint32();
			obj->deserialize(this, &gameFile, nextoff - gameFile.tell());
			assert(nextoff == gameFile.tell());
		}
	}
}

void KEnvironment::loadLevel(int lvlNumber)
{
	if (levelLoaded)
		unloadLevel();

	this->loadingSector = -1;
	char lvlfn[300];
	snprintf(lvlfn, sizeof(lvlfn), "%s/LVL%03u/LVL%02u.%s", gamePath.c_str(), lvlNumber, lvlNumber, platformExt[platform]);

	IOFile lvlFile(lvlfn, "rb");
	if (platform == PLATFORM_PC && version == KVERSION_XXL1) {
		std::string asthead = lvlFile.readString(8);
		assert(asthead == "Asterix ");
	}
	if (version < KVERSION_XXL2) {
		this->lvlUnk1 = lvlFile.readUint32();
		//lvlFile.readUint32(); // DRM
		this->numSectors = lvlFile.readUint8();
		this->lvlUnk2 = lvlFile.readUint32();
	}
	else {
		if (this->version >= KVERSION_ARTHUR)
			uint32_t headerSize = lvlFile.readUint32();
		this->numSectors = lvlFile.readUint8();
		kuuid lvlGameUuid;
		lvlFile.read(lvlGameUuid.data(), 16);
		assert(lvlGameUuid == this->gameManagerUuid);
		this->lvlUnk2 = lvlFile.readUint32();
	}

	this->sectorObjects.resize(this->numSectors);

	for (int clcat = 0; clcat < 15; clcat++) {
		uint16_t numClasses = lvlFile.readUint16();
		this->levelObjects.categories[clcat].type.resize(numClasses);
		for (uint16_t clid = 0; clid < numClasses; clid++) {
			auto &lvltype = this->levelObjects.categories[clcat].type[clid];

			uint32_t fid = clcat | (clid << 6);
			uint16_t numTotalObjects = lvlFile.readUint16();
			uint16_t numLevelObjects = lvlFile.readUint16();
			uint16_t numGlobs;
			if (version >= KVERSION_XXL2) {
				numGlobs = lvlFile.readUint16();
				lvltype.globByte = lvlFile.readUint8();
			}
			uint8_t info = lvlFile.readUint8();

			if (version >= KVERSION_XXL2) {
				lvltype.globUuids.resize(numGlobs);
				for (kuuid &id : lvltype.globUuids) {
					lvlFile.read(id.data(), 16);
				}
			}

			lvltype.info = info;
			lvltype.totalCount = numTotalObjects;
			lvltype.startId = 0;
			lvltype.objects.reserve(numLevelObjects);
			for (uint16_t i = 0; i < numLevelObjects; i++) {
				CKObject *obj = constructObject(fid);
				lvltype.objects.push_back(obj);
				if (lvltype.globByte) {
					kuuid id;
					lvlFile.read(id.data(), 16);
					levelUuidMap[id] = obj;
				}
				//printf("Constructed %s\n", lvltype.objects.back()->getClassName());
			}
		}
	}

	lvlFile.seek(8 /*+ 4 (drm)*/, SEEK_CUR);

	for (int clcat : clcatReorder) {
		uint16_t numClasses = lvlFile.readUint16();
		uint32_t nextCat = lvlFile.readUint32();
		printf("Cat %i at %08X, next at %08X, numClasses = %i\n", clcat, lvlFile.tell(), nextCat, numClasses);
		for (size_t clid = 0; clid < this->levelObjects.categories[clcat].type.size(); clid++) {
			auto &cltype = this->levelObjects.categories[clcat].type[clid];
			if (cltype.objects.empty() && cltype.globUuids.empty())
				continue;
			uint32_t nextClass = lvlFile.readUint32();
			//printf("Class %i %i at %08X, next at %08X\n", clcat, clid, lvlFile.tell(), nextClass);
			if (cltype.info) {
				if (version >= KVERSION_XXL2) {
					uint16_t numGlobals = lvlFile.readUint16();
					assert(numGlobals == cltype.globUuids.size());
					for (int gb = 0; gb < numGlobals; gb++) {
						uint32_t nextGlob = lvlFile.readUint32();
						globalUuidMap.at(cltype.globUuids[gb])->deserializeLvlSpecific(this, &lvlFile, nextGlob - lvlFile.tell());
						assert(lvlFile.tell() == nextGlob);
					}
				}
				uint16_t startid = lvlFile.readUint16();
				assert(startid == 0);
			}
			//printf("* %08X\n", lvlFile.tell());
			for (CKObject *obj : this->levelObjects.categories[clcat].type[clid].objects) {
				uint32_t nextObjOffset = lvlFile.readUint32();
				obj->deserialize(this, &lvlFile, nextObjOffset - lvlFile.tell());
				assert(lvlFile.tell() == nextObjOffset);
			}
			assert(lvlFile.tell() == nextClass);
			numClasses--;
		}
		assert(numClasses == 0);
		assert(lvlFile.tell() == nextCat);
	}

	for(int i = 0; i < this->numSectors; i++)
		loadSector(i, lvlNumber);

	this->loadingSector = -1;
	for (auto &cat : levelObjects.categories)
		for (auto &cl : cat.type)
			for (CKObject *obj : cl.objects)
				obj->onLevelLoaded(this);
	for(auto &str : sectorObjects)
		for (auto &cat : str.categories)
			for (auto &cl : cat.type)
				for (CKObject *obj : cl.objects)
					obj->onLevelLoaded(this);

	this->loadingSector = -1;
	for (auto &cat : levelObjects.categories)
		for (auto &cl : cat.type)
			for (CKObject *obj : cl.objects)
				obj->onLevelLoaded2(this);
	for (auto &str : sectorObjects)
		for (auto &cat : str.categories)
			for (auto &cl : cat.type)
				for (CKObject *obj : cl.objects)
					obj->onLevelLoaded2(this);

	if (version >= KVERSION_XXL2) {
		uint32_t weirdOffset = lvlFile.readUint32();
		assert(lvlFile.tell() == weirdOffset);
		uint32_t numNameTables = lvlFile.readUint32(); // 1 for lvl + 1 per sector
		assert(numNameTables == 1 + numSectors);
		sectorObjNames.resize(numSectors);
		for (int ntindex = 0; ntindex < numNameTables; ntindex++) {
			uint32_t ntNumObjects = lvlFile.readUint32();
			auto &objNameList = (ntindex == 0) ? levelObjNames : sectorObjNames[ntindex - 1];
			for (int i = 0; i < ntNumObjects; i++) {
				CKObject *obj = this->readObjPnt(&lvlFile, ntindex - 1);
				objNameList.order.push_back(obj);
				auto &info = objNameList.dict[obj];
				uint16_t nameSize = lvlFile.readUint16();
				info.name = lvlFile.readString(nameSize);
				info.anotherId = lvlFile.readUint32();
				info.user = this->readObjRef<CKObject>(&lvlFile, ntindex - 1);
				for (int32_t j : {0, 0, 0, 0})
					assert(lvlFile.readUint32() == (uint32_t)j);
				info.user2 = this->readObjRef<CKObject>(&lvlFile, ntindex - 1);
				assert(lvlFile.readUint32() == 0xFFFFFFFF);
			}
		}
	}

	levelLoaded = true;
}

struct OffsetStack {
	std::stack<uint32_t> offsets;
	File *file;
	void push()
	{
		offsets.push(file->tell());
		file->writeUint32(0);
	}
	void pop()
	{
		uint32_t endpos = file->tell();
		uint32_t prevpos = offsets.top();
		file->seek(prevpos, SEEK_SET);
		file->writeUint32(endpos);
		offsets.pop();
		file->seek(endpos, SEEK_SET);
	}
	OffsetStack(File *file) : file(file) {}
};

void KEnvironment::saveLevel(int lvlNumber)
{
	char lvlfn[300];
	snprintf(lvlfn, sizeof(lvlfn), "%s/LVL%03u/LVL%02u.%s", outGamePath.c_str(), lvlNumber, lvlNumber, platformExt[platform]);

	prepareSavingMap();

	IOFile lvlFile(lvlfn, "wb");
	OffsetStack offsetStack(&lvlFile);
	if (version == KVERSION_XXL1 && platform == PLATFORM_PC)
		lvlFile.write("Asterix ", 8);
	if (version < KVERSION_XXL2) {
		lvlFile.writeUint32(this->lvlUnk1);
		lvlFile.writeUint8(this->numSectors);
		lvlFile.writeUint32(this->lvlUnk2);
	}
	else {
		if (this->version >= KVERSION_ARTHUR)
			offsetStack.push();
		lvlFile.writeUint8(this->numSectors);
		lvlFile.write(this->gameManagerUuid.data(), 16);
		lvlFile.writeUint32(this->lvlUnk2);
	}

	for (auto &cat : this->levelObjects.categories) {
		lvlFile.writeUint16(cat.type.size());
		for (auto &kcl : cat.type) {
			lvlFile.writeUint16(kcl.totalCount);
			lvlFile.writeUint16(kcl.objects.size());
			if (version >= KVERSION_XXL2) {
				lvlFile.writeUint16(kcl.globUuids.size());
				lvlFile.writeUint8(kcl.globByte);
			}
			lvlFile.writeUint8(kcl.info);
			if (version >= KVERSION_XXL2) {
				for (const kuuid &id : kcl.globUuids)
					lvlFile.write(id.data(), 16);
				if (kcl.globByte) {
					for (CKObject *obj : kcl.objects) {
						lvlFile.write(saveUuidMap.at(obj).data(), 16);
					}
				}
			}
		}
	}

	if (version >= KVERSION_ARTHUR)
		offsetStack.pop();
	lvlFile.writeUint32(0);
	lvlFile.writeUint32(0);

	for (int nclcat : clcatReorder) {
		auto &cat = this->levelObjects.categories[nclcat];

		uint32_t clcnt = 0;
		for (auto &kcl : cat.type)
			if (!kcl.objects.empty() || !kcl.globUuids.empty())
				clcnt++;

		lvlFile.writeUint16(clcnt);
		offsetStack.push();

		for (auto &kcl : cat.type) {
			if (!kcl.objects.empty() || !kcl.globUuids.empty()) {
				offsetStack.push();
				if (kcl.info) {
					if (version >= KVERSION_XXL2) {
						lvlFile.writeUint16(kcl.globUuids.size());
						for (kuuid &id : kcl.globUuids) {
							offsetStack.push();
							globalUuidMap.at(id)->serializeLvlSpecific(this, &lvlFile);
							offsetStack.pop();
						}
					}
					lvlFile.writeUint16(0); // startid
				}
				for (CKObject *obj : kcl.objects) {
					offsetStack.push();
					obj->serialize(this, &lvlFile);
					offsetStack.pop();
				}
				offsetStack.pop();
			}
		}
		offsetStack.pop();
	}

	if (version >= KVERSION_XXL2) {
		lvlFile.writeUint32(lvlFile.tell() + 4);
		lvlFile.writeUint32(1 + numSectors);
		for (int str = -1; str < (int)this->numSectors; str++) {
			ObjNameList &objNameList = (str == -1) ? levelObjNames : sectorObjNames[str];
			lvlFile.writeUint32(objNameList.order.size());
			for (CKObject *obj : objNameList.order) {
				this->writeObjID(&lvlFile, obj);
				auto &info = objNameList.dict.at(obj);
				lvlFile.writeUint16(info.name.size());
				lvlFile.write(info.name.data(), info.name.size());
				lvlFile.writeUint32(info.anotherId);
				this->writeObjRef(&lvlFile, info.user);
				for (int32_t i : {0, 0, 0, 0})
					lvlFile.writeUint32((uint32_t)i);
				this->writeObjRef(&lvlFile, info.user2);
				lvlFile.writeUint32(0xFFFFFFFF);
			}
		}
	}

	for (int i = 0; i < this->numSectors; i++)
		saveSector(i, lvlNumber);

	saveMap.clear();
	saveUuidMap.clear();
}

bool KEnvironment::loadSector(int strNumber, int lvlNumber)
{
	printf("Loading sector %i\n", strNumber);
	this->loadingSector = strNumber;
	char strfn[300];
	snprintf(strfn, sizeof(strfn), "%s/LVL%03u/STR%02u_%02u.%s", gamePath.c_str(), lvlNumber, lvlNumber, strNumber, platformExt[platform]);

	IOFile strFile(strfn, "rb");
	KObjectList &strObjList = this->sectorObjects[strNumber];

	int clcat = 0;
	for (auto &cat : strObjList.categories) {
		uint16_t numClasses = strFile.readUint16();
		cat.type.resize(numClasses);
		int clid = 0;
		for (auto &kcl : cat.type) {
			uint16_t numInsts = strFile.readUint16();

			auto &lvltype = levelObjects.categories[clcat].type[clid];
			kcl.startId = lvltype.objects.size();
			if (lvltype.info != 2) {
				for (int p = 0; p < strNumber; p++)
					kcl.startId += sectorObjects[p].categories[clcat].type[clid].objects.size();
			}

			kcl.objects.reserve(numInsts);
			for (uint16_t i = 0; i < numInsts; i++) {
				kcl.objects.push_back(constructObject(clcat, clid));
				//printf("Constructed in STR %s\n", kcl.objects.back()->getClassName());
			}
			clid++;
		}
		clcat++;
	}

	for (int ncat : this->clcatReorder) {
		auto &cat = strObjList.categories[ncat];

		uint16_t numClasses = strFile.readUint16();
		uint32_t nextCat = strFile.readUint32();

		uint16_t count = numClasses;
		for (auto &kcl : cat.type) {
			if (kcl.objects.empty())
				continue;
			uint32_t nextClass = strFile.readUint32();
			uint16_t startid = strFile.readUint16();
			//kcl.startId = startid;
			assert(kcl.startId == startid);
			for (CKObject *obj : kcl.objects) {
				//printf("Deserializing %s\n", obj->getClassName());
				uint32_t nextObj = strFile.readUint32();
				obj->deserialize(this, &strFile, nextObj - strFile.tell());
				assert(strFile.tell() == nextObj);
			}
			assert(strFile.tell() == nextClass);
			count--;
		}

		assert(count == 0);
		assert(strFile.tell() == nextCat);
	}
	return true;
}

void KEnvironment::saveSector(int strNumber, int lvlNumber)
{
	printf("Saving sector %i\n", strNumber);
	char strfn[300];
	snprintf(strfn, sizeof(strfn), "%s/LVL%03u/STR%02u_%02u.%s", outGamePath.c_str(), lvlNumber, lvlNumber, strNumber, platformExt[platform]);

	IOFile strFile(strfn, "wb");
	OffsetStack offsetStack(&strFile);
	KObjectList &strObjList = this->sectorObjects[strNumber];

	for (auto &cat : strObjList.categories) {
		strFile.writeUint16(cat.type.size());
		for (auto &kcl : cat.type) {
			strFile.writeUint16(kcl.objects.size());
		}
	}

	int clcat = 0;
	for (int ncat : this->clcatReorder) {
		auto &cat = strObjList.categories[ncat];
		uint16_t clcnt = 0;
		for (auto &kcl : cat.type)
			if (!kcl.objects.empty())
				clcnt++;

		strFile.writeUint16(clcnt);
		offsetStack.push();

		int clid = -1;
		for (auto &kcl : cat.type) {
			clid++;
			if (kcl.objects.empty())
				continue;
			offsetStack.push();
			strFile.writeUint16(kcl.startId);
			for (CKObject *obj : kcl.objects) {
				offsetStack.push();
				obj->serialize(this, &strFile);
				offsetStack.pop();
			}
			offsetStack.pop();
		}
		offsetStack.pop();
		clcat++;
	}
}


void KEnvironment::prepareSavingMap()
{
	saveMap.clear();
	for (int clcat = 0; clcat < 15; clcat++) {
		auto &lvlcat = levelObjects.categories[clcat];
		for (int clid = 0; clid < lvlcat.type.size(); clid++) {
			auto &lvltype = lvlcat.type[clid];

			for (int i = 0; i < lvltype.objects.size(); i++)
				saveMap[lvltype.objects[i]] = clcat | (clid << 6) | (i << 17);

			//if (lvltype.info == 0)
			//	assert(lvltype.objects.empty());
			lvltype.totalCount = lvltype.objects.size();
			if (lvltype.info <= 1) {
				for (auto &str : sectorObjects) {
					auto &strtype = str.categories[clcat].type[clid];
					strtype.startId = lvltype.totalCount;
					for (CKObject *obj : strtype.objects) {
						saveMap[obj] = clcat | (clid << 6) | (lvltype.totalCount << 17);
						lvltype.totalCount++;
					}
				}
			}
			else if (lvltype.info == 2) {
				size_t maxStrObjects = lvltype.totalCount;
				for (auto &str : sectorObjects) {
					auto &strtype = str.categories[clcat].type[clid];
					strtype.startId = lvltype.totalCount;
					size_t curid = lvltype.totalCount;
					for (CKObject *obj : strtype.objects) {
						saveMap[obj] = clcat | (clid << 6) | (curid << 17);
						curid++;
					}
					if (curid > maxStrObjects)
						maxStrObjects = curid;
				}
				lvltype.totalCount = maxStrObjects;
			}
		}
	}

	saveUuidMap.clear();
	for (auto &elem : globalUuidMap)
		saveUuidMap[elem.second] = elem.first;
	for (auto &elem : levelUuidMap)
		saveUuidMap[elem.second] = elem.first;
}

void KEnvironment::unloadLevel()
{
	levelObjNames.clear();
	sectorObjNames.clear();
	levelUuidMap.clear();
	for (auto &str : sectorObjects)
		for (auto &cat : str.categories)
			for (auto &cl : cat.type)
				for (CKObject *obj : cl.objects)
					delete obj;
	sectorObjects.clear();
	numSectors = 0;
	for (auto &cat : levelObjects.categories) {
		for (auto &cl : cat.type)
			for (CKObject *obj : cl.objects)
				delete obj;
		cat.type.clear();
	}
	for (CKObject *glob : globalObjects)
		glob->resetLvlSpecific(this);

	// check and clean the refCount map
	for (auto it = CKObject::refCounts.begin(); it != CKObject::refCounts.end(); ) {
		if (it->second == 0)
			it = CKObject::refCounts.erase(it);
		else {
			// if refcount is not null, be sure that it's a global object and not one from the LVL
			assert(std::find(globalObjects.begin(), globalObjects.end(), it->first) != globalObjects.end());
			++it;
		}
	}

	levelLoaded = false;
}

void KEnvironment::unloadGame()
{
	unloadLevel();
	for (CKObject *obj : globalObjects)
		delete obj;
	globalUuidMap.clear();
	globalObjNames.clear();
}

CKObject * KEnvironment::constructObject(uint32_t fid)
{
	auto it = factories.find(fid);
	if (it != factories.end())
		return factories.at(fid).create();
	else
		return new CKUnknown(fid & 63, fid >> 6);
}

CKObject * KEnvironment::createObject(uint32_t fid, int sector)
{
	CKObject *obj = constructObject(fid);
	if (sector == -1)
		levelObjects.categories[fid & 63].type[fid >> 6].objects.push_back(obj);
	else
		sectorObjects[sector].categories[fid & 63].type[fid >> 6].objects.push_back(obj);
	return obj;
}

void KEnvironment::removeObject(CKObject * obj)
{
	assert(obj->getRefCount() == 0);

	int clcat = obj->getClassCategory();
	int clid = obj->getClassID();

	auto &lvlobjvec = levelObjects.categories[clcat].type[clid].objects;
	lvlobjvec.erase(std::remove(lvlobjvec.begin(), lvlobjvec.end(), obj), lvlobjvec.end());
	
	for (auto &str : sectorObjects) {
		auto &strobjvec = str.categories[clcat].type[clid].objects;
		strobjvec.erase(std::remove(strobjvec.begin(), strobjvec.end(), obj), strobjvec.end());
	}

	delete obj;
}

CKObject * KEnvironment::getObjPnt(uint32_t objid, int sector)
{
	if (sector == -1)
		sector = this->loadingSector;
	if (objid == 0xFFFFFFFF)
		return nullptr;
	unsigned int clcat = objid & 63;
	unsigned int clid = (objid >> 6) & 2047;
	unsigned int objnb = objid >> 17;
	assert(clcat < 15);
	assert(clid < levelObjects.categories[clcat].type.size());
	//assert(objnb < levelObjects.categories[clcat].type[clid].objects.size());
	auto &cllvlobjs = levelObjects.categories[clcat].type[clid].objects;
	if (objnb < cllvlobjs.size())
		return cllvlobjs[objnb];
	else {
		assert(sector != -1);
		auto &strtype = sectorObjects[sector].categories[clcat].type[clid];
		return strtype.objects[objnb - strtype.startId];
		//return loadMap.at(std::make_pair(objid, sector));
	}
}

CKObject * KEnvironment::readObjPnt(File * file, int sector)
{
	uint32_t objid = file->readUint32();
	if (objid == 0xFFFFFFFD) {
		kuuid uid;
		file->read(uid.data(), 16);
		auto it = levelUuidMap.find(uid);
		if (it != levelUuidMap.end())
			return it->second;
		it = globalUuidMap.find(uid);
		assert(it != globalUuidMap.end());
		return it->second;
	}
	else
		return getObjPnt(objid, sector);
}

uint32_t KEnvironment::getObjID(CKObject * obj)
{
	if (obj == nullptr)
		return 0xFFFFFFFF;
	auto it = saveMap.find(obj);
	assert(it != saveMap.end());
	return it->second;
}

void KEnvironment::writeObjID(File * file, CKObject * obj)
{
	auto it = saveUuidMap.find(obj);
	if (it != saveUuidMap.end()) {
		file->writeUint32(0xFFFFFFFD);
		file->write(it->second.data(), 16);
	}
	else
		file->writeUint32(getObjID(obj));
}