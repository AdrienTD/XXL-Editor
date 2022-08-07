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
struct CKCamera;
struct CKHkSkyLife;
struct CBillboard2d;
struct CKFlaggedPath;
struct CKSceneNode;
struct CSGSectorRoot;

struct CKCinematicNode : CKMemberReflectable<CKCategory<8>> {
	void reflectMembers(MemberListener &r);
};
struct CKCinematicBloc : CKMRSubclass<CKCinematicBloc, CKCinematicNode, 33> {
	uint16_t cbStartOutEdge;
	uint16_t cbFinishOutEdge;
	uint16_t cbNumOutEdges;
	kobjref<CKCinematicSceneData> cbSceneData;
	kobjref<CKCinematicBloc> cbGroupBloc;
	uint32_t cbUnk5;
	uint32_t cbUnk6; // Sekens?
	uint32_t cbUnk7; // Sekens?
	kobjref<CKObject> cbSekensMarker;
	float cbUnk8;
	kobjref<CKCinematicScene> cbScene;
	void reflectMembers2(MemberListener &r, KEnvironment* kenv);
};
struct CKCinematicDoor : CKMRSubclass<CKCinematicDoor, CKCinematicNode, 34> {
	uint32_t cdNumInEdges;
	uint16_t cdStartOutEdge;
	uint16_t cdFinishOutEdge;
	uint16_t cdNumOutEdges;
	uint32_t cdUnk4;
	kobjref<CKCinematicBloc> cdGroupBloc;
	kobjref<CKCinematicScene> cdScene;
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
	float ckpacbUnk0;
	int32_t ckpacbUnk1;
	float ckpacbUnk2;
	uint8_t ckpacbUnk3;
	float ckpacbUnk4;
	float ckpacbUnk5;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKPathFindingCinematicBloc : CKMRSubclass<CKPathFindingCinematicBloc, CKCinematicBloc, 4> {
	MarkerIndex ckpfcbUnk0;
	float ckpfcbUnk1;
	int32_t ckpfcbUnk2;
	float ckpfcbUnk3;
	float ckpfcbUnk4;
	uint8_t ckpfcbUnk5;
	float ckpfcbUnk6;

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
	float ckfpcbUnk0;
	float ckfpcbUnk1;
	float ckfpcbUnk2;
	float ckfpcbUnk3;
	kobjref<CKFlaggedPath> ckfpcbUnk4;
	std::array<int32_t, 3> ckfpcbUnk5;
	uint8_t ckfpcbUnk6;
	std::array<float, 3> ckfpcbUnk7;
	std::array<float, 3> ckfpcbUnk8;
	float ckfpcbUnk9;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKAttachObjectsCinematicBloc : CKMRSubclass<CKAttachObjectsCinematicBloc, CKCinematicBloc, 9> {
	KPostponedRef<CKSceneNode> ckaocbUnk0;
	KPostponedRef<CSGSectorRoot> ckaocbUnk1;
	KPostponedRef<CKSceneNode> ckaocbUnk2;
	uint8_t ckaocbUnk3;
	int32_t ckaocbUnk4;
	std::array<float, 3> ckaocbUnk5;
	std::array<float, 3> ckaocbUnk6;
	uint8_t ckaocbUnk7;
	uint8_t ckaocbUnk8;
	Matrix ckaocbUnk9;
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
	float cksacbUnk0;
	float cksacbUnk1;
	int32_t cksacbUnk2;
	float cksacbUnk3;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKManageEventCinematicBloc : CKMRSubclass<CKManageEventCinematicBloc, CKCinematicBloc, 15> {
	uint8_t ckmecbUnk0;
	kobjref<CKCinematicNode> ckmecbUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKManagerEventStopCinematicBloc : CKMRSubclass<CKManagerEventStopCinematicBloc, CKCinematicBloc, 16> {
	uint8_t ckmescbUnk0;
	uint8_t ckmescbUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSekensorCinematicBloc : CKMRSubclass<CKSekensorCinematicBloc, CKCinematicBloc, 20> {
	int32_t ckscbUnk0;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKManageCameraCinematicBloc : CKMRSubclass<CKManageCameraCinematicBloc, CKCinematicBloc, 23> {
	kobjref<CKCamera> ckmccbUnk0;
	float ckmccbUnk1;
	uint8_t ckmccbUnk2;
	KPostponedRef<CKSceneNode> ckmccbUnk3;
	float ckmccbUnk4;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKPlaySoundCinematicBloc : CKMRSubclass<CKPlaySoundCinematicBloc, CKCinematicBloc, 28> {
	int32_t ckpscbUnk0;
	float ckpscbUnk1;
	float ckpscbUnk2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKLightningCinematicBloc : CKMRSubclass<CKLightningCinematicBloc, CKCinematicBloc, 27> {
	std::array<float, 3> cklcbUnk0;
	std::array<float, 3> cklcbUnk1;
	float cklcbUnk2;
	float cklcbUnk3;
	float cklcbUnk4;
	uint8_t cklcbUnk5;
	float cklcbUnk6;
	int32_t cklcbUnk7;
	float cklcbUnk8;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSkyCinematicBloc : CKMRSubclass<CKSkyCinematicBloc, CKCinematicBloc, 26> {
	kobjref<CKHkSkyLife> ckscbUnk0;
	float ckscbUnk1;
	int32_t ckscbUnk2;
	int32_t ckscbUnk3;
	float ckscbUnk4;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKDisplayPictureCinematicBloc : CKMRSubclass<CKDisplayPictureCinematicBloc, CKCinematicBloc, 22> {
	kobjref<CBillboard2d> ckdpcbUnk0;
	int32_t ckdpcbUnk1;
	std::array<float, 5> ckdpcbUnk2;
	std::array<float, 4> ckdpcbUnk3;
	std::array<uint8_t, 3> ckdpcbUnk4;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKParticleCinematicBloc : CKMRSubclass<CKParticleCinematicBloc, CKCinematicBloc, 12> {
	//int32_t ckpcbUnk0;
	std::vector<KPostponedRef<CKObject>> ckpcbUnk1;
	float ckpcbUnk2;
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