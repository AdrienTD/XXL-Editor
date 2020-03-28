#pragma once

struct SDL_Window;

struct Window {
private:
	SDL_Window *_sw;
	int _width, _height;
	bool _keyDown[512] = {}, _keyPressed[512] = {};
	bool _mouseDown[16] = {};
	int _mouseX, _mouseY, _mouseWheel = 0;
	bool _quit = false;

public:
	Window();

	int getWidth() { return _width; }
	int getHeight() { return _height; }
	bool getKeyDown(int key) { return _keyDown[key]; }
	bool getKeyPressed(int key) { return _keyPressed[key]; }
	bool getMouseDown(int button) { return _mouseDown[button]; }
	int getMouseX() { return _mouseX; }
	int getMouseY() { return _mouseY; }
	int getMouseWheel() { return _mouseWheel; }
	bool quitted() { return _quit; }
	SDL_Window *getSDLWindow() { return _sw; }
	void *getNativeWindow();

	void handle();
};