#include "HomeInterface.h"
#include "imgui/imgui.h"
#include "GuiUtils.h"
#include "GamePatcher.h"
#include "KEnvironment.h"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <thread>

namespace {
	void IGInputPath(const char* label, std::string& str, bool isFolder, Window* window, const char* filter = nullptr, const char* defext = nullptr) {
		ImGui::PushID(label);
		ImGui::SetNextItemWidth(-24.0f);
		ImGui::InputText("##pathInput", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &str);
		ImGui::SameLine();
		if (ImGui::Button("...")) {
			if (isFolder) {
				auto path = GuiUtils::SelectFolderDialogBox(window, label);
				if (!path.empty())
					str = std::move(path);
			}
			else {
				auto path = GuiUtils::OpenDialogBox(window, filter, defext);
				if (!path.empty())
					str = std::move(path);
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(isFolder ? "Select folder..." : "Select file...");
		ImGui::PopID();
	}
}

void HomeInterface::iter()
{
	ImGui::Begin("Home", nullptr, ImGuiWindowFlags_NoCollapse);
	static const char* title = "XXL Editor";
	ImVec2 titleSize = ImGui::CalcTextSize(title);
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - titleSize.x * 0.5f);
	ImGui::Text("XXL Editor");
	static int curItem = -1;
	static const char* items[] = { "My XXL Project", "Beta Rome", "Funny character mod" };
	ImGui::SetNextItemWidth(-1);
	if (ImGui::ListBoxHeader("##ProjectList", ImVec2(0.0f, 400.0f))) {
		for (size_t i = 0; i < projectPaths.size(); i++) {
			auto& path = projectPaths[i];
			ImGui::PushID(i);
			if (ImGui::Selectable("##Project", curItem == i))
				curItem = i;
			auto projname = std::filesystem::path(path).stem().u8string();
			ImGui::SameLine();
			ImGui::TextUnformatted(projname.c_str());
			//ImGui::SameLine(ImGui::GetWindowWidth() * 0.2f);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::TextUnformatted(path.c_str());
			ImGui::PopStyleColor(1);
			ImGui::PopID();
		}
		ImGui::ListBoxFooter();
	}
	bool isItemSelected = curItem >= 0 && curItem < projectPaths.size();
	if (ImGui::Button("Open") && isItemSelected) {
		namespace fs = std::filesystem;
		fs::path projpath = fs::path(projectPaths[curItem]);
		try {
			nlohmann::json proj;
			try {
				std::ifstream jfile(projpath);
				jfile >> proj;
			}
			catch (const std::exception& ex) {
				printf("project file \"%s\" not found: %s\n", projectPaths[curItem].c_str(), ex.what());
			}

			const auto& pgame = proj.at("game");
			this->gameVersion = pgame.at("id").get<int>();
			this->cfgPlatformName = pgame.at("platform").get<std::string>();
			this->isRemaster = pgame.at("isRemaster").get<bool>();
			const auto& ppaths = proj.at("paths");
			if (ppaths.contains("inputPath"))
				this->gamePath = (projpath.parent_path() / ppaths.at("inputPath").get<std::string>()).u8string();
			else
				this->gamePath = projpath.parent_path().u8string().c_str();
			if (ppaths.contains("outputPath"))
				this->outGamePath = (projpath.parent_path() / ppaths.at("outputPath").get<std::string>()).u8string();
			else
				this->outGamePath = this->gamePath;
			if (ppaths.contains("gameModule"))
				this->gameModule = (projpath.parent_path() / ppaths.at("gameModule").get<std::string>()).u8string();
			else
				this->gameModule = (projpath.parent_path() / "GameModule_MP_windowed.exe").u8string();
			projectChosen = true;
			goToEditor = true;
		}
		catch (const std::exception& exc) {
			printf("cannot open project: %s\n", exc.what());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("New"))
		ImGui::OpenPopup("New project");
	ImGui::SameLine();
	if (ImGui::Button("Import")) {
		auto jsonpath = GuiUtils::OpenDialogBox(window, "XXL Editor Project file\0*.json\0", "json");
		if (!jsonpath.empty()) {
			projectPaths.push_back(std::move(jsonpath));
			writeProjectPaths();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Duplicate") && isItemSelected);
	ImGui::SameLine();
	if (ImGui::Button("Remove") && isItemSelected) {
		ImGui::OpenPopup("Remove project");
	}
	ImGui::SameLine();
	if (ImGui::Button("Refresh"))
		readProjectPaths();
	ImGui::SameLine();
	if (ImGui::Button("Exit"))
		quitApp = true;

	if (ImGui::BeginPopupModal("New project")) {
		static std::string curName = "My XXL Mod";
		static int curGame = 0, curPlatform = 0;
		static bool curIsRemaster = false;
		static std::string curGameModule, inputFolder, outputFolder;
		static const char* gameNames[] = { "Asterix XXL", "Asterix XXL2", "Arthur", "Olympic Games" };
		static const char* platformNames[] = { "PC", "PlayStation 2", "GameCube", "PSP" };
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

			if (patchSuccess) {
				ImGui::CloseCurrentPopup();
				nlohmann::json js;
				js["formatVersion"] = "1.0";
				auto& jgame = js["game"];
				auto& jmeta = js["meta"];
				auto& jpaths = js["paths"];
				jgame["id"] = curGame + 1;
				jgame["platform"] = KEnvironment::platformExt[curPlatform + 1];
				jgame["isRemaster"] = curIsRemaster;
				jmeta["name"] = curName;
				jpaths["gameModule"] = std::filesystem::u8path(curGameModule).filename().u8string();
				std::string newProjPath = outputFolder + "/" + curName + ".xec";
				std::ofstream file(newProjPath);
				file << js;
				projectPaths.insert(projectPaths.begin(), std::move(newProjPath));
				writeProjectPaths();
			}
		}

		ImGui::Text("1. Give a name for your mod:");
		ImGui::SetNextItemWidth(-1.0f);
		ImGui::InputText("##name", curName.data(), curName.capacity() + 1, ImGuiInputTextFlags_CallbackResize, GuiUtils::IGStdStringInputCallback, &curName);
		ImGui::NewLine();
		ImGui::Text("2. What is the game you want to mod?");
		ImGui::Combo("Game", &curGame, gameNames, std::size(gameNames));
		ImGui::Combo("Platform", &curPlatform, platformNames, std::size(platformNames));
		if (ImGui::Checkbox("Is Remaster", &curIsRemaster))
			if (curIsRemaster)
				curPlatform = 0;
		ImGui::NewLine();
		ImGui::Text("3. Original folder to copy from:");
		IGInputPath("InputFolder", inputFolder, true, window);
		ImGui::NewLine();
		ImGui::Text("4. Folder for the mod to be placed into:");
		IGInputPath("OutputFolder", outputFolder, true, window);
		ImGui::NewLine();
		ImGui::Text("5. Path to GameModule.elb for launching the game:");
		IGInputPath("GameModule", curGameModule, false, window, "GameModule (*.elb,*.exe)\0*.elb;*.exe\0", "elb");
		ImGui::NewLine();
		if ((curGame == 0 || curGame == 1) && curPlatform == 0 && !curIsRemaster) {
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "The GameModule and Level files will also be patched to make it moddable.\nThis can take more time.");
			ImGui::Spacing();
		}
		ImGui::TextUnformatted(statusText.c_str());
		float prog = patcher ? (patcher->progress / 10.0f) : 0.0f;
		ImGui::ProgressBar(prog);
		ImGui::Spacing();
		if (ImGui::Button("OK")) {
			if (!patcher) {
				if (curGame == 0 && curPlatform == 0 && !curIsRemaster)
					patcher = std::make_unique<GamePatcher::GamePatcherThreadX1>();
				else if (curGame == 1 && curPlatform == 0 && !curIsRemaster)
					patcher = std::make_unique<GamePatcher::GamePatcherThreadX2>();
				else
					patcher = std::make_unique<GamePatcher::GamePatcherThreadCopy>(curPlatform + 1);
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
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	if (ImGui::BeginPopupModal("Remove project")) {
		ImGui::Text("Are you sure to remove this project from the list?");
		ImGui::TextUnformatted(projectPaths[curItem].c_str());
		ImGui::Text("This will only remove from the home menu, the project files won't be deleted.");
		ImGui::Text("If you want to remove the files, please do it manually.");
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

	ImGui::End();
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
