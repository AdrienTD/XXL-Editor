#include "window.h"
#include <SDL2/SDL.h>
#include "imgui/imgui.h"
#include <SDL2/SDL_syswm.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#endif

Window::Window()
{
	_sw = SDL_CreateWindow("XXL Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width=1280, _height=720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
}

static int igMouseIndex(int sdl) {
	switch (sdl) {
	case SDL_BUTTON_LEFT: return 0;
	case SDL_BUTTON_RIGHT: return 1;
	case SDL_BUTTON_MIDDLE: return 2;
	}
	return -1;
}

static ImGuiKey SdlToImGuiKey(SDL_Keycode code)
{
	switch (code) {
		case SDLK_TAB: return ImGuiKey_Tab;
		case SDLK_LEFT: return ImGuiKey_LeftArrow;
		case SDLK_RIGHT: return ImGuiKey_RightArrow;
		case SDLK_UP: return ImGuiKey_UpArrow;
		case SDLK_DOWN: return ImGuiKey_DownArrow;
		case SDLK_PAGEUP: return ImGuiKey_PageUp;
		case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
		case SDLK_HOME: return ImGuiKey_Home;
		case SDLK_END: return ImGuiKey_End;
		case SDLK_DELETE: return ImGuiKey_Delete;
		case SDLK_BACKSPACE: return ImGuiKey_Backspace;
		case SDLK_SPACE: return ImGuiKey_Space;
		case SDLK_RETURN: return ImGuiKey_Enter;
		case SDLK_ESCAPE: return ImGuiKey_Escape;
		case SDLK_a: return ImGuiKey_A;
		case SDLK_c: return ImGuiKey_C;
		case SDLK_v: return ImGuiKey_V;
		case SDLK_x: return ImGuiKey_X;
		case SDLK_y: return ImGuiKey_Y;
		case SDLK_z: return ImGuiKey_Z;
	}
	return ImGuiKey_None;
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
	case SDL_KEYUP: {
		auto kmod = event.key.keysym.mod;
		igio.AddKeyEvent(ImGuiKey_ModCtrl, kmod & KMOD_CTRL);
		igio.AddKeyEvent(ImGuiKey_ModShift, kmod & KMOD_SHIFT);
		igio.AddKeyEvent(ImGuiKey_ModAlt, kmod & KMOD_ALT);
		igio.AddKeyEvent(SdlToImGuiKey(event.key.keysym.sym), event.type == SDL_KEYDOWN);
		break;
	}
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

	// Pause when the window is minimized, until it is visible again.
	while (SDL_GetWindowFlags(_sw) & SDL_WINDOW_MINIMIZED) {
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT) {
			_quit = true;
			break;
		}
	}
}
