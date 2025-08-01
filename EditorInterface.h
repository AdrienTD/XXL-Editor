#pragma once

#include <vector>
#include "rwrenderer.h"
#include "Camera.h"
#include "vecmat.h"
#include "window.h"
#include "GroundRenderer.h"
#include "GameLauncher.h"
#include "KObject.h"
#include "CKUtils.h"
#include "Encyclopedia.h"

struct KEnvironment;
struct Renderer;
struct CKSceneNode;
struct CKGroup;
struct RwClump;
struct CKGrpSquadEnemy;
struct CKEnemyCpnt;
struct CKPFGraphNode;
struct CKHook;
struct CSGBranch;
struct CKTrigger;
struct CKGrpSquadX2;
struct EventNode;
struct EventNodeX1;
struct EventNodeX2;
struct CKDetectorBase;
struct CAnyAnimatedNode;
struct LocaleEditor;
namespace GameX2 {
	struct CKGrpFightZone;
	struct IKGrpEnemy;
}

namespace EditorUI {

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
	virtual void duplicate() {}
	virtual bool remove() { return false; }
	virtual void onSelected() {}
	virtual std::string getInfo() { return "Unimplemented"; }
	virtual void onDetails() {}

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
		wndShowCinematic = false, wndShowLocale = false, wndShowTriggers = false,
		wndShowCollision = false, wndShowLines = false, wndShowLevel = false,
		wndShowAbout = false, wndShowCamera = false, wndShowCounters = false,
		wndShowMusic = false, wndShowSekens = false, wndShowObjInspector = false,
		wndShowAnimViewer = false;

	int selTexID = 0;
	RwGeometry *selGeometry = nullptr; int selGeoCloneIndex;
	std::vector<uint32_t> selClones;
	Vector3 selgeoPos = Vector3(0, 0, 0);
	Camera camera = Camera(Vector3(2.0f, 11.0f, -7.0f), Vector3(-0.785f, 3.141f, 0.0f));
	float _camspeed = 30.0f; // units per second
	int levelNum = 0;

	int framesInSecond = 0;
	int lastFps = 0;
	uint32_t lastFpsTime;

	bool showTextures = true, showBeacons = true,
		showBeaconKlusterBounds = false, showSasBounds = true,
		showGroundBounds = false, showGrounds = false, showInfiniteWalls = false,
		showNodes = true, showInvisibleNodes = false, showClones = true,
		showLines = true, showSquadBoxes = false, showSquadChoreos = true,
		showPFGraph = false, showMarkers = true, showDetectors = true,
		showLights = false, showMsgActionBoxes = false;
	bool showImGuiDemo = false;
	int showingChoreography = 0;
	int showingChoreoKey = 0;
	bool enableAlphaClip = true;

	int showingSector = 0;

	KWeakRef<CKSceneNode> selNode;
	int selBeaconSector = -1, selBeaconKluster, selBeaconBing, selBeaconIndex;
	KWeakRef<CGround> selGround;
	KWeakRef<CKGrpSquadEnemy> selectedSquad; KWeakRef<CKGrpSquadX2> selectedX2Squad;
	KWeakRef<GameX2::CKGrpFightZone> selectedX2FightZone; bool viewFightZoneInsteadOfSquad = false;
	KWeakRef<CKPFGraphNode> selectedPFGraphNode;
	int selectedMarkerIndex = -1;
	KWeakRef<CKHook> selectedHook; KWeakRef<CKGroup> selectedGroup; bool viewGroupInsteadOfHook = false;
	int selectedEventSequence = 0;
	KWeakRef<CKTrigger> selectedTrigger;
	int selectedShapeType = -1; size_t selectedShapeIndex = -1;
	KWeakRef<CKDetectorBase> selectedX2Detector;
	KWeakRef<CKObject> selectedInspectorObjectRef;

	KWeakRef<CAnyAnimatedNode> selectedAnimatedNode;
	int selectedAnimationIndex = -1, selectedAnimationSector = 0;
	Vector3 selectedAnimRenderPos;
	bool showStickman = false;

	int numRayHits = 0;
	std::vector<std::unique_ptr<UISelection>> rayHits;
	UISelection *nearestRayHit = nullptr;
	template <class T, class... Us> void select(Us ... args) {
		rayHits = {std::make_unique<T>(*this, args...)};
		nearestRayHit = &rayHits[0];
	}

	Vector3 cursorPosition;

	GameLauncher launcher;

	std::unique_ptr<RwClump> sphereModel, swordModel, spawnStarModel;

	std::map<CSGBranch*, int> nodeCloneIndexMap;
	std::set<std::vector<uint32_t>> cloneSet;

	int guizmoOperation = 0;

	Encyclopedia g_encyclo;
	std::unique_ptr<LocaleEditor> g_localeEditor;

	EditorInterface(KEnvironment &kenv, Window *window, Renderer *gfx, const std::string& gameModule);
	~EditorInterface();

	void prepareLevelGfx();
	void iter();
	void render();

	GameX2::IKGrpEnemy* getX2PlusEnemyGroup();

private:
	void IGMain();
	void IGLocaleEditor();
	void checkNodeRayCollision(CKSceneNode *node, const Vector3 &rayDir, const Matrix &matrix);
	void checkMouseRay();
};

}