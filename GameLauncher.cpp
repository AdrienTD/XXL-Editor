#include "GameLauncher.h"
#include <Windows.h>
#include <cstdio>

void GameLauncher::openGame()
{
	STARTUPINFOA startInfo;
	PROCESS_INFORMATION procInfo;
	ZeroMemory(&startInfo, sizeof(startInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));
	startInfo.cb = sizeof(startInfo);
	if (!CreateProcessA(modulePath.c_str(),
		NULL, NULL, NULL, FALSE, 0, NULL, gamePath.c_str(), &startInfo, &procInfo)) {
		MessageBox(NULL, "Failed to create game process!\n", NULL, 16);
		return;
	}
	this->processHandle = procInfo.hProcess;
	this->threadHandle = procInfo.hThread;
}

void GameLauncher::closeGame()
{
	if (processHandle) {
		CloseHandle(processHandle);
		CloseHandle(threadHandle);
		processHandle = nullptr;
		threadHandle = nullptr;
	}
}

bool GameLauncher::isGameRunning()
{
	DWORD exitCode;
	if(processHandle)
		if (GetExitCodeProcess(processHandle, &exitCode))
			if (exitCode == STILL_ACTIVE)
				return true;
	return false;
}

static BOOL CALLBACK EnumCallback(HWND hWnd, LPARAM lParam)
{
	GameLauncher *gl = (GameLauncher*)lParam;
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	if (pid == GetProcessId(gl->processHandle)) {
		char wndName[32];
		if (GetClassNameA(hWnd, wndName, sizeof(wndName))) {
			if (!strcmp(wndName, "Asterix & Obelix XXL")) {
				gl->windowHandle = hWnd;
				return FALSE;
			}
		}
	}
	return TRUE;
}

void GameLauncher::loadLevel(uint32_t lvlNum)
{
	if (!isGameRunning()) {
		closeGame();
		openGame();
	}
	uint32_t adYellowPages = 0, adGameManager;
	while (!adYellowPages) {
		ReadProcessMemory(processHandle, (void*)0x6621F4, &adYellowPages, 4, NULL);
		Sleep(250);
	}
	Sleep(1000);
	printf("%08X ", adYellowPages);
	ReadProcessMemory(processHandle, (void*)(adYellowPages + 0x8c), &adGameManager, 4, NULL);
	printf("%08X ", adGameManager);
	WriteProcessMemory(processHandle, (void*)(adGameManager + 8), &lvlNum, 4, NULL);

	// Bring window in front
	windowHandle = nullptr;
	EnumWindows(EnumCallback, (LPARAM)this);
	if (windowHandle) {
		BringWindowToTop((HWND)windowHandle);
	}
}
