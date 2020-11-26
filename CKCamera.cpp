#include "CKCamera.h"

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
