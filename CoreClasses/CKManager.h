#pragma once

#include "KObject.h"
#include "CKService.h"
#include <vector>

struct CKSound;
struct CTextureDictionary;

struct CKManager : CKCategory<0> {};

struct CKReflectableManager : CKMRSubclass<CKReflectableManager, CKMemberReflectable<CKManager>, 0xBADB01> {
	void reflectMembers2(MemberListener &r, KEnvironment* kenv) {}
};

struct CKServiceManager : CKSubclass<CKManager, 1> {
	std::vector<kobjref<CKService>> services;

	template <typename T> T *addService(KEnvironment *kenv) {
		static_assert(T::CATEGORY == CKService::CATEGORY, "T must be a service.");
		T *srv = kenv->createAndInitObject<T>();
		services.emplace_back(srv);
		return srv;
	}

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGraphic : CKMRSubclass<CKGraphic, CKReflectableManager, 2> {
	kobjref<CKObject> kgfcSgRootNode;
	std::array<float, 4> kgfcUnk1 = { 1.0f,1.0f,1.0f,1.0f };
	std::array<float, 4> kgfcUnk2 = { 1.0f,1.0f,1.0f,1.0f };
	uint32_t kgfcUnk3 = 0xFF0000FF;
	float kgfcUnk4 = 27.0f;
	uint32_t kgfcUnk5 = 1;
	uint8_t kgfcRomasterValue = 1;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};

struct CKGraphicX2 : CKMRSubclass<CKGraphicX2, CKReflectableManager, 2> {
	kobjref<CKObject> kgfcSgRootNode;
	kobjref<CKObject> kgfcSgRootNode2; // OG
	int32_t ckgUnk1;
	float ckgUnk2;
	int32_t ckgUnk3;
	//int32_t ckgUnk4;
	std::vector<kobjref<CKObject>> ckgGfxManagers;
	struct VideoReplacement {
		std::string texName;
		uint8_t ckgUnk17;
		int32_t ckgUnk18;
		int32_t ckgUnk19;
		int32_t ckgUnk20;
		float ckgUnk21;
		int32_t ogvrUnk1;
		int32_t ogvrUnk2;
	};
	std::vector<VideoReplacement> ckgVideoReplacements;
	kobjref<CTextureDictionary> ckgTexDict;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSoundManager : CKMRSubclass<CKSoundManager, CKReflectableManager, 3> {
	struct Tune {
		std::string remasterPath; // Remaster only
		float duration;
		uint32_t arValue1, arValue2;
	};
	// uint32_t sizeFor_ksndmgrSndDicts;
	std::vector<uint32_t> ksndmgrSndDicts;
	kobjref<CKObject> ksndmgrSndDictID;
	uint8_t ksndmgrUnk3 = 1;
	uint8_t ksndmgrUnk4 = 1;
	uint8_t ksndmgrX2UnkByte = 0; // XXL2+
	Vector3 ksndmgrX2UnkVector; // XXL2+
	float ksndmgrUnk5 = 1.0f;
	float ksndmgrUnk6 = 1.0f;
	float ksndmgrUnk7 = 0;
	float ksndmgrUnk8 = 1.0f;
	// uint32_t sizeFor_ksndmgrDings;
	std::vector<Tune> ksndmgrDings;
	float ksndmgrUnk11 = 0.7f;
	float ksndmgrUnk12 = 0.3f;
	float ksndmgrUnk13 = 1.5f;
	float ksndmgrUnk14 = 1.0f;
	uint32_t ksndmgrSampleRate = 22050; // XXL1 only
	std::vector<kobjref<CKSound>> ksndmgrSoundObjects; // XXL2+

	// OG+:
	std::vector<kobjref<CKObject>> ogSoundOutputEffects;
	std::vector<kobjref<CKObject>> ogSoundOutputEffects2;
	std::array<float, 4> ogFloatValues;

	// Global (GAME) variables (Arthur+):
	std::array<float, 3> arGlobFloatValues1; // OG remove one of both
	std::array<float, 3> arGlobFloatValues2;
	// OG+:
	std::vector<kobjref<CKObject>> ogGlobStreamWaves;
	struct StreamType {
		std::string strtName;
		std::array<float, 4> strtValues;
	};
	std::vector<StreamType> ogGlobStreamTypes;

	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
};

struct CKInput : CKSubclass<CKManager, 4> {
	// TODO
	std::vector<uint8_t> data;
	void deserialize(KEnvironment* kenv, File* file, size_t length) {}
	void serialize(KEnvironment* kenv, File* file) {}
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
	void serializeGlobal(KEnvironment* kenv, File* file) override;
};