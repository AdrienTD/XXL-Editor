#pragma once

#include "KObject.h"
#include <vector>
#include <array>
#include "vecmat.h"
#include "Shape.h"
#include "Events.h"
#include "DynArray.h"

struct CKHook;
struct CKHookLife;
struct CKGroup;
struct CKGroupLife;
struct CKCinematicNode;
struct CKCinematicDoor;
struct CKStartDoor;
struct CAnimationDictionary;
struct CKSoundDictionaryID;
struct CKSceneNode;
struct CSGSectorRoot;
struct CKMeshKluster;
struct CKBeaconKluster;
struct CKPFGraphNode;
struct CKCinematicSceneData;

struct CKLogic : CKCategory<12> {};

struct CKPFGraphTransition : CKSubclass<CKLogic, 2> {
	uint32_t unk1; // XXL1: byte with value 0/1, XXL2+: u32 with first bit set/clear, remaining bits are random
	kobjref<CKPFGraphNode> node;
	uint32_t unk2;
	float x2UnkA = 1.0f, ogUnkB = 0.2f, ogUnkC = 1.0f; // TODO: ogUnkC before or after x2UnkA?
	struct Thing {
		std::array<float, 12> matrix;
		uint32_t unk;
	};
	std::vector<Thing> things;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBundle : CKSubclass<CKLogic, 3> {
	kobjref<CKBundle> next;
	uint8_t flags;
	kobjref<CKGroupLife> grpLife;
	kobjref<CKHookLife> firstHookLife, otherHookLife;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSector : CKSubclass<CKLogic, 4> {
	kobjref<CKObject> sgRoot;
	uint16_t strId, unk1;
	//uint32_t numSas;
	std::vector<kobjref<CKObject>> sases;
	kobjref<CKObject> soundDictionary;
	kobjref<CKBeaconKluster> beaconKluster;
	kobjref<CKMeshKluster> meshKluster;
	AABoundingBox boundaries;
	EventNode evt1, evt2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKChoreoKey : CKSubclass<CKLogic, 15> {
	//uint32_t numSlots;
	struct ChoreoSlot {
		Vector3 position = Vector3(0,0,0), direction = Vector3(1,0,0);
		uint8_t enemyGroup = 255;
	};
	std::vector<ChoreoSlot> slots;
	float unk1, unk2, unk3;
	uint16_t flags;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKPFGraphNode : CKSubclass<CKLogic, 16> {
	Vector3 lowBBCorner, highBBCorner;
	uint8_t numCellsX, numCellsZ;
	std::vector<uint8_t> cells;
	std::vector<kobjref<CKPFGraphTransition>> transitions;
	std::vector<kobjref<CKPFGraphNode>> others; // XXL1: only at most one single ref, XXL2+: multiple refs

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	float getCellWidth() const  { return (highBBCorner.x - lowBBCorner.x) / numCellsX; }
	float getCellHeight() const { return (highBBCorner.z - lowBBCorner.z) / numCellsZ; }
};

struct CKSas : CKSubclass<CKLogic, 17> {
	std::array<uint32_t, 2> sector;
	std::array<AABoundingBox, 2> boxes;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CGround : CKSubclass<CKLogic, 18> {
	struct Triangle {
		std::array<uint16_t, 3> indices;
	};
	struct InfiniteWall {
		std::array<uint16_t, 2> baseIndices;
	};
	struct FiniteWall {
		std::array<uint16_t, 2> baseIndices;
		std::array<float, 2> heights;
	};

	uint32_t numa;
	std::vector<Triangle> triangles;
	std::vector<Vector3> vertices;
	AABoundingBox aabb;
	uint16_t param1, param2;

	// ... new stuff from XXL2/OG
	uint8_t x2neoByte;
	kobjref<CKObject> x4unkRef;
	kobjref<CKSector> x2sectorObj;

	std::vector<InfiniteWall> infiniteWalls;
	std::vector<FiniteWall> finiteWalls;
	float param3, param4;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CDynamicGround : CKSubclass<CGround, 19> {
	Vector3 mpos, mrot;
	kobjref<CKSceneNode> node; uint32_t nodeId;
	Matrix transform;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment *kenv) override;
};

struct CKFlaggedPath : CKSubclass<CKLogic, 23> {
	kobjref<CKObject> line;
	uint32_t numPoints;
	float fpSomeFloat;
	std::vector<float> pntValues;
	std::vector<EventNode> pntEvents;

	// sector at which the path is used, set by onLevelLoaded from the hooks using it (assuming hooks are loaded before misc classes!)
	int usingSector = -1;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKMsgAction : CKSubclass<CKLogic, 24> {
	struct MAStruct4 {
		uint32_t type;
		union {
			uint32_t valU32;
			float valFloat;
		};
		kobjref<CKObject> ref;
	};
	struct MAStruct3 {
		uint8_t num;
		std::vector<MAStruct4> mas4;
	};
	struct MAStruct2 {
		uint32_t event;
		std::vector<MAStruct3> mas3;
	};
	struct MAStruct1 {
		std::vector<MAStruct2> mas2;
	};
	std::vector<MAStruct1> mas1;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKChoreography : CKSubclass<CKLogic, 27> {
	float unkfloat;
	uint8_t unk2;
	uint8_t numKeys;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKLine : CKSubclass<CKLogic, 30> {
	uint8_t numSegments;
	float somenum;
	std::vector<Vector3> points;
	std::vector<float> segmentWeights;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSpline4L : CKSubclass<CKLogic, 31> {
	uint8_t unkchar1;
	float unkfloat1, unkfloat2;
	uint8_t unkchar2;
	uint32_t numBings;
	std::vector<Vector3> bings;
	uint32_t numDings;
	std::vector<Vector3> dings;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKCinematicScene : CKSubclass<CKLogic, 37> {
	uint16_t csUnk1;
	std::vector<kobjref<CKCinematicSceneData>> cineDatas;
	std::vector<kobjref<CKCinematicNode>> cineNodes;
	kobjref<CKStartDoor> startDoor;
	uint8_t csUnk2;
	uint32_t csUnk3; float csUnk4, csUnk5, csUnk6, csUnk7, csUnk8, csUnk9, csUnkA;
	EventNode onSomething;
	std::vector<kobjref<CKObject>> groups;
	kobjref<CKSoundDictionaryID> sndDict;
	uint8_t csUnkF; std::array<uint8_t, 19> otherUnkF;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKCinematicSceneData : CKSubclass<CKLogic, 42> {
	kobjref<CKHook> hook;
	kobjref<CAnimationDictionary> animDict;
	uint8_t csdUnkA;
	uint32_t csdUnkB; // looks special

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKMeshKluster : CKSubclass<CKLogic, 66> {
	AABoundingBox aabb;
	//uint16_t numGrounds, numWalls, unk2;
	std::vector<kobjref<CGround>> grounds;
	std::vector<kobjref<CKObject>> walls;
	kobjref<CKObject> unkRef;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBeaconKluster : CKSubclass<CKLogic, 73> {
	struct Beacon {
		int16_t posx, posy, posz; uint16_t params;
		Vector3 getPosition() { return Vector3(posx, posy, posz) * 0.1f; }
		void setPosition(const Vector3 &ts) { posx = (int16_t)(ts.x * 10); posy = (int16_t)(ts.y * 10); posz = (int16_t)(ts.z * 10); }
	};
	struct Bing {
		bool active;
		uint32_t numBeacons;
		uint8_t unk2a, numBits, handlerId;
		uint16_t sectorIndex; uint32_t klusterIndex; uint16_t handlerIndex;
		uint16_t bitIndex;
		kobjref<CKObject> handler;
		uint32_t unk6;	// class ID? (12,74), (12,78)
		std::vector<Beacon> beacons;
	};
	kobjref<CKBeaconKluster> nextKluster;
	BoundingSphere bounds;
	uint16_t numUsedBings;
	std::vector<Bing> bings;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKTrigger : CKSubclass<CKLogic, 142> {
	struct Action {
		kobjref<CKObject> target;
		uint16_t event;
		uint32_t valType;
		union {
			uint8_t valU8;		// 0
			uint32_t valU32;	// 1
			float valFloat;		// 2
		};
		kobjref<CKObject> valRef; // 3
	};
	kobjref<CKObject> condition;
	std::vector<Action> actions;

	// OG+:
	std::vector<kobjref<CKObject>> ogDatas;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKTriggerDomain : CKSubclass<CKLogic, 163> {
	kobjref<CKObject> unkRef;
	uint32_t activeSector;
	uint32_t flags;
	std::vector<kobjref<CKTriggerDomain>> subdomains;
	std::vector<kobjref<CKTrigger>> triggers;

	// OG+:
	kobjref<CKObject> triggerSynchro;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGameState : CKSubclass<CKLogic, 203> {
	template <typename DataType> struct StateValue {
		kobjref<CKObject> object;
		DataType data;
	};

	std::string gsName;
	uint32_t gsUnk1;
	uint32_t gsStructureRef; // could be a kobjref, but big problem when loading from GAME.KWN!
	kobjref<CKObject> gsSpawnPoint;

	std::vector<StateValue<uint8_t>> gsStages, gsModules;

	std::vector<std::vector<StateValue<DynArray<uint8_t>>>> lvlValuesArray;

	void readSVV8(KEnvironment *kenv, File *file, std::vector<StateValue<uint8_t>> &list, bool hasByte);
	void writeSVV8(KEnvironment *kenv, File *file, std::vector<StateValue<uint8_t>> &list, bool hasByte);

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void deserializeLvlSpecific(KEnvironment* kenv, File *file, size_t length) override;
	void serializeLvlSpecific(KEnvironment* kenv, File *file) override;
	void resetLvlSpecific(KEnvironment *kenv) override;
};

struct CKA2GameState : CKSubclass<CKGameState, 222> {
	std::vector<StateValue<uint8_t>> gsDiamondHelmets, gsVideos, gsShoppingAreas;
	uint32_t gsUnk2;
	std::array<uint8_t, 0x23> gsRemainder;

	CKA2GameState() { lvlValuesArray.resize(5); }
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void resetLvlSpecific(KEnvironment *kenv) override;
};

struct CKA3GameState : CKSubclass<CKGameState, 341> {
	std::vector<StateValue<uint8_t>> gsVideos, gsUnkObjects;
	kobjref<CKObject> gsStdText;
	std::vector<StateValue<uint8_t>> gsBirdZones, gsBirdlimes, gsShields, gsTalcs;
	std::array<uint8_t, 13> gsRemainder;

	CKA3GameState() { lvlValuesArray.resize(8); }
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void resetLvlSpecific(KEnvironment *kenv) override;
};