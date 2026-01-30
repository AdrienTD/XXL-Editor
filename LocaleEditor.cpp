#include "LocaleEditor.h"
#include "imgui/imgui.h"
#include "KEnvironment.h"
#include "File.h"
#include "CKLocalObjectSubs.h"
#include <cstdlib>
#include "window.h"
#include <stb_image_write.h>
#include "GuiUtils.h"
#include <io.h>
#include <filesystem>
#include <charconv>
#include <fmt/format.h>

using namespace GuiUtils;

namespace
{
	Image importImage(const std::filesystem::path& filePath)
	{
		Image image = Image::loadFromFile(filePath.c_str());
		if (auto palImage = image.palettize(); palImage.has_value()) {
			image = *std::move(palImage);
		}
		return image;
	}
}

void LocaleEditor::gui()
{
	if (kenv.version == KEnvironment::KVERSION_XXL1 && (kenv.platform == KEnvironment::PLATFORM_PS2 || kenv.isRemaster)) {
		ImGui::Text("Not available for XXL1 PS2 and Romaster. Sorry.");
		return;
	}

	// Converts loc file text to editable Utf8 format and vice versa, also transforms special button icons to \a \b ...
	struct TextConverter {
		static std::string decode(const std::wstring& wstr) {
			//return wcharToUtf8(wstr.c_str());
			if (wstr.empty())
				return "\\0";
			std::string dec;
			for (const wchar_t& wc : wstr) {
				if (wc == '\\')
					dec.append("\\\\");
				else if (wc >= 0xE000 && wc <= 0xE000 + 'z' - 'a') {
					dec.push_back('\\');
					dec.push_back(wc - 0xE000 + 'a');
				}
				else if (wc == 0) {
					; // ignore null byte at the end (normally)
				}
				else {
					dec.append(wcharToUtf8(std::wstring_view(&wc, 1)));
				}
			}
			return dec;
		}
		static std::wstring encode(const std::string& str) {
			//return utf8ToWchar(str.c_str());
			if (str == "\\0")
				return {};
			std::wstring enc;
			int j = 0, i;
			for (i = 0; i < (int)str.size(); ) {
				if (str[i] == '\\') {
					if (str[i + 1] == '\\') {
						enc.append(utf8ToWchar(std::string_view(str.data() + j, i - j)));
						enc.push_back('\\');
						i += 2;
						j = i;
					}
					else if (str[i + 1] >= 'a' && str[i + 1] <= 'z') {
						enc.append(utf8ToWchar(std::string_view(str.data() + j, i - j)));
						enc.push_back(0xE000 + str[i + 1] - 'a');
						i += 2;
						j = i;
					}
					else
						i++;
				}
				else i++;
			}
			enc.append(utf8ToWchar(std::string_view(str.data() + j, i - j)));
			enc.push_back(0);
			return enc;
		}
	};

	auto resetFontTextures = [this]() {
		for (auto& doc : documents) {
			for (texture_t& tex : doc.fontTextures)
				gfx->deleteTexture(tex);
			doc.fontTextures.clear();
			doc.fntTexMap.clear();
			int i = 0;
			for (auto& pit : doc.cmgr2d.piTexDict.textures) {
				doc.fontTextures.push_back(gfx->createTexture(pit.images.front().image));
				doc.fntTexMap[pit.texture.name] = i++;
			}
		}
		};

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
		auto fsInputPath = std::filesystem::u8path(kenv.gamePath);

		std::vector<int> fndLevels;
		// Find all levels by searching for LVL*** folders in the gamepath
		for (auto& fsElem : std::filesystem::directory_iterator{ fsInputPath }) {
			if (fsElem.is_directory()) {
				auto dirname = fsElem.path().filename().string();
				if (dirname.size() == 6 && dirname.compare(0, 3, "LVL") == 0) {
					int lvl;
					auto [cptr, ec] = std::from_chars(dirname.c_str() + 3, dirname.c_str() + 6, lvl);
					if (ec == std::errc()) { // valid number
						fndLevels.push_back(lvl);
					}
				}
			}
		}

		int numLang = 1;
		for (int langid = 0; langid < numLang; langid++) {
			documents.emplace_back();
			auto& doc = documents.back();
			auto& locpack = doc.locpack;
			//locpack = KLocalPack();
			locpack.addFactory<Loc_CLocManager>();
			if (kenv.version < kenv.KVERSION_ARTHUR)
				locpack.addFactory<Loc_CManager2d>();
			char tbuf[32];
			sprintf_s(tbuf, kenv.isUsingNewFilenames() ? "%02uGLC.%s" : "%02uGLOC.%s", langid, KEnvironment::platformExt[kenv.platform]);
			std::unique_ptr<File> gloc = GetGzipFile((fsInputPath / tbuf).c_str(), "rb", kenv);
			locpack.deserialize(&kenv, gloc.get());

			if (Loc_CLocManager* loc = locpack.get<Loc_CLocManager>()) {
				numLang = loc->numLanguages;
				if (kenv.version < kenv.KVERSION_OLYMPIC || kenv.version >= KEnvironment::KVERSION_ALICE)
					doc.langStrIndex = loc->langStrIndices[langid];
				doc.langID = loc->langIDs[langid];
				if (kenv.version >= kenv.KVERSION_ARTHUR)
					doc.langArIndex = loc->langArIndices[langid];
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
			bool cmgr2dFound = kenv.version < kenv.KVERSION_ARTHUR;
			for (int lvl : fndLevels) {
				KLocalPack& llpack = doc.lvlLocpacks[lvl];
				llpack.addFactory<Loc_CKGraphic>();
				llpack.addFactory<Loc_CManager2d>();
				if (kenv.version < KEnvironment::KVERSION_ALICE)
					llpack.addFactory<Loc_CKSrvSekensor>();
				if (cmgr2dFound)
					llpack.kclassToNotDeserialize = Loc_CManager2d::FULL_ID;

				sprintf_s(tbuf, kenv.isUsingNewFilenames() ? "LVL%03u/%02uLLC%03u.%s" : "LVL%03u/%02uLLOC%02u.%s", lvl, langid, lvl, KEnvironment::platformExt[kenv.platform]);
				auto llpath = fsInputPath / tbuf;
				if (!std::filesystem::is_regular_file(llpath)) {
					// LLOC file missing... Just duplicate another one with same lang id
					if (!missingLlocWarningShown) {
						missingLlocWarningShown = true;
						MsgBox_Ok(window, "Some LLOC files are missing!\nThe editor will instead duplicate another LLOC file as a replacement.\nPlease check in the Level textures that the editor chose the correct language to duplicate!", MsgBoxIcon::Warning);
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
					std::unique_ptr<File> llocfile = GetGzipFile(llpath.c_str(), "rb", kenv);
					llpack.deserialize(&kenv, llocfile.get());
				}

				if (Loc_CKGraphic* kgfx = llpack.get<Loc_CKGraphic>()) {
					auto& texvec = doc.lvlTextures[lvl];
					for (auto& ktex : kgfx->textures)
						texvec.push_back(gfx->createTexture(ktex.img.image));
				}

				if (!cmgr2dFound) {
					if (Loc_CManager2d* lmgr = llpack.get<Loc_CManager2d>()) {
						assert(!lmgr->empty);
						cmgr2dFound = true;
						doc.cmgr2d = std::move(*lmgr);
					}
				}
			}

			assert(cmgr2dFound);
			if (kenv.version < kenv.KVERSION_ARTHUR) {
				Loc_CManager2d* lmgr = locpack.get<Loc_CManager2d>();
				assert(lmgr);
				doc.cmgr2d = std::move(*lmgr);
			}
			int i = 0;
			for (auto& tex : doc.cmgr2d.piTexDict.textures) {
				doc.fontTextures.push_back(gfx->createTexture(tex.images[0].image));
				doc.fntTexMap[tex.texture.name] = i++;
			}
		}
		langLoaded = true;
	}

	if (documents.empty())
		return;

	static int langid = 0;
	static unsigned int selfont = 0, selglyph = 0;

	char tbuf[256];
	auto getLangStr = [this](int langid, size_t index) -> const std::string& {
		static const std::string oobString = "(out of bounds)";
		if (index < documents[langid].stdTextU8.size())
			return documents[langid].stdTextU8[index];
		else
			return oobString;
	};
	sprintf_s(tbuf, "%i: %s", langid, getLangStr(langid, documents[langid].langStrIndex).c_str());
	if (ImGui::BeginCombo("Language", tbuf)) {
		for (int i = 0; i < (int)documents.size(); i++) {
			sprintf_s(tbuf, "%i: %s", i, getLangStr(i, documents[i].langStrIndex).c_str());
			if (ImGui::Selectable(tbuf)) {
				langid = i;
				selglyph = 0;
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Save all")) {
		std::vector<uint32_t> globStrIndices, globIDs, globArIndices;
		for (auto& doc : documents) {
			globStrIndices.push_back(doc.langStrIndex);
			globIDs.push_back(doc.langID);
			globArIndices.push_back(doc.langArIndex);
		}
		auto fsOutputPath = std::filesystem::u8path(kenv.outGamePath);
		for (int langid = 0; langid < (int)documents.size(); langid++) {
			LocalDocument& doc = documents[langid];
			if (Loc_CLocManager* loc = doc.locpack.get<Loc_CLocManager>()) {
				loc->numLanguages = (uint16_t)documents.size();
				loc->langStrIndices = globStrIndices;
				loc->langIDs = globIDs;
				loc->langArIndices = globArIndices;
				for (int i = 0; i < (int)loc->trcStrings.size(); i++) {
					std::wstring& lstr = loc->trcStrings[i].second;
					lstr = TextConverter::encode(doc.trcTextU8[i]);
				}
				for (int i = 0; i < (int)loc->stdStrings.size(); i++) {
					std::wstring& lstr = loc->stdStrings[i];
					lstr = TextConverter::encode(doc.stdTextU8[i]);
				}
			}

			// GLOC
			Loc_CManager2d* lmgr = doc.locpack.get<Loc_CManager2d>();
			if (kenv.version < kenv.KVERSION_ARTHUR)
				*lmgr = std::move(doc.cmgr2d);
			
			char tbuf[32];
			sprintf_s(tbuf, kenv.isUsingNewFilenames() ? "%02uGLC.%s" : "%02uGLOC.%s", langid, KEnvironment::platformExt[kenv.platform]);
			IOFile gloc = IOFile((fsOutputPath / tbuf).c_str(), "wb");
			doc.locpack.serialize(&kenv, &gloc);
			
			if (kenv.version < kenv.KVERSION_ARTHUR)
				doc.cmgr2d = std::move(*lmgr);

			// LLOCs
			for (auto& [lvl, lpack] : doc.lvlLocpacks) {
				Loc_CManager2d* lmgr = lpack.get<Loc_CManager2d>();
				if (kenv.version >= kenv.KVERSION_ARTHUR)
					*lmgr = std::move(doc.cmgr2d);

				sprintf_s(tbuf, "LVL%03u", lvl);
				std::filesystem::create_directory(fsOutputPath / tbuf); // ensure LVL directory is present
				
				sprintf_s(tbuf, kenv.isUsingNewFilenames() ? "LVL%03u/%02uLLC%03u.%s" : "LVL%03u/%02uLLOC%02u.%s", lvl, langid, lvl, KEnvironment::platformExt[kenv.platform]);
				IOFile lloc = IOFile((fsOutputPath / tbuf).c_str(), "wb");
				lpack.serialize(&kenv, &lloc);
				
				if (kenv.version >= kenv.KVERSION_ARTHUR)
					doc.cmgr2d = std::move(*lmgr);
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Duplicate language")) {
		documents.emplace_back(documents[langid]);
		documents.back().langID = 0xFFFFFFFF;
		documents.back().fontTextures.clear();
		for (auto& tex : documents.back().cmgr2d.piTexDict.textures) {
			documents.back().fontTextures.push_back(gfx->createTexture(tex.images[0].image));
		}
		documents.back().lvlTextures.clear();
		for (auto& e : documents.back().lvlLocpacks) {
			if (Loc_CKGraphic* kgfx = e.second.get<Loc_CKGraphic>()) {
				auto& texvec = documents.back().lvlTextures[e.first];
				for (auto& ktex : kgfx->textures)
					texvec.push_back(gfx->createTexture(ktex.img.image));
			}
		}
		langid = documents.size() - 1;
	}
	if (documents.size() > 1) {
		ImGui::SameLine();
		if (ImGui::Button("Remove language")) {
			if (MsgBox_YesNo(window, "Are you sure you want to remove the selected language?", MsgBoxIcon::Warning) == MsgBoxButton::Yes) {
				for (texture_t tex : documents[langid].fontTextures)
					gfx->deleteTexture(tex);
				for (auto& lt : documents[langid].lvlTextures)
					for (texture_t& tex : lt.second)
						gfx->deleteTexture(tex);
				documents.erase(documents.begin() + langid);
				if (langid >= (int)documents.size())
					langid = documents.size() - 1;
			}
		}
	}

	static bool showTextPreview = false;
	static std::string previewText = "Oblebolix Technologies";
	ImGui::SameLine();
	ImGui::Checkbox("Show Text Preview", &showTextPreview);

	auto& doc = documents[langid];

	if (ImGui::BeginTabBar("LangTabBar")) {
		if (Loc_CLocManager* loc = doc.locpack.get<Loc_CLocManager>()) {
			auto TextCommonButtons = [&]() {
				if (ImGui::Button("Export text")) {
					auto filepath = SaveDialogBox(window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
					if (!filepath.empty()) {
						FILE* tsv;
						if (!fsfopen_s(&tsv, filepath, "w")) {
							for (size_t i = 0; i < loc->stdStrings.size(); i++) {
								fprintf(tsv, "s%u\t%s\n", (int)i, doc.stdTextU8[i].c_str());
							}
							for (size_t i = 0; i < loc->trcStrings.size(); i++) {
								fprintf(tsv, "t%u\t%s\n", loc->trcStrings[i].first, doc.trcTextU8[i].c_str());
							}
							fclose(tsv);
						}
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Import text")) {
					auto filepath = OpenDialogBox(window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
					if (!filepath.empty()) {
						FILE* tsv;
						fsfopen_s(&tsv, filepath, "r");
						char line[2048];
						int lineIndex = 0;
						try {
							while (!feof(tsv)) {
								++lineIndex;
								fgets(line, sizeof(line), tsv);
								size_t lineLength = strlen(line);
								if (lineLength >= sizeof(line) - 2)
									throw std::runtime_error(fmt::format("Line {} is too long.", lineIndex));
								// Remove new line
								if (line[lineLength - 1] == '\n') {
									line[lineLength - 1] = 0;
									lineLength -= 1;
								}
								// Get type
								char type = line[0];
								if(type != 's' && type != 't')
									throw std::runtime_error(fmt::format("First character of line {} is expected to be 's' or 't'.", lineIndex));
								// Get ID
								unsigned int id;
								auto res = std::from_chars(line + 1, line + lineLength, id);
								if (res.ec != std::errc{})
									throw std::runtime_error(fmt::format("Failed to parse the ID at line {}.", lineIndex));
								// Check for TAB
								const char* ptr = res.ptr;
								if(*ptr != '\t')
									throw std::runtime_error(fmt::format("Line {} does not have a TAB character next to the ID.", lineIndex));
								ptr++;

								if (type == 's') {
									doc.stdTextU8.at(id) = ptr;
								}
								else if (type == 't') {
									auto it = std::find_if(loc->trcStrings.begin(), loc->trcStrings.end(), [&](auto& e) {return e.first == id; });
									if (it == loc->trcStrings.end()) throw std::runtime_error(fmt::format("Line {} contains invalid Trc ID {}.", lineIndex, id));
									size_t tindex = it - loc->trcStrings.begin();
									doc.trcTextU8.at(tindex) = ptr;
								}
							}
						}
						catch (std::exception& ex) {
							MsgBox_Ok(window, ex.what(), MsgBoxIcon::Error);
						}
						fclose(tsv);
					}
				}
				};
			auto TextLambda = [&](size_t numStrings, auto getPair) -> void {
				TextCommonButtons();
				ImGui::BeginChild("TextWnd");
				ImGuiListClipper clipper;
				clipper.Begin((int)numStrings);
				while (clipper.Step()) {
					for (size_t i = (size_t)clipper.DisplayStart; i < (size_t)clipper.DisplayEnd; i++) {
						auto [id, str] = getPair(i);
						ImGui::AlignTextToFramePadding();
						ImGui::Text("%u", id);
						ImGui::SameLine(48.0f);
						ImGui::PushID(&str);
						ImGui::SetNextItemWidth(-1.0f);
						ImGui::InputText("##Text", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
						if (showTextPreview && ImGui::IsItemActive())
							previewText = str;
						ImGui::PopID();
					}
				}
				ImGui::EndChild();
				};
			if (ImGui::BeginTabItem("TRC Text")) {
				TextLambda(loc->trcStrings.size(), [&](size_t i) -> std::pair<int, std::string&> { return { loc->trcStrings[i].first, doc.trcTextU8[i] }; });
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Standard Text")) {
				TextLambda(loc->stdStrings.size(), [&](size_t i) -> std::pair<int, std::string&> { return { (int)i, doc.stdTextU8[i] }; });
				ImGui::EndTabItem();
			}
		}

		if (Loc_CManager2d* lmgr = &doc.cmgr2d) {
			if (ImGui::BeginTabItem("Font textures")) {
				ImGui::BeginChild("FontTexWnd");
				for (int i = 0; i < (int)doc.fontTextures.size(); i++) {
					auto& tex = lmgr->piTexDict.textures[i];
					auto& image = tex.images[0].image;
					ImGui::PushID(&tex);
					ImGui::BulletText("%s (%i*%i, %i bpp)", tex.texture.name.c_str(), image.width, image.height, image.bpp);
					if (ImGui::Button("Export")) {
						auto filepath = SaveDialogBox(window, "PNG Image\0*.PNG\0\0", "png", tex.texture.name.c_str());
						if (!filepath.empty()) {
							Image cimg = image.convertToRGBA32();
							FILE* file; fsfopen_s(&file, filepath, "wb");
							auto callback = [](void* context, void* data, int size) {fwrite(data, size, 1, (FILE*)context); };
							stbi_write_png_to_func(callback, file, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
							fclose(file);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Replace")) {
						auto filepath = OpenDialogBox(window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
						if (!filepath.empty()) {
							image = importImage(filepath);
							gfx->deleteTexture(doc.fontTextures[i]);
							doc.fontTextures[i] = gfx->createTexture(image);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Fix")) {
						if (image.bpp == 32) {
							uint32_t* pix = (uint32_t*)image.pixels.data();
							int sz = image.width * image.height;
							for (int p = 0; p < sz; p++)
								if (pix[p] == 0xFF00FF00 || pix[p] == 0xFF8000FF)
									pix[p] &= 0x00FFFFFF;
							gfx->deleteTexture(doc.fontTextures[i]);
							doc.fontTextures[i] = gfx->createTexture(image);
						}
					}
					ImGui::Image(doc.fontTextures[i], ImVec2((float)image.width, (float)image.height));
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Font glyphs")) {
				if (ImGui::InputInt("Font", (int*)&selfont))
					selglyph = 0;
				selfont %= lmgr->fonts.size();
				RwFont2D& font = lmgr->fonts[selfont].rwFont;
				ImGui::Columns(2);

				if (ImGui::Button("Add glyph")) {
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
							uint16_t* glyphSlot = font.createGlyphSlot(chToAdd);
							*glyphSlot = glindex;
						}
					}
					else {
						ImGui::AlignTextToFramePadding();
						ImGui::TextUnformatted("Character already has a glyph!");
					}
					ImGui::EndPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Import BMFont")) {
					auto filepath = GuiUtils::OpenDialogBox(window, "BMFont Font file (*.fnt)\0*.FNT\0\0", "fnt");
					if (!filepath.empty()) {
						auto fontDir = filepath.parent_path();
						IOFile file(filepath.c_str(), "rb");
						file.seek(0, SEEK_END);
						size_t fileSize = file.tell();
						file.seek(0, SEEK_SET);
						std::unique_ptr<char[]> fileText = std::make_unique<char[]>(fileSize);
						file.read(fileText.get(), fileSize);

						// Parsing code
						const char* ptr = fileText.get();
						const char* const endPtr = ptr + fileSize;
						auto isWhitespace = [](char c) {return c == ' ' || c == '\t'; };
						auto isNewline = [](char c) {return c == '\n' || c == '\r'; };
						auto skipWhitespace = [&]() -> void {while (ptr < endPtr && isWhitespace(*ptr)) ++ptr; };
						auto getWord = [&]() -> std::string_view {
							skipWhitespace();
							const char* start = ptr;
							while (ptr < endPtr && !isWhitespace(*ptr) && !isNewline(*ptr)) ++ptr;
							return std::string_view(start, ptr - start);
							};
						auto getKeyValuePair = [&]() -> std::pair<std::string_view, std::string_view> {
							skipWhitespace();
							const char* startKey = ptr;
							while (ptr < endPtr && *ptr != '=' && !isNewline(*ptr)) ++ptr;
							const char* sep = ptr;
							if (ptr < endPtr && *sep == '=') ++ptr;
							const char* startVal = ptr;
							const char* endVal;
							if (ptr < endPtr && *startVal == '"') {
								++startVal; ++ptr;
								while (ptr < endPtr && *ptr != '"' && !isNewline(*ptr)) ++ptr;
								endVal = ptr;
								if (ptr < endPtr && *ptr == '"') ++ptr;
							}
							else {
								while (ptr < endPtr && !isWhitespace(*ptr) && !isNewline(*ptr)) ++ptr;
								endVal = ptr;
							}
							return { std::string_view(startKey, sep - startKey), std::string_view(startVal, endVal - startVal) };
							};
						auto nextLine = [&]() -> void {
							while (ptr < endPtr && !isNewline(*ptr)) ++ptr;
							while (ptr < endPtr && isNewline(*ptr)) ++ptr;
							};
						auto toInt = [](std::string_view str) -> int {
							int val;
							auto res = std::from_chars(str.data(), str.data() + str.size(), val);
							if (res.ec != std::errc{}) throw std::runtime_error("Failed to parse integer");
							return val;
							};

						int scaleW = -1, scaleH = -1;
						unsigned int padding[4] = { 0,0,0,0 };
						std::map<int, int> impTextureIndices;

						try {
							while (ptr < endPtr) {
								// read tag
								auto tag = getWord();
								// for each tag, we keep reading key-value pairs until end of line reached (= key is empty)
								if (tag == "common") {
									while (true) {
										auto [key, value] = getKeyValuePair();
										if (key.empty()) break;
										else if (key == "scaleW") scaleW = toInt(value);
										else if (key == "scaleH") scaleH = toInt(value);
									}
								}
								else if (tag == "page") {
									int pageId = -1;
									std::string_view pageFilename;
									while (true) {
										auto [key, value] = getKeyValuePair();
										if (key.empty()) break;
										else if (key == "id") pageId = toInt(value);
										else if (key == "file") pageFilename = value;
									}
									// find texture of same name
									size_t t;
									for (t = 0; t < doc.cmgr2d.piTexDict.textures.size(); ++t) {
										if (doc.cmgr2d.piTexDict.textures[t].texture.name == pageFilename)
											break;
									}
									// if not found, create new one
									if (t == doc.cmgr2d.piTexDict.textures.size()) {
										auto& pit = doc.cmgr2d.piTexDict.textures.emplace_back();
										pit.texture.name = pageFilename;
										pit.texture.filtering = 1;
										pit.texture.uAddr = 1;
										pit.texture.vAddr = 1;
										pit.texture.usesMips = true;
									}
									auto& pit = doc.cmgr2d.piTexDict.textures[t];
									// load the image
									auto texPath = fontDir / pageFilename;
									pit.images = { RwImage{.image = importImage(texPath) } };
									// assign texture to font
									auto it = std::find(font.texNames.begin(), font.texNames.end(), pageFilename);
									if (it != font.texNames.end()) {
										impTextureIndices[pageId] = it - font.texNames.begin();
									}
									else {
										impTextureIndices[pageId] = font.texNames.size();
										font.texNames.emplace_back(pageFilename);
									}
								}
								else if (tag == "char") {
									int charId = -1;
									int cx = -1, cy = -1, cw = -1, ch = -1, cpage = -1;
									while (true) {
										auto [key, value] = getKeyValuePair();
										if (key.empty()) break;
										else if (key == "id") charId = toInt(value);
										else if (key == "x") cx = toInt(value);
										else if (key == "y") cy = toInt(value);
										else if (key == "width") cw = toInt(value);
										else if (key == "height") ch = toInt(value);
										else if (key == "page") cpage = toInt(value);
									}
									if (cx < 0 || cy < 0 || cw < 0 || ch < 0 || cpage < 0 || charId < 0)
										throw std::runtime_error("Invalid or missing char arguments.");
									if (charId >= 0xFFFF) {
										throw std::runtime_error(fmt::format("Character code {} is outside of Renderware's supported range.", charId));
									}
									// Get charecter's glyph slot
									uint16_t* glyphSlot = font.createGlyphSlot((uint16_t)charId);
									// Create new glyph if slot empty.
									if (*glyphSlot == 0xFFFF) {
										*glyphSlot = font.glyphs.size();
										font.glyphs.emplace_back();
									}
									// Edit the glyph
									auto& glyph = font.glyphs[*glyphSlot];
									glyph.coords[0] = ((float)cx + 0.5f) / (float)scaleW;
									glyph.coords[1] = ((float)cy + 0.5f) / (float)scaleH;
									glyph.coords[2] = ((float)(cx + cw) + 0.5f) / (float)scaleW;
									glyph.coords[3] = ((float)(cy + ch) + 0.5f) / (float)scaleH;
									glyph.glUnk1 = (float)cw / (float)ch;
									glyph.texIndex = impTextureIndices[cpage];
								}
								nextLine();
							}
						}
						catch (std::runtime_error& ex) {
							MsgBox_Ok(window, ex.what(), MsgBoxIcon::Error);
						}
						resetFontTextures();
					}
				}

				ImGui::BeginChild("GlyphList");

				auto enumchar = [&doc, &font, &lmgr](int c, int g) {
					auto& glyph = font.glyphs[g];
					ImGui::PushID(&glyph);
					if (ImGui::Selectable("##glyph", selglyph == g, 0, ImVec2(0.0f, 32.0f)))
						selglyph = g;

					if (ImGui::GetIO().KeyCtrl) {
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
					}

					ImGui::SameLine();
					auto& ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
					float ratio = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
					auto& img = lmgr->piTexDict.textures[ti].images[0].image;
					float texratio = (float)img.width / (float)img.height;
					ImGui::Image(doc.fontTextures[ti], ImVec2(ratio * 32 * texratio, 32), ImVec2(glyph.coords[0], glyph.coords[1]), ImVec2(glyph.coords[2], glyph.coords[3]));

					ImGui::SameLine(48.0f);
					wchar_t wbuf[2] = { (wchar_t)c, 0 };
					ImGui::Text("Char %s\n0x%04X, %i", wcharToUtf8(std::wstring_view(wbuf, 1)).c_str(), c, c);
					ImGui::PopID();
				};

				for (int c = 0; c < 128; c++) {
					uint16_t g = font.charGlyphTable[c];
					if (g != 0xFFFF)
						enumchar(c, g);
				}
				for (int c = 0; c < (int)font.wideGlyphTable.size(); c++) {
					uint16_t g = font.wideGlyphTable[c];
					if (g != 0xFFFF)
						enumchar(c + font.firstWideChar, g);
				}
				ImGui::EndChild();

				ImGui::NextColumn();

				auto& glyph = font.glyphs[selglyph];
				auto& ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
				auto& img = lmgr->piTexDict.textures[ti].images[0].image;
				int pixcoords[4];
				for (size_t i = 0; i < 4; ++i)
					pixcoords[i] = (int)(glyph.coords[i] * ((i & 1) ? img.height : img.width));
				bool mod = false;
				mod |= ImGui::DragInt2("Upper-left  corner", &pixcoords[0], 0.5f);
				mod |= ImGui::DragInt2("Lower-right conner", &pixcoords[2], 0.5f);
				ImGui::InputScalar("Texture", ImGuiDataType_U8, &glyph.texIndex);
				glyph.texIndex %= font.texNames.size();
				auto updateGlyphRatio = [&img](RwFont2D::Glyph& glyph) {
					float u = glyph.coords[2] - glyph.coords[0], v = glyph.coords[3] - glyph.coords[1];
					glyph.glUnk1 = std::abs((img.width * u) / (img.height * v));
					};
				if (mod) {
					for (size_t i = 0; i < 4; ++i)
						glyph.coords[i] = ((float)pixcoords[i] + 0.5f) / ((i & 1) ? img.height : img.width);
					updateGlyphRatio(glyph);
				}

				ImGui::Spacing();

				static bool lockCharHeight = false;
				static bool hasBorders = false;
				ImGui::Checkbox("Lock character height", &lockCharHeight);
				ImGui::BeginDisabled(!lockCharHeight);
				ImGui::SameLine();
				ImGui::Checkbox("Has Borders (to set V coord. correctly)", &hasBorders);
				ImGui::EndDisabled();
				//ImGui::InputFloat("Auto height", &font.glyphHeight);

				ImGui::BeginChild("FontTexPreview", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				drawList->AddImage(doc.fontTextures[ti], pos, ImVec2(pos.x + img.width, pos.y + img.height));
				ImVec2 pmin = ImVec2(std::floor(pos.x + glyph.coords[0] * img.width), std::floor(pos.y + glyph.coords[1] * img.height));
				ImVec2 pmax = ImVec2(std::floor(pos.x + glyph.coords[2] * img.width), std::floor(pos.y + glyph.coords[3] * img.height));
				drawList->AddRect(pmin, pmax, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0, 1)));

				ImVec2 spos = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("FontTexPreview", ImVec2((float)img.width, (float)img.height), ImGuiButtonFlags_MouseButtonLeft);
				if (ImGui::IsItemClicked()) {
					glyph.coords[0] = glyph.coords[2] = (ImGui::GetMousePos().x - spos.x + 0.5f) / img.width;
					if (lockCharHeight) {
						int rhi = (int)(font.glyphHeight + (hasBorders ? 3 : 1));
						int row = (int)(ImGui::GetMousePos().y - spos.y) / rhi;
						float off = hasBorders ? 2.5f : 0.5f;
						glyph.coords[1] = ((float)(row * rhi) + off) / img.height;
						glyph.coords[3] = ((float)(row * rhi) + off + font.glyphHeight) / img.height;
					}
					else {
						glyph.coords[1] = glyph.coords[3] = (ImGui::GetMousePos().y - spos.y + 0.5f) / img.height;
					}
					updateGlyphRatio(glyph);
				}
				if (ImGui::IsItemActive() && ImGui::IsItemHovered()) {
					glyph.coords[2] = (ImGui::GetMousePos().x - spos.x + 0.5f) / img.width;
					if(!lockCharHeight)
						glyph.coords[3] = (ImGui::GetMousePos().y - spos.y + 0.5f) / img.height;
					updateGlyphRatio(glyph);
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
					for (int t = 0; t < (int)kgfx->textures.size(); t++) {
						auto& tex = kgfx->textures[t];
						auto& image = tex.img.image;
						ImGui::PushID(t);
						ImGui::BulletText("%s (%i*%i, %i bpp)", tex.name.c_str(), image.width, image.height, image.bpp);
						if (ImGui::Button("Export")) {
							auto filepath = SaveDialogBox(window, "PNG Image\0*.PNG\0\0", "png", tex.name.c_str());
							if (!filepath.empty()) {
								Image cimg = image.convertToRGBA32();
								FILE* file; fsfopen_s(&file, filepath, "wb");
								auto callback = [](void* context, void* data, int size) {fwrite(data, size, 1, (FILE*)context); };
								stbi_write_png_to_func(callback, file, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
								fclose(file);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Replace")) {
							auto filepath = OpenDialogBox(window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
							if (!filepath.empty()) {
								image = importImage(filepath);
								gfx->deleteTexture(doc.lvlTextures[sellvl][t]);
								doc.lvlTextures[sellvl][t] = gfx->createTexture(image);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Replace in all levels")) {
							auto filepath = OpenDialogBox(window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
							if (!filepath.empty()) {
								Image newimg = importImage(filepath);
								for (auto& e : doc.lvlLocpacks) {
									if (Loc_CKGraphic* kgfx = e.second.get<Loc_CKGraphic>()) {
										int c = 0;
										for (auto& cand : kgfx->textures) {
											if (cand.name == tex.name) {
												cand.img.image = newimg;
												gfx->deleteTexture(doc.lvlTextures[e.first][c]);
												doc.lvlTextures[e.first][c] = gfx->createTexture(image);
											}
											c++;
										}
									}
								}
							}
						}
						ImGui::Image(doc.lvlTextures[sellvl][t], ImVec2((float)image.width, (float)image.height));
						ImGui::PopID();
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

		if (showTextPreview) {
			Loc_CManager2d* lmgr = &doc.cmgr2d;
			ImGui::Begin("Font preview", &showTextPreview);
			if (ImGui::InputInt("Font", (int*)&selfont))
				selglyph = 0;
			selfont %= lmgr->fonts.size();
			RwFont2D& font = lmgr->fonts[selfont].rwFont;
			static float fontScale = 1.0f;
			ImGui::DragFloat("Scale", &fontScale, 0.02f, 0.0f, 16.f);
			ImGui::InputText("Text", (char*)previewText.c_str(), previewText.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &previewText);
			std::wstring encText = TextConverter::encode(previewText);
			for (size_t i = 0; i < encText.size(); ++i) {
				wchar_t c = encText[i];
				if (c == '\\') {
					if (encText[i + 1] == 'n') {
						ImGui::NewLine();
						++i;
						continue;
					}
					else if (encText[i + 1] == 'e') {
						c = ' ';
						++i;
					}
				}
				uint16_t glyphId = 0xFFFF;
				if (c >= 0 && c < 128)
					glyphId = font.charGlyphTable[c];
				else if (c >= font.firstWideChar && c < font.firstWideChar + font.wideGlyphTable.size())
					glyphId = font.wideGlyphTable[c - font.firstWideChar];
				if (glyphId != 0xFFFF) {
					auto& glyph = font.glyphs[glyphId];
					float height = font.glyphHeight * fontScale;
					auto& ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
					auto& img = lmgr->piTexDict.textures[ti].images[0].image;
					float ax = 0.5f / img.width;
					float ay = 0.5f / img.height;
					ImGui::Image(doc.fontTextures[ti], ImVec2(glyph.glUnk1 * height, height), ImVec2(glyph.coords[0]-ax, glyph.coords[1]-ay), ImVec2(glyph.coords[2]-ax, glyph.coords[3]-ay));
					ImGui::SameLine(0.0f, 0.0f);
				}
			}
			ImGui::End();
		}
	}
}
