#include "KEnvironment.h"
#include "File.h"
#include "KObject.h"
#include <cassert>

void KEnvironment::loadGame(const char * path, int version)
{
	this->gamePath = path;
	this->version = version;

	char gamefn[300];
	snprintf(gamefn, sizeof(gamefn), "%s/GAME.%s", gamePath.c_str(), "KWN");

	IOFile gameFile(gamefn, "rb");
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

void KEnvironment::loadLevel(int lvlNumber)
{
	char lvlfn[300];
	snprintf(lvlfn, sizeof(lvlfn), "%s/LVL%03u/LVL%02u.%s", gamePath.c_str(), lvlNumber, lvlNumber, "KWN");

	IOFile lvlFile(lvlfn, "rb");
	uint32_t unk1 = lvlFile.readUint32();
	lvlFile.readUint32();
	uint8_t unk2 = lvlFile.readUint8();
	uint32_t unk3 = lvlFile.readUint32();

	for (int clcat = 0; clcat < 15; clcat++) {
		uint16_t numClasses = lvlFile.readUint16();
		this->levelObjects.categories[clcat].type.resize(numClasses);
		for (uint16_t clid = 0; clid < numClasses; clid++) {
			uint32_t fid = clcat | (clid << 6);
			uint16_t numTotalObjects = lvlFile.readUint16();
			uint16_t numLevelObjects = lvlFile.readUint16();
			uint8_t info = lvlFile.readUint8();
			this->levelObjects.categories[clcat].type[clid].info = info;
			this->levelObjects.categories[clcat].type[clid].totalCount = numTotalObjects;
			this->levelObjects.categories[clcat].type[clid].objects.reserve(numLevelObjects);
			for (uint16_t i = 0; i < numLevelObjects; i++) {
				this->levelObjects.categories[clcat].type[clid].objects.push_back(constructObject(fid));
			}
		}
	}

	for (int clcat = 0; clcat < 15; clcat++) {
		auto &type = this->levelObjects.categories[clcat].type;
		for (size_t clid = 0; clid < type.size(); clid++) {
			printf("Class (%i,%i) : %i %i %i\n", clcat, clid, type[clid].totalCount,  type[clid].objects.size(), type[clid].info);
		}
	}

	lvlFile.seek(8 + 4/*drm*/, SEEK_CUR);

	for (int clcat : clcatReorder) {
		uint16_t numClasses = lvlFile.readUint16();
		uint32_t nextCat = lvlFile.readUint32();
		printf("Cat %i at %08X, next at %08X, numClasses = %i\n", clcat, lvlFile.tell(), nextCat, numClasses);
		for (size_t clid = 0; clid < this->levelObjects.categories[clcat].type.size(); clid++) {
			if (this->levelObjects.categories[clcat].type[clid].objects.empty())
				continue;
			uint32_t nextClass = lvlFile.readUint32();
			printf("Class %i %i at %08X, next at %08X\n", clcat, clid, lvlFile.tell(), nextClass);
			if (this->levelObjects.categories[clcat].type[clid].info) {
				uint16_t startid = lvlFile.readUint16();
				assert(startid == 0);
			}
			printf("* %08X\n", lvlFile.tell());
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
}

//const KFactory & KEnvironment::getFactory(uint32_t fid) const {
//	static KFactory unkfactory = KFactory::of<CKUnknown>();
//	auto it = factories.find(fid);
//	if (it != factories.end())
//		return factories.at(fid);
//	else
//		return unkfactory;
//}

CKObject * KEnvironment::constructObject(uint32_t fid)
{
	auto it = factories.find(fid);
	if (it != factories.end())
		return factories.at(fid).create();
	else
		return new CKUnknown(fid & 63, fid >> 6);
}

CKObject * KEnvironment::getObjPnt(uint32_t objid, int sector)
{
	int clcat = objid & 63;
	int clid = (objid >> 6) & 2047;
	int objnb = objid >> 17;
	return levelObjects.categories[clcat].type[clid].objects[objnb];
}

CKObject * KEnvironment::readObjPnt(File * file, int sector)
{
	return getObjPnt(file->readUint32(), sector);
}
