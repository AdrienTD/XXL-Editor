#pragma once

#include <cstdint>
#include <string>

struct GameLauncher {
	std::string modulePath, gamePath;

	void *processHandle = nullptr;
	void *threadHandle = nullptr;
	void *windowHandle = nullptr;

	GameLauncher(std::string modulePath, std::string gamePath) : modulePath(modulePath), gamePath(gamePath) {}

	bool openGame();
	void closeGame();
	bool isGameRunning();
	bool loadLevel(uint32_t lvlNum);
};