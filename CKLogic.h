#pragma once

#include "KObject.h"
#include <vector>
#include <array>
#include "vecmat.h"
#include "Shape.h"
#include "Events.h"
#include "DynArray.h"
#include "CKUtils.h"

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
struct CKServiceLife;
struct CKGroupRoot;
struct CKTriggerSynchro;
struct CBillboard2d;
struct CNode;
struct CAnimatedNode;
struct CParticlesNodeFx;

struct CKLogic : CKCategory<12> {};

struct CKReflectableLogic : CKMRSubclass<CKReflectableLogic, CKMemberReflectable<CKLogic>, 0xBADB01> {
	void reflectMembers(MemberListener &r) {}
};

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
	uint16_t strId = 0, unk1 = 1;
	//uint32_t numSas;
	std::vector<kobjref<CKObject>> sases;
	kobjref<CKObject> soundDictionary;
	kobjref<CKBeaconKluster> beaconKluster;
	kobjref<CKMeshKluster> meshKluster;
	AABoundingBox boundaries;
	EventNode evt1, evt2;

	// XXL2+:
	std::vector<kobjref<CKObject>> x2compdatas1, x2compdatas2;
	kobjref<CKObject> x2sectorDetector;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKLevel : CKSubclass<CKLogic, 5> {
	uint32_t lvlNumber;
	std::vector<kobjref<CKSector>> sectors;
	std::vector<kobjref<CKObject>> objs;
	std::array<uint32_t, 20> initialSector = { 0 };
	std::array<std::string, 20> lvlRemasterCheatSpawnNames;
	uint8_t lvlUnk2 = 0;
	uint32_t lvlUnk3 = 0x107;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKCoreManager : CKSubclass<CKLogic, 9> {
	kobjref<CKGroupRoot> groupRoot;
	kobjref<CKServiceLife> srvLife;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void init(KEnvironment *kenv) override;
};

struct CKChoreoKey : CKSubclass<CKLogic, 15> {
	//uint32_t numSlots;
	struct ChoreoSlot {
		Vector3 position = Vector3(0,0,0), direction = Vector3(1,0,0);
		int16_t enemyGroup = -1;
	};
	std::vector<ChoreoSlot> slots;
	float unk1 = 5.0f, unk2 = 10.0f, unk3 = 0.5f;	// Only in XXL1
	float x2unk1 = 3.0f;							// Only in XXL2+
	uint16_t flags = 0;

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
	float unkfloat = 0.0f;
	uint8_t unk2 = 0;
	uint8_t numKeys = 0;

	// XXL2+ have pointers to ChoreoKeys inside CKCheoreography, whereas XXL1 has them in the Squad
	std::vector<kobjref<CKChoreoKey>> keys;

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

struct CKDefaultGameManager : CKMRSubclass<CKDefaultGameManager, CKReflectableLogic, 49> {
	kobjref<CKObject> dgmGrpEnemy;
	kobjref<CKObject> dgmGrpTrio;
	kobjref<CKObject> dgmGrpMeca;
	kobjref<CKObject> dgmGrpBonus;
	void reflectMembers(MemberListener &r);
};
struct CKAsterixGameManager : CKMRSubclass<CKAsterixGameManager, CKDefaultGameManager, 51> {
	std::array<uint8_t, 0x44> dgmGlobalBytes;
	kobjref<CKObject> dgmBillboard;
	EventNode dgmUnk1;
	EventNode dgmUnk2;
	EventNode dgmUnk3;
	EventNode dgmUnk4;
	EventNode dgmUnk5;
	EventNode dgmUnk6;
	EventNode dgmUnk7;
	EventNode dgmUnk8;
	EventNode dgmUnk9;
	EventNode dgmUnk10;
	EventNode dgmUnk11;
	EventNode dgmUnk12;
	EventNode dgmUnk13;
	EventNode dgmUnk14;
	EventNode dgmUnk15;
	EventNode dgmUnk16;
	EventNode dgmUnk17;
	EventNode dgmUnk18;
	EventNode dgmUnk19;
	EventNode dgmUnk20;
	EventNode dgmUnk21;
	EventNode dgmUnk22;
	EventNode dgmUnk23;
	EventNode dgmUnk24;
	EventNode dgmUnk25;
	EventNode dgmUnk26;
	EventNode dgmUnk27;
	EventNode dgmUnk28;
	EventNode dgmUnk29;
	EventNode dgmUnk30;
	EventNode dgmUnk31;
	EventNode dgmUnk32;
	EventNode dgmUnk33;
	EventNode dgmUnk34;
	EventNode dgmUnk35;
	EventNode dgmUnk36;
	EventNode dgmUnk37;
	EventNode dgmUnk38;
	EventNode dgmUnk39;
	EventNode dgmUnk40;
	EventNode dgmUnk41;
	EventNode dgmUnk42;
	EventNode dgmUnk43;
	EventNode dgmUnk44;
	EventNode dgmUnk45;
	EventNode dgmUnk46;
	EventNode dgmUnk47;
	EventNode dgmUnk48;
	EventNode dgmUnk49;
	EventNode dgmUnk50;
	EventNode dgmUnk51;
	EventNode dgmUnk52;
	EventNode dgmUnk53;
	EventNode dgmUnk54;
	EventNode dgmUnk55;
	EventNode dgmUnk56;
	EventNode dgmUnk57;
	EventNode dgmUnk58;
	EventNode dgmUnk59;
	EventNode dgmUnk60;
	EventNode dgmUnk61;
	EventNode dgmUnk62;
	EventNode dgmUnk63;
	EventNode dgmUnk64;
	EventNode dgmUnk65;
	EventNode dgmUnk66;
	EventNode dgmUnk67;
	EventNode dgmUnk68;
	EventNode dgmUnk69;
	EventNode dgmUnk70;
	EventNode dgmUnk71;
	EventNode dgmUnk72;
	EventNode dgmUnk73;
	EventNode dgmUnk74;
	EventNode dgmUnk75;
	EventNode dgmUnk76;
	EventNode dgmUnk77;
	EventNode dgmUnk78;
	EventNode dgmUnk79;
	EventNode dgmUnk80;
	EventNode dgmUnk81;
	EventNode dgmUnk82;
	EventNode dgmUnk83;
	EventNode dgmUnk84;
	EventNode dgmUnk85;
	EventNode dgmUnk86;
	EventNode dgmUnk87;
	EventNode dgmUnk88;
	EventNode dgmUnk89;
	EventNode dgmUnk90;
	EventNode dgmUnk91;
	EventNode dgmUnk92;
	EventNode dgmUnk93;
	EventNode dgmUnk94;
	EventNode dgmUnk95;
	EventNode dgmUnk96;
	EventNode dgmUnk97;
	EventNode dgmUnk98;
	EventNode dgmUnk99;
	EventNode dgmUnk100;
	EventNode dgmUnk101;
	EventNode dgmUnk102;
	EventNode dgmUnk103;
	kobjref<CKObject> dgmUnk104;
	kobjref<CKObject> dgmCamera;
	EventNode dgmUnk106;
	kobjref<CKObject> dgmUnk107;
	kobjref<CKObject> dgmUnk108;
	kobjref<CKObject> dgmUnk109;
	float dgmUnk110 = 3.0f;
	uint32_t dgmUnk111 = -1; // makes CMenuManager2d work
	EventNode dgmUnk112;
	uint8_t dgmUnk113 = 20;
	float dgmUnk114 = 1.0f;
	EventNode dgmUnk115;
	kobjref<CKObject> dgmGrpCheckpoint;
	EventNode dgmUnk117;
	EventNode dgmUnk118;
	uint32_t dgmDrmValue = 0x24011980;
	uint32_t dgmUnk120 = 0xFFFFFFFF;
	uint32_t dgmUnk121 = 0xFFFFFFFF;
	kobjref<CKObject> dgmUnk122;
	float dgmUnk123 = 4.0f;
	float dgmUnk124 = 100.0f;
	float dgmUnk125 = 1.0f;
	float dgmUnk126 = 1.0f;
	std::string dgmUnk127;
	uint32_t dgmUnk128 = 0xFFFAFAFA;
	std::string dgmUnk129;
	float dgmUnk130 = 50.0f;
	float dgmUnk131 = 50.0f;
	float dgmUnk132 = 0.01f;
	uint8_t dgmUnk133 = 1;
	uint8_t dgmUnk134 = 1;
	float dgmUnk135 = 0.01f;
	uint32_t dgmUnk136 = 0xFFFF0000;
	uint8_t dgmUnk137 = 0;
	float dgmUnk138 = 15.0f;
	float dgmUnk139 = 6.0f;
	kobjref<CKObject> dgmSkyLife;
	float dgmUnk141 = 0.05f;
	float dgmUnk142 = 2.5f;
	float dgmUnk143 = 5.0f;
	uint8_t dgmUnk144 = 0;
	float dgmUnk145 = 1.0f;
	uint32_t dgmUnk146 = 0xCDCDCDCD;
	float dgmUnk147 = 100.0f;
	uint32_t dgmUnk148 = 0x32FF6464;
	uint32_t dgmUnk149 = 0x00ff0000;
	std::string dgmUnk150;
	uint8_t dgmUnk151 = 1;
	float dgmUnk152;
	float dgmUnk153 = 0.5f;
	kobjref<CKObject> dgmCam1;
	kobjref<CKObject> dgmCam2;
	float dgmUnk156 = 5.0f;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
};

struct CLocManager : CKSubclass<CKLogic, 59> {
	// Global only:
	uint32_t lmUnk0, numTrcStrings, lmNumDings, numStdStrings;
	std::vector<std::pair<uint32_t, uint32_t>> lmDings;

	// XXL1 PS2 + Romaster have numLanguages + language IDs in GAME.K*
	// GC+PC have them in *GLOC.K* (see Loc_CLocManager)
	uint16_t numLanguages;
	std::vector<uint32_t> langStrIndices, langIDs;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override {}
	void serialize(KEnvironment* kenv, File* file) override {}
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
};

struct CKSekens : CKMRSubclass<CKSekens, CKReflectableLogic, 61> {
	struct SLine {
		uint32_t mUnk0;
		float mUnk1;
		float mUnk2;
		void reflectMembers(MemberListener &r);
	};
	kobjref<CKObject> sekManager2d;
	kobjref<CKObject> sekSndManager;
	// uint32_t sizeFor_sekLines;
	std::vector<SLine> sekLines;
	kobjref<CKObject> sekRomaSndDictID;
	std::vector<std::string> sekRomaLineNames;
	uint32_t sekUnk4;
	uint8_t sekUnk5;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
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
		bool active = false;
		//uint32_t numBeacons;
		uint8_t unk2a, numBits, handlerId;
		uint16_t sectorIndex; uint32_t klusterIndex; uint16_t handlerIndex;
		uint16_t bitIndex;
		kobjref<CKObject> handler;
		uint32_t unk6;	// class ID? (12,74), (12,78)
		std::vector<Beacon> beacons;
	};
	kobjref<CKBeaconKluster> nextKluster;
	BoundingSphere bounds;
	uint16_t numUsedBings = 0;
	std::vector<Bing> bings;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKFlashNode2dFx : CKMRSubclass<CKFlashNode2dFx, CKReflectableLogic, 87> {
	kobjref<CBillboard2d> billboard;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKElectricArcNodeFx : CKMRSubclass<CKElectricArcNodeFx, CKReflectableLogic, 89> {
	kobjref<CNode> node;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKQuadNodeFx : CKMRSubclass<CKQuadNodeFx, CKReflectableLogic, 94> {
	kobjref<CNode> node;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKLightningObjectNodeFx : CKMRSubclass<CKLightningObjectNodeFx, CKReflectableLogic, 101> {
	kobjref<CAnimatedNode> node;
	kobjref<CAnimationDictionary> animDict;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFilterNode2dFx : CKMRSubclass<CKFilterNode2dFx, CKReflectableLogic, 103> {
	kobjref<CBillboard2d> billboard;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKExplosionNodeFx : CKMRSubclass<CKExplosionNodeFx, CKReflectableLogic, 105> {
	kobjref<CNode> node, node2;
	kobjref<CParticlesNodeFx> partNode;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKTrigger : CKSubclass<CKLogic, 142> {
	struct Action {
		KPostponedRef<CKObject> target;
		uint16_t event;
		uint32_t valType;
		union {
			uint8_t valU8;		// 0
			uint32_t valU32;	// 1
			float valFloat;		// 2
		};
		KPostponedRef<CKObject> valRef; // 3
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
	kobjref<CKTriggerSynchro> triggerSynchro;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment* kenv) override;
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

struct CKTriggerSynchro : CKSubclass<CKLogic, 347> {
	struct SynchroElement {
		// u32cnt
		std::vector<kobjref<CKTriggerDomain>> domains;
		uint32_t syeunk;
	};
	// u32cnt
	std::vector<SynchroElement> elements;
	kobjref<CKObject> syncModule;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};