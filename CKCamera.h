#pragma once

#include "KObject.h"
#include "CKUtils.h"
#include "CKPartlyUnknown.h"
#include "vecmat.h"

struct CKCameraBase : CKMemberReflectable<CKCategory<7>> {
	void reflectMembers(MemberListener &r) {}
};

struct CKCamera : CKMRSubclass<CKCamera, CKCameraBase, 1> {
	uint32_t kcamUnk0;
	uint32_t kcamUnk1;
	float kcamUnk2, kcamUnk3;
	float kcamUnk2_dup, kcamUnk3_dup; // duplicates for Romaster
	float kcamUnk4;
	float kcamUnk5;
	std::array<float, 9> kcamUnk6;
	float kcamUnk7;
	std::array<float, 9> kcamUnk8;
	kobjref<CKObject> kcamNextCam;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKCameraRigidTrack : CKPartlyUnknown<CKCamera, 2> {};

struct CKCameraClassicTrack : CKMRSubclass<CKCameraClassicTrack, CKCamera, 3> {
	uint32_t kclscamUnk0;
	std::array<float, 9> kclscamUnk1;
	std::array<float, 2> kclscamUnk2;
	std::array<float, 12> kclscamUnk3;
	uint32_t kclscamUnk4;

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
	uint32_t cftUnk1 = 0;
	uint32_t cftRoma1 = 0, cftRoma2 = 0; // romaster only
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKCameraAxisTrack : CKPartlyUnknown<CKCamera, 7> {};
struct CKCameraSpyTrack : CKPartlyUnknown<CKCamera, 8> {};
struct CKCameraPassivePathTrack : CKPartlyUnknown<CKCamera, 10> {};