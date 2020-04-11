#pragma once

#include "KObject.h"
#include "vecmat.h"

//struct CKGeometry;
#include "CKGeometry.h"

#include "CKPartlyUnknown.h"

//struct CSGMovable : CKSubclass<CKSceneNode, 5> {
//	Matrix transform;
//};

struct CKSceneNode : CKCategory<11> {
	Matrix transform;
	kobjref<CKSceneNode> parent, next;
	uint16_t unk1;
	uint8_t unk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

};

struct CSGLeaf : CKSubclass<CKSceneNode, 8> {
};

struct CSGBranch : CKSubclass<CSGLeaf, 9> {
	kobjref<CKSceneNode> child;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	void insertChild(CKSceneNode *newChild);
};

struct CSGRootNode : CKSubclass<CSGBranch, 1> {
};

struct CNode : CKSubclass<CSGBranch, 3> {
	kobjref<CKAnyGeometry> geometry;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CSGSectorRoot : CKSubclass<CNode, 2> {
	// texture dictionary reference
	kobjref<CKObject> texDictionary;

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
	kobjref<CSGBranch> branchs;
	kobjref<CKObject> unkref;
	uint32_t numBones;
	RwFrameList *frameList;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CAnimatedClone : CKSubclass<CNode, 22> {
	kobjref<CSGBranch> branchs; // in common with CAnimatedNode!!!
	uint32_t cloneInfo;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBoundingShape : CKSubclass<CSGLeaf, 13> {
	uint16_t unk1, unk2;
	float radius;
	kobjref<CKObject> object;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKBoundingSphere : CKSubclass<CKBoundingShape, 14> {};
struct CKDynamicBoundingSphere : CKSubclass<CKBoundingShape, 15> {};

struct CKBoundingBox : CKSubclass<CKBoundingShape, 16> {
	Vector3 boxSize; // or corner position?

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKAABB : CKSubclass<CKBoundingBox, 17> {};
struct CKOBB : CKSubclass<CKBoundingBox, 18> {};

struct CKAACylinder : CKSubclass<CKBoundingShape, 24> {
	float cylinderRadius, cylinderHeight; // I guess, but which one begins first?

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CGlowNodeFx : CKPartlyUnknown<CNode, 11> {};
struct CParticlesNodeFx : CKPartlyUnknown<CNode, 19> {};
struct CSkyNodeFx : CKPartlyUnknown<CNode, 25> {};
struct CFogBoxNodeFx : CKPartlyUnknown<CNode, 26> {};

struct CTrailNodeFx : CKSubclass<CNode, 27> {
	struct TrailPart {
		uint8_t unk1, unk2;
		uint32_t unk3;
		kobjref<CKSceneNode> branch1, branch2;
	};
	uint32_t unk1, unk2, unk3;
	std::vector<TrailPart> parts;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

// Looks similar to CKBoundingSphere :thinking:
struct CKDynBSphereProjectile : CKSubclass<CSGLeaf, 6> {
	uint16_t unk1, unk2;
	float radius;
	kobjref<CKObject> cameraService;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};
