#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "renderer.h"
#include "rw.h"
#include "window.h"
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>


uint32_t g_imguiLastTime;
const char *g_imguiFontFile = nullptr;
float g_imguiFontSize = 12;

#define BGRA_TO_RGBA(x) ( (((x)&0xFF)<<16) | (((x)&0xFF0000)>>16) | ((x)&0xFF00FF00) )

void ImGuiImpl_RenderDrawLists(ImDrawData *dr, Renderer *renderer)
{
	ImGuiIO &io = ImGui::GetIO();
	//if(winMinimized) return;

	renderer->initFormDrawing();
	renderer->enableScissor();

	for(int i = 0; i < dr->CmdListsCount; i++)
	{
		ImDrawList *cl = dr->CmdLists[i];

		RVertexBuffer *vb = renderer->createVertexBuffer(cl->VtxBuffer.size());
		RIndexBuffer *ib = renderer->createIndexBuffer(cl->IdxBuffer.size());
		RVertex *vm = vb->lock();
		for(int j = 0; j < cl->VtxBuffer.size(); j++)
		{
			ImDrawVert *a = &cl->VtxBuffer[j];
			vm[j].x = a->pos.x;
			vm[j].y = a->pos.y;
			vm[j].z = 0;
			vm[j].color = a->col;
			vm[j].u = a->uv.x;
			vm[j].v = a->uv.y;
		}
		vb->unlock();
		uint16_t *im = ib->lock();
		for(int j = 0; j < cl->IdxBuffer.size(); j++)
			im[j] = cl->IdxBuffer[j];
		ib->unlock();
		renderer->setVertexBuffer(vb);
		renderer->setIndexBuffer(ib);

		for(int j = 0; j < cl->CmdBuffer.size(); j++)
		{
			ImDrawCmd *cmd = &cl->CmdBuffer[j];
			renderer->bindTexture(0, (texture_t)cmd->TextureId);
			renderer->setScissorRect(
				(int)cmd->ClipRect.x, (int)cmd->ClipRect.y,
				(int)(cmd->ClipRect.z - cmd->ClipRect.x),
				(int)(cmd->ClipRect.w - cmd->ClipRect.y)
			);
			renderer->drawBuffer(cmd->IdxOffset, cmd->ElemCount);
		}

		delete vb; delete ib;
	}

	renderer->disableScissor();
}

void ImGuiImpl_CreateFontsTexture(Renderer *gfx)
{
	ImGuiIO &io = ImGui::GetIO();
	uint8_t *pix; int w, h, bpp;
	if(g_imguiFontFile)
		io.Fonts->AddFontFromFileTTF(g_imguiFontFile, g_imguiFontSize);
	io.Fonts->GetTexDataAsRGBA32(&pix, &w, &h, &bpp);
	RwImage bm;
	bm.width = w; bm.height = h; bm.bpp = 32; bm.pitch = w * 4;
	bm.pixels.resize(w*h * 4);
	memcpy(bm.pixels.data(), pix, w*h * 4);
	texture_t t = gfx->createTexture(bm);
	io.Fonts->TexID = (void*)t;
}

void ImGuiImpl_Init(Window *window)
{
	ImGui::CreateContext();
	//imguienabled = true;
	ImGuiIO &io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_A] = SDL_GetScancodeFromKey(SDLK_a);
	io.KeyMap[ImGuiKey_C] = SDL_GetScancodeFromKey(SDLK_c);
	io.KeyMap[ImGuiKey_V] = SDL_GetScancodeFromKey(SDLK_v);
	io.KeyMap[ImGuiKey_X] = SDL_GetScancodeFromKey(SDLK_x);
	io.KeyMap[ImGuiKey_Y] = SDL_GetScancodeFromKey(SDLK_y);
	io.KeyMap[ImGuiKey_Z] = SDL_GetScancodeFromKey(SDLK_z);

#ifdef _WIN32
	SDL_SysWMinfo syswm;
	SDL_GetWindowWMInfo(window->getSDLWindow(), &syswm);
	HWND hWindow = syswm.info.win.window;
	//io.ImeWindowHandle = hWindow; // FIXME
#endif

	g_imguiLastTime = SDL_GetTicks();
}

void ImGuiImpl_NewFrame(Window *window)
{
	ImGuiIO &io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)window->getWidth(), (float)window->getHeight());

	uint32_t newtime = SDL_GetTicks();
	io.DeltaTime = (float)(newtime - g_imguiLastTime) / 1000.f;
	if (io.DeltaTime == 0.0f)
		io.DeltaTime = 1e-40f;
	g_imguiLastTime = newtime;

	SDL_Keymod kmod = SDL_GetModState();
	io.KeyCtrl = kmod  & KMOD_CTRL;
	io.KeyShift = kmod & KMOD_SHIFT;
	io.KeyAlt = kmod & KMOD_ALT;
	io.KeySuper = false;

	ImGui::NewFrame();
}

void ImGuiImpl_Render(Renderer *gfx)
{
	ImGui::Render();
	ImGuiImpl_RenderDrawLists(ImGui::GetDrawData(), gfx);
}
