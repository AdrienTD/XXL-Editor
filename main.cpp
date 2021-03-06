#include "main.h"
#include <cassert>
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
#include "window.h"
#include "renderer.h"
#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "EditorInterface.h"
#include "ClassRegister.h"
#include "tests.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <io.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <INIReader.h>

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
	Window* g_window = new Window();

	// Create a Kal engine environment/simulation
	KEnvironment kenv;

	// Register factories to known classes
	ClassRegister::registerClasses(kenv, gameVersion, gamePlatform, isRemaster);

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
