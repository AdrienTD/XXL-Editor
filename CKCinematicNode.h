#pragma once

#include "KEnvironment.h"
#include "CKUtils.h"
#include "vecmat.h"
#include "Events.h"
#include "CKPartlyUnknown.h"
#include <array>
#include <vector>

struct CKCinematicScene;
struct CKCinematicSceneData;
struct CKCameraBase;
struct CKHkSkyLife;
struct CBillboard2d;
struct CKFlaggedPath;
struct CKSceneNode;
struct CSGSectorRoot;
struct CKCinematicBloc;
struct CKGroupBlocCinematicBloc;

struct CKCinematicNode : CKMemberReflectable<CKCategory<8>> {
	uint16_t cnStartOutEdge = 0xFFFF;
	uint16_t cnFinishOutEdge = 0xFFFF;
	uint16_t cnNumOutEdges = 0;
	uint32_t cnFlags = 0x8242;
	kobjref<CKGroupBlocCinematicBloc> cnGroupBloc;
	kobjref<CKCinematicScene> cnScene;
	void reflectMembers(MemberListener &r);
};
struct CKCinematicBloc : CKMRSubclass<CKCinematicBloc, CKCinematicNode, 33> {
	kobjref<CKCinematicSceneData> cbSceneData;
	uint32_t cbUnk6 = 0xFFFFFFFF; // Sekens?
	uint32_t cbUnk7 = 0xFFFFFFFF; // Sekens?
	kobjref<CKObject> cbSekensMarker;
	float cbUnk8 = 0.1f;
	void reflectMembers2(MemberListener &r, KEnvironment* kenv);
};
struct CKCinematicDoor : CKMRSubclass<CKCinematicDoor, CKCinematicNode, 34> {
	uint32_t cdNumInEdges = 0;
	void reflectMembers(MemberListener &r);
};
struct CKLogicalAnd : CKMRSubclass<CKLogicalAnd, CKCinematicDoor, 1> {
	void reflectMembers(MemberListener &r);
};
struct CKLogicalOr : CKMRSubclass<CKLogicalOr, CKCinematicDoor, 2> {
	void reflectMembers(MemberListener &r);
};
struct CKRandLogicalDoor : CKMRSubclass<CKRandLogicalDoor, CKCinematicDoor, 11> {
	void reflectMembers(MemberListener &r);
};
struct CKStartDoor : CKMRSubclass<CKStartDoor, CKCinematicDoor, 17> {
	void reflectMembers(MemberListener &r);
};
struct CKGroupBlocCinematicBloc : CKMRSubclass<CKGroupBlocCinematicBloc, CKCinematicBloc, 7> {
	// uint32_t sizeFor_gbSubnodes;
	std::vector<kobjref<CKCinematicNode>> gbSubnodes;
	kobjref<CKCinematicNode> gbFirstNode;
	kobjref<CKCinematicNode> gbSecondNode;
	void reflectMembers2(MemberListener &r, KEnvironment* kenv);
};
struct CKStreamGroupBlocCinematicBloc : CKMRSubclass<CKStreamGroupBlocCinematicBloc, CKGroupBlocCinematicBloc, 14> {
	float sgbUnk0;
	float sgbUnk1;
	void reflectMembers2(MemberListener &r, KEnvironment* kenv);
};
struct CKStartEventCinematicBloc : CKMRSubclass<CKStartEventCinematicBloc, CKCinematicBloc, 25> {
	EventNode seEvtNode;
	void reflectMembers2(MemberListener &r, KEnvironment* kenv);
};

//

struct CKPlayAnimCinematicBloc : CKMRSubclass<CKPlayAnimCinematicBloc, CKCinematicBloc, 3> {
	float ckpacbUnk0 = 3.0f;
	int32_t ckpacbUnk1 = 0;
	float ckpacbUnk2 = -1.0f;
	uint8_t ckpacbUnk3 = 209;
	float ckpacbUnk4 = 0.0f;
	float ckpacbUnk5 = 0.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKPathFindingCinematicBloc : CKMRSubclass<CKPathFindingCinematicBloc, CKCinematicBloc, 4> {
	MarkerIndex ckpfcbUnk0;
	float ckpfcbUnk1 = 5.0f;
	int32_t ckpfcbUnk2 = -1;
	float ckpfcbUnk3 = 1.0f;
	float ckpfcbUnk4 = 0.0f;
	uint8_t ckpfcbUnk5 = 220;
	float ckpfcbUnk6 = 3.1416f;

	float x2_ckpfcbUnk0;
	float x2_ckpfcbUnk1;
	float x2_ckpfcbUnk2;
	float x2_ckpfcbUnk3;
	int32_t x2_ckpfcbUnk4;
	uint8_t x2_ckpfcbUnk5;
	MarkerIndex x2_ckpfcbUnk6;
	float ar_ckpfcbUnk7;
	int32_t ar_ckpfcbUnk8;
	float ar_ckpfcbUnk9;
	float ar_ckpfcbUnk10;
	float ar_ckpfcbUnk11;
	int32_t ar_ckpfcbUnk12;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlaggedPathCinematicBloc : CKMRSubclass<CKFlaggedPathCinematicBloc, CKCinematicBloc, 5> {
	float ckfpcbUnk0 = 10.0f;
	float ckfpcbSpeed = 6.0f;
	float ckfpcbUnk2 = 0.0f;
	float ckfpcbUnk3 = 5.0f;
	kobjref<CKFlaggedPath> ckfpcbUnk4;
	std::array<int32_t, 3> ckfpcbUnk5 = { -1, 1, -1 };
	uint8_t ckfpcbUnk6 = 0;
	std::array<float, 3> ckfpcbUnk7 = { 5.0f, 5.0f, 5.0f };
	std::array<float, 3> ckfpcbUnk8 = { 1.0f, 1.0f, 1.0f };
	float ckfpcbUnk9 = 0.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKAttachObjectsCinematicBloc : CKMRSubclass<CKAttachObjectsCinematicBloc, CKCinematicBloc, 9> {
	KPostponedRef<CKSceneNode> ckaocbUnk0;
	KPostponedRef<CSGSectorRoot> ckaocbUnk1;
	KPostponedRef<CKSceneNode> ckaocbUnk2;
	uint8_t ckaocbUnk3 = 0;
	int32_t ckaocbUnk4 = 0;
	std::array<float, 3> ckaocbUnk5 = { 0.0f, 0.0f, 0.0f };
	std::array<float, 3> ckaocbUnk6 = { 0.0f, 0.0f, 0.0f };
	uint8_t ckaocbUnk7 = 1;
	uint8_t ckaocbUnk8 = 1;
	Matrix ckaocbUnk9 = Matrix::getIdentity();
	kobjref<CKObject> ckaocbSGHotspot;
	uint8_t ckaocbSpyroByte = 0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKStreamCinematicBloc : CKMRSubclass<CKStreamCinematicBloc, CKCinematicBloc, 10> {
	//int32_t ckscbUnk0;
	float ckscbUnk1;
	float ckscbUnk2;
	float ckscbUnk3;
	float ckscbUnk4;
	int32_t ckscbUnk5;
	std::vector<kobjref<CKStreamCinematicBloc>> ckscbUnk6;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKStreamAloneCinematicBloc : CKMRSubclass<CKStreamAloneCinematicBloc, CKCinematicBloc, 13> {
	float cksacbFadeInDuration = 1.0f;
	float cksacbVolume = 1.0f;
	int32_t cksacbStreamIndex = 0;
	float cksacbUnk3 = 3.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKManageEventCinematicBloc : CKMRSubclass<CKManageEventCinematicBloc, CKCinematicBloc, 15> {
	uint8_t ckmecbUnk0 = 0;
	kobjref<CKCinematicNode> ckmecbUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKManagerEventStopCinematicBloc : CKMRSubclass<CKManagerEventStopCinematicBloc, CKCinematicBloc, 16> {
	uint8_t ckmescbUnk0 = 0;
	uint8_t ckmescbUnk1 = 0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSekensorCinematicBloc : CKMRSubclass<CKSekensorCinematicBloc, CKCinematicBloc, 20> {
	int32_t ckscbUnk0 = 0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKManageCameraCinematicBloc : CKMRSubclass<CKManageCameraCinematicBloc, CKCinematicBloc, 23> {
	kobjref<CKCameraBase> ckmccbUnk0;
	float ckmccbUnk1 = 0.0f;
	uint8_t ckmccbUnk2 = 0;
	KPostponedRef<CKSceneNode> ckmccbUnk3;
	float ckmccbUnk4 = 2.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKPlaySoundCinematicBloc : CKMRSubclass<CKPlaySoundCinematicBloc, CKCinematicBloc, 28> {
	int32_t ckpscbSoundIndex = 0;
	float ckpscbVolume = 1.0f;
	float ckpscbSpeed = 1.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKLightningCinematicBloc : CKMRSubclass<CKLightningCinematicBloc, CKCinematicBloc, 27> {
	std::array<float, 3> cklcbUnk0 = { 0.0f, 10.0f, 0.0f };
	std::array<float, 3> cklcbUnk1 = { 0.0f, 0.0f, 0.0f };
	float cklcbUnk2 = 0.3f;
	float cklcbUnk3 = 10.0f;
	float cklcbUnk4 = 20.0f;
	uint8_t cklcbUnk5 = 1;
	float cklcbUnk6 = 0.0f;
	int32_t cklcbColor = -1;
	float cklcbUnk8 = 0.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSkyCinematicBloc : CKMRSubclass<CKSkyCinematicBloc, CKCinematicBloc, 26> {
	kobjref<CKHkSkyLife> ckscbUnk0;
	float ckscbUnk1 = 0.233f;
	int32_t ckscbUnk2 = 0xFFFFFFFF;
	int32_t ckscbUnk3 = 0xFF808080;
	float ckscbUnk4 = 5.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void init(KEnvironment* kenv);
};

struct CKDisplayPictureCinematicBloc : CKMRSubclass<CKDisplayPictureCinematicBloc, CKCinematicBloc, 22> {
	kobjref<CBillboard2d> ckdpcbBillboard;
	uint32_t ckdpcbColor = 0xFFFFFFFF;
	std::array<float, 5> ckdpcbUnk2 = { 1.0f, 1.0f, 2.0f, 1.0f, 1.0f };
	std::array<float, 4> ckdpcbUnk3 = { 0.5f, 0.5f, 1.0f, 1.0f };
	std::array<uint8_t, 3> ckdpcbUnk4 = { 0, 255, 0 };
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void init(KEnvironment* kenv);
};

struct CKParticleCinematicBloc : CKMRSubclass<CKParticleCinematicBloc, CKCinematicBloc, 12> {
	//int32_t ckpcbUnk0;
	std::vector<KPostponedRef<CKObject>> ckpcbUnk1;
	float ckpcbUnk2 = 1.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

// Cinematic nodes appearing in XXL2+, but also in XXL1 Romastered for some reason
struct CKPauseCinematicBloc : CKMRSubclass<CKPauseCinematicBloc, CKCinematicBloc, 29> {
	float duration = 1.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKTeleportCinematicBloc : CKMRSubclass<CKTeleportCinematicBloc, CKCinematicBloc, 30> {
	float tcbMaybeDuration = 1.0f;
	MarkerIndex tcbMaybeMarkerIndex;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
struct CKEndDoor : CKMRSubclass<CKEndDoor, CKCinematicDoor, 31> {};

// XXL2+

struct CKPlayVideoCinematicBloc : CKMRSubclass<CKPlayVideoCinematicBloc, CKCinematicBloc, 34> {
	struct Video {
		kobjref<CKObject> video;
		uint32_t vidUnk1;
		uint8_t vidUnk2;
	};
	std::vector<Video> ckpvcbVideos;
	int32_t ckpvcbUnk5;
	kobjref<CKObject> arColorizedScreenFx;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashUICinematicBloc : CKMRSubclass<CKFlashUICinematicBloc, CKCinematicBloc, 35> {
	uint8_t swfCbUnk1;
	uint32_t swfCbUnk2;
	kobjref<CKObject> swfCbFlashUI;
	kobjref<CKObject> swfCbOut1;
	kobjref<CKObject> swfCbIn1;
	kobjref<CKObject> swfCbOut2;
	kobjref<CKObject> swfCbIn2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKLockUnlockCinematicBloc : CKMRSubclass<CKLockUnlockCinematicBloc, CKCinematicBloc, 36> {
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};