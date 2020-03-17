#include "KEnvironment.h"
#include "CKManager.h"
#include "CKGeometry.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKGraphical.h"
#include <cassert>
#include "rw.h"
#include "rwext.h"
#include <stack>

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
		if (gndref->isSubclassOf(CGround::FULL_ID) && (gndref.get() != lastGround))
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

void dfftest(KEnvironment &kenv)
{
	CAnimatedNode *idefixNode = (CAnimatedNode*)kenv.levelObjects.categories[CAnimatedNode::CATEGORY].type[CAnimatedNode::CLASS_ID].objects[0];
	CKSkinGeometry *idefixGeo = (CKSkinGeometry*)idefixNode->geometry.get();
	exportDFF(&idefixGeo->clump->atomic, idefixNode->frameList);

	RwClump *testClump = new RwClump;
	IOFile dff("test.dff", "rb");
	rwCheckHeader(&dff, 0x10);
	testClump->deserialize(&dff);
	dff.close();
	IOFile sdff = IOFile("test2.dff", "wb");
	testClump->serialize(&sdff);
	sdff.close();

	//IOFile rdff("C:\\Users\\Adrien\\Desktop\\kthings\\bigsmoke\\bs.dff", "rb");
	IOFile rdff("C:\\Users\\Adrien\\Downloads\\1566756389_Multibot\\Multibot.dff", "rb");
	rwCheckHeader(&rdff, 0x10);
	RwClump *bs = new RwClump;
	bs->deserialize(&rdff);
	rdff.close();

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

int main()
{
	// Create a Kal engine environment/simulation
	KEnvironment kenv;

	// Register factories to known classes
	kenv.addFactory<CKServiceManager>();

	kenv.addFactory<CKParticleGeometry>();
	kenv.addFactory<CKGeometry>();
	kenv.addFactory<CKSkinGeometry>();
	
	kenv.addFactory<CSGRootNode>();
	kenv.addFactory<CSGSectorRoot>();
	kenv.addFactory<CNode>();
	kenv.addFactory<CSGBranch>();
	kenv.addFactory<CClone>();
	kenv.addFactory<CAnimatedNode>();

	kenv.addFactory<CKSector>();
	kenv.addFactory<CGround>();
	kenv.addFactory<CKMeshKluster>();

	kenv.addFactory<CCloneManager>();

	// Load the game and level
	kenv.loadGame("C:\\Users\\Adrien\\Desktop\\kthings\\xxl1plus", 1);
	kenv.loadLevel(6);

	//sporq(kenv);

	//for (CKObject *obj : kenv.levelObjects.getClassType<CClone>().objects) {
	//	((CClone*)obj)->unk1 &= ~2;
	//	((CClone*)obj)->cloneInfo = 0x010041;
	//}

	// Save the level back
	kenv.saveLevel(5);

	printf("lol\n");
	getchar();
	return 0;
}