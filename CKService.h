#pragma once

#include "KObject.h"
#include <vector>
#include <array>
#include "vecmat.h"
#include "Shape.h"
#include "CKUtils.h"
#include "Events.h"
#include "CKCamera.h"

struct CKBeaconKluster;
struct CKSceneNode;
struct CKPFGraphNode;
struct CKHook;
struct CKCinematicScene;
struct CKTriggerDomain;
struct CKBundle;
struct CKSekens;
struct CKParticleGeometry;
struct CNode;
struct CKTrigger;
struct CKProjectileTypeBase;
struct CKBoundingShape;
struct CKCamera;
struct CKCameraSector;
struct CKIntegerCounter;
struct CKTimeCounter;

struct CKService : CKCategory<1> {};

struct CKReflectableService : CKMRSubclass<CKReflectableService, CKMemberReflectable<CKService>, 0xBADB01> {
	void reflectMembers(MemberListener &r) {}
};

struct CKServiceLife : CKSubclass<CKService, 1> {
	kobjref<CKBundle> firstBundle;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	void addBundle(CKBundle* bundle);
	void removeBundle(CKBundle* bundle);
};

struct CKSrvCollision : CKSubclass<CKService, 2> {
	using CollIndex = uint16_t;

	//uint16_t numWhat;
	uint8_t huh;
	std::array<kobjref<CKSceneNode>, 50> dynBSphereProjectiles;
	std::vector<std::vector<kobjref<CKObject>>> objs;
	//uint16_t unk1, unk2;
	std::vector<kobjref<CKObject>> objs2; // * unk1
	struct Bing {
		uint16_t v1 = 0xC; // least nibble: 1=root+branch, E=child+leaf, C=root+leaf
		kobjref<CKObject> obj1, obj2;
		uint16_t b1 = 0xFFFF, b2 = 0xFFFF;
		uint8_t v2 = 255;
		std::array<CollIndex, 3> aa = { 0xFFFF, 0xFFFF, 0xFFFF };
	};
	std::vector<Bing> bings;
	CollIndex inactiveList = 0xFFFF, activeList = 0xFFFF;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	uint16_t addOrGetHandler(CKObject* handler);
	CollIndex addCollision(CKObject* handler1, CKBoundingShape* shape1, CKObject* handler2, CKBoundingShape* shape2);
	void setParent(CollIndex colIndex, CollIndex parentIndex);
};

struct CKSrvCamera : CKMRSubclass<CKSrvCamera, CKReflectableService, 3> {
	uint32_t scamUnk0;
	kobjref<CKCameraBase> scamCam;
	kobjref<CKCameraSector> scamCamstr;
	std::array<kobjref<CKCamera>, 20> scamCamfixtrack;
	CKCamera scamCameraInst;
	std::array<kobjref<CKSceneNode>, 20> scamAnimNode;
	std::array<float, 3> scamUnk15;
	std::array<float, 7> scamUnk16;
	std::array<float, 39> scamUnk17;
	std::array<float, 39> scamUnk18;
	std::array<float, 39> scamUnk19;
	std::array<float, 39> scamUnk20;
	std::array<float, 13> scamUnk21;
	std::array<float, 13> scamUnk22;
	kobjref<CKSceneNode> scamSphere1;
	kobjref<CKSceneNode> scamSphere2;
	float scamUnk25;
	float scamUnk26;
	float scamUnk27;
	float scamUnk28;
	float scamUnk29;
	float scamUnk30;
	float scamUnk31;
	float scamUnk32;
	float scamUnk33;
	float scamUnk34;
	float scamUnk35;
	float scamUnk36;
	float scamRoma1 = 25.0f;
	float scamRoma2 = 5.0f;

	// XXL2+:
	std::vector<std::vector<kobjref<CKCameraSector>>> x2scCameraSectors;
	std::array<float, 39> x2scUnk1;
	std::array<float, 39> x2scUnk2;
	std::array<float, 10> x2scUnk3;
	std::array<float, 10> x2scUnk4;
	std::array<float, 10> x2scUnk5;
	std::array<float, 10> x2scUnk6;
	std::array<float, 10> x2scUnk7;
	float x2scUnk8;
	std::array<std::array<float, 6>, 10> x2scUnkMass;
	float x2scUnkA = 5.0f, x2scUnkB = 25.0f, x2scUnkC = 3.141592f;
	std::vector<kobjref<CKObject>> x2scQuakeDatas;

	// OG+:
	std::array<float, 103> og103Floats;
	std::array<std::array<float, 4>, 10> ogUnkMass;
	struct QuakeData {
		kobjref<CKObject> quakeCpnt;
		std::vector<kobjref<CKObject>> quakeCpntUpdaters;
	};
	std::vector<QuakeData> ogQuakeDatas;
	float ogUnk1, ogUnk2, ogUnk3;

	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKSrvCinematic : CKSubclass<CKService, 4> {
	std::vector<kobjref<CKCinematicScene>> cineScenes;
	kobjref<CKObject> cineBillboard1, cineBillboard2, cineBillboard3;
	std::vector<kobjref<CKObject>> arObjectList; // Arthur+

	// Global (Arthur+):
	int32_t arGlobalValue;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
};

struct CKSrvEvent : CKSubclass<CKService, 5>
{
	struct EventSequence {
		uint8_t numActions = 0, bitMask = 1;
		std::vector<CKObject *> users; bool userFound = false;
		int sector = -2;
	};
	union {
		struct { uint16_t numA, numB, numC; };
		uint16_t numSeqs[3] = { 0,0,0 };
	};
	uint16_t numTotalActions = 0;
	std::vector<EventSequence> sequences;
	std::vector<KPostponedRef<CKObject>> objs;
	std::vector<uint16_t> objInfos;

	std::vector<int16_t> evtSeqIDs;
	int16_t nextSeqID = 0;

	std::vector<std::string> evtSeqNames;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded2(KEnvironment *kenv) override;

	int getAddendumVersion() override;
	void deserializeAddendum(KEnvironment* kenv, File* file, int version) override;
	void serializeAddendum(KEnvironment* kenv, File* file) override;
};

struct CKSrvPathFinding : CKSubclass<CKService, 6> {
	std::vector<kobjref<CKPFGraphNode>> nodes;
	std::array<kobjref<CKObject>, 4> arQuadTreeBranches;
	std::array<uint32_t, 4> arQTBInts;
	float x2flt = 2.0f;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSrvDetector : CKSubclass<CKService, 7> {
	struct Rectangle {
		Vector3 center;
		float length1 = 1.0f, length2 = 1.0f;
		uint8_t direction = 0;
	};

	struct Detector {
		uint16_t shapeIndex = 0, nodeIndex = 0, flags = 0; EventNodeX1 eventSeqIndex;
	};

	uint16_t numJ;

	std::vector<AABoundingBox> aaBoundingBoxes;
	std::vector<BoundingSphere> spheres;
	std::vector<Rectangle> rectangles;

	std::vector<Detector> aDetectors, bDetectors, cDetectors, dDetectors, eDetectors;

	std::vector<kobjref<CKSceneNode>> nodes;

	// Addendum
	std::vector<std::string> aabbNames, sphNames, rectNames;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	int getAddendumVersion() override;
	void deserializeAddendum(KEnvironment* kenv, File* file, int version) override;
	void serializeAddendum(KEnvironment* kenv, File* file) override;
};

struct CKSrvMarker : CKSubclass<CKService, 8> {
	struct Marker {
		Vector3 position;
		uint8_t orientation1 = 0, orientation2 = 0;
		uint16_t val3 = 1;
		std::string name; // Addendum
	};
	std::vector<std::vector<Marker>> lists;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	int getAddendumVersion() override;
	void deserializeAddendum(KEnvironment* kenv, File* file, int version) override;
	void serializeAddendum(KEnvironment* kenv, File* file) override;
};

struct CKSrvAvoidance : CKSubclass<CKService, 9> {
	float avoidValue = 1.5f;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSrvSekensor : CKSubclass<CKService, 10> {
	std::vector<kobjref<CKSekens>> sekens;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSrvBeacon : CKSubclass<CKService, 11> {
	uint8_t unk1;
	//uint32_t numHandlers;
	struct Handler {
		uint8_t unk2a, numBits, handlerIndex, handlerId, persistent, x2respawn;
		kobjref<CKObject> object;
	};
	std::vector<Handler> handlers;
	//uint32_t numSectors;
	struct BeaconSector {
		uint32_t numUsedBings = 0, numBings = 0, beaconArraySize = 0; // , numBits;
		std::vector<bool> bits;
		//uint32_t numBeaconKlusters;
		std::vector<uint32_t> _bkids;
		std::vector<kobjref<CKBeaconKluster>> beaconKlusters;
	};
	std::vector<BeaconSector> beaconSectors;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment *kenv) override;

	void removeBeacon(int sectorIndex, int klusterIndex, int bingIndex, int beaconIndex);
	void addHandler(CKObject* handler, uint8_t numBits, uint8_t handlerId, uint8_t persistent, uint8_t respawn);
	int addKluster(KEnvironment& kenv, int sectorIndex);
	void enableBing(int sectorIndex, int klusterIndex, int bingIndex);
	void addBeacon(int sectorIndex, int klusterIndex, int handlerId, const void * beacon);
};

struct CKSrvShadow : CKSubclass<CKService, 12> {
	uint32_t shadUnk1;
	uint32_t shadUnk2;
	kobjref<CNode> shadNode;
	kobjref<CKParticleGeometry> shadGeometry;
	std::string shadTexture;
	std::vector<std::array<float, 4>> shadValues;
	uint8_t shadUnk3 = 32, shadUnk4 = 32;
	float shadUnk5 = 0.3f;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKSrvProjectiles : CKSubclass<CKService, 13> {
	std::vector<kobjref<CKProjectileTypeBase>> projectiles;
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKSrvFx : CKSubclass<CKService, 14> {
	struct FxType {
		uint32_t clsFullId;
		uint8_t numInstances = 0;
		uint8_t startIndex = 0xFF;
	};
	std::vector<FxType> fxTypes;
	std::vector<kobjref<CKObject>> fxInstances;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKSrvCounter : CKSubclass<CKService, 17> {
	std::vector<kobjref<CKIntegerCounter>> integerCounters;
	std::vector<kobjref<CKTimeCounter>> timeCounters;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKSrvTrigger : CKSubclass<CKService, 18> {
	kobjref<CKTriggerDomain> rootDomain;
	uint32_t stUnk1, stUnk2;
	std::vector<std::pair<kobjref<CKTrigger>, std::string>> spTriggers; // Spyro

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};
