#include "IGAbout.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include <imgui/imgui.h>
#include "renderer.h"
#include "File.h"
#include "rw.h" // for nth time, this is only for loading the logo

void EditorUI::IGAbout(EditorInterface& ui)
{
	static bool loaded = false;
	static texture_t logo = nullptr;
	static int logoWidth, logoHeight;
	if (!loaded) {
		auto [ptr, len] = GetResourceContent("logo.png");
		RwImage img = RwImage::loadFromMemory(ptr, len);
		logo = ui.gfx->createTexture(img);
		logoWidth = img.width;
		logoHeight = img.height;
		loaded = true;
	}

	ImGui::Image(logo, ImVec2(400.0f, 400.0f * (float)logoHeight / (float)logoWidth));
#ifdef XEC_APPVEYOR
	static const char* version = "Version " XEC_APPVEYOR;
#else
	static const char* version = "Development version";
#endif
	ImGui::Text("XXL Editor\n%s\nbuilt on " __DATE__, version);
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImGui::Text("Developed by AdrienTD\nThanks to S.P.Q.R");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImGui::TextLinkOpenURL("Wiki", "https://github.com/AdrienTD/XXL-Editor/wiki");
	ImGui::TextUnformatted("for documentation, tutorials, and links to Discord servers");
	ImGui::TextLinkOpenURL("GitHub repo", "https://github.com/AdrienTD/XXL-Editor");
	ImGui::TextUnformatted("for source code and stable releases");
	ImGui::TextLinkOpenURL("AppVeyor", "https://ci.appveyor.com/project/AdrienTD/xxl-editor");
	ImGui::TextUnformatted("for the latest development build");
}