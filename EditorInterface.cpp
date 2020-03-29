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
	float camspeed = 0.5f;
	if (ImGui::GetIO().KeyShift)
		camspeed = 0.2f;
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
		}
		int dx = g_window->getMouseX() - rotStartX;
		int dy = g_window->getMouseY() - rotStartY;
		camera.orientation = rotOrigOri + Vector3(-dy * 0.01f, dx*0.01f, 0);
	}
	else
		rotating = false;

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
		ImGui::Checkbox("Show textures", &showTextures);
		ImGui::Separator();
		ImGui::DragFloat3("Geo pos", &selgeoPos.x, 0.1f);
		if (ImGui::Button("Move geo to front"))
			selgeoPos = camera.position + camera.direction * 3;
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
}
