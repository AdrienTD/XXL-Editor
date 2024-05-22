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

struct CKReflectableGraphical : CKMRSubclass<CKReflectableGraphical, CKMemberReflectable<CKGraphical>, 0x1AAB> {
	void reflectMembers2(MemberListener& r, KEnvironment* kenv) {}
};

template <int I> using CKGraphicalRenderable = CKSubclass<IKRenderable, I, 13>;

struct CKReflectableRenderable : CKMRSubclass<CKReflectableRenderable, CKMemberReflectable<CKGraphicalRenderable<0x1AAD>>, 0x1AAE> {
	void reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		IKRenderable::reflectMembers2(r, kenv);
	}
};

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
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override {}

	int32_t addAnimation(RwAnimAnimation& rwAnim, int sectorIndex = 0);
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

struct CScene2d : CKGraphicalRenderable<20> {
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
	uint32_t ce2dUnk3 = 2;
	uint32_t ce2dUnk4 = 0xFFFFFFFF; // Removed in OG
	std::wstring text;
	uint32_t ce2dUnk5 = 0;
	RwBrush2D brush;
	std::array<uint32_t, 7> ce2dUnk6;
	uint8_t ce2dUnk7 = 1, ce2dUnk8 = 1, ce2dUnk9 = 0;
	float ce2dUnkA = 1.0f;

	// OG:
	kobjref<CKObject> ogLocTextAccessor;
	kobjref<CKObject> ogText2d_1;
	kobjref<CKObject> ogText2d_2;

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

//

struct CBlurData : CKMRSubclass<CBlurData, CKReflectableGraphical, 10> {
	int32_t cbdUnk0;
	uint8_t cbdUnk1;
	float cbdUnk2;
	float cbdUnk3;
	float cbdUnk4;
	float cbdUnk5;
	float cbdUnk6;
	float cbdUnk7;
	float cbdUnk8;
	float cbdUnk9;
	float cbdUnk10;
	uint8_t cbdUnk11;
	int32_t cbdUnk12;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKPBuffer : CKMRSubclass<CKPBuffer, CKReflectableRenderable, 13> {
	std::string ogString;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CDistortionScreenFx;
struct CPostRenderingFx : CKMRSubclass<CPostRenderingFx, CKReflectableRenderable, 14> {
	kobjref<CKPBuffer> cprfUnk0;
	kobjref<CDistortionScreenFx> cprfUnk1;
	int32_t cprfUnk2;
	int32_t cprfUnk3;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CHDRData : CKMRSubclass<CHDRData, CKReflectableGraphical, 21> {
	int32_t chdrdUnk0;
	int32_t chdrdUnk1;
	int32_t chdrdUnk2;
	uint8_t chdrdUnk3;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKSpawnPoolParams : CKMRSubclass<CKSpawnPoolParams, CKReflectableGraphical, 24> {
	float cksppUnk0;
	float cksppUnk1;
	float cksppUnk2;
	int32_t cksppUnk3;
	float cksppUnk4;

	// OG
	kobjref<CKObject> ogQuakeCpnt;
	uint32_t ogFlags;
	std::vector<kobjref<CKObject>> ogQCUpdaters;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CBackgroundManager : CKMRSubclass<CBackgroundManager, CKReflectableRenderable, 26> {
	uint8_t cbmUnk0;
	std::vector<std::tuple<KPostponedRef<CKSceneNode>, float, float, float, float, float>> cbmUnk1;
	uint8_t cbmUnk2;
	uint8_t cbmUnk3;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void onLevelLoaded(KEnvironment* kenv) override;
};

struct CKFlashAnimation;
struct CKFlashText;
struct CKFlashMessageIn;
struct CKFlashMessageOut;
struct CFlashHotSpot;
struct CKSoundDictionaryID;
struct CKSector;

struct CKFlashUI : CKMRSubclass<CKFlashUI, CKReflectableGraphical, 27> {
	std::vector<kobjref<CKFlashAnimation>> ckfuiAnims;
	std::vector<kobjref<CKFlashText>> ckfuiTexts;
	std::vector<kobjref<CKFlashMessageIn>> ckfuiMsgIns;
	std::vector<kobjref<CKFlashMessageOut>> ckfuiMsgOuts;
	std::vector<kobjref<CKObject>> ckfuiUsers;
	kobjref<CKSoundDictionaryID> ckfuiSndDict;
	int32_t ckfuiUnk24;
	std::vector<kobjref<CFlashHotSpot>> ckfuiHotSpots;
	std::vector<int32_t> ckfuiUnk28; // count 16 for XXL2, 40 for OG
	int32_t ckfuiUnk29;

	std::vector<uint8_t> rwMaestroData;

	// OG
	kobjref<CKObject> ogUnkObj;
	int32_t ogEnd1;
	kobjref<CKSector> ogSector;
	int32_t ogEnd2;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
};

struct CKFlashManager : CKMRSubclass<CKFlashManager, CKReflectableGraphical, 28> {
	//int32_t ckfmUnk0;
	std::vector<kobjref<CKFlashUI>> ckfmFlashUI;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashBase : CKMRSubclass<CKFlashBase, CKReflectableGraphical, 29> {
	std::string ckfaUnk0;
	kobjref<CKFlashUI> ckfaUnk1;
	int32_t ckfaUnk2;
	int8_t ckfaOgUnk = 1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashAnimation : CKMRSubclass<CKFlashAnimation, CKFlashBase, 30> {
	std::vector<std::pair<uint8_t, std::string>> ckfaUnk5;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashText : CKMRSubclass<CKFlashText, CKFlashBase, 31> {
	int32_t ckftUnk3;
	uint8_t ckftUnk4;
	float ckftUnk5;
	int32_t ckftUnk6;
	int32_t ckftUnk7;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashPlaySoundEvent;
struct CKFlashMessageIn : CKMRSubclass<CKFlashMessageIn, CKFlashBase, 33> {
	//int32_t ckfmiUnk0;
	std::vector<kobjref<CKFlashPlaySoundEvent>> ckfmiUnk1;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CKFlashMessageOut : CKMRSubclass<CKFlashMessageOut, CKFlashBase, 34> {
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CLightSet;
struct CLightManager : CKGraphicalRenderable<36> {
	std::vector<kobjref<CLightSet>> lightSets;
	std::vector<kobjref<CKObject>> spHDStuff; // used on HD platforms (PS3/X360), empty on previous gen platforms
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

struct CColorizedScreenFx;
struct CVideoManager : CKGraphicalRenderable<39> {
	// Global
	std::vector<kobjref<CKObject>> videos;
	float vmFloat1, vmFloat2;

	// OG LVL:
	kobjref<CColorizedScreenFx> ogColorizedScreenFx;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override;
};

struct CKSpawnPool;
struct CSpawnManager : CKMRSubclass<CSpawnManager, CKReflectableRenderable, 46> {
	std::vector<kobjref<CKSpawnPool>> spawnPools;
	std::vector<kobjref<CKSpawnPoolParams>> spParams;
	// OG
	std::vector<kobjref<CKObject>> ogRenderParams;
	std::vector<kobjref<CKSceneNode>> ogNodes;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	void deserializeGlobal(KEnvironment* kenv, File* file, size_t length) override {}
};

struct CKSpawnPool : CKMRSubclass<CKSpawnPool, CKReflectableGraphical, 47> {
	//int32_t ckspUnk0;
	std::vector<kobjref<CKSpawnPoolParams>> ckspUnk1;
	uint32_t ckspUnk8;
	std::vector<std::tuple<uint32_t, std::shared_ptr<RwFrameList>, float>> ckspUnk9;
	// OG
	std::vector<kobjref<CKObject>> ogRenderParams;
	std::vector<kobjref<CKSceneNode>> ogNodes;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CFlashHotSpot : CKMRSubclass<CFlashHotSpot, CKReflectableGraphical, 48> {
	std::vector<std::tuple<kobjref<CKGraphical>, std::array<float, 2>, int32_t>> fhs2DList;
	std::vector<std::tuple<kobjref<CKSceneNode>, Vector3, int32_t>> fhs3DList;
	kobjref<CKGraphical> cfhsUnk5;
	kobjref<CKFlashUI> cfhsUnk6;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CDistortionScreenFx : CKGraphicalRenderable<49> {};

struct CDistortionScreenData : CKMRSubclass<CDistortionScreenData, CKReflectableGraphical, 50> {
	uint8_t cdsdUnk0;
	float cdsdUnk1;
	float cdsdUnk2;
	float cdsdUnk3;
	int32_t cdsdUnk4;
	//int32_t cdsdUnk5;
	std::vector<std::array<float, 2>> cdsdUnk6;
	std::array<float, 3> cdsdUnk7;
	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};

struct CColorizedScreenFx : CKGraphicalRenderable<51> {};

struct CFlashMessageBox2d : CKMRSubclass<CFlashMessageBox2d, CKReflectableGraphical, 52> {
	kobjref<CKFlashUI> cfmbUnk0;
	kobjref<CFlashHotSpot> cfmbUnk1;
	kobjref<CKFlashAnimation> cfmbUnk2;
	kobjref<CKFlashAnimation> cfmbUnk3;
	std::array<kobjref<CKFlashMessageOut>, 7> cfmbUnk4;
	std::array<std::pair<kobjref<CKFlashText>, kobjref<CKFlashMessageIn>>, 3> cfmbUnk12;
	kobjref<CText2d> cfmbUnk17;

	// OG
	kobjref<CKObject> ogMenuMessageBox;

	void reflectMembers2(MemberListener& r, KEnvironment* kenv);
};