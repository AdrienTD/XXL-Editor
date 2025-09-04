#include "IGTextureEditor.h"
#include "EditorInterface.h"
#include "GuiUtils.h"

#include "KEnvironment.h"
#include "CoreClasses/CKDictionary.h"

#include <imgui/imgui.h>
#include <stb_image_write.h>

void EditorUI::IGTextureEditor(EditorInterface& ui)
{
	using namespace GuiUtils;
	auto& kenv = ui.kenv;

	static int currentTexDictSector = 0;
	ImGui::InputInt("Sector", &currentTexDictSector);
	int currentTexDict = currentTexDictSector - 1;
	CTextureDictionary* texDict;
	ProTexDict* cur_protexdict;
	if (currentTexDict >= 0 && currentTexDict < (int)kenv.numSectors) {
		texDict = kenv.sectorObjects[currentTexDict].getFirst<CTextureDictionary>();
		cur_protexdict = &ui.str_protexdicts[currentTexDict];
	}
	else {
		texDict = kenv.levelObjects.getObject<CTextureDictionary>(0);
		cur_protexdict = &ui.protexdict;
		currentTexDictSector = 0;
		currentTexDict = -1;
	}
	if (ui.selTexID >= (int)texDict->piDict.textures.size())
		ui.selTexID = (int)texDict->piDict.textures.size() - 1;
	if (ImGui::Button("Insert")) {
		auto filepaths = MultiOpenDialogBox(ui.g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
		for (const auto& filepath : filepaths) {
			std::string name = filepath.stem().string().substr(0, 31);
			size_t index = texDict->piDict.findTexture(name);
			if (index == -1) {
				RwPITexDict::PITexture& tex = texDict->piDict.textures.emplace_back();
				tex.images.push_back(RwImage{ .image = Image::loadFromFile(filepath.c_str()) });
				tex.texture.name = name;
				tex.texture.filtering = 2;
				tex.texture.uAddr = 1;
				tex.texture.vAddr = 1;
			}
			else {
				RwPITexDict::PITexture& tex = texDict->piDict.textures[index];
				tex.images.clear();
				tex.images.push_back(RwImage{ .image = Image::loadFromFile(filepath.c_str()) });
			}
		}
		if (!filepaths.empty())
			cur_protexdict->reset(texDict);
	}
	ImGui::SameLine();
	if ((ui.selTexID != -1) && ImGui::Button("Replace")) {
		auto filepath = OpenDialogBox(ui.g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
		if (!filepath.empty()) {
			texDict->piDict.textures[ui.selTexID].images = { RwImage{.image = Image::loadFromFile(filepath.c_str()) } };
			texDict->piDict.textures[ui.selTexID].nativeVersion.reset();
			cur_protexdict->reset(texDict);
		}
	}
	ImGui::SameLine();
	if ((ui.selTexID != -1) && ImGui::Button("Remove")) {
		texDict->piDict.textures.erase(texDict->piDict.textures.begin() + ui.selTexID);
		cur_protexdict->reset(texDict);
		if (ui.selTexID >= (int)texDict->piDict.textures.size())
			ui.selTexID = -1;
	}
	ImGui::SameLine();
	if ((ui.selTexID != -1) && ImGui::Button("Export")) {
		auto& tex = texDict->piDict.textures[ui.selTexID];
		auto filepath = SaveDialogBox(ui.g_window, "PNG Image\0*.PNG\0\0", "png", tex.texture.name.c_str());
		if (!filepath.empty()) {
			Image cimg = tex.images[0].image.convertToRGBA32();
			FILE* file; fsfopen_s(&file, filepath, "wb");
			auto callback = [](void* context, void* data, int size) {fwrite(data, size, 1, (FILE*)context); };
			stbi_write_png_to_func(callback, file, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
			fclose(file);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Export all")) {
		auto dirname = SelectFolderDialogBox(ui.g_window, "Export all the textures to folder:");
		if (!dirname.empty()) {
			for (auto& tex : texDict->piDict.textures) {
				auto pname = dirname / (std::string(tex.texture.name.c_str()) + ".png");
				Image cimg = tex.images[0].image.convertToRGBA32();
				FILE* file; fsfopen_s(&file, pname, "wb");
				auto callback = [](void* context, void* data, int size) {fwrite(data, size, 1, (FILE*)context); };
				stbi_write_png_to_func(callback, file, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
				fclose(file);
			}
		}
	}

	if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableSetupColumn("TexLeft", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("TexRight", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		static ImGuiTextFilter search;
		search.Draw();
		ImGui::BeginChild("TexSeletion");
		for (int i = 0; i < (int)texDict->piDict.textures.size(); i++) {
			auto& tex = texDict->piDict.textures[i];
			if (!search.PassFilter(tex.texture.name.c_str()))
				continue;
			ImGui::PushID(i);
			if (ImGui::Selectable("##texsel", i == ui.selTexID, 0, ImVec2(0, 32))) {
				ui.selTexID = i;
			}
			if (ImGui::IsItemVisible()) {
				ImGui::SameLine();
				ImGui::Image(cur_protexdict->find(tex.texture.name.c_str()).second, ImVec2(32, 32));
				ImGui::SameLine();
				const auto& topImage = tex.images[0].image;
				ImGui::Text("%s\n%i*%i*%i", tex.texture.name.c_str(), topImage.width, topImage.height, topImage.bpp);
			}
			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::TableNextColumn();
		ImGui::BeginChild("TexViewer", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (ui.selTexID != -1) {
			const auto& tex = texDict->piDict.textures[ui.selTexID];
			const auto& topImage = tex.images[0].image;
			ImGui::Image(cur_protexdict->find(tex.texture.name.c_str()).second, ImVec2((float)topImage.width, (float)topImage.height));
		}
		ImGui::EndChild();
		ImGui::EndTable();
	}
}
