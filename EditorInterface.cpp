#include "EditorInterface.h"
#include "KEnvironment.h"
#include "imgui/imgui.h"
#include "imguiimpl.h"
#include <SDL2/SDL.h>
#include <Windows.h>
#include <commdlg.h>
#include "rwrenderer.h"
#include "CKDictionary.h"
#include "main.h"
#include "CKNode.h"
#include "CKGraphical.h"
#include "CKLogic.h"
#include "CKComponent.h"
#include "CKGroup.h"
#include "CKHook.h"
#include <shlobj_core.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "rwext.h"
#include <stack>

namespace {
	void InvertTextures(KEnvironment &kenv)
	{
		auto f = [](KObjectList &objlist) {
			CTextureDictionary *dict = (CTextureDictionary*)objlist.getClassType<CTextureDictionary>().objects[0];
			for (auto &tex : dict->textures) {
				if (uint32_t *pal = tex.image.palette.data())
					for (size_t i = 0; i < (1 << tex.image.bpp); i++)
						pal[i] ^= 0xFFFFFF;
			}
		};
		f(kenv.levelObjects);
		for (KObjectList &ol : kenv.sectorObjects)
			f(ol);
	}

	void DrawSceneNode(CKSceneNode *node, const Matrix &transform, Renderer *gfx, ProGeoCache &geocache, ProTexDict *texdict, bool showTextures = true)
	{
		if (!node)
			return;
		Matrix nodeTransform = node->transform;
		nodeTransform.m[0][3] = nodeTransform.m[1][3] = nodeTransform.m[2][3] = 0.0f;
		nodeTransform.m[3][3] = 1.0f;
		Matrix globalTransform = nodeTransform * transform;
		if (node->isSubclassOf<CNode>()) {
			gfx->setTransformMatrix(globalTransform);
			for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
				if (RwMiniClump *rwminiclp = kgeo->clump)
					if (RwGeometry *rwgeo = rwminiclp->atomic.geometry.get())
						geocache.getPro(rwgeo, texdict)->draw(showTextures);
			}
		}
		if (node->isSubclassOf<CSGBranch>())
			DrawSceneNode(node->cast<CSGBranch>()->child.get(), globalTransform, gfx, geocache, texdict, showTextures);
		DrawSceneNode(node->next.get(), transform, gfx, geocache, texdict, showTextures);
	}
}

EditorInterface::EditorInterface(KEnvironment & kenv, Window * window, Renderer * gfx)
	: kenv(kenv), g_window(window), gfx(gfx), protexdict(gfx), progeocache(gfx)
{
	lastFpsTime = SDL_GetTicks() / 1000;
}

void EditorInterface::prepareLevelGfx()
{
	protexdict.reset(kenv.levelObjects.getObject<CTextureDictionary>(0));
	str_protexdicts.clear();
	str_protexdicts.reserve(kenv.numSectors);
	for (int i = 0; i < kenv.numSectors; i++) {
		ProTexDict strptd(gfx, kenv.sectorObjects[i].getObject<CTextureDictionary>(0));
		strptd._next = &protexdict;
		str_protexdicts.push_back(std::move(strptd));
		//printf("should be zero: %i\n", strptd.dict.size());
	}
}

void EditorInterface::iter()
{
	framesInSecond++;
	uint32_t sec = SDL_GetTicks() / 1000;
	if (sec != lastFpsTime) {
		lastFps = framesInSecond;
		framesInSecond = 0;
		lastFpsTime = sec;
	}

	camera.aspect = (float)g_window->getWidth() / g_window->getHeight();
	camera.updateMatrix();
	float camspeed = _camspeed;
	if (ImGui::GetIO().KeyShift)
		camspeed *= 0.5f;
	Vector3 camside = camera.direction.cross(Vector3(0, 1, 0));
	if (g_window->getKeyDown(SDL_SCANCODE_UP))
		camera.position += camera.direction * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_DOWN))
		camera.position -= camera.direction * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_RIGHT))
		camera.position += camside * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_LEFT))
		camera.position -= camside * camspeed;

	static bool rotating = false;
	static int rotStartX, rotStartY;
	static Vector3 rotOrigOri;
	if (g_window->getKeyDown(SDL_SCANCODE_KP_0) || g_window->getMouseDown(SDL_BUTTON_LEFT)) {
		if (!rotating) {
			rotStartX = g_window->getMouseX();
			rotStartY = g_window->getMouseY();
			rotOrigOri = camera.orientation;
			rotating = true;
			SDL_CaptureMouse(SDL_TRUE);
		}
		int dx = g_window->getMouseX() - rotStartX;
		int dy = g_window->getMouseY() - rotStartY;
		camera.orientation = rotOrigOri + Vector3(-dy * 0.01f, dx*0.01f, 0);
	}
	else {
		rotating = false;
		SDL_CaptureMouse(SDL_FALSE);
	}

	camera.updateMatrix();

	ImGui::Begin("Main");
	ImGui::Text("Hello to all people from Stinkek's server!");
	ImGui::Text("FPS: %i", lastFps);
	ImGui::BeginTabBar("MainTabBar", 0);
	static int levelNum = 0;
	if (ImGui::BeginTabItem("Main")) {
		ImGui::Text("Hello!");
		ImGui::InputInt("Level number##LevelNum", &levelNum);
		if (ImGui::Button("Load")) {
			kenv.loadLevel(levelNum);
			prepareLevelGfx();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			kenv.saveLevel(levelNum);
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Textures")) {
		CTextureDictionary *texDict = kenv.levelObjects.getObject<CTextureDictionary>(0);
		if (selTexID >= texDict->textures.size())
			selTexID = texDict->textures.size() - 1;
		if (ImGui::Button("Insert")) {
			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = NULL;
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG;\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_FILEMUSTEXIST;
			if (GetOpenFileNameA(&ofn)) {
				printf("%s\n", filepath);
				AddTexture(kenv, filepath);
				protexdict.reset(texDict);
			}
			else printf("GetOpenFileName fail: 0x%X\n", CommDlgExtendedError());
		}
		ImGui::SameLine();
		if ((selTexID != -1) && ImGui::Button("Replace")) {
			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = NULL;
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG;\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_FILEMUSTEXIST;
			if (GetOpenFileNameA(&ofn)) {
				printf("%s\n", filepath);
				texDict->textures[selTexID].image = RwImage::loadFromFile(filepath);
				protexdict.reset(texDict);
			}
			else printf("GetOpenFileName fail: 0x%X\n", CommDlgExtendedError());
		}
		ImGui::SameLine();
		if ((selTexID != -1) && ImGui::Button("Remove")) {
			texDict->textures.erase(texDict->textures.begin() + selTexID);
			protexdict.reset(texDict);
		}
		ImGui::SameLine();
		if (ImGui::Button("Invert all")) {
			InvertTextures(kenv);
			protexdict.reset(texDict);
			for (auto &sd : str_protexdicts)
				sd.reset(texDict);
		}
		ImGui::SameLine();
		if (ImGui::Button("Export all")) {
			char dirname[MAX_PATH+1], pname[MAX_PATH+1];
			BROWSEINFOA bri;
			memset(&bri, 0, sizeof(bri));
			bri.pszDisplayName = dirname;
			bri.lpszTitle = "Export all the textures to folder:";
			bri.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
			PIDLIST_ABSOLUTE pid = SHBrowseForFolderA(&bri);
			if (pid != NULL) {
				SHGetPathFromIDListA(pid, dirname);
				printf("%s\n", dirname);
				for (auto &tex : texDict->textures) {
					sprintf_s(pname, "%s/%s.png", dirname, tex.name);
					RwImage cimg = tex.image.convertToRGBA32();
					stbi_write_png(pname, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
				}
			}
		}
		ImGui::Columns(2);
		ImGui::BeginChild("TexSeletion");
		int i = 0;
		for (auto &tex : texDict->textures) {
			ImGui::PushID(i);
			if (ImGui::Selectable("##texsel", i == selTexID, 0, ImVec2(0,32))) {
				selTexID = i;
			}
			ImGui::SameLine();
			ImGui::Image(protexdict.find(texDict->textures[i].name).second, ImVec2(32, 32));
			ImGui::SameLine();
			ImGui::Text("%s\n%i*%i*%i", tex.name, tex.image.width, tex.image.height, tex.image.bpp);
			ImGui::PopID();
			i++;
		}
		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::BeginChild("TexViewer", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (selTexID != -1) {
			auto &tex = texDict->textures[selTexID];
			ImGui::Image(protexdict.find(tex.name).second, ImVec2(tex.image.width, tex.image.height));
		}
		ImGui::EndChild();
		ImGui::Columns();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Geo")) {
		ImGui::DragFloat3("Cam pos", &camera.position.x, 0.1f);
		ImGui::DragFloat3("Cam ori", &camera.orientation.x, 0.1f);
		ImGui::DragFloat("Cam speed", &_camspeed, 0.1f);
		ImGui::Checkbox("Show textures", &showTextures);
		ImGui::Checkbox("Beacons", &showBeacons); ImGui::SameLine();
		ImGui::Checkbox("Beacon kluster bounds", &showBeaconKlusterBounds); //ImGui::SameLine();
		ImGui::Checkbox("Sas bounds", &showSasBounds);
		ImGui::Separator();
		ImGui::DragFloat3("Geo pos", &selgeoPos.x, 0.1f);
		if (ImGui::Button("Move geo to front"))
			selgeoPos = camera.position + camera.direction * 3;
		ImGui::SameLine();
		if (ImGui::Button("Import DFF")) {
			//HWND hWindow = (HWND)g_window->getNativeWindow();
			HWND hWindow = NULL;

			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWindow;
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Renderware Clump\0*.DFF\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = "dff";
			if (GetOpenFileNameA(&ofn)) {
				printf("%s\n", filepath);

				RwClump *impClump = LoadDFF(filepath); //"C:\\Users\\Adrien\\Desktop\\kthings\\xecpp_dff_test\\GameCube Hat\\gamecube.blend.dff"
				//cloneManager->_teamDict._bings[39]._clump->atomic.geometry = std::unique_ptr<RwGeometry>(new RwGeometry(*pyra->geoList.geometries[0]));
				*selGeometry = *impClump->geoList.geometries[0];
				progeocache.dict.clear();
			}
			else printf("GetOpenFileName fail: 0x%X\n", CommDlgExtendedError());
		}

		ImGui::BeginChild("RwGeoSelection");
		auto enumRwGeo = [this](RwGeometry *rwgeo, int i) {
			std::string fndname = "?";
			if (rwgeo->materialList.materials.size())
				fndname = rwgeo->materialList.materials[0].texture.name;
			ImGui::PushID(i);
			if (ImGui::Selectable("##rwgeo", selGeometry == rwgeo)) {
				selGeometry = rwgeo;
			}
			ImGui::SameLine();
			ImGui::Text("%i (%s)", i, fndname.c_str());
			ImGui::PopID();
		};
		for (int j = 1; j <= 3; j++) {
			static const std::array<const char*, 3> geotypenames = { "Particle geometries", "Geometries", "Skinned geometries" };
			ImGui::PushID(j);
			if (ImGui::TreeNode(geotypenames[j - 1])) {
				int i = 0;
				for (CKObject *obj : kenv.levelObjects.getClassType(10, j).objects) {
					if (RwMiniClump *clp = ((CKAnyGeometry*)obj)->clump)
						enumRwGeo(clp->atomic.geometry.get(), i);
					i++;
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (kenv.levelObjects.getClassType<CCloneManager>().objects.size()) {
			CCloneManager *cloneManager = kenv.levelObjects.getObject<CCloneManager>(0);
			if (cloneManager->_numClones > 0) {
				if (ImGui::TreeNode("Clones")) {
					int i = 0;
					ImGui::PushID("Clones");
					for (auto &bing : cloneManager->_teamDict._bings) {
						enumRwGeo(bing._clump->atomic.geometry.get(), i++);
					}
					ImGui::PopID();
					ImGui::TreePop();
				}
			}
		}
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Scene graph")) {
		ImGui::Columns(2);
		ImGui::BeginChild("SceneNodeTree");
		IGSceneGraph();
		ImGui::EndChild();
		ImGui::NextColumn();
		ImGui::BeginChild("SceneNodeProperties");
		IGSceneNodeProperties();
		ImGui::EndChild();
		ImGui::Columns();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Beacons")) {
		auto enumBeaconKluster = [this](CKBeaconKluster* bk) {
			if (ImGui::TreeNode(bk, "%f %f %f | %f %f", bk->bounds[0], bk->bounds[1], bk->bounds[2], bk->bounds[3], bk->bounds[4])) {
				ImGui::DragFloat3("Center##beaconKluster", &bk->bounds[0], 0.1f);
				if (ImGui::DragFloat("Radius##beaconKluster", &bk->bounds[3], 0.1f))
					bk->bounds[4] = bk->bounds[3] * bk->bounds[3];
				for (auto &bing : bk->bings) {
					for (auto &beacon : bing.beacons) {
						ImGui::PushID(&beacon);
						Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
						bool tn_open = ImGui::TreeNodeEx("beacon", ImGuiTreeNodeFlags_OpenOnArrow, "(%i,%i) %f %f %f 0x%04X", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z, beacon.params);
						//if (ImGui::Selectable("##beacon")) {
						if (ImGui::IsItemClicked()) {
							camera.position = pos - camera.direction * 5.0f;
						}
						if (tn_open) {
							ImGui::DragScalarN("Position##beacon", ImGuiDataType_S16, &beacon.posx, 3, 0.1f);
							ImGui::InputScalar("Params##beacon", ImGuiDataType_U16, &beacon.params, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
							ImGui::TreePop();
						}
						//ImGui::SameLine();
						//ImGui::Text("(%i,%i) %f %f %f", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z);
						ImGui::PopID();
					}
				}
				ImGui::TreePop();
			}
		};
		if (ImGui::TreeNode("Level")) {
			for (CKBeaconKluster *bk = kenv.levelObjects.getObject<CKBeaconKluster>(0); bk; bk = bk->nextKluster.get())
				enumBeaconKluster(bk);
			ImGui::TreePop();
		}
		int i = 0;
		for (auto &str : kenv.sectorObjects) {
			if (ImGui::TreeNode(&str, "Sector %i", i)) {
				if (str.getClassType<CKBeaconKluster>().objects.size())
					for (CKBeaconKluster *bk = str.getObject<CKBeaconKluster>(0); bk; bk = bk->nextKluster.get())
						enumBeaconKluster(bk);
				ImGui::TreePop();
			}
			i++;
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Objects")) {
		static const char *catnames[15] = { "Managers", "Services", "Hooks",
			"Hook Lives", "Groups", "Group Lives", "Components", "Camera",
			"Cinematic blocs", "Dictionaries", "Geometries", "Scene nodes",
			"Logic stuff", "Graphical stuff", "Errors"
		};
		auto enumObjList = [this](KObjectList &objlist) {
			for (int i = 0; i < 15; i++) {
				if (ImGui::TreeNode(catnames[i])) {
					for (auto &cl : objlist.categories[i].type) {
						int n = 0;
						for (CKObject *obj : cl.objects) {
							if (ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%s (%i, %i) %i, refCount=%i", obj->getClassName(), obj->getClassCategory(), obj->getClassID(), n, obj->refCount))
								ImGui::TreePop();
							n++;
						}
					}
					ImGui::TreePop();
				}
			}
		};
		if (ImGui::TreeNode("Level")) {
			enumObjList(kenv.levelObjects);
			ImGui::TreePop();
		}
		int i = 0;
		for (auto &str : kenv.sectorObjects) {
			if (ImGui::TreeNode(&str, "Sector %i", i)) {
				enumObjList(str);
				ImGui::TreePop();
			}
			i++;
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Misc")) {
		ImGui::Checkbox("Show ImGui Demo", &showImGuiDemo);
		if (ImGui::CollapsingHeader("Unknown classes")) {
			for (auto &cl : CKUnknown::hits) {
				ImGui::BulletText("%i %i", cl.first, cl.second);
			}
		}
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();

	ImGui::End();

	if (showImGuiDemo)
		ImGui::ShowDemoWindow(&showImGuiDemo);

}

void EditorInterface::render()
{
	gfx->initModelDrawing();
	if (selGeometry) {
		gfx->setTransformMatrix(Matrix::getTranslationMatrix(selgeoPos) * camera.sceneMatrix);
		progeocache.getPro(selGeometry, &protexdict)->draw();
	}

	CSGSectorRoot *rootNode = kenv.levelObjects.getObject<CSGSectorRoot>(0);
	DrawSceneNode(rootNode, camera.sceneMatrix, gfx, progeocache, &protexdict, showTextures);
	for (int str = 0; str < kenv.numSectors; str++) {
		CSGSectorRoot * strRoot = kenv.sectorObjects[str].getObject<CSGSectorRoot>(0);
		DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, &str_protexdicts[str], showTextures);
	}

	auto drawBox = [this](const Vector3 &a, const Vector3 &b) {
		Vector3 _b1(a.x, a.y, a.z);
		Vector3 _b2(a.x, a.y, b.z);
		Vector3 _b3(b.x, a.y, b.z);
		Vector3 _b4(b.x, a.y, a.z);
		Vector3 _t1(a.x, b.y, a.z);
		Vector3 _t2(a.x, b.y, b.z);
		Vector3 _t3(b.x, b.y, b.z);
		Vector3 _t4(b.x, b.y, a.z);
		gfx->drawLine3D(_b1, _b2);
		gfx->drawLine3D(_b2, _b3);
		gfx->drawLine3D(_b3, _b4);
		gfx->drawLine3D(_b4, _b1);
		gfx->drawLine3D(_t1, _t2);
		gfx->drawLine3D(_t2, _t3);
		gfx->drawLine3D(_t3, _t4);
		gfx->drawLine3D(_t4, _t1);
		gfx->drawLine3D(_b1, _t1);
		gfx->drawLine3D(_b2, _t2);
		gfx->drawLine3D(_b3, _t3);
		gfx->drawLine3D(_b4, _t4);
	};

	CCloneManager *clm = kenv.levelObjects.getFirst<CCloneManager>();
	auto getCloneIndex = [this,clm](CClone *node) {
		auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const objref<CClone> &ref) {return ref.get() == node; });
		assert(it != clm->_clones.end());
		size_t clindex = it - clm->_clones.begin();
		return clindex;
	};
	auto drawClone = [this, clm](size_t clindex) {
		for (uint32_t part : clm->_team.dongs[clindex].bongs)
			if (part != 0xFFFFFFFF) {
				RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump->atomic.geometry.get();
				progeocache.getPro(rwgeo, &protexdict)->draw();
			}
	};

	auto drawBeaconKluster = [this,clm,&getCloneIndex,&drawClone,&drawBox](CKBeaconKluster* bk) {
		Vector3 center(bk->bounds[0], bk->bounds[1], bk->bounds[2]);
		if (showBeaconKlusterBounds) {
			gfx->setTransformMatrix(camera.sceneMatrix);
			gfx->unbindTexture(0);
			float rd = bk->bounds[3];
			float hh = bk->bounds[4];
			drawBox(center + Vector3(rd, rd, rd), center - Vector3(rd, rd, rd));
		}
		if (showBeacons) {
			for (auto &bing : bk->bings) {
				if (!bing.active)
					continue;
				uint32_t handlerID = bing.handler->getClassFullID();
				for (auto &beacon : bing.beacons) {
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					if (handlerID == CKCrateCpnt::FULL_ID) {
						int numCrates = beacon.params & 7;

						CKCrateCpnt *cratecpnt = bing.handler->cast<CKCrateCpnt>();
						size_t clindex = getCloneIndex(cratecpnt->crateNode->cast<CClone>());

						for (int c = 0; c < numCrates; c++) {
							gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos + Vector3(0, 0.5f + c, 0)) * camera.sceneMatrix);
							drawClone(clindex);
						}
					}
					else if (handlerID == CKGrpAsterixBonusPool::FULL_ID) {
						CKGrpAsterixBonusPool *pool = bing.handler->cast<CKGrpAsterixBonusPool>();
						CKHkBasicBonus *hook = pool->childHook->cast<CKHkBasicBonus>();

						size_t clindex = getCloneIndex(hook->node->cast<CClone>());

						// rotation
						Matrix rotmat = Matrix::getRotationYMatrix(SDL_GetTicks() * 3.1415f / 1000.0f);

						gfx->setTransformMatrix(rotmat * Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
						drawClone(clindex);
					}
					else if (selGeometry) {
						gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
						progeocache.getPro(selGeometry, &protexdict)->draw();
					}
				}
			}
		}
	};
	for (CKBeaconKluster *bk = kenv.levelObjects.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
		drawBeaconKluster(bk);
	for (auto &str : kenv.sectorObjects)
		if(str.getClassType<CKBeaconKluster>().objects.size())
			for (CKBeaconKluster *bk = str.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
				drawBeaconKluster(bk);

	if (showSasBounds) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		for (CKObject *obj : kenv.levelObjects.getClassType<CKSas>().objects) {
			CKSas *sas = (CKSas*)obj;
			for (auto &box : sas->boxes) {
				drawBox(box.highCorner, box.lowCorner);
			}
		}
	}
}

void EditorInterface::IGEnumNode(CKSceneNode *node, const char *description)
{
	if (!node)
		return;
	bool hassub = false;
	if (node->isSubclassOf<CSGBranch>())
		hassub = node->cast<CSGBranch>()->child.get();
	bool open = ImGui::TreeNodeEx(node, hassub ? 0 : ImGuiTreeNodeFlags_Leaf, "%s %s", node->getClassName(), description);
	if (ImGui::IsItemClicked()) {
		selNode = node;
	}
	if (open) {
		if (hassub) {
			CSGBranch *branch = node->cast<CSGBranch>();
			IGEnumNode(branch->child.get());
		}
		ImGui::TreePop();
	}
	IGEnumNode(node->next.get());
}

void EditorInterface::IGSceneGraph()
{
	CSGSectorRoot *lvlroot = kenv.levelObjects.getObject<CSGSectorRoot>(0);
	IGEnumNode(lvlroot, "Commun sector");
	for (int i = 0; i < kenv.numSectors; i++) {
		CSGSectorRoot *strroot = kenv.sectorObjects[i].getObject<CSGSectorRoot>(0);
		char buf[40];
		sprintf_s(buf, "Sector %i", i);
		IGEnumNode(strroot, buf);
	}
}

void EditorInterface::IGSceneNodeProperties()
{
	if (!selNode) {
		ImGui::Text("No node selected!");
		return;
	}
	ImGui::DragFloat3("Position", &selNode->transform._41, 0.1f);
	if (ImGui::Button("Place camera there")) {
		Matrix &m = selNode->transform;
		camera.position = Vector3(m._41, m._42, m._43) - camera.direction * 5.0f;
	}
	if (selNode->isSubclassOf<CNode>()) {
		CNode *geonode = selNode->cast<CNode>();
		if (ImGui::Button("Import geometry from DFF")) {
			//HWND hWindow = (HWND)g_window->getNativeWindow();
			HWND hWindow = NULL;

			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWindow;
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Renderware Clump\0*.DFF\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt = "dff";
			if (GetOpenFileNameA(&ofn)) {
				RwClump *impClump = LoadDFF(filepath);
				//*geonode->geometry-> = *impClump->geoList.geometries[0];

				// Remove current geometry
				CKAnyGeometry *kgeo = geonode->geometry.get();
				geonode->geometry.reset();
				while (kgeo) {
					CKAnyGeometry *next = kgeo->nextGeo.get();
					kenv.removeObject(kgeo);
					kgeo = next;
				}

				// Create new geometry
				CKGeometry *prevgeo = nullptr;
				for (RwAtomic *atom : impClump->atomics) {
					RwGeometry *rwgeo = impClump->geoList.geometries[atom->geoIndex];
					rwgeo->flags &= ~0x60;
					rwgeo->materialList.materials[0].color = 0xFFFFFFFF;
					CKGeometry *newgeo = kenv.createObject<CKGeometry>(-1);
					if (prevgeo) prevgeo->nextGeo = objref<CKAnyGeometry>(newgeo);
					else geonode->geometry.reset(newgeo);
					prevgeo = newgeo;
					newgeo->flags = 1;
					newgeo->flags2 = 0;
					newgeo->clump = new RwMiniClump;
					newgeo->clump->atomic.flags = 5;
					newgeo->clump->atomic.unused = 0;
					newgeo->clump->atomic.geometry.reset(new RwGeometry(*impClump->geoList.geometries[atom->geoIndex]));
				}

				progeocache.dict.clear();
			}
			else printf("GetOpenFileName fail: 0x%X\n", CommDlgExtendedError());
		}
		if (ImGui::Button("Export geometry to DFF")) {
			HWND hWindow = NULL;

			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWindow;
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Renderware Clump\0*.DFF\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_OVERWRITEPROMPT;
			ofn.lpstrDefExt = "dff";
			if (GetSaveFileNameA(&ofn)) {
				RwClump clump;

				RwFrame frame;
				for (int i = 0; i < 4; i++)
					for (int j = 0; j < 3; j++)
						frame.matrix[i][j] = (i == j) ? 1.0f : 0.0f;
				frame.index = 0xFFFFFFFF;
				frame.flags = 0;
				clump.frameList.frames.push_back(frame);
				clump.frameList.extensions.emplace_back();

				int n = 1, ng = 0;
				CKAnyGeometry *kgeo = geonode->geometry.get();
				while (kgeo) {
					frame.index = 0; //.index++;
					clump.frameList.frames.push_back(frame);
					clump.frameList.extensions.emplace_back();
					clump.geoList.geometries.push_back(new RwGeometry(*kgeo->clump->atomic.geometry.get()));
					RwAtomic *atom = new RwAtomic;
					atom->frameIndex = n;
					atom->geoIndex = ng;
					atom->flags = 5;
					atom->unused = 0;
					clump.atomics.push_back(atom);
					n++; // n becomes frame of bone root

					if (geonode->isSubclassOf<CAnimatedNode>()) {
						RwFrameList *framelist = geonode->cast<CAnimatedNode>()->frameList;
						RwExtHAnim *hanim = (RwExtHAnim*)framelist->extensions[0].exts[0];
						frame.index = n-1;
						clump.frameList.frames.push_back(frame);
						clump.frameList.extensions.push_back(framelist->extensions[0]);

						std::stack<uint32_t> parBoneStack;
						parBoneStack.push(0);
						uint32_t parBone = 0;
						std::vector<std::pair<uint32_t, uint32_t>> bones;

						for (uint32_t i = 1; i < hanim->bones.size(); i++) {
							auto &hb = hanim->bones[i];
							assert(hb.nodeIndex == i);
							bones.push_back(std::make_pair(hb.nodeId, parBone));
							if (hb.flags & 2)
								parBoneStack.push(parBone);
							parBone = i;
							if (hb.flags & 1) {
								parBone = parBoneStack.top();
								parBoneStack.pop();
							}
						}

						for (auto &bn : bones) {
							frame.index = bn.second + n;
							clump.frameList.frames.push_back(frame);

							RwExtHAnim *bha = new RwExtHAnim;
							bha->version = 0x100;
							bha->nodeId = bn.first;
							RwsExtHolder reh;
							reh.exts.push_back(bha);
							clump.frameList.extensions.push_back(std::move(reh));
						}

						n += hanim->bones.size();
					}

					kgeo = kgeo->nextGeo.get();
					ng++;
				}
				printf("done\n");
				IOFile dff(filepath, "wb");
				clump.serialize(&dff);
			}
		}
	}
}
