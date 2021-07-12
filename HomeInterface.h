#pragma once

#include <string>
#include <vector>

struct Window;

struct HomeInterface {
public:
	bool goToEditor = false;
	bool quitApp = false;
	Window* window;

	bool projectChosen = false;
	std::string gamePath = ".";
	std::string outGamePath = ".";
	int gameVersion = 1;
	std::string cfgPlatformName = "KWN";
	int gamePlatform = 1;
	bool isRemaster = false;
	std::string gameModule;
	int initialLevel = 8;

	HomeInterface(Window* window) : window(window) { readProjectPaths(); }
	void iter();

private:
	std::vector<std::string> projectPaths;

	void readProjectPaths();
	void writeProjectPaths();
};