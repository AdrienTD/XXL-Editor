#pragma once

#include <vector>
#include <string>
#include <map>
#include <array>
#include "KObject.h"

struct KObjectList {
	struct ClassType {
		std::vector<CKObject*> objects;
		uint16_t totalCount, startId;
		uint8_t info;
	};
	struct Category {
		std::vector<ClassType> type;
	};
	std::array<Category, 15> categories;
	ClassType &getClassType(int clsCategory, int clsId) { return categories[clsCategory].type[clsId]; }
	ClassType &getClassType(int clsFullId) { return categories[clsFullId & 63].type[clsFullId >> 6]; }
	template <class T> ClassType &getClassType() { return categories[T::CATEGORY].type[T::CLASS_ID]; }
	template <class T> T *getObject(size_t i) { return (T*)categories[T::CATEGORY].type[T::CLASS_ID].objects[i]; }
	template <class T> T *getFirst() {
		auto &objs = categories[T::CATEGORY].type[T::CLASS_ID].objects;
		if(objs.size() > 0)
			return (T*)objs[0];
		return nullptr;
	}
};

struct KEnvironment {
	enum {
		PLATFORM_UNKNOWN = 0,
		PLATFORM_PC = 1,
		PLATFORM_PS2 = 2,
		PLATFORM_GCN = 3,
	};
	static const char * platformExt[4];

	int version, platform;
	std::map<uint32_t, KFactory> factories;
	std::array<int, 15> clcatReorder = { 0,9,1,2,3,4,5,6,7,8,10,11,12,13,14 };

	std::string gamePath;
	std::vector<CKObject*> globalObjects;

	KObjectList levelObjects;
	std::vector<KObjectList> sectorObjects;
	unsigned int numSectors;
	uint32_t lvlUnk1, lvlUnk2;
	unsigned int loadingSector;
	bool levelLoaded = false;

	//std::map<uint32_t, uint16_t> strLoadStartIDs;
	std::map<CKObject*, uint32_t> saveMap;

	void loadGame(const char *path, int version, int platform);
	void loadLevel(int lvlNumber);
	void saveLevel(int lvlNumber);
	bool loadSector(int strNumber, int lvlNumber);
	void saveSector(int strNumber, int lvlNumber);
	void prepareLoadingMap();
	void prepareSavingMap();
	void unloadLevel();

	//const KFactory &getFactory(uint32_t fid) const;
	//const KFactory &getFactory(int clcat, int clid) const { return getFactory(clcat | (clid << 6)); }
	CKObject *constructObject(uint32_t fid);
	CKObject *constructObject(int clcat, int clid) { return constructObject(clcat | (clid << 6)); }
	CKObject *createObject(uint32_t fid, int sector);
	CKObject *createObject(int clcat, int clid, int sector) { return createObject(clcat | (clid << 6), sector); }
	template<class T> T *createObject(int sector) { return (T*)createObject(T::FULL_ID, sector); }

	void removeObject(CKObject *obj);

	CKObject *getObjPnt(uint32_t objid, int sector = -1);
	CKObject *readObjPnt(File *file, int sector = -1);
	uint32_t getObjID(CKObject *obj);
	void writeObjID(File *file, CKObject *obj);
	template<class T> kobjref<T> getObjRef(uint32_t objid, int sector = -1) { return kobjref<T>((T*)getObjPnt(objid, sector)); }
	template<class T> kobjref<T> readObjRef(File *file, int sector = -1) { return kobjref<T>((T*)readObjPnt(file, sector)); }
	template<class T> void writeObjRef(File *file, const kobjref<T> &ref) { writeObjID(file, ref.get()); }

	template<class T> void addFactory() { factories[T::FULL_ID] = KFactory::of<T>(); }
};

