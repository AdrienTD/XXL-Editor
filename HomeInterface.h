#pragma once

#include <string>
#include <vector>

struct Window;
struct Renderer;

struct HomeInterface {
public:
	bool goToEditor = false;
	bool quitApp = false;
	Window* window;
	Renderer* gfx;

	bool projectChosen = false;
	std::string gamePath = ".";
	std::string outGamePath = ".";
	int gameVersion = 1;
	std::string cfgPlatformName = "KWN";
	int gamePlatform = 1;
	bool isRemaster = false;
	std::string gameModule;
	int initialLevel = 8;

	void* logoTexture = nullptr, *helpTexture = nullptr;
	int logoWidth, logoHeight;

	HomeInterface(Window* window, Renderer* gfx);
	void iter();
	void openProject(const std::string& u8filename);

private:
	std::vector<std::string> projectPaths;

	void readProjectPaths();
	void writeProjectPaths();

	void HelpMarker(const char* message, bool padding = false);
};