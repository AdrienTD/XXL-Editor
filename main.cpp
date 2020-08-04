#include "KEnvironment.h"
#include "CKManager.h"
#include "CKHook.h"
#include "CKGroup.h"
#include "CKComponent.h"
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
#include "EditorInterface.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <Windows.h>
#include <commdlg.h>
#include <io.h>

#include <INIReader.h>

Window *g_window = nullptr;

void sporq(KEnvironment &kenv)
{
	int strnum = 0;
	auto &strgndlist = kenv.sectorObjects[strnum].categories[CGround::CATEGORY].type[CGround::CLASS_ID];
	AABoundingBox largeBoundaries = AABoundingBox(Vector3(66666.6f, 66666.6f, 66666.6f), Vector3(-66666.6f, -66666.6f, -66666.6f));

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
		CKAnyGeometry *geo = sgsr->geometry.get();
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

	img = RwImage::loadFromFile(filename);
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

void nogui(KEnvironment &kenv)
{
	// Load the game and level
	kenv.loadGame("C:\\Users\\Adrien\\Desktop\\kthings\\xxl1plus", 1, KEnvironment::PLATFORM_PC);
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

void gc2wn()
{
	KEnvironment kwn, kgc;

	// No factories!!!
	kwn.loadGame("C:\\Users\\Adrien\\Desktop\\kthings\\mptest", 1, KEnvironment::PLATFORM_PC);
	kgc.loadGame("C:\\Users\\Adrien\\Desktop\\kthings\\gcfiles", 1, KEnvironment::PLATFORM_GCN);

	for (int i = 0; i <= 8; i++) {
		kwn.loadLevel(i);
		kgc.loadLevel(i);

		// Check obj count
		for (int clcat = 0; clcat < 15; clcat++) {
			assert(kwn.levelObjects.categories[clcat].type.size() == kgc.levelObjects.categories[clcat].type.size());
			for (int clid = 0; clid < kwn.levelObjects.categories[clcat].type.size(); clid++) {
				if(kwn.levelObjects.categories[clcat].type[clid].objects.size() != kgc.levelObjects.categories[clcat].type[clid].objects.size())
					printf("Level %i: class (%i, %i) has different object count: kwn=%i, kgc=%i", i, clcat, clid,
						kwn.levelObjects.categories[clcat].type[clid].objects.size(), kgc.levelObjects.categories[clcat].type[clid].objects.size());
			}
		}

		auto copy_gc2wn = [&kwn,&kgc](int clcat, int clid) {
			auto &pcobjvec = kwn.levelObjects.getClassType(clcat, clid).objects;
			auto &gcobjvec = kgc.levelObjects.getClassType(clcat, clid).objects;
			assert(pcobjvec.size() == gcobjvec.size());
			for (size_t i = 0; i < pcobjvec.size(); i++) {
				CKUnknown *pcobj = (CKUnknown*)pcobjvec[i];
				CKUnknown *gcobj = (CKUnknown*)gcobjvec[i];
				if (pcobj->length == gcobj->length) {
					assert(!memcmp(pcobj->mem, gcobj->mem, pcobj->length));
				}
				else {
					free(pcobj->mem);
					pcobj->mem = malloc(gcobj->length);
					pcobj->length = gcobj->length;
					memcpy(pcobj->mem, gcobj->mem, gcobj->length);
				}
			}
		};

		for (const std::pair<int, int> &clsid : { std::make_pair<int,int>(4,26), std::make_pair(2,32), std::make_pair(12,51) })
			copy_gc2wn(clsid.first, clsid.second);
		//for (int j = 0; j < kwn.levelObjects.categories[6].type.size(); j++)
		for (int j : {6,7,8,9,10,11,12,17,19,23,26,28,29,30,31})
			if(j != 22)
				copy_gc2wn(6, j);

		kwn.saveLevel(i);
	}

	printf("Success!!\n");
	getchar();
}

int llmain()
{
	KEnvironment kenv;

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

	//kenv.loadGame("C:\\Users\\Adrien\\Downloads\\virtualboxshare\\aoxxl2demo\\Astérix & Obélix XXL2 DEMO", KEnvironment::KVERSION_XXL2, KEnvironment::PLATFORM_PC);
	kenv.loadGame("D:\\", KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PS2);
	kenv.loadLevel(0);
	getchar();
	return 0;
}

int cpntcsv() {
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.loadGame(gamePath.c_str(), KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC);

	struct NameListener : MemberListener {
		FILE *csv;
		NameListener(FILE *csv) : csv(csv) {}
		void write(const char *name) { fprintf(csv, "%s\t", name); }
		void reflect(uint8_t &ref, const char *name) override { write(name); }
		void reflect(uint16_t &ref, const char *name) override { write(name); }
		void reflect(uint32_t &ref, const char *name) override { write(name); }
		void reflect(float &ref, const char *name) override { write(name); }
		void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { write(name); }
		void reflect(Vector3 &ref, const char *name) override { fprintf(csv, "%s X\t%s Y\t%s Z\t", name, name, name); }
	};
	struct ValueListener : MemberListener {
		FILE *csv;
		ValueListener(FILE *csv) : csv(csv) {}
		void reflect(uint8_t &ref, const char *name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint16_t &ref, const char *name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint32_t &ref, const char *name) override { fprintf(csv, "%u\t", ref); }
		void reflect(float &ref, const char *name) override { fprintf(csv, "%f\t", ref); }
		void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { fprintf(csv, "%s\t", ref._pointer->getClassName()); }
		void reflect(Vector3 &ref, const char *name) override { fprintf(csv, "%f\t%f\t%f\t", ref.x, ref.y, ref.z); }
	};

	FILE *csv;
	fopen_s(&csv, "EnemyCpnts.txt", "w");
	NameListener nl(csv);
	ValueListener vl(csv);

	for (int lvl = 1; lvl <= 6; lvl++) {
		kenv.loadLevel(lvl);
		if (lvl == 1) {
			CKBasicEnemyCpnt *firstcpnt = kenv.levelObjects.getFirst<CKBasicEnemyCpnt>();
			fprintf(csv, "Level\tIndex\t");
			firstcpnt->reflectMembers(nl);
		}
		int index = 0;
		for (CKObject *obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
			fprintf(csv, "\n%i\t%i\t", lvl, index);
			obj->cast<CKBasicEnemyCpnt>()->reflectMembers(vl);
			index++;
		}
	}

	fclose(csv);
	return 0;
}

#ifdef XEC_RELEASE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main()
#endif
{
	// Initialize SDL
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);
	g_window = new Window();

	// Create a Kal engine environment/simulation
	KEnvironment kenv;

	// Register factories to known classes
	kenv.addFactory<CKServiceManager>();

	kenv.addFactory<CKSrvCollision>();
	kenv.addFactory<CKSrvEvent>();
	kenv.addFactory<CKSrvPathFinding>();
	kenv.addFactory<CKSrvDetector>();
	kenv.addFactory<CKSrvMarker>();
	kenv.addFactory<CKSrvBeacon>();

	//kenv.addFactory<CKHkBasicBonus>();
	//kenv.addFactory<CKHkWildBoar>();
	//kenv.addFactory<CKHkBasicEnemy>();
	//kenv.addFactory<CKHkRocketRoman>();
	kenv.addFactory<CKHkPressionStone>();
	kenv.addFactory<CKHkAsterix>();
	kenv.addFactory<CKHkObelix>();
	kenv.addFactory<CKHkIdefix>();
	kenv.addFactory<CKHkMachinegun>();
	kenv.addFactory<CKHkTorch>();
	kenv.addFactory<CKHkHearth>();
	kenv.addFactory<CKHkDrawbridge>();
	kenv.addFactory<CKHkMegaAshtray>();
	kenv.addFactory<CKHkBoat>();
	kenv.addFactory<CKHkCorkscrew>();
	kenv.addFactory<CKHkTurnstile>();
	kenv.addFactory<CKHkLifter>();
	kenv.addFactory<CKHkActivator>();
	kenv.addFactory<CKHkRotaryBeam>();
	kenv.addFactory<CKHkLightPillar>();
	kenv.addFactory<CKHkWind>();
	kenv.addFactory<CKHkJumpingRoman>();
	kenv.addFactory<CKHkWaterJet>();
	kenv.addFactory<CKHkPowderKeg>();
	kenv.addFactory<CKHkTriangularTurtle>();
	kenv.addFactory<CKHkBasicEnemy>();
	kenv.addFactory<CKHkRomanArcher>();
	kenv.addFactory<CKHkAnimatedCharacter>();
	kenv.addFactory<CKHkSwingDoor>();
	kenv.addFactory<CKHkSlideDoor>();
	kenv.addFactory<CKHkCrumblyZone>();
	kenv.addFactory<CKHkHelmetCage>();
	kenv.addFactory<CKHkSquareTurtle>();
	kenv.addFactory<CKHkTeleBridge>();
	kenv.addFactory<CKHkCrate>();
	kenv.addFactory<CKHkBasicBonus>();
	kenv.addFactory<CKHkDonutTurtle>();
	kenv.addFactory<CKHkPyramidalTurtle>();
	kenv.addFactory<CKHkRollingStone>();
	kenv.addFactory<CKHkInterfaceBase>();
	kenv.addFactory<CKHkInterfaceEvolution>();
	kenv.addFactory<CKHkCatapult>();
	kenv.addFactory<CKHkInterfacePause>();
	kenv.addFactory<CKHkInterfaceInGame>();
	kenv.addFactory<CKHkInterfaceOption>();
	kenv.addFactory<CKHkInterfaceMain>();
	kenv.addFactory<CKHkInterfaceLoadSave>();
	kenv.addFactory<CKHkInterfaceCloth>();
	kenv.addFactory<CKHkInterfaceShop>();
	kenv.addFactory<CKHkPushPullAsterix>();
	kenv.addFactory<CKHkBasicEnemyLeader>();
	kenv.addFactory<CKHkTelepher>();
	kenv.addFactory<CKHkTowedTelepher>();
	kenv.addFactory<CKHkBumper>();
	kenv.addFactory<CKHkClueMan>();
	kenv.addFactory<CKHkSky>();
	kenv.addFactory<CKHkRocketRoman>();
	kenv.addFactory<CKHkJetPackRoman>();
	kenv.addFactory<CKHkWildBoar>();
	kenv.addFactory<CKHkAsterixShop>();
	kenv.addFactory<CKHkWater>();
	kenv.addFactory<CKHkMobileTower>();
	kenv.addFactory<CKHkBoss>();
	kenv.addFactory<CKHkInterfaceDemo>();
	kenv.addFactory<CKHkWaterFx>();
	kenv.addFactory<CKHkHighGrass>();
	kenv.addFactory<CKHkWaterFall>();
	kenv.addFactory<CKHkInterfaceGallery>();
	kenv.addFactory<CKHkTrioCatapult>();
	kenv.addFactory<CKHkObelixCatapult>();
	kenv.addFactory<CKHkInterfaceOpening>();
	kenv.addFactory<CKHkAsterixCheckpoint>();
	kenv.addFactory<CKHkBonusSpitter>();
	kenv.addFactory<CKHkLight>();

	kenv.addFactory<CKHkAsterixLife>();
	kenv.addFactory<CKHkBoatLife>();
	kenv.addFactory<CKHkObelixLife>();
	kenv.addFactory<CKHkMecaLife>();
	kenv.addFactory<CKHkIdefixLife>();
	kenv.addFactory<CKHkEnemyLife>();
	kenv.addFactory<CKHkTriangularLife>();
	kenv.addFactory<CKHkAnimatedCharacterLife>();
	kenv.addFactory<CKHkSquareTurtleLife>();
	kenv.addFactory<CKHkDonutTurtleLife>();
	kenv.addFactory<CKHkPyramidalTurtleLife>();
	kenv.addFactory<CKHkCatapultLife>();
	kenv.addFactory<CKHkSkyLife>();
	kenv.addFactory<CKHkWaterLife>();
	kenv.addFactory<CKHkBossLife>();
	kenv.addFactory<CKHkWaterFxLife>();
	kenv.addFactory<CKHkAsterixCheckpointLife>();
	kenv.addFactory<CKHkWaterFallLife>();

	kenv.addFactory<CKGroupRoot>();
	kenv.addFactory<CKGrpMeca>();
	kenv.addFactory<CKGrpTrio>();
	kenv.addFactory<CKGrpBoat>();
	kenv.addFactory<CKGrpSquadEnemy>();
	kenv.addFactory<CKGrpEnemy>();
	kenv.addFactory<CKGrpPoolSquad>();
	kenv.addFactory<CKGrpWalkingCharacter>();
	kenv.addFactory<CKGrpBonus>();
	kenv.addFactory<CKGrpFrontEnd>();
	kenv.addFactory<CKGrpCatapult>();
	kenv.addFactory<CKGrpMap>();
	kenv.addFactory<CKGrpStorageStd>();
	kenv.addFactory<CKGrpCrate>();
	kenv.addFactory<CKGrpBonusPool>();
	kenv.addFactory<CKGrpAsterixBonusPool>();
	kenv.addFactory<CKGrpSquadJetPack>();
	kenv.addFactory<CKGrpWildBoarPool>();
	kenv.addFactory<CKGrpAsterixCheckpoint>();
	kenv.addFactory<CKGrpBonusSpitter>();
	kenv.addFactory<CKGrpLight>();

	kenv.addFactory<CKGrpTrioLife>();
	kenv.addFactory<CKGrpMecaLife>();
	kenv.addFactory<CKGrpBonusLife>();
	kenv.addFactory<CKGrpMapLife>();
	kenv.addFactory<CKGrpEnemyLife>();
	kenv.addFactory<CKGrpAsterixCheckpointLife>();

	kenv.addFactory<CKCrateCpnt>();
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.addFactory<CKBasicEnemyLeaderCpnt>();
	kenv.addFactory<CKJumpingRomanCpnt>();
	kenv.addFactory<CKRomanArcherCpnt>();
	kenv.addFactory<CKRocketRomanCpnt>();
	kenv.addFactory<CKJetPackRomanCpnt>();
	kenv.addFactory<CKMobileTowerCpnt>();
	kenv.addFactory<CKTriangularTurtleCpnt>();
	kenv.addFactory<CKSquareTurtleCpnt>();
	kenv.addFactory<CKDonutTurtleCpnt>();
	kenv.addFactory<CKPyramidalTurtleCpnt>();

	kenv.addFactory<CTextureDictionary>();
	kenv.addFactory<CAnimationDictionary>();
	kenv.addFactory<CKSoundDictionary>();
	kenv.addFactory<CKSoundDictionaryID>();

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

	kenv.addFactory<CKPFGraphTransition>();
	kenv.addFactory<CKSector>();
	kenv.addFactory<CKChoreoKey>();
	kenv.addFactory<CKPFGraphNode>();
	kenv.addFactory<CKSas>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CDynamicGround>();
	kenv.addFactory<CKChoreography>();
	kenv.addFactory<CKLine>();
	kenv.addFactory<CKSpline4L>();
	kenv.addFactory<CKMeshKluster>();
	kenv.addFactory<CKBeaconKluster>();

	kenv.addFactory<CCloneManager>();

	// Load INI file
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");
	std::string outGamePath = config.Get("XXL-Editor", "outgamepath", gamePath);

	// Verify if GAME.KWN is in the gamepath
	std::string testPath = gamePath + "/GAME.KWN";
	if (_access(testPath.c_str(), 0) == -1) {
		MessageBox((HWND)g_window->getNativeWindow(), "GAME.KWN file not found!\n"
			"Be sure you that you set in the file xec-settings.ini the path to where you installed the patched copy of Asterix XXL 1.", NULL, 16);
		return -1;
	}

	// Load the game and level
	kenv.loadGame(gamePath.c_str(), 1, KEnvironment::PLATFORM_PC);
	kenv.outGamePath = outGamePath;
	kenv.loadLevel(8);

	// Initialize graphics renderer
	Renderer *gfx = CreateRendererD3D9(g_window);
	// Initialize Dear ImGui
	ImGuiImpl_Init(g_window);
	ImGuiImpl_CreateFontsTexture(gfx);
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Initialize the editor user interface
	EditorInterface editUI(kenv, g_window, gfx, config);
	editUI.prepareLevelGfx();

	while (!g_window->quitted()) {
		// Get window input
		g_window->handle();

		// Input + ImGui handling
		ImGuiImpl_NewFrame(g_window);
		editUI.iter();

		// Rendering
		gfx->setSize(g_window->getWidth(), g_window->getHeight());
		gfx->beginFrame();
		editUI.render();
		ImGuiImpl_Render(gfx);
		gfx->endFrame();
	}

	return 0;
}
