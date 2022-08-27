#pragma once

#include "KObject.h"
#include <vector>
#include "rw.h"
#include "KLocalObject.h"
#include "CKUtils.h"
#include "IKRenderable.h"
#include "Shape.h"

struct CClone;
struct CSGBranch;
struct CKSceneNode;

struct CKGraphical : CKCategory<13> {

};

template <int I> using CKGraphicalRenderable = CKSubclass<IKRenderable, I, 13>;

struct CCloneManager : CKGraphicalRenderable<3> {
	uint32_t _numClones = 0, _unk1, _unk2, _unk3, _unk4;
	std::vector<kobjref<CSGBranch>> _clones;
	RwTeamDictionary _teamDict;
	RwTeam _team;
	std::vector<std::array<float, 4>> flinfos;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CSectorAnimation : CKSubclass<CKGraphical, 55> {
	struct Animation {
		RwAnimAnimation rwAnim;
		std::array<float, 4> x2AnimVals;
		std::array<int32_t, 4> arAnimValues;
	};
	std::vector<Animation> anims;
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CAnimationManager : CKSubclass<CKGraphical, 8> {
	CSectorAnimation commonAnims; // XXL1/2
	std::vector<KPostponedRef<CSectorAnimation>> arSectors; // Arthur+

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CScene2d;
struct CMessageBox2d;
struct CMenuManager;
struct CText2d;
struct CBillboard2d;
struct CColorTextButton2d;

struct CElement2d : CKSubclass<CKGraphical, 9> {
	uint8_t e2dUnk1, e2dUnk2, e2dUnk3 = 1, e2dUnk4 = 1;
	kobjref<CScene2d> scene;
	kobjref<CElement2d> previous, next;
	float e2dUnk5 = 0.0f, e2dUnk6 = 0.0f, e2dUnk7 = 0.0f;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CManager2d : CKGraphicalRenderable<16> {
	// Global
	uint32_t numFonts;

	// Level
	kobjref<CMenuManager> menuManager;
	kobjref<CScene2d> scene1, scene2, x2scene3, ogscene4;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
};

struct CMenuManager : CKSubclass<CKGraphical, 17> {
	kobjref<CScene2d> scene;
	kobjref<CMessageBox2d> messageBox;
	uint8_t cmm2dVal = 1;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CContainer2d : CKSubclass<CElement2d, 19> {
	std::string name;
	kobjref<CScene2d> scene1, scene2;
	uint8_t cctr2dVal = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CScene2d : CKSubclass<CKGraphical, 20> {
	kobjref<CElement2d> first, last;
	uint32_t numElements = 0;
	uint8_t cs2dVal = 1;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CMessageBox2d : CKSubclass<CElement2d, 21> {
	kobjref<CContainer2d> container;
	uint32_t msgboxUnk = 0;
	kobjref<CText2d> text;
	kobjref<CBillboard2d> billboard;
	kobjref<CColorTextButton2d> button1, button2, button3;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CText2d : CKSubclass<CElement2d, 23> {
	kobjref<CKObject> locManager;
	float ce2dUnk1 = 0.1f, ce2dUnk2 = 0.1f;
	uint32_t ce2dUnk3 = 2, ce2dUnk4 = 0xFFFFFFFF;
	std::wstring text;
	uint32_t ce2dUnk5 = 0;
	RwBrush2D brush;
	std::array<uint32_t, 7> ce2dUnk6;
	uint8_t ce2dUnk7 = 1, ce2dUnk8 = 1, ce2dUnk9 = 0;
	float ce2dUnkA = 1.0f;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CColorTextButton2d : CKSubclass<CText2d, 12> {
	uint32_t ccolor = 0xFF00FF;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CBillboard2d : CKSubclass<CElement2d, 15> {
	RwBrush2D brush;
	std::string texture;
	uint32_t color1 = 0xFFFFFFFF, color2 = 0xCDCDCDCD;
	float flt1 = 1.0f, flt2 = 1.0f;
	uint8_t bb1 = 1, bb2 = 0;
	std::array<float, 8> fltarr = { 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };

	// XXL+2: (maybe x2Byte and arByte are the same?)
	uint8_t x2Byte = 1;
	uint8_t arByte = 1;
	kobjref<CKObject> arUnkObject;
	std::array<int32_t, 5> spIntArray = { 0,0,0,1,1 };

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CBillboard2dList : CKSubclass<CElement2d, 22> {
	struct Bill {
		int32_t cblUnk1;
		RwBrush2D brush;
		uint8_t cblUnk4;
		std::array<float, 8> cblUnk5;
	};
	//int32_t cblUnk0;
	std::vector<Bill> cblBills;

	uint32_t cblUnk10;
	//int32_t cblUnk17;
	std::vector<std::string> cblTextures;
	std::array<float, 2> cblUnk33;
	uint8_t cblUnk34;
	float cblUnk35;
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CBillboardButton2d : CKSubclass<CBillboard2d, 25> {
	uint32_t billButtonColor;
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CLightSet;
struct CLightManager : CKGraphicalRenderable<36> {
	std::vector<kobjref<CLightSet>> lightSets;
	int32_t spUnkInt = 0;
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CLightSet : CKSubclass<CKGraphical, 37> {
	int32_t ogSector = 0;
	int32_t clsUnk0;
	AABoundingBox bbox;
	KPostponedRef<CKSceneNode> sceneNode;
	int32_t spUnkInt = -1;
	std::array<kobjref<CKObject>, 5> lightComponents;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void onLevelLoaded(KEnvironment* kenv) override;
};