#ifndef XEC_RELEASE

#include "tests.h"
#include <string>
#include <INIReader.h>
#include "KEnvironment.h"
#include "DynArray.h"
#include "Events.h"
#include "ClassRegister.h"
#include "CKComponent.h"
#include "CKLogic.h"
#include "CKGeometry.h"
#include "rw.h"
#include "CKGroup.h"
#include <cassert>

#include "window.h"
#include "SDL2/SDL.h"
#include "renderer.h"
#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"

void Test_Cpnt2CSV() {
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.loadGame(gamePath.c_str(), KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC);

	struct NameListener : MemberListener {
		FILE* csv;
		NameListener(FILE* csv) : csv(csv) {}
		void write(const char* name) { fprintf(csv, "%s\t", name); }
		void reflect(uint8_t& ref, const char* name) override { write(name); }
		void reflect(uint16_t& ref, const char* name) override { write(name); }
		void reflect(uint32_t& ref, const char* name) override { write(name); }
		void reflect(float& ref, const char* name) override { write(name); }
		void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override { write(name); }
		void reflect(Vector3& ref, const char* name) override { fprintf(csv, "%s X\t%s Y\t%s Z\t", name, name, name); }
		void reflect(EventNode& ref, const char* name, CKObject* user) override { write(name); };
		void reflect(std::string& ref, const char* name) override { abort(); } // TODO
	};
	struct ValueListener : MemberListener {
		FILE* csv;
		ValueListener(FILE* csv) : csv(csv) {}
		void reflect(uint8_t& ref, const char* name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint16_t& ref, const char* name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint32_t& ref, const char* name) override { fprintf(csv, "%u\t", ref); }
		void reflect(float& ref, const char* name) override { fprintf(csv, "%f\t", ref); }
		void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override { fprintf(csv, "%s\t", ref._pointer->getClassName()); }
		void reflect(Vector3& ref, const char* name) override { fprintf(csv, "%f\t%f\t%f\t", ref.x, ref.y, ref.z); }
		void reflect(EventNode& ref, const char* name, CKObject* user) override { fprintf(csv, "(%i,%i)\t", ref.seqIndex, ref.bit); };
		void reflect(std::string& ref, const char* name) override { abort(); } // TODO
	};

	FILE* csv;
	fopen_s(&csv, "EnemyCpnts.txt", "w");
	NameListener nl(csv);
	ValueListener vl(csv);

	for (int lvl = 1; lvl <= 6; lvl++) {
		kenv.loadLevel(lvl);
		if (lvl == 1) {
			CKBasicEnemyCpnt* firstcpnt = kenv.levelObjects.getFirst<CKBasicEnemyCpnt>();
			fprintf(csv, "Level\tIndex\t");
			firstcpnt->reflectMembers2(nl, &kenv);
		}
		int index = 0;
		for (CKObject* obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
			fprintf(csv, "\n%i\t%i\t", lvl, index);
			obj->cast<CKBasicEnemyCpnt>()->reflectMembers2(vl, &kenv);
			index++;
		}
	}

	fclose(csv);
}

void Test_LevelSizeCheck()
{
	static constexpr std::array<std::tuple<const char*, int, int>, 4> games = { {
		{"C:\\Users\\Adrien\\Downloads\\virtualboxshare\\aoxxl2demo\\Astérix & Obélix XXL2 DEMO", KEnvironment::KVERSION_XXL2, KEnvironment::PLATFORM_PC},
		{"C:\\Apps\\Asterix at the Olympic Games", KEnvironment::KVERSION_OLYMPIC, KEnvironment::PLATFORM_PC},
		{"D:\\PSP_GAME\\USRDIR", KEnvironment::KVERSION_ARTHUR, KEnvironment::PLATFORM_PSP},
		{"C:\\Users\\Adrien\\Desktop\\kthings\\xxl1_mp_orig", KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC},
	} };
	static const std::string outputPath = "C:\\Users\\Adrien\\Desktop\\kthings\\x2savetest";
	for (const auto& game : games) {
		const char* gamePath; int gameVersion, gamePlatform;
		std::tie(gamePath, gameVersion, gamePlatform) = game;

		printf("****** %s ******\n", gamePath);

		KEnvironment kenv;
		ClassRegister::registerClasses(kenv, gameVersion, gamePlatform, false);
		kenv.loadGame(gamePath, gameVersion, gamePlatform);
		kenv.outGamePath = outputPath;
		kenv.loadLevel(1);
		kenv.saveLevel(1);

		std::string lvlpath = std::string("\\LVL001\\LVL01.") + KEnvironment::platformExt[kenv.platform];
		std::string inlvlpath = gamePath + lvlpath;
		std::string outlvlpath = outputPath + lvlpath;
		DynArray<uint8_t> inCnt, outCnt;

		for (std::pair<const char*, DynArray<uint8_t>*> p : { std::make_pair(inlvlpath.c_str(), &inCnt), std::make_pair(outlvlpath.c_str(), &outCnt) }) {
			FILE* file;
			fopen_s(&file, p.first, "rb");
			fseek(file, 0, SEEK_END);
			size_t len = ftell(file);
			fseek(file, 0, SEEK_SET);
			p.second->resize(len);
			fread(p.second->data(), len, 1, file);
			fclose(file);
		}

		int numDiffBytes = 0;
		size_t minSize = (inCnt.size() < outCnt.size()) ? inCnt.size() : outCnt.size();

		for (size_t i = 0; i < minSize; i++)
			if (inCnt[i] != outCnt[i])
				numDiffBytes++;

		printf("************\n");
		printf(" Input file size: %10u bytes\n", inCnt.size());
		printf("Output file size: %10u bytes\n", outCnt.size());
		printf(" Different bytes: %10u bytes\n", numDiffBytes);
		printf("************\n");
	}
	getchar();
}

void Test_ChoreoFreqValues() {
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	//ClassRegister::registerClassesForXXL1PC(kenv);
	kenv.addFactory<CKChoreoKey>();
	kenv.loadGame(gamePath.c_str(), KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC, true);

	std::map<std::array<float, 3>, int> triplets;
	for (int i = 1; i <= 6; i++) {
		kenv.loadLevel(i);

		for (CKObject* okey : kenv.levelObjects.getClassType<CKChoreoKey>().objects) {
			CKChoreoKey* key = okey->cast<CKChoreoKey>();
			//printf("key %f %f %f\n", key->unk1, key->unk2, key->unk3);
			triplets[{ key->unk1, key->unk2, key->unk3 }]++;
		}
	}
	for (auto& a : triplets) {
		printf("%f %f %f %i\n", a.first[0], a.first[1], a.first[2], a.second);
	}
	getchar();
}

void Test_extractRemasterFilepaths() {
	KEnvironment kenv;
	ClassRegister::registerClassesForXXL1PC(kenv);
	kenv.loadGame("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Asterix & Obelix XXL 1\\XXL1Resources\\K", KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC, true);
	FILE* tsv;
	fopen_s(&tsv, "C:\\Users\\Adrien\\Desktop\\kthings\\remaster_model_paths.txt", "w");
	if (!tsv) return;
	std::map<std::tuple<int, int, int, std::string>, std::set<std::tuple<std::string, std::string, int>>> modelMap;
	auto visitSector = [&kenv,&modelMap](KObjectList& objlist, int strnum) -> void {
		for (auto& type : objlist.categories[CKAnyGeometry::CATEGORY].type) {
			for (CKObject* obj : type.objects) {
				CKAnyGeometry* geo = obj->cast<CKAnyGeometry>();
				if (!geo->clump) continue;
				RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
				if (geo->hdKifPath.empty())
					continue;
				size_t pathstart = geo->hdKifPath.find("\\origins\\");
				assert(pathstart != std::string::npos);
				if (geo->hdKifPath.find("_remaster\\item_ambiance_only_remaster") != std::string::npos)
					continue;
				const auto &texname = rwgeo->materialList.materials.front().texture.name;
				modelMap[{geo->getClassID(), (int)rwgeo->verts.size(), (int)rwgeo->tris.size(), texname}].insert({ geo->hdKifPath.substr(pathstart), geo->hdMatName, geo->hdUnk1 });
				//fprintf(tsv, "%i\t%i\t%i\t%i\t%s\t%s\n", geo->getClassID(), rwgeo->verts.size(), rwgeo->tris.size(), geo->hdUnk1, geo->hdKifPath.c_str(), geo->hdMatName.c_str());
			}
		}
	};
	for (int lvl = 0; lvl <= 8; lvl++) {
		kenv.loadLevel(lvl);
		visitSector(kenv.levelObjects, -1);
		for (int i = 0; i < kenv.numSectors; i++)
			visitSector(kenv.sectorObjects[i], i);
	}
	for (auto& p : modelMap) {
		fprintf(tsv, "[Key] (%i %i %i %s)%s\n", std::get<0>(p.first), std::get<1>(p.first), std::get<2>(p.first), std::get<3>(p.first).c_str(), (p.second.size() > 1) ? " [ALERT!]" : "");
		for (auto& val : p.second)
			fprintf(tsv, "%i %s %s\n", std::get<2>(val), std::get<0>(val).c_str(), std::get<1>(val).c_str());
	}
	fclose(tsv);
	getchar();
}

void Test_MarioDifficulty() {
	KEnvironment kenv;
	ClassRegister::registerClassesForXXL2Remaster(kenv);
	kenv.loadGame("C:\\Program Files (x86)\\Steam\\steamapps\\common\\Asterix XXL 2\\XXL2Resources\\K", KEnvironment::KVERSION_XXL2, KEnvironment::PLATFORM_PC, true);
	kenv.outGamePath = "C:\\Users\\Adrien\\Desktop\\kthings\\xxl2hd_mod\\XXL2Resources\\K";
	for (int lvlindex : {1, 2}) { //, 2, 3, 4, 6, 7, 8, 9, 10, 11}) {
		kenv.loadLevel(lvlindex);
		// Find Mario's pool
		CKGrpPoolSquad* marioPool = nullptr;
		const std::string marioPoolName = "Mario";
		for (CKObject* obj : kenv.levelObjects.getClassType<CKGrpPoolSquad>().objects)
			if (kenv.getObjectName(obj) == marioPoolName)
				marioPool = obj->cast<CKGrpPoolSquad>();
		assert(marioPool != nullptr);
		// Replace all pool references to Mario's
		for (CKObject* obj : kenv.levelObjects.getClassType<CKGrpSquadX2>().objects) {
			CKGrpSquadX2* squad = obj->cast<CKGrpSquadX2>();
			for (auto& pool : squad->pools) {
				pool.pool = marioPool;
			}
		}
		kenv.saveLevel(lvlindex);
	}
}

void Test_HexEditor()
{
	KEnvironment kenv;

	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");
	std::string outGamePath = config.Get("XXL-Editor", "outgamepath", gamePath);
	int gameVersion = config.GetInteger("XXL-Editor", "version", 1);
	std::string cfgPlatformName = config.GetString("XXL-Editor", "platform", "KWN");
	bool isRemaster = config.GetBoolean("XXL-Editor", "remaster", false);
	int initlevel = config.GetInteger("XXL-Editor", "initlevel", 8);
	auto itPlatform = std::find_if(std::begin(KEnvironment::platformExt), std::end(KEnvironment::platformExt), [&cfgPlatformName](const char* s) {return _stricmp(s, cfgPlatformName.c_str()) == 0; });
	int platform = (itPlatform != std::end(KEnvironment::platformExt)) ? (itPlatform - std::begin(KEnvironment::platformExt)) : 1;

	kenv.loadGame(gamePath.c_str(), gameVersion, platform, isRemaster);
	kenv.outGamePath = outGamePath;
	kenv.loadLevel(initlevel);

	// Initialize SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	Window window;

	// Initialize graphics renderer
	Renderer* gfx = CreateRendererD3D9(&window);
	// Initialize Dear ImGui
	ImGuiImpl_Init(&window);
	ImGuiImpl_CreateFontsTexture(gfx);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
		auto objnode = [&kenv](CKObject* obj) {
			bool selected = obj == selobject;
			auto pushed = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0), "(%i,%i) %p: %s", obj->getClassCategory(), obj->getClassID(), obj, kenv.getObjectName(obj));
			if (ImGui::IsItemClicked())
				selobject = obj;
			if (pushed)
				ImGui::TreePop();
		};
		if (ImGui::TreeNode("Globals")) {
			for (CKObject* glob : kenv.globalObjects)
				objnode(glob);
			ImGui::TreePop();
		}
		auto walkstr = [&objnode](KObjectList& objlist) {
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
						if (ImGui::TreeNode(&cl, "(%i, %i)", i, clid)) {
							for (CKObject* obj : cl.objects)
								objnode(obj);
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
				int32_t newLength = (int32_t)unkobj->length;
				static const int32_t step = 1;
				bool b = ImGui::InputScalar("Size", ImGuiDataType_U32, &newLength, &step, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue);
				if (b && newLength > 0 && (int32_t)unkobj->length != newLength) {
					void* orimem = unkobj->mem;
					int32_t orilen = (int32_t)unkobj->length;
					void* newmem = malloc(newLength);
					if (newmem) {
						memcpy(newmem, orimem, std::min(orilen, newLength));
						if (newLength - orilen > 0)
							memset((char*)newmem + orilen, 0, newLength - orilen);
						unkobj->mem = newmem;
						unkobj->length = newLength;
						free(orimem);
					}
				}
				if (ImGui::Button("Copy hex")) {
					std::string str = std::string(unkobj->length * 2, 0);
					static const char decimals[17] = "0123456789ABCDEF";
					for (size_t i = 0; i < unkobj->length; i++) {
						str[2 * i] = decimals[((uint8_t*)unkobj->mem)[i] >> 4];
						str[2 * i + 1] = decimals[((uint8_t*)unkobj->mem)[i] & 15];
					}
					ImGui::SetClipboardText(str.c_str());
				}
				memedit.DrawContents(unkobj->mem, unkobj->length);
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

			char* cursor = (char*)unkobj->mem;
			auto getBytes = [&cursor, &unkobj](size_t numBytes) -> char* {
				size_t bytesUsed = cursor - (char*)unkobj->mem;
				if (bytesUsed + numBytes > unkobj->length)
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
				ImGui::TextColored(ImVec4(1,0,0,1), "FAIL");
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

void Test_Diff()
{
	static const char* kdir1 = u8R"(C:\Users\Adrien\Desktop\kthings\romaster_versions\33\XXL1Resources\K)";
	static const char* kdir2 = u8R"(C:\Program Files (x86)\Steam\steamapps\common\Asterix & Obelix XXL 1\XXL1Resources\K)";
	KEnvironment kenv1, kenv2;
	kenv1.loadGame(kdir1, KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC, true);
	kenv2.loadGame(kdir2, KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC, true);
	for (int lvlnum : {1,2,3,4,5,6,7}) {
		kenv1.loadLevel(lvlnum);
		kenv2.loadLevel(lvlnum);
		assert(kenv1.numSectors == kenv2.numSectors);
		for (int strnum = -1; strnum < (int)kenv1.numSectors; strnum++) {
			printf("----- Level %i, Sector %i -----\n", lvlnum, strnum);
			KObjectList& kol1 = (strnum == -1) ? kenv1.levelObjects : kenv1.sectorObjects[strnum];
			KObjectList& kol2 = (strnum == -1) ? kenv2.levelObjects : kenv2.sectorObjects[strnum];
			for (int catnum = 0; catnum < 15; catnum++) {
				auto& kcat1 = kol1.categories[catnum];
				auto& kcat2 = kol2.categories[catnum];
				assert(kcat1.type.size() == kcat2.type.size());
				for (size_t clnum = 0; clnum < kcat1.type.size(); clnum++) {
					auto& kcl1 = kcat1.type[clnum];
					auto& kcl2 = kcat2.type[clnum];
					if (kcl1.objects.size() != kcl2.objects.size()) {
						printf(" * Class (%i, %i) has a different object count! (%zu -> %zu)\n", catnum, clnum, kcl1.objects.size(), kcl2.objects.size());
					}
					else if(false) {
						for (size_t objnum = 0; objnum < kcl1.objects.size(); objnum++) {
							CKUnknown* obj1 = static_cast<CKUnknown*>(kcl1.objects[objnum]);
							CKUnknown* obj2 = static_cast<CKUnknown*>(kcl2.objects[objnum]);
							if (obj1->length != obj2->length) {
								printf(" * Object (%i, %i, %i) has a different size! (%zi, %zu -> %zu bytes)\n", catnum, clnum, objnum, obj2->length - obj1->length, obj1->length, obj2->length);
							}
							else {
								const char* mem1 = (const char*)obj1->mem;
								const char* mem2 = (const char*)obj2->mem;
								const size_t len = obj1->length;
								size_t numChanges = 0;
								for (size_t i = 0; i < len; i++)
									if (mem1[i] != mem2[i])
										numChanges++;
								if (numChanges != 0)
									printf(" * Object (%i, %i, %i) has same size but different content! (%zu bytes are different!)\n", catnum, clnum, objnum, numChanges);
							}
						}
					}
				}
			}
		}
	}
	getchar();
}

void Tests::TestPrompt()
{
	static const std::pair<void(*)(), const char*> tests[] = {
		{Test_Cpnt2CSV, "Component values to CSV"},
		{Test_LevelSizeCheck, "Level size check"},
		{Test_ChoreoFreqValues, "Chereokey value frequency"},
		{Test_extractRemasterFilepaths, "Extract Remaster model filepaths"},
		{Test_MarioDifficulty, "Extreme Mario Mode"},
		{Test_HexEditor, "Hex Editor"},
		{Test_Diff, "LVL/STR file diff"},
	};
	const size_t numTests = std::size(tests);
	printf("Available tests:\n");
	for (size_t t = 0; t < numTests; t++) {
		printf("%3i. %s\n", t, tests[t].second);
	}
	printf("Run which test number? ");
	char input[32];
	gets_s(input);
	size_t t = atoi(input);
	if (t < numTests)
		tests[t].first();
}

#endif