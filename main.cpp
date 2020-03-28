#include "KEnvironment.h"
#include "CKManager.h"
#include "CKDictionary.h"
#include "CKGeometry.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKGraphical.h"
#include <cassert>
#include "rw.h"
#include "rwext.h"
#include <stack>
#include "main.h"
#include "window.h"
#include "renderer.h"
#include "imguiimpl.h"
#include "imgui/imgui.h"
#include "rwrenderer.h"
#include "Camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <Windows.h>
#include <commdlg.h>

Window *g_window = nullptr;

void sporq(KEnvironment &kenv)
{
	int strnum = 0;
	auto &strgndlist = kenv.sectorObjects[strnum].categories[CGround::CATEGORY].type[CGround::CLASS_ID];
	std::array<float, 6> largeBoundaries = { 66666.6f, 66666.6f, 66666.6f, -66666.6f, -66666.6f, -66666.6f };

	// Remove grounds
	CGround *lastGround = (CGround*)strgndlist.objects[strgndlist.objects.size() - 1];
	lastGround->aabb = largeBoundaries;
	CKMeshKluster *meshKluster = (CKMeshKluster*)kenv.sectorObjects[strnum].categories[CKMeshKluster::CATEGORY].type[CKMeshKluster::CLASS_ID].objects[0];
	decltype(meshKluster->grounds) newGrounds;
	for (auto &gndref : meshKluster->grounds) {
		if (gndref->isSubclassOfID(CGround::FULL_ID) && (gndref.get() != lastGround))
			;
		else
			newGrounds.push_back(gndref);
	}
	meshKluster->grounds = std::move(newGrounds);
	meshKluster->aabb = largeBoundaries;

	// Remove sector root scene node geometries
	CSGSectorRoot *strsgsr = (CSGSectorRoot*)kenv.sectorObjects[strnum].categories[CSGSectorRoot::CATEGORY].type[CSGSectorRoot::CLASS_ID].objects[0];
	CSGSectorRoot *lvlsgsr = (CSGSectorRoot*)kenv.levelObjects.categories[CSGSectorRoot::CATEGORY].type[CSGSectorRoot::CLASS_ID].objects[0];
	for (CSGSectorRoot *sgsr : { strsgsr }) {
		CKGeometry *geo = sgsr->geometry.get();
		sgsr->geometry.reset();
		while (geo) {
			//assert(geo->refCount == 0);
			CKGeometry *nextGeo = (CKGeometry*)(geo->nextGeo.get());
			kenv.removeObject(geo);
			geo = nextGeo;
		}
	}

	// Fix boundaries in sector
	CKSector *kStrObj = (CKSector*)kenv.levelObjects.getClassType<CKSector>().objects[strnum + 1];
	kStrObj->boundaries = largeBoundaries;
}

void exportDFF(const RwAtomic *atomic, const RwFrameList *frameList = nullptr)
{
	RwClump *clump = new RwClump;
	if (frameList) {
		//clump->frameList = *frameList;
		RwFrame frame;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 3; j++)
				frame.matrix[i][j] = (i == j) ? 1.0f : 0.0f;
		frame.index = 0xFFFFFFFF;
		frame.flags = 0; //0x020003;

		std::stack<uint32_t> parBoneStack;
		parBoneStack.push(0);
		uint32_t parBone = 0;
		std::vector<std::pair<uint32_t, uint32_t>> bones;

		RwExtHAnim *hanim = (RwExtHAnim*)(frameList->extensions[0].exts[0]);

		for (uint32_t i = 0; i < hanim->bones.size(); i++) {
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

		clump->frameList.frames.push_back(frame);
		clump->frameList.extensions.push_back(RwsExtHolder());
		clump->frameList.frames.push_back(frameList->frames[0]);
		clump->frameList.frames.back().index = 0;
		clump->frameList.extensions.push_back(frameList->extensions[0]);

		for (auto &bn : bones) {
			RwFrame bf;
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 3; j++)
					bf.matrix[i][j] = (i == j) ? 1.0f : 0.0f;
			bf.index = bn.second + 1;
			bf.flags = 3;
			clump->frameList.frames.push_back(std::move(frame));

			RwExtHAnim *bha = new RwExtHAnim;
			bha->version = 0x100;
			bha->nodeId = bn.first;
			RwsExtHolder reh;
			reh.exts.push_back(bha);
			clump->frameList.extensions.push_back(std::move(reh));
		}
	}
	else {
		RwFrame frame;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 3; j++)
				frame.matrix[i][j] = (i == j) ? 1.0f : 0.0f;
		frame.index = 0xFFFFFFFF;
		frame.flags = 0; //0x020003;
		clump->frameList.frames.push_back(std::move(frame));
	}
	clump->geoList.geometries.push_back(atomic->geometry.get());
	RwAtomic glatom;
	glatom.frameIndex = 0;
	glatom.geoIndex = 0;
	glatom.flags = atomic->flags;
	glatom.unused = atomic->unused;
	glatom.extensions = atomic->extensions;
	clump->atomics.push_back(&glatom);
	IOFile file("test.dff", "wb");
	clump->serialize(&file);
}

RwClump * LoadDFF(const char *filename)
{
	RwClump *clump = new RwClump;
	IOFile dff(filename, "rb");
	rwCheckHeader(&dff, 0x10);
	clump->deserialize(&dff);
	dff.close();
	return clump;
}

void dfftest(KEnvironment &kenv)
{
	CAnimatedNode *idefixNode = (CAnimatedNode*)kenv.levelObjects.categories[CAnimatedNode::CATEGORY].type[CAnimatedNode::CLASS_ID].objects[0];
	CKSkinGeometry *idefixGeo = (CKSkinGeometry*)idefixNode->geometry.get();
	exportDFF(&idefixGeo->clump->atomic, idefixNode->frameList);

	RwClump *testClump = LoadDFF("test.dff");
	IOFile sdff = IOFile("test2.dff", "wb");
	testClump->serialize(&sdff);
	sdff.close();

	//IOFile rdff("C:\\Users\\Adrien\\Desktop\\kthings\\bigsmoke\\bs.dff", "rb");
	RwClump *bs = LoadDFF("C:\\Users\\Adrien\\Downloads\\1566756389_Multibot\\Multibot.dff");
	return;

}

void unknown()
{
	//CNode *newNode = kenv.createObject<CNode>(-1);
//CNode *firstNode = (CNode*)kenv.levelObjects.categories[CNode::CATEGORY].type[CNode::CLASS_ID].objects[0];
//*newNode = *firstNode;

//CKUnknown *newNode = new CKUnknown(CKGeometry::CATEGORY, CKGeometry::CLASS_ID);
//kenv.levelObjects.categories[CKGeometry::CATEGORY].type[CKGeometry::CLASS_ID].objects.push_back(newNode);
//CKUnknown *firstNode = (CKUnknown*)kenv.levelObjects.categories[CKGeometry::CATEGORY].type[CKGeometry::CLASS_ID].objects[0];
//*newNode = *firstNode;

}

void DoEvents(KEnvironment &kenv)
{
	CKSrvEvent *srvEvent = (CKSrvEvent*)kenv.levelObjects.getClassType<CKSrvEvent>().objects[0];
	int sum1 = 0, sum2 = 0;
	for (CKSrvEvent::StructB &b : srvEvent->bees) {
		printf("%i\t%i\n", b._1, b._2);
		sum1 += b._1;
		sum2 += b._2;
	}
	printf("Sums: %i %i\n-------\n", sum1, sum2);

	int ev = 0;
	for (auto &b : srvEvent->bees) {
		for (int i = 0; i < b._1; i++) {
			CKObject *obj = srvEvent->objs[ev].get();
			printf("OBJ (%2i,%3i) EVENT 0x%04X\n", obj->getClassCategory(), obj->getClassID(), srvEvent->objInfos[ev]);
			ev++;
		}
		printf("-------------\n");
	}

	for (auto &b : srvEvent->bees) {
		//b._1 = 0;
		b._2 = 31;
	}
}

CKGeometry *CreateTestGeometry(KEnvironment &kenv)
{
	CKGeometry *kgeo = kenv.createObject<CKGeometry>(-1);
	kgeo->flags = 1;
	kgeo->flags2 = 0;

	RwMiniClump *mclp = new RwMiniClump;
	kgeo->clump = mclp;
	mclp->atomic.flags = 5;
	mclp->atomic.unused = 0;

	auto rwgeo = std::make_unique<RwGeometry>();
	rwgeo->flags = 0x1000F;
	rwgeo->numTris = 1;
	rwgeo->numVerts = 3;
	rwgeo->numMorphs = 1;
	rwgeo->colors = {0xFFFF0000, 0xFF00FFFF, 0xFF0000FF};
	rwgeo->texSets.push_back(std::vector<std::array<float, 2>>({ {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f} }));
	RwGeometry::Triangle tri;
	tri.indices = { 0,1,2 };
	tri.materialId = 0;
	rwgeo->tris.push_back(std::move(tri));
	rwgeo->spherePos = Vector3(0, 0, 0);
	rwgeo->sphereRadius = 2.0f;
	rwgeo->hasVertices = 1;
	rwgeo->hasNormals = 0;
	rwgeo->verts = { Vector3(-1,0,0), Vector3(1,0,0), Vector3(0,1,0) };

	RwMaterial rwmat;
	rwmat.flags = 0;
	rwmat.color = 0xFFFFFFFF;
	rwmat.unused = 4;
	rwmat.isTextured = 0;
	rwmat.ambient = 1;
	rwmat.specular = 0;
	rwmat.diffuse = 1;

	rwgeo->materialList.slots = { 0xFFFFFFFF };
	rwgeo->materialList.materials.push_back(std::move(rwmat));

	mclp->atomic.geometry = std::move(rwgeo);
	return kgeo;
}

void HackGeo(KEnvironment &kenv)
{
	for (CKObject *obj : kenv.levelObjects.getClassType<CKGeometry>().objects) {
		CKGeometry *geo = (CKGeometry*)obj;
		if (geo->clump) {
			geo->clump->atomic.frameIndex = 0;
			geo->clump->atomic.geoIndex = 0;
		}
	}
}

void DoGeo(KEnvironment &kenv)
{
	//DoEvents(kenv);
	//HackGeo(kenv);
	CKGeometry *geo = CreateTestGeometry(kenv);
	CNode *node = kenv.createObject<CNode>(-1);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			node->transform.m[i][j] = (i == j) ? 1.0f : 0.0f;
	//node->transform._41 = 3;
	//node->transform._42 = 1;
	//node->transform._43 = -16.5;
	node->transform._41 = 7;
	node->transform._42 = 3;
	node->transform._43 = -20;
	node->unk1 = 0;
	node->unk2 = 255;
	node->geometry = geo;
	CSGSectorRoot *sgstr = (CSGSectorRoot*)kenv.levelObjects.getClassType<CSGSectorRoot>().objects[0];
	sgstr->insertChild(node);
}

const char * GetPathFilename(const char *path)
{
	const char *ptr = path;
	const char *fnd = path;
	while (*ptr) {
		if (*ptr == '\\' || *ptr == '/')
			fnd = ptr + 1;
		ptr++;
	}
	return fnd;
}

std::string GetPathFilenameNoExt(const char *path)
{
	const char *fe = GetPathFilename(path);
	const char *end = strrchr(fe, '.');
	if (end)
		return std::string(fe, end);
	return std::string(fe);
}

void AddTexture(KEnvironment &kenv, const char *filename)
{
	CTextureDictionary::Texture tex;
	RwImage &img = tex.image;

	int sizx, sizy, origBpp;
	void *pix = stbi_load(filename, &sizx, &sizy, &origBpp, 4);
	if (!pix) {
		printf("Failed to load image file\n");
		return;
	}
	img.width = sizx;
	img.height = sizy;
	img.bpp = 32;
	img.pitch = img.width * 4;
	img.pixels.resize(img.pitch * img.height);
	memcpy(img.pixels.data(), pix, img.pixels.size());

	strcpy_s(tex.name, GetPathFilenameNoExt(filename).c_str());
	tex.unk1 = 2;
	tex.unk2 = 1;
	tex.unk3 = 1;

	CTextureDictionary *dict = (CTextureDictionary*)kenv.levelObjects.getClassType<CTextureDictionary>().objects[0];
	dict->textures.push_back(std::move(tex));
}

void CloneEdit(KEnvironment &kenv)
{
	CCloneManager *cloneManager = (CCloneManager*)kenv.levelObjects.getClassType<CCloneManager>().objects[0];

	int i = 0;
	for (auto &bing : cloneManager->_teamDict._bings) {
		printf("Bing %i\n", i++);
		if (bing._clump)
			if (RwGeometry *geo = bing._clump->atomic.geometry.get())
				for (auto &mat : geo->materialList.materials)
					printf(" - %s\n", mat.texture.name.c_str());
	}

	//std::swap(cloneManager->_teamDict._bings[38], cloneManager->_teamDict._bings[89]);
	std::swap(cloneManager->_teamDict._bings[89], cloneManager->_teamDict._bings[91]);
	std::swap(cloneManager->_teamDict._bings[90], cloneManager->_teamDict._bings[92]);
	//std::swap(cloneManager->_teamDict._bings[38], cloneManager->_teamDict._bings[39]);

	RwClump *pyra = LoadDFF("C:\\Users\\Adrien\\Desktop\\kthings\\xecpp_dff_test\\GameCube Hat\\gamecube.blend.dff");
	cloneManager->_teamDict._bings[39]._clump->atomic.geometry = std::unique_ptr<RwGeometry>(new RwGeometry(*pyra->geoList.geometries[0]));

	AddTexture(kenv, "C:\\Users\\Adrien\\Desktop\\kthings\\xecpp_dff_test\\GameCube Hat\\hat_gamecube_color.png");
}

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

void IGEnumNode(CKSceneNode *node)
{
	if (!node)
		return;
	bool hassub = node->isSubclassOf<CSGBranch>();
	bool open = ImGui::TreeNodeEx(node, hassub ? 0 : ImGuiTreeNodeFlags_Leaf, "%s", node->getClassName());
	if (open) {
		if (hassub) {
			CSGBranch *branch = node->cast<CSGBranch>();
			IGEnumNode(branch->child.get());
		}
		ImGui::TreePop();
	}
	IGEnumNode(node->next.get());
}

void IGSceneGraph(KEnvironment &kenv)
{
	CSGRootNode *root = kenv.levelObjects.getObject<CSGRootNode>(0);
	IGEnumNode(root);
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

void nogui(KEnvironment &kenv)
{
	// Load the game and level
	kenv.loadGame("C:\\Users\\Adrien\\Desktop\\kthings\\xxl1plus", 1);
	kenv.loadLevel(1);

	//sporq(kenv);

	//for (CKObject *obj : kenv.levelObjects.getClassType<CClone>().objects) {
	//	((CClone*)obj)->unk1 &= ~2;
	//	((CClone*)obj)->cloneInfo = 0x010041;
	//}

	//dfftest(kenv);

	//DoGeo(kenv);

	//CloneEdit(kenv);
	//InvertTextures(kenv);

	// Save the level back
	//kenv.saveLevel(8);

	printf("lol\n");
	//getchar();
}

int main()
{
	// Initialize SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	g_window = new Window();

	// Create a Kal engine environment/simulation
	KEnvironment kenv;

	// Register factories to known classes
	kenv.addFactory<CKServiceManager>();
	//kenv.addFactory<CKSrvEvent>();

	kenv.addFactory<CTextureDictionary>();

	kenv.addFactory<CKParticleGeometry>();
	kenv.addFactory<CKGeometry>();
	kenv.addFactory<CKSkinGeometry>();
	
	kenv.addFactory<CSGRootNode>();
	kenv.addFactory<CSGSectorRoot>();
	kenv.addFactory<CNode>();
	kenv.addFactory<CKDynBSphereProjectile>();
	kenv.addFactory<CSGBranch>();
	kenv.addFactory<CGlowNodeFx>();
	kenv.addFactory<CClone>();
	kenv.addFactory<CKBoundingSphere>();
	kenv.addFactory<CKDynamicBoundingSphere>();
	kenv.addFactory<CKAABB>();
	kenv.addFactory<CKOBB>();
	kenv.addFactory<CParticlesNodeFx>();
	kenv.addFactory<CAnimatedNode>();
	kenv.addFactory<CAnimatedClone>();
	kenv.addFactory<CKAACylinder>();
	kenv.addFactory<CSkyNodeFx>();
	kenv.addFactory<CFogBoxNodeFx>();
	kenv.addFactory<CTrailNodeFx>();

	kenv.addFactory<CKSector>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CKMeshKluster>();

	kenv.addFactory<CCloneManager>();

	kenv.loadGame("C:\\Users\\Adrien\\Desktop\\kthings\\xxl1plus", 1);
	kenv.loadLevel(1);

	CTextureDictionary *texDict = (CTextureDictionary*)kenv.levelObjects.getClassType<CTextureDictionary>().objects[0];

	Renderer *gfx = CreateRenderer(g_window);
	ImGuiImpl_Init(g_window);
	ImGuiImpl_CreateFontsTexture(gfx);

	static int selTexID = 0;
	RwImage *selImage = &texDict->textures[selTexID].image;
	texture_t sometex = gfx->createTexture(*selImage);
	auto selectTexture = [gfx,texDict,&sometex,&selImage](int newTexID) {
		if (newTexID < 0 || newTexID >= texDict->textures.size())
			return;
		selTexID = newTexID;
		gfx->unbindTexture(0);
		gfx->deleteTexture(sometex);
		selImage = &texDict->textures[selTexID].image;
		sometex = gfx->createTexture(*selImage);
	};

	ProTexDict *protexdict = new ProTexDict(gfx, texDict);
	ProTexDict **str_protexdicts = new ProTexDict*[kenv.numSectors];
	for (int i = 0; i < kenv.numSectors; i++) {
		str_protexdicts[i] = new ProTexDict(gfx, kenv.sectorObjects[i].getObject<CTextureDictionary>(0));
		str_protexdicts[i]->_next = protexdict;
	}
	ProGeoCache progeocache(gfx);

	CKAnyGeometry *mygeo = (CKAnyGeometry*)kenv.levelObjects.getClassType<CKSkinGeometry>().objects[0];
	RwGeometry *selGeometry = mygeo->clump->atomic.geometry.get();
	Vector3 selgeoPos(0, 0, 0);

	Camera camera;
	camera.position = Vector3(0, 0, -5);
	camera.orientation = Vector3(0, 0, 0);

	static int framesInSecond = 0;
	static int lastFps = 0;
	static uint32_t lastFpsTime = SDL_GetTicks() / 1000;

	bool showTextures = true;

	bool showImGuiDemo = false;

	while (!g_window->quitted()) {
		g_window->handle();
		//bool pressLeft = g_window->getKeyPressed(SDL_SCANCODE_LEFT);
		//bool pressRight = g_window->getKeyPressed(SDL_SCANCODE_RIGHT);
		//if (pressLeft)
		//	selectTexture(selTexID - 1);
		//if (pressRight)
		//	selectTexture(selTexID + 1);

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
			camera.orientation = rotOrigOri + Vector3(-dy*0.01f, dx*0.01f, 0);
		}
		else
			rotating = false;

		camera.updateMatrix();

		ImGuiImpl_NewFrame(g_window);
		ImGui::Begin("Main");
		ImGui::Text("Hello to all people from Stinkek's server!");
		ImGui::Text("FPS: %i", lastFps);
		ImGui::BeginTabBar("MainTab", 0);
		if (ImGui::BeginTabItem("Textures")) {
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
					protexdict->reset(texDict);
				}
				else printf("GetOpenFileName fail: 0x%X\n", CommDlgExtendedError());
			}
			if (ImGui::Button("Invert")) {
				InvertTextures(kenv);
				protexdict->reset(texDict);
			}
			ImGui::Columns(2);
			ImGui::BeginChild("TexSeletion");
			int i = 0;
			for (auto &img : texDict->textures) {
				if (ImGui::Selectable(img.name, i == selTexID))
					selectTexture(i);
				i++;
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("TexViewer");
			ImGui::Image(sometex, ImVec2(selImage->width, selImage->height));
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
			auto enumRwGeo = [&kenv,&selGeometry](RwGeometry *rwgeo, int i) {
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
				static const std::array<const char*,3> geotypenames = { "Particle geometries", "Geometries", "Skinned geometries" };
				ImGui::PushID(j);
				if (ImGui::TreeNode(geotypenames[j-1])) {
					int i = 0;
					for (CKObject *obj : kenv.levelObjects.getClassType(10, j).objects) {
						if(RwMiniClump *clp = ((CKAnyGeometry*)obj)->clump)
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
			IGSceneGraph(kenv);
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

		if(showImGuiDemo)
			ImGui::ShowDemoWindow(&showImGuiDemo);

		gfx->beginFrame();
		
		gfx->initModelDrawing();
		gfx->setTransformMatrix(Matrix::getTranslationMatrix(selgeoPos) * camera.sceneMatrix);
		progeocache.getPro(selGeometry, protexdict)->draw();

		//gfx->setTransformMatrix(camera.sceneMatrix);
		//for (CKObject *obj : kenv.levelObjects.getClassType<CKGeometry>().objects) {
		//	CKAnyGeometry *kgeo = (CKAnyGeometry*)obj;
		//	if (RwGeometry *rwgeo = kgeo->clump->atomic.geometry.get()) {
		//		progeocache.getPro(rwgeo, protexdict)->draw(showTextures);
		//	}
		//}

		//for (int str = 0; str < kenv.numSectors; str++) {
		//	for (CKObject *obj : kenv.sectorObjects[str].getClassType<CKGeometry>().objects) {
		//		CKAnyGeometry *kgeo = (CKAnyGeometry*)obj;
		//		if (RwGeometry *rwgeo = kgeo->clump->atomic.geometry.get()) {
		//			progeocache.getPro(rwgeo, str_protexdicts[str])->draw(showTextures);
		//		}
		//	}
		//}

		CSGSectorRoot *rootNode = kenv.levelObjects.getObject<CSGSectorRoot>(0);
		DrawSceneNode(rootNode, camera.sceneMatrix, gfx, progeocache, protexdict, showTextures);
		for (int str = 0; str < kenv.numSectors; str++) {
			CSGSectorRoot * strRoot = kenv.sectorObjects[str].getObject<CSGSectorRoot>(0);
			DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, str_protexdicts[str], showTextures);
		}

		gfx->initFormDrawing();
		gfx->bindTexture(0, sometex);
		gfx->fillRect(g_window->getMouseX(), g_window->getMouseY(), 32, 32, -1);
		ImGuiImpl_Render(gfx);
		
		gfx->endFrame();

		framesInSecond++;
		uint32_t sec = SDL_GetTicks() / 1000;
		if (sec != lastFpsTime) {
			lastFps = framesInSecond;
			framesInSecond = 0;
			lastFpsTime = sec;
		}
	}

	delete protexdict;

	return 0;
}
