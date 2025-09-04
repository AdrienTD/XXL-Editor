#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "File.h"
#include "renderer.h"
#include "Image.h"
#include "window.h"
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>


uint32_t g_imguiLastTime;
const char* g_imguiFontFile = "resources/UbuntuMono-R.ttf";
float g_imguiFontSize = 14;

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
			renderer->drawBuffer(cmd->IdxOffset, cmd->ElemCount, cmd->VtxOffset);
		}

		delete vb; delete ib;
	}

	renderer->disableScissor();
}

void ImGuiImpl_CreateFontsTexture(Renderer *gfx)
{
	ImGuiIO &io = ImGui::GetIO();
	uint8_t *pix; int w, h, bpp;
	ImFontGlyphRangesBuilder glyphes;
	ImVector<ImWchar> ranges;
	static const ImWchar myranges[] = {
		0x0100, 0x024F, // Latin Extended
		0x0370, 0x03FF, // Greek and Coptic
		0x2000, 0x206F, // General Punctuation
		0
	};
	glyphes.AddRanges(io.Fonts->GetGlyphRangesDefault());
	glyphes.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	glyphes.AddRanges(myranges);
	glyphes.BuildRanges(&ranges);
	ImFontConfig config;
	config.OversampleH = 1;
	config.OversampleV = 1;
	config.RasterizerMultiply = 1.4f;
	//if(g_imguiFontFile)
	//	io.Fonts->AddFontFromFileTTF(g_imguiFontFile, g_imguiFontSize, &config, ranges.Data);
	auto [fntptr, fntsize] = GetResourceContent("UbuntuMono-R.ttf");
	io.Fonts->AddFontFromMemoryTTF(fntptr, (int)fntsize, 14, &config, ranges.Data);
	io.Fonts->GetTexDataAsRGBA32(&pix, &w, &h, &bpp);
	Image bm;
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

	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	
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

	ImGui::NewFrame();
}

void ImGuiImpl_Render(Renderer *gfx)
{
	ImGui::Render();
	ImGuiImpl_RenderDrawLists(ImGui::GetDrawData(), gfx);
}
