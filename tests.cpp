#ifndef XEC_RELEASE

#include "tests.h"
#include <string>
#include <INIReader.h>
#include "KEnvironment.h"
#include "DynArray.h"
#include "Events.h"
#include "ClassRegister.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGeometry.h"
#include "rw.h"
#include "CoreClasses/CKGroup.h"
#include <cassert>
#include "GameClasses/CKGameX1.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKNode.h"
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <queue>

#include "window.h"
#include "SDL2/SDL.h"
#include "renderer.h"
#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "HexEditor.h"

void Test_Cpnt2CSV() {
	using namespace GameX1;
	using CpntType = CKBasicEnemyCpnt;
	
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	kenv.addFactory<CpntType>();
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
		void reflect(MarkerIndex& ref, const char* name) override { write(name); };
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
		void reflect(EventNode& ref, const char* name, CKObject* user) override { fprintf(csv, "Event\t"); };
		void reflect(MarkerIndex& ref, const char* name) override { fprintf(csv, "Marker\t"); };
		void reflect(std::string& ref, const char* name) override { abort(); } // TODO
	};

	FILE* csv;
	fopen_s(&csv, "EnemyCpnts.txt", "w");
	NameListener nl(csv);
	ValueListener vl(csv);

	bool headerDone = false;
	for (int lvl = 1; lvl <= 6; lvl++) {
		kenv.loadLevel(lvl);
		CpntType* firstcpnt = kenv.levelObjects.getFirst<CpntType>();
		if (!headerDone && firstcpnt) {
			fprintf(csv, "Level\tIndex\t");
			firstcpnt->reflectMembers2(nl, &kenv);
			headerDone = true;
		}
		int index = 0;
		for (CKObject* obj : kenv.levelObjects.getClassType<CpntType>().objects) {
			fprintf(csv, "\n%i\t%i\t", lvl, index);
			obj->cast<CpntType>()->reflectMembers2(vl, &kenv);
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
	ClassRegister::registerClassesForXXL2PlusPC(kenv);
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
			for (auto& pool : squad->fightData.pools) {
				pool.pool = marioPool;
			}
		}
		kenv.saveLevel(lvlindex);
	}
}

void Test_HexEditor()
{
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");
	std::string outGamePath = config.Get("XXL-Editor", "outgamepath", gamePath);
	int gameVersion = config.GetInteger("XXL-Editor", "version", 1);
	std::string cfgPlatformName = config.GetString("XXL-Editor", "platform", "KWN");
	bool isRemaster = config.GetBoolean("XXL-Editor", "remaster", false);
	int initlevel = config.GetInteger("XXL-Editor", "initlevel", 8);
	auto itPlatform = std::find_if(std::begin(KEnvironment::platformExt), std::end(KEnvironment::platformExt), [&cfgPlatformName](const char* s) {return _stricmp(s, cfgPlatformName.c_str()) == 0; });
	int platform = (itPlatform != std::end(KEnvironment::platformExt)) ? (itPlatform - std::begin(KEnvironment::platformExt)) : 1;

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

	HexEditorUI(gamePath, outGamePath, gameVersion, platform, isRemaster, initlevel, window, gfx);
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
							if (obj1->mem.size() != obj2->mem.size()) {
								printf(" * Object (%i, %i, %i) has a different size! (%zi, %zu -> %zu bytes)\n", catnum, clnum, objnum, obj2->mem.size() - obj1->mem.size(), obj1->mem.size(), obj2->mem.size());
							}
							else {
								const char* mem1 = (const char*)obj1->mem.data();
								const char* mem2 = (const char*)obj2->mem.data();
								const size_t len = obj1->mem.size();
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

struct BoundingShapeToHookMemberMapGenerator : NamedMemberListener
{
	std::map<int, std::string> classNames;
	std::map<CKBoundingShape*, std::pair<int, std::string>> info;
	CKObject* currentHook;

	void reflect(uint8_t& ref, const char* name) override {}
	void reflect(uint16_t& ref, const char* name) override {}
	void reflect(uint32_t& ref, const char* name) override {}
	void reflect(float& ref, const char* name) override {}
	void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override {
		if (!ref)
			return;
		CKBoundingShape* shape = ref.get()->dyncast<CKBoundingShape>();
		if (!shape)
			return;
		registerShape(currentHook, shape, getFullName(name));
	}
	void reflect(Vector3& ref, const char* name) override {}
	void reflect(EventNode& ref, const char* name, CKObject* user) override {}
	void reflect(MarkerIndex& ref, const char* name) override {}
	void reflect(std::string& ref, const char* name) override {}

	void registerShape(CKObject* owner, CKBoundingShape* shape, std::string name) {
		const int classFid = owner->getClassFullID();
		auto& className = classNames[classFid];
		if (className.empty()) {
			className = owner->getClassName();
		}
		if (info.contains(shape)) {
			// There should not be different owner and member for the same bounding volume node.
			// Exception for CKHkJetPackRoman, where hjpUnk0[3] and hjpUnk0[4] point to the same node.
			if (classFid != GameX1::CKHkJetPackRoman::FULL_ID) {
				assert(info[shape] == std::make_pair(classFid, name));
			}
			return;
		}
		info[shape] = { classFid, std::move(name) };
	}
};

void Test_ExtractCollisionTests()
{
	std::tuple<const char*, int, int, bool, std::vector<int>> games[] = {
		{ R"(C:\Users\Adrien\Desktop\kthings\xxl1_mp_orig)", KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC, false, { 1,2,3,4,5,6 } },
		{ R"(C:\Users\Adrien\Desktop\kthings\XXL2_DRM_OUT)", KEnvironment::KVERSION_XXL2, KEnvironment::PLATFORM_PC, false, { 1,2,3,4,6,7,8,9,10,11 } },
	};

	for (const auto& [gamePath, gameVersion, gamePlatform, gameIsRemaster, gameLevels] : games) {
		KEnvironment kenv;
		ClassRegister::registerClasses(kenv, gameVersion, gamePlatform, gameIsRemaster);
		kenv.loadGame(gamePath, gameVersion, gamePlatform, gameIsRemaster);

		using TestKey = nlohmann::ordered_json;
		struct TestInfo {
			bool wasActive = false, wasInactive = false;
			std::map<TestKey, TestInfo> children;
		};
		std::map<TestKey, TestInfo> commonTests;

		for (int levelNumber : gameLevels) {
			kenv.loadLevel(levelNumber);
			CKSrvCollision* srvCollision = kenv.levelObjects.getFirst<CKSrvCollision>();

			BoundingShapeToHookMemberMapGenerator bsthmmg;
			auto reflectAs = [&]<typename T>() {
				for (const auto& cltype : kenv.levelObjects.categories[T::CATEGORY].type) {
					for (CKObject* obj : cltype.objects) {
						bsthmmg.currentHook = obj;
						if (auto* tobj = obj->dyncast<T>())
							tobj->virtualReflectMembers(bsthmmg, &kenv);
					}
				}
			};
			reflectAs.template operator() < CKReflectableService > ();
			reflectAs.template operator() < CKHook > ();
			reflectAs.template operator() < CKGroup > ();
			reflectAs.template operator() < CKComponent > ();
			reflectAs.template operator() < CKReflectableLogic > ();

			bsthmmg.classNames[CKCrateCpnt::FULL_ID] = "CKCrateCpnt";
			for (auto* objCrateCpnt : kenv.levelObjects.getClassType<CKCrateCpnt>().objects) {
				auto* crateCpnt = objCrateCpnt->cast<CKCrateCpnt>();
				for (CKSceneNode* node = crateCpnt->crateNode.get(); node != nullptr; node = node->next.get()) {
					auto* branch = node->cast<CSGBranch>();
					auto* box = branch->child->cast<CKOBB>();
					bsthmmg.registerShape(crateCpnt, box, "!crateCloneOBB");
				}
			}

			for (const auto& cltype : kenv.levelObjects.categories[CKHook::CATEGORY].type) {
				for (CKObject* obj : cltype.objects) {
					CKHook* hook = obj->cast<CKHook>();
					if (!hook->node.get()) {
						continue;
					}
					auto walk = [&](CKSceneNode* node, std::string str, const auto& rec) -> void {
						if (CKBoundingShape* shape = node->dyncast<CKBoundingShape>()) {
							if (!bsthmmg.info.contains(shape))
								bsthmmg.registerShape(hook, shape, str);
						}
						if (CSGBranch* branch = node->dyncast<CSGBranch>()) {
							int index = 0;
							for (CKSceneNode* child = branch->child.get(); child != nullptr; child = child->next.get()) {
								rec(child, str + '.' + std::to_string(index++), rec);
							}
						}
						};
					walk(hook->node.get(), "@", walk);
				}
			}

			//fmt::println("-------------------");
			//for (const auto& [shape, set] : bsthmmg.info) {
			//	fmt::print("{}: ", fmt::ptr(shape));
			//	for (const auto& [objClassName, objMemberName] : set)
			//		fmt::print("{}::{}", objClassName, objMemberName);
			//	fmt::print("\n");
			//}

			// ===== construct active and inactive sets =====
			std::set<const CKSrvCollision::CollisionTest*> activeTests;
			std::queue<uint16_t> nextToVisit;
			nextToVisit.push(srvCollision->activeList);
			while (!nextToVisit.empty()) {
				uint16_t index = nextToVisit.front();
				nextToVisit.pop();

				auto& test = srvCollision->collisionTests[index];
				activeTests.insert(&test);

				if (test.aa[0] != 0xFFFF && !activeTests.contains(&srvCollision->collisionTests[test.aa[0]]))
					nextToVisit.push(test.aa[0]);
				//if (test.aa[1] != 0xFFFF && !activeTests.contains(&srvCollision->collisionTests[test.aa[1]]))
				//	nextToVisit.push(test.aa[1]);
			}

			std::set<const CKSrvCollision::CollisionTest*> inactiveTests;
			nextToVisit = {};
			nextToVisit.push(srvCollision->inactiveList);
			while (!nextToVisit.empty()) {
				uint16_t index = nextToVisit.front();
				nextToVisit.pop();

				auto& test = srvCollision->collisionTests[index];
				//assert((test.flags & 3) != 1); // no parents in inactive list
				//assert(test.aa[1] == 0xFFFF);
				inactiveTests.insert(&test);

				if (test.aa[0] != 0xFFFF && !inactiveTests.contains(&srvCollision->collisionTests[test.aa[0]]))
					nextToVisit.push(test.aa[0]);
			}

			for (const auto& test : srvCollision->collisionTests) {
				assert(activeTests.contains(&test) xor inactiveTests.contains(&test));
			}

			// ==========

			auto identifyTest = [&](const CKSrvCollision::CollisionTest& test) -> TestKey {
				TestKey info;
				auto it1 = bsthmmg.info.find(test.shapeNode1->cast<CKBoundingShape>());
				auto it2 = bsthmmg.info.find(test.shapeNode2->cast<CKBoundingShape>());
				if (it1 != bsthmmg.info.end() && it2 != bsthmmg.info.end()) {
					int testType = test.flags & 3;
					const char* testTypeName = (testType == 0) ? "SINGLE" : (testType == 1) ? "PARENT" : (testType == 2) ? "CHILD" : "?";
					bool active = test.flags & 4;
					info["obj1"] = it1->second;
					info["obj2"] = it2->second;
					//info["type"] = testTypeName;
					//info["active"] = activeTests.contains(&test);
				}
				return info;
				};

			int found = 0, notFound = 0, partiallyFound = 0;
			int testIndex = 0;
			for (auto& test : srvCollision->collisionTests) {
				auto it1 = bsthmmg.info.find(test.shapeNode1->cast<CKBoundingShape>());
				auto it2 = bsthmmg.info.find(test.shapeNode2->cast<CKBoundingShape>());
				if (it1 != bsthmmg.info.end() && it2 != bsthmmg.info.end()) {
					int testType = test.flags & 3;
					if (testType == 2) { // CHILD
						assert(test.aa[2] != 0xFFFF);
						const auto& parentTest = srvCollision->collisionTests[test.aa[2]];
						auto& parentInfo = commonTests[identifyTest(parentTest)];
						(activeTests.contains(&parentTest) ? parentInfo.wasActive : parentInfo.wasInactive) = true;
						auto& childInfo = parentInfo.children[identifyTest(test)];
						(activeTests.contains(&test) ? childInfo.wasActive : childInfo.wasInactive) = true;
					}
					else {
						auto& info = commonTests[identifyTest(test)];
						(activeTests.contains(&test) ? info.wasActive : info.wasInactive) = true;
					}
					found += 1;
				}
				else if (it1 == bsthmmg.info.end() && it2 == bsthmmg.info.end()) {
					notFound += 1;
				}
				else {
					partiallyFound += 1;
					if (it1 != bsthmmg.info.end()) {
						fmt::println("!! {} !! it2 missing, it1: {}::{}", testIndex, it1->second.first, it1->second.second);
					}
					if (it2 != bsthmmg.info.end()) {
						fmt::println("!! {} !! it1 missing, it2: {}::{}", testIndex, it2->second.first, it2->second.second);
					}
				}
				testIndex += 1;
			}

			fmt::println("--- XXL{} LEVEL {}: --------------", gameVersion, levelNumber);
			fmt::println("          Found: {}", found);
			fmt::println("      Not found: {}", notFound);
			fmt::println("Partially found: {}", partiallyFound);
		}

		nlohmann::ordered_json js;
		auto& jsTestList = js["collisionTests"];
		for (const auto& [parentKey, parentInfo] : commonTests) {
			auto jsParent = parentKey;
			jsParent["active"] = !parentInfo.wasInactive;
			if (!parentInfo.children.empty()) {
				auto& jsChildList = jsParent["children"];
				for (const auto& [childKey, childInfo] : parentInfo.children) {
					auto jsChild = childKey;
					jsChild["active"] = !childInfo.wasInactive;
					jsChildList.push_back(std::move(jsChild));
				}
			}
			jsTestList.push_back(std::move(jsParent));
		}

		std::filesystem::path baseFileName = fmt::format("C:\\Users\\Adrien\\Desktop\\kthings\\NodeCollisionTestInfo_{}.json", gameVersion);
		{
			IOFile file(baseFileName.c_str(), "wb");
			std::string buf = js.dump();
			file.write(buf.data(), buf.size());
		}
		{
			baseFileName.replace_extension(".bson");
			IOFile file(baseFileName.c_str(), "wb");
			auto buf = nlohmann::json::to_bson(js);
			file.write(buf.data(), buf.size());
		}
		{
			baseFileName.replace_extension(".cbor");
			IOFile file(baseFileName.c_str(), "wb");
			auto buf = nlohmann::json::to_cbor(js);
			file.write(buf.data(), buf.size());
		}
	}

	fmt::println("Success! Press key to quit.");
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
		{Test_ExtractCollisionTests, "Extract Collision Tests"},
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