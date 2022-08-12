#include "CKCamera.h"
#include "CKNode.h"
#include "CKLogic.h"

void CKCameraBase::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	if (kenv->version == KEnvironment::KVERSION_XXL1) {
		r.reflect(kcamUnk0, "kcamUnk0");
		r.reflect(kcamUnk1, "kcamUnk1");
		r.reflect(kcamFarDistance, "kcamFarDistance");
		r.reflect(kcamUnk3, "kcamUnk3");
		if (kenv->isRemaster) {
			r.reflect(kcamFarDistance_dup, "kcamFarDistance_dup");
			r.reflect(kcamUnk3_dup, "kcamUnk3_dup");
		}
		r.reflect(kcamUnk4, "kcamUnk4");
		r.reflect(kcamFOV, "kcamFOV");
		r.reflect(kcamPosition, "kcamPosition");
		r.reflect(kcamLookAt, "kcamLookAt");
		r.reflect(kcamUpVector, "kcamUpVector");
		r.reflect(kcamFOV_dup, "kcamFOV_dup");
		r.reflect(kcamPosition_dup, "kcamPosition_dup");
		r.reflect(kcamLookAt_dup, "kcamLookAt_dup");
		r.reflect(kcamUpVector_dup, "kcamUpVector_dup");
		r.reflect(kcamNextCam, "kcamNextCam");
	}
	else if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		r.reflect(kcamX2Unk0, "kcamX2Unk0");
		r.reflect(kcamX2Unk1, "kcamX2Unk1");
		r.reflect(kcamX2Unk2, "kcamX2Unk2");
		r.reflect(kcamX2Unk3, "kcamX2Unk3");
		r.reflect(kcamFarDistance, "kcamFarDistance");
		r.reflect(kcamFOV, "kcamFOV");
		r.reflect(kcamPosition, "kcamPosition");
		r.reflect(kcamLookAt, "kcamLookAt");
		r.reflect(kcamNextCam, "kcamNextCam");
		r.reflect(kcamX2Unk4, "kcamX2Unk4");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogFogData, "ogFogData");
			r.reflect(ogUnk1, "ogUnk1");
			r.reflect(ogUnk2a, "ogUnk2a");
			r.reflect(ogUnk3a, "ogUnk3a");
			r.reflect(ogUnk4a, "ogUnk4a");
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
				r.reflect(ogUnk2b, "ogUnk2b");
			}
		}
	}
	r.message("End of CKCamera");
}

void CKCameraBase::init(KEnvironment* kenv)
{
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		ogFogData = kenv->createAndInitObject<CKCameraFogDatas>();
	}
}

void CKCamera::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKCameraBase::reflectMembers2(r, kenv);
	if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		r.reflect(kcamX2DerUnk0, "kcamX2DerUnk0");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(kcamOgUnk1, "kcamOgUnk1");
			r.reflect(kcamOgUnk2, "kcamOgUnk2");
		}
	}
}

void CKCameraFixTrack::reflectMembers2(MemberListener &r, KEnvironment *kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(cftUnk1, "cftUnk1");
	if (kenv->version == KEnvironment::KVERSION_XXL1 && kenv->isRemaster) {
		r.reflect(cftRoma1, "cftRoma1");
		r.reflect(cftRoma2, "cftRoma2");
	}
	if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		r.reflect(cftX2Unk2, "cftX2Unk2");
	}
}

void CKCameraClassicTrack::reflectMembers2(MemberListener & r, KEnvironment *kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(kclscamUnk0, "kclscamUnk0");
	r.reflect(kclscamUnk1, "kclscamUnk1");
	r.reflect(kclscamUnk2, "kclscamUnk2");
	r.reflect(kclscamUnk3, "kclscamUnk3");
	r.reflect(kclscamUnk4, "kclscamUnk4");
	if (kenv->isRemaster) {
		r.reflect(kclscamUnk5, "kclscamUnk5");
		r.reflect(kclscamUnk6, "kclscamUnk6");
	}
}

void CKCameraPathTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(kcptSpline, "kcptSpline");
	r.reflect(kcptUnk2, "kcptUnk2");
	if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		r.reflectSize<uint32_t>(kcptX2FltArray, "kcptX2FltArray_size");
		r.reflect(kcptX2FltArray, "kcptX2FltArray");
		r.reflect(kcptX2UnkVector, "kcptX2UnkVector");
	}
	r.reflect(kcptUnk5, "kcptUnk5");
}

void CKCameraAxisTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	if (kenv->version == KEnvironment::KVERSION_XXL1) {
		r.reflect(catUnk1, "catUnk1");
		r.reflect(catUnk2, "catUnk2");
		r.reflect(catNode, "catNode");
	}
	else if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		r.reflect(catUnk2, "catUnk2");
		r.reflect(catX2Unk, "catX2Unk");
		r.reflect(catNode, "catNode");
		r.reflect(catUnk1, "catUnk1");
	}
}

void CKCameraAxisTrack::onLevelLoaded(KEnvironment* kenv)
{
	if (kenv->version != KEnvironment::KVERSION_XXL1)
		return;
	// In XXL1 Rome, some referenced scene nodes are in STR,
	// and we have to find their corresponding sectors again...
	uint32_t gid = catNode.id;
	int str = -1;
	if (gid != 0xFFFFFFFF) {
		int clcat = gid & 63;
		int clid = (gid >> 6) & 2047;
		int objid = gid >> 17;

		Vector3 pos = kcamPosition;
		CKLevel* klevel = kenv->levelObjects.getFirst<CKLevel>();

		int bestSector = -1;
		float bestDist = std::numeric_limits<float>::infinity();
		for (int cand = 0; cand < (int)kenv->numSectors; ++cand) {
			auto& cl = kenv->sectorObjects[cand].categories[clcat].type[clid];
			int objIndex = objid - cl.startId;
			if (objIndex >= 0 && objIndex < (int)cl.objects.size()) {
				CKSector* ksector = klevel->sectors[cand + 1].get();
				const AABoundingBox& bb1 = ksector->boundaries;
				CKSceneNode* node = (CKSceneNode*)cl.objects[objIndex];
				// Shortest 2D Euclidean distance between sector bounding box and node's position
				float x = std::max(bb1.lowCorner.x - pos.x, 0.0f) + std::max(pos.x - bb1.highCorner.x, 0.0f);
				float y = std::max(bb1.lowCorner.y - pos.y, 0.0f) + std::max(pos.y - bb1.highCorner.y, 0.0f);
				float z = std::max(bb1.lowCorner.z - pos.z, 0.0f) + std::max(pos.z - bb1.highCorner.z, 0.0f);
				float dist = x*x + z*z;
				printf(" - Sector %i, Dist %f\n", cand, dist);
				if (dist < bestDist) {
					bestDist = dist;
					bestSector = cand;
				}
			}
		}
		str = bestSector;
	}
	printf("binding CKCameraAxisTrack's node to sector %i\n", str);
	catNode.bind(kenv, str);
}

void CKCameraSpyTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(kcamspyValues, "kcamspyValues");
}

void CKCameraPassivePathTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(kcpptSpline, "kcpptSpline");
	if (kenv->version == KEnvironment::KVERSION_XXL1) {
		r.reflect(kcpptUnk1, "kcpptUnk1");
		r.reflect(kcpptUnk2, "kcpptUnk2");
		r.reflect(kcpptUnk3, "kcpptUnk3");
		r.reflect(kcpptUnk4, "kcpptUnk4");
		r.reflect(kcpptUnk5, "kcpptUnk5");
		r.reflect(kcpptUnk6, "kcpptUnk6");
	}
	else if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(kcpptSecondSpline, "kcpptSecondSpline");
			r.reflect(kcpptOgUnk1, "kcpptOgUnk1");
		}
		r.reflect(kcpptX2Values, "kcpptX2Values");
	}
	if (kenv->version < KEnvironment::KVERSION_ARTHUR)
		r.reflectAs<uint8_t>(kcpptUnk7, "kcpptUnk7");
	else
		r.reflect(kcpptUnk7, "kcpptUnk7");
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		r.reflect(kcpptOgUnk2, "kccptOgUnk2");
	}
}

void CKCameraRigidTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(kcrtUnk1, "kcrtUnk1");
	r.reflect(kcrtUnk2, "kcrtUnk2");
}

void CKCameraBalistTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(cambalUnk1, "cambalUnk1");
	r.reflect(cambalUnk2, "cambalUnk2");
	r.reflect(cambalUnk3, "cambalUnk3");
	r.reflect(cambalUnk4, "cambalUnk4");
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		r.reflect(arUnk1, "arUnk1");
		r.reflect(arUnk2, "arUnk2");
	}
}

void CKCameraClassicTrack2::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	if(kenv->version < KEnvironment::KVERSION_OLYMPIC)
		r.reflect(values, "values");
	else
		r.reflect(ogValues, "ogValues");
}

void CKCameraFirstPersonTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCameraBase::reflectMembers2(r, kenv);
	r.reflect(cfptUnk1, "cfptUnk1");
	r.reflect(cfptUnk2, "cfptUnk2");
	r.reflect(cfptUnk3, "cfptUnk3");
}
