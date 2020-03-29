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

	int framesInSecond = 0;
	int lastFps = 0;
	uint32_t lastFpsTime;

	bool showTextures = true;
	bool showImGuiDemo = false;

	CKSceneNode *selNode = nullptr;

	EditorInterface(KEnvironment &kenv, Window *window, Renderer *gfx);

	void prepareLevelGfx();
	void iter();
	void render();

private:
	void IGEnumNode(CKSceneNode *node, const char *description = "");
	void IGSceneGraph();
	void IGSceneNodeProperties();
};