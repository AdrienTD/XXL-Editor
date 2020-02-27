#pragma once

#include <vector>
#include <string>
#include <map>
#include <array>
#include "KObject.h"

struct KObjectList {
	struct ClassType {
		std::vector<CKObject*> objects;
		uint16_t totalCount;
		uint8_t info;
	};
	struct Category {
		std::vector<ClassType> type;
	};
	Category categories[15];
};

struct KEnvironment {
	int version;
	std::map<uint32_t, KFactory> factories;

	std::string gamePath;
	std::vector<CKObject*> globalObjects;
	KObjectList levelObjects;
	std::vector<KObjectList> sectorObjects;

	std::array<int, 15> clcatReorder = {0,9,1,2,3,4,5,6,7,8,10,11,12,13,14};

	void loadGame(const char *path, int version);
	void loadLevel(int lvlNumber);
	void saveLevel(int lvlNumber);

	//const KFactory &getFactory(uint32_t fid) const;
	//const KFactory &getFactory(int clcat, int clid) const { return getFactory(clcat | (clid << 6)); }
	CKObject *constructObject(uint32_t fid);
	CKObject *constructObject(int clcat, int clid) { return constructObject(clcat | (clid << 6)); }

	CKObject *getObjPnt(uint32_t objid, int sector);
	CKObject *readObjPnt(File *file, int sector);
	template<class T> objref<T> getObjRef(uint32_t objid, int sector) { return objref<T>((T*)getObjPnt(objid, sector)); }
	template<class T> objref<T> readObjRef(File *file, int sector) { return objref<T>((T*)readObjPnt(file, sector)); }
};

