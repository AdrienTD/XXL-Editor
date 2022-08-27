#pragma once

#include "KObject.h"

struct CLightSet;
struct MemberListener;
struct KEnvironment;

// For now, instead of deriving from it, we will compose it in every class
// based on it, because right now doing by derivation can be problematic
// (varying class CATEGORY, etc.)
struct IKRenderable : CKSubclass<CKCategory<12>, 42, 13> {
	kobjref<IKRenderable> anotherRenderable;
	uint32_t spUnk1;
	kobjref<CLightSet> lightSet;
	uint32_t flags = 0;

	void deserialize(KEnvironment* kenv, File* file, size_t length = 0) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};
