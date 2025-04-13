#include "GeoUtils.h"

#include <cassert>
#include <stack>

#include "rw.h"
#include "rwext.h"
#include "File.h"

#include "KEnvironment.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"

std::unique_ptr<RwClump> GeoUtils::LoadDFF(const std::filesystem::path& filename)
{
	std::unique_ptr<RwClump> clump = std::make_unique<RwClump>();
	IOFile dff(filename.c_str(), "rb");
	auto rwverBackup = HeaderWriter::rwver; // TODO FIXME hack
	rwCheckHeader(&dff, 0x10);
	clump->deserialize(&dff);
	dff.close();
	HeaderWriter::rwver = rwverBackup;
	return clump;
}

RwClump GeoUtils::CreateClumpFromGeo(std::shared_ptr<RwGeometry> rwgeo, RwExtHAnim* hanim)
{
	RwClump clump;

	RwFrame frame;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
			frame.matrix[i][j] = (i == j) ? 1.0f : 0.0f;
	frame.index = 0xFFFFFFFF;
	frame.flags = 0;
	clump.frameList.frames.push_back(frame);
	clump.frameList.extensions.emplace_back();

	clump.geoList.geometries.push_back(rwgeo);

	RwAtomic& atom = clump.atomics.emplace_back();
	atom.frameIndex = 0;
	atom.geoIndex = 0;
	atom.flags = 5;
	atom.unused = 0;

	if (hanim) {
		frame.index = 0;
		clump.frameList.frames.push_back(frame);
		RwsExtHolder freh;
		auto haclone = hanim->clone();
		((RwExtHAnim*)haclone.get())->nodeId = hanim->bones[0].nodeId;
		freh.exts.push_back(std::move(haclone));
		clump.frameList.extensions.push_back(std::move(freh));

		std::stack<uint32_t> parBoneStack;
		parBoneStack.push(0);
		uint32_t parBone = 0;
		std::vector<std::pair<uint32_t, uint32_t>> bones;

		for (uint32_t i = 1; i < hanim->bones.size(); i++) {
			auto& hb = hanim->bones[i];
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

		for (auto& bn : bones) {
			frame.index = bn.second + 1;
			clump.frameList.frames.push_back(frame);

			auto bha = std::make_unique<RwExtHAnim>();
			bha->version = 0x100;
			bha->nodeId = bn.first;
			RwsExtHolder reh;
			reh.exts.push_back(std::move(bha));
			clump.frameList.extensions.push_back(std::move(reh));
		}
	}
	return clump;
}

RwGeometry GeoUtils::createEmptyGeo()
{
	RwGeometry geo;
	geo.flags = RwGeometry::RWGEOFLAG_POSITIONS;
	geo.numVerts = 3;
	geo.numTris = 1;
	geo.numMorphs = 1;
	RwGeometry::Triangle tri;
	tri.indices = { 0,1,2 };
	tri.materialId = 0;
	geo.tris = { std::move(tri) };
	geo.spherePos = Vector3(0, 0, 0);
	geo.sphereRadius = 0;
	geo.hasVertices = 1;
	geo.hasNormals = 0;
	geo.verts = { Vector3(0,0,0), Vector3(0,0,0), Vector3(0,0,0) };
	geo.materialList.slots = { 0xFFFFFFFF };
	RwMaterial mat;
	mat.flags = 0;
	mat.color = 0xFFFFFFFF;
	mat.unused = 0;
	mat.isTextured = 0;
	mat.ambient = mat.specular = mat.diffuse = 1.0f;
	geo.materialList.materials = { std::move(mat) };
	return geo;
}


void GeoUtils::ChangeNodeGeometry(KEnvironment& kenv, CNode* geonode, RwGeometry** rwgeos, size_t numRwgeos)
{
	// Remove current geometry
	// TODO: Proper handling of duplicate geometries
	CKAnyGeometry* kgeo = geonode->geometry.get();
	CLightSet* lightSetBackup = kgeo ? kgeo->lightSet.get() : nullptr;
	geonode->geometry.reset();
	while (kgeo) {
		if (CMaterial* mat = kgeo->material.get()) {
			kgeo->material.reset();
			if (mat->getRefCount() == 0)
				kenv.removeObject(mat);
		}
		CKAnyGeometry* next = kgeo->nextGeo.get();
		kenv.removeObject(kgeo);
		kgeo = next;
	}

	// Create new geometry
	CKAnyGeometry* prevgeo = nullptr;
	for (size_t g = 0; g < numRwgeos; ++g) {
		RwGeometry* rwgeotot = rwgeos[g];
		auto splitgeos = rwgeotot->splitByMaterial();
		for (auto& rwgeo : splitgeos) {
			if (rwgeo->tris.empty())
				continue;
			rwgeo->flags &= ~0x60;
			rwgeo->materialList.materials[0].color = 0xFFFFFFFF;

			// Create BinMeshPLG extension for RwGeo
			auto bmplg = std::make_unique<RwExtBinMesh>();
			bmplg->flags = 0;
			bmplg->totalIndices = rwgeo->numTris * 3;
			bmplg->meshes.emplace_back();
			RwExtBinMesh::Mesh& bmesh = bmplg->meshes.front();
			bmesh.material = 0;
			for (const auto& tri : rwgeo->tris) {
				bmesh.indices.push_back(tri.indices[0]);
				bmesh.indices.push_back(tri.indices[2]);
				bmesh.indices.push_back(tri.indices[1]);
			}
			rwgeo->extensions.exts.push_back(std::move(bmplg));

			// Create MatFX extension for RwAtomic
			std::unique_ptr<RwExtUnknown> fxaext = nullptr;
			if (rwgeo->materialList.materials[0].extensions.find(0x120)) {
				fxaext = std::make_unique<RwExtUnknown>();
				fxaext->_type = 0x120;
				fxaext->_length = 4;
				fxaext->_ptr = malloc(4);
				uint32_t one = 1;
				memcpy(fxaext->_ptr, &one, 4);
			}

			int sector = kenv.getObjectSector(geonode);

			CKAnyGeometry* newgeo;
			if (geonode->isSubclassOf<CAnimatedNode>())
				newgeo = kenv.createObject<CKSkinGeometry>(sector);
			else
				newgeo = kenv.createObject<CKGeometry>(sector);
			kenv.setObjectName(newgeo, "XE Geometry");
			if (prevgeo) prevgeo->nextGeo = kobjref<CKAnyGeometry>(newgeo);
			else geonode->geometry.reset(newgeo);
			prevgeo = newgeo;
			newgeo->flags = 1;
			newgeo->flags2 = 0;
			newgeo->clump = std::make_shared<RwMiniClump>();
			newgeo->clump->atomic.flags = 5;
			newgeo->clump->atomic.unused = 0;
			newgeo->clump->atomic.geometry = std::move(rwgeo);
			if (fxaext)
				newgeo->clump->atomic.extensions.exts.push_back(std::move(fxaext));

			// Create material for XXL2+
			if (kenv.version >= kenv.KVERSION_XXL2) {
				CMaterial* mat = kenv.createObject<CMaterial>(sector);
				kenv.setObjectName(mat, "XE Material");
				mat->geometry = newgeo;
				mat->flags = 0x10;
				newgeo->material = mat;
				newgeo->lightSet = lightSetBackup;
			}
		}
	}
}
