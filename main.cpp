#include "KEnvironment.h"
#include "CKManager.h"
#include "CKHook.h"
#include "CKGroup.h"
#include "CKComponent.h"
#include "CKCamera.h"
#include "CKCinematicNode.h"
#include "CKDictionary.h"
#include "CKGeometry.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKGraphical.h"
#include <cassert>
#include "rw.h"
#include "rwext.h"
#include <stack>
#include "main.h"
#include "window.h"
#include "renderer.h"
#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "rwrenderer.h"
#include "Camera.h"
#include "EditorInterface.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <Windows.h>
#include <commdlg.h>
#include <io.h>

#include <INIReader.h>

Window *g_window = nullptr;

int cpntcsv() {
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.loadGame(gamePath.c_str(), KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC);

	struct NameListener : MemberListener {
		FILE *csv;
		NameListener(FILE *csv) : csv(csv) {}
		void write(const char *name) { fprintf(csv, "%s\t", name); }
		void reflect(uint8_t &ref, const char *name) override { write(name); }
		void reflect(uint16_t &ref, const char *name) override { write(name); }
		void reflect(uint32_t &ref, const char *name) override { write(name); }
		void reflect(float &ref, const char *name) override { write(name); }
		void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { write(name); }
		void reflect(Vector3 &ref, const char *name) override { fprintf(csv, "%s X\t%s Y\t%s Z\t", name, name, name); }
		void reflect(EventNode &ref, const char *name, CKObject *user) override { write(name); };
		void reflect(std::string &ref, const char *name) override { abort(); } // TODO
	};
	struct ValueListener : MemberListener {
		FILE *csv;
		ValueListener(FILE *csv) : csv(csv) {}
		void reflect(uint8_t &ref, const char *name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint16_t &ref, const char *name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint32_t &ref, const char *name) override { fprintf(csv, "%u\t", ref); }
		void reflect(float &ref, const char *name) override { fprintf(csv, "%f\t", ref); }
		void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { fprintf(csv, "%s\t", ref._pointer->getClassName()); }
		void reflect(Vector3 &ref, const char *name) override { fprintf(csv, "%f\t%f\t%f\t", ref.x, ref.y, ref.z); }
		void reflect(EventNode &ref, const char *name, CKObject *user) override { fprintf(csv, "(%i,%i)\t", ref.seqIndex, ref.bit); };
		void reflect(std::string &ref, const char *name) override { abort(); } // TODO
	};

	FILE *csv;
	fopen_s(&csv, "EnemyCpnts.txt", "w");
	NameListener nl(csv);
	ValueListener vl(csv);

	for (int lvl = 1; lvl <= 6; lvl++) {
		kenv.loadLevel(lvl);
		if (lvl == 1) {
			CKBasicEnemyCpnt *firstcpnt = kenv.levelObjects.getFirst<CKBasicEnemyCpnt>();
			fprintf(csv, "Level\tIndex\t");
			firstcpnt->reflectMembers2(nl, &kenv);
		}
		int index = 0;
		for (CKObject *obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
			fprintf(csv, "\n%i\t%i\t", lvl, index);
			obj->cast<CKBasicEnemyCpnt>()->reflectMembers2(vl, &kenv);
			index++;
		}
	}

	fclose(csv);
	return 0;
}

int x2plus_test()
{
	static constexpr std::array<std::tuple<const char *, int, int>, 4> games = { {
		{"C:\\Users\\Adrien\\Downloads\\virtualboxshare\\aoxxl2demo\\Astérix & Obélix XXL2 DEMO", KEnvironment::KVERSION_XXL2, KEnvironment::PLATFORM_PC},
		{"C:\\Apps\\Asterix at the Olympic Games", KEnvironment::KVERSION_OLYMPIC, KEnvironment::PLATFORM_PC},
		{"D:\\PSP_GAME\\USRDIR", KEnvironment::KVERSION_ARTHUR, KEnvironment::PLATFORM_PSP},
		{"C:\\Users\\Adrien\\Desktop\\kthings\\xxl1_mp_orig", KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC},
	} };
	static const std::string outputPath = "C:\\Users\\Adrien\\Desktop\\kthings\\x2savetest";
	for (const auto &game : games) {
		printf("****** %s ******\n", std::get<0>(game));

		KEnvironment kenv;
		kenv.loadGame(std::get<0>(game), std::get<1>(game), std::get<2>(game));
		kenv.outGamePath = outputPath;
		kenv.loadLevel(1);
		kenv.saveLevel(1);

		std::string lvlpath = std::string("\\LVL001\\LVL01.") + KEnvironment::platformExt[kenv.platform];
		std::string inlvlpath = std::get<0>(game) + lvlpath;
		std::string outlvlpath = outputPath + lvlpath;
		DynArray<uint8_t> inCnt, outCnt;

		for (std::pair<const char *, DynArray<uint8_t> *> p : { std::make_pair(inlvlpath.c_str(), &inCnt), std::make_pair(outlvlpath.c_str(), &outCnt) }) {
			FILE *file;
			fopen_s(&file, p.first, "rb");
			fseek(file, 0, SEEK_END);
			size_t len = ftell(file);
			fseek(file, 0, SEEK_SET);
			p.second->resize(len);
			fread(p.second->data(), len, 1, file);
			fclose(file);
		}

		int numDiffBytes = 0;
		size_t minSize = (inCnt.size() < outCnt.size()) ? inCnt.size() : outCnt.size();

		for (size_t i = 0; i < minSize; i++)
			if (inCnt[i] != outCnt[i])
				numDiffBytes++;

		printf("************\n");
		printf(" Input file size: %10u bytes\n", inCnt.size());
		printf("Output file size: %10u bytes\n", outCnt.size());
		printf(" Different bytes: %10u bytes\n", numDiffBytes);
		printf("************\n");
	}
	getchar();
	return 0;
}

// Creates an empty level to the kenv.
// Note that for now it creates objects one by one, but this might change soon...
void MakeEmptyXXL1Level(KEnvironment &kenv)
{
	kenv.numSectors = 0;
	kenv.lvlUnk1 = 0;
	static const int clcnt[15] = { 5, 15, 208, 127, 78, 30, 32, 11, 33, 5, 4, 28, 133, 26, 6 };
	static const int definfo[15] = { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 2, 0 };
	for (int i = 0; i < 15; i++) {
		kenv.levelObjects.categories[i].type.resize(clcnt[i]);
		for (auto &cl : kenv.levelObjects.categories[i].type)
			cl.info = definfo[i];
	}
	kenv.levelObjects.getClassType<CKLevel>().info = 0;
	kenv.levelObjects.getClassType<CKCoreManager>().info = 0;
	kenv.levelObjects.getClassType<CKAsterixGameManager>().info = 0;
	kenv.levelObjects.getClassType<CSGRootNode>().info = 1;
	kenv.levelObjects.getClassType<CManager2d>().info = 0;
	kenv.levelObjects.getClassType<CMenuManager>().info = 1;
	kenv.levelObjects.getClassType<CScene2d>().info = 1;
	kenv.levelLoaded = true;

	CKCoreManager *core = kenv.createObject<CKCoreManager>(-1);
	CKGroupRoot *grpRoot = kenv.createObject<CKGroupRoot>(-1);
	CKServiceLife *srvLife = kenv.createObject<CKServiceLife>(-1);
	core->groupRoot = grpRoot;
	core->srvLife = srvLife;

	//CKSrvCinematic *srvCinematic = kenv.createObject<CKSrvCinematic>(-1);
	CKServiceManager *srvManager = kenv.createObject<CKServiceManager>(-1);
	//for (CKService* srv : { srvCinematic })
	//	srvManager->services.emplace_back(srv);
	srvManager->addService<CKSrvCinematic>(&kenv);

	CKAsterixGameManager *gameMgr = kenv.createObject<CKAsterixGameManager>(-1);

	CKSector *sector = kenv.createObject<CKSector>(-1);
	sector->strId = 0;
	CKMeshKluster *meshKluster = kenv.createObject<CKMeshKluster>(-1);
	sector->meshKluster = meshKluster;
	CKSoundDictionary *sndDict = kenv.createObject<CKSoundDictionary>(-1);
	sndDict->inactive = 0;
	sector->soundDictionary = sndDict;
	CSGSectorRoot *strRoot = kenv.createObject<CSGSectorRoot>(-1);
	sector->sgRoot = strRoot;

	CKLevel *level = kenv.createObject<CKLevel>(-1);
	level->lvlNumber = 8;
	level->sectors.emplace_back(sector);

	CSGRootNode *sgroot = kenv.createObject<CSGRootNode>(-1);
	CKGraphic *graphic = kenv.createObject<CKGraphic>(-1);
	graphic->kgfcSgRootNode = sgroot;

	CManager2d *mgr2d = kenv.createObject<CManager2d>(-1);
	mgr2d->scene1 = kenv.createObject<CScene2d>(-1);
	mgr2d->scene2 = kenv.createObject<CScene2d>(-1);
	mgr2d->menuManager = kenv.createObject<CMenuManager>(-1);
	mgr2d->menuManager->scene = kenv.createObject<CScene2d>(-1);
	auto *ccon = kenv.createObject<CContainer2d>(-1);
	ccon->e2dUnk1 = 5;
	ccon->e2dUnk2 = 0;
	ccon->scene = mgr2d->menuManager->scene;
	ccon->scene1 = kenv.createObject<CScene2d>(-1);
	ccon->scene2 = kenv.createObject<CScene2d>(-1);
	mgr2d->menuManager->scene->first = mgr2d->menuManager->scene->last = ccon;
	mgr2d->menuManager->scene->numElements = 1;

	auto *msgbox = kenv.createObject<CMessageBox2d>(-1);
	mgr2d->menuManager->messageBox = msgbox;
	msgbox->container = kenv.createObject<CContainer2d>(-1);
	msgbox->text = kenv.createObject<CText2d>(-1);
	msgbox->billboard = kenv.createObject<CBillboard2d>(-1);
	msgbox->button1 = kenv.createObject<CColorTextButton2d>(-1);
	msgbox->button2 = kenv.createObject<CColorTextButton2d>(-1);
	msgbox->button3 = kenv.createObject<CColorTextButton2d>(-1);

	//kenv.createObject<CTextureDictionary>(-1);
	strRoot->texDictionary = kenv.createAndInitObject<CTextureDictionary>(-1);
	sgroot->insertChild(strRoot);

	CKSoundManager *sndmgr = kenv.createObject<CKSoundManager>(-1);
	CKSoundDictionaryID *smSndDict = kenv.createAndInitObject<CKSoundDictionaryID>();
	sndmgr->ksndmgrSndDictID = smSndDict;
}

#ifdef XEC_RELEASE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main()
#endif
{
	// Load INI file
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");
	std::string outGamePath = config.Get("XXL-Editor", "outgamepath", gamePath);
	int gameVersion = config.GetInteger("XXL-Editor", "version", 1);
	std::string cfgPlatformName = config.GetString("XXL-Editor", "platform", "KWN");
	int gamePlatform = KEnvironment::PLATFORM_PC;
	int i = 0;
	for (const char *ext : KEnvironment::platformExt) {
		if (_stricmp(cfgPlatformName.c_str(), ext) == 0) {
			gamePlatform = i; break;
		}
		i++;
	}
	bool isRemaster = config.GetBoolean("XXL-Editor", "remaster", false);

	// Initialize SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	g_window = new Window();

	// Create a Kal engine environment/simulation
	KEnvironment kenv;

	// Register factories to known classes
	if (gameVersion == KEnvironment::KVERSION_XXL1 && gamePlatform == KEnvironment::PLATFORM_PC) {
		// XXL1 PC Original+Romaster

		kenv.addFactory<CKServiceManager>();
		kenv.addFactory<CKGraphic>();
		kenv.addFactory<CKSoundManager>();

		kenv.addFactory<CKServiceLife>();
		kenv.addFactory<CKSrvCollision>();
		kenv.addFactory<CKSrvCamera>();
		kenv.addFactory<CKSrvCinematic>();
		kenv.addFactory<CKSrvEvent>();
		kenv.addFactory<CKSrvPathFinding>();
		kenv.addFactory<CKSrvDetector>();
		kenv.addFactory<CKSrvMarker>();
		kenv.addFactory<CKSrvAvoidance>();
		kenv.addFactory<CKSrvSekensor>();
		kenv.addFactory<CKSrvBeacon>();

		kenv.addFactory<CKHkPressionStone>();
		kenv.addFactory<CKHkAsterix>();
		kenv.addFactory<CKHkObelix>();
		kenv.addFactory<CKHkIdefix>();
		kenv.addFactory<CKHkMachinegun>();
		kenv.addFactory<CKHkTorch>();
		kenv.addFactory<CKHkHearth>();
		kenv.addFactory<CKHkDrawbridge>();
		kenv.addFactory<CKHkMegaAshtray>();
		kenv.addFactory<CKHkBoat>();
		kenv.addFactory<CKHkCorkscrew>();
		kenv.addFactory<CKHkTurnstile>();
		kenv.addFactory<CKHkLifter>();
		kenv.addFactory<CKHkActivator>();
		kenv.addFactory<CKHkRotaryBeam>();
		kenv.addFactory<CKHkLightPillar>();
		kenv.addFactory<CKHkWind>();
		kenv.addFactory<CKHkJumpingRoman>();
		kenv.addFactory<CKHkWaterJet>();
		kenv.addFactory<CKHkPowderKeg>();
		kenv.addFactory<CKHkTriangularTurtle>();
		kenv.addFactory<CKHkBasicEnemy>();
		kenv.addFactory<CKHkRomanArcher>();
		kenv.addFactory<CKHkAnimatedCharacter>();
		kenv.addFactory<CKHkSwingDoor>();
		kenv.addFactory<CKHkSlideDoor>();
		kenv.addFactory<CKHkCrumblyZone>();
		kenv.addFactory<CKHkHelmetCage>();
		kenv.addFactory<CKHkSquareTurtle>();
		kenv.addFactory<CKHkTeleBridge>();
		kenv.addFactory<CKHkCrate>();
		kenv.addFactory<CKHkBasicBonus>();
		kenv.addFactory<CKHkDonutTurtle>();
		kenv.addFactory<CKHkPyramidalTurtle>();
		kenv.addFactory<CKHkRollingStone>();
		kenv.addFactory<CKHkInterfaceBase>();
		kenv.addFactory<CKHkInterfaceEvolution>();
		kenv.addFactory<CKHkCatapult>();
		kenv.addFactory<CKHkInterfacePause>();
		kenv.addFactory<CKHkInterfaceInGame>();
		kenv.addFactory<CKHkInterfaceOption>();
		kenv.addFactory<CKHkInterfaceMain>();
		kenv.addFactory<CKHkInterfaceLoadSave>();
		kenv.addFactory<CKHkInterfaceCloth>();
		kenv.addFactory<CKHkInterfaceShop>();
		kenv.addFactory<CKHkPushPullAsterix>();
		kenv.addFactory<CKHkBasicEnemyLeader>();
		kenv.addFactory<CKHkTelepher>();
		kenv.addFactory<CKHkTowedTelepher>();
		kenv.addFactory<CKHkBumper>();
		kenv.addFactory<CKHkClueMan>();
		kenv.addFactory<CKHkSky>();
		kenv.addFactory<CKHkRocketRoman>();
		kenv.addFactory<CKHkJetPackRoman>();
		kenv.addFactory<CKHkWildBoar>();
		kenv.addFactory<CKHkAsterixShop>();
		kenv.addFactory<CKHkWater>();
		kenv.addFactory<CKHkMobileTower>();
		kenv.addFactory<CKHkBoss>();
		kenv.addFactory<CKHkInterfaceDemo>();
		kenv.addFactory<CKHkWaterFx>();
		kenv.addFactory<CKHkHighGrass>();
		kenv.addFactory<CKHkWaterFall>();
		kenv.addFactory<CKHkInterfaceGallery>();
		kenv.addFactory<CKHkTrioCatapult>();
		kenv.addFactory<CKHkObelixCatapult>();
		kenv.addFactory<CKHkInterfaceOpening>();
		kenv.addFactory<CKHkAsterixCheckpoint>();
		kenv.addFactory<CKHkBonusSpitter>();
		kenv.addFactory<CKHkLight>();
		kenv.addFactory<CKHkParkourSteleAsterix>();

		kenv.addFactory<CKHkAsterixLife>();
		kenv.addFactory<CKHkBoatLife>();
		kenv.addFactory<CKHkObelixLife>();
		kenv.addFactory<CKHkMecaLife>();
		kenv.addFactory<CKHkIdefixLife>();
		kenv.addFactory<CKHkEnemyLife>();
		kenv.addFactory<CKHkTriangularTurtleLife>();
		kenv.addFactory<CKHkAnimatedCharacterLife>();
		kenv.addFactory<CKHkSquareTurtleLife>();
		kenv.addFactory<CKHkDonutTurtleLife>();
		kenv.addFactory<CKHkPyramidalTurtleLife>();
		kenv.addFactory<CKHkCatapultLife>();
		kenv.addFactory<CKHkSkyLife>();
		kenv.addFactory<CKHkWaterLife>();
		kenv.addFactory<CKHkBossLife>();
		kenv.addFactory<CKHkWaterFxLife>();
		kenv.addFactory<CKHkAsterixCheckpointLife>();
		kenv.addFactory<CKHkWaterFallLife>();

		kenv.addFactory<CKGroupRoot>();
		kenv.addFactory<CKGrpMeca>();
		kenv.addFactory<CKGrpTrio>();
		kenv.addFactory<CKGrpBoat>();
		kenv.addFactory<CKGrpSquadEnemy>();
		kenv.addFactory<CKGrpEnemy>();
		kenv.addFactory<CKGrpPoolSquad>();
		kenv.addFactory<CKGrpWalkingCharacter>();
		kenv.addFactory<CKGrpBonus>();
		kenv.addFactory<CKGrpFrontEnd>();
		kenv.addFactory<CKGrpCatapult>();
		kenv.addFactory<CKGrpMap>();
		kenv.addFactory<CKGrpStorageStd>();
		kenv.addFactory<CKGrpCrate>();
		kenv.addFactory<CKGrpBonusPool>();
		kenv.addFactory<CKGrpAsterixBonusPool>();
		kenv.addFactory<CKGrpSquadJetPack>();
		kenv.addFactory<CKGrpWildBoarPool>();
		kenv.addFactory<CKGrpAsterixCheckpoint>();
		kenv.addFactory<CKGrpBonusSpitter>();
		kenv.addFactory<CKGrpLight>();

		kenv.addFactory<CKGrpTrioLife>();
		kenv.addFactory<CKGrpMecaLife>();
		kenv.addFactory<CKGrpBonusLife>();
		kenv.addFactory<CKGrpMapLife>();
		kenv.addFactory<CKGrpEnemyLife>();
		kenv.addFactory<CKGrpAsterixCheckpointLife>();

		kenv.addFactory<CKGrpMecaCpntAsterix>();
		kenv.addFactory<CKCrateCpnt>();
		kenv.addFactory<CKBasicEnemyCpnt>();
		kenv.addFactory<CKBasicEnemyLeaderCpnt>();
		kenv.addFactory<CKJumpingRomanCpnt>();
		kenv.addFactory<CKRomanArcherCpnt>();
		kenv.addFactory<CKRocketRomanCpnt>();
		kenv.addFactory<CKJetPackRomanCpnt>();
		kenv.addFactory<CKMobileTowerCpnt>();
		kenv.addFactory<CKTriangularTurtleCpnt>();
		kenv.addFactory<CKSquareTurtleCpnt>();
		kenv.addFactory<CKDonutTurtleCpnt>();
		kenv.addFactory<CKPyramidalTurtleCpnt>();

		kenv.addFactory<CKCamera>();
		kenv.addFactory<CKCameraRigidTrack>();
		kenv.addFactory<CKCameraClassicTrack>();
		kenv.addFactory<CKCameraPathTrack>();
		kenv.addFactory<CKCameraFixTrack>();
		kenv.addFactory<CKCameraAxisTrack>();
		kenv.addFactory<CKCameraSpyTrack>();
		kenv.addFactory<CKCameraPassivePathTrack>();

		kenv.addFactory<CKLogicalAnd>();
		kenv.addFactory<CKLogicalOr>();
		kenv.addFactory<CKPlayAnimCinematicBloc>();
		kenv.addFactory<CKPathFindingCinematicBloc>();
		kenv.addFactory<CKFlaggedPathCinematicBloc>();
		kenv.addFactory<CKGroupBlocCinematicBloc>();
		kenv.addFactory<CKAttachObjectsCinematicBloc>();
		kenv.addFactory<CKStreamCinematicBloc>();
		kenv.addFactory<CKRandLogicalDoor>();
		kenv.addFactory<CKParticleCinematicBloc>();
		kenv.addFactory<CKStreamAloneCinematicBloc>();
		kenv.addFactory<CKStreamGroupBlocCinematicBloc>();
		kenv.addFactory<CKManageEventCinematicBloc>();
		kenv.addFactory<CKManagerEventStopCinematicBloc>();
		kenv.addFactory<CKStartDoor>();
		kenv.addFactory<CKSekensorCinematicBloc>();
		kenv.addFactory<CKDisplayPictureCinematicBloc>();
		kenv.addFactory<CKManageCameraCinematicBloc>();
		kenv.addFactory<CKStartEventCinematicBloc>();
		kenv.addFactory<CKSkyCinematicBloc>();
		kenv.addFactory<CKLightningCinematicBloc>();
		kenv.addFactory<CKPlaySoundCinematicBloc>();
		kenv.addFactory<CKRomaOnly1CinematicBloc>();
		kenv.addFactory<CKRomaOnly2CinematicBloc>();
		kenv.addFactory<CKLogicalRomaOnly>();

		kenv.addFactory<CTextureDictionary>();
		kenv.addFactory<CAnimationDictionary>();
		kenv.addFactory<CKSoundDictionary>();
		kenv.addFactory<CKSoundDictionaryID>();

		kenv.addFactory<CKParticleGeometry>();
		kenv.addFactory<CKGeometry>();
		kenv.addFactory<CKSkinGeometry>();

		kenv.addFactory<CSGRootNode>();
		kenv.addFactory<CSGSectorRoot>();
		kenv.addFactory<CNode>();
		kenv.addFactory<CKDynBSphereProjectile>();
		kenv.addFactory<CSGBranch>();
		kenv.addFactory<CGlowNodeFx>();
		kenv.addFactory<CClone>();
		kenv.addFactory<CKBoundingSphere>();
		kenv.addFactory<CKDynamicBoundingSphere>();
		kenv.addFactory<CKAABB>();
		kenv.addFactory<CKOBB>();
		kenv.addFactory<CParticlesNodeFx>();
		kenv.addFactory<CAnimatedNode>();
		kenv.addFactory<CAnimatedClone>();
		kenv.addFactory<CKAACylinder>();
		kenv.addFactory<CSkyNodeFx>();
		kenv.addFactory<CFogBoxNodeFx>();
		kenv.addFactory<CTrailNodeFx>();

		kenv.addFactory<CKPFGraphTransition>();
		kenv.addFactory<CKBundle>();
		kenv.addFactory<CKSector>();
		kenv.addFactory<CKLevel>();
		kenv.addFactory<CKCoreManager>();
		kenv.addFactory<CKChoreoKey>();
		kenv.addFactory<CKPFGraphNode>();
		kenv.addFactory<CKSas>();
		kenv.addFactory<CGround>();
		kenv.addFactory<CDynamicGround>();
		kenv.addFactory<CKFlaggedPath>();
		kenv.addFactory<CKMsgAction>();
		kenv.addFactory<CKChoreography>();
		kenv.addFactory<CKLine>();
		kenv.addFactory<CKSpline4L>();
		kenv.addFactory<CKCinematicScene>();
		kenv.addFactory<CKCinematicSceneData>();
		kenv.addFactory<CKDefaultGameManager>();
		kenv.addFactory<CKAsterixGameManager>();
		kenv.addFactory<CKSekens>();
		kenv.addFactory<CKMeshKluster>();
		kenv.addFactory<CKBeaconKluster>();

		kenv.addFactory<CCloneManager>();
		kenv.addFactory<CManager2d>();
		kenv.addFactory<CMenuManager>();
		kenv.addFactory<CContainer2d>();
		kenv.addFactory<CScene2d>();
		kenv.addFactory<CMessageBox2d>();
		kenv.addFactory<CText2d>();
		kenv.addFactory<CColorTextButton2d>();
		kenv.addFactory<CBillboard2d>();

	}
	else if (gameVersion <= KEnvironment::KVERSION_XXL1) {
		// XXL1 GC/PS2

		kenv.addFactory<CKServiceManager>();

		kenv.addFactory<CKSrvCollision>();
		kenv.addFactory<CKSrvCinematic>();
		//kenv.addFactory<CKSrvEvent>();
		kenv.addFactory<CKSrvPathFinding>();
		kenv.addFactory<CKSrvDetector>();
		kenv.addFactory<CKSrvMarker>();
		kenv.addFactory<CKSrvBeacon>();

		kenv.addFactory<CKHkPressionStone>();
		kenv.addFactory<CKHkAsterix>();
		kenv.addFactory<CKHkObelix>();
		kenv.addFactory<CKHkIdefix>();
		kenv.addFactory<CKHkMachinegun>();
		kenv.addFactory<CKHkTorch>();
		kenv.addFactory<CKHkHearth>();
		kenv.addFactory<CKHkDrawbridge>();
		kenv.addFactory<CKHkMegaAshtray>();
		kenv.addFactory<CKHkBoat>();
		kenv.addFactory<CKHkCorkscrew>();
		kenv.addFactory<CKHkTurnstile>();
		kenv.addFactory<CKHkLifter>();
		kenv.addFactory<CKHkActivator>();
		kenv.addFactory<CKHkRotaryBeam>();
		kenv.addFactory<CKHkLightPillar>();
		kenv.addFactory<CKHkWind>();
		kenv.addFactory<CKHkJumpingRoman>();
		kenv.addFactory<CKHkWaterJet>();
		kenv.addFactory<CKHkPowderKeg>();
		kenv.addFactory<CKHkTriangularTurtle>();
		kenv.addFactory<CKHkBasicEnemy>();
		kenv.addFactory<CKHkRomanArcher>();
		kenv.addFactory<CKHkAnimatedCharacter>();
		kenv.addFactory<CKHkSwingDoor>();
		kenv.addFactory<CKHkSlideDoor>();
		kenv.addFactory<CKHkCrumblyZone>();
		kenv.addFactory<CKHkHelmetCage>();
		kenv.addFactory<CKHkSquareTurtle>();
		kenv.addFactory<CKHkTeleBridge>();
		kenv.addFactory<CKHkCrate>();
		kenv.addFactory<CKHkBasicBonus>();
		kenv.addFactory<CKHkDonutTurtle>();
		kenv.addFactory<CKHkPyramidalTurtle>();
		kenv.addFactory<CKHkRollingStone>();
		kenv.addFactory<CKHkInterfaceBase>();
		kenv.addFactory<CKHkInterfaceEvolution>();
		kenv.addFactory<CKHkCatapult>();
		kenv.addFactory<CKHkInterfacePause>();
		kenv.addFactory<CKHkInterfaceInGame>();
		kenv.addFactory<CKHkInterfaceOption>();
		kenv.addFactory<CKHkInterfaceMain>();
		kenv.addFactory<CKHkInterfaceLoadSave>();
		kenv.addFactory<CKHkInterfaceCloth>();
		kenv.addFactory<CKHkInterfaceShop>();
		kenv.addFactory<CKHkPushPullAsterix>();
		kenv.addFactory<CKHkBasicEnemyLeader>();
		kenv.addFactory<CKHkTelepher>();
		kenv.addFactory<CKHkTowedTelepher>();
		kenv.addFactory<CKHkBumper>();
		kenv.addFactory<CKHkClueMan>();
		kenv.addFactory<CKHkSky>();
		kenv.addFactory<CKHkRocketRoman>();
		kenv.addFactory<CKHkJetPackRoman>();
		kenv.addFactory<CKHkWildBoar>();
		kenv.addFactory<CKHkAsterixShop>();
		kenv.addFactory<CKHkWater>();
		kenv.addFactory<CKHkMobileTower>();
		kenv.addFactory<CKHkBoss>();
		kenv.addFactory<CKHkInterfaceDemo>();
		kenv.addFactory<CKHkWaterFx>();
		kenv.addFactory<CKHkHighGrass>();
		kenv.addFactory<CKHkWaterFall>();
		kenv.addFactory<CKHkInterfaceGallery>();
		kenv.addFactory<CKHkTrioCatapult>();
		kenv.addFactory<CKHkObelixCatapult>();
		kenv.addFactory<CKHkInterfaceOpening>();
		kenv.addFactory<CKHkAsterixCheckpoint>();
		kenv.addFactory<CKHkBonusSpitter>();
		kenv.addFactory<CKHkLight>();

		kenv.addFactory<CKHkAsterixLife>();
		kenv.addFactory<CKHkBoatLife>();
		kenv.addFactory<CKHkObelixLife>();
		kenv.addFactory<CKHkMecaLife>();
		kenv.addFactory<CKHkIdefixLife>();
		kenv.addFactory<CKHkEnemyLife>();
		kenv.addFactory<CKHkTriangularTurtleLife>();
		kenv.addFactory<CKHkAnimatedCharacterLife>();
		kenv.addFactory<CKHkSquareTurtleLife>();
		kenv.addFactory<CKHkDonutTurtleLife>();
		kenv.addFactory<CKHkPyramidalTurtleLife>();
		kenv.addFactory<CKHkCatapultLife>();
		kenv.addFactory<CKHkSkyLife>();
		kenv.addFactory<CKHkWaterLife>();
		kenv.addFactory<CKHkBossLife>();
		kenv.addFactory<CKHkWaterFxLife>();
		kenv.addFactory<CKHkAsterixCheckpointLife>();
		kenv.addFactory<CKHkWaterFallLife>();

		kenv.addFactory<CKGroupRoot>();
		kenv.addFactory<CKGrpMeca>();
		kenv.addFactory<CKGrpTrio>();
		kenv.addFactory<CKGrpBoat>();
		kenv.addFactory<CKGrpSquadEnemy>();
		kenv.addFactory<CKGrpEnemy>();
		kenv.addFactory<CKGrpPoolSquad>();
		kenv.addFactory<CKGrpWalkingCharacter>();
		kenv.addFactory<CKGrpBonus>();
		kenv.addFactory<CKGrpFrontEnd>();
		kenv.addFactory<CKGrpCatapult>();
		kenv.addFactory<CKGrpMap>();
		kenv.addFactory<CKGrpStorageStd>();
		kenv.addFactory<CKGrpCrate>();
		kenv.addFactory<CKGrpBonusPool>();
		kenv.addFactory<CKGrpAsterixBonusPool>();
		kenv.addFactory<CKGrpSquadJetPack>();
		kenv.addFactory<CKGrpWildBoarPool>();
		kenv.addFactory<CKGrpAsterixCheckpoint>();
		kenv.addFactory<CKGrpBonusSpitter>();
		kenv.addFactory<CKGrpLight>();

		kenv.addFactory<CKGrpTrioLife>();
		kenv.addFactory<CKGrpMecaLife>();
		kenv.addFactory<CKGrpBonusLife>();
		kenv.addFactory<CKGrpMapLife>();
		kenv.addFactory<CKGrpEnemyLife>();
		kenv.addFactory<CKGrpAsterixCheckpointLife>();

		kenv.addFactory<CKCrateCpnt>();
		kenv.addFactory<CKBasicEnemyCpnt>();
		kenv.addFactory<CKBasicEnemyLeaderCpnt>();
		kenv.addFactory<CKJumpingRomanCpnt>();
		kenv.addFactory<CKRomanArcherCpnt>();
		kenv.addFactory<CKRocketRomanCpnt>();
		kenv.addFactory<CKJetPackRomanCpnt>();
		kenv.addFactory<CKMobileTowerCpnt>();
		kenv.addFactory<CKTriangularTurtleCpnt>();
		kenv.addFactory<CKSquareTurtleCpnt>();
		kenv.addFactory<CKDonutTurtleCpnt>();
		kenv.addFactory<CKPyramidalTurtleCpnt>();

		//kenv.addFactory<CKCinematicBloc>();
		//kenv.addFactory<CKCinematicDoor>();
		kenv.addFactory<CKLogicalAnd>();
		kenv.addFactory<CKLogicalOr>();
		kenv.addFactory<CKPlayAnimCinematicBloc>();
		kenv.addFactory<CKPathFindingCinematicBloc>();
		kenv.addFactory<CKFlaggedPathCinematicBloc>();
		kenv.addFactory<CKGroupBlocCinematicBloc>();
		kenv.addFactory<CKAttachObjectsCinematicBloc>();
		kenv.addFactory<CKStreamCinematicBloc>();
		kenv.addFactory<CKRandLogicalDoor>();
		kenv.addFactory<CKParticleCinematicBloc>();
		kenv.addFactory<CKStreamAloneCinematicBloc>();
		kenv.addFactory<CKStreamGroupBlocCinematicBloc>();
		kenv.addFactory<CKManageEventCinematicBloc>();
		kenv.addFactory<CKManagerEventStopCinematicBloc>();
		kenv.addFactory<CKStartDoor>();
		kenv.addFactory<CKSekensorCinematicBloc>();
		kenv.addFactory<CKDisplayPictureCinematicBloc>();
		kenv.addFactory<CKManageCameraCinematicBloc>();
		kenv.addFactory<CKStartEventCinematicBloc>();
		kenv.addFactory<CKSkyCinematicBloc>();
		kenv.addFactory<CKLightningCinematicBloc>();
		kenv.addFactory<CKPlaySoundCinematicBloc>();

		kenv.addFactory<CAnimationDictionary>();
		kenv.addFactory<CKSoundDictionaryID>();

		kenv.addFactory<CKPFGraphTransition>();
		kenv.addFactory<CKBundle>();
		kenv.addFactory<CKSector>();
		kenv.addFactory<CKChoreoKey>();
		kenv.addFactory<CKPFGraphNode>();
		kenv.addFactory<CKSas>();
		kenv.addFactory<CGround>();
		kenv.addFactory<CDynamicGround>();
		kenv.addFactory<CKFlaggedPath>();
		kenv.addFactory<CKMsgAction>();
		kenv.addFactory<CKChoreography>();
		kenv.addFactory<CKLine>();
		kenv.addFactory<CKSpline4L>();
		kenv.addFactory<CKCinematicScene>();
		kenv.addFactory<CKCinematicSceneData>();
		kenv.addFactory<CKMeshKluster>();
		kenv.addFactory<CKBeaconKluster>();

	}
	else if (gamePlatform == KEnvironment::PLATFORM_PC && isRemaster) {
		// XXL2+ PC Remaster
		kenv.addFactory<CKSrvPathFinding>();
		kenv.addFactory<CKSrvBeacon>();
		//kenv.addFactory<CKSrvTrigger>();

		kenv.addFactory<CKHkBasicBonus>();

		kenv.addFactory<CKGrpA2BonusPool>();

		kenv.addFactory<CKCrateCpnt>();

		kenv.addFactory<CTextureDictionary>();
		//kenv.addFactory<CKSoundDictionary>();

		kenv.addFactory<CKParticleGeometry>();
		kenv.addFactory<CKGeometry>();
		kenv.addFactory<CKSkinGeometry>();

		kenv.addFactory<CSGRootNode>();
		kenv.addFactory<CSGSectorRoot>();
		kenv.addFactory<CNode>();
		kenv.addFactory<CKDynBSphereProjectile>();
		kenv.addFactory<CSGLeaf>();
		kenv.addFactory<CSGBranch>();
		kenv.addFactory<CGlowNodeFx>();
		kenv.addFactory<CClone>();
		kenv.addFactory<CKBoundingSphere>();
		kenv.addFactory<CKDynamicBoundingSphere>();
		kenv.addFactory<CKAABB>();
		kenv.addFactory<CKOBB>();
		kenv.addFactory<CParticlesNodeFx>();
		kenv.addFactory<CAnimatedNode>();
		kenv.addFactory<CAnimatedClone>();
		kenv.addFactory<CKAACylinder>();
		kenv.addFactory<CSkyNodeFx>();
		kenv.addFactory<CFogBoxNodeFx>();
		kenv.addFactory<CTrailNodeFx>();
		kenv.addFactory<CSGLight>();
		kenv.addFactory<CCloudsNodeFx>();
		kenv.addFactory<CZoneNode>();
		kenv.addFactory<CSpawnNode>();
		kenv.addFactory<CSpawnAnimatedNode>();

		kenv.addFactory<CSGAnchor>();
		kenv.addFactory<CSGBkgRootNode>();

		kenv.addFactory<CKPFGraphTransition>();
		kenv.addFactory<CKPFGraphNode>();
		kenv.addFactory<CGround>();
		kenv.addFactory<CDynamicGround>();
		kenv.addFactory<CKMeshKluster>();
		kenv.addFactory<CKBeaconKluster>();
		//kenv.addFactory<CKTrigger>();
		//kenv.addFactory<CKTriggerDomain>();
		//kenv.addFactory<CKA2GameState>();

		kenv.addFactory<CCloneManager>();
	}
	else if(gamePlatform == KEnvironment::PLATFORM_PC) {
		// XXL2+ PC

		kenv.addFactory<CKSrvPathFinding>();
		kenv.addFactory<CKSrvBeacon>();
		//kenv.addFactory<CKSrvTrigger>();

		kenv.addFactory<CKHkBasicBonus>();

		kenv.addFactory<CKGrpA2BonusPool>();

		kenv.addFactory<CKCrateCpnt>();

		kenv.addFactory<CTextureDictionary>();
		kenv.addFactory<CKSoundDictionary>();

		kenv.addFactory<CKParticleGeometry>();
		kenv.addFactory<CKGeometry>();
		kenv.addFactory<CKSkinGeometry>();

		kenv.addFactory<CSGRootNode>();
		kenv.addFactory<CSGSectorRoot>();
		kenv.addFactory<CNode>();
		kenv.addFactory<CKDynBSphereProjectile>();
		kenv.addFactory<CSGLeaf>();
		kenv.addFactory<CSGBranch>();
		kenv.addFactory<CGlowNodeFx>();
		kenv.addFactory<CClone>();
		kenv.addFactory<CKBoundingSphere>();
		kenv.addFactory<CKDynamicBoundingSphere>();
		kenv.addFactory<CKAABB>();
		kenv.addFactory<CKOBB>();
		kenv.addFactory<CParticlesNodeFx>();
		kenv.addFactory<CAnimatedNode>();
		kenv.addFactory<CAnimatedClone>();
		kenv.addFactory<CKAACylinder>();
		kenv.addFactory<CSkyNodeFx>();
		kenv.addFactory<CFogBoxNodeFx>();
		kenv.addFactory<CTrailNodeFx>();
		kenv.addFactory<CSGLight>();
		kenv.addFactory<CCloudsNodeFx>();
		kenv.addFactory<CZoneNode>();
		kenv.addFactory<CSpawnNode>();
		kenv.addFactory<CSpawnAnimatedNode>();

		kenv.addFactory<CSGAnchor>();
		kenv.addFactory<CSGBkgRootNode>();

		kenv.addFactory<CKPFGraphTransition>();
		kenv.addFactory<CKPFGraphNode>();
		kenv.addFactory<CGround>();
		kenv.addFactory<CDynamicGround>();
		kenv.addFactory<CKMeshKluster>();
		kenv.addFactory<CKBeaconKluster>();
		//kenv.addFactory<CKTrigger>();
		//kenv.addFactory<CKTriggerDomain>();
		kenv.addFactory<CKA2GameState>();
		kenv.addFactory<CKA3GameState>();

		kenv.addFactory<CCloneManager>();
	}
	else {
		// XXL2+ console

		kenv.addFactory<CKSrvPathFinding>();
		kenv.addFactory<CKSrvBeacon>();
		//kenv.addFactory<CKSrvTrigger>();

		//kenv.addFactory<CKHkBasicBonus>();

		//kenv.addFactory<CKGrpA2BonusPool>();

		//kenv.addFactory<CKCrateCpnt>();

		//kenv.addFactory<CTextureDictionary>();
		//kenv.addFactory<CKSoundDictionary>();

		//kenv.addFactory<CKParticleGeometry>();
		//kenv.addFactory<CKGeometry>();
		//kenv.addFactory<CKSkinGeometry>();

		//kenv.addFactory<CSGRootNode>();
		//kenv.addFactory<CSGSectorRoot>();
		//kenv.addFactory<CNode>();
		//kenv.addFactory<CKDynBSphereProjectile>();
		//kenv.addFactory<CSGLeaf>();
		//kenv.addFactory<CSGBranch>();
		//kenv.addFactory<CGlowNodeFx>();
		//kenv.addFactory<CClone>();
		//kenv.addFactory<CKBoundingSphere>();
		//kenv.addFactory<CKDynamicBoundingSphere>();
		//kenv.addFactory<CKAABB>();
		//kenv.addFactory<CKOBB>();
		//kenv.addFactory<CParticlesNodeFx>();
		//kenv.addFactory<CAnimatedNode>();
		//kenv.addFactory<CAnimatedClone>();
		//kenv.addFactory<CKAACylinder>();
		//kenv.addFactory<CSkyNodeFx>();
		//kenv.addFactory<CFogBoxNodeFx>();
		//kenv.addFactory<CTrailNodeFx>();
		//kenv.addFactory<CSGLight>();
		//kenv.addFactory<CCloudsNodeFx>();
		//kenv.addFactory<CZoneNode>();
		//kenv.addFactory<CSpawnNode>();
		//kenv.addFactory<CSpawnAnimatedNode>();

		//kenv.addFactory<CSGAnchor>();
		//kenv.addFactory<CSGBkgRootNode>();

		kenv.addFactory<CKPFGraphTransition>();
		kenv.addFactory<CKPFGraphNode>();
		kenv.addFactory<CGround>();
		kenv.addFactory<CDynamicGround>();
		kenv.addFactory<CKMeshKluster>();
		kenv.addFactory<CKBeaconKluster>();
		//kenv.addFactory<CKTrigger>();
		//kenv.addFactory<CKTriggerDomain>();
		kenv.addFactory<CKA2GameState>();
		kenv.addFactory<CKA3GameState>();

		//kenv.addFactory<CCloneManager>();
	}

	// Verify if GAME.KWN is in the gamepath
	std::string testPath = gamePath + "/GAME." + KEnvironment::platformExt[gamePlatform];
	if (_access(testPath.c_str(), 0) == -1) {
		MessageBox((HWND)g_window->getNativeWindow(), "GAME.KWN file not found!\n"
			"Be sure you that you set in the file xec-settings.ini the path to where you installed the patched copy of Asterix XXL 1.", NULL, 16);
		return -1;
	}

	// Load the game
	kenv.loadGame(gamePath.c_str(), gameVersion, gamePlatform, isRemaster);
	kenv.outGamePath = outGamePath;

	// Load the level
	int initlevel = config.GetInteger("XXL-Editor", "initlevel", 8);
	if (initlevel == -1)
		MakeEmptyXXL1Level(kenv);
	else
		kenv.loadLevel(initlevel);

	// Initialize graphics renderer
	Renderer *gfx = CreateRendererD3D9(g_window);
	// Initialize Dear ImGui
	ImGuiImpl_Init(g_window);
	ImGuiImpl_CreateFontsTexture(gfx);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Initialize the editor user interface
	EditorInterface editUI(kenv, g_window, gfx, config);
	editUI.prepareLevelGfx();

	while (!g_window->quitted()) {
		// Get window input
		g_window->handle();

		// Input + ImGui handling
		ImGuiImpl_NewFrame(g_window);
		editUI.iter();

		// Rendering
		gfx->setSize(g_window->getWidth(), g_window->getHeight());
		gfx->beginFrame();
		uint32_t bgndColor = 0xFF4040FF;
		if (kenv.version == kenv.KVERSION_XXL1) {
			if (CKHkSkyLife* hkSkyLife = kenv.levelObjects.getFirst<CKHkSkyLife>()) {
				bgndColor = hkSkyLife->skyColor;
			}
		}
		gfx->clearFrame(true, true, bgndColor);
		editUI.render();
		ImGuiImpl_Render(gfx);
		gfx->endFrame();
	}

	return 0;
}
