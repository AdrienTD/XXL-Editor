#pragma once

#include <cstdint>

struct GameLauncher {
	void *processHandle = nullptr;
	void *threadHandle = nullptr;
	void *windowHandle = nullptr;

	void openGame();
	void closeGame();
	bool isGameRunning();
	void loadLevel(uint32_t lvlNum);
};