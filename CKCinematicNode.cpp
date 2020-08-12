#include "CKCinematicNode.h"
#include "File.h"
void CKCinematicNode::reflectMembers(MemberListener &r) {
}
void CKCinematicBloc::reflectMembers(MemberListener &r) {
	CKCinematicNode::reflectMembers(r);
	r.reflect(cbUnk0, "cbUnk0");
	r.reflect(cbUnk1, "cbUnk1");
	r.reflect(cbUnk2, "cbUnk2");
	r.reflect(cbUnk3, "cbUnk3");
	r.reflect(cbUnk4, "cbUnk4");
	r.reflect(cbUnk5, "cbUnk5");
	r.reflect(cbUnk6, "cbUnk6");
	r.reflect(cbUnk7, "cbUnk7");
	r.reflect(cbUnk8, "cbUnk8");
	r.reflect(cbUnk9, "cbUnk9");
}
void CKCinematicDoor::reflectMembers(MemberListener &r) {
	CKCinematicNode::reflectMembers(r);
	r.reflect(cdUnk0, "cdUnk0");
	r.reflect(cdUnk1, "cdUnk1");
	r.reflect(cdUnk2, "cdUnk2");
	r.reflect(cdUnk3, "cdUnk3");
	r.reflect(cdUnk4, "cdUnk4");
	r.reflect(cdUnk5, "cdUnk5");
	r.reflect(cdUnk6, "cdUnk6");
}
void CKStartEventCinematicBloc::reflectMembers(MemberListener &r) {
	CKCinematicBloc::reflectMembers(r);
	r.reflect(seEvtNode, "seEvtNode", this);
}
