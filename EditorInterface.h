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
struct INIReader;
struct CKGroup;

struct EditorInterface {
	KEnvironment &kenv;
	Window *g_window;

	Renderer *gfx;
	ProTexDict protexdict;
	std::vector<ProTexDict> str_protexdicts;
	ProGeoCache progeocache;
	GroundModelCache gndmdlcache;

	int selTexID = 0;
	RwGeometry *selGeometry = nullptr;
	Vector3 selgeoPos = Vector3(0, 0, 0);
	Camera camera;
	float _camspeed = 0.5f;

	int framesInSecond = 0;
	int lastFps = 0;
	uint32_t lastFpsTime;

	bool showTextures = true, showBeacons = true,
		showBeaconKlusterBounds = false, showSasBounds = true,
		showGroundBounds = false, showGrounds = false, showInfiniteWalls = false,
		showNodes = true, showInvisibleNodes = false, showClones = true,
		showLines = true, showSquadBoxes = false, showSquadChoreos = true;
	bool showImGuiDemo = false;
	int showingChoreoKey = 0;

	struct Selection {
		Vector3 hitPos;
		int type;
		void *obj, *obj2;
		Selection() : type(0) {}
		Selection(Vector3 hitPos, int type, void *obj, void *obj2 = nullptr) : hitPos(hitPos), type(type), obj(obj), obj2(obj2) {}
	};
	int selectionType = 0;
	CKSceneNode *selNode = nullptr;
	void *selBeacon = nullptr, *selBeaconKluster = nullptr;
	CGround *selGround = nullptr;
	int numRayHits = 0;
	std::vector<Selection> rayHits;
	Selection nearestRayHit;

	GameLauncher launcher;

	EditorInterface(KEnvironment &kenv, Window *window, Renderer *gfx, INIReader &config);

	void prepareLevelGfx();
	void iter();
	void render();

private:
	void IGObjectSelector(const char *name, CKObject** ptr, uint32_t clfid = 0xFFFFFFFF);
	template<class T> void IGObjectSelectorRef(const char *name, kobjref<T> &ref) { IGObjectSelector(name, &ref._pointer, T::FULL_ID); };
	void IGMain();
	void IGMiscTab();
	void IGObjectTree();
	void IGBeaconGraph();
	void IGGeometryViewer();
	void IGTextureEditor();
	void IGEnumNode(CKSceneNode *node, const char *description = "");
	void IGSceneGraph();
	void IGSceneNodeProperties();
	void IGGroundEditor();
	void IGEventEditor();
	void IGSoundEditor();
	void IGSquadEditor();
	void IGEnumGroup(CKGroup *group);
	void IGHookEditor();
	void checkNodeRayCollision(CKSceneNode *node, const Vector3 &rayDir, const Matrix &matrix);
	void checkMouseRay();
};