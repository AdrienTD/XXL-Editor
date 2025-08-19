#pragma once

#include "KObject.h"
#include <vector>
#include <array>
#include "vecmat.h"
#include "Shape.h"
#include "Events.h"
#include "DynArray.h"
#include "CKUtils.h"
#include <variant>

struct CKHook;
struct CKHookLife;
struct CKGroup;
struct CKGroupLife;
struct CKCinematicNode;
struct CKCinematicDoor;
struct CKStartDoor;
struct CAnimationDictionary;
struct CKSoundDictionary;
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
struct CKShadowCpnt;
struct CClone;
struct CTrailNodeFx;
struct CKCamera;
struct CKSekensEntry;
struct CKSas;

// Default-construct the variant's holding value with specified type index
// if holding value's type is different, else keep the value unchanged.
template<typename T, size_t N = 0> void changeVariantType(T& var, size_t index) {
	if constexpr (N < std::variant_size_v<T>) {
		if (index == N) {
			if (var.index() != N)
				var.emplace<N>();
		}
		else {
			changeVariantType<T, N + 1>(var, index);
		}
	}
}

struct CKLogic : CKCategory<12> {};

struct CKReflectableLogic : CKMRSubclass<CKReflectableLogic, CKMemberReflectable<CKLogic>, 0xBADB01> {
	void reflectMembers2(MemberListener &r, KEnvironment* kenv) {}
};

struct CKPFGraphTransition : CKSubclass<CKLogic, 2> {
	uint32_t unk1 = 0; // XXL1: byte with value 0/1, XXL2+: u32 with first bit set/clear, remaining bits are random
	kobjref<CKPFGraphNode> node;
	uint32_t unk2 = 0xFFFFFFFF;
	float x2UnkA = 1.0f, ogUnkB = 0.2f, ogUnkC = 1.0f; // TODO: ogUnkC before or after x2UnkA?
	struct Door {
		AABoundingBox sourceBox;
		AABoundingBox destinationBox;
		uint32_t unk = 0;
	};
	std::vector<Door> doors;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBundle : CKSubclass<CKLogic, 3> {
	kobjref<CKBundle> next;
	uint32_t priority;
	kobjref<CKGroupLife> groupLife;
	kobjref<CKHookLife> activeHookLife, inactiveHookLife;

	// XXL2+
	kobjref<CKGroup> x2Group;
	kobjref<CKHook> activeHook, inactiveHook;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSector : CKSubclass<CKLogic, 4> {
	KPostponedRef<CKObject> sgRoot;
	uint16_t strId = 0, unk1 = 1;
	//uint32_t numSas;
	std::vector<kobjref<CKSas>> sases;
	KPostponedRef<CKObject> soundDictionary;
	KPostponedRef<CKBeaconKluster> beaconKluster;
	KPostponedRef<CKMeshKluster> meshKluster;
	AABoundingBox boundaries;
	EventNode evt1, evt2;

	// XXL2+:
	kobjref<CKObject> x2sectorDetector;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKLevel : CKSubclass<CKLogic, 5> {
	uint32_t lvlNumber;
	std::vector<kobjref<CKSector>> sectors;
	std::vector<kobjref<CKObject>> objs;
	std::array<uint32_t, 20> initialSector = { 0 };
	std::array<std::string, 20> lvlRemasterCheatSpawnNames;
	uint8_t lvlUnk2 = 0;
	uint32_t lvlUnk3 = 0x107;

	// OG
	kobjref<CKTriggerSynchro> triggerSynchro;
	kuuid levelUuid;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKCameraSector : CKMRSubclass<CKCameraSector, CKReflectableLogic, 8> {
	uint8_t ckcsUnk0 = 3;
	std::array<float, 6> ckcsUnk1;
	kobjref<CKCamera> ckcsUnk2;
	float ckcsUnk3 = 1.0f;
	uint8_t ckcsUnk4 = 1;
	kobjref<CKCameraSector> ckcsUnk5; // XXL1 only

	// XXL2+
	std::vector<kobjref<CKCamera>> ckcsCameras;
	uint8_t ckcsUnk6 = 0;
	uint8_t ckcsUnk7 = 0;

	// OG
	uint32_t ckcsOgUnk1, ckcsOgUnk2;
	kobjref<CKObject> ckcsOgUnkRef;
	uint8_t ckcsOgUnk4, ckcsOgUnk5, ckcsOgUnk6, ckcsOgUnk7;
	uint32_t ckcsOgSector;
	EventNode ckcsOgEvent1, ckcsOgEvent2;
	float ckcsOgUnk9 = -1.0f;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCoreManager : CKSubclass<CKLogic, 9> {
	kobjref<CKGroupRoot> groupRoot;
	kobjref<CKServiceLife> srvLife;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void init(KEnvironment *kenv) override;
};

struct CKSpline4 : CKMRSubclass<CKSpline4, CKReflectableLogic, 11> {
	uint8_t cksNumParts = 0;
	float cksTotalLength = 0.0f;
	float cksDelta = 0.01f;
	uint8_t cksUnk3 = 1;
	std::vector<Vector3> cksPoints;
	std::vector<float> cksPartLengths; // size = cksNumParts
	std::vector<uint8_t> cksSplRangeToPartIndices;
	std::vector<float> cksSplRangeToPartRange;
	std::vector<float> cksUnk13; // size = cksNumParts
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct ICollisionMesh : CKSubclass<CKLogic, 13> {
	struct Triangle {
		std::array<uint16_t, 3> indices;
	};

	std::vector<Triangle> triangles;
	std::vector<Vector3> vertices;
	AABoundingBox aabb;
	uint16_t param1 = 0, param2 = 1;

	// ... new stuff from XXL2/OG
	uint8_t x2neoByte = 0;
	kobjref<CKObject> x4unkRef;
	kobjref<CKSector> x2sectorObj;
	uint32_t alValue = 0;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKChoreoKey : CKSubclass<CKLogic, 15> {
	//uint32_t numSlots;
	struct ChoreoSlot {
		Vector3 position = Vector3(0,0,0), direction = Vector3(0,0,1);
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
	AABoundingBox boundingBox;
	uint8_t numCellsX, numCellsZ;
	std::vector<uint8_t> cells;
	std::vector<kobjref<CKPFGraphTransition>> transitions;
	std::vector<kobjref<CKPFGraphNode>> others; // XXL1: only at most one single ref, XXL2+: multiple refs

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	float getCellWidth() const  { return (boundingBox.highCorner.x - boundingBox.lowCorner.x) / numCellsX; }
	float getCellHeight() const { return (boundingBox.highCorner.z - boundingBox.lowCorner.z) / numCellsZ; }
};

struct CKSas : CKSubclass<CKLogic, 17> {
	std::array<uint32_t, 2> sector;
	std::array<AABoundingBox, 2> boxes;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CGround : CKSubclass<ICollisionMesh, 18> {
	struct InfiniteWall {
		std::array<uint16_t, 2> baseIndices;
	};
	struct FiniteWall {
		std::array<uint16_t, 2> baseIndices;
		std::array<float, 2> heights;
	};

	std::vector<InfiniteWall> infiniteWalls;
	std::vector<FiniteWall> finiteWalls;
	float param3 = 0.0f, param4 = 0.0f;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	uint32_t computeSize() const {
		return static_cast<uint32_t>(((6 * triangles.size() + 12 * vertices.size() + 4 * infiniteWalls.size() + 12 * finiteWalls.size()) + 3) & ~3);
	}
};

struct CDynamicGround : CKSubclass<CGround, 19> {
	Vector3 mpos, mrot;
	kobjref<CKSceneNode> node; uint32_t nodeId;
	Matrix transform;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment *kenv) override;
};

struct CWall : CKSubclass<ICollisionMesh, 20> {
	Matrix wallTransform, wallTransformInv;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKFlaggedPath : CKSubclass<CKLogic, 23> {
	kobjref<CKObject> line;
	uint32_t numPoints;
	float fpSomeFloat;
	std::vector<float> pntValues;
	std::vector<EventNode> pntEvents;

	// Arthur+
	std::vector<std::pair<int32_t, float>> arPathThings;

	// sector at which the path is used, set by onLevelLoaded from the hooks using it (assuming hooks are loaded before misc classes!)
	int usingSector = -1;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKMsgAction : CKSubclass<CKLogic, 24> {
	using MAParameter = std::variant<uint32_t, uint32_t, float, kobjref<CKObject>, MarkerIndex>;
	struct MAAction {
		uint8_t num;
		std::vector<MAParameter> parameters;
	};
	struct MAMessage {
		uint32_t event;
		std::vector<MAAction> actions;
	};
	struct MAState {
		std::vector<MAMessage> messageHandlers;
		std::string name; // Addendum
	};
	std::vector<MAState> states;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	int getAddendumVersion() override;
	void deserializeAddendum(KEnvironment* kenv, File* file, int version) override;
	void serializeAddendum(KEnvironment* kenv, File* file) override;
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
	uint8_t numSegments = 0;
	float totalLength = 0.0f;
	std::vector<Vector3> points;
	std::vector<float> segmentLengths;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSpline4L : CKSubclass<CKLogic, 31> {
	uint8_t cksNumParts = 0;
	float cksTotalLength = 0.0f;
	float cksDelta = 0.01f;
	uint8_t unkchar2 = 0;
	//uint32_t numBings;
	std::vector<Vector3> cksPoints;
	//uint32_t numDings;
	std::vector<Vector3> cksPrecomputedPoints;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKCinematicScene : CKSubclass<CKLogic, 37> {
	uint16_t csFlags = 0, arcsUnk1a;
	std::vector<kobjref<CKCinematicSceneData>> cineDatas;
	std::vector<kobjref<CKCinematicNode>> cineNodes;
	kobjref<CKStartDoor> startDoor;
	uint8_t csUnk2 = 0;
	uint32_t csBarsColor = 0xFF000000; float csUnk4 = 0.2f, csUnk5 = 0.5f, csUnk6 = 0.5f, csUnk7 = 0.5f, csUnk8 = 0.5f, csUnk9 = 1.0f, csUnkA = 0.0f;
	EventNode onSceneEnded, ogOnSceneStart, spyroOnSceneSkipped;
	std::vector<kobjref<CKObject>> groups;
	KPostponedRef<CKSoundDictionaryID> sndDict;
	uint8_t csUnkF = 1;
	float x2CameraEndDuration = 0.0f;
	uint8_t arthurOnlyByte = 0;
	kobjref<CKCinematicScene> spyroSkipScene;
	std::array<uint8_t, 19> otherUnkFromRomaster;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void init(KEnvironment* kenv) override;

	size_t findEdge(CKCinematicNode* source, CKCinematicNode* dest, bool isFinish);
	std::tuple<CKCinematicNode*, CKCinematicNode*, bool> getEdgeInfo(size_t edgeIndex, KEnvironment* kenv);
	void addEdge(CKCinematicNode* source, CKCinematicNode* dest, bool isFinish, KEnvironment* kenv);
	void removeEdge(CKCinematicNode* source, CKCinematicNode* dest, bool isFinish, KEnvironment* kenv);
};

struct CLightComponent : CKMRSubclass<CLightComponent, CKReflectableLogic, 41> {
	int32_t clcUnk0;
	int32_t clcUnk1;
	float clcUnk2;
	float clcUnk3;
	std::array<float, 3> clcUnk4;

	// when clcUnk0 & 8 != 0
	float clcUnk5;
	uint32_t clcUnk6;
	//int32_t clcUnk7;
	std::vector<std::pair<float, int32_t>> fipVec;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCinematicSceneData : CKSubclass<CKLogic, 42> {
	kobjref<CKHook> hook;
	KPostponedRef<CAnimationDictionary> animDict;
	uint8_t csdUnkA;
	MarkerIndex csdUnkB; // looks special

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKDefaultGameManager : CKMRSubclass<CKDefaultGameManager, CKReflectableLogic, 49> {
	kobjref<CKObject> dgmGrpEnemy;
	kobjref<CKObject> dgmGrpTrio;
	kobjref<CKObject> dgmGrpMeca;
	kobjref<CKObject> dgmGrpBonus;
	void reflectMembers2(MemberListener &r, KEnvironment* kenv);
};
struct CKAsterixGameManager : CKMRSubclass<CKAsterixGameManager, CKDefaultGameManager, 51> {
	std::array<uint8_t, 0x44> dgmGlobalBytes;
	kobjref<CKObject> dgmBillboard;
	EventNode dgmUnk1;
	EventNode dgmUnk2;
	EventNode dgmUnk3;
	std::array<EventNode, 100> dgmRespawnedEvents;
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

struct CKVibrationData : CKMRSubclass<CKVibrationData, CKReflectableLogic, 51> {
	uint8_t ckvdUnk0;
	uint8_t ckvdUnk1;
	uint8_t ckvdUnk2;
	float ckvdUnk3;
	uint8_t ckvdUnk4;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CLocManager : CKSubclass<CKLogic, 59> {
	struct Ding { uint32_t lmdUnk1, lmdUnk2, lmdUnk3; };

	// Global only:
	uint32_t lmUnk0, numTrcStrings, lmNumDings, numStdStrings;
	std::vector<Ding> lmDings;
	std::vector<uint32_t> lmArStdStrInfo; // Arthur only, has numStdStrings elements

	// XXL1 PS2 + Romaster have numLanguages + language IDs in GAME.K*
	// GC+PC have them in *GLOC.K* (see Loc_CLocManager)
	uint16_t numLanguages;
	std::vector<uint32_t> langStrIndices, langIDs;

	// OG+:
	std::vector<kobjref<CKObject>> stdStringRefs;
	std::vector<std::pair<uint32_t, kobjref<CKObject>>> trcStringRefs;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override {}
	void serialize(KEnvironment* kenv, File* file) override {}
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
	void serializeGlobal(KEnvironment* kenv, File* file) override;
};

struct CKSekens : CKMRSubclass<CKSekens, CKReflectableLogic, 61> {
	struct SLine {
		uint32_t mUnk0 = -1;
		float mUnk1 = 0.0f;
		float mUnk2 = -1.0f;
		int32_t x2hdValue = -1;
		uint8_t mArByte1 = 0;
		uint8_t mArByte2 = 0;
		void reflectMembers(MemberListener &r);
	};
	kobjref<CKObject> sekManager2d;
	kobjref<CKObject> sekSndManager;
	// uint32_t sizeFor_sekLines;
	std::vector<SLine> sekLines;
	kobjref<CKObject> sekRomaSndDictID;
	std::vector<std::string> sekRomaLineNames;
	uint32_t sekUnk4 = 0;
	uint8_t sekSkippable = 1;

	// OG
	std::vector<kobjref<CKSekensEntry>> ogLines;
	int32_t ogUnk0;
	uint8_t ogUnk1;
	uint8_t ogUnk2;
	uint8_t ogUnk3;
	uint8_t ogUnk4;
	int32_t ogUnk5;
	int32_t ogUnk6;
	EventNode ogUnk7;
	EventNode ogUnk8;
	int32_t ogUnk9;
	std::array<int32_t, 4> ogUnk10; // could be a UUID?

	void init(KEnvironment* kenv);
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

struct CColorizedScreenData : CKMRSubclass<CColorizedScreenData, CKReflectableLogic, 68> {
	uint32_t ccsdColor0;
	uint32_t ccsdColor1;
	float ccsdUnk2;
	float ccsdUnk3;
	uint8_t ccsdUnk4;
	uint8_t ccsdUnk5;
	uint8_t ccsdUnk6;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct SBeacon {
	int16_t posx, posy, posz; uint16_t params;
	Vector3 getPosition() const { return Vector3(posx, posy, posz) * 0.1f; }
	void setPosition(const Vector3& ts) { posx = (int16_t)(ts.x * 10); posy = (int16_t)(ts.y * 10); posz = (int16_t)(ts.z * 10); }
};

struct CKBeaconKluster : CKSubclass<CKLogic, 73> {
	struct Bing {
		bool active = false;
		//uint32_t numBeacons;
		uint8_t unk2a, numBits, handlerId;
		uint16_t sectorIndex; uint32_t klusterIndex; uint16_t handlerIndex;
		uint16_t bitIndex;
		kobjref<CKObject> handler;
		uint32_t unk6;	// class ID? (12,74), (12,78)
		std::vector<SBeacon> beacons;
	};
	kobjref<CKBeaconKluster> nextKluster;
	BoundingSphere bounds;
	uint16_t numUsedBings = 0;
	std::vector<Bing> bings;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	bool empty() const;
};

struct CKProjectileTypeBase : CKMRSubclass<CKProjectileTypeBase, CKReflectableLogic, 76> {
	uint8_t ckptbpfxUnk0;
	int32_t ckptbpfxUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKProjectileTypeScrap : CKMRSubclass<CKProjectileTypeScrap, CKProjectileTypeBase, 77> {
	std::array<float, 6> ckptsUnk2;
	std::vector<kobjref<CClone>> ckptsUnk3;

	std::array<float, 4> x2FloatExt;
	kobjref<CParticlesNodeFx> x2Particle1, x2Particle2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKProjectileTypeAsterixBomb : CKMRSubclass<CKProjectileTypeAsterixBomb, CKProjectileTypeBase, 79> {
	std::array<float, 8> ckptabUnk2;
	std::array<kobjref<CParticlesNodeFx>, 5> ckptabUnk3;
	float ckptabUnk4;
	float ckptabUnk5;
	float ckptabUnk6;
	float ckptabUnk7;
	float ckptabUnk8;
	float ckptabUnk9;
	float ckptabUnk10;
	kobjref<CKSoundDictionaryID> ckptabUnk11;
	kobjref<CKShadowCpnt> ckptabUnk12;
	std::vector<kobjref<CClone>> ckptabUnk13;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKExplosionFxData;
struct CKShockWaveFxData;
struct CKFireBallFxData;

struct CKProjectileTypeBallisticPFX : CKMRSubclass<CKProjectileTypeBallisticPFX, CKProjectileTypeBase, 80> {
	std::array<float, 7> ckptbpfxUnk2;
	kobjref<CParticlesNodeFx> ckptbpfxUnk3;
	kobjref<CParticlesNodeFx> ckptbpfxUnk4;
	kobjref<CTrailNodeFx> ckptbpfxUnk5;
	uint16_t ckptbpfxUnk6;
	uint8_t ckptbpfxUnk7;
	std::vector<kobjref<CClone>> ckptbpfxUnk8;

	kobjref<CKExplosionFxData> x2ExplosionFxData;
	kobjref<CKShockWaveFxData> x2ShockWaveFxData;
	kobjref<CKFireBallFxData> x2FireBallFxData;
	int32_t x2LastInt = -1;

	std::array<float, 2> ogFloatExt;
	uint8_t ogByte1, ogByte2;
	kobjref<CKSoundDictionaryID> soundDictId;
	float soundValue;
	uint8_t ogByte3;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashNode2dFx : CKMRSubclass<CKFlashNode2dFx, CKReflectableLogic, 87> {
	kobjref<CBillboard2d> billboard;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKElectricArcNodeFx : CKMRSubclass<CKElectricArcNodeFx, CKReflectableLogic, 89> {
	kobjref<CNode> node;
	std::array<uint8_t, 3> x2Bytes;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKQuadNodeFx : CKMRSubclass<CKQuadNodeFx, CKReflectableLogic, 94> {
	kobjref<CNode> node;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKDefaultPlayer : CKSubclass<CKLogic, 97> {
	// game
	kobjref<CKObject> playerStage;
	void deserialize(KEnvironment* kenv, File* file, size_t length) {}
	void serialize(KEnvironment* kenv, File* file) {}
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length);
	void serializeGlobal(KEnvironment* kenv, File* file);
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

	kobjref<CKObject> x2Node3;
	std::array<int32_t, 2> x2IntArr;
	uint8_t x2Byte;

	std::vector<std::pair<int32_t, kobjref<CKObject>>> ogParticleAccessors;
	uint8_t ogByte;
	kobjref<CKObject> ogCameraQuakeLauncher;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKTimeCounter : CKMRSubclass<CKTimeCounter, CKReflectableLogic, 135> {
	float maxTime = 1.0f, maxTime2 = 1.0f;
	int32_t maxLoops = 1;
	EventNode event1, event2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKIntegerCounter : CKMRSubclass<CKIntegerCounter, CKReflectableLogic, 136> {
	int32_t startValue = 0, minValue = 0;
	int32_t maxValue = 0, maxLoops = 1;
	EventNode event1, event2, event3;
	int32_t alNewThing = 0; // Alice+: Event node or just int32??? Not even sure if it's at the correct place.
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKConditionNode : CKMRSubclass<CKConditionNode, CKReflectableLogic, 138> {
	kobjref<CKConditionNode> nextCondNode; // XXL2 only, removed in Arthur+ in favor of a vector in CKCombiner
	uint32_t condNodeType = 0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCombiner : CKMRSubclass<CKCombiner, CKConditionNode, 139> {
	kobjref<CKConditionNode> childCondNode; // XXL2
	std::vector<kobjref<CKConditionNode>> condNodeChildren; // Arthur+
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
	void onLevelSave(KEnvironment* kenv) override;
};

struct CKComparedData;

struct CKComparator : CKMRSubclass<CKComparator, CKConditionNode, 140> {
	kobjref<CKComparedData> leftCmpData, rightCmpData;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKTrigger;

struct CKComparedData : CKMRSubclass<CKComparedData, CKReflectableLogic, 141> {
	uint32_t cmpdatType = 0;
	struct CmpDataObjectProperty {
		kobjref<CKObject> cmpdatT0Ref;
		uint32_t cmpdatT0Unk1, cmpdatT0Unk2, cmpdatT0Unk3;
	};
	struct CmpDataConstant {
		std::variant<uint8_t, uint32_t, float, KPostponedRef<CKObject>> value;
	};
	struct CmpDataEventNode {
		uint8_t cmpdatT2Unk1 = 0;
		kobjref<CKTrigger> cmpdatT2Trigger;
	};
	std::variant<CmpDataObjectProperty, CmpDataConstant, CmpDataEventNode> cmpdatValue;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);

	int getDataType() const { return (cmpdatType >> 2) & 3; }
};

struct CKTrigger : CKSubclass<CKLogic, 142> {
	struct Action {
		KPostponedRef<CKObject> target;
		uint16_t event = 0;
		std::variant<uint8_t, uint32_t, float, KPostponedRef<CKObject>> value;
	};
	kobjref<CKConditionNode> condition;
	std::vector<Action> actions;

	// OG+:
	std::vector<kobjref<CKObject>> ogDatas;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment* kenv) override;
	void onLevelSave(KEnvironment* kenv) override;
};

struct CKProjectileTypeTargetLock : CKMRSubclass<CKProjectileTypeTargetLock, CKProjectileTypeBallisticPFX, 146> {
	float ckpttlUnk30;
	float ckpttlUnk31;
	float ckpttlUnk32;
	uint8_t ckpttOgByte = 1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CMultiGeometry;
struct CKDetectedMovable;

struct CKDetectorBase : CKMRSubclass<CKDetectorBase, CKReflectableLogic, 156> {
	uint32_t dbFlags;
	kobjref<CMultiGeometry> dbGeometry;
	kobjref<CKDetectedMovable> dbMovable;
	uint32_t dbSectorIndex;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSectorDetector : CKMRSubclass<CKSectorDetector, CKReflectableLogic, 157> {
	std::vector<kobjref<CKDetectorBase>> sdDetectors;
	std::vector<kobjref<CMultiGeometry>> sdGeometries;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CMultiGeometryBasic : CKMRSubclass<CMultiGeometry, CKReflectableLogic, 197> {
	std::variant<AABoundingBox, BoundingSphere, AARectangle> mgShape;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CMultiGeometry : CKSubclass<CMultiGeometryBasic, 158> {};

struct CKDetectorEvent : CKMRSubclass<CKDetectorEvent, CKDetectorBase, 160> {
	EventNode deOnExit, deOnPresence, deOnEnter;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKDetectedMovable : CKMRSubclass<CKDetectedMovable, CKReflectableLogic, 161> {
	struct Movable {
		KPostponedRef<CKSceneNode> dtmovSceneNode;
		uint32_t dtmovSectorIndex = 0;
		uint8_t dtmovUnkFlag = 0;
		void reflectMembers(MemberListener& r);
	};
	std::vector<Movable> dtmovMovables;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
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

struct CKSound : CKMRSubclass<CKSound, CKReflectableLogic, 171> {
	int32_t sndIndex = -1;
	KPostponedRef<CKObject> sndWaveObj;
	float sndVolume = 1.0f;
	float sndSpeed = 1.0f;
	float sndVal3 = 0.0f;
	int32_t sndFlags = 0;
	std::variant<Vector3, KPostponedRef<CKSceneNode>> sndPosition;
	float sndVal4 = 15.0f;
	float sndVal5 = 40.0f;
	float sndVal6 = 0.0f;
	Vector3 sndBoxHigh = { 15.0f,15.0f,15.0f }, sndBoxLow = { -15.0f,-15.0f,-15.0f };
	int32_t sndSector = 0;

	float ogVal1 = 0.0f, ogVal2 = 0.0f, ogVal3 = 0.0f;
	int32_t ogVal4 = 10, ogVal5 = 1, ogLastVal = 1;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CSGHotSpot : CKMRSubclass<CSGHotSpot, CKReflectableLogic, 173> {
	KPostponedRef<CKSceneNode> csghsUnk0;
	uint8_t csghsUnk1 = 255;
	std::array<float, 7> csghsUnk2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKAnyGeometry;
struct CMaterial : CKMRSubclass<CMaterial, CKReflectableLogic, 175> {
	// TODO: Check Arthur, OG X360, Spyro
	struct Extension {
		int extensionType = 0;
		uint32_t extUnkCom_1 = 0, extUnkCom_2 = 0;
		uint8_t extUnk0_1, extUnk0_2; uint32_t extUnk0_3, extUnk0_4;
		uint32_t extUnk2_1 = 0, extUnk2_2 = 0, extUnk2_3 = 0;
		uint32_t extUnk4_1 = 0, extUnk4_2 = 0; uint8_t extUnk4_3 = 0;
		void reflectMembers(MemberListener& r);
	};
	kobjref<CKAnyGeometry> geometry;
	uint32_t flags = 0;
	std::vector<Extension> extensions;
	// Type data
	kobjref<CKObject> t78_kpbufferRef; // Arthur+
	uint32_t t78_unk1; // Arthur+
	uint32_t t78_unk2;
	uint32_t t8_unk3;
	std::string t8_texName;
	// Arthur+:
	uint8_t ogUnkA1; std::array<uint8_t, 7> ogUnkA2;
	uint32_t arPfx1, arPfx2, arPfx3, arPfx4;
	kobjref<CMaterial> arPfxAnotherMaterial;
	kobjref<CKAnyGeometry> arPfxGeometry;
	// Olympic+:
	float ogUnkFlt = 50.0f;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashMessageIn;
struct CKFlashPlaySoundEvent : CKMRSubclass<CKFlashPlaySoundEvent, CKReflectableLogic, 185> {
	kobjref<CKFlashMessageIn> ckfpseUnk0;
	int32_t ckfpseUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKStreamObject;
struct CKMusicPlayList : CKMRSubclass<CKMusicPlayList, CKReflectableLogic, 187> {
	struct X2Stream {
		int32_t streamIndex = 0;
		float param1 = 0.0f, param2 = 0.0f, param3 = 1.0f, param4 = 1.0f;
		int32_t x2Unk1 = 1;
	};
	std::vector<X2Stream> x2Streams;
	std::vector<std::pair<kobjref<CKStreamObject>, uint32_t>> ogStreams;
	float mplUnk1 = 2.5f, mplUnk2 = 1.0f;
	float mplSpUnk1, mplSpUnk2, mplSpUnk3; uint32_t mplSpUnk4;
	uint8_t mplUnk3 = 0, mplUnk4 = 0, mplUnk5 = 0, mplUnk6 = 1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSandal : CKMRSubclass<CKSandal, CKProjectileTypeBase, 188> {
	float cksUnk0;
	float cksUnk1;
	kobjref<CKObject> cksUnk2;
	std::vector<kobjref<CAnimatedNode>> cksUnk3;
	kobjref<CAnimationDictionary> cksUnk8;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKDetectorMusic : CKMRSubclass<CKDetectorMusic, CKDetectorBase, 189> {
	uint32_t dtmusUnk1 = 0, dtmusUnk2 = 0;
	uint8_t dtmusUnk3 = 0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKHkMoveCpnt : CKMRSubclass<CKHkMoveCpnt, CKReflectableLogic, 192> {
	float ckaecUnk1;
	float ckaecUnk2;
	float ckaecUnk3;
	float ckaecUnk4;
	float ckaecUnk5;
	float ckaecUnk6;
	float ckaecUnk7;
	float ckaecUnk8;
	float ckaecUnk9;
	float ckaecUnk10;
	float ckaecUnk11;
	float ckaecUnk12;
	float ckaecUnk13;
	float ckaecUnk14;
	float ckaecUnk15;
	float ckaecUnk16;
	float ckaecUnk17;
	float ckaecUnk18;
	float ckaecUnk19;
	float ckaecUnk20;
	float ckaecUnk21;
	float ckaecUnk22;
	float ckaecUnk23;
	float ckaecUnk24;
	float ckaecUnk25;
	float ckaecUnk26;
	float ckaecUnk27;

	// OG
	float ogUnk1;
	float ogUnk2;
	
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGameDefinition : CKSubclass<CKLogic, 204> {
	std::string gsName;
	uint32_t gsUnk1;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void deserializeLvlSpecific(KEnvironment* kenv, File* file, size_t length) override {}
	void serializeLvlSpecific(KEnvironment* kenv, File* file) override {}
};

struct CKReflectableGameDef : CKSubclass<CKGameDefinition, 10000> {
	virtual void reflectGame(MemberListener& ml, KEnvironment* kenv) {}
	virtual void reflectLevel(MemberListener& ml, KEnvironment* kenv) {}
	void deserialize(KEnvironment* kenv, File* file, size_t length) override {
		CKGameDefinition::deserialize(kenv, file, length);
		ReadingMemberListener ml{ file, kenv };
		reflectGame(ml, kenv);
	}
	void serialize(KEnvironment* kenv, File* file) override {
		CKGameDefinition::serialize(kenv, file);
		WritingMemberListener ml{ file,kenv };
		reflectGame(ml, kenv);
	}
	void deserializeLvlSpecific(KEnvironment* kenv, File* file, size_t length) override {
		ReadingMemberListener ml{ file, kenv };
		reflectLevel(ml, kenv);
	}
	void serializeLvlSpecific(KEnvironment* kenv, File* file) override {
		WritingMemberListener ml{ file,kenv };
		reflectLevel(ml, kenv);
	}
};

struct CKGameSpawnPoint : CKSubclass<CKReflectableGameDef, 198> {
	// game
	kobjref<CKObject> gspStage;
	int gspSector;
	// level
	uint32_t gspMainBeacon;
	kobjref<CKObject> gspHDR;
	EventNode gspEvent;

	void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
	void reflectLevel(MemberListener& ml, KEnvironment* kenv) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKGameStage : CKSubclass<CKReflectableGameDef, 202> {
	// game
	uint32_t gsgGameStructure; // ref as int OK since singleton
	int gsgLevelNumber;
	std::array<uint32_t, 4> gsgUuid;
	std::vector<kobjref<CKObject>> gsgModules;
	std::vector<kobjref<CKObject>> gsgSpawnPoints;
	// level
	std::vector<kobjref<CKObject>> gsgUnkObjList1;
	std::vector<kobjref<CKObject>> gsgUnkObjList2;
	kobjref<CKCinematicScene> gsgLaunchScene;

	void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
	void reflectLevel(MemberListener& ml, KEnvironment* kenv) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKGameState : CKSubclass<CKGameDefinition, 203> {
	template <typename DataType> struct StateValue {
		kobjref<CKObject> object;
		DataType data;
	};

	uint32_t gsStructureRef; // could be a kobjref, but big problem when loading from GAME.KWN!
	kobjref<CKObject> gsSpawnPoint;

	std::vector<StateValue<uint8_t>> gsStages;
	std::vector<StateValue<uint16_t>> gsModules;

	std::vector<std::vector<StateValue<DynArray<uint8_t>>>> lvlValuesArray;

	void readSVV8(KEnvironment *kenv, File *file, std::vector<StateValue<uint8_t>> &list, bool hasByte);
	void writeSVV8(KEnvironment *kenv, File *file, std::vector<StateValue<uint8_t>> &list, bool hasByte);
	void readSVV16(KEnvironment* kenv, File* file, std::vector<StateValue<uint16_t>>& list, size_t numBytes);
	void writeSVV16(KEnvironment* kenv, File* file, std::vector<StateValue<uint16_t>>& list, size_t numBytes);

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void deserializeLvlSpecific(KEnvironment* kenv, File *file, size_t length) override;
	void serializeLvlSpecific(KEnvironment* kenv, File *file) override;
	void resetLvlSpecific(KEnvironment *kenv) override;
};

struct CKGameModule : CKSubclass<CKReflectableGameDef, 205> {
	// game
	kobjref<CKObject> gmStage;
	// level
	struct GMImpactedObject {
		kobjref<CKObject> gmObjRef;
		int32_t gmObjUnk1;
		int32_t gmObjUnk2;
		int32_t gmObjUnk3;
	};
	std::vector<GMImpactedObject> gmImpactedObjects;

	void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
	void reflectLevel(MemberListener& ml, KEnvironment* kenv) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKArGameState : CKSubclass<CKGameState, 32> {
	std::vector<StateValue<uint8_t>> gsVideos, gsGameSekens;
	uint32_t gsStdText = -1;
	std::vector<StateValue<uint8_t>> gsBirdZones;
	std::vector<StateValue<uint8_t>> gsRunes;
	std::vector<StateValue<uint8_t>> gsEggbags;
	uint32_t gsRemainderGlobal = 0;
	uint32_t gsRemainderSpecific = 0;

	CKArGameState() { lvlValuesArray.resize(7); }
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void deserializeLvlSpecific(KEnvironment* kenv, File* file, size_t length) override;
	void serializeLvlSpecific(KEnvironment* kenv, File* file) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKS08GameState : CKSubclass<CKGameState, 485> {
	// modification in CKGameState: state of Module has 2 bytes instead of 1
	std::vector<StateValue<uint8_t>> gsVideos, gsGameSekens;
	kobjref<CKObject> gsStdText;
	std::array<uint8_t, 0x48> gsMaybeOtherTextData;
	std::vector<StateValue<uint8_t>> gsUpgrades; // no bytes
	std::array<uint8_t, 0x6A> gsSpRest;

	CKS08GameState() { lvlValuesArray.resize(5); } // stages, modules, ?videos?, ?sekens?, upgrades
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKAliceGameState : CKSubclass<CKGameState, 485> {
	// similar to Spyro, even has same ID lol, just ending has tons of data
	std::vector<StateValue<uint8_t>> gsVideos, gsGameSekens;
	kobjref<CKObject> gsStdText;
	std::vector<StateValue<uint8_t>> gsUpgrades; // no bytes
	std::array<uint8_t, 0x1209> gsAlRest;

	CKAliceGameState() { lvlValuesArray.resize(5); } // stages, modules, ?videos?, ?sekens?, upgrades
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKTydGameState : CKSubclass<CKGameState, 485> {
	// again, similar to Alice, but no upgrades, and less remaining data at the end
	// problem: remaining data size is variable...
	std::vector<StateValue<uint8_t>> gsVideos, gsGameSekens;
	kobjref<CKObject> gsStdText;

	struct Dragon {
		uint32_t head1;
		uint32_t head2;
		std::array<uint8_t, 0x19> headRem;
		std::vector<uint8_t> unkB1;
		std::array<uint8_t, 0xC4> unhandledData;
		std::vector<uint32_t> unkE1;
		uint8_t unkE2 = 0;
		uint8_t unkE3 = 0xFF;
		std::vector<float> unkE4;
		float unkE5;
		uint32_t unkE6 = 0;
		std::vector<uint32_t> unkE7;
		uint32_t unkE8 = 0;
		uint8_t unkE9 = 0;
	};
	std::vector<Dragon> tydDragons;
	std::array<uint8_t, 0x7B1> gsTydRest;

	CKTydGameState() { lvlValuesArray.resize(4); } // stages, modules, ?videos?, ?sekens?
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void resetLvlSpecific(KEnvironment* kenv) override;
};

struct CKMarkerBeacon : CKMRSubclass<CKMarkerBeacon, CKReflectableLogic, 214> {
	std::array<float, 3> ckmbUnk0;
	std::array<float, 4> ckmbUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraQuakeDatas : CKMRSubclass<CKCameraQuakeDatas, CKReflectableLogic, 217> {
	std::vector<std::array<float, 2>> ckcqdUnk1;
	std::vector<std::array<float, 2>> ckcqdUnk2;
	float ckcqdUnk3 = 1.0f;

	struct OgQuake {
		kobjref<CKObject> sinCurve;
		int32_t unk1;
		float unk2, unk3, unk4, unk5;
	};
	std::vector<OgQuake> ogQuakes;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
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

struct CKCameraFogDatas : CKMRSubclass<CKCameraFogDatas, CKReflectableLogic, 233> {
	uint32_t color1 = 0xFF719377;
	float unk2 = 60.0f;
	float unk3 = 0.0106f;
	uint32_t color2 = 0xFFFFFCDC;
	float unk4 = 99.999f;
	float unk5 = 100.0f;
	float unk6 = 0.0006f;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKGameStructure : CKMRSubclass<CKGameStructure, CKReflectableLogic, 237> {
	// GAME
	uint32_t gameMgrRef;
	kobjref<CKGameState> someGameState;
	std::vector<kobjref<CKGameStage>> gameStages;
	std::vector<kobjref<CKGameState>> gameStates;

	// LEVEL
	std::vector<kobjref<CKObject>> lvlUnkObjects;
	std::vector<kobjref<CKGameState>> lvlStates;

	void reflectGame(MemberListener& r, KEnvironment* kenv);
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
	void serializeGlobal(KEnvironment* kenv, File* file) override;
};

struct CKExtendedMarkerBeacon : CKMRSubclass<CKExtendedMarkerBeacon, CKMarkerBeacon, 244> {
	int32_t ckembUnk0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKNumber : CKMRSubclass<CKNumber, CKProjectileTypeBase, 248> {
	std::vector<std::pair<kobjref<CNode>, uint8_t>> numberNodes;
	std::array<float, 10> numberFltValues;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
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

struct CKStreamWave;
struct CKStreamObject : CKMRSubclass<CKStreamObject, CKReflectableLogic, 361> {
	kobjref<CKStreamWave> streamPointer;
	float param1 = 0.0f, param2 = 0.0f, param3 = 1.0f, param4 = 1.0f;
	uint8_t ogUnk1 = 0; float ogUnk2 = 0.8f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKStreamWave : CKMRSubclass<CKStreamWave, CKReflectableLogic, 362> {
	std::string wavePath;
	float waveDurationSec = 0.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void deserializeLvlSpecific(KEnvironment* kenv, File* file, size_t length) {}
	void serializeLvlSpecific(KEnvironment* kenv, File* file) {}
};

struct CKDisplayBox : CKSubclass<CMultiGeometryBasic, 385> {};

struct CKSekensEntry : CKMRSubclass<CKSekensEntry, CKReflectableLogic, 406> {
	float skbkUnk1 = 0;
	kobjref<CKSekens> skbkOwningSekens;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSekensBlock : CKMRSubclass<CKSekensBlock, CKSekensEntry, 407> {
	kobjref<CKObject> skbkTextRef;
	float skbkUnk2 = -1.0f;
	uint8_t skbkUnk3 = 0;
	uint8_t skbkUnk4 = 0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSekensMarker : CKSubclass<CKSekensEntry, 408> {};

struct IKFxData : CKMRSubclass<IKFxData, CKReflectableLogic, 193> {
	int32_t ckefdUnk0;
	uint8_t ckefdUnk1; // one of them is removed in OG, which one?
	uint8_t ckefdUnk2; //
	uint32_t ogHdUnk = 0xFF;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct IKNodeFx : CKMRSubclass<IKNodeFx, CKReflectableLogic, 82> {
	kobjref<CKObject> nfxObjectRef;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CBlurData;
struct CHDRData;

struct CKBlurFxData : CKMRSubclass<CKBlurFxData, IKFxData, 10> {
	kobjref<CBlurData> ckbfdUnk0;
	uint8_t ckbfdUnk1;
	uint8_t ckbfdUnk2 = 0;
	float ckbfdUnk3;
	std::vector<std::array<float, 2>> ckbfdUnk4;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKBlurNodeFx : CKSubclass<IKNodeFx, 12> {};

struct CKHDRFxData : CKMRSubclass<CKHDRFxData, IKFxData, 60> {
	kobjref<CHDRData> ckhdrfdUnk0;
	float ckhdrfdUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKHDRNodeFx : CKMRSubclass<CKHDRNodeFx, IKNodeFx, 62> {
	kobjref<CHDRData> ckhdrnfUnk0;
	kobjref<CHDRData> ckhdrnfUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKScreenColorFxData : CKMRSubclass<CKScreenColorFxData, IKFxData, 65> {
	kobjref<CKObject> screenData;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKScreenColorNodeFx : CKSubclass<IKNodeFx, 67> {};

struct CKFlashFxData : CKMRSubclass<CKFlashFxData, IKFxData, 92> {
	float ckffdUnk0;
	float ckffdUnk1;
	std::array<float, 2> ckffdUnk2;
	std::string ckffdString;
	uint32_t ckffdColor;

	float ogckffdUnk0;
	float ogckffdUnk1;
	float ogckffdUnk2;
	float ogckffdUnk3;
	float ogckffdUnk4;
	std::array<float, 2> ogckffdUnk5;
	std::array<float, 2> ogckffdUnk6;
	std::array<float, 2> ogckffdUnk7;
	std::array<float, 2> ogckffdUnk8;
	std::array<float, 2> ogckffdUnk9;
	//std::string ogckffdMsgString;
	//int32_t ogckffdColor;
	uint8_t ogckffdUnk13;
	float ogckffdUnk14;
	float ogckffdUnk15;
	float ogckffdUnk16;
	float ogckffdUnk17;
	uint8_t ogckffdUnk18;
	uint8_t ogckffdUnk19;
	uint8_t ogckffdUnk20;
	std::array<float, 2> ogckffdUnk21;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKElectricArcFxData : CKMRSubclass<CKElectricArcFxData, IKFxData, 93> {
	float ckhadfUnk0;
	float ckhadfUnk1;
	float ckhadfUnk2;
	float ckhadfUnk3;
	float ckhadfUnk4;
	float ckhadfUnk5;
	float ckhadfUnk6;
	float ckhadfUnk7;
	uint8_t ckhadfUnk8;
	float ckhadfUnk9;
	float ckhadfUnk10;
	uint8_t ckhadfUnk11;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKShockWaveFxData;

struct CKExplosionFxData : CKMRSubclass<CKExplosionFxData, IKFxData, 106> {
	float ckefdUnk3;
	float ckefdUnk4;
	float ckefdUnk5;
	float ckefdUnk6;
	int32_t ckefdUnk7;
	float ckefdUnk8;
	float ckefdUnk9;
	kobjref<CKSoundDictionaryID> ckefdUnk10;
	kobjref<CKObject> ckefdUnk11;

	// OG:
	uint8_t ogUnk1;
	struct EFDType0 {
		std::array<float, 6> fltPack1;
		int32_t efdUnk2;
		float efdUnk3;
		std::array<Vector3, 4> efdUnk4;
		std::array<float, 4> efdUnk5;
		float efdLastFloat;
	};
	struct EFDType1 {
		int32_t efdUnk1;
		float efdUnk2;
		float efdUnk3;
		Vector3 efdUnk4;
		Vector3 efdUnk5;
		float efdLastFloat;
	};
	struct EFDType2 {
		kobjref<CKShockWaveFxData> shockWaveFxData;
		float efdLastFloat;
	};
	struct EFDType3 {
		kobjref<CKObject> sparkFxData;
		float efdLastFloat;
	};
	struct EFDType4 {
		kobjref<CKObject> flashFxData;
		float efdLastFloat;
	};
	using EFDVariant = std::variant<EFDType0, EFDType1, EFDType2, EFDType3, EFDType4>;
	std::vector<EFDVariant> efdParts;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKHedgeHopTrailNodeFx : CKMRSubclass<CKHedgeHopTrailNodeFx, IKNodeFx, 177> {
	int32_t ckhhtnfUnk0;
	int32_t ckhhtnfUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKHedgeHopTrailFxData : CKMRSubclass<CKHedgeHopTrailFxData, IKFxData, 181> {
	int32_t ckaecUnk98;
	float ckaecUnk99;
	float ckaecUnk100;
	float ckaecUnk101;
	float ckaecUnk102;
	float ckaecUnk103;
	float ckaecUnk104;
	float ckaecUnk105;
	float ckaecUnk106;
	float ckaecUnk107;
	float ckaecUnk108;
	float ckaecUnk109;
	float ckaecUnk110;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKShockWaveNodeFx : CKSubclass<IKNodeFx, 195> {};
struct CKShockWaveFxData : CKMRSubclass<CKShockWaveFxData, IKFxData, 196> {
	float ckswfdUnk0;
	float ckswfdUnk1;
	//int32_t ckswfdUnk2;
	std::vector<std::array<float, 2>> ckswfdUnk3;
	//int32_t ckswfdUnk4;
	std::vector<std::array<float, 2>> ckswfdUnk5;
	//int32_t ckswfdUnk6;
	std::vector<std::array<float, 2>> ckswfdUnk7;
	uint8_t ckswfdUnk8;
	float ckswfdUnk9;
	float ckswfdUnk10;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFireBallFxData : CKMRSubclass<CKFireBallFxData, IKFxData, 206> {
	float ckfbfdUnk0;
	float ckfbfdUnk1;
	float ckfbfdUnk2;
	float ckfbfdUnk3;
	float ckfbfdUnk4;
	float ckfbfdUnk5;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKFireBallNodeFx : CKMRSubclass<CKFireBallNodeFx, IKNodeFx, 207> {
	kobjref<CKObject> ckfbnfUnk0;
	int32_t ckfbnfUnk1;
	uint8_t ckfbnfUnk2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKDistortionNodeFx : CKSubclass<IKNodeFx, 227> {};
struct CKDistortionFxData : CKMRSubclass<CKDistortionFxData, IKFxData, 229> {
	kobjref<CKObject> screenData;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKWaterWaveFxData : CKMRSubclass<CKWaterWaveFxData, IKFxData, 238> {
	float wwUnk1, wwUnk2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKWaterWaveNodeFx : CKMRSubclass<CKWaterWaveNodeFx, IKNodeFx, 239> {
	kobjref<CKObject> ckwwnfUnk0;
	int32_t ckwwnfUnk1;
	uint8_t ckwwnfUnk2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKWaterSplashFxData : CKMRSubclass<CKWaterSplashFxData, IKFxData, 240> {
	float wsUnk1;
	std::vector<std::array<float, 2>> wsUnk2;
	std::vector<std::array<float, 2>> wsUnk3;
	std::vector<std::array<float, 2>> wsUnk4;
	std::vector<std::array<float, 2>> wsUnk5;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKWaterSplashNodeFx : CKMRSubclass<CKWaterSplashNodeFx, IKNodeFx, 241> {
	kobjref<CKObject> ckwsnfUnk0;
	kobjref<CKObject> ckwsnfUnk1;
	kobjref<CNode> ckwsnfUnk2;
	int32_t ckwsnfUnk3;
	int32_t ckwsnfUnk4;
	uint8_t ckwsnfUnk5;
	// OG
	kobjref<CKObject> ogParticleAccessor1;
	kobjref<CKObject> ogParticleAccessor2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKPowerBallFxData : CKMRSubclass<CKPowerBallFxData, IKFxData, 242> {
	float ckhabUnk271;
	float ckhabUnk272;
	float ckhabUnk273;
	float ckhabUnk274;
	float ckhabUnk275;
	float ckhabUnk276;
	float ckhabUnk277;
	float ckhabUnk278;
	float ckhabUnk279;
	float ckhabUnk280;
	//int32_t ckhabUnk281;
	std::vector<std::array<float, 2>> ckhabUnk282;
	float ckhabUnk283;
	float ckhabUnk284;
	float ckhabUnk285;
	//int32_t ckhabUnk286;
	std::vector<std::array<float, 2>> ckhabUnk287;
	//int32_t ckhabUnk288;
	std::vector<std::array<float, 2>> ckhabUnk289;
	uint8_t ckhabUnk290;
	float ckhabUnk291;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKPowerBallNodeFx : CKMRSubclass<CKPowerBallNodeFx, IKNodeFx, 243> {
	kobjref<CParticlesNodeFx> ckpbnfUnk0;
	kobjref<CNode> ckpbnfUnk1;
	kobjref<CParticlesNodeFx> ckpbnfUnk2;
	kobjref<CNode> ckpbnfUnk3;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKBomb : CKMRSubclass<CKBomb, CKProjectileTypeBase, 184> {
	std::array<float, 8> ckbUnk0;
	std::array<kobjref<CParticlesNodeFx>, 5> ckbUnk1;

	CKExplosionFxData explosionFx;

	kobjref<CKShadowCpnt> ckbUnk18;
	std::vector<kobjref<CClone>> ckbUnk19;
	int32_t ckbUnk29;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
