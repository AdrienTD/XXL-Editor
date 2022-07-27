#include "GameLauncher.h"
#include <Windows.h>
#include <cstdio>
#include <filesystem>

bool GameLauncher::openGame()
{
	STARTUPINFOW startInfo;
	PROCESS_INFORMATION procInfo;
	ZeroMemory(&startInfo, sizeof(startInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));
	startInfo.cb = sizeof(startInfo);
	namespace fs = std::filesystem;
	if (!CreateProcessW(fs::u8path(modulePath).c_str(), NULL, NULL, NULL, FALSE, 0, NULL, fs::u8path(gamePath).c_str(), &startInfo, &procInfo)) {
		return false;
	}
	this->processHandle = procInfo.hProcess;
	this->threadHandle = procInfo.hThread;
	return true;
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
		//char wndName[32];
		//if (GetClassNameA(hWnd, wndName, sizeof(wndName))) {
		//	if (!strcmp(wndName, "Asterix & Obelix XXL")) {
				gl->windowHandle = hWnd;
				return FALSE;
		//	}
		//}
	}
	return TRUE;
}

bool GameLauncher::loadLevel(uint32_t lvlNum)
{
	static const uint32_t yellowPagePtr[6] = { 0, 0x6621F4, 0x663D04, 0, 0x765BF8, 0 };
	static const uint32_t ypGameMgrOffset[6] = { 0, 0x8c, 0x80, 0, 0x8c, 0 };
	static const uint32_t gmLevelOffset[6] = { 0, 8, 0x70, 0, 0x1A4, 0 };
	if (yellowPagePtr[version] == 0) return false;

	if (!isGameRunning()) {
		closeGame();
		bool b = openGame();
		if (!b) return false;
	}
	uint32_t adYellowPages = 0, adGameManager = 0;
	while (!adYellowPages) {
		BOOL b = ReadProcessMemory(processHandle, (void*)yellowPagePtr[version], &adYellowPages, 4, NULL);
		if (!b) return false;
		Sleep(250);
	}
	//if (firstTime) Sleep(1000);
	printf("%08X ", adYellowPages);
	while (!adGameManager) {
		BOOL b = ReadProcessMemory(processHandle, (void*)(adYellowPages + ypGameMgrOffset[version]), &adGameManager, 4, NULL);
		if (!b) return false;
		Sleep(250);
	}
	printf("%08X ", adGameManager);
	BOOL b = WriteProcessMemory(processHandle, (void*)(adGameManager + gmLevelOffset[version]), &lvlNum, 4, NULL);
	if (!b) return false;

	// Bring window in front
	windowHandle = nullptr;
	EnumWindows(EnumCallback, (LPARAM)this);
	if (windowHandle) {
		HWND wnd = (HWND)windowHandle;
		BringWindowToTop(wnd);
		OpenIcon(wnd);
	}
	return true;
}
