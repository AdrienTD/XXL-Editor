#include "CKCamera.h"
#include "CKNode.h"
#include "CKLogic.h"

void CKCamera::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	//CKCameraBase::reflectMembers(r);
	r.reflect(kcamUnk0, "kcamUnk0");
	r.reflect(kcamUnk1, "kcamUnk1");
	r.reflect(kcamUnk2, "kcamUnk2");
	r.reflect(kcamUnk3, "kcamUnk3");
	if (kenv->isRemaster) {
		r.reflect(kcamUnk2_dup, "kcamUnk2_dup");
		r.reflect(kcamUnk3_dup, "kcamUnk3_dup");
	}
	r.reflect(kcamUnk4, "kcamUnk4");
	r.reflect(kcamUnk5, "kcamUnk5");
	r.reflect(kcamUnk6, "kcamUnk6");
	r.reflect(kcamUnk7, "kcamUnk7");
	r.reflect(kcamUnk8, "kcamUnk8");
	r.reflect(kcamNextCam, "kcamNextCam");
}

void CKCameraFixTrack::reflectMembers2(MemberListener &r, KEnvironment *kenv)
{
	CKCamera::reflectMembers2(r, kenv);
	r.reflect(cftUnk1, "cftUnk1");
	if (kenv->isRemaster) {
		r.reflect(cftRoma1, "cftRoma1");
		r.reflect(cftRoma2, "cftRoma2");
	}
}

void CKCameraClassicTrack::reflectMembers2(MemberListener & r, KEnvironment *kenv)
{
	CKCamera::reflectMembers2(r, kenv);
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
	CKCamera::reflectMembers2(r, kenv);
	r.reflect(kcptSpline, "kcptSpline");
	r.reflect(kcptUnk2, "kcptUnk2");
	r.reflect(kcptUnk5, "kcptUnk5");
}

void CKCameraAxisTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCamera::reflectMembers2(r, kenv);
	r.reflect(catUnk1, "catUnk1");
	r.reflect(catUnk2, "catUnk2");
	r.reflect(catNode, "catNode");
}

void CKCameraAxisTrack::onLevelLoaded(KEnvironment* kenv)
{
	// In XXL1 Rome, some referenced scene nodes are in STR,
	// and we have to find their corresponding sectors again...
	uint32_t gid = catNode.id;
	int str = -1;
	if (gid != 0xFFFFFFFF) {
		int clcat = gid & 63;
		int clid = (gid >> 6) & 2047;
		int objid = gid >> 17;

		Vector3 pos = { kcamUnk6[0], kcamUnk6[1], kcamUnk6[2] };
		CKLevel* klevel = kenv->levelObjects.getFirst<CKLevel>();

		int bestSector = -1;
		float bestDist = std::numeric_limits<float>::infinity();
		for (int cand = 0; cand < kenv->numSectors; ++cand) {
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
	CKCamera::reflectMembers2(r, kenv);
	r.reflect(kcamspyValues, "kcamspyValues");
}

void CKCameraPassivePathTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCamera::reflectMembers2(r, kenv);
	r.reflect(kcpptSpline, "kcpptSpline");
	r.reflect(kcpptUnk1, "kcpptUnk1");
	r.reflect(kcpptUnk2, "kcpptUnk2");
	r.reflect(kcpptUnk3, "kcpptUnk3");
	r.reflect(kcpptUnk4, "kcpptUnk4");
	r.reflect(kcpptUnk5, "kcpptUnk5");
	r.reflect(kcpptUnk6, "kcpptUnk6");
	r.reflect(kcpptUnk7, "kcpptUnk7");
}

void CKCameraRigidTrack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKCamera::reflectMembers2(r, kenv);
	r.reflect(kcrtUnk1, "kcrtUnk1");
	r.reflect(kcrtUnk2, "kcrtUnk2");
}
