#include "CKCinematicNode.h"
#include "File.h"
#include "CKLogic.h"
#include "CKNode.h"
#include "CKHook.h"
#include "CKGraphical.h"
#include "CKCamera.h"

void CKCinematicNode::reflectMembers(MemberListener &r) {
}
void CKCinematicBloc::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKCinematicNode::reflectMembers(r);
	r.reflect(cbStartOutEdge, "cbStartOutEdge");
	r.reflect(cbFinishOutEdge, "cbFinishOutEdge");
	r.reflect(cbNumOutEdges, "cbNumOutEdges");
	r.reflect(cbSceneData, "cbSceneData");
	r.reflect(cbGroupBloc, "cbGroupBloc");
	r.reflect(cbUnk5, "cbUnk5");
	if (kenv->version < KEnvironment::KVERSION_OLYMPIC) {
		r.reflect(cbUnk6, "cbUnk6");
		r.reflect(cbUnk7, "cbUnk7");
	}
	else {
		r.reflect(cbSekensMarker, "cbSekensMarker");
	}
	r.reflect(cbUnk8, "cbUnk8");
	r.reflect(cbScene, "cbScene");
}
void CKCinematicDoor::reflectMembers(MemberListener &r) {
	CKCinematicNode::reflectMembers(r);
	r.reflect(cdNumInEdges, "cdNumInEdges");
	r.reflect(cdStartOutEdge, "cdStartOutEdge");
	r.reflect(cdFinishOutEdge, "cdFinishOutEdge");
	r.reflect(cdNumOutEdges, "cdNumOutEdges");
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

void CKGroupBlocCinematicBloc::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(gbSubnodes, "sizeFor_gbSubnodes");
	r.reflect(gbSubnodes, "gbSubnodes");
	r.reflect(gbFirstNode, "gbFirstNode");
	r.reflect(gbSecondNode, "gbSecondNode");
}
void CKStreamGroupBlocCinematicBloc::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKGroupBlocCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(sgbUnk0, "sgbUnk0");
	r.reflect(sgbUnk1, "sgbUnk1");
}
void CKStartEventCinematicBloc::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(seEvtNode, "seEvtNode", this);
}

void CKPlayAnimCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckpacbUnk0, "ckpacbUnk0");
	r.reflect(ckpacbUnk1, "ckpacbUnk1");
	r.reflect(ckpacbUnk2, "ckpacbUnk2");
	r.reflect(ckpacbUnk3, "ckpacbUnk3");
	r.reflect(ckpacbUnk4, "ckpacbUnk4");
	r.reflect(ckpacbUnk5, "ckpacbUnk5");
};

void CKPathFindingCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	if (kenv->version < KEnvironment::KVERSION_XXL2) {
		r.reflect(ckpfcbUnk0, "ckpfcbUnk0");
		r.reflect(ckpfcbUnk1, "ckpfcbUnk1");
		r.reflect(ckpfcbUnk2, "ckpfcbUnk2");
		r.reflect(ckpfcbUnk3, "ckpfcbUnk3");
		r.reflect(ckpfcbUnk4, "ckpfcbUnk4");
		r.reflect(ckpfcbUnk5, "ckpfcbUnk5");
		r.reflect(ckpfcbUnk6, "ckpfcbUnk6");
	}
	else {
		r.reflect(x2_ckpfcbUnk0, "x2_ckpfcbUnk0");
		r.reflect(x2_ckpfcbUnk1, "x2_ckpfcbUnk1");
		r.reflect(x2_ckpfcbUnk2, "x2_ckpfcbUnk2");
		r.reflect(x2_ckpfcbUnk3, "x2_ckpfcbUnk3");
		r.reflect(x2_ckpfcbUnk4, "x2_ckpfcbUnk4");
		r.reflect(x2_ckpfcbUnk5, "x2_ckpfcbUnk5");
		r.reflect(x2_ckpfcbUnk6, "x2_ckpfcbUnk6");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ar_ckpfcbUnk7, "ar_ckpfcbUnk7");
			r.reflect(ar_ckpfcbUnk8, "ar_ckpfcbUnk8");
			r.reflect(ar_ckpfcbUnk9, "ar_ckpfcbUnk9");
			r.reflect(ar_ckpfcbUnk10, "ar_ckpfcbUnk10");
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
				r.reflect(ar_ckpfcbUnk11, "ar_ckpfcbUnk11");
				r.reflect(ar_ckpfcbUnk12, "ar_ckpfcbUnk12");
			}
		}
	}
};
void CKFlaggedPathCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckfpcbUnk0, "ckfpcbUnk0");
	r.reflect(ckfpcbUnk1, "ckfpcbUnk1");
	r.reflect(ckfpcbUnk2, "ckfpcbUnk2");
	r.reflect(ckfpcbUnk3, "ckfpcbUnk3");
	r.reflect(ckfpcbUnk4, "ckfpcbUnk4");
	r.reflect(ckfpcbUnk5, "ckfpcbUnk5");
	r.reflect(ckfpcbUnk6, "ckfpcbUnk6");
	r.reflect(ckfpcbUnk7, "ckfpcbUnk7");
	r.reflect(ckfpcbUnk8, "ckfpcbUnk8");
	r.reflect(ckfpcbUnk9, "ckfpcbUnk9");
};
void CKAttachObjectsCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckaocbUnk0, "ckaocbUnk0");
	r.reflect(ckaocbUnk1, "ckaocbUnk1");
	r.reflect(ckaocbUnk2, "ckaocbUnk2");
	r.reflect(ckaocbUnk3, "ckaocbUnk3");
	r.reflect(ckaocbUnk4, "ckaocbUnk4");
	r.reflect(ckaocbUnk5, "ckaocbUnk5");
	r.reflect(ckaocbUnk6, "ckaocbUnk6");
	r.reflect(ckaocbUnk7, "ckaocbUnk7");
	r.reflect(ckaocbUnk8, "ckaocbUnk8");
	r.reflect(ckaocbUnk9, "ckaocbUnk9");
	if (kenv->version >= KEnvironment::KVERSION_XXL2)
		r.reflect(ckaocbSGHotspot, "ckaocbSGHotspot");
	if (kenv->version >= KEnvironment::KVERSION_SPYRO)
		r.reflect(ckaocbSpyroByte, "ckaocbSpyroByte");
}
void CKAttachObjectsCinematicBloc::onLevelLoaded(KEnvironment* kenv)
{
	if (kenv->version < KEnvironment::KVERSION_ARTHUR) {
		ckaocbUnk0.bind(kenv, -1);
		ckaocbUnk2.bind(kenv, -1);
	}
}
void CKStreamCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	//r.reflect(ckscbUnk0, "ckscbUnk0");
	r.reflectSize<uint32_t>(ckscbUnk6, "ckscbUnk6_size");
	r.reflect(ckscbUnk1, "ckscbUnk1");
	r.reflect(ckscbUnk2, "ckscbUnk2");
	r.reflect(ckscbUnk3, "ckscbUnk3");
	r.reflect(ckscbUnk4, "ckscbUnk4");
	r.reflect(ckscbUnk5, "ckscbUnk5");
	r.reflect(ckscbUnk6, "ckscbUnk6");
};
void CKStreamAloneCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(cksacbUnk0, "cksacbUnk0");
	r.reflect(cksacbUnk1, "cksacbUnk1");
	r.reflect(cksacbUnk2, "cksacbUnk2");
	r.reflect(cksacbUnk3, "cksacbUnk3");
};
void CKManageEventCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckmecbUnk0, "ckmecbUnk0");
	r.reflect(ckmecbUnk1, "ckmecbUnk1");
};
void CKManagerEventStopCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckmescbUnk0, "ckmescbUnk0");
	r.reflect(ckmescbUnk1, "ckmescbUnk1");
};
void CKSekensorCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckscbUnk0, "ckscbUnk0");
};
void CKManageCameraCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckmccbUnk0, "ckmccbUnk0");
	r.reflect(ckmccbUnk1, "ckmccbUnk1");
	r.reflect(ckmccbUnk2, "ckmccbUnk2");
	r.reflect(ckmccbUnk3, "ckmccbUnk3");
	r.reflect(ckmccbUnk4, "ckmccbUnk4");
}
void CKManageCameraCinematicBloc::onLevelLoaded(KEnvironment* kenv)
{
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		ckmccbUnk3.bind(kenv, -1);
	}
}
void CKPlaySoundCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckpscbUnk0, "ckpscbUnk0");
	r.reflect(ckpscbUnk1, "ckpscbUnk1");
	r.reflect(ckpscbUnk2, "ckpscbUnk2");
};
void CKLightningCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(cklcbUnk0, "cklcbUnk0");
	r.reflect(cklcbUnk1, "cklcbUnk1");
	r.reflect(cklcbUnk2, "cklcbUnk2");
	r.reflect(cklcbUnk3, "cklcbUnk3");
	r.reflect(cklcbUnk4, "cklcbUnk4");
	r.reflect(cklcbUnk5, "cklcbUnk5");
	r.reflect(cklcbUnk6, "cklcbUnk6");
	r.reflect(cklcbUnk7, "cklcbUnk7");
	r.reflect(cklcbUnk8, "cklcbUnk8");
};
void CKSkyCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckscbUnk0, "ckscbUnk0");
	r.reflect(ckscbUnk1, "ckscbUnk1");
	r.reflect(ckscbUnk2, "ckscbUnk2");
	r.reflect(ckscbUnk3, "ckscbUnk3");
	r.reflect(ckscbUnk4, "ckscbUnk4");
};
void CKDisplayPictureCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(ckdpcbUnk0, "ckdpcbUnk0");
	r.reflect(ckdpcbUnk1, "ckdpcbUnk1");
	r.reflect(ckdpcbUnk2, "ckdpcbUnk2");
	r.reflect(ckdpcbUnk3, "ckdpcbUnk3");
	r.reflect(ckdpcbUnk4, "ckdpcbUnk4");
};
void CKParticleCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	//r.reflect(ckpcbUnk0, "ckpcbUnk0");
	r.reflectSize<uint32_t>(ckpcbUnk1, "ckpcbUnk1_size");
	r.reflect(ckpcbUnk1, "ckpcbUnk1");
	r.reflect(ckpcbUnk2, "ckpcbUnk2");
}
void CKParticleCinematicBloc::onLevelLoaded(KEnvironment* kenv)
{
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		for (auto& ref : ckpcbUnk1)
			ref.bind(kenv, -1);
	}
}

void CKPauseCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(duration, "duration");
}

void CKTeleportCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(tcbMaybeDuration, "tcbMaybeDuration");
	r.reflect(tcbMaybeMarkerIndex, "tcbMaybeMarkerIndex");
}

void CKPlayVideoCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(ckpvcbVideos, "ckpvcbVideos_size");
	r.enterArray("ckpvcbVideos_video");
	for (size_t i = 0; i < ckpvcbVideos.size(); ++i) {
		r.setNextIndex(i);
		r.reflect(ckpvcbVideos[i].video, "video");
	}
	r.leaveArray();
	r.enterArray("ckpvcbVideos_vidUnk1");
	for (size_t i = 0; i < ckpvcbVideos.size(); ++i) {
		r.setNextIndex(i);
		r.reflect(ckpvcbVideos[i].vidUnk1, "vidUnk1");
	}
	r.leaveArray();
	r.enterArray("ckpvcbVideos_vidUnk2");
	for (size_t i = 0; i < ckpvcbVideos.size(); ++i) {
		r.setNextIndex(i);
		r.reflect(ckpvcbVideos[i].vidUnk2, "vidUnk2");
	}
	r.leaveArray();
	r.reflect(ckpvcbUnk5, "ckpvcbUnk5");
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		r.reflect(arColorizedScreenFx, "arColorizedScreenFx");
};

void CKFlashUICinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
	r.reflect(swfCbUnk1, "swfCbUnk1");
	r.reflect(swfCbUnk2, "swfCbUnk2");
	r.reflect(swfCbFlashUI, "swfCbFlashUI");
	r.reflect(swfCbOut1, "swfCbOut1");
	r.reflect(swfCbIn1, "swfCbIn1");
	r.reflect(swfCbOut2, "swfCbOut2");
	r.reflect(swfCbIn2, "swfCbIn2");
}

void CKLockUnlockCinematicBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCinematicBloc::reflectMembers2(r, kenv);
}
