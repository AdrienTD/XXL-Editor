#include "main.h"
#include <cassert>
#include "KEnvironment.h"
#include "CoreClasses/CKManager.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKCamera.h"
#include "CoreClasses/CKCinematicNode.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKGeometry.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"
#include "window.h"
#include "renderer.h"
#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "EditorInterface.h"
#include "ClassRegister.h"
#include "tests.h"
#include "HomeInterface.h"
#include "HexEditor.h"
#include "GuiUtils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <INIReader.h>
#include <filesystem>

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
			cl.instantiation = KInstantiation(definfo[i]);
	}
	kenv.levelObjects.getClassType<CKLevel>().instantiation = KInstantiation::Globally;
	kenv.levelObjects.getClassType<CKCoreManager>().instantiation = KInstantiation::Globally;
	kenv.levelObjects.getClassType<CKAsterixGameManager>().instantiation = KInstantiation::Globally;
	kenv.levelObjects.getClassType<CSGRootNode>().instantiation = KInstantiation::LevelUnique;
	kenv.levelObjects.getClassType<CManager2d>().instantiation = KInstantiation::Globally;
	kenv.levelObjects.getClassType<CMenuManager>().instantiation = KInstantiation::LevelUnique;
	kenv.levelObjects.getClassType<CScene2d>().instantiation = KInstantiation::LevelUnique;
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
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
#else
int wmain()
#endif
{
	// Get command line argument
	std::filesystem::path fileArg;
	if (__argc >= 2)
		fileArg = __wargv[1];

	// Load INI file
	INIReader config("xec-settings.ini");
#ifndef XEC_RELEASE
	bool testMode = config.GetBoolean("XXL-Editor", "test", false);
	if (testMode) {
		Tests::TestPrompt();
		return 0;
	}
#endif
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");
	std::string outGamePath = config.Get("XXL-Editor", "outgamepath", gamePath);
	int gameVersion = config.GetInteger("XXL-Editor", "version", 1);
	std::string cfgPlatformName = config.GetString("XXL-Editor", "platform", "KWN");
	int gamePlatform = KEnvironment::PLATFORM_PC;
	bool isRemaster = config.GetBoolean("XXL-Editor", "remaster", false);
	std::string gameModule = config.Get("XXL-Editor", "gamemodule", "./GameModule_MP_windowed.exe");
	int initlevel = config.GetInteger("XXL-Editor", "initlevel", 0);
	bool hexMode = false;

	// Initialize SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	Window window;
	Window* g_window = &window;

	// Initialize graphics renderer
	Renderer* gfx = CreateRendererD3D9(g_window);
	// Initialize Dear ImGui
	ImGuiImpl_Init(g_window);
	ImGuiImpl_CreateFontsTexture(gfx);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::GetStyle().WindowRounding = 7.0f;

	// Home screen
	if (!fileArg.empty()) {
		// Automatically open project if cmd line argument given
		HomeInterface home(g_window, gfx);
		home.openProject(fileArg.u8string());
		if (!home.projectChosen)
			return -2;
		gamePath = home.gamePath;
		outGamePath = home.outGamePath;
		gameVersion = home.gameVersion;
		cfgPlatformName = home.cfgPlatformName;
		isRemaster = home.isRemaster;
		gameModule = home.gameModule;
		initlevel = home.initialLevel;
	}
	else if (config.GetBoolean("XXL-Editor", "homeScreen", true)) {
		HomeInterface home(g_window, gfx);
		while (!g_window->quitted() && !home.goToEditor && !home.quitApp) {
			// Get window input
			g_window->handle();

			// Input + ImGui handling
			ImGuiImpl_NewFrame(g_window);
			home.iter();

			// Rendering
			gfx->setSize(g_window->getWidth(), g_window->getHeight());
			gfx->beginFrame();
			gfx->clearFrame(true, true, 0xFFFF8080);
			ImGuiImpl_Render(gfx);
			gfx->endFrame();
		}
		if (g_window->quitted() || home.quitApp)
			return 0;
		if (home.projectChosen) {
			gamePath = home.gamePath;
			outGamePath = home.outGamePath;
			gameVersion = home.gameVersion;
			cfgPlatformName = home.cfgPlatformName;
			isRemaster = home.isRemaster;
			gameModule = home.gameModule;
			initlevel = home.initialLevel;
			hexMode = home.hexMode;
		}
	}

	// ----- LOAD GAME AND START EDITOR -----

	// Find platform
	int i = 0;
	for (const char* ext : KEnvironment::platformExt) {
		if (_stricmp(cfgPlatformName.c_str(), ext) == 0) {
			gamePlatform = i; break;
		}
		i++;
	}

	// Convert paths from UTF8
	namespace fs = std::filesystem;
	auto fsInputPath = fs::u8path(gamePath);
	auto fsOutputPath = fs::u8path(outGamePath);

	// Verify if GAME.K** is in the gamepath
	std::string testFile = std::string("GAME.") + KEnvironment::platformExt[gamePlatform];
	if (!fs::is_regular_file(fsInputPath / testFile)) {
		MsgBox_Ok(g_window, (testFile + " file not found!\n"
			"Be sure that the path to the game's folder is correctly set in the project file or the xec-settings.ini file.").c_str(), GuiUtils::MsgBoxIcon::Error);
		return -1;
	}

	// Enter hex editor mode if requested
	if (hexMode) {
		HexEditorUI(gamePath, outGamePath, gameVersion, gamePlatform, isRemaster, initlevel, *g_window, gfx);
		return 0;
	}

	// Create a Kal engine environment/simulation
	KEnvironment kenv;

	// Register factories to known classes
	ClassRegister::registerClasses(kenv, gameVersion, gamePlatform, isRemaster);

	// Load the game
	kenv.loadGame(fsInputPath.u8string().c_str(), gameVersion, gamePlatform, isRemaster);
	kenv.outGamePath = fsOutputPath.u8string();

	// Load the level
	if (initlevel == -1)
		MakeEmptyXXL1Level(kenv);
	else
		kenv.loadLevel(initlevel);

	// Initialize the editor user interface
	EditorUI::EditorInterface editUI(kenv, g_window, gfx, gameModule);
	editUI.prepareLevelGfx();
	editUI.levelNum = initlevel;

	while (!g_window->quitted()) {
		// Get window input
		g_window->handle();

		// Input + ImGui handling
		ImGuiImpl_NewFrame(g_window);
		editUI.iter();

		// Rendering
		gfx->setSize(g_window->getWidth(), g_window->getHeight());
		gfx->beginFrame();
		uint32_t bgndColor = 0xFFFF4040;
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
