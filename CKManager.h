#pragma once

#include "KObject.h"
#include "CKService.h"
#include <vector>

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
struct CKSoundManager : CKMRSubclass<CKSoundManager, CKReflectableManager, 3> {
	struct Tune {
		std::string remasterPath; // Remaster only
		float value;
		void reflectMembers(MemberListener &r) {
			r.reflect(remasterPath, "remasterPath");
			r.reflect(value, "value");
		}
	};
	// uint32_t sizeFor_ksndmgrSndDicts;
	std::vector<uint32_t> ksndmgrSndDicts;
	kobjref<CKObject> ksndmgrSndDictID;
	uint8_t ksndmgrUnk3 = 1;
	uint8_t ksndmgrUnk4 = 1;
	float ksndmgrUnk5 = 1.0f;
	float ksndmgrUnk6 = 1.0f;
	uint32_t ksndmgrUnk7 = 0;
	float ksndmgrUnk8 = 1.0f;
	// uint32_t sizeFor_ksndmgrDings;
	std::vector<Tune> ksndmgrDings;
	float ksndmgrUnk11 = 0.7f;
	float ksndmgrUnk12 = 0.3f;
	float ksndmgrUnk13 = 1.5f;
	float ksndmgrUnk14 = 1.0f;
	uint32_t ksndmgrSampleRate = 22050;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};