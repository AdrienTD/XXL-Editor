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

struct CKCinematicNode : CKMemberReflectable<CKCategory<8>> {
	void reflectMembers(MemberListener &r);
};
struct CKCinematicBloc : CKMRSubclass<CKCinematicBloc, CKCinematicNode, 33> {
	uint16_t cbUnk0;
	uint16_t cbUnk1;
	uint16_t cbUnk2;
	kobjref<CKCinematicSceneData> cbSceneData;
	kobjref<CKCinematicBloc> cbGroupBloc;
	uint32_t cbUnk5;
	uint32_t cbUnk6;
	uint32_t cbUnk7;
	uint32_t cbUnk8;
	kobjref<CKCinematicScene> cbScene;
	void reflectMembers(MemberListener &r);
};
struct CKCinematicDoor : CKMRSubclass<CKCinematicDoor, CKCinematicNode, 34> {
	uint32_t cdUnk0;
	uint16_t cdUnk1;
	uint16_t cdUnk2;
	uint16_t cdUnk3;
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
	void reflectMembers(MemberListener &r);
};
struct CKStreamGroupBlocCinematicBloc : CKMRSubclass<CKStreamGroupBlocCinematicBloc, CKGroupBlocCinematicBloc, 14> {
	float sgbUnk0;
	float sgbUnk1;
	void reflectMembers(MemberListener &r);
};
struct CKStartEventCinematicBloc : CKMRSubclass<CKStartEventCinematicBloc, CKCinematicBloc, 25> {
	EventNode seEvtNode;
	void reflectMembers(MemberListener &r);
};

struct CKPlayAnimCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 3> {};
struct CKPathFindingCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 4> {};
struct CKFlaggedPathCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 5> {};
//struct CKGroupBlocCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 7> {};
struct CKAttachObjectsCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 9> {};
struct CKStreamCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 10> {};
struct CKParticleCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 12> {};
struct CKStreamAloneCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 13> {};
//struct CKStreamGroupBlocCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 14> {};
struct CKManageEventCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 15> {};
struct CKManagerEventStopCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 16> {};
struct CKSekensorCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 20> {};
struct CKDisplayPictureCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 22> {};
struct CKManageCameraCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 23> {};
//struct CKStartEventCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 25> {};
struct CKSkyCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 26> {};
struct CKLightningCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 27> {};
struct CKPlaySoundCinematicBloc : CKPartlyUnknown<CKCinematicBloc, 28> {};
