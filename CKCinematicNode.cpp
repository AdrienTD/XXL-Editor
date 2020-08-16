#include "CKCinematicNode.h"
#include "File.h"
#include "CKLogic.h"
void CKCinematicNode::reflectMembers(MemberListener &r) {
}
void CKCinematicBloc::reflectMembers(MemberListener &r) {
	CKCinematicNode::reflectMembers(r);
	r.reflect(cbUnk0, "cbUnk0");
	r.reflect(cbUnk1, "cbUnk1");
	r.reflect(cbUnk2, "cbUnk2");
	r.reflect(cbSceneData, "cbSceneData");
	r.reflect(cbGroupBloc, "cbGroupBloc");
	r.reflect(cbUnk5, "cbUnk5");
	r.reflect(cbUnk6, "cbUnk6");
	r.reflect(cbUnk7, "cbUnk7");
	r.reflect(cbUnk8, "cbUnk8");
	r.reflect(cbScene, "cbScene");
}
void CKCinematicDoor::reflectMembers(MemberListener &r) {
	CKCinematicNode::reflectMembers(r);
	r.reflect(cdUnk0, "cdUnk0");
	r.reflect(cdUnk1, "cdUnk1");
	r.reflect(cdUnk2, "cdUnk2");
	r.reflect(cdUnk3, "cdUnk3");
	r.reflect(cdUnk4, "cdUnk4");
	r.reflect(cdGroupBloc, "cdGroupBloc");
	r.reflect(cdScene, "cdScene");
}
void CKLogicalAnd::reflectMembers(MemberListener &r) {
	CKCinematicDoor::reflectMembers(r);
}
void CKLogicalOr::reflectMembers(MemberListener &r) {
	CKCinematicDoor::reflectMembers(r);
}
void CKRandLogicalDoor::reflectMembers(MemberListener &r) {
	CKCinematicDoor::reflectMembers(r);
}
void CKStartDoor::reflectMembers(MemberListener &r) {
	CKCinematicDoor::reflectMembers(r);
}
void CKGroupBlocCinematicBloc::reflectMembers(MemberListener &r) {
	CKCinematicBloc::reflectMembers(r);
	r.reflectSize<uint32_t>(gbSubnodes, "sizeFor_gbSubnodes");
	r.reflect(gbSubnodes, "gbSubnodes");
	r.reflect(gbFirstNode, "gbFirstNode");
	r.reflect(gbSecondNode, "gbSecondNode");
}
void CKStreamGroupBlocCinematicBloc::reflectMembers(MemberListener &r) {
	CKGroupBlocCinematicBloc::reflectMembers(r);
	r.reflect(sgbUnk0, "sgbUnk0");
	r.reflect(sgbUnk1, "sgbUnk1");
}
void CKStartEventCinematicBloc::reflectMembers(MemberListener &r) {
	CKCinematicBloc::reflectMembers(r);
	r.reflect(seEvtNode, "seEvtNode", this);
}