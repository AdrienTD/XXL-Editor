#pragma once

struct Renderer;
struct Window;

extern const char *g_imguiFontFile;
extern float g_imguiFontSize;

void ImGuiImpl_Init(Window *window);
void ImGuiImpl_CreateFontsTexture(Renderer *gfx);
void ImGuiImpl_NewFrame(Window *window);
void ImGuiImpl_Render(Renderer *gfx);
