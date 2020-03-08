#pragma once

#include "KObject.h"
#include "vecmat.h"

//struct CKGeometry;
#include "CKGeometry.h"

struct CKSceneNode : CKCategory<11> {
	Matrix transform;
	objref<CKSceneNode> parent, next;
	uint16_t unk1;
	uint8_t unk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

};

struct CSGLeaf : CKSubclass<CKSceneNode, 8> {
};

struct CSGBranch : CKSubclass<CSGLeaf, 9> {
	objref<CKSceneNode> child;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	void insertChild(CKSceneNode *newChild);
};

struct CSGRootNode : CKSubclass<CSGBranch, 1> {
};

struct CNode : CKSubclass<CSGBranch, 3> {
	objref<CKGeometry> geometry;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CSGSectorRoot : CKSubclass<CNode, 2> {
	// texture dictionary reference
	objref<CKObject> texDictionary;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CClone : CKSubclass<CNode, 12> {
	uint32_t cloneInfo;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct RwFrameList;

struct CAnimatedNode : CKSubclass<CNode, 21> {
	objref<CSGBranch> branchs;
	objref<CKObject> unkref;
	uint32_t numBones;
	RwFrameList *frameList;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};