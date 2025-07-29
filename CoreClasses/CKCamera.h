#pragma once

#include "KObject.h"
#include "CKUtils.h"
#include "CKPartlyUnknown.h"
#include "vecmat.h"

struct CKSceneNode;
struct CKCameraFogDatas;

struct CKCameraBase : CKMemberReflectable<CKSubclass<CKCategory<7>, 0>> {
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

	// XXL2+:
	uint32_t kcamX2Unk0 = 0x60;
	uint8_t kcamX2Unk1 = 2, kcamX2Unk2 = 2, kcamX2Unk3 = 0;
	uint8_t kcamX2Unk4 = 0;

	// Arthur+:
	kobjref<CKCameraFogDatas> ogFogData;
	uint8_t ogUnk1 = 0;
	int32_t ogUnk2a = 1; float ogUnk3a = 0.0f; float ogUnk4a = 0.0f;
	int8_t ogUnk2b = 1; // OG

	// OG360+:
	int32_t ogHdUnk1 = 0;
	kobjref<CKObject> ogHdUnk2;

	kobjref<CKCameraBase> kcamNextCam;

	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
	void init(KEnvironment* kenv) override;
};

struct CKCamera : CKMRSubclass<CKCamera, CKCameraBase, 1> {
	float kcamX2DerUnk0 = 0.0f;
	float kcamOgUnk1 = 0.0f;
	Vector3 kcamOgUnk2 = { 0.0f, 0.0f, 1.0f };
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraRigidTrack : CKMRSubclass<CKCameraRigidTrack, CKCameraBase, 2> {
	// XXL1
	kobjref<CKObject> kcrtUnk1;
	std::array<float, 6> kcrtUnk2;

	// XXL2+:
	Vector3 kcrtX2Vec1;
	Vector3 kcrtX2Vec2;
	float kcrtX2Flt1;
	float kcrtOgFlt2;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraClassicTrack : CKMRSubclass<CKCameraClassicTrack, CKCameraBase, 3> {
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

struct CKCameraPathTrack : CKMRSubclass<CKCameraPathTrack, CKCameraBase, 4> {
	kobjref<CKObject> kcptSpline;
	Vector3 kcptUnk2;
	uint8_t kcptUnk5 = 0xDC;

	std::vector<std::array<float, 2>> kcptX2FltArray;
	Vector3 kcptX2UnkVector;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraFixTrack : CKMRSubclass<CKCameraFixTrack, CKCameraBase, 6> {
	float cftUnk1 = 0;
	uint32_t cftRoma1 = 0, cftRoma2 = 0; // romaster only
	float cftX2Unk2 = 0.0f;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKCameraAxisTrack : CKMRSubclass<CKCameraAxisTrack, CKCameraBase, 7> {
	uint8_t catUnk1;
	Vector3 catUnk2;
	KPostponedRef<CKSceneNode> catNode;
	float catX2Unk = 0.0f;

	// Arthur:
	uint32_t arUnk1; float arUnk2, arUnk3, arUnk4;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKCameraSpyTrack : CKMRSubclass<CKCameraSpyTrack, CKCameraBase, 8> {
	std::array<float, 6> kcamspyValues;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraPassivePathTrack : CKMRSubclass<CKCameraPassivePathTrack, CKCameraBase, 10> {
	kobjref<CKObject> kcpptSpline;
	float kcpptUnk1;
	float kcpptUnk2;
	float kcpptUnk3;
	float kcpptUnk4;
	float kcpptUnk5;
	float kcpptUnk6;
	
	// XXL2+:
	std::array<float, 11> kcpptX2Values;
	uint16_t kcpptUnk7 = 3;
	// OG+:
	kobjref<CKObject> kcpptSecondSpline;
	float kcpptOgUnk1 = 0.0f;
	float kcpptOgUnk2 = 100.0f;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraBalistTrack : CKMRSubclass<CKCameraBalistTrack, CKCameraBase, 11> {
	float cambalUnk1 = 2.0f, cambalUnk2 = 2.5f, cambalUnk3 = -10.0f, cambalUnk4 = 0.0f;
	float arUnk1 = 0.0f, arUnk2 = 0.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraClassicTrack2 : CKMRSubclass<CKCameraClassicTrack2, CKCameraBase, 12> {
	std::array<float, 21> values;
	float arValue;
	std::array<float, 33> ogValues;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKCameraFirstPersonTrack : CKMRSubclass<CKCameraFirstPersonTrack, CKCameraBase, 15> {
	float cfptUnk1 = 1.6f, cfptUnk2 = 90.0f, cfptUnk3 = 90.0f;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
