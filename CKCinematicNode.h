#pragma once

#include "KEnvironment.h"
#include "CKUtils.h"
#include "vecmat.h"
#include "Events.h"
#include <array>
#include <vector>

struct CKCinematicNode : CKMemberReflectable<CKCategory<8>> {
	void reflectMembers(MemberListener &r);
};
struct CKCinematicBloc : CKMRSubclass<CKCinematicBloc, CKCinematicNode, 33> {
	uint16_t cbUnk0;
	uint16_t cbUnk1;
	uint16_t cbUnk2;
	kobjref<CKObject> cbUnk3;
	kobjref<CKObject> cbUnk4;
	uint32_t cbUnk5;
	uint32_t cbUnk6;
	uint32_t cbUnk7;
	uint32_t cbUnk8;
	kobjref<CKObject> cbUnk9;
	void reflectMembers(MemberListener &r);
};
struct CKCinematicDoor : CKMRSubclass<CKCinematicDoor, CKCinematicNode, 34> {
	uint32_t cdUnk0;
	uint16_t cdUnk1;
	uint16_t cdUnk2;
	uint16_t cdUnk3;
	uint32_t cdUnk4;
	kobjref<CKObject> cdUnk5;
	kobjref<CKObject> cdUnk6;
	void reflectMembers(MemberListener &r);
};
struct CKStartEventCinematicBloc : CKMRSubclass<CKStartEventCinematicBloc, CKCinematicBloc, 25> {
	EventNode seEvtNode;
	void reflectMembers(MemberListener &r);
};