#pragma once

#include <cstdint>
#include <string>

struct GameLauncher {
	std::string modulePath, gamePath;
	int version;

	void *processHandle = nullptr;
	void *threadHandle = nullptr;
	void *windowHandle = nullptr;

	GameLauncher(const std::string& modulePath, const std::string& gamePath, int version) :
		modulePath(modulePath), gamePath(gamePath), version(version) {}

	bool openGame();
	void closeGame();
	bool isGameRunning();
	bool loadLevel(uint32_t lvlNum);
};