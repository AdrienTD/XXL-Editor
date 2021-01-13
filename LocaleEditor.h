#pragma once
#include <map>
#include <string>
#include <vector>
#include "KLocalObject.h"
#include "renderer.h"

struct KEnvironment;
struct Window;

struct LocaleEditor {
	// Instance of GLOC file loaded in memory
	struct LocalDocument {
		KLocalPack locpack;
		std::vector<texture_t> fontTextures;
		std::map<std::string, int> fntTexMap;
		std::vector<std::string> trcTextU8, stdTextU8;
		uint32_t langStrIndex, langID;
		std::map<int, KLocalPack> lvlLocpacks;
		std::map<int, std::vector<texture_t>> lvlTextures;
	};

	KEnvironment& kenv;
	Renderer* gfx;
	Window* window;
	std::vector<LocalDocument> documents;
	bool langLoaded = false;

	LocaleEditor(KEnvironment& kenv, Renderer *gfx, Window* window) : kenv(kenv), gfx(gfx), window(window) {}
	void gui();
};