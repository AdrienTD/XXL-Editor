#include "LocaleEditor.h"
#include "imgui/imgui.h"
#include "KEnvironment.h"
#include "CKLocalObjectSubs.h"
#include <cstdlib>
#include <Windows.h>
#include "window.h"
#include <stb_image_write.h>
#include "GuiUtils.h"
#include <io.h>

using namespace GuiUtils;

void LocaleEditor::gui()
{
	if (kenv.version > KEnvironment::KVERSION_XXL1 || kenv.platform == KEnvironment::PLATFORM_PS2) {
		ImGui::Text("Only available for XXL1 PC/GCN");
		return;
	}

	// Converts loc file text to editable Utf8 format and vice versa, also transforms special button icons to \a \b ...
	struct TextConverter {
		static std::string decode(const std::wstring& wstr) {
			//return wcharToUtf8(wstr.c_str());
			std::string dec;
			for (wchar_t wc : wstr) {
				if (wc == '\\')
					dec.append("\\\\");
				else if (wc >= 0xE000 && wc <= 0xE000 + 'z' - 'a') {
					dec.push_back('\\');
					dec.push_back(wc - 0xE000 + 'a');
				}
				else {
					dec.append(wcharToUtf8(std::wstring(1, wc).c_str()));
				}
			}
			return dec;
		}
		static std::wstring encode(const std::string& str) {
			//return utf8ToWchar(str.c_str());
			std::wstring enc;
			int j = 0, i;
			for (i = 0; i < str.size(); ) {
				if (str[i] == '\\') {
					if (str[i + 1] == '\\') {
						enc.append(utf8ToWchar(std::string(str, j, i - j).c_str()));
						enc.push_back('\\');
						i += 2;
						j = i;
					}
					else if (str[i + 1] >= 'a' && str[i + 1] <= 'z') {
						enc.append(utf8ToWchar(std::string(str, j, i - j).c_str()));
						enc.push_back(0xE000 + str[i + 1] - 'a');
						i += 2;
						j = i;
					}
					else
						i++;
				}
				else i++;
			}
			enc.append(utf8ToWchar(std::string(str, j, i - j).c_str()));
			return enc;
		}
	};

	static std::vector<int> fndLevels = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	// TODO: Find all levels by searching for LVL*** folders in the gamepath

	if (!langLoaded) {
		for (auto& doc : documents) {
			for (texture_t& tex : doc.fontTextures)
				gfx->deleteTexture(tex);
			for (auto& lt : doc.lvlTextures)
				for (texture_t& tex : lt.second)
					gfx->deleteTexture(tex);
		}
		documents.clear();

		bool missingLlocWarningShown = false;

		int numLang = 1;
		for (int langid = 0; langid < numLang; langid++) {
			documents.emplace_back();
			auto& doc = documents.back();
			auto& locpack = doc.locpack;
			//locpack = KLocalPack();
			locpack.addFactory<Loc_CLocManager>();
			locpack.addFactory<Loc_CManager2d>();
			char tbuf[512];
			sprintf_s(tbuf, "%s/%02uGLOC.%s", kenv.gamePath.c_str(), langid, KEnvironment::platformExt[kenv.platform]);
			IOFile gloc = IOFile(tbuf, "rb");
			locpack.deserialize(&kenv, &gloc);

			if (Loc_CManager2d* lmgr = locpack.get<Loc_CManager2d>()) {
				int i = 0;
				for (auto& tex : lmgr->piTexDict.textures) {
					doc.fontTextures.push_back(gfx->createTexture(tex.image));
					doc.fntTexMap[tex.texture.name] = i++;
				}
			}

			if (Loc_CLocManager* loc = locpack.get<Loc_CLocManager>()) {
				numLang = loc->numLanguages;
				doc.langStrIndex = loc->langStrIndices[langid];
				doc.langID = loc->langIDs[langid];
				//static const std::wstring zerostr = std::wstring(1, '\0');
				for (auto& trc : loc->trcStrings) {
					//assert(trc.second != zerostr);
					doc.trcTextU8.emplace_back(TextConverter::decode(trc.second));
				}
				for (std::wstring& str : loc->stdStrings) {
					//assert(str != zerostr);
					doc.stdTextU8.emplace_back(TextConverter::decode(str));
				}
			}

			// LLOCs
			for (int lvl : fndLevels) {
				KLocalPack& llpack = doc.lvlLocpacks[lvl];
				llpack.addFactory<Loc_CKGraphic>();
				sprintf_s(tbuf, "%s/LVL%03u/%02uLLOC%02u.%s", kenv.gamePath.c_str(), lvl, langid, lvl, KEnvironment::platformExt[kenv.platform]);
				if (_access(tbuf, 0) == -1) {
					// LLOC file missing... Just duplicate another one with same lang id
					if (!missingLlocWarningShown) {
						missingLlocWarningShown = true;
						MessageBox((HWND)window->getNativeWindow(), "Some LLOC files are missing!\nThe editor will instead duplicate another LLOC file as a replacement.\nPlease check in the Level textures that the editor chose the correct language to duplicate!", "XXL Editor", 48);
					}
					bool fnd = false;
					for (auto& dd : documents) {
						if (dd.langStrIndex == doc.langStrIndex) {
							auto it = dd.lvlLocpacks.find(lvl);
							if (it != dd.lvlLocpacks.end()) {
								llpack = it->second;
								fnd = true;
								break;
							}
						}
					}
					assert(fnd && "Missing LLOC file, no similar found!");
				}
				else {
					IOFile llocfile(tbuf, "rb");
					llpack.deserialize(&kenv, &llocfile);
				}

				if (Loc_CKGraphic* kgfx = llpack.get<Loc_CKGraphic>()) {
					auto& texvec = doc.lvlTextures[lvl];
					for (auto& ktex : kgfx->textures)
						texvec.push_back(gfx->createTexture(ktex.img));
				}
			}
		}
		langLoaded = true;
	}

	if (documents.empty())
		return;

	static int langid = 0;
	static unsigned int selfont = 0, selglyph = 0;

	char tbuf[256];
	sprintf_s(tbuf, "%i: %s", langid, documents[langid].stdTextU8[documents[langid].langStrIndex].c_str());
	if (ImGui::BeginCombo("Language", tbuf)) {
		for (int i = 0; i < documents.size(); i++) {
			sprintf_s(tbuf, "%i: %s", i, documents[i].stdTextU8[documents[i].langStrIndex].c_str());
			if (ImGui::Selectable(tbuf)) {
				langid = i;
				selglyph = 0;
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Save all")) {
		std::vector<uint32_t> globStrIndices, globIDs;
		for (auto& doc : documents) {
			globStrIndices.push_back(doc.langStrIndex);
			globIDs.push_back(doc.langID);
		}
		for (int langid = 0; langid < documents.size(); langid++) {
			LocalDocument& doc = documents[langid];
			if (Loc_CLocManager* loc = doc.locpack.get<Loc_CLocManager>()) {
				loc->numLanguages = documents.size();
				loc->langStrIndices = globStrIndices;
				loc->langIDs = globIDs;
				for (int i = 0; i < loc->trcStrings.size(); i++) {
					std::wstring& lstr = loc->trcStrings[i].second;
					lstr = TextConverter::encode(doc.trcTextU8[i]);
					if (!lstr.empty())
						lstr.push_back(0);
				}
				for (int i = 0; i < loc->stdStrings.size(); i++) {
					std::wstring& lstr = loc->stdStrings[i];
					lstr = TextConverter::encode(doc.stdTextU8[i]);
					if (!lstr.empty())
						lstr.push_back(0);
				}
			}

			char tbuf[512];
			sprintf_s(tbuf, "%s/%02uGLOC.%s", kenv.outGamePath.c_str(), langid, KEnvironment::platformExt[kenv.platform]);
			IOFile gloc = IOFile(tbuf, "wb");
			doc.locpack.serialize(&kenv, &gloc);

			// LLOCs
			for (int lvl : fndLevels) {
				sprintf_s(tbuf, "%s/LVL%03u/%02uLLOC%02u.%s", kenv.outGamePath.c_str(), lvl, langid, lvl, KEnvironment::platformExt[kenv.platform]);
				IOFile lloc = IOFile(tbuf, "wb");
				doc.lvlLocpacks.at(lvl).serialize(&kenv, &lloc);
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Duplicate language")) {
		documents.emplace_back(documents[langid]);
		documents.back().langID = 0xFFFFFFFF;
		documents.back().fontTextures.clear();
		if (Loc_CManager2d* lmgr = documents.back().locpack.get<Loc_CManager2d>()) {
			for (auto& tex : lmgr->piTexDict.textures) {
				documents.back().fontTextures.push_back(gfx->createTexture(tex.image));
			}
		}
		documents.back().lvlTextures.clear();
		for (auto& e : documents.back().lvlLocpacks) {
			if (Loc_CKGraphic* kgfx = e.second.get<Loc_CKGraphic>()) {
				auto& texvec = documents.back().lvlTextures[e.first];
				for (auto& ktex : kgfx->textures)
					texvec.push_back(gfx->createTexture(ktex.img));
			}
		}
		langid = documents.size() - 1;
	}
	if (documents.size() > 1) {
		ImGui::SameLine();
		if (ImGui::Button("Remove language")) {
			for (texture_t tex : documents[langid].fontTextures)
				gfx->deleteTexture(tex);
			for (auto& lt : documents[langid].lvlTextures)
				for (texture_t& tex : lt.second)
					gfx->deleteTexture(tex);
			documents.erase(documents.begin() + langid);
			if (langid >= documents.size())
				langid = documents.size() - 1;
		}
	}

	auto& doc = documents[langid];

	if (ImGui::BeginTabBar("LangTabBar")) {
		if (Loc_CLocManager* loc = doc.locpack.get<Loc_CLocManager>()) {
			if (ImGui::BeginTabItem("TRC Text")) {
				if (ImGui::Button("Export all")) {
					std::string filepath = SaveDialogBox(window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
					if (!filepath.empty()) {
						FILE* tsv;
						if (!fopen_s(&tsv, filepath.c_str(), "w")) {
							for (int i = 0; i < loc->stdStrings.size(); i++) {
								fprintf(tsv, "/\t%s\n", doc.stdTextU8[i].c_str());
							}
							for (int i = 0; i < loc->trcStrings.size(); i++) {
								fprintf(tsv, "%i\t%s\n", loc->trcStrings[i].first, doc.trcTextU8[i].c_str());
							}
							fclose(tsv);
						}
					}
				}
				ImGui::BeginChild("TRCTextWnd");
				for (int i = 0; i < loc->trcStrings.size(); i++) {
					auto& trc = loc->trcStrings[i];
					ImGui::Text("%u", trc.first);
					ImGui::SameLine(48.0f);
					auto& str = doc.trcTextU8[i];
					ImGui::PushID(&str);
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::InputText("##Text", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Standard Text")) {
				ImGui::BeginChild("StdTextWnd");
				for (int i = 0; i < loc->stdStrings.size(); i++) {
					ImGui::Text("%i", i);
					ImGui::SameLine(48.0f);
					auto& str = doc.stdTextU8[i];
					ImGui::PushID(&str);
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::InputText("##Text", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
		}

		if (Loc_CManager2d* lmgr = doc.locpack.get<Loc_CManager2d>()) {
			if (ImGui::BeginTabItem("Font textures")) {
				ImGui::BeginChild("FontTexWnd");
				for (int i = 0; i < doc.fontTextures.size(); i++) {
					auto& tex = lmgr->piTexDict.textures[i];
					ImGui::PushID(&tex);
					ImGui::BulletText("%s (%i*%i*%i)", tex.texture.name.c_str(), tex.image.width, tex.image.height, tex.image.bpp);
					if (ImGui::Button("Export")) {
						std::string filepath = SaveDialogBox(window, "PNG Image\0*.PNG\0\0", "png", tex.texture.name.c_str());
						if (!filepath.empty()) {
							RwImage cimg = tex.image.convertToRGBA32();
							stbi_write_png(filepath.c_str(), cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Replace")) {
						std::string filepath = OpenDialogBox(window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
						if (!filepath.empty()) {
							tex.image = RwImage::loadFromFile(filepath.c_str());
							gfx->deleteTexture(doc.fontTextures[i]);
							doc.fontTextures[i] = gfx->createTexture(tex.image);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Fix")) {
						if (tex.image.bpp == 32) {
							uint32_t* pix = (uint32_t*)tex.image.pixels.data();
							int sz = tex.image.width * tex.image.height;
							for (int p = 0; p < sz; p++)
								if (pix[p] == 0xFF00FF00 || pix[p] == 0xFF8000FF)
									pix[p] &= 0x00FFFFFF;
							gfx->deleteTexture(doc.fontTextures[i]);
							doc.fontTextures[i] = gfx->createTexture(tex.image);
						}
					}
					ImGui::Image(doc.fontTextures[i], ImVec2(tex.image.width, tex.image.height));
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Font glyphs")) {
				if (ImGui::InputInt("Font", (int*)&selfont))
					selglyph = 0;
				selfont %= lmgr->fonts.size();
				RwFont2D& font = lmgr->fonts[selfont].second;
				ImGui::Columns(2);

				if (ImGui::Button("Add")) {
					ImGui::OpenPopup("AddGlyphPopup");
				}
				if (ImGui::BeginPopup("AddGlyphPopup")) {
					static uint16_t chToAdd = 0;
					static std::string charPreview;
					ImGui::TextUnformatted("Add character:");
					bool mod = false;
					mod |= ImGui::InputScalar("Dec", ImGuiDataType_U16, &chToAdd);
					mod |= ImGui::InputScalar("Hex", ImGuiDataType_U16, &chToAdd, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::InputText("Char", (char*)charPreview.c_str(), charPreview.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &charPreview)) {
						chToAdd = utf8ToWchar(charPreview.c_str())[0];
					}
					ImGui::PushButtonRepeat(true);
					if (ImGui::ArrowButton("PreviousChar", ImGuiDir_Left)) { chToAdd--; mod = true; }
					ImGui::SameLine();
					if (ImGui::ArrowButton("NextChar", ImGuiDir_Right)) { chToAdd++; mod = true; }
					//ImGui::SameLine();
					ImGui::PopButtonRepeat();
					if (mod) {
						wchar_t wcs[2] = { chToAdd, 0 };
						charPreview = wcharToUtf8(wcs);
					}
					uint16_t* slotdest = nullptr;
					if (chToAdd < 128)
						slotdest = &font.charGlyphTable[chToAdd];
					else if (chToAdd >= font.firstWideChar && chToAdd < (font.firstWideChar + font.wideGlyphTable.size()))
						slotdest = &font.wideGlyphTable[chToAdd - font.firstWideChar];
					if (!(slotdest && *slotdest != 0xFFFF)) {
						if (ImGui::Button("OK")) {
							uint16_t glindex = (uint16_t)font.glyphs.size();
							font.glyphs.emplace_back();
							auto& glyph = font.glyphs.back();
							glyph.coords = { 0.0f, 0.0f, 0.25f, 0.25f };
							glyph.glUnk1 = 1.0f;
							glyph.texIndex = 0;
							if (slotdest) {
								*slotdest = glindex;
							}
							else {
								// need to resize widechar glyph table
								if (chToAdd < font.firstWideChar) {
									size_t origsize = font.wideGlyphTable.size();
									size_t origstart = font.firstWideChar - chToAdd;
									font.wideGlyphTable.resize(origsize + (font.firstWideChar - chToAdd));
									std::move(font.wideGlyphTable.begin(), font.wideGlyphTable.begin() + origsize, font.wideGlyphTable.begin() + origstart);
									font.wideGlyphTable[0] = glindex;
									for (int i = 1; i < origstart; i++)
										font.wideGlyphTable[i] = 0xFFFF;
									font.firstWideChar = chToAdd;
								}
								else if (chToAdd >= font.firstWideChar + font.wideGlyphTable.size()) {
									size_t origsize = font.wideGlyphTable.size();
									font.wideGlyphTable.resize(chToAdd - font.firstWideChar + 1);
									std::fill(font.wideGlyphTable.begin() + origsize, font.wideGlyphTable.end() - 1, 0xFFFF);
									font.wideGlyphTable.back() = glindex;
								}
								else assert(nullptr && "nani?!");
							}
						}
					}
					else {
						ImGui::AlignTextToFramePadding();
						ImGui::TextUnformatted("Character already has a glyph!");
					}
					ImGui::EndPopup();
				}

				ImGui::BeginChild("GlyphList");

				auto enumchar = [&doc, &font](int c, int g) {
					auto& glyph = font.glyphs[g];
					ImGui::PushID(&glyph);
					if (ImGui::Selectable("##glyph", selglyph == g, 0, ImVec2(0.0f, 32.0f)))
						selglyph = g;

					if (ImGui::BeginDragDropSource()) {
						ImGui::SetDragDropPayload("Glyph", &g, sizeof(g));
						wchar_t wbuf[2] = { (wchar_t)c, 0 };
						ImGui::Text("Glyph #%i\nChar %s (0x%04X)", g, wcharToUtf8(wbuf).c_str(), c);
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Glyph")) {
							glyph = font.glyphs[*(int*)payload->Data];
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					auto& ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
					float ratio = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
					ImGui::Image(doc.fontTextures[ti], ImVec2(ratio * 32, 32), ImVec2(glyph.coords[0], glyph.coords[1]), ImVec2(glyph.coords[2], glyph.coords[3]));

					ImGui::SameLine(48.0f);
					wchar_t wbuf[2] = { (wchar_t)c, 0 };
					ImGui::Text("Glyph #%i\nChar %s (0x%04X)", g, wcharToUtf8(wbuf).c_str(), c);
					ImGui::PopID();
				};

				for (int c = 0; c < 128; c++) {
					uint16_t g = font.charGlyphTable[c];
					if (g != 0xFFFF)
						enumchar(c, g);
				}
				for (int c = 0; c < font.wideGlyphTable.size(); c++) {
					uint16_t g = font.wideGlyphTable[c];
					if (g != 0xFFFF)
						enumchar(c + font.firstWideChar, g);
				}
				ImGui::EndChild();

				ImGui::NextColumn();

				auto& glyph = font.glyphs[selglyph];
				bool mod = false;
				mod |= ImGui::DragFloat2("UV Low", &glyph.coords[0], 0.01f);
				mod |= ImGui::DragFloat2("UV High", &glyph.coords[2], 0.01f);
				ImGui::DragFloat("w/h", &glyph.glUnk1);
				ImGui::InputScalar("Texture", ImGuiDataType_U8, &glyph.texIndex);
				glyph.texIndex %= font.texNames.size();

				if (mod) {
					glyph.glUnk1 = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
				}

				ImGui::Spacing();

				static bool hasBorders = false;
				ImGui::Checkbox("Has Borders (to set V coord. correctly)", &hasBorders);
				ImGui::InputFloat("Auto height", &font.glyphHeight);

				ImGui::BeginChild("FontTexPreview", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				auto& ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
				auto& img = lmgr->piTexDict.textures[ti].image;
				ImVec2 pos = ImGui::GetCursorScreenPos();
				drawList->AddImage(doc.fontTextures[ti], pos, ImVec2(pos.x + img.width, pos.y + img.height));
				ImVec2 pmin = ImVec2(std::floor(pos.x + glyph.coords[0] * img.width), std::floor(pos.y + glyph.coords[1] * img.height));
				ImVec2 pmax = ImVec2(std::floor(pos.x + glyph.coords[2] * img.width), std::floor(pos.y + glyph.coords[3] * img.height));
				drawList->AddRect(pmin, pmax, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0, 1)));

				ImVec2 spos = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("FontTexPreview", ImVec2(img.width, img.height), ImGuiButtonFlags_MouseButtonLeft);
				if (ImGui::IsItemClicked()) {
					glyph.coords[0] = glyph.coords[2] = (ImGui::GetMousePos().x - spos.x + 0.5f) / img.width;
					int rhi = (int)(font.glyphHeight + (hasBorders ? 3 : 1));
					int row = (int)(ImGui::GetMousePos().y - spos.y) / rhi;
					float off = hasBorders ? 2.5f : 0.5f;
					glyph.coords[1] = ((float)(row * rhi) + off) / img.height;
					glyph.coords[3] = ((float)(row * rhi) + off + font.glyphHeight) / img.height;
					glyph.glUnk1 = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
				}
				if (ImGui::IsItemActive() && ImGui::IsItemHovered()) {
					glyph.coords[2] = (ImGui::GetMousePos().x - spos.x + 0.5f) / img.width;
					glyph.glUnk1 = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
				}
				ImGui::EndChild();

				ImGui::Columns();
				ImGui::EndTabItem();
			}
		}

		if (ImGui::BeginTabItem("Level textures")) {
			static int sellvl = 0;
			ImGui::InputInt("Level", &sellvl);
			auto it = doc.lvlLocpacks.find(sellvl);
			if (it != doc.lvlLocpacks.end()) {
				KLocalPack& llpack = it->second;
				if (Loc_CKGraphic* kgfx = llpack.get<Loc_CKGraphic>()) {
					for (int t = 0; t < kgfx->textures.size(); t++) {
						auto& tex = kgfx->textures[t];
						ImGui::BulletText("%s", tex.name.c_str());
						if (ImGui::Button("Export")) {
							std::string filepath = SaveDialogBox(window, "PNG Image\0*.PNG\0\0", "png", tex.name.c_str());
							if (!filepath.empty()) {
								RwImage cimg = tex.img.convertToRGBA32();
								stbi_write_png(filepath.c_str(), cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Replace")) {
							std::string filepath = OpenDialogBox(window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
							if (!filepath.empty()) {
								tex.img = RwImage::loadFromFile(filepath.c_str());
								gfx->deleteTexture(doc.lvlTextures[sellvl][t]);
								doc.lvlTextures[sellvl][t] = gfx->createTexture(tex.img);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Replace in all levels")) {
							std::string filepath = OpenDialogBox(window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
							if (!filepath.empty()) {
								RwImage newimg = RwImage::loadFromFile(filepath.c_str());
								for (auto& e : doc.lvlLocpacks) {
									if (Loc_CKGraphic* kgfx = e.second.get<Loc_CKGraphic>()) {
										int c = 0;
										for (auto& cand : kgfx->textures) {
											if (cand.name == tex.name) {
												cand.img = newimg;
												gfx->deleteTexture(doc.lvlTextures[e.first][c]);
												doc.lvlTextures[e.first][c] = gfx->createTexture(tex.img);
											}
											c++;
										}
									}
								}
							}
						}
						ImGui::Image(doc.lvlTextures[sellvl][t], ImVec2(tex.img.width, tex.img.height));
					}
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Misc")) {
			ImGui::InputScalar("Lang text index", ImGuiDataType_U32, &doc.langStrIndex);
			ImGui::InputScalar("Lang ID", ImGuiDataType_S32, &doc.langID);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}
