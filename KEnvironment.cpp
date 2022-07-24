#include "KEnvironment.h"
#include "File.h"
#include "KObject.h"
#include <cassert>
#include <stack>
#include <algorithm>
#include <filesystem>
#include "GuiUtils.h"

const char * KEnvironment::platformExt[7] = { "K", "KWN", "KP2", "KGC", "KPP", "KRV", "KXE"};

static auto ConcatGamePath(const std::string& gameDir, const std::string_view& gameFile) {
	return std::filesystem::u8path(gameDir).append(gameFile);
}

void KEnvironment::loadGame(const char * path, int version, int platform, bool isRemaster)
{
	this->gamePath = this->outGamePath = path;
	this->version = version;
	this->platform = platform;
	this->isRemaster = isRemaster;

	if (version < KVERSION_XXL2)
		clcatReorder = { 0,9,1,2,3,4,5,6,7,8,10,11,12,13,14 };
	else
		clcatReorder = { 9,0,1,2,3,4,5,6,7,8,10,11,12,13,14 };

	auto gamefn = ConcatGamePath(gamePath, std::string("GAME.") + platformExt[platform]);

	IOFile gameFile(gamefn.c_str(), "rb");
	if (version < KVERSION_XXL2) {
		uint32_t numGameObjects = gameFile.readUint32();
		uint32_t gameManagerId = gameFile.readUint32();
		this->globalObjects.reserve(numGameObjects);
		for (uint32_t i = 0; i < numGameObjects; i++) {
			uint32_t clcat = gameFile.readUint32();
			uint32_t clid = gameFile.readUint32();
			uint32_t nextoff = gameFile.readUint32();
			CKObject *obj = constructObject(clcat, clid);
			obj->deserializeGlobal(this, &gameFile, nextoff - gameFile.tell());
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
			obj->deserializeGlobal(this, &gameFile, nextoff - gameFile.tell());
			assert(nextoff == gameFile.tell());
		}
	}
}

void KEnvironment::loadLevel(int lvlNumber)
{
	if (levelLoaded)
		unloadLevel();

	this->loadingSector = -1;
	char lvlfn[32];
	const char* fnfmt = isUsingNewFilenames() ? "LVL%03u/LVL%03u.%s" : "LVL%03u/LVL%02u.%s";
	snprintf(lvlfn, sizeof(lvlfn), fnfmt, lvlNumber, lvlNumber, platformExt[platform]);

	IOFile lvlFile(ConcatGamePath(gamePath, lvlfn).c_str(), "rb");
	if (platform == PLATFORM_PC && version == KVERSION_XXL1) {
		std::string asthead = lvlFile.readString(8);
		if (asthead == "Asterix ") {
			printf("XXL1 Original PC\n");
		} else {
			printf("XXL1 Romastered??\n");
			lvlFile.seek(0, SEEK_SET);
		}
	}
	else if (platform == PLATFORM_PC && version == KVERSION_XXL2) {
		std::string asthead = lvlFile.readString(12);
		if (asthead == "Asterix-XXL2") {
			printf("XXL2 Modded Full version\n");
			isXXL2Demo = false;
		} else {
			printf("XXL2 Demo\n");
			isXXL2Demo = true;
			lvlFile.seek(0, SEEK_SET);
		}
	}
	if (version < KVERSION_XXL2) {
		this->lvlUnk1 = lvlFile.readUint32();
		//lvlFile.readUint32(); // DRM
		this->numSectors = lvlFile.readUint8();
		this->lvlUnk2 = lvlFile.readUint32();
	}
	else {
		if (this->version >= KVERSION_ARTHUR || (this->version == KVERSION_XXL2 && (this->platform != PLATFORM_PC || this->isRemaster)))
			uint32_t headerSize = lvlFile.readUint32();
		this->numSectors = lvlFile.readUint8();
		kuuid lvlGameUuid;
		lvlFile.read(lvlGameUuid.data(), 16);
		assert(lvlGameUuid == this->gameManagerUuid);
		this->lvlUnk2 = lvlFile.readUint32();
	}

	this->sectorObjects.resize(this->numSectors);
	sectorObjNames.resize(numSectors);

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

	if (version == KVERSION_XXL1)
		loadAddendum(lvlNumber);

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
				auto [it, isNew] = objNameList.dict.try_emplace(obj);
				auto& info = it->second;
				auto rname = lvlFile.readSizedString<uint16_t>();
				if (isNew)
					info.name = GuiUtils::latinToUtf8(rname);
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
	char lvlfn[32];
	const char* fnfmt = isUsingNewFilenames() ? "LVL%03u/LVL%03u.%s" : "LVL%03u/LVL%02u.%s";
	snprintf(lvlfn, sizeof(lvlfn), fnfmt, lvlNumber, lvlNumber, platformExt[platform]);

	prepareSavingMap();

	this->lvlUnk2 = 0;
	for (auto &cat : this->levelObjects.categories) {
		this->lvlUnk2 += cat.type.size();
	}

	IOFile lvlFile(ConcatGamePath(outGamePath, lvlfn).c_str(), "wb");
	OffsetStack offsetStack(&lvlFile);
	if (version == KVERSION_XXL1 && platform == PLATFORM_PC && !isRemaster)
		lvlFile.write("Asterix ", 8);
	else if (version == KVERSION_XXL2 && platform == PLATFORM_PC && !isXXL2Demo)
		lvlFile.write("Asterix-XXL2", 12);
	if (version < KVERSION_XXL2) {
		lvlFile.writeUint32(this->lvlUnk1);
		lvlFile.writeUint8(this->numSectors);
		lvlFile.writeUint32(this->lvlUnk2);
	}
	else {
		if (this->version >= KVERSION_ARTHUR || (this->version == KVERSION_XXL2 && (this->platform != PLATFORM_PC || this->isRemaster)))
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

	if (this->version >= KVERSION_ARTHUR || (this->version == KVERSION_XXL2 && (this->platform != PLATFORM_PC || this->isRemaster)))
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
				lvlFile.writeSizedString<uint16_t>(GuiUtils::utf8ToLatin(info.name));
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

	// Save addendum only on XXL1, works on XXL2+ too but not that useful right now
	if (version == KVERSION_XXL1)
		saveAddendum(lvlNumber);

	saveMap.clear();
	saveUuidMap.clear();
}

bool KEnvironment::loadSector(int strNumber, int lvlNumber)
{
	printf("Loading sector %i\n", strNumber);
	this->loadingSector = strNumber;
	char strfn[32];
	const char* fnfmt = isUsingNewFilenames() ? "LVL%03u/STR%03u%02u.%s" : "LVL%03u/STR%02u_%02u.%s";
	snprintf(strfn, sizeof(strfn), fnfmt, lvlNumber, lvlNumber, strNumber, platformExt[platform]);

	IOFile strFile(ConcatGamePath(gamePath, strfn).c_str(), "rb");
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
	char strfn[32];
	const char* fnfmt = isUsingNewFilenames() ? "LVL%03u/STR%03u%02u.%s" : "LVL%03u/STR%02u_%02u.%s";
	snprintf(strfn, sizeof(strfn), fnfmt, lvlNumber, lvlNumber, strNumber, platformExt[platform]);

	IOFile strFile(ConcatGamePath(outGamePath, strfn).c_str(), "wb");
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

void KEnvironment::loadAddendum(int lvlNumber)
{
	char addfn[32];
	snprintf(addfn, sizeof(addfn), "LVL%03u/XEADD%03u.%s", lvlNumber, lvlNumber, platformExt[platform]);
	auto addPath = ConcatGamePath(gamePath, addfn);
	if (!std::filesystem::exists(addPath))
		return;

	IOFile add(addPath.c_str(), "rb");
	auto header = add.readString(12);
	assert(header == "XEC-ADDENDUM");
	int headFileVersion = add.readInt32();
	int headFlags = add.readInt32();
	int headGameVersion = add.readInt32();
	int headGamePlatform = add.readInt32();
	int headGameIsRemaster = add.readInt32();
	int headNumSectors = add.readInt32();
	assert(headFileVersion <= 1);
	assert(headGameVersion == version);
	assert(headGamePlatform == platform);
	assert(headGameIsRemaster == (isRemaster ? 1 : 0));
	assert(headNumSectors == (int)numSectors);
	auto moredata = add.readUint32();
	add.seek(moredata, SEEK_SET);

	auto atnamelist = [this, &add](KEnvironment::ObjNameList& namelist, int str) {
		while (CKObject* obj = readObjPnt(&add, str)) {
			namelist.getObjInfoRef(obj).name = add.readSizedString<uint16_t>();
		}
	};
	auto endNamelist = add.readUint32();
	atnamelist(levelObjNames, -1);
	for (int str = 0; str < headNumSectors; ++str)
		atnamelist(sectorObjNames[str], str);
	assert(add.tell() == endNamelist);

	auto atkobjlist = [this, &add](KObjectList& objlist, int str) {
		while (CKObject* obj = readObjPnt(&add, str)) {
			int objver = add.readInt32();
			auto nextOffset = add.readUint32();
			if (objver <= obj->getAddendumVersion()) {
				obj->deserializeAddendum(this, &add, objver);
				assert(add.tell() == nextOffset);
			}
			else {
				printf("Addendum object skipped\n");
				add.seek(nextOffset, SEEK_SET);
			}
		}
	};
	auto endData = add.readUint32();
	atkobjlist(levelObjects, -1);
	for (int str = 0; str < headNumSectors; ++str)
		atkobjlist(sectorObjects[str], str);
	assert(add.tell() == endData);
}

void KEnvironment::saveAddendum(int lvlNumber)
{
	char addfn[32];
	snprintf(addfn, sizeof(addfn), "LVL%03u/XEADD%03u.%s", lvlNumber, lvlNumber, platformExt[platform]);

	IOFile add(ConcatGamePath(outGamePath, addfn).c_str(), "wb");
	OffsetStack offsetStack(&add);

	add.write("XEC-ADDENDUM", 12);
	add.writeInt32(1); // file version
	add.writeInt32(0); // flags
	add.writeInt32(version);
	add.writeInt32(platform);
	add.writeInt32(isRemaster);
	add.writeInt32(numSectors);
	// additional data for future versions
	offsetStack.push();
	offsetStack.pop();

	auto atnamelist = [this, &add](KEnvironment::ObjNameList& namelist) {
		for (CKObject* obj : namelist.order) {
			auto& info = namelist.dict.at(obj);
			writeObjID(&add, obj);
			add.writeSizedString<uint16_t>(info.name);
		}
		writeObjID(&add, nullptr); // indicate end of name list
	};
	offsetStack.push();
	atnamelist(levelObjNames);
	for (auto& str : sectorObjNames)
		atnamelist(str);
	offsetStack.pop();

	auto atkobjlist = [this,&add,&offsetStack](KObjectList& objlist) {
		for (auto& cat : objlist.categories) {
			for (auto& type : cat.type) {
				for (CKObject* obj : type.objects) {
					int objver = obj->getAddendumVersion();
					if (objver != 0) {
						writeObjID(&add, obj);
						add.writeInt32(objver);
						offsetStack.push();
						obj->serializeAddendum(this, &add);
						offsetStack.pop();
					}
				}
			}
		}
		writeObjID(&add, nullptr); // indicate end of objlist
	};
	offsetStack.push();
	atkobjlist(levelObjects);
	for (auto& str : sectorObjects)
		atkobjlist(str);
	offsetStack.pop();
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
			//assert(std::find(globalObjects.begin(), globalObjects.end(), it->first) != globalObjects.end());
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
	globalObjects.clear();
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

	// erase object info
	for (int i = -1; i < (int)numSectors; i++) {
		ObjNameList &onl = (i == -1) ? levelObjNames : sectorObjNames[i];
		size_t removed = onl.dict.erase(obj);
		if (removed > 0)
			onl.order.erase(std::remove(onl.order.begin(), onl.order.end(), obj), onl.order.end());
	}
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
	auto& cllvl = levelObjects.categories[clcat].type[clid];
	auto& cllvlobjs = cllvl.objects;
	if (objnb < cllvlobjs.size())
		return cllvlobjs[objnb];
	else if(cllvl.info == 2) {
		assert(sector != -1);
		auto &strtype = sectorObjects[sector].categories[clcat].type[clid];
		return strtype.objects[objnb - strtype.startId];
		//return loadMap.at(std::make_pair(objid, sector));
	}
	else {
		// find sector
		for (auto& str : sectorObjects) {
			auto& clstr = str.categories[clcat].type[clid];
			int x = (int)objnb - (int)clstr.startId;
			if (x >= 0 && x < (int)clstr.objects.size())
				return clstr.objects[x];
		}
		assert(false);
		return nullptr; // silence the linker warning
	}
}

CKObject* KEnvironment::getObjPnt(const kuuid& uuid)
{
	auto it = levelUuidMap.find(uuid);
	if (it != levelUuidMap.end())
		return it->second;
	it = globalUuidMap.find(uuid);
	assert(it != globalUuidMap.end());
	return it->second;
}

CKObject * KEnvironment::readObjPnt(File * file, int sector)
{
	uint32_t objid = file->readUint32();
	if (objid == 0xFFFFFFFD) {
		kuuid uid;
		file->read(uid.data(), 16);
		return getObjPnt(uid);
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

CKObject * KEnvironment::getGlobal(uint32_t clfid)
{
	for (CKObject *obj : globalObjects)
		if (obj->getClassFullID() == clfid)
			return obj;
	return nullptr;
}

int KEnvironment::getObjectSector(CKObject* obj) const
{
	// TODO: find in globals
	int clfid = obj->getClassFullID();
	auto findAt = [this, clfid, obj](const KObjectList& objlist) -> bool {
		const auto& objs = objlist.getClassType(clfid).objects;
		return std::find(objs.begin(), objs.end(), obj) != objs.end();
	};
	if (findAt(levelObjects))
		return -1;
	for (int i = 0; i < (int)numSectors; ++i)
		if (findAt(sectorObjects[i]))
			return i;
	assert(false);
	return -9;
}

KEnvironment::ObjNameList::ObjInfo& KEnvironment::makeObjInfo(CKObject* obj)
{
	int str = getObjectSector(obj);
	ObjNameList& nlist = (str >= 0) ? sectorObjNames[str] : ((str == -1) ? levelObjNames : globalObjNames);
	auto [it, inserted] = nlist.dict.try_emplace(obj);
	if (inserted)
		nlist.order.push_back(obj);
	return it->second;
}

const char * KEnvironment::getObjectName(CKObject * obj) const
{
	auto it = globalObjNames.dict.find(obj);
	if (it != globalObjNames.dict.end())
		return it->second.name.c_str();
	it = levelObjNames.dict.find(obj);
	if (it != levelObjNames.dict.end())
		return it->second.name.c_str();
	for (auto &str : sectorObjNames) {
		it = str.dict.find(obj);
		if (it != str.dict.end())
			return it->second.name.c_str();
	}
	return "?";
}

void KEnvironment::setObjectName(CKObject* obj, std::string name)
{
	auto& info = makeObjInfo(obj);
	info.name = std::move(name);
}

KEnvironment::ObjNameList::ObjInfo& KEnvironment::ObjNameList::getObjInfoRef(CKObject* obj)
{
	auto [it, isNew] = dict.try_emplace(obj);
	if (isNew)
		order.push_back(obj);
	return it->second;
}
