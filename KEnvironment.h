#pragma once

#include <vector>
#include <string>
#include <map>
#include <array>
#include "KObject.h"

//struct kuuid {
//	std::array<uint8_t, 16> content;
//};
typedef std::array<uint8_t, 16> kuuid;

enum class KInstantiation : uint8_t {
	Globally = 0, // use game/global instances (mainly for singletons)
	LevelUnique = 1, // each object has its own instance in the level (even when coming from different sectors)
	SectorShared = 2, // instances are shared between objects of different sectors
	Invalid = 255
};

struct KObjectList {
	struct ClassType {
		std::vector<CKObject*> objects;
		uint16_t totalCount, startId;
		KInstantiation instantiation = KInstantiation::Invalid;
		// Only in XXL2+:
		std::vector<kuuid> globUuids;
		uint8_t globByte = 0;
	};
	struct Category {
		std::vector<ClassType> type;
	};
	std::array<Category, 15> categories;
	ClassType &getClassType(int clsCategory, int clsId) { return categories[clsCategory].type[clsId]; }
	ClassType &getClassType(int clsFullId) { return categories[clsFullId & 63].type[clsFullId >> 6]; }
	template <class T> ClassType &getClassType() { return categories[T::CATEGORY].type[T::CLASS_ID]; }
	const ClassType& getClassType(int clsCategory, int clsId) const { return categories[clsCategory].type[clsId]; }
	const ClassType& getClassType(int clsFullId) const { return categories[clsFullId & 63].type[clsFullId >> 6]; }
	template <class T> const ClassType& getClassType() const { return categories[T::CATEGORY].type[T::CLASS_ID]; }
	template <class T> T* getObject(size_t i) const { return (T*)categories[T::CATEGORY].type[T::CLASS_ID].objects[i]; }
	template <class T> T* getFirst() const {
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
		PLATFORM_PSP = 4,
		PLATFORM_WII = 5,
		PLATFORM_X360 = 6,
		PLATFORM_PS3 = 7
	};
	enum {
		KVERSION_UNKNOWN = 0,
		KVERSION_XXL1 = 1,
		KVERSION_XXL2 = 2,
		KVERSION_ARTHUR = 3,
		KVERSION_OLYMPIC = 4,
		KVERSION_SPYRO = 5,
		KVERSION_ALICE = 6,
		KVERSION_HTTYD = 7
	};
	static const char * platformExt[8];

	int version, platform;
	std::map<uint32_t, KFactory> factories;
	std::array<int, 15> clcatReorder = { 0,9,1,2,3,4,5,6,7,8,10,11,12,13,14 };
	bool isXXL2Demo = false, isRemaster = false;

	std::string gamePath, outGamePath;
	std::vector<CKObject*> globalObjects;
	kuuid gameManagerUuid;
	std::map<kuuid, CKObject*> globalUuidMap;

	KObjectList levelObjects;
	std::vector<KObjectList> sectorObjects;
	unsigned int numSectors = 0;
	uint32_t lvlUnk1, lvlUnk2;
	unsigned int loadingSector;
	bool levelLoaded = false;
	std::map<kuuid, CKObject*> levelUuidMap;

	//std::map<uint32_t, uint16_t> strLoadStartIDs;
	std::map<CKObject*, uint32_t> saveMap;
	std::map<CKObject*, kuuid> saveUuidMap;

	struct ObjNameList {
		struct ObjInfo {
			std::string name;
			uint32_t anotherId;
			kobjref<CKObject> user; // hook for shadowcpnt
			kobjref<CKObject> user2; // scene node for geometry
		};
		std::map<CKObject*, ObjInfo> dict;
		std::vector<CKObject*> order;
		void clear() { dict.clear(); order.clear(); }
		ObjInfo& getObjInfoRef(CKObject* obj);
	};
	ObjNameList globalObjNames, levelObjNames;
	std::vector<ObjNameList> sectorObjNames;

	void loadGame(const char *path, int version, int platform, bool isRemaster = false);
	void saveGameFile();
	void loadLevel(int lvlNumber);
	void saveLevel(int lvlNumber);
	bool loadSector(int strNumber, int lvlNumber);
	void saveSector(int strNumber, int lvlNumber);
	void prepareSavingMap();
	void loadAddendum(int lvlNumber);
	void saveAddendum(int lvlNumber);
	void unloadLevel();
	void unloadGame();
	~KEnvironment() { unloadGame(); }

	//const KFactory &getFactory(uint32_t fid) const;
	//const KFactory &getFactory(int clcat, int clid) const { return getFactory(clcat | (clid << 6)); }
	CKObject *constructObject(uint32_t fid);
	CKObject *constructObject(int clcat, int clid) { return constructObject(clcat | (clid << 6)); }
	CKObject *createObject(uint32_t fid, int sector);
	CKObject *createObject(int clcat, int clid, int sector) { return createObject(clcat | (clid << 6), sector); }
	template<class T> T *createObject(int sector) { return (T*)createObject(T::FULL_ID, sector); }
	template<class T> T *createAndInitObject(int sector = -1) { T *obj = createObject<T>(sector); obj->init(this); return obj; }
	template<class T> T* cloneObject(const T* original, int sector = -1) { return cloneObject<CKObject>(original, sector)->cast<T>(); }
	template<> CKObject* cloneObject<CKObject>(const CKObject* original, int sector) {
		uint32_t fid = ((CKObject*)original)->getClassFullID();
		CKObject* clone = createObject(fid, sector);
		factories.at(fid).copy(original, clone);
		return clone;
	}

	void removeObject(CKObject *obj);

	CKObject *getObjPnt(uint32_t objid, int sector = -1);
	CKObject* getObjPnt(const kuuid& uuid);
	CKObject *readObjPnt(File *file, int sector = -1);
	uint32_t getObjID(CKObject *obj);
	void writeObjID(File *file, CKObject *obj);
	template<class T> kobjref<T> getObjRef(uint32_t objid, int sector = -1) { return kobjref<T>((T*)getObjPnt(objid, sector)); }
	template<class T> kobjref<T> getObjRef(const kuuid& uuid) { return kobjref<T>((T*)getObjPnt(uuid)); }
	template<class T> kobjref<T> readObjRef(File *file, int sector = -1) { return kobjref<T>((T*)readObjPnt(file, sector)); }
	template<class T> void writeObjRef(File *file, const kobjref<T> &ref) { writeObjID(file, ref.get()); }

	template<class T> void addFactory() {
		auto [it, inserted] = factories.insert_or_assign(T::FULL_ID, KFactory::of<T>());
		assert(inserted);
	}
	template<class T> bool hasClass() { return factories.count(T::FULL_ID) != 0; }

	CKObject *getGlobal(uint32_t clfid);
	template<class T> T *getGlobal() { return (T*)getGlobal(T::FULL_ID); }

	int getObjectSector(CKObject* obj) const;

	ObjNameList::ObjInfo& makeObjInfo(CKObject* obj);
	const char* getObjectName(CKObject* obj) const;
	void setObjectName(CKObject* obj, std::string name);

	bool isUsingNewFilenames() const { return version >= KVERSION_SPYRO || (version == KVERSION_OLYMPIC && platform == PLATFORM_X360); }

	KObjectList& getObjectList(int streamIndex) { return (streamIndex == -1) ? levelObjects : sectorObjects[streamIndex]; }
};

