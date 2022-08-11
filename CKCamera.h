#pragma once

#include "KObject.h"
#include "CKUtils.h"
#include "CKPartlyUnknown.h"
#include "vecmat.h"

struct CKSceneNode;

struct CKCameraBase : CKMemberReflectable<CKCategory<7>> {
	void reflectMembers(MemberListener &r) {}
};

struct CKCamera : CKMRSubclass<CKCamera, CKCameraBase, 1> {
	uint32_t kcamUnk0 = 0;
	uint32_t kcamUnk1 = 257;
	float kcamFarDistance = 100.0f, kcamUnk3 = 0.1f;
	float kcamFarDistance_dup = kcamFarDistance, kcamUnk3_dup = kcamUnk3; // duplicates for Romaster
	float kcamUnk4 = 0.1f;

	float kcamFOV = 70.0f;
	Vector3 kcamPosition = { 0.0f, 0.0f, 0.0f },
		kcamLookAt = { 0.0f, 0.0f, 1.0f },
		kcamUpVector = { 0.0f, 1.0f, 0.0f };
	float kcamFOV_dup = kcamFOV;
	Vector3 kcamPosition_dup = kcamPosition, kcamLookAt_dup = kcamLookAt, kcamUpVector_dup = kcamUpVector;

	kobjref<CKCamera> kcamNextCam;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKCameraRigidTrack : CKMRSubclass<CKCameraRigidTrack, CKCamera, 2> {
	kobjref<CKObject> kcrtUnk1;
	std::array<float, 6> kcrtUnk2;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraClassicTrack : CKMRSubclass<CKCameraClassicTrack, CKCamera, 3> {
	uint32_t kclscamUnk0;
	std::array<float, 9> kclscamUnk1;
	std::array<float, 2> kclscamUnk2;
	std::array<float, 12> kclscamUnk3;
	float kclscamUnk4;

	// Romaster only:
	uint8_t kclscamUnk5;
	Vector3 kclscamUnk6;

	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKCameraPathTrack : CKMRSubclass<CKCameraPathTrack, CKCamera, 4> {
	kobjref<CKObject> kcptSpline;
	Vector3 kcptUnk2;
	uint8_t kcptUnk5 = 0xDC;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraFixTrack : CKMRSubclass<CKCameraFixTrack, CKCamera, 6> {
	float cftUnk1 = 0;
	uint32_t cftRoma1 = 0, cftRoma2 = 0; // romaster only
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKCameraAxisTrack : CKMRSubclass<CKCameraAxisTrack, CKCamera, 7> {
	uint8_t catUnk1;
	Vector3 catUnk2;
	KPostponedRef<CKSceneNode> catNode;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKCameraSpyTrack : CKMRSubclass<CKCameraSpyTrack, CKCamera, 8> {
	std::array<float, 6> kcamspyValues;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraPassivePathTrack : CKMRSubclass<CKCameraPassivePathTrack, CKCamera, 10> {
	kobjref<CKObject> kcpptSpline;
	float kcpptUnk1;
	float kcpptUnk2;
	float kcpptUnk3;
	float kcpptUnk4;
	float kcpptUnk5;
	float kcpptUnk6;
	uint8_t kcpptUnk7;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};