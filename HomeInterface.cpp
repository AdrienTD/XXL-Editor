#include "HomeInterface.h"
#include "imgui/imgui.h"
#include "GuiUtils.h"
#include "GamePatcher.h"
#include "KEnvironment.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <thread>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "window.h"
#include "rw.h" // seriously need to separate Image from Rw
#include "renderer.h"
#include "File.h"

namespace {
	void IGInputPath(const char* label, std::string& str, bool isFolder, Window* window, const char* filter = nullptr, const char* defext = nullptr) {
		ImGui::PushID(label);
		float origItemWidth = ImGui::CalcItemWidth();
		ImGui::SetNextItemWidth(origItemWidth - ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
		ImGui::InputText("##pathInput", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &str);
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		if (ImGui::Button("...", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()))) {
			if (isFolder) {
				auto path = GuiUtils::SelectFolderDialogBox(window, label);
				if (!path.empty())
					str = std::filesystem::path(std::move(path)).u8string();
			}
			else {
				auto path = GuiUtils::OpenDialogBox(window, filter, defext);
				if (!path.empty())
					str = std::filesystem::path(std::move(path)).u8string();
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(isFolder ? "Select folder..." : "Select file...");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text(label);
		ImGui::PopID();
	}

	void IGInputString(const char* label, std::string& str) {
		ImGui::InputText(label, str.data(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &str);
	}
}

HomeInterface::HomeInterface(Window* window, Renderer* gfx) : window(window), gfx(gfx) {
	readProjectPaths();
	auto [logoPtr, logoSize] = GetResourceContent("logo.png");
	RwImage imgLogo = RwImage::loadFromMemory(logoPtr, logoSize);
	logoTexture = (void*)gfx->createTexture(imgLogo);
	logoWidth = imgLogo.width;
	logoHeight = imgLogo.height;
	auto [helpPtr, helpSize] = GetResourceContent("HelpMarker.png");
	helpTexture = (void*)gfx->createTexture(RwImage::loadFromMemory(helpPtr, helpSize));
}

void HomeInterface::iter()
{
	namespace fs = std::filesystem;

	// ----- PROJECT EDITOR VARIABLES

	static fs::path editProjectPath;
	static int editGame, editPlatform;
	static bool editIsRemaster;
	static std::string editInputPath, editOutputPath, editGameModule;
	static int editInitialLevel;
	static auto defaultEdit = []() {
		editGame = 0; editPlatform = 0;
		editIsRemaster = false;
		editInputPath.clear(); editOutputPath.clear(); editGameModule.clear();
		editInitialLevel = 0;
	};

	auto loadEdit = [this](const fs::path& path) {
		nlohmann::json proj;
		defaultEdit();
		editProjectPath = path;
		try {
			std::ifstream jfile(path);
			jfile >> proj;

			auto& pgame = proj.at("game");
			editGame = pgame.value<int>("id", 1) - 1;
			std::string p = pgame.value("platform", "kwn");
			auto& exts = KEnvironment::platformExt;
			auto it = std::find_if(std::begin(exts), std::end(exts), [&p](const char* s) {return !_stricmp(s, p.c_str()); });
			editPlatform = ((it != std::end(KEnvironment::platformExt)) ? (it - std::begin(KEnvironment::platformExt)) : 1) - 1;
			editIsRemaster = pgame.value<bool>("isRemaster", false);
			auto& ppaths = proj.at("paths");
			editInputPath = ppaths.value("inputPath", "");
			editOutputPath = ppaths.value("outputPath", "");
			editGameModule = ppaths.value("gameModule", "");
			if (proj.contains("editor"))
				editInitialLevel = proj.at("editor").value<int>("initialLevel", 0);
		}
		catch (const std::exception& ex) {
			printf("project file \"%s\" parsing failed: %s\n", editProjectPath.u8string().c_str(), ex.what());
		}
	};

	auto saveEdit = [this](const fs::path &path, bool isNew) {
		nlohmann::json js;
		js["formatVersion"] = "1.0";
		auto& jgame = js["game"];
		auto& jmeta = js["meta"];
		auto& jpaths = js["paths"];
		auto& jeditor = js["editor"];
		jgame["id"] = editGame + 1;
		jgame["platform"] = KEnvironment::platformExt[editPlatform + 1];
		jgame["isRemaster"] = editIsRemaster;
		jmeta["name"] = path.stem().u8string();
		jpaths["inputPath"] = editInputPath;
		jpaths["outputPath"] = editOutputPath;
		jpaths["gameModule"] = editGameModule;
		jeditor["initialLevel"] = editInitialLevel;
		std::ofstream file(path);
		file << js.dump(4);

		if (isNew) {
			projectPaths.insert(projectPaths.begin(), path.u8string());
			writeProjectPaths();
		}
	};

	// ----- BACKGROUND

	static const uint32_t bkgMainColorTop = 0xFFFFFFFF;
	static const uint32_t bkgMainColorBottom = 0xFF40C0FF;
	static uint32_t bkgCurrentColorTop = bkgMainColorTop;
	static uint32_t bkgCurrentColorBottom = bkgMainColorBottom;
	static double bkgLerpStartTime = 0.0, bkgLerpDuration = 0.0;
	static Vector3 bkgLerpStartColorTop, bkgLerpStartColorBottom;
	static Vector3 bkgLerpEndColorTop, bkgLerpEndColorBottom;
	static bool bkgLerping = false, bkgLerpDone = false;

	auto startBackgroundLerp = [](float duration, uint32_t top2, uint32_t bottom2) {
		bkgLerpStartTime = ImGui::GetTime();
		bkgLerpDuration = (double)duration;
		auto convert = [](uint32_t col) {
			ImVec4 v = ImGui::ColorConvertU32ToFloat4(col);
			return Vector3(v.x, v.y, v.z);
		};
		bkgLerpStartColorTop = convert(bkgCurrentColorTop);
		bkgLerpEndColorTop = convert(top2);
		bkgLerpStartColorBottom = convert(bkgCurrentColorBottom);
		bkgLerpEndColorBottom = convert(bottom2);
		bkgLerping = true;
		bkgLerpDone = false;
	};

	if (bkgLerping) {
		double ltime = ImGui::GetTime() - bkgLerpStartTime;
		float dt = (float)std::clamp(ltime / bkgLerpDuration, 0.0, 1.0);
		Vector3 topColor = bkgLerpStartColorTop + (bkgLerpEndColorTop - bkgLerpStartColorTop) * dt;
		Vector3 bottomColor = bkgLerpStartColorBottom + (bkgLerpEndColorBottom - bkgLerpStartColorBottom) * dt;
		bkgCurrentColorTop = ImGui::ColorConvertFloat4ToU32(ImVec4(topColor.x, topColor.y, topColor.z, 1.0f));
		bkgCurrentColorBottom = ImGui::ColorConvertFloat4ToU32(ImVec4(bottomColor.x, bottomColor.y, bottomColor.z, 1.0f));
		if (dt >= 1.0f) {
			bkgLerping = false;
			bkgLerpDone = true;
		}
	}

	auto fdl = ImGui::GetBackgroundDrawList();
	fdl->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2((float)window->getWidth(), (float)window->getHeight()), bkgCurrentColorTop, bkgCurrentColorTop, bkgCurrentColorBottom, bkgCurrentColorBottom);
	int scrw = window->getWidth(), scrh = window->getHeight();
	int mx = scrw / 2 - logoWidth / 2;
	fdl->AddImage(logoTexture, ImVec2((float)mx, 0.0f), ImVec2((float)(mx + logoWidth), (float)logoHeight));

	// ----- VERSION

#ifdef XEC_APPVEYOR
	static const char* version = "v" XEC_APPVEYOR " (" __DATE__ ")";
#else
	static const char* version = "Development version";
#endif
	//fdl->AddText(ImVec2(mx + 352, 132), 0xFF000000, version);

	// ----- Main Window

	float cntwidth = 780.0f, cntheight = 450.0f;
	ImGui::SetNextWindowSize(ImVec2(cntwidth, cntheight), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(scrw / 2 - cntwidth / 2, std::max(150.0f, scrh / 2 - cntheight / 2)), ImGuiCond_Always);
	ImGui::Begin("Home", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	auto TextCentered = [](const char* str) {
		ImVec2 cs = ImGui::CalcTextSize(str);
		ImGui::SetCursorPosX(std::round(ImGui::GetContentRegionMax().x * 0.5f - cs.x * 0.5f));
		ImGui::TextUnformatted(str);
	};
	auto ButtonCentered = [](const char* str) -> bool {
		ImVec2 cs = ImGui::CalcTextSize(str);
		ImGui::SetCursorPosX(std::round(ImGui::GetContentRegionMax().x * 0.5f - cs.x * 0.5f - ImGui::GetStyle().FramePadding.x));
		return ImGui::Button(str);
	};

	if (ButtonCentered("Create mod project from patched XXL1/2 PC copy"))
		ImGui::OpenPopup("Patcher");
	ImGui::SameLine();
	HelpMarker("Use this if you want to mod the original XXL1/2 on PC.\n"
		"This will copy the game files into a new folder,\n"
		"and patch the files so that you can open them in the editor.", true);
	TextCentered("For original XXL1/XXL2 PC");
	if (ButtonCentered("Create mod project from existing files")) {
		editProjectPath.clear();
		defaultEdit();
		ImGui::OpenPopup("Project editor");
	}
	ImGui::SameLine();
	HelpMarker("Use this if you want to mod Olympic Games, or XXL1/2 on console and Remasters.\n"
		"This will create a project using preexisting game files without copying\n"
		"and patching them, as patching is not required for these games.\n"
		"Thus, be sure to make backups of them first!", true);
	TextCentered("For other games/platforms and Remasters");
	ImGui::Spacing();

	bool wtOpenProject = false;
	ImGui::TextUnformatted("Projects:");
	static int curItem = -1;
	ImGui::SetNextItemWidth(-1);
	if (ImGui::BeginListBox("##ProjectList", ImVec2(0.0f, 300.0f))) {
		for (size_t i = 0; i < projectPaths.size(); i++) {
			auto& path = projectPaths[i];
			ImGui::PushID(i);
			if (ImGui::Selectable("##Project", curItem == i))
				curItem = i;
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				curItem = i;
				wtOpenProject = true;
			}
			auto projname = std::filesystem::u8path(path).stem().u8string();
			ImGui::SameLine();
			ImGui::TextUnformatted(projname.c_str());
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::TextUnformatted(path.c_str());
			ImGui::PopStyleColor(1);
			ImGui::PopID();
		}
		ImGui::EndListBox();
	}
	bool isItemSelected = curItem >= 0 && curItem < (int)projectPaths.size();
	if (ImGui::Button("Open") && isItemSelected) {
		wtOpenProject = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Hex") && isItemSelected) {
		wtOpenProject = true;
		hexMode = true;
	}
	ImGui::SameLine();
	const char* xecproj_filter = "XXL Editor Project file (.xecproj, .json)\0*.xecproj;*.json\0";
	const char* xecproj_ext = "xecproj";
	if (ImGui::Button("Import")) {
		auto jsonpath = GuiUtils::OpenDialogBox(window, xecproj_filter, xecproj_ext);
		if (!jsonpath.empty()) {
			projectPaths.insert(projectPaths.begin(), jsonpath.u8string());
			writeProjectPaths();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Edit") && isItemSelected) {
		loadEdit(fs::u8path(projectPaths[curItem]));
		ImGui::OpenPopup("Project editor");
	}
	ImGui::SameLine();
	if (ImGui::Button("Move up") && isItemSelected) {
		if (curItem > 0) {
			std::swap(projectPaths[curItem], projectPaths[curItem - 1]);
			curItem--;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Move down") && isItemSelected) {
		if (curItem < (int)projectPaths.size()-1) {
			std::swap(projectPaths[curItem], projectPaths[curItem + 1]);
			curItem++;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove") && isItemSelected) {
		ImGui::OpenPopup("Remove project");
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh"))
		readProjectPaths();
	ImGui::SameLine();
	if (ImGui::Button("More..."))
		ImGui::OpenPopup("AdvancedMenu");
	ImGui::SameLine();
	if (ImGui::Button("Exit"))
		quitApp = true;

	if (ImGui::BeginPopup("AdvancedMenu")) {
		if (ImGui::MenuItem("Extract resources")) {
			auto enumproc = [](HMODULE hModule, LPCWSTR lpType, LPWSTR lpName, LONG_PTR lParam) -> BOOL {
				if ((IS_INTRESOURCE(lpType)) || (IS_INTRESOURCE(lpName)))
					return TRUE;
				if (wcscmp(lpType, L"DATA") != 0)
					return TRUE;
				HRSRC rs = FindResourceW(hModule, lpName, L"DATA");
				HGLOBAL gl = LoadResource(hModule, rs);
				void* ptr = LockResource(gl);
				size_t len = SizeofResource(hModule, rs);
				wchar_t path[MAX_PATH];
				swprintf_s(path, L"%s\\%s", L"xec_resources", lpName);
				FILE* file = nullptr;
				_wfopen_s(&file, path, L"wb");
				if (file) {
					fwrite(ptr, len, 1, file);
					fclose(file);
				}
				return TRUE;
			};
			std::filesystem::create_directory("xec_resources");
			EnumResourceNamesW(nullptr, L"DATA", enumproc, 0);
		}
		ImGui::EndPopup();
	}

	float vslen = ImGui::CalcTextSize(version).x;
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - vslen - ImGui::GetStyle().WindowPadding.x);
	ImGui::TextUnformatted(version);

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();

	// ----- Patcher

	ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Patcher")) {
		static std::string curName = "My XXL Mod";
		static int curGame = 0;
		static std::string curGameModule, inputFolder, outputFolder;
		static std::unique_ptr<GamePatcher::GamePatcherThread> patcher;
		static std::thread patcherThread;
		static std::string statusText;
		static bool patchDone = false;
		static bool patchSuccess = false;

		if (patcher) {
			statusText = patcher->getStatus();
		}
		if (patchDone) {
			patcherThread.join();
			patchDone = false;
			patcher.reset();
			startBackgroundLerp(2.0f, bkgMainColorTop, bkgMainColorBottom);

			if (patchSuccess) {
				ImGui::CloseCurrentPopup();
				nlohmann::json js;
				js["formatVersion"] = "1.0";
				auto& jgame = js["game"];
				auto& jmeta = js["meta"];
				auto& jpaths = js["paths"];
				auto& jeditor = js["editor"];
				jgame["id"] = curGame + 1;
				jgame["platform"] = "kwn";
				jgame["isRemaster"] = false;
				jmeta["name"] = curName;
				jpaths["gameModule"] = "GameModule_MP_windowed.exe";
				jeditor["initialLevel"] = 8;
				std::string newProjPath = outputFolder + "/" + curName + ".xecproj";
				std::ofstream file(fs::u8path(newProjPath));
				file << js.dump(4);
				projectPaths.insert(projectPaths.begin(), std::move(newProjPath));
				writeProjectPaths();
			}
		}

		ImGui::Text("1. Give a name for your mod:");
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputText("##name", curName.data(), curName.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &curName);
		ImGui::NewLine();
		ImGui::Text("2. What is the game you want to mod?");
		ImGui::RadioButton("XXL 1", &curGame, 0);
		ImGui::SameLine();
		ImGui::RadioButton("XXL 2", &curGame, 1);
		ImGui::NewLine();
		ImGui::PushItemWidth(-1.0f);
		ImGui::Text("3. Original folder to copy from:");
		ImGui::SameLine(); HelpMarker("The folder where you installed the game,\ne.g. " R"(C:\Program Files (x86)\Atari\Asterix & Obelix XXL)" "\nIt's the folder that has the GameModule, GAME.KWN and the LVL folders in it.");
		IGInputPath("Input Folder", inputFolder, true, window);
		ImGui::NewLine();
		ImGui::Text("4. Folder for the mod to be placed into:");
		ImGui::SameLine(); HelpMarker("Path to a new folder where the game files will be copied and patched,\nso that your original copy won't be affected.");
		IGInputPath("Output Folder", outputFolder, true, window);
		ImGui::NewLine();
		ImGui::Text("5. Path to GameModule.elb for launching the game:");
		ImGui::SameLine(); HelpMarker("The GameModule file that already has a No-DRM patch.\nThe editor will have to patch it again to make modding possible.");
		IGInputPath("GameModule", curGameModule, false, window, "GameModule (*.elb,*.exe)\0*.elb;*.exe\0", "elb");
		ImGui::NewLine();
		ImGui::PopItemWidth();
		std::string statusTextRepr = statusText;
		if (patcher) {       
			int elipsisCount = 1 + (int)(ImGui::GetTime() * 2.0f) % 3;
			statusTextRepr.append((size_t)elipsisCount, '.');
			if (!bkgLerping) {
				static bool frame = false;
				startBackgroundLerp(1.0f, 0xFF004000, frame ? 0xFF008000 : 0xFF00FF00);
				frame = !frame;
			}
		}
		ImGui::TextUnformatted(statusTextRepr.c_str());
		int maxProgress = (curGame == 0) ? 11 : 13;
		float prog = patcher ? (patcher->progress / (float)maxProgress) : 0.0f;
		ImGui::ProgressBar(prog);
		ImGui::Spacing();
		if (ImGui::Button("OK")) {
			if (!patcher) {
				if (curGame == 0)
					patcher = std::make_unique<GamePatcher::GamePatcherThreadX1>();
				else if (curGame == 1)
					patcher = std::make_unique<GamePatcher::GamePatcherThreadX2>();
				patcher->inputPath = std::filesystem::u8path(inputFolder);
				patcher->outputPath = std::filesystem::u8path(outputFolder);
				patcher->elbPath = std::filesystem::u8path(curGameModule);
				patcherThread = std::thread([]() {
					patchSuccess = false;
					try {
						patcher->start();
						patcher->setStatus("Done!");
						patchSuccess = true;
					}
					catch (const GamePatcher::GamePatcherException& exc) {
						printf("thread exception: %s\n", exc.what());
						patcher->setStatus(exc.what());
					}
					patchDone = true;
				});
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			if (!patcher)
				ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// ----- Project editor

	ImGui::SetNextWindowSize(ImVec2(400.0f, 0.0f), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Project editor")) {
		static const char* gameNames[] = { "Asterix XXL", "Asterix XXL2", "Arthur", "Olympic Games", "Spyro DotD", "Alice", "HTTY Dragon" };
		static const char* platformNames[] = { "PC", "PlayStation 2", "GameCube", "PSP", "Wii", "Xbox 360", "PlayStation 3" };

		if (!editProjectPath.empty()) {
			if (ImGui::Button("Save")) {
				saveEdit(editProjectPath, false);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", editProjectPath.u8string().c_str());
			ImGui::SameLine();
		}
		if (ImGui::Button("Save as...")) {
			auto sas = GuiUtils::SaveDialogBox(window, xecproj_filter, xecproj_ext);
			if (!sas.empty()) {
				editProjectPath = std::move(sas);
				saveEdit(editProjectPath, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::SeparatorText("Game");
		ImGui::SameLine();
		HelpMarker("Indicate the game to mod.\n"
			"Not all configurations are supported by the editor yet.\n"
			"Tick \"Is Remaster\" and choose PC if you want to mod the\n"
			"2018/2020 remasters of XXL1/2.", true);
		ImGui::Combo("Game Version", &editGame, gameNames, std::size(gameNames));
		ImGui::Combo("Platform", &editPlatform, platformNames, std::size(platformNames));
		if (ImGui::Checkbox("Is Remaster", &editIsRemaster))
			if (editIsRemaster)
				editPlatform = 0;

		ImGui::SeparatorText("Paths");
		ImGui::SameLine();
		HelpMarker("Indicate the paths to the game's folder you want to mod.\n"
			" - Input path is where the game files are loaded from.\n"
			" - Output path is where your modifications are saved.\n"
			"   Optional, empty = Input path\n"
			" - GameModule path is the game's executable, applies to PC versions.\n"
			"   Optional, this is only used to launch the game from the editor for testing.\n"
			"\nAll paths can be absolute, or relative to the project file's folder.\n"
			"If you leave all paths empty, then the game files will be\n"
			"loaded from the folder where the project file is saved.", true);
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Remember to make backups!");
		IGInputPath("Input path", editInputPath, true, window);
		IGInputPath("Output path", editOutputPath, true, window);
		IGInputPath("GameModule", editGameModule, false, window, "GameModule (*.elb, *.exe)\0*.elb;*.exe\0");
		
		ImGui::SeparatorText("Editor");
		ImGui::InputInt("Initial level", &editInitialLevel);
		ImGui::EndPopup();
	}

	// ----- Project removal message

	ImGui::SetNextWindowSize(ImVec2(), ImGuiCond_Appearing);
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Remove project")) {
		ImGui::TextUnformatted("Are you sure to remove this project from the list?");
		ImGui::Spacing();
		ImGui::TextUnformatted(projectPaths[curItem].c_str());
		ImGui::Spacing();
		ImGui::TextUnformatted("This will only remove from the home menu, the project files won't be deleted.\n"
							   "If you want to remove the files, please do it manually.");
		if (ImGui::Button("Yes")) {
			projectPaths.erase(projectPaths.begin() + curItem);
			writeProjectPaths();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// ----- Project opening
	if (wtOpenProject) {
		writeProjectPaths();
		openProject(projectPaths[curItem]);
	}

	ImGui::End();
}

void HomeInterface::openProject(const std::string& u8filename)
{
	namespace fs = std::filesystem;
	fs::path projpath = fs::u8path(u8filename);
	try {
		nlohmann::json proj;
		std::ifstream jfile(projpath);
		jfile >> proj;

		const auto& pgame = proj.at("game");
		this->gameVersion = pgame.at("id").get<int>();
		this->cfgPlatformName = pgame.at("platform").get<std::string>();
		this->isRemaster = pgame.at("isRemaster").get<bool>();
		const auto& ppaths = proj.at("paths");

		if (std::string p = ppaths.value("inputPath", ""); !p.empty())
			this->gamePath = (projpath.parent_path() / fs::u8path(p)).u8string();
		else
			this->gamePath = projpath.parent_path().u8string();

		if (std::string p = ppaths.value("outputPath", ""); !p.empty())
			this->outGamePath = (projpath.parent_path() / fs::u8path(p)).u8string();
		else
			this->outGamePath = this->gamePath;

		if (std::string p = ppaths.value("gameModule", ""); !p.empty())
			this->gameModule = (projpath.parent_path() / fs::u8path(p)).u8string();
		else
			this->gameModule = (projpath.parent_path() / "GameModule_MP_windowed.exe").u8string();

		if (proj.contains("editor")) {
			const auto& peditor = proj.at("editor");
			if (peditor.contains("initialLevel"))
				this->initialLevel = peditor.at("initialLevel").get<int>();
		}

		projectChosen = true;
		goToEditor = true;
	}
	catch (const std::exception& ex) {
		wchar_t error[800];
		swprintf_s(error, L"Error when opening project file \"%s\":\n%S\n", projpath.c_str(), ex.what());
		MessageBoxW((HWND)window->getNativeWindow(), error, L"Project opening failure", 16);
	}
}

void HomeInterface::readProjectPaths()
{
	std::ifstream file("projectPaths.txt");
	if (!file.is_open()) return;
	projectPaths.clear();
	while (!file.eof()) {
		std::string buffer;
		std::getline(file, buffer);
		auto len = buffer.size();
		if (len > 0 && buffer[len - 1] == '\n')
			buffer.pop_back();
		if(!buffer.empty())
			projectPaths.push_back(std::move(buffer));
	}
}

void HomeInterface::writeProjectPaths()
{
	FILE* file;
	fopen_s(&file, "projectPaths.txt", "w");
	if (!file) return;
	for (auto& path : projectPaths) {
		fputs(path.c_str(), file);
		fputc('\n', file);
	}
	fclose(file);
}

void HomeInterface::HelpMarker(const char* message, bool padding)
{
	//ImGui::TextDisabled("(?)");
	float oldy = ImGui::GetCursorPosY();
	if (padding) {
		ImGui::SetCursorPosY(oldy + ImGui::GetStyle().FramePadding.y);
	}
	ImGui::Image(helpTexture, ImVec2(13, 13));
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("%s", message);
	}
	if (padding) {
		ImGui::SameLine();
		ImGui::SetCursorPosY(oldy);
		ImGui::NewLine();
	}
}
