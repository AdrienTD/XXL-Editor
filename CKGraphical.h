#pragma once

#include "KObject.h"
#include <vector>
#include "rw.h"
#include "KLocalObject.h"
#include "CKUtils.h"

struct CClone;
struct CSGBranch;

struct CKGraphical : CKCategory<13> {

};

struct CCloneManager : CKSubclass<CKGraphical, 3> {
	// For XXL2+ : Same values as for the beginning of CGeometry:
	kobjref<CKObject> x2_unkobj1; uint32_t sp_unk1 = 0;
	kobjref<CKObject> x2_lightSet;
	uint32_t x2_flags;

	uint32_t _numClones, _unk1, _unk2, _unk3, _unk4;
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

struct CManager2d : CKSubclass<CKGraphical, 16> {
	// Unfinished, TODO: Differentiate between GAME and LVL !!!
	//uint32_t numFonts;
	kobjref<CMenuManager> menuManager;
	kobjref<CScene2d> scene1, scene2;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
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

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};