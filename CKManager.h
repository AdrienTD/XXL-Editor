#pragma once

#include "KObject.h"
#include "CKService.h"
#include <vector>

struct CKManager : CKCategory<0> {};

struct CKReflectableManager : CKMRSubclass<CKReflectableManager, CKMemberReflectable<CKManager>, 0xBADB01> {
	void reflectMembers(MemberListener &r) {}
};

struct CKServiceManager : CKSubclass<CKManager, 1> {
	std::vector<kobjref<CKService>> services;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKGraphic : CKMRSubclass<CKGraphic, CKReflectableManager, 2> {
	kobjref<CKObject> kgfcSgRootNode;
	std::array<float, 4> kgfcUnk1;
	std::array<float, 4> kgfcUnk2;
	uint32_t kgfcUnk3;
	float kgfcUnk4;
	uint32_t kgfcUnk5;
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
	uint8_t ksndmgrUnk3;
	uint8_t ksndmgrUnk4;
	float ksndmgrUnk5;
	float ksndmgrUnk6;
	uint32_t ksndmgrUnk7;
	float ksndmgrUnk8;
	// uint32_t sizeFor_ksndmgrDings;
	std::vector<Tune> ksndmgrDings;
	float ksndmgrUnk11;
	float ksndmgrUnk12;
	float ksndmgrUnk13;
	float ksndmgrUnk14;
	uint32_t ksndmgrSampleRate;
	void reflectMembers2(MemberListener &r, KEnvironment *kenv);
};