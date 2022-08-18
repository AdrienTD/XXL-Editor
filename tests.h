#pragma once

#include <string>
struct Window;
struct Renderer;

namespace Tests {
	void TestPrompt();
	void HexEditor(const std::string& gamePath, const std::string& outGamePath, int gameVersion, int platform, bool isRemaster, int initlevel, Window& window, Renderer* gfx);
}