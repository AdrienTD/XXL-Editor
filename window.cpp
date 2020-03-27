#include "window.h"
#include <SDL2/SDL.h>
#include "imgui/imgui.h"

Window::Window()
{
	_sw = SDL_CreateWindow("XXL Editor C++", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _width=800, _height=600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
}

static void HandleImGui(const SDL_Event &event)
{
	ImGuiIO &igio = ImGui::GetIO();
	switch (event.type) {
	case SDL_MOUSEMOTION:
		igio.MousePos = ImVec2(event.motion.x, event.motion.y);
		break;
	case SDL_MOUSEBUTTONDOWN:
		igio.MouseDown[0] = true;
		break;
	case SDL_MOUSEBUTTONUP:
		igio.MouseDown[0] = false;
		break;
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

void Window::handle()
{
	SDL_Event event;
	bool imguion = ImGui::GetCurrentContext() != nullptr;
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
		_keyPressed[i] = false;
	_mouseWheel = 0;

	bool igWantsMouse = false;
	if (imguion) {
		ImGuiIO &io = ImGui::GetIO();
		igWantsMouse = io.WantCaptureMouse;
	}

	while (SDL_PollEvent(&event)) {
		if (imguion)
			HandleImGui(event);
		switch (event.type) {
		case SDL_QUIT:
			_quit = true;
			break;
		case SDL_KEYDOWN:
			_keyDown[event.key.keysym.scancode] = true;
			_keyPressed[event.key.keysym.scancode] = true; break;
		case SDL_KEYUP:
			_keyDown[event.key.keysym.scancode] = false; break;
		case SDL_WINDOWEVENT:
			//printf("window event!!!\n");
			switch (event.window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				_width = event.window.data1;
				_height = event.window.data2;
				printf("Resized! %i %i\n", _width, _height);
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
			if (!igWantsMouse)
				_mouseDown[event.button.button] = false;
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
