#pragma once

#include <vector>
#include "rwrenderer.h"
#include "Camera.h"
#include "vecmat.h"
#include "window.h"
#include "GroundRenderer.h"
#include "GameLauncher.h"
#include "KObject.h"

struct KEnvironment;
struct Renderer;
struct CKSceneNode;
class INIReader;
struct CKGroup;
struct RwClump;
struct CKGrpSquadEnemy;
struct CKEnemyCpnt;
struct CKPFGraphNode;
struct CKHook;
struct CSGBranch;

struct EditorInterface;
struct ImGuiMemberListener;

struct UISelection {
	EditorInterface &ui;
	Vector3 hitPosition;

	UISelection(EditorInterface &ui, const Vector3 &hitPos) : ui(ui), hitPosition(hitPos) {}
	virtual ~UISelection() = default;

	virtual int getTypeID() { return 0; }
	virtual bool hasTransform() { return false; }
	virtual Matrix getTransform() { return Matrix::getIdentity(); }
	virtual void setTransform(const Matrix &mat) {}

	template <class T> bool is() { return getTypeID() == T::ID; }
	template <class T> T* cast() { return (getTypeID() == T::ID) ? (T*)this : nullptr; }
};

struct EditorInterface {
	KEnvironment &kenv;
	Window *g_window;

	Renderer *gfx;
	ProTexDict protexdict;
	std::vector<ProTexDict> str_protexdicts;
	ProGeoCache progeocache;
	GroundModelCache gndmdlcache;

	bool wndShowMain = true, wndShowTextures = false, wndShowClones = false,
		wndShowSceneGraph = false, wndShowBeacons = false, wndShowGrounds = false,
		wndShowEvents = false, wndShowSounds = false, wndShowSquads = false,
		wndShowHooks = false, wndShowPathfinding = false, wndShowMarkers = false,
		wndShowDetectors = false, wndShowObjects = false, wndShowMisc = false,
		wndShowCinematic = false, wndShowLocale = false, wndShowTriggers = false;

	int selTexID = 0;
	RwGeometry *selGeometry = nullptr; int selGeoCloneIndex;
	std::vector<uint32_t> selClones;
	Vector3 selgeoPos = Vector3(0, 0, 0);
	Camera camera = Camera(Vector3(2.0f, 11.0f, -7.0f), Vector3(-0.785f, 3.141f, 0.0f));
	float _camspeed = 0.5f;

	int framesInSecond = 0;
	int lastFps = 0;
	uint32_t lastFpsTime;

	bool showTextures = true, showBeacons = true,
		showBeaconKlusterBounds = false, showSasBounds = true,
		showGroundBounds = false, showGrounds = false, showInfiniteWalls = false,
		showNodes = true, showInvisibleNodes = false, showClones = true,
		showLines = true, showSquadBoxes = false, showSquadChoreos = true,
		showPFGraph = false, showMarkers = true, showDetectors = true,
		showLights = false;
	bool showImGuiDemo = false;
	int showingChoreoKey = 0;

	CKSceneNode *selNode = nullptr;
	void *selBeacon = nullptr, *selBeaconKluster = nullptr;
	CGround *selGround = nullptr;
	CKGrpSquadEnemy *selectedSquad = nullptr;
	CKPFGraphNode *selectedPFGraphNode = nullptr;
	void *selectedMarker = nullptr;
	CKHook *selectedHook = nullptr;
	int selectedEventSequence = 0;

	int numRayHits = 0;
	std::vector<std::unique_ptr<UISelection>> rayHits;
	UISelection *nearestRayHit = nullptr;
	template <class T, class... Us> void select(Us ... args) {
		rayHits = {std::make_unique<T>(*this, args...)};
		nearestRayHit = &rayHits[0];
	}

	GameLauncher launcher;

	std::unique_ptr<RwClump> sphereModel, swordModel;

	std::map<CSGBranch*, int> nodeCloneIndexMap;
	std::set<std::vector<uint32_t>> cloneSet;

	EditorInterface(KEnvironment &kenv, Window *window, Renderer *gfx, INIReader &config);

	void prepareLevelGfx();
	void iter();
	void render();

	static void IGObjectSelector(KEnvironment &kenv, const char *name, kanyobjref &ptr, uint32_t clfid = 0xFFFFFFFF);
	static void IGObjectSelectorRef(KEnvironment &kenv, const char *name, kobjref<CKObject> &ref) { IGObjectSelector(kenv, name, ref, 0xFFFFFFFF); };
	template<class T> static void IGObjectSelectorRef(KEnvironment &kenv, const char *name, kobjref<T> &ref) { IGObjectSelector(kenv, name, ref, T::FULL_ID); };

private:
	void IGMain();
	void IGMiscTab();
	void IGObjectTree();
	void IGBeaconGraph();
	void IGGeometryViewer();
	void IGTextureEditor();
	void IGEnumNode(CKSceneNode *node, const char *description = "", bool isAnimBranch = false);
	void IGSceneGraph();
	void IGSceneNodeProperties();
	void IGGroundEditor();
	void IGEventEditor();
	void IGSoundEditor();
	void IGSquadEditor();
	void IGEnumGroup(CKGroup *group);
	void IGHookEditor();
	void IGCloneEditor();
	void IGComponentEditor(CKEnemyCpnt *cpnt);
	void IGPathfindingEditor();
	void IGMarkerEditor();
	void IGDetectorEditor();
	void IGCinematicEditor();
	void IGLocaleEditor();
	void IGTriggerEditor();
	void checkNodeRayCollision(CKSceneNode *node, const Vector3 &rayDir, const Matrix &matrix);
	void checkMouseRay();
};