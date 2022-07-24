#include "window.h"
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include <SDL2/SDL_syswm.h>

#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#endif

Window::Window()
{
	_sw = SDL_CreateWindow("XXL Editor C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width=800, _height=600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
}

static int igMouseIndex(int sdl) {
	switch (sdl) {
	case SDL_BUTTON_LEFT: return 0;
	case SDL_BUTTON_RIGHT: return 1;
	case SDL_BUTTON_MIDDLE: return 2;
	}
	return -1;
}

static void HandleImGui(const SDL_Event &event)
{
	ImGuiIO &igio = ImGui::GetIO();
	switch (event.type) {
	case SDL_MOUSEMOTION:
		igio.MousePos = ImVec2((float)event.motion.x, (float)event.motion.y);
		break;
	case SDL_MOUSEBUTTONDOWN: {
		int x = igMouseIndex(event.button.button);
		if (x != -1)
			igio.MouseDown[x] = true;
		break;
	}
	case SDL_MOUSEBUTTONUP: {
		int x = igMouseIndex(event.button.button);
		if (x != -1)
			igio.MouseDown[x] = false;
		break;
	}
	case SDL_MOUSEWHEEL:
		igio.MouseWheel += event.wheel.y;
		break;
	case SDL_KEYDOWN:
		igio.KeysDown[event.key.keysym.scancode] = true;
		break;
	case SDL_KEYUP:
		igio.KeysDown[event.key.keysym.scancode] = false;
		break;
	case SDL_TEXTINPUT:
		igio.AddInputCharactersUTF8(event.text.text);
		break;
	}
}

void * Window::getNativeWindow()
{
	SDL_SysWMinfo syswm;
	SDL_VERSION(&syswm.version);
	SDL_GetWindowWMInfo(getSDLWindow(), &syswm);
	HWND hWindow = syswm.info.win.window;
	return (void*)hWindow;
}

void Window::handle()
{
	SDL_Event event;
	bool imguion = ImGui::GetCurrentContext() != nullptr;
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
		_keyPressed[i] = false;
	_mouseWheel = 0;
	for (bool &b : _mousePressed)
		b = false;

	bool igWantsMouse = false, igWantsKeyboard = false;
	if (imguion) {
		ImGuiIO &io = ImGui::GetIO();
		igWantsMouse = io.WantCaptureMouse;
		igWantsKeyboard = io.WantCaptureKeyboard;
	}

	SDL_Keymod kmod = SDL_GetModState();
	_ctrlPressed = kmod & KMOD_CTRL;
	_shiftPressed = kmod & KMOD_SHIFT;
	_altPressed = kmod & KMOD_ALT;

	while (SDL_PollEvent(&event)) {
		if (imguion)
			HandleImGui(event);
		switch (event.type) {
		case SDL_QUIT:
			_quit = true;
			break;
		case SDL_KEYDOWN:
			if (igWantsKeyboard) break;
			_keyDown[event.key.keysym.scancode] = true;
			_keyPressed[event.key.keysym.scancode] = true;
			break;
		case SDL_KEYUP:
			_keyDown[event.key.keysym.scancode] = false;
			break;
		case SDL_WINDOWEVENT:
			//printf("window event!!!\n");
			switch (event.window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				_width = event.window.data1;
				_height = event.window.data2;
				//printf("Resized! %i %i\n", _width, _height);
				//if (g_gfxRenderer)
				//	g_gfxRenderer->Reset();
				break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (!igWantsMouse)
				_mouseDown[event.button.button] = true;
			break;
		case SDL_MOUSEBUTTONUP:
			_mouseDown[event.button.button] = false;
			if (!igWantsMouse)
				_mousePressed[event.button.button] = true;
			break;
		case SDL_MOUSEMOTION:
			_mouseX = event.motion.x;
			_mouseY = event.motion.y;
			break;
		case SDL_MOUSEWHEEL:
			if (!igWantsMouse)
				_mouseWheel += event.wheel.y;
			//printf("mouse wheel %i\n", g_mouseWheel);
			break;
		}
	}
}
