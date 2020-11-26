#pragma once

#include "KObject.h"
#include "vecmat.h"

//struct CKGeometry;
#include "CKGeometry.h"

#include "CKPartlyUnknown.h"
#include "CKUtils.h"

//struct CSGMovable : CKSubclass<CKSceneNode, 5> {
//	Matrix transform;
//};

struct CKSceneNode : CKCategory<11> {
	Matrix transform;
	kobjref<CKSceneNode> parent, next;
	uint32_t unk1;
	uint8_t unk2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	Matrix getGlobalMatrix() const;
};

struct CSGLeaf : CKSubclass<CKSceneNode, 8> {
};

struct CSGBranch : CKSubclass<CSGLeaf, 9> {
	kobjref<CKSceneNode> child;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;

	void insertChild(CKSceneNode *newChild);
	void removeChild(CKSceneNode *toremove);
};

struct CSGRootNode : CKSubclass<CSGBranch, 1> {
};

struct CNode : CKSubclass<CSGBranch, 3> {
	kobjref<CKAnyGeometry> geometry;
	std::string romSomeName; // XXL1 Romaster only
	float ogUnkFloat = 1.0f;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CSGSectorRoot : CKSubclass<CNode, 2> {
	uint32_t sectorNum = 0;
	// texture dictionary reference
	kobjref<CKObject> texDictionary;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CClone : CKSubclass<CNode, 12> {
	uint32_t cloneInfo;

	// remaster
	std::string hdKifName;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct RwFrameList;

struct CAnyAnimatedNode : CKSubclass<CNode, 1024> {
	kobjref<CSGBranch> branchs;

	// XXL2 Romaster
	std::vector<std::string> hdBoneNames;

	void deserialize(KEnvironment *kenv, File *file, size_t length) override = 0;
};

struct CAnimatedNode : CKSubclass<CAnyAnimatedNode, 21> {
	//kobjref<CSGBranch> branchs;
	kobjref<CKObject> unkref;
	uint32_t numBones;
	RwFrameList *frameList;

	// XXL2+
	int32_t x2someNum = -1;

	// Arthur+
	std::array<uint8_t, 16> ogBlendBytes;
	kobjref<CKObject> ogBlender;
	float ogBlendFloat = 1.0f;

	~CAnimatedNode();
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CAnimatedClone : CKSubclass<CAnyAnimatedNode, 22> {
	//kobjref<CSGBranch> branchs; // in common with CAnimatedNode!!!
	uint32_t cloneInfo;

	// XXL2+
	int32_t x2someNum = -1;

	// XXL2 Remaster
	std::string hdKifName;

	// Arthur+
	kobjref<CKObject> ogBlender;
	std::array<uint8_t, 16> ogBlendBytes;
	float ogBlendFloat = 1.0f;

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

struct CNodeFx : CKSubclass<CNode, 20> {
	uint8_t fxUnkByte = 0; // only for Arthur+

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CGlowNodeFx : CKPartlyUnknown<CNodeFx, 11> {};
struct CParticlesNodeFx : CKPartlyUnknown<CNodeFx, 19> {};
struct CSkyNodeFx : CKPartlyUnknown<CNodeFx, 25> {};

struct CFogBoxNodeFx : CKSubclass<CNodeFx, 26> {
	uint32_t fogUnk01;
	// u32 count for matrices
	std::vector<std::array<float, 16>> matrices;
	std::string fogUnk02;
	uint8_t fogType, fogUnk04, fogUnk05, fogUnk06, fogUnk07;
	uint32_t fogUnk08, fogUnk09 /* !!! 20 */, fogUnk10;
	Vector3 fogUnk11;
	float fogUnk12;
	// u32 count
	std::vector<std::array<float, 2>> coords;
	std::vector<uint8_t> x2coordStuff;
	uint8_t fogUnk13, fogUnk14;
	uint32_t fogUnk15;
	// u32 count for dings
	struct Ding {
		float flt1,flt2;
		uint32_t color1, color2;
		void reflectMembers(MemberListener &r);
	};
	std::vector<Ding> fogDings;
	// { cond
	std::vector<std::array<float, 7>> fogVec3;
	// }
	float fogUnk16, fogUnk17;
	uint32_t fogX2Unk1, fogX2Unk2, fogX2Unk3, fogX2Unk4;
	// { cond
	std::vector<float> fogVecA, fogVecB;
	// }
	float fogUnk18, fogUnk19, fogUnk20, fogUnk21;
	Vector3 fogUnk22;
	float fogOgUnk1, fogOgUnk2, fogOgUnk3, fogOgUnk4;	// OG specific
	std::array<float, 2> fogUnk23;
	float fogUnk24;
	std::string fogRomaName;

	// OG specific:
	float fogOgUnk5, fogOgUnk6;
	uint8_t fogOgUnk7;
	std::vector<uint8_t> fogOgUnk8;

	void reflectFog(MemberListener &r, KEnvironment *kenv);
	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CTrailNodeFx : CKSubclass<CNodeFx, 27> {
	struct TrailPart {
		uint8_t unk1, unk2;
		uint32_t unk3;
		kobjref<CKSceneNode> branch1, branch2;
		// XXL2+:
		uint32_t tnUnk2, tnUnk3;
	};
	uint32_t tnUnk1, tnUnk2, tnUnk3;
	std::vector<TrailPart> parts;

	// XXL1 Romaster:
	std::string romFxName, romTexName;
	uint8_t romUnkByte;

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

// XXL2+:
// TODO: deserialize refs!!!

struct CSGLight : CKPartlyUnknown<CSGLeaf, 32> {};
struct CCloudsNodeFx : CKPartlyUnknown<CNode, 33> {};
struct CZoneNode : CKPartlyUnknown<CNode, 34> {};
struct CSpawnNode : CKPartlyUnknown<CNode, 35> {};
struct CSpawnAnimatedNode : CKPartlyUnknown<CNode, 36> {}; // not sure about subclass

// Arthur/OG+:

struct CSGAnchor : CKPartlyUnknown<CSGLeaf, 4> {};
struct CSGBkgRootNode : CKSubclass<CNode, 10> {};
