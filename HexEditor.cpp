#include "HexEditor.h"
#include "KEnvironment.h"
#include "window.h"
#include "renderer.h"
#include "imgui/imgui.h"
#include "imguiimpl.h"
#include "imgui/imgui_memory_editor.h"
#include "Encyclopedia.h"

void HexEditorUI(const std::string& gamePath, const std::string& outGamePath, int gameVersion, int platform, bool isRemaster, int initlevel, Window& window, Renderer* gfx)
{
	Encyclopedia encyclo;
	KEnvironment kenv;

	encyclo.setKVersion(gameVersion);
	encyclo.window = &window;

	kenv.loadGame(gamePath.c_str(), gameVersion, platform, isRemaster);
	kenv.outGamePath = outGamePath;
	kenv.loadLevel(initlevel);

	while (!window.quitted()) {
		// Get window input
		window.handle();

		// Input + ImGui handling
		ImGuiImpl_NewFrame(&window);

		static CKObject* selobject = nullptr;
		ImGui::Begin("Objects");
		static int nextLevel = 8;
		ImGui::InputInt("Level number", &nextLevel);
		if (ImGui::Button("Load")) {
			selobject = nullptr;
			kenv.loadLevel(nextLevel);
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			//kenv.saveLevel(nextLevel);
		}
		ImGui::Separator();
		ImGui::BeginChild("ObjList");
		auto objnode = [&kenv, &encyclo](CKObject* obj, int index) {
			int fid = obj->getClassFullID();
			const char* classname = "?";
			const nlohmann::json* classInfoPtr = encyclo.getClassJson(fid);
			// TODO: Check to see if there is a key similar to "name" but is not "name" (display if user has made a typo)
			if (classInfoPtr != nullptr && classInfoPtr->contains("name"))
				classname = classInfoPtr->at("name").get_ref<const std::string&>().c_str();
			bool selected = obj == selobject;
			auto pushed = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0), "%s (%i,%i) %i: %s", classname, obj->getClassCategory(), obj->getClassID(), index, kenv.getObjectName(obj));
			if (ImGui::IsItemClicked())
				selobject = obj;
			if (pushed)
				ImGui::TreePop();
		};
		if (ImGui::TreeNode("Globals")) {
			int index = 0;
			for (CKObject* glob : kenv.globalObjects)
				objnode(glob, index++);
			ImGui::TreePop();
		}
		auto walkstr = [&objnode, &encyclo](KObjectList& objlist) {
			static const char* catnames[15] = { "Managers", "Services", "Hooks",
				"Hook Lives", "Groups", "Group Lives", "Components", "Camera",
				"Cinematic blocs", "Dictionaries", "Geometries", "Scene nodes",
				"Logic stuff", "Graphical stuff", "Errors"
			};
			for (int i = 0; i < 15; i++) {
				if (ImGui::TreeNode(catnames[i])) {
					auto& cat = objlist.categories[i];
					for (size_t clid = 0; clid < cat.type.size(); clid++) {
						auto& cl = cat.type[clid];
						if (cl.objects.empty())
							continue;
						int fid = i | (clid << 6);
						const char* classname = "?";
						const nlohmann::json* classInfoPtr = encyclo.getClassJson(fid);
						// TODO: Check to see if there is a key similar to "name" but is not "name" (display if user has made a typo)
						if (classInfoPtr != nullptr && classInfoPtr->contains("name"))
							classname = classInfoPtr->at("name").get_ref<const std::string&>().c_str();
						if (ImGui::TreeNode(&cl, "%s (%i, %i), %zu objects", classname, i, clid, cl.objects.size())) {
							int index = 0;
							for (CKObject* obj : cl.objects)
								objnode(obj, index++);
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
			};
		if (ImGui::TreeNode("Level")) {
			walkstr(kenv.levelObjects);
			ImGui::TreePop();
		}
		for (int str = 0; str < (int)kenv.numSectors; str++) {
			if (ImGui::TreeNode((void*)str, "Sector %i", str)) {
				walkstr(kenv.sectorObjects[str]);
				ImGui::TreePop();
			}
		}
		ImGui::EndChild();
		ImGui::End();

		static MemoryEditor memedit;
		ImGui::Begin("Hex View");
		if (selobject) {
			if (CKUnknown* unkobj = dynamic_cast<CKUnknown*>(selobject)) {
				int32_t newLength = (int32_t)unkobj->mem.size();
				//static const int32_t step = 1;
				bool b = ImGui::InputScalar("Size", ImGuiDataType_U32, &newLength);
				if (ImGui::IsItemDeactivatedAfterEdit() && newLength > 0 && (int32_t)unkobj->mem.size() != newLength) {
					void* orimem = unkobj->mem.data();
					int32_t orilen = (int32_t)unkobj->mem.size();
					unkobj->mem.resize(newLength);
					if (newLength - orilen > 0)
						memset((char*)unkobj->mem.data() + orilen, 0, newLength - orilen);
				}
				if (ImGui::Button("Copy hex")) {
					std::string str = std::string(unkobj->mem.size() * 2, 0);
					static const char decimals[17] = "0123456789ABCDEF";
					for (size_t i = 0; i < unkobj->mem.size(); i++) {
						str[2 * i] = decimals[((uint8_t*)unkobj->mem.data())[i] >> 4];
						str[2 * i + 1] = decimals[((uint8_t*)unkobj->mem.data())[i] & 15];
					}
					ImGui::SetClipboardText(str.c_str());
				}
				memedit.DrawContents(unkobj->mem.data(), unkobj->mem.size());
			}
		}
		ImGui::End();

		if (selobject) {
			CKUnknown* unkobj = (CKUnknown*)selobject;
			static char scriptBuffer[8192];
			ImGui::Begin("Reflection Script");
			ImGui::InputTextMultiline("##Script", scriptBuffer, sizeof(scriptBuffer));
			ImGui::End();
			ImGui::Begin("Reflection Result");
			ImGui::Columns(2);
			//ImGui::Text("todo");

			char* cursor = (char*)unkobj->mem.data();
			auto getBytes = [&cursor, &unkobj](size_t numBytes) -> char* {
				size_t bytesUsed = cursor - (char*)unkobj->mem.data();
				if (bytesUsed + numBytes > unkobj->mem.size())
					throw "overflow";
				char* prevCursor = cursor;
				cursor += numBytes;
				return prevCursor;
				};
			auto hexstr = [](char* bytes, int len) -> std::string {
				if (len <= 0)
					return {};
				static const char decimals[17] = "0123456789ABCDEF";
				std::string res = std::string(3 * len - 1, ' ');
				for (int i = 0; i < len; i++) {
					res[3 * i] = decimals[(bytes[i] >> 4) & 15];
					res[3 * i + 1] = decimals[bytes[i] & 15];
				}
				return res;
			};

			char* scriptPtr = scriptBuffer;
			auto iswhitespace = [](char c) -> bool {return c == ' ' || c == '\t' || c == '\n'; };
			try {
				while (*scriptPtr) {
					if (iswhitespace(*scriptPtr))
						scriptPtr++;
					else {
						char* wordPtr = scriptPtr;
						while (*scriptPtr && !iswhitespace(*scriptPtr))
							scriptPtr++;
						std::string term = std::string(wordPtr, scriptPtr);
						if (term == "u8") {
							char* byte = getBytes(1);
							ImGui::TextUnformatted(hexstr(byte, 1).c_str());
							ImGui::NextColumn();
							ImGui::PushID(byte);
							ImGui::InputScalar("##x", ImGuiDataType_U8, byte);
							ImGui::PopID();
							ImGui::NextColumn();
						}
						else if (term == "u16") {
							char* byte = getBytes(2);
							ImGui::TextUnformatted(hexstr(byte, 2).c_str());
							ImGui::NextColumn();
							ImGui::PushID(byte);
							ImGui::InputScalar("##x", ImGuiDataType_S16, byte);
							ImGui::PopID();
							ImGui::NextColumn();
						}
						else if (term == "u32") {
							char* byte = getBytes(4);
							ImGui::TextUnformatted(hexstr(byte, 4).c_str());
							ImGui::NextColumn();
							ImGui::PushID(byte);
							ImGui::InputScalar("##x", ImGuiDataType_S32, byte);
							ImGui::PopID();
							ImGui::NextColumn();
						}
						else if (term == "flt") {
							char* byte = getBytes(4);
							ImGui::TextUnformatted(hexstr(byte, 4).c_str());
							ImGui::NextColumn();
							ImGui::PushID(byte);
							ImGui::InputScalar("##x", ImGuiDataType_Float, byte);
							ImGui::PopID();
							ImGui::NextColumn();
						}
						else if (term == "ref") {
							char* byte = getBytes(4);
							ImGui::TextUnformatted(hexstr(byte, 4).c_str());
							ImGui::NextColumn();
							uint32_t ref = *(uint32_t*)byte;
							if (ref == 0xFFFFFFFF)
								ImGui::TextUnformatted("(null)");
							else
								ImGui::Text("(%i, %i, %i)", ref & 63, (ref >> 6) & 2047, ref >> 17);
							ImGui::NextColumn();
						}
					}
				}
			}
			catch (...) {
				ImGui::TextColored(ImVec4(1, 0, 0, 1), "FAIL");
			}
			ImGui::Columns();
			ImGui::End();
		}

		// Rendering
		gfx->setSize(window.getWidth(), window.getHeight());
		gfx->beginFrame();
		gfx->clearFrame(true, true, 0);
		ImGuiImpl_Render(gfx);
		gfx->endFrame();
	}
}
