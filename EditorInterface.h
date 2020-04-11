#pragma once

#include <vector>
#include "rwrenderer.h"
#include "Camera.h"
#include "vecmat.h"
#include "window.h"

struct KEnvironment;
struct Renderer;
struct CKSceneNode;

struct EditorInterface {
	KEnvironment &kenv;
	Window *g_window;

	Renderer *gfx;
	ProTexDict protexdict;
	std::vector<ProTexDict> str_protexdicts;
	ProGeoCache progeocache;

	int selTexID = 0;
	RwGeometry *selGeometry = nullptr;
	Vector3 selgeoPos = Vector3(0, 0, 0);
	Camera camera;
	float _camspeed = 0.5f;

	int framesInSecond = 0;
	int lastFps = 0;
	uint32_t lastFpsTime;

	bool showTextures = true, showBeacons = true, showBeaconKlusterBounds = false, showSasBounds = true;
	bool showImGuiDemo = false;

	struct Selection {
		Vector3 hitPos;
		int type;
		void *obj;
		Selection() : type(0) {}
		Selection(Vector3 hitPos, int type, void *obj) : hitPos(hitPos), type(type), obj(obj) {}
	};
	int selectionType = 0;
	CKSceneNode *selNode = nullptr;
	void *selBeacon = nullptr;
	int numRayHits = 0;
	std::vector<Selection> rayHits;
	Selection nearestRayHit;

	EditorInterface(KEnvironment &kenv, Window *window, Renderer *gfx);

	void prepareLevelGfx();
	void iter();
	void render();

private:
	void IGEnumNode(CKSceneNode *node, const char *description = "");
	void IGSceneGraph();
	void IGSceneNodeProperties();
	void checkNodeRayCollision(CKSceneNode *node, const Vector3 &rayDir, const Matrix &matrix);
	void checkMouseRay();
};