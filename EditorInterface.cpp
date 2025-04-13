#include "EditorInterface.h"
#include "KEnvironment.h"
#include "imgui/imgui.h"
#include "imguiimpl.h"
#include <SDL2/SDL.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>
#include "rwrenderer.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKGraphical.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKHook.h"
#include <shlobj_core.h>
#include <stb_image_write.h>
#include "rwext.h"
#include <stack>
#include "imgui/ImGuizmo.h"
#include "GameLauncher.h"
#include "Shape.h"
#include "CoreClasses/CKService.h"
#include <INIReader.h>
#include "rw.h"
#include "WavDocument.h"
#include "CoreClasses/CKCinematicNode.h"
#include "KLocalObject.h"
#include "CKLocalObjectSubs.h"
#include <io.h>
#include "GuiUtils.h"
#include "LocaleEditor.h"
#include "Duplicator.h"
#include <nlohmann/json.hpp>
#include <charconv>
#include <fmt/format.h>
#include "CoreClasses/CKManager.h"
#include "GameClasses/CKGameX1.h"
#include "GameClasses/CKGameX2.h"
#include "GameClasses/CKGameOG.h"
#include "Encyclopedia.h"

#include "EditorUI/IGTextureEditor.h"
#include "EditorUI/IGPathfindingEditor.h"
#include "EditorUI/IGMarkerEditor.h"
#include "EditorUI/IGDetectorEditor.h"
#include "EditorUI/IGCinematicEditor.h"
#include "EditorUI/IGCollisionEditor.h"
#include "EditorUI/IGLineEditor.h"
#include "EditorUI/IGCameraEditor.h"
#include "EditorUI/IGCounterEditor.h"
#include "EditorUI/IGMusicEditor.h"
#include "EditorUI/IGSekensEditor.h"
#include "EditorUI/IGAnimationViewer.h"
#include "EditorUI/IGLevelInfoEditor.h"
#include "EditorUI/IGObjectInspector.h"
#include "EditorUI/IGObjectList.h"
#include "EditorUI/IGMisc.h"
#include "EditorUI/IGAbout.h"
#include "EditorUI/EditorWidgets.h"
#include "EditorUI/DictionaryEditors.h"
#include "EditorUI/EditorUtils.h"
#include "EditorUI/ImGuiMemberListener.h"
#include "EditorUI/PropFlagsEditor.h"

using namespace GuiUtils;

namespace EditorUI {

namespace {
	const int maxGameSupportingAdvancedBeaconEditing = KEnvironment::KVERSION_XXL1;

	template<typename C> std::unique_ptr<RwClump> LoadDFF(const C* filename)
	{
		std::unique_ptr<RwClump> clump = std::make_unique<RwClump>();
		IOFile dff(filename, "rb");
		auto rwverBackup = HeaderWriter::rwver; // TODO FIXME hack
		rwCheckHeader(&dff, 0x10);
		clump->deserialize(&dff);
		dff.close();
		HeaderWriter::rwver = rwverBackup;
		return clump;
	}

	const char* GetPathFilename(const char* path)
	{
		const char* ptr = path;
		const char* fnd = path;
		while (*ptr) {
			if (*ptr == '\\' || *ptr == '/')
				fnd = ptr + 1;
			ptr++;
		}
		return fnd;
	}

	std::string GetPathFilenameNoExt(const char* path)
	{
		const char* fe = GetPathFilename(path);
		const char* end = strrchr(fe, '.');
		if (end)
			return std::string(fe, end);
		return std::string(fe);
	}

	void InvertTextures(KEnvironment &kenv)
	{
		auto f = [](KObjectList &objlist) {
			CTextureDictionary *dict = (CTextureDictionary*)objlist.getClassType<CTextureDictionary>().objects[0];
			for (auto &tex : dict->piDict.textures) {
				if (uint32_t *pal = tex.images[0].palette.data())
					for (size_t i = 0; i < (size_t)(1u << tex.images[0].bpp); i++)
						pal[i] ^= 0xFFFFFF;
			}
		};
		f(kenv.levelObjects);
		for (KObjectList &ol : kenv.sectorObjects)
			f(ol);
	}

	bool IsNodeInvisible(CKSceneNode *node, bool isXXL2) {
		return isXXL2 ? ((node->unk1 & 4) && !(node->unk1 & 0x10)) : (node->unk1 & 2);
	}

	void DrawSceneNode(CKSceneNode *node, const Matrix &transform, Renderer *gfx, ProGeoCache &geocache, ProTexDict *texdict, CCloneManager *clm, bool showTextures, bool showInvisibles, bool showClones, std::map<CSGBranch*, int> &nodeCloneIndexMap, bool isXXL2)
	{
		for (; node; node = node->next.get()) {
			Matrix nodeTransform = node->transform;
			nodeTransform.m[0][3] = nodeTransform.m[1][3] = nodeTransform.m[2][3] = 0.0f;
			nodeTransform.m[3][3] = 1.0f;
			Matrix globalTransform = nodeTransform * transform;
			if (showInvisibles || !IsNodeInvisible(node, isXXL2)) {
				if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
					if (showClones) {
						//auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const kobjref<CSGBranch> &ref) {return ref.get() == node; });
						//assert(it != clm->_clones.end());
						//size_t clindex = it - clm->_clones.begin();
						int clindex = nodeCloneIndexMap.at((CSGBranch*)node);
						gfx->setTransformMatrix(globalTransform);
						for (uint32_t part : clm->_team.dongs[clindex].bongs)
							if (part != 0xFFFFFFFF) {
								RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump.atomic.geometry.get();
								geocache.getPro(rwgeo, texdict)->draw(showTextures);
							}
					}
				}
				else if (node->isSubclassOf<CNode>()) {
					gfx->setTransformMatrix(globalTransform);
					for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
						CKAnyGeometry *rgeo = kgeo->duplicateGeo ? kgeo->duplicateGeo.get() : kgeo;
						if (auto& rwminiclp = rgeo->clump)
							if (RwGeometry *rwgeo = rwminiclp->atomic.geometry.get())
								if((rwgeo->flags & RwGeometry::RWGEOFLAG_NATIVE) == 0)
									geocache.getPro(rwgeo, texdict)->draw(showTextures);
					}
				}
				if (node->isSubclassOf<CSGBranch>())
					DrawSceneNode(node->cast<CSGBranch>()->child.get(), globalTransform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones, nodeCloneIndexMap, isXXL2);
				if (CAnyAnimatedNode *anyanimnode = node->dyncast<CAnyAnimatedNode>())
					DrawSceneNode(anyanimnode->branchs.get(), globalTransform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones, nodeCloneIndexMap, isXXL2);
			}
		}
	}

	Vector3 getRay(const Camera &cam, Window *window) {
		const float zNear = 0.1f;
		Vector3 xvec = cam.direction.normal().cross(Vector3(0, 1, 0)).normal();
		Vector3 yvec = cam.direction.normal().cross(xvec).normal();
		float ys = tan(0.45f) * zNear;
		yvec *= ys;
		xvec *= ys * window->getWidth() / window->getHeight();
		yvec *= (1 - window->getMouseY() * 2.0f / window->getHeight());
		xvec *= (1 - window->getMouseX() * 2.0f / window->getWidth());
		return cam.direction.normal() * zNear - xvec - yvec;
	}

	bool rayIntersectsSphere(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &spherePos, float sphereRadius) {
		Vector3 rayDir = _rayDir.normal();
		Vector3 rs2sp = spherePos - rayStart;
		float sphereRadiusSq = sphereRadius * sphereRadius;
		if (rs2sp.sqlen3() <= sphereRadiusSq)
			return true;
		float dot = rayDir.dot(rs2sp);
		if (dot < 0.0f)
			return false;
		Vector3 shortPoint = rayStart + rayDir * dot;
		float shortDistSq = (spherePos - shortPoint).sqlen3();
		return shortDistSq <= sphereRadiusSq;
	}

	std::pair<bool, Vector3> getRaySphereIntersection(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &spherePos, float sphereRadius) {
		Vector3 rayDir = _rayDir.normal();
		Vector3 rs2sp = spherePos - rayStart;
		float sphereRadiusSq = sphereRadius * sphereRadius;
		if (rs2sp.sqlen3() <= sphereRadiusSq)
			return std::make_pair(true, rayStart);
		float dot = rayDir.dot(rs2sp);
		if (dot < 0.0f)
			return std::make_pair(false, Vector3());
		Vector3 shortPoint = rayStart + rayDir * dot;
		float shortDistSq = (spherePos - shortPoint).sqlen3();
		if (shortDistSq > sphereRadiusSq)
			return std::make_pair(false, Vector3());
		Vector3 ix = shortPoint - rayDir * sqrt(sphereRadiusSq - shortDistSq);
		return std::make_pair(true, ix);
	}

	std::pair<bool, Vector3> getRayTriangleIntersection(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3) {
		Vector3 rayDir = _rayDir.normal();
		Vector3 v2 = p2 - p1, v3 = p3 - p1;
		Vector3 trinorm = v2.cross(v3).normal(); // order?
		if (trinorm == Vector3(0, 0, 0))
			return std::make_pair(false, trinorm);
		float rayDir_dot_trinorm = rayDir.dot(trinorm);
		if (rayDir_dot_trinorm < 0.0f)
			return std::make_pair(false, Vector3(0, 0, 0));
		float p = p1.dot(trinorm);
		float alpha = (p - rayStart.dot(trinorm)) / rayDir_dot_trinorm;
		if (alpha < 0.0f)
			return std::make_pair(false, Vector3(0,0,0));
		Vector3 sex = rayStart + rayDir * alpha;

		Vector3 c = sex - p1;
		float d = v2.sqlen3() * v3.sqlen3() - v2.dot(v3) * v2.dot(v3);
		//assert(d != 0.0f);
		float a = (c.dot(v2) * v3.sqlen3() - c.dot(v3) * v2.dot(v3)) / d;
		float b = (c.dot(v3) * v2.sqlen3() - c.dot(v2) * v3.dot(v2)) / d;
		if (a >= 0.0f && b >= 0.0f && (a + b) <= 1.0f)
			return std::make_pair(true, sex);
		else
			return std::make_pair(false, Vector3(0, 0, 0));
	}

	bool isPointInAABB(const Vector3 &point, const Vector3 &highCorner, const Vector3 &lowCorner) {
		for (int i = 0; i < 3; i++)
			if (point.coord[i] < lowCorner.coord[i] || point.coord[i] > highCorner.coord[i])
				return false;
		return true;
	}

	std::pair<bool, Vector3> getRayAABBIntersection(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &highCorner, const Vector3 &lowCorner) {
		if (isPointInAABB(rayStart, highCorner, lowCorner))
			return std::make_pair(true, rayStart);
		Vector3 rayDir = _rayDir.normal();
		for (int i = 0; i < 3; i++) {
			if (rayDir.coord[i] != 0.0f) {
				int j = (i + 1) % 3, k = (i + 2) % 3;
				for (const std::pair<const Vector3 &, float>& pe : { std::make_pair(highCorner,1.0f), std::make_pair(lowCorner,-1.0f) }) {
					if (rayDir.coord[i] * pe.second > 0)
						continue;
					float t = (pe.first.coord[i] - rayStart.coord[i]) / rayDir.coord[i];
					Vector3 candidate = rayStart + rayDir * t;
					if (candidate.coord[j] >= lowCorner.coord[j]  && candidate.coord[k] >= lowCorner.coord[k] &&
						candidate.coord[j] <= highCorner.coord[j] && candidate.coord[k] <= highCorner.coord[k])
						return std::make_pair(true, candidate);
				}
			}
		}
		return std::make_pair(false, Vector3(0,0,0));
	}

	RwClump CreateClumpFromGeo(std::shared_ptr<RwGeometry> rwgeo, RwExtHAnim *hanim = nullptr) {
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

	RwGeometry createEmptyGeo() {
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

	void ImportGroundOBJ(KEnvironment &kenv, const std::filesystem::path& filename, int sector) {
		KObjectList& objlist = (sector == -1) ? kenv.levelObjects : kenv.sectorObjects[sector];
		CKMeshKluster* kluster = objlist.getFirst<CKMeshKluster>();
		CKSector* ksector = kenv.levelObjects.getClassType<CKSector>().objects[sector + 1]->cast<CKSector>();

		std::map<std::string, CGround*> groundMap;
		for (auto& gnd : kluster->grounds) {
			if (!gnd->isSubclassOf<CDynamicGround>()) {
				groundMap[kenv.getObjectName(gnd.get())] = gnd.get();
			}
		}

		FILE *wobj;
		fsfopen_s(&wobj, filename, "rt");
		if (!wobj) return;
		char line[512]; char* context = nullptr; const char* const spaces = " \t\r\n";
		std::vector<Vector3> positions;
		std::vector<CGround::Triangle> triangles; // change int16 to int32 ??
		std::vector<std::array<int, 4>> walls;
		std::set<int> floorIndices; // in if vertex index used in some ground floor, not in if only used in walls
		CGround* currentGround = nullptr;
		std::string nextGroundName;
		auto flushTriangles = [&positions, &triangles, &kenv, &sector, &currentGround, kluster, ksector, &nextGroundName, &walls, &floorIndices]() {
			if (!triangles.empty()) {
				CGround* gnd = currentGround;
				if (!gnd) {
					gnd = kenv.createObject<CGround>(sector);
					kluster->grounds.emplace_back(gnd);
					kenv.setObjectName(gnd, std::move(nextGroundName));
					gnd->x2sectorObj = ksector;
				}
				gnd->vertices.clear();
				gnd->triangles.clear();
				gnd->aabb = {};
				gnd->finiteWalls.clear();
				gnd->infiniteWalls.clear();

				std::map<int, int> idxmap;
				uint16_t nextIndex = 0;
				for (auto &tri : triangles) {
					CGround::Triangle cvtri;
					for (int c = 0; c < 3; c++) {
						int objIndex = tri.indices[c];
						int cvIndex;
						const Vector3 &objPos = positions[objIndex];

						auto pmit = idxmap.find(objIndex);
						if (pmit == idxmap.end()) {
							idxmap[objIndex] = nextIndex;
							gnd->vertices.push_back(objPos);
							cvIndex = nextIndex++;
						}
						else {
							cvIndex = pmit->second;
						}

						cvtri.indices[c] = cvIndex;
					}
					gnd->triangles.push_back(std::move(cvtri));
				}
				for (auto& wall : walls) {
					auto sorted = wall;
					size_t cnt = std::count_if(sorted.begin(), sorted.end(), [&floorIndices](int x) {return floorIndices.count(x); });
					if (cnt == 2) {
						// sorting after the inclusion of index in floorIndices first, and those that aren't
						std::sort(sorted.begin(), sorted.end(), [&floorIndices](int a, int b) {return floorIndices.count(a) > floorIndices.count(b); });
					}
					else {
						// sorting after the height of the vertices, first the bottom of the wall which becomes the base, then the top
						std::sort(sorted.begin(), sorted.end(), [&positions](int a, int b) {return positions[a].y < positions[b].y; });
					}
					// after sorting, sorted[0] and sorted[1] are the base of the wall, sorted[2] and sorted[3] are the top/bottom of wall
					int oriindex0 = std::find(wall.begin(), wall.end(), sorted[0]) - wall.begin();
					int oriindex1 = std::find(wall.begin(), wall.end(), sorted[1]) - wall.begin();
					// swap to keep order and front/backface
					if (((oriindex1 - oriindex0) & 3) == 3) {
						std::swap(sorted[0], sorted[1]);
					}
					// swap so that sorted[0] and [2] are base and top/bottom corresponding to each other, on same XZ
					float swap_no_dist = (positions[sorted[2]] - positions[sorted[0]]).len2xz() + (positions[sorted[3]] - positions[sorted[1]]).len2xz();
					float swap_yes_dist = (positions[sorted[3]] - positions[sorted[0]]).len2xz() + (positions[sorted[2]] - positions[sorted[1]]).len2xz();
					if (swap_yes_dist < swap_no_dist) {
						std::swap(sorted[2], sorted[3]);
					}

					CGround::FiniteWall finWall;
					for (int c = 0; c < 2; c++) {
						int objIndex = sorted[c];
						int cvIndex;
						const Vector3& objPos = positions[objIndex];

						auto pmit = idxmap.find(objIndex);
						if (pmit == idxmap.end()) {
							idxmap[objIndex] = nextIndex;
							gnd->vertices.push_back(objPos);
							cvIndex = nextIndex++;
						}
						else {
							cvIndex = pmit->second;
						}

						finWall.baseIndices[c] = cvIndex;
					}
					finWall.heights[0] = positions[sorted[2]].y - positions[sorted[0]].y;
					finWall.heights[1] = positions[sorted[3]].y - positions[sorted[1]].y;
					// flip wall except for walls with negative heights
					if (!(finWall.heights[0] < 0.0f && finWall.heights[1] < 0.0f)) {
						std::swap(finWall.baseIndices[0], finWall.baseIndices[1]);
						std::swap(finWall.heights[0], finWall.heights[1]);
					}
					gnd->finiteWalls.push_back(std::move(finWall));
				}
				gnd->aabb = AABoundingBox(gnd->vertices[0]);
				for (Vector3 &vec : gnd->vertices) {
					gnd->aabb.mergePoint(vec);
				}
				AABoundingBox safeaabb = gnd->aabb;
				safeaabb.highCorner.y += 10.0f;
				safeaabb.lowCorner.y -= 5.0f;
				kluster->aabb.merge(safeaabb);
				ksector->boundaries.merge(safeaabb);

				// Compute negative height extension (param3)
				float minheight = gnd->aabb.lowCorner.y;
				for (auto& wall : gnd->finiteWalls) {
					float h0 = gnd->vertices[wall.baseIndices[0]].y + wall.heights[0];
					float h1 = gnd->vertices[wall.baseIndices[1]].y + wall.heights[1];
					minheight = std::min({ minheight, h0, h1 });
				}
				gnd->param3 = minheight - gnd->aabb.lowCorner.y;
			}
			currentGround = nullptr;
			triangles.clear();
			walls.clear();
			floorIndices.clear();
		};
		while (!feof(wobj)) {
			fgets(line, 511, wobj);
			std::string word = strtok_s(line, spaces, &context);
			if (word == "o") {
				flushTriangles();
				std::string name = strtok_s(NULL, spaces, &context);
				auto it = groundMap.find(name);
				if (it != groundMap.end())
					currentGround = it->second;
				else
					nextGroundName = std::move(name);
			}
			else if (word == "v") {
				Vector3 vec;
				for (float &coord : vec) {
					coord = std::strtof(strtok_s(NULL, spaces, &context), nullptr);
				}
				positions.push_back(vec);
			}
			else if (word == "f") {
				std::vector<uint16_t> face;
				while (char *arg = strtok_s(NULL, spaces, &context)) {
					int index = 1;
					sscanf_s(arg, "%i", &index);
					if (index < 0) index += positions.size();
					else index -= 1;
					face.push_back(index);
				}
				bool isWall = false;
				if (face.size() == 4) {
					Vector3 v1 = positions[face[0]] - positions[face[2]];
					Vector3 v2 = positions[face[1]] - positions[face[3]];
					Vector3 norm = v1.cross(v2).normal();
					if (std::abs(norm.dot(Vector3(0, 1, 0))) < 0.001f) {
						// It's a wall!
						walls.push_back({ face[0], face[1], face[2], face[3] });
						isWall = true;
					}
				}
				if (!isWall) {
					// It's a ground!
					for (size_t i = 2; i < face.size(); i++) {
						CGround::Triangle tri;
						tri.indices = { face[0], face[i - 1], face[i] };
						triangles.push_back(std::move(tri));
					}
					for (auto& index : face) {
						floorIndices.insert(index);
					}
				}
			}
		}
		flushTriangles();
		fclose(wobj);
	}

	void ChangeNodeGeometry(KEnvironment& kenv, CNode* geonode, RwGeometry** rwgeos, size_t numRwgeos) {
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

	float decode8bitAngle(uint8_t byte) {
		return byte * M_PI / 128.0f;
	}
}

// Selection classes

struct NodeSelection : UISelection {
	static const int ID = 1;

	KWeakRef<CKSceneNode> node;

	NodeSelection(EditorInterface &ui, Vector3 &hitpos, CKSceneNode *node) : UISelection(ui, hitpos), node(node) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return node.get() != nullptr; }
	Matrix getTransform() override {
		Matrix mat = node->transform;
		for (int i = 0; i < 4; i++)
			mat.m[i][3] = (i == 3) ? 1.0f : 0.0f;
		return mat;
	}
	void setTransform(const Matrix &mat) override { node->transform = mat; }
	void onSelected() override {
		CKSceneNode* node = this->node.get();
		ui.selNode = node;
		// Find hook attached to node
		for (auto& hkclass : ui.kenv.levelObjects.categories[CKHook::CATEGORY].type) {
			for (CKObject* obj : hkclass.objects) {
				CKHook* hook = obj->dyncast<CKHook>();
				if (hook && hook->node.bound) {
					if (hook->node.get() == node) {
						ui.selectedHook = hook;
						ui.viewGroupInsteadOfHook = false;
					}
				}
			}
		}
	}
	std::string getInfo() override {
		if (node)
			return fmt::format("Node {} ({})", ui.kenv.getObjectName(node.get()), node->getClassName());
		else
			return "Node removed";
	}
	void onDetails() override {
		ui.selNode = node;
		ui.wndShowSceneGraph = true;
	}
};

struct BeaconSelection : UISelection {
	static const int ID = 2;

	int sectorIndex, klusterIndex, bingIndex, beaconIndex;

	BeaconSelection(EditorInterface& ui, Vector3& hitpos, int sectorIndex, int klusterIndex, int bingIndex, int beaconIndex) :
		UISelection(ui, hitpos), sectorIndex(sectorIndex), klusterIndex(klusterIndex), bingIndex(bingIndex), beaconIndex(beaconIndex) {}

	CKBeaconKluster* getKluster() const {
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		if (sectorIndex >= 0 && sectorIndex <= (int)srvBeacon->beaconSectors.size())
			if (klusterIndex >= 0 && klusterIndex < (int)srvBeacon->beaconSectors[sectorIndex].beaconKlusters.size())
				return srvBeacon->beaconSectors[sectorIndex].beaconKlusters[klusterIndex].get();
		return nullptr;
	}

	SBeacon* getBeaconPtr() const {
		if (CKBeaconKluster* kluster = getKluster())
			if (bingIndex >= 0 && bingIndex < (int)kluster->bings.size())
				if (beaconIndex >= 0 && beaconIndex < (int)kluster->bings[bingIndex].beacons.size())
					return &kluster->bings[bingIndex].beacons[beaconIndex];
		return nullptr;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return getBeaconPtr() != nullptr; }
	Matrix getTransform() override {
		Matrix mat = Matrix::getTranslationMatrix(getBeaconPtr()->getPosition());
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		if (auto* beaconInfo = ui.g_encyclo.getBeaconJson(srvBeacon->handlers[bingIndex].handlerId)) {
			if (beaconInfo->is_object() && beaconInfo->value<bool>("orientable", false)) {
				mat = Matrix::getRotationYMatrix(decode8bitAngle(getBeaconPtr()->params & 255)) * mat;
			}
		}
		return mat;
	}
	void setTransform(const Matrix &mat) override {
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		if (ui.kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
			SBeacon beacon = *getBeaconPtr();
			srvBeacon->removeBeacon(sectorIndex, klusterIndex, bingIndex, beaconIndex);
			srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[sectorIndex].beaconKlusters[klusterIndex].get());
			srvBeacon->cleanEmptyKlusters(ui.kenv, sectorIndex);
			beacon.setPosition(mat.getTranslationVector());
			if (auto* beaconInfo = ui.g_encyclo.getBeaconJson(srvBeacon->handlers[bingIndex].handlerId)) {
				if (beaconInfo->is_object() && beaconInfo->value<bool>("orientable", false)) {
					const float angle = std::atan2(mat._31, mat._11);
					beacon.params = (beacon.params & 0xFF00) | (uint8_t)std::round(angle * 128.0f / M_PI);
				}
			}
			std::tie(klusterIndex, beaconIndex) = srvBeacon->addBeaconToNearestKluster(ui.kenv, sectorIndex, bingIndex, beacon);
			std::tie(ui.selBeaconKluster, ui.selBeaconIndex) = std::tie(klusterIndex, beaconIndex); // bad
			srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[sectorIndex].beaconKlusters[klusterIndex].get());
		}
		else {
			auto* beacon = getBeaconPtr();
			beacon->setPosition(mat.getTranslationVector());
			if (auto* beaconInfo = ui.g_encyclo.getBeaconJson(srvBeacon->handlers[bingIndex].handlerId)) {
				if (beaconInfo->is_object() && beaconInfo->value<bool>("orientable", false)) {
					const float angle = std::atan2(mat._31, mat._11);
					beacon->params = (beacon->params & 0xFF00) | (uint8_t)(int)(angle * 128.0f / M_PI);
				}
			}
			ui.kenv.levelObjects.getFirst<CKSrvBeacon>()->updateKlusterBounds(getKluster());
		}
	}

	void duplicate() override {
		if (!hasTransform()) return;
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		const SBeacon* originalBeacon = getBeaconPtr();
		int dupKlusterIndex = klusterIndex;
		if (ui.kenv.version > maxGameSupportingAdvancedBeaconEditing) {
			dupKlusterIndex = srvBeacon->addKluster(ui.kenv, sectorIndex);
		}
		srvBeacon->addBeacon(sectorIndex, dupKlusterIndex, bingIndex, *originalBeacon);
		srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[sectorIndex].beaconKlusters[dupKlusterIndex].get());
	}
	bool remove() override {
		if (!hasTransform()) return false;
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		srvBeacon->removeBeacon(sectorIndex, klusterIndex, bingIndex, beaconIndex);
		if (ui.kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
			srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[sectorIndex].beaconKlusters[klusterIndex].get());
			srvBeacon->cleanEmptyKlusters(ui.kenv, sectorIndex);
		}
		ui.selBeaconSector = -1;
		return true;
	}

	void onSelected() override {
		ui.selBeaconSector = sectorIndex;
		ui.selBeaconKluster = klusterIndex;
		ui.selBeaconBing = bingIndex;
		ui.selBeaconIndex = beaconIndex;
	}

	std::string getInfo() override {
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		return fmt::format("Beacon {}", ui.g_encyclo.getBeaconName(srvBeacon->handlers[bingIndex].handlerId));
	}
	void onDetails() override {
		onSelected();
		ui.wndShowBeacons = true;
	}
};

struct GroundSelection : UISelection {
	static const int ID = 3;

	KWeakRef<CGround> ground;

	GroundSelection(EditorInterface &ui, Vector3 &hitpos, CGround *gnd) : UISelection(ui, hitpos), ground(gnd) {}
	
	int getTypeID() override { return ID; }
	void onSelected() override { ui.selGround = ground; }
	std::string getInfo() override { return fmt::format("Ground {}", ground ? ui.kenv.getObjectName(ground.get()) : "removed"); }
	void onDetails() override { onSelected(); ui.wndShowGrounds = true; }
};

struct SquadSelection : UISelection {
	static const int ID = 4;

	KWeakRef<CKGrpSquadEnemy> squad;

	SquadSelection(EditorInterface &ui, Vector3 &hitpos, CKGrpSquadEnemy *squad) : UISelection(ui, hitpos), squad(squad) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return squad.get(); }
	Matrix getTransform() override { return squad->mat1; }
	void setTransform(const Matrix &mat) override { squad->mat1 = mat; }
	void onSelected() override { ui.selectedSquad = squad; }
	std::string getInfo() override { return fmt::format("Squad {}", squad ? ui.kenv.getObjectName(squad.get()) : "Removed"); }
	void onDetails() override { onSelected(); ui.wndShowSquads = true; }
};

struct ChoreoSpotSelection : UISelection {
	static const int ID = 5;

	KWeakRef<CKGrpSquadEnemy> squad; int spotIndex;

	ChoreoSpotSelection(EditorInterface &ui, Vector3 &hitpos, CKGrpSquadEnemy *squad, int spotIndex) : UISelection(ui, hitpos), squad(squad), spotIndex(spotIndex) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override {
		if (!squad) return false;
		if (ui.showingChoreoKey < 0 || ui.showingChoreoKey >= (int)squad->choreoKeys.size()) return false;
		if (spotIndex < 0 || spotIndex >= (int)squad->choreoKeys[ui.showingChoreoKey]->slots.size()) return false;
		return true;
	}
	Matrix getTransform() override {
		auto& spot = squad->choreoKeys[ui.showingChoreoKey]->slots[spotIndex];
		Matrix mRot = Matrix::getIdentity();
		Vector3 v1 = spot.direction.normal();
		Vector3 v3 = v1.cross(Vector3(0.0f, 1.0f, 0.0f));
		const Vector3& v4 = spot.position;
		std::tie(mRot._11, mRot._12, mRot._13) = std::tie(v1.x, v1.y, v1.z);
		std::tie(mRot._31, mRot._32, mRot._33) = std::tie(v3.x, v3.y, v3.z);
		std::tie(mRot._41, mRot._42, mRot._43) = std::tie(v4.x, v4.y, v4.z);
		return mRot * squad->mat1;
	}
	void setTransform(const Matrix &mat) override {
		Matrix inv = squad->mat1.getInverse4x4();
		Matrix spotMat = mat * inv;
		auto& spot = squad->choreoKeys[ui.showingChoreoKey]->slots[spotIndex];
		spot.position = spotMat.getTranslationVector();
		spot.direction = Vector3(spotMat._11, spotMat._12, spotMat._13);
	}

	void duplicate() override {
		if (!hasTransform()) return;
		auto& slots = squad->choreoKeys[ui.showingChoreoKey]->slots;
		slots.push_back(slots[spotIndex]);
	}
	bool remove() override {
		if (!hasTransform()) return false;
		auto& slots = squad->choreoKeys[ui.showingChoreoKey]->slots;
		slots.erase(slots.begin() + spotIndex);
		return true;
	}
	void onSelected() override { ui.selectedSquad = squad; }
	std::string getInfo() override { return fmt::format("Choreo spot {} from Squad {}", spotIndex, squad ? ui.kenv.getObjectName(squad.get()) : "removed"); }
	void onDetails() override { onSelected(); ui.wndShowSquads = true; }
};

struct MarkerSelection : UISelection {
	static const int ID = 6;

	int markerIndex;

	MarkerSelection(EditorInterface &ui, Vector3 &hitpos, int markerIndex) : UISelection(ui, hitpos), markerIndex(markerIndex) {}

	CKSrvMarker::Marker* getMarker() const {
		auto* srvMarker = ui.kenv.levelObjects.getFirst<CKSrvMarker>();
		if (!srvMarker || srvMarker->lists.empty()) return nullptr;
		auto& list = srvMarker->lists[0];
		if (markerIndex >= 0 && markerIndex < list.size())
			return &list[markerIndex];
		return nullptr;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return getMarker() != nullptr; }
	Matrix getTransform() override {
		auto* marker = getMarker();
		return Matrix::getRotationYMatrix(decode8bitAngle(marker->orientation1)) * Matrix::getTranslationMatrix(marker->position);
	}
	void setTransform(const Matrix &mat) override {
		auto* marker = getMarker();
		marker->position = mat.getTranslationVector();
		const float angle = std::atan2(mat._31, mat._11);
		marker->orientation1 = (uint8_t)std::round(angle * 128.0f / M_PI);
		marker->orientation2 = 0;
	}
	void onSelected() override { ui.selectedMarkerIndex = markerIndex; }
	std::string getInfo() override {
		auto* marker = getMarker();
		return fmt::format("Marker {}: {}", markerIndex, marker ? marker->name : "OOB");
	}
	void onDetails() override { onSelected(); ui.wndShowMarkers = true; }
};

struct HkLightSelection : UISelection {
	static const int ID = 7;

	CKGrpLight* grpLight;
	int lightIndex;

	HkLightSelection(EditorInterface& ui, Vector3& hitpos, CKGrpLight* grpLight, int lightIndex) : UISelection(ui, hitpos), grpLight(grpLight), lightIndex(lightIndex) {}

	Vector3& position() { return grpLight->node->cast<CNode>()->geometry->cast<CKParticleGeometry>()->pgPoints[lightIndex]; }
	CKHook* getHook() {
		int i = 0;
		for (CKHook* hook = grpLight->childHook.get(); hook; hook = hook->next.get()) {
			if (i++ == lightIndex) {
				return hook;
			}
		}
		return nullptr;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return true; }
	Matrix getTransform() override { return Matrix::getTranslationMatrix(position()); }
	void setTransform(const Matrix& mat) override { position() = mat.getTranslationVector(); }
	void onSelected() override {
		if (CKHook* hook = getHook()) {
			ui.selectedHook = hook;
			ui.viewGroupInsteadOfHook = false;
		}
	}
	std::string getInfo() override {
		if (CKHook* hook = getHook()) {
			return fmt::format("Light Hook {} ({})", ui.kenv.getObjectName(hook), lightIndex);
		}
		return "Light Hook ???";
	}
	void onDetails() override { onSelected(); ui.wndShowHooks = true; }
};

struct X1DetectorSelection : UISelection {
	static const int ID = 8;
	enum ShapeType {
		BOUNDINGBOX = 0,
		SPHERE = 1,
		RECTANGLE = 2,
	};
	ShapeType type;
	size_t index;
	Vector3 bbCenter, bbHalf;
	X1DetectorSelection(EditorInterface& ui, Vector3& hitpos, ShapeType type, size_t index) : UISelection(ui, hitpos), type(type), index(index) {}
	CKSrvDetector* getSrvDetector() { return ui.kenv.levelObjects.getFirst<CKSrvDetector>(); }
	Vector3& position() {
		CKSrvDetector* srvDetector = getSrvDetector();
		if (type == BOUNDINGBOX)
			return bbCenter;
		else if (type == SPHERE)
			return srvDetector->spheres[index].center;
		else if (type == RECTANGLE)
			return srvDetector->rectangles[index].center;
		return bbCenter;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override {
		CKSrvDetector* srvDetector = getSrvDetector();
		size_t counts[3] = { srvDetector->aaBoundingBoxes.size(), srvDetector->spheres.size(), srvDetector->rectangles.size() };
		return index >= 0 && index < counts[type];
	}
	Matrix getTransform() override {
		if (type == BOUNDINGBOX) {
			auto& bb = getSrvDetector()->aaBoundingBoxes[index];
			bbCenter = (bb.highCorner + bb.lowCorner) * 0.5f;
			bbHalf = (bb.highCorner - bb.lowCorner) * 0.5f;
		}
		return Matrix::getTranslationMatrix(position());
	}
	void setTransform(const Matrix& mat) override {
		position() = mat.getTranslationVector();
		if (type == BOUNDINGBOX) {
			auto& bb = getSrvDetector()->aaBoundingBoxes[index];
			bb.highCorner = bbCenter + bbHalf;
			bb.lowCorner = bbCenter - bbHalf;
		}
	}
	void onSelected() override {
		ui.selectedShapeType = type;
		ui.selectedShapeIndex = index;
	}
	std::string getInfo() override {
		CKSrvDetector* srvDetector = getSrvDetector();
		if (type == BOUNDINGBOX)
			return "Box Detector " + srvDetector->aabbNames[index];
		else if (type == SPHERE)
			return "Sphere Detector " + srvDetector->sphNames[index];
		else if (type == RECTANGLE)
			return "Rectangle Detector " + srvDetector->rectNames[index];
		return "Unknown Detector";
	}
	void onDetails() override { onSelected(); ui.wndShowDetectors = true; }
};

struct X2DetectorSelection : UISelection {
	static const int ID = 9;

	KWeakRef<CMultiGeometryBasic> geometry;
	KWeakRef<CKDetectorBase> detector;
	Vector3 bbCenter, bbHalf;

	X2DetectorSelection(EditorInterface& ui, Vector3& hitpos, CMultiGeometryBasic* geometry, CKDetectorBase* detector) :
		UISelection(ui, hitpos), geometry(geometry), detector(detector) {}
	Vector3& position() {
		if (std::holds_alternative<AABoundingBox>(geometry->mgShape))
			return bbCenter;
		else if (auto* sphere = std::get_if<BoundingSphere>(&geometry->mgShape))
			return sphere->center;
		else if (auto* rect = std::get_if<AARectangle>(&geometry->mgShape))
			return rect->center;
		return bbCenter;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override {
		return geometry.get() != nullptr;
	}
	Matrix getTransform() override {
		if (auto* bb = std::get_if<AABoundingBox>(&geometry->mgShape)) {
			bbCenter = (bb->highCorner + bb->lowCorner) * 0.5f;
			bbHalf = (bb->highCorner - bb->lowCorner) * 0.5f;
		}
		return Matrix::getTranslationMatrix(position());
	}
	void setTransform(const Matrix& mat) override {
		position() = mat.getTranslationVector();
		if (auto* bb = std::get_if<AABoundingBox>(&geometry->mgShape)) {
			bb->highCorner = bbCenter + bbHalf;
			bb->lowCorner = bbCenter - bbHalf;
		}
	}
	void onSelected() override {
		ui.selectedX2Detector = detector;
	}
	std::string getInfo() override {
		std::string name = ui.kenv.getObjectName(geometry.get());
		if (geometry->mgShape.index() == 0)
			return "Box Detector " + name;
		else if (geometry->mgShape.index() == 1)
			return "Sphere Detector " + name;
		else if (geometry->mgShape.index() == 2)
			return "Rectangle Detector " + name;
		return "Unknown Detector " + name;
	}
	void onDetails() override { onSelected(); ui.wndShowDetectors = true; }
};

EditorInterface::EditorInterface(KEnvironment & kenv, Window * window, Renderer * gfx, const std::string & gameModule)
	: kenv(kenv), g_window(window), gfx(gfx), protexdict(gfx), progeocache(gfx), gndmdlcache(gfx),
	launcher(gameModule, kenv.outGamePath, kenv.version)
{
	lastFpsTime = SDL_GetTicks() / 1000;

	auto loadModel = [](const char *fn) {
		auto clp = std::make_unique<RwClump>();
		File *dff = GetResourceFile(fn);
		rwCheckHeader(dff, 0x10);
		clp->deserialize(dff);
		delete dff;
		return clp;
	};

	auto origRwVer = HeaderWriter::rwver; // backup Renderware vesion
	sphereModel = loadModel("sphere.dff");
	swordModel = loadModel("sword.dff");
	spawnStarModel = loadModel("SpawnStar.dff");

	HeaderWriter::rwver = origRwVer;

	g_encyclo.setKVersion(kenv.version);
	g_encyclo.window = g_window;
}

EditorInterface::~EditorInterface() = default;

void EditorInterface::prepareLevelGfx()
{
	if (kenv.hasClass<CTextureDictionary>()) {
		protexdict.reset(kenv.levelObjects.getObject<CTextureDictionary>(0));
		str_protexdicts.clear();
		str_protexdicts.reserve((size_t)kenv.numSectors);
		for (int i = 0; i < (int)kenv.numSectors; i++) {
			ProTexDict strptd(gfx, kenv.sectorObjects[i].getObject<CTextureDictionary>(0));
			strptd._next = &protexdict;
			str_protexdicts.push_back(std::move(strptd));
			//printf("should be zero: %i\n", strptd.dict.size());
		}
	}
	nodeCloneIndexMap.clear();
	cloneSet.clear();
	if (kenv.hasClass<CCloneManager>()) {
		if (CCloneManager* cloneMgr = kenv.levelObjects.getFirst<CCloneManager>()) {
			if (cloneMgr->_numClones > 0) {
				for (int i = 0; i < (int)cloneMgr->_clones.size(); i++)
					nodeCloneIndexMap.insert({ cloneMgr->_clones[i].get(), i });
				for (auto& dong : cloneMgr->_team.dongs)
					cloneSet.insert(dong.bongs);
			}
		}
	}
}

void EditorInterface::iter()
{
	// FPS Counter
	framesInSecond++;
	uint32_t sec = SDL_GetTicks() / 1000;
	if (sec != lastFpsTime) {
		lastFps = framesInSecond;
		framesInSecond = 0;
		lastFpsTime = sec;
	}

	// Camera update and movement
	static auto lastTicks = SDL_GetTicks();
	const auto nowTicks = SDL_GetTicks();
	camera.aspect = (float)g_window->getWidth() / g_window->getHeight();
	camera.updateMatrix();
	float camspeed = _camspeed * (nowTicks - lastTicks) / 1000.0f;
	if (ImGui::GetIO().KeyShift)
		camspeed *= 0.5f;
	Vector3 camside = camera.direction.cross(Vector3(0, 1, 0)).normal();
	Vector3 camuxs = -camside.cross(Vector3(0, 1, 0)).normal();
	if (g_window->getKeyDown(SDL_SCANCODE_UP) || g_window->getKeyDown(SDL_SCANCODE_W))
		camera.position += (ImGui::GetIO().KeyCtrl ? camuxs : camera.direction) * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_DOWN) || g_window->getKeyDown(SDL_SCANCODE_S))
		camera.position -= (ImGui::GetIO().KeyCtrl ? camuxs : camera.direction) * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_RIGHT) || g_window->getKeyDown(SDL_SCANCODE_D))
		camera.position += camside * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_LEFT) || g_window->getKeyDown(SDL_SCANCODE_A))
		camera.position -= camside * camspeed;
	lastTicks = nowTicks;

	// Camera rotation
	static bool rotating = false;
	static int rotStartX, rotStartY;
	static Vector3 rotOrigOri;
	if (g_window->getKeyDown(SDL_SCANCODE_KP_0) || g_window->getMouseDown(SDL_BUTTON_RIGHT)) {
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

	// Mouse ray selection
	if (g_window->getMousePressed(SDL_BUTTON_LEFT)) {
		checkMouseRay();
		if (nearestRayHit) {
			nearestRayHit->onSelected();
		}
	}

	// Selection operation keys
	if (nearestRayHit) {
		if (g_window->isAltPressed() && g_window->getKeyPressed(SDL_SCANCODE_C)) {
			nearestRayHit->duplicate();
		}
		if (g_window->getKeyPressed(SDL_SCANCODE_DELETE)) {
			bool removed = nearestRayHit->remove();
			if (removed) {
				rayHits.clear();
				nearestRayHit = nullptr;
			}
		}
	}

	// ImGuizmo
	static int gzoperation = ImGuizmo::TRANSLATE;
	if (g_window->getKeyPressed(SDL_SCANCODE_E)) guizmoOperation = 0;
	if (g_window->getKeyPressed(SDL_SCANCODE_R)) guizmoOperation = 1;
	if (g_window->getKeyPressed(SDL_SCANCODE_T)) guizmoOperation = 2;
	if (!ImGuizmo::IsUsing())
		gzoperation = (g_window->isCtrlPressed() || guizmoOperation == 1) ? ImGuizmo::ROTATE : ((g_window->isShiftPressed() || guizmoOperation == 2) ? ImGuizmo::SCALE : ImGuizmo::TRANSLATE);
	ImGuizmo::BeginFrame();
	ImGuizmo::SetRect(0.0f, 0.0f, (float)g_window->getWidth(), (float)g_window->getHeight());

	auto* selection = nearestRayHit;
	if (selection && selection->hasTransform()) {
		static Matrix gzmat = Matrix::getIdentity();
		if (!ImGuizmo::IsUsing() || gzoperation == ImGuizmo::TRANSLATE) {
			gzmat = selection->getTransform();
		}

		Matrix originalMat = gzmat;
		const float snapAngle = 15.0f;
		const float* snap = (gzoperation == ImGuizmo::ROTATE && g_window->isAltPressed()) ? &snapAngle : nullptr;
		ImGuizmo::Manipulate(camera.viewMatrix.v, camera.projMatrix.v, (ImGuizmo::OPERATION)gzoperation, ImGuizmo::WORLD, gzmat.v, nullptr, snap);
		if (gzmat != originalMat)
			selection->setTransform(gzmat);
	}

	// Menu bar

	static bool tbIconsLoaded = false;
	static texture_t tbTexture = nullptr;
	static texture_t helpTexture = nullptr;
	if (!tbIconsLoaded) {
		auto [ptr, siz] = GetResourceContent("ToolbarIcons.png");
		RwImage img = RwImage::loadFromMemory(ptr, siz);
		tbTexture = gfx->createTexture(img);
		std::tie(ptr, siz) = GetResourceContent("HelpMarker.png");
		img = RwImage::loadFromMemory(ptr, siz);
		helpTexture = gfx->createTexture(img);
		tbIconsLoaded = true;
	}

	ImGui::BeginMainMenuBar();
/*	if (ImGui::BeginMenu("Window")) {
		ImGui::MenuItem("Main", nullptr, &wndShowMain);
		ImGui::MenuItem("Scene graph", nullptr, &wndShowSceneGraph);
		ImGui::MenuItem("Beacons", nullptr, &wndShowBeacons);
		ImGui::MenuItem("Grounds", nullptr, &wndShowGrounds);
		ImGui::Separator();
		ImGui::MenuItem("Textures", nullptr, &wndShowTextures);
		ImGui::MenuItem("Clones", nullptr, &wndShowClones);
		ImGui::MenuItem("Sounds", nullptr, &wndShowSounds);
		ImGui::Separator();
		ImGui::MenuItem("Hooks", nullptr, &wndShowHooks);
		ImGui::MenuItem("Squads", nullptr, &wndShowSquads);
		ImGui::Separator();
		if (kenv.version <= kenv.KVERSION_XXL1)
			ImGui::MenuItem("Events", nullptr, &wndShowEvents);
		else
			ImGui::MenuItem("Triggers", nullptr, &wndShowTriggers);
		ImGui::MenuItem("Detectors", nullptr, &wndShowDetectors);
		ImGui::Separator();
		ImGui::MenuItem("Pathfinding", nullptr, &wndShowPathfinding);
		if(kenv.version <= kenv.KVERSION_XXL1)
			ImGui::MenuItem("Markers", nullptr, &wndShowMarkers);
		ImGui::MenuItem("Cinematic", nullptr, &wndShowCinematic);
		ImGui::MenuItem("Collision", nullptr, &wndShowCollision);
		ImGui::MenuItem("Lines", nullptr, &wndShowLines);
		ImGui::Separator();
		ImGui::MenuItem("Localization", nullptr, &wndShowLocale);
		ImGui::MenuItem("Objects", nullptr, &wndShowObjects);
		ImGui::MenuItem("Misc", nullptr, &wndShowMisc);
		ImGui::EndMenu();
	}
	*/
	static bool toolbarCollapsed = false;
	static float toolbarIconSize = 48.0f;
	if (ImGui::ArrowButton("ToolbarCollapse", toolbarCollapsed ? ImGuiDir_Right : ImGuiDir_Down))
		toolbarCollapsed = !toolbarCollapsed;
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s toolbar", toolbarCollapsed ? "Show" : "Hide");
	ImVec2 respos = ImGui::GetCursorScreenPos();
	float reslen = ImGui::GetFrameHeight();
	if (ImGui::Button("##ToolbarResizeIcons", ImVec2(reslen, reslen)))
		toolbarIconSize = (toolbarIconSize >= 48.0f) ? 32.0f : 48.0f;
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Resize toolbar icons");
	ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(respos.x + reslen * 0.5f, respos.y + reslen * 0.5f), reslen * ((toolbarIconSize >= 48.0f) ? 0.35f : 0.2f), -1);
	ImGui::Spacing();
#ifdef XEC_APPVEYOR
	ImGui::Text("XXL Editor v" XEC_APPVEYOR " (" __DATE__ ") by AdrienTD, FPS %i", lastFps);
#else
	ImGui::Text("XXL Editor Development version, by AdrienTD, FPS: %i", lastFps);
#endif
	const char* needhelp = "Need help?";
	ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 13 - ImGui::CalcTextSize(needhelp).x - ImGui::GetStyle().ItemSpacing.x);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemInnerSpacing.y);
	ImGui::Image(helpTexture, ImVec2(13, 13));
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemInnerSpacing.y);
	IGLink(needhelp, L"https://github.com/AdrienTD/XXL-Editor/wiki", g_window);
	ImGui::EndMainMenuBar();

	// Toolbar

	static int windowOpenCounter = -1;

	float BUTTON_SIZE = toolbarIconSize;
	static constexpr float CATEGORY_SEPARATION = 8.0f;
	static constexpr int TEX_ICONS_PER_ROW = 5;

	auto toolbarButton = [&](const char* title, bool* wndShowBoolean, int tid, const char* description = nullptr) {
		ImGui::PushID(title);
		bool pushed = *wndShowBoolean;
		if (pushed)
			ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		float tx = (float)(tid % TEX_ICONS_PER_ROW) / (float)TEX_ICONS_PER_ROW;
		float ty = (float)(tid / TEX_ICONS_PER_ROW) / (float)TEX_ICONS_PER_ROW;
		constexpr float delta = 1.0f / (float)TEX_ICONS_PER_ROW;
		if (ImGui::ImageButton("button", tbTexture, ImVec2(BUTTON_SIZE, BUTTON_SIZE), ImVec2(tx, ty), ImVec2(tx + delta, ty + delta))) {
			*wndShowBoolean = !*wndShowBoolean;
			if(*wndShowBoolean)
				windowOpenCounter = (windowOpenCounter + 1) & 3;
			else
				windowOpenCounter = (windowOpenCounter - 1) & 3;
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			float x = ImGui::GetCursorPosX();
			ImGui::TextUnformatted(title);
			ImGui::SameLine(x + 1.0f);
			ImGui::TextUnformatted(title);
			ImGui::TextUnformatted(description);
			ImGui::EndTooltip();
		}
		if (pushed)
			ImGui::PopStyleColor();
		ImGui::PopID();
		ImGui::SameLine();
	};
	auto toolbarSeparator = [&]() {
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(CATEGORY_SEPARATION, 1.0f));
		ImGui::SameLine();
		ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x + CATEGORY_SEPARATION / 2.0f, pos.y), ImVec2(pos.x + CATEGORY_SEPARATION / 2.0f, pos.y + BUTTON_SIZE + 16.0f), 0xFFFFFFFF);
	};
	const char* groupTitle = nullptr;
	float groupStartX = 0.0f;
	auto toolbarGroupStart = [&](const char* title) {
		ImGui::BeginGroup();
		groupTitle = title;
		groupStartX = ImGui::GetCursorPosX();
	};
	auto toolbarGroupEnd = [&]() {
		float groupEndX = ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x;
		float len = ImGui::CalcTextSize(groupTitle).x;
		ImGui::NewLine();
		ImGui::SetCursorPosX(groupStartX + std::round((groupEndX - groupStartX) * 0.5f - len * 0.5f));
		ImGui::TextUnformatted(groupTitle);
		ImGui::EndGroup();
		ImGui::SameLine(0.0f, 0.0f);
	};
	if (!toolbarCollapsed) {
		ImVec2 minCorner = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() + 2.0f);
		ImVec2 tbSize = ImVec2((float)g_window->getWidth(), BUTTON_SIZE + 16.0f + ImGui::GetTextLineHeightWithSpacing());
		ImVec2 maxCorner = ImVec2(minCorner.x + tbSize.x, minCorner.y + tbSize.y);
		ImGui::SetNextWindowPos(minCorner, ImGuiCond_Always);
		ImGui::SetNextWindowSize(tbSize, ImGuiCond_Always);
		ImVec4 bgndcolor = ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg);
		bgndcolor.x *= 0.7f; bgndcolor.y *= 0.7f; bgndcolor.z *= 0.7f; bgndcolor.w = 240.0f / 255.0f;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, bgndcolor);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollWithMouse /* | ImGuiWindowFlags_NoBackground*/);
		ImGui::PopStyleVar(1);
		ImGui::PopStyleColor(1);
		toolbarGroupStart("General");
		toolbarButton("Main", &wndShowMain, 0, "Load and save level, manage the camera and view");
		toolbarButton("Scene graph", &wndShowSceneGraph, 1, "View the Scene Graph and manipulate the Scene Nodes\n - Add new scene nodes\n - Import/export of rendered geometry");
		toolbarButton("Beacons", &wndShowBeacons, 2, "Manage beacons\nBeacons are 3D points used to position objects such as:\nbonuses, crates, respawn points, merchant, ...");
		toolbarButton("Grounds", &wndShowGrounds, 3, "Manage the Grounds in the sectors\nGrounds are 3D collision models that indicate where entities\nsuch as heroes and enemies can stand and walk on.\nThey also include walls that prevent heroes to pass through.");
		toolbarButton("Pathfinding", &wndShowPathfinding, 4, "Manipulate the pathfinding grids and cells.\nPathfinding is used to guide AI-controlled heroes and enemies through the world\nwhile preventing them to access undesired areas such as walls\nusing some form of A* algorithm.");
		if (kenv.version <= kenv.KVERSION_XXL2)
			toolbarButton("Level properties", &wndShowLevel, 17, "Edit the properties of the current level, such as:\n - Set Asterix spawning position\n - Add new sector\n - Sky color\n - ...");
		toolbarGroupEnd();
		toolbarSeparator();
		toolbarGroupStart("Scripting");
		if (kenv.hasClass<CKGroupRoot>())
			toolbarButton("Hooks", &wndShowHooks, 5, "Manipulate the hooks\nHooks are attached to Scene Nodes and handle their behaviours\n(similar to adding a Component to a GameObject/Actor in Unity/Unreal)");
		if (kenv.hasClass<CKGrpSquadEnemy>() || kenv.hasClass<CKGrpSquadX2>())
			toolbarButton("Squads", &wndShowSquads, 6, "Manipulate the squads and its enemies.\nSquads are groups of enemies, represented by giant swords.");
		//toolbarSeparator();
		if (kenv.version <= kenv.KVERSION_XXL1)
			toolbarButton("Events", &wndShowEvents, 7, "Manipulate the scripting events.");
		else
			toolbarButton("Triggers", &wndShowTriggers, 7, "Manipulate the scripting triggers.");
		toolbarButton("Detectors", &wndShowDetectors, 8, "Manipulate the detectors.\nDetectors are shapes that trigger events when entered.");
		if (kenv.version <= kenv.KVERSION_XXL1)
			toolbarButton("Markers", &wndShowMarkers, 9, "Manipulate the markers.\nMarkers are points in the level that can be used in scripting.");
		toolbarButton("Lines", &wndShowLines, 10, "Manipulate lines used in scripting and mechanisms.");
		toolbarGroupEnd();
		//toolbarSeparator();
		//toolbarButton("Cinematic", &wndShowCinematic, "Manipulate the cutscenes");
		//toolbarButton("Collision", &wndShowCollision, "Show the collisions registered between bounding shape nodes\n(for debugging purposes)");
		toolbarSeparator();
		toolbarGroupStart("Assets");
		toolbarButton("Textures", &wndShowTextures, 11, "Manage the textures used by the models as well as the interface\nin this level and its sectors.");
		toolbarButton("Clones", &wndShowClones, 12, "Show the geometries that are clones,\nreused by multiple clone nodes (bonuses, enemies, etc.).");
		if (kenv.hasClass<CKSoundDictionary>())
			toolbarButton("Sounds", &wndShowSounds, 13, "Manage the sounds used in this level and its sectors.");
		toolbarGroupEnd();
		toolbarSeparator();
		toolbarGroupStart("Misc");
		toolbarButton("Localization", &wndShowLocale, 14, "Add/Modify language text and fonts in the game.");
		toolbarButton("Objects", &wndShowObjects, 15, "Show a list of all objects (Kal class instances) in the level and sectors.");
		bool openMisc = false;
		toolbarButton("Misc", &openMisc, 16, "Access to debugging and incomplete features that are not recommended to be used.");
		toolbarButton("Information", &wndShowAbout, 18, "Display information about the editor\nand links to documentation and updates.");
		if (openMisc)
			ImGui::OpenPopup("MiscWindowsMenu");
		if (ImGui::BeginPopup("MiscWindowsMenu")) {
			ImGui::MenuItem("Cinematic", nullptr, &wndShowCinematic);
			ImGui::MenuItem("Cameras", nullptr, &wndShowCamera);
			ImGui::MenuItem("Counters", nullptr, &wndShowCounters);
			ImGui::MenuItem("Music", nullptr, &wndShowMusic);
			ImGui::MenuItem("Sekens", nullptr, &wndShowSekens);
			ImGui::MenuItem("Collision", nullptr, &wndShowCollision);
			ImGui::MenuItem("Object inspector", nullptr, &wndShowObjInspector);
			ImGui::MenuItem("Animation viewer", nullptr, &wndShowAnimViewer);
			ImGui::MenuItem("Misc", nullptr, &wndShowMisc);
			ImGui::EndPopup();
		}
		toolbarGroupEnd();
		ImGui::End();
	}

	//ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

	auto igwindow = [this](const char *name, bool *flag, void(*func)(EditorInterface *ui), ImVec2 initPos = ImVec2(360.0f, 105.0f), ImVec2 initSize = ImVec2(500.0f, 500.0f)) {
		if (*flag) {
			float counter = (float)std::max(0, windowOpenCounter);
			ImVec2 demoPos = { initPos.x + counter * 20.0f, initPos.y + counter * 20.0f };
			ImGui::SetNextWindowPos(demoPos, ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(initSize, ImGuiCond_FirstUseEver);
			if (ImGui::Begin(name, flag))
				func(this);
			ImGui::End();
		}
	};
	igwindow("Main", &wndShowMain, [](EditorInterface *ui) { ui->IGMain(); }, ImVec2(7.0f, 105.0f), ImVec2(340.0f, 570.0f));
	if (kenv.hasClass<CTextureDictionary>())
		igwindow("Textures", &wndShowTextures, [](EditorInterface *ui) { IGTextureEditor(*ui); });
	if (kenv.hasClass<CCloneManager>())
		igwindow("Clones", &wndShowClones, [](EditorInterface *ui) { ui->IGCloneEditor(); });
	if (kenv.hasClass<CSGSectorRoot>())
		igwindow("Scene graph", &wndShowSceneGraph, [](EditorInterface *ui) {
			if (ImGui::BeginTable("SceneGraphTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::BeginChild("SceneNodeTree");
				ui->IGSceneGraph();
				ImGui::EndChild();
				ImGui::TableNextColumn();
				ImGui::BeginChild("SceneNodeProperties");
				ui->IGSceneNodeProperties();
				ImGui::EndChild();
				ImGui::EndTable();
			}
		});
	if (kenv.hasClass<CKBeaconKluster>())
		igwindow("Beacons", &wndShowBeacons, [](EditorInterface *ui) { ui->IGBeaconGraph(); });
	if (kenv.hasClass<CKMeshKluster>())
		igwindow("Grounds", &wndShowGrounds, [](EditorInterface *ui) { ui->IGGroundEditor(); });
	if (kenv.hasClass<CKSrvEvent>())
		igwindow("Events", &wndShowEvents, [](EditorInterface *ui) { ui->IGEventEditor(); });
	if (kenv.hasClass<CKSrvTrigger>())
		igwindow("Triggers", &wndShowTriggers, [](EditorInterface *ui) { ui->IGTriggerEditor(); });
	if (kenv.hasClass<CKSoundDictionary>())
		igwindow("Sounds", &wndShowSounds, [](EditorInterface *ui) { ui->IGSoundEditor(); });
	if (kenv.hasClass<CKGrpEnemy>() || (kenv.version >= kenv.KVERSION_XXL2 && kenv.hasClass<CKGrpSquadX2>()))
		igwindow("Squads", &wndShowSquads, [](EditorInterface* ui) { if (ui->kenv.version >= KEnvironment::KVERSION_XXL2) ui->IGX2SquadEditor(); else ui->IGSquadEditor(); });
	if (kenv.hasClass<CKGroupRoot>())
		igwindow("Hooks", &wndShowHooks, [](EditorInterface *ui) { ui->IGHookEditor(); });
	if (kenv.hasClass<CKSrvPathFinding>())
		igwindow("Pathfinding", &wndShowPathfinding, [](EditorInterface *ui) { IGPathfindingEditor(*ui); });
	if (kenv.hasClass<CKSrvMarker>())
		igwindow("Markers", &wndShowMarkers, [](EditorInterface *ui) { IGMarkerEditor(*ui); });
	if (kenv.hasClass<CKSrvDetector>())
		igwindow("Detectors", &wndShowDetectors, [](EditorInterface *ui) { IGDetectorEditorXXL1(*ui); });
	if (kenv.hasClass<CKSectorDetector>())
		igwindow("X2 Detectors", &wndShowDetectors, [](EditorInterface* ui) { IGDetectorEditorXXL2Plus(*ui); });
	if (kenv.hasClass<CKSrvCinematic>())
		igwindow("Cinematic", &wndShowCinematic, [](EditorInterface *ui) { IGCinematicEditor(*ui); });
	if (kenv.hasClass<CKSrvCollision>())
		igwindow("Collision", &wndShowCollision, [](EditorInterface *ui) { IGCollisionEditor(*ui); });
	if (kenv.hasClass<CKLine>())
		igwindow("Lines", &wndShowLines, [](EditorInterface* ui) { IGLineEditor(*ui); });
	igwindow("Localization", &wndShowLocale, [](EditorInterface *ui) { ui->IGLocaleEditor(); });
	igwindow("Objects", &wndShowObjects, [](EditorInterface *ui) { IGObjectList(*ui); });
	igwindow("Level", &wndShowLevel, [](EditorInterface* ui) { IGLevelInfoEditor(*ui); });
	igwindow("Misc", &wndShowMisc, [](EditorInterface *ui) { IGMisc(*ui); });
	igwindow("About", &wndShowAbout, [](EditorInterface* ui) { IGAbout(*ui); });
	if (kenv.hasClass<CKSrvCamera>())
		igwindow("Camera", &wndShowCamera, [](EditorInterface* ui) { IGCameraEditor(*ui); });
	if (kenv.hasClass<CKSrvCounter>())
		igwindow("Counters", &wndShowCounters, [](EditorInterface* ui) { IGCounterEditor(*ui); });
	if (kenv.hasClass<CKSrvMusic>() && kenv.hasClass<CKMusicPlayList>())
		igwindow("Music", &wndShowMusic, [](EditorInterface* ui) {IGMusicEditor(*ui); });
	if (kenv.hasClass<CKSrvSekensor>() && kenv.hasClass<CKSekens>())
		igwindow("Sekens (dialogue)", &wndShowSekens, [](EditorInterface* ui) { IGSekensEditor(*ui); });
	igwindow("Object inspector", &wndShowObjInspector, [](EditorInterface* ui) { IGObjectInspector(*ui); });
	igwindow("Animation viewer", &wndShowAnimViewer, [](EditorInterface* ui) { IGAnimationViewer(*ui); });

#ifndef XEC_RELEASE
	if (showImGuiDemo)
		ImGui::ShowDemoWindow(&showImGuiDemo);
#endif
}

void EditorInterface::render()
{
	gfx->initModelDrawing();
	if (enableAlphaClip)
		gfx->enableAlphaClip();
	else
		gfx->disableAlphaClip();
	if (selGeometry) {
		gfx->setTransformMatrix(Matrix::getTranslationMatrix(selgeoPos) * camera.sceneMatrix);
		progeocache.getPro(selGeometry, &protexdict)->draw();
	}

	CCloneManager *clm = kenv.levelObjects.getFirst<CCloneManager>();

	if (clm && !selClones.empty()) {
		gfx->setTransformMatrix(Matrix::getTranslationMatrix(selgeoPos) * camera.sceneMatrix);
		for (uint32_t ci : selClones)
			if(ci != 0xFFFFFFFF)
				progeocache.getPro(clm->_teamDict._bings[ci]._clump.atomic.geometry.get(), &protexdict)->draw();
	}

	if (showNodes && kenv.hasClass<CSGSectorRoot>() && kenv.hasClass<CKGeometry>()) {
		CSGSectorRoot *rootNode = kenv.levelObjects.getObject<CSGSectorRoot>(0);
		bool isXXL2 = kenv.version >= 2;
		DrawSceneNode(rootNode, camera.sceneMatrix, gfx, progeocache, &protexdict, clm, showTextures, showInvisibleNodes, showClones, nodeCloneIndexMap, isXXL2);
		int showingStream = showingSector - 1;
		if (showingStream < 0) {
			for (int str = 0; str < (int)kenv.numSectors; str++) {
				CSGSectorRoot * strRoot = kenv.sectorObjects[str].getObject<CSGSectorRoot>(0);
				DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, &str_protexdicts[str], clm, showTextures, showInvisibleNodes, showClones, nodeCloneIndexMap, isXXL2);
			}
		} else if(showingStream < (int)kenv.numSectors) {
			CSGSectorRoot * strRoot = kenv.sectorObjects[showingStream].getObject<CSGSectorRoot>(0);
			DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, &str_protexdicts[showingStream], clm, showTextures, showInvisibleNodes, showClones, nodeCloneIndexMap, isXXL2);
		}
	}

	auto drawBox = [this](const Vector3 &a, const Vector3 &b, uint32_t cl = 0xFFFFFFFF) {
		Vector3 _b1(a.x, a.y, a.z);
		Vector3 _b2(a.x, a.y, b.z);
		Vector3 _b3(b.x, a.y, b.z);
		Vector3 _b4(b.x, a.y, a.z);
		Vector3 _t1(a.x, b.y, a.z);
		Vector3 _t2(a.x, b.y, b.z);
		Vector3 _t3(b.x, b.y, b.z);
		Vector3 _t4(b.x, b.y, a.z);
		gfx->drawLine3D(_b1, _b2, cl);
		gfx->drawLine3D(_b2, _b3, cl);
		gfx->drawLine3D(_b3, _b4, cl);
		gfx->drawLine3D(_b4, _b1, cl);
		gfx->drawLine3D(_t1, _t2, cl);
		gfx->drawLine3D(_t2, _t3, cl);
		gfx->drawLine3D(_t3, _t4, cl);
		gfx->drawLine3D(_t4, _t1, cl);
		gfx->drawLine3D(_b1, _t1, cl);
		gfx->drawLine3D(_b2, _t2, cl);
		gfx->drawLine3D(_b3, _t3, cl);
		gfx->drawLine3D(_b4, _t4, cl);
	};

	auto getCloneIndex = [this, clm](CSGBranch *node) {
		//auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const kobjref<CSGBranch> &ref) {return ref.get() == node; });
		//assert(it != clm->_clones.end());
		//size_t clindex = it - clm->_clones.begin();
		//return clindex;
		return nodeCloneIndexMap.at(node);
	};
	auto drawClone = [this, clm](size_t clindex) {
		for (uint32_t part : clm->_team.dongs[clindex].bongs)
			if (part != 0xFFFFFFFF) {
				RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump.atomic.geometry.get();
				progeocache.getPro(rwgeo, &protexdict)->draw();
			}
	};

	// beacon rotation
	const Matrix rotmat = Matrix::getRotationYMatrix(SDL_GetTicks() * 3.1415f / 1000.0f);

	auto drawBeaconKluster = [this, clm, &getCloneIndex, &drawClone, &drawBox, &rotmat](CKBeaconKluster* bk) {
		if (showBeaconKlusterBounds) {
			gfx->setTransformMatrix(camera.sceneMatrix);
			gfx->unbindTexture(0);
			float rd = bk->bounds.radius;
			drawBox(bk->bounds.center + Vector3(rd, rd, rd), bk->bounds.center - Vector3(rd, rd, rd));
		}
		if (showBeacons) {
			uint32_t fallbackSphereColor = 255 - (SDL_GetTicks() % 1000) * 128 / 1000;
			for (auto &bing : bk->bings) {
				if (!bing.active || bing.beacons.empty())
					continue;
				uint32_t handlerFID = bing.handler->getClassFullID();
				for (auto &beacon : bing.beacons) {
					Vector3 pos = beacon.getPosition();
					pos.y += 0.5f;
					if (handlerFID == CKCrateCpnt::FULL_ID && kenv.hasClass<CSGRootNode>()) {
						const int numCrates = beacon.params & 7;
						const int transformPackIndex = (beacon.params >> 6) & 7;

						CKCrateCpnt *cratecpnt = bing.handler->cast<CKCrateCpnt>();
						if (!cratecpnt->crateNode) // happens in Romaster
							goto drawFallbackSphere;
						const size_t clindex = getCloneIndex(cratecpnt->crateNode->cast<CClone>());

						for (int c = 0; c < numCrates; c++) {
							const int transformIndex = transformPackIndex * 7 + c;
							const Matrix rotation = Matrix::getRotationYMatrix(cratecpnt->pack2[transformIndex]);
							const Matrix translation = Matrix::getTranslationMatrix(pos + Vector3(cratecpnt->pack1[transformIndex * 2], (float)c, cratecpnt->pack1[transformIndex * 2 + 1]));
							gfx->setTransformMatrix(rotation * translation * camera.sceneMatrix);
							drawClone(clindex);
						}
					}
					else if (bing.handler->isSubclassOf<CKGrpBonusPool>() && kenv.hasClass<CSGRootNode>()) {
						CKGrpBonusPool *pool = bing.handler->cast<CKGrpBonusPool>();
						CKHook *hook = pool->childHook.get();

						Matrix beaconTransform;
						if (bing.handler->isSubclassOf<CKGrpWildBoarPool>()) {
							const float angle = decode8bitAngle(beacon.params & 255);
							beaconTransform = Matrix::getRotationYMatrix(angle) * Matrix::getTranslationMatrix(beacon.getPosition()) * camera.sceneMatrix;
						}
						else {
							beaconTransform = rotmat * Matrix::getTranslationMatrix(pos) * camera.sceneMatrix;
						}
						gfx->setTransformMatrix(beaconTransform);

						if (hook->node->isSubclassOf<CClone>() || hook->node->isSubclassOf<CAnimatedClone>()) {
							size_t clindex = getCloneIndex(hook->node->cast<CSGBranch>());
							drawClone(clindex);
						}
						else if (hook->node->isSubclassOf<CNode>() && hook->node->cast<CNode>()->geometry->clump->atomic.geometry->numTris) {
							for (CKAnyGeometry* geo = hook->node->cast<CNode>()->geometry.get(); geo; geo = geo->nextGeo.get()) {
								RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
								progeocache.getPro(rwgeo, &protexdict)->draw();
							}
						}
						else
							goto drawFallbackSphere;
					}
					else if (bing.handlerId == 5 || bing.handlerId == 0x16) {
						gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos)* camera.sceneMatrix);
						progeocache.getPro(spawnStarModel->geoList.geometries[0], &protexdict)->draw();
					}
					else {
					drawFallbackSphere:
						bool isOrientable = false;
						if (auto* jsBeaconInfo = g_encyclo.getBeaconJson(bing.handlerId)) {
							isOrientable = jsBeaconInfo->is_object() && jsBeaconInfo->value<bool>("orientable", false);
						}
						Matrix transform = Matrix::getTranslationMatrix(pos) * camera.sceneMatrix;
						if (isOrientable) {
							const float angle = decode8bitAngle(beacon.params & 255);
							transform = Matrix::getRotationYMatrix(angle) * transform;
						}
						gfx->setTransformMatrix(transform);
						gfx->setBlendColor(0xFF000000 | fallbackSphereColor);
						progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
						if (isOrientable) {
							gfx->drawLine3D(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 2.0f));
							gfx->drawLine3D(Vector3(0.0f, 0.0f, 2.0f), Vector3(0.5f, 0.0f, 1.5f));
							gfx->drawLine3D(Vector3(0.0f, 0.0f, 2.0f), Vector3(-0.5f, 0.0f, 1.5f));
						}
						gfx->setBlendColor(0xFFFFFFFF);
					}
				}
			}
		}
	};
	if (kenv.hasClass<CKBeaconKluster>()) {
		for (CKBeaconKluster *bk = kenv.levelObjects.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
			drawBeaconKluster(bk);
		int showingStream = showingSector - 1;
		if(showingStream < 0)
			for (auto &str : kenv.sectorObjects)
				for (CKBeaconKluster *bk = str.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
					drawBeaconKluster(bk);
		else if(showingStream < (int)kenv.numSectors)
			for (CKBeaconKluster *bk = kenv.sectorObjects[showingStream].getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
				drawBeaconKluster(bk);
	}

	if (showSasBounds && kenv.hasClass<CKSas>()) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		for (CKObject *obj : kenv.levelObjects.getClassType<CKSas>().objects) {
			CKSas *sas = (CKSas*)obj;
			for (auto &box : sas->boxes) {
				drawBox(box.highCorner, box.lowCorner);
			}
		}
	}

	gfx->setTransformMatrix(camera.sceneMatrix);
	gfx->unbindTexture(0);

	// Cursor box
	const Vector3 rad = Vector3(1, 1, 1) * 0.1f;
	drawBox(cursorPosition + rad, cursorPosition - rad);

	if (showGroundBounds && kenv.hasClass<CGround>()) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		auto drawGroundBounds = [this,&drawBox](CGround* gnd) {
			auto &b = gnd->aabb;
			drawBox(b.highCorner, b.lowCorner, (selGround == gnd) ? 0xFF00FF00 : 0xFFFFFFFF);
		};
		for (CKObject* obj : kenv.levelObjects.getClassType<CGround>().objects)
			drawGroundBounds(obj->cast<CGround>());
		int showingStream = showingSector - 1;
		if (showingStream < 0)
			for (auto &str : kenv.sectorObjects)
				for (CKObject *obj : str.getClassType<CGround>().objects)
					drawGroundBounds(obj->cast<CGround>());
		else if(showingStream < (int)kenv.numSectors)
			for (CKObject *obj : kenv.sectorObjects[showingStream].getClassType<CGround>().objects)
				drawGroundBounds(obj->cast<CGround>());
	}

	if (showGrounds && kenv.hasClass<CGround>()) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		auto drawGround = [this](CGround* gnd) {
			if (selGround == gnd) gfx->setBlendColor(0xFF00FF00);
			gndmdlcache.getModel(gnd)->draw(showInfiniteWalls);
			if (selGround == gnd) gfx->setBlendColor(0xFFFFFFFF);
		};
		for (CKObject* obj : kenv.levelObjects.getClassType<CGround>().objects)
			drawGround(obj->cast<CGround>());
		int showingStream = showingSector - 1;
		if (showingStream < 0)
			for (auto &str : kenv.sectorObjects)
				for (CKObject *obj : str.getClassType<CGround>().objects)
					drawGround(obj->cast<CGround>());
		else if (showingStream < (int)kenv.numSectors)
			for (CKObject *obj : kenv.sectorObjects[showingStream].getClassType<CGround>().objects)
				drawGround(obj->cast<CGround>());

		// CWalls
		if (kenv.hasClass<CWall>()) {
			gfx->setTransformMatrix(camera.sceneMatrix);
			gfx->unbindTexture(0);
			gfx->setBlendColor(0xFFFFFFFF);
			auto tf = [](const Vector3& vec, CWall* wall) {
				return vec.transform(wall->wallTransform);
				};
			auto drawWall = [this, &tf](CWall* wall) {
				for (const auto& tri : wall->triangles) {
					for (int i = 0; i < 3; ++i) {
						gfx->drawLine3D(tf(wall->vertices[tri.indices[i]], wall), tf(wall->vertices[tri.indices[(i + 1) % 3]], wall), 0xFFFF0080);
					}
				}
				};
			for (CKObject* obj : kenv.levelObjects.getClassType<CWall>().objects)
				drawWall(obj->cast<CWall>());
			if (showingStream < 0)
				for (auto& str : kenv.sectorObjects)
					for (CKObject* obj : str.getClassType<CWall>().objects)
						drawWall(obj->cast<CWall>());
			else if (showingStream < (int)kenv.numSectors)
				for (CKObject* obj : kenv.sectorObjects[showingStream].getClassType<CWall>().objects)
					drawWall(obj->cast<CWall>());
		}
	}

	// CKLine
	if (showLines && kenv.hasClass<CKLine>()) {
		auto drawKLine = [this](CKLine* kl) {
			for (size_t i = 0; i < kl->numSegments; i++)
				gfx->drawLine3D(kl->points[i], kl->points[i + 1]);
		};
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		for (CKObject* obj : kenv.levelObjects.getClassType<CKLine>().objects)
			drawKLine(obj->cast<CKLine>());
		for (auto &str : kenv.sectorObjects)
			for (CKObject *obj : str.getClassType<CKLine>().objects)
				drawKLine(obj->cast<CKLine>());
	}

	// CKSpline4L
	if (showLines && kenv.hasClass<CKSpline4L>()) {
		auto drawSpline = [this](CKSpline4L* kl) {
			for (size_t i = 0; i < kl->cksPrecomputedPoints.size()-1; i++)
				gfx->drawLine3D(kl->cksPrecomputedPoints[i], kl->cksPrecomputedPoints[i + 1]);
		};
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		for (CKObject* obj : kenv.levelObjects.getClassType<CKSpline4L>().objects)
			drawSpline(obj->cast<CKSpline4L>());
		for (auto &str : kenv.sectorObjects)
			for (CKObject *obj : str.getClassType<CKSpline4L>().objects)
				drawSpline(obj->cast<CKSpline4L>());
	}

	// CKSpline4
	if (showLines && kenv.hasClass<CKSpline4>()) {
		auto drawSpline = [this](CKSpline4* kl) {
			for (size_t i = 0; i < kl->cksPoints.size() - 1; i++)
				gfx->drawLine3D(kl->cksPoints[i], kl->cksPoints[i + 1]);
		};
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		for (CKObject* obj : kenv.levelObjects.getClassType<CKSpline4>().objects)
			drawSpline(obj->cast<CKSpline4>());
		for (auto& str : kenv.sectorObjects)
			for (CKObject* obj : str.getClassType<CKSpline4>().objects)
				drawSpline(obj->cast<CKSpline4>());
	}

	CKGroup *grpEnemy = kenv.hasClass<CKGrpEnemy>() ? kenv.levelObjects.getFirst<CKGrpEnemy>() : nullptr;

	if (grpEnemy && (showSquadBoxes || showMsgActionBoxes)) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		//for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
		for (CKGroup* osquad = grpEnemy->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
			if (!osquad->isSubclassOf<CKGrpSquadEnemy>()) continue;
			CKGrpSquadEnemy* squad = osquad->cast<CKGrpSquadEnemy>();
			if (showSquadBoxes) {
				for (const auto& bb : { squad->sqUnk3, squad->sqUnk4 }) {
					const Vector3& v1 = bb[0];
					const Vector3& v2 = bb[1];
					drawBox(v1 - v2 * 0.5f, v1 + v2 * 0.5f);
				}
			}
			if (showMsgActionBoxes) {
				// CKMsgAction Triggers
				for (const auto& size : { squad->seUnk1 }) {
					Vector3 translation = squad->mat1.getTranslationVector();
					gfx->setBlendColor(0xFF0000FF); // red
					drawBox(translation - size, translation + size);
					gfx->setBlendColor(0xFFFFFFFF); // white
				}
			}
		}
	}
	if (showSquadChoreos && grpEnemy) {
		auto prosword = progeocache.getPro(swordModel->geoList.geometries[0], &protexdict);
		for (CKGroup *osquad = grpEnemy->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
			if (!osquad->isSubclassOf<CKGrpSquadEnemy>()) continue;
			CKGrpSquadEnemy *squad = osquad->cast<CKGrpSquadEnemy>();
			gfx->setTransformMatrix(squad->mat1 * camera.sceneMatrix);
			prosword->draw();
		}

		gfx->setTransformMatrix(camera.sceneMatrix);
		for (CKGroup *osquad = grpEnemy->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
			if (!osquad->isSubclassOf<CKGrpSquadEnemy>()) continue;
			CKGrpSquadEnemy *squad = osquad->cast<CKGrpSquadEnemy>();

			if (0 <= showingChoreoKey && showingChoreoKey < (int)squad->choreoKeys.size()) {
				CKChoreoKey *ckey = squad->choreoKeys[showingChoreoKey].get();
				const Matrix &gmat = squad->mat1;
				for (auto& slot : ckey->slots) {
					Vector3 spos = slot.position.transform(gmat);

					gfx->setBlendColor((slot.enemyGroup == -1) ? 0xFF0000FF : 0xFFFFFFFF);
					gfx->setTransformMatrix(camera.sceneMatrix);
					gfx->unbindTexture(0);
					drawBox(spos + Vector3(-1, 0, -1), spos + Vector3(1, 2, 1));
					gfx->setBlendColor(0xFFFFFFFF);

					if (kenv.hasClass<CCloneManager>()) {
						uint8_t poolindex = (slot.enemyGroup != -1) ? slot.enemyGroup : 0;
						if (poolindex < squad->pools.size()) {
							auto hook = squad->pools[poolindex].pool->childHook;
							auto nodegeo = hook->node->cast<CAnimatedClone>();
							size_t clindex = getCloneIndex(nodegeo);
							float angle = std::atan2(slot.direction.x, slot.direction.z);
							gfx->setTransformMatrix(Matrix::getRotationYMatrix(angle) * Matrix::getTranslationMatrix(slot.position) * squad->mat1 * camera.sceneMatrix);

							drawClone(clindex);
							for (CKSceneNode* subnode = nodegeo->child.get(); subnode; subnode = subnode->next.get()) {
								if (subnode->isSubclassOf<CAnimatedClone>()) {
									int ci = getCloneIndex((CSGBranch*)subnode);
									drawClone(ci);
								}
							}
						}
					}
				}
			}
		}
	}

	if (showSquadChoreos && kenv.version >= kenv.KVERSION_XXL2 && kenv.hasClass<CKGrpSquadX2>()) {
		auto prosword = progeocache.getPro(swordModel->geoList.geometries[0], &protexdict);
		for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadX2>().objects) {
			CKGrpSquadX2* squad = osquad->cast<CKGrpSquadX2>();
			if ((size_t)showingChoreography < squad->phases.size()) {
				Matrix mat = squad->phases[showingChoreography].mat;
				std::tie(mat._14, mat._24, mat._34, mat._44) = std::make_tuple(0.0f, 0.0f, 0.0f, 1.0f);
				gfx->setTransformMatrix(mat * camera.sceneMatrix);
				prosword->draw();
			}
		}

		gfx->setTransformMatrix(camera.sceneMatrix);
		for (CKObject* osquad : kenv.levelObjects.getClassType<CKGrpSquadX2>().objects) {
			CKGrpSquadX2* squad = osquad->cast<CKGrpSquadX2>();
			if ((size_t)showingChoreography < squad->phases.size()) {
				auto& phase = squad->phases[showingChoreography];
				CKChoreography* choreo = phase.choreography.get();
				if (0 <= showingChoreoKey && showingChoreoKey < (int)choreo->keys.size()) {
					CKChoreoKey* ckey = choreo->keys[showingChoreoKey].get();
					Matrix gmat = phase.mat;
					std::tie(gmat._14, gmat._24, gmat._34, gmat._44) = std::make_tuple(0.0f, 0.0f, 0.0f, 1.0f);
					for (auto& slot : ckey->slots) {
						Vector3 spos = slot.position.transform(gmat);

						gfx->setBlendColor((slot.enemyGroup == -1) ? 0xFF0000FF : 0xFFFFFFFF);
						gfx->setTransformMatrix(camera.sceneMatrix);
						gfx->unbindTexture(0);
						drawBox(spos + Vector3(-1, 0, -1), spos + Vector3(1, 2, 1));
						gfx->setBlendColor(0xFFFFFFFF);

						if (kenv.hasClass<CCloneManager>() && (kenv.version == KEnvironment::KVERSION_XXL2 || kenv.version == KEnvironment::KVERSION_OLYMPIC)) {
							uint8_t poolindex = (slot.enemyGroup != -1) ? slot.enemyGroup : 0;
							GameX2::CKGrpFightZone* zone = squad->parentGroup->cast<GameX2::CKGrpFightZone>();
							const X2FightData& fightData = (kenv.version >= KEnvironment::KVERSION_ARTHUR) ?
								zone->fightData : squad->fightData;
							if (poolindex < fightData.pools.size()) {
								CKHook* hook = fightData.pools[poolindex].pool->childHook.get();
								float angle = std::atan2(slot.direction.x, slot.direction.z);
								gfx->setTransformMatrix(Matrix::getRotationYMatrix(angle) * Matrix::getTranslationMatrix(slot.position) * gmat * camera.sceneMatrix);
								if (kenv.version == KEnvironment::KVERSION_XXL2) {
									auto* nodegeo = hook->node->cast<CAnimatedClone>();
									size_t clindex = getCloneIndex(nodegeo);

									drawClone(clindex);
									for (CKSceneNode* subnode = nodegeo->child.get(); subnode; subnode = subnode->next.get()) {
										if (subnode->isSubclassOf<CAnimatedClone>()) {
											int ci = getCloneIndex((CSGBranch*)subnode);
											drawClone(ci);
										}
									}
								}
								else if (kenv.version == KEnvironment::KVERSION_OLYMPIC) {
									GameOG::CKHkA3Enemy* a3enemy = hook->cast<GameOG::CKHkA3Enemy>();
									auto it = std::find_if(a3enemy->ckhaeEnemySectorCpnts.begin(), a3enemy->ckhaeEnemySectorCpnts.end(), [](auto& ref) -> bool {return (bool)ref; });
									if (it != a3enemy->ckhaeEnemySectorCpnts.end()) {
										GameOG::CKEnemySectorCpnt* cpnt = it->get();
										for (auto& node : cpnt->ckescSceneNodes) {
											auto* nodegeo = node->cast<CAnimatedClone>();
											size_t clindex = getCloneIndex(nodegeo);
											drawClone(clindex);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (showPFGraph && kenv.hasClass<CKSrvPathFinding>()) {
		if (CKSrvPathFinding *srvpf = kenv.levelObjects.getFirst<CKSrvPathFinding>()) {
			gfx->setTransformMatrix(camera.sceneMatrix);
			gfx->unbindTexture(0);
			for (auto &pfnode : srvpf->nodes) {
				drawBox(pfnode->lowBBCorner, pfnode->highBBCorner);

				float h = pfnode->highBBCorner.y - pfnode->lowBBCorner.y;
				float cw = pfnode->getCellWidth();
				float ch = pfnode->getCellHeight();
				for (float y : {h}) {
					for (int z = 1; z < pfnode->numCellsZ; z++) {
						gfx->drawLine3D(pfnode->lowBBCorner + Vector3(0, y, z*ch), pfnode->lowBBCorner + Vector3(pfnode->numCellsX*cw, y, z*ch), 0xFF00FFFF);
					}
					for (int x = 1; x < pfnode->numCellsX; x++) {
						gfx->drawLine3D(pfnode->lowBBCorner + Vector3(x*cw, y, 0), pfnode->lowBBCorner + Vector3(x*cw, y, pfnode->numCellsZ*ch), 0xFF00FFFF);
					}
				}

				for (int z = 0; z < pfnode->numCellsZ; z++) {
					for (int x = 0; x < pfnode->numCellsX; x++) {
						uint8_t val = pfnode->cells[z*pfnode->numCellsX + x];
						if (val != 1) {
							Vector3 cellsize(pfnode->getCellWidth(), 1, pfnode->getCellHeight());
							ImVec4 igcolor = getPFCellColor(val);
							uint32_t ddcolor = ((int)(igcolor.x * 255.0f) << 16) | ((int)(igcolor.y * 255.0f) << 8) | ((int)(igcolor.z * 255.0f)) | ((int)(igcolor.w * 255.0f) << 24);
							float fx = (float)x, fz = (float)z;
							gfx->drawLine3D(pfnode->lowBBCorner + Vector3(fx, h, fz)*cellsize, pfnode->lowBBCorner + Vector3(fx + 1, h, fz + 1)*cellsize, ddcolor);
							gfx->drawLine3D(pfnode->lowBBCorner + Vector3(fx + 1, h, fz)*cellsize, pfnode->lowBBCorner + Vector3(fx, h, fz + 1)*cellsize, ddcolor);
						}
					}
				}

				for (auto &pftrans : pfnode->transitions) {
					for (auto &thing : pftrans->things) {
						drawBox(Vector3(thing.matrix[0], thing.matrix[1], thing.matrix[2]),
							Vector3(thing.matrix[3], thing.matrix[4], thing.matrix[5]),
							0xFF00FF00);
					}
				}
			}
		}
	}

	if (showMarkers && kenv.hasClass<CKSrvMarker>()) {
		if (CKSrvMarker *srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>()) {
			gfx->unbindTexture(0);
			gfx->setBlendColor(0xFFFFFF00);
			for (auto &list : srvMarker->lists) {
				for (auto &marker : list) {
					const float angle = decode8bitAngle(marker.orientation1);
					gfx->setTransformMatrix(
						Matrix::getRotationYMatrix(angle)
						* Matrix::getTranslationMatrix(marker.position)
						* camera.sceneMatrix);
					progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
					gfx->drawLine3D(Vector3(0.0f, 0.0f, 0.0f), Vector3(2.0f, 0.0f, 0.0f));
					gfx->drawLine3D(Vector3(2.0f, 0.0f, 0.0f), Vector3(1.5f, 0.0f, 0.5f));
					gfx->drawLine3D(Vector3(2.0f, 0.0f, 0.0f), Vector3(1.5f, 0.0f, -0.5f));
				}
			}
		}
	}
	if (showDetectors) {
		auto drawRectDetector = [this, &drawBox](AARectangle& h) {
			Vector3 dir, side1, side2;
			switch (h.direction | 1) {
			case 1: dir = Vector3(1, 0, 0); side1 = Vector3(0, 1, 0); side2 = Vector3(0, 0, 1); break;
			case 3: dir = Vector3(0, 1, 0); side1 = Vector3(1, 0, 0); side2 = Vector3(0, 0, 1); break;
			case 5: dir = Vector3(0, 0, 1); side1 = Vector3(1, 0, 0); side2 = Vector3(0, 1, 0); break;
			}
			if (h.direction & 1)
				dir *= -1.0f;
			gfx->drawLine3D(h.center, h.center + dir * 4.0f);
			Vector3 corner = side1 * h.length1 + side2 * h.length2;
			drawBox(h.center + corner, h.center - corner);
			};
		if (kenv.version == kenv.KVERSION_XXL1) {
			if (CKSrvDetector* srvDetector = kenv.levelObjects.getFirst<CKSrvDetector>()) {
				ProGeometry* progeoSphere = progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict);
				gfx->setTransformMatrix(camera.sceneMatrix);
				gfx->unbindTexture(0);
				gfx->setBlendColor(0xFF00FF00); // green
				for (auto& aabb : srvDetector->aaBoundingBoxes)
					drawBox(aabb.highCorner, aabb.lowCorner);
				gfx->setBlendColor(0xFF0080FF); // orange
				for (auto& sph : srvDetector->spheres)
					drawBox(sph.center + Vector3(1, 1, 1) * sph.radius, sph.center - Vector3(1, 1, 1) * sph.radius);
				gfx->setBlendColor(0xFFFF00FF); // pink
				for (auto& h : srvDetector->rectangles) {
					drawRectDetector(h);
				}
				//
				gfx->setBlendColor(0xFF00FF00); // green
				for (auto& aabb : srvDetector->aaBoundingBoxes) {
					gfx->setTransformMatrix(Matrix::getTranslationMatrix((aabb.highCorner + aabb.lowCorner) * 0.5f) * camera.sceneMatrix);
					progeoSphere->draw();
				}
				gfx->setBlendColor(0xFF0080FF); // orange
				for (auto& sph : srvDetector->spheres) {
					gfx->setTransformMatrix(Matrix::getTranslationMatrix(sph.center) * camera.sceneMatrix);
					progeoSphere->draw();
				}
				gfx->setBlendColor(0xFFFF00FF); // pink
				for (auto& rect : srvDetector->rectangles) {
					gfx->setTransformMatrix(Matrix::getTranslationMatrix(rect.center) * camera.sceneMatrix);
					progeoSphere->draw();
				}
			}
		}
		if (kenv.hasClass<CKSectorDetector>()) {
			ProGeometry* progeoSphere = progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict);
			gfx->unbindTexture(0);
			int strid = -2;
			for (CKObject* osector : kenv.levelObjects.getClassType<CKSectorDetector>().objects) {
				++strid;
				int showingStream = showingSector - 1;
				if (!(showingStream < 0 || strid == -1 || strid == showingStream))
					continue;
				gfx->setTransformMatrix(camera.sceneMatrix);
				CKSectorDetector* sector = osector->cast<CKSectorDetector>();
				for (auto& detector : sector->sdDetectors) {
					auto& geo = detector->dbGeometry;
					if (auto* aabb = std::get_if<AABoundingBox>(&geo->mgShape)) {
						gfx->setBlendColor(0xFF00FF00); // green
						drawBox(aabb->highCorner, aabb->lowCorner);
					}
					else if (auto* sph = std::get_if<BoundingSphere>(&geo->mgShape)) {
						gfx->setBlendColor(0xFF0080FF); // orange
						Vector3 ext = Vector3(1, 1, 1) * sph->radius;
						drawBox(sph->center + ext, sph->center - ext);
					}
					else if (auto* rect = std::get_if<AARectangle>(&geo->mgShape)) {
						gfx->setBlendColor(0xFFFF00FF); // pink
						drawRectDetector(*rect);
					}
				}
				//
				for (auto& detector : sector->sdDetectors) {
					auto& geo = detector->dbGeometry;
					if (auto* aabb = std::get_if<AABoundingBox>(&geo->mgShape)) {
						gfx->setBlendColor(0xFF00FF00); // green
						gfx->setTransformMatrix(Matrix::getTranslationMatrix((aabb->highCorner + aabb->lowCorner) * 0.5f) * camera.sceneMatrix);
						progeoSphere->draw();
					}
					else if (auto* sph = std::get_if<BoundingSphere>(&geo->mgShape)) {
						gfx->setBlendColor(0xFF0080FF); // orange
						gfx->setTransformMatrix(Matrix::getTranslationMatrix(sph->center) * camera.sceneMatrix);
						progeoSphere->draw();
					}
					else if (auto* rect = std::get_if<AARectangle>(&geo->mgShape)) {
						gfx->setBlendColor(0xFFFF00FF); // pink
						gfx->setTransformMatrix(Matrix::getTranslationMatrix(rect->center) * camera.sceneMatrix);
						progeoSphere->draw();
					}
				}
			}
		}
	}

	if (false) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->setBlendColor(0xFFFFFFFF); // white
		for (CKObject *sobj : kenv.levelObjects.getClassType<CKSector>().objects) {
			CKSector *str = sobj->cast<CKSector>();
			if(str != kenv.levelObjects.getFirst<CKSector>())
				if(str->boundaries.highCorner.x >= str->boundaries.lowCorner.x &&
					str->boundaries.highCorner.y >= str->boundaries.lowCorner.y &&
					str->boundaries.highCorner.z >= str->boundaries.lowCorner.z)
						drawBox(str->boundaries.lowCorner, str->boundaries.highCorner);
		}
	}

	if (showLights && kenv.hasClass<CKGrpLight>()) {
		gfx->unbindTexture(0);
		gfx->setBlendColor(0xFF00FFFF); // yellow
		if (CKGrpLight *grpLight = kenv.levelObjects.getFirst<CKGrpLight>()) {
			auto &points = grpLight->node->cast<CNode>()->geometry->cast<CKParticleGeometry>()->pgPoints;
			ProGeometry *progeo = progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict);
			for (const Vector3 &pnt : points) {
				gfx->setTransformMatrix(Matrix::getTranslationMatrix(pnt) * camera.sceneMatrix);
				progeo->draw();
			}
		}
	}

	RenderAnimation(*this);
}

void EditorInterface::IGMain()
{
	ImGui::InputInt("Level number##LevelNum", &levelNum);
	if (ImGui::Button("Load")) {
		const char* fnfmt = kenv.isUsingNewFilenames() ? "LVL%03u/LVL%03u.%s" : "LVL%03u/LVL%02u.%s";
		char lvlfn[64];
		snprintf(lvlfn, sizeof(lvlfn), fnfmt, levelNum, levelNum, kenv.platformExt[kenv.platform]);
		if (!std::filesystem::exists(std::filesystem::u8path(kenv.gamePath) / lvlfn)) {
			snprintf(lvlfn, sizeof(lvlfn), "Level %i does not exist.", levelNum);
			MsgBox(g_window, lvlfn, 16);
		}
		else {
			selGeometry = nullptr;
			selNode = nullptr;
			selBeaconSector = -1;
			selGround = nullptr;
			selectedSquad = nullptr;
			selectedX2Squad = nullptr;
			selectedPFGraphNode = nullptr;
			selectedMarkerIndex = -1;
			selectedHook = nullptr;
			selectedGroup = nullptr;
			selectedTrigger = nullptr;
			selectedShapeType = -1;
			selectedShapeIndex = -1;
			selClones.clear();
			rayHits.clear();
			nearestRayHit = nullptr;

			progeocache.clear();
			gndmdlcache.clear();
			kenv.loadLevel(levelNum);
			prepareLevelGfx();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		// Check for level's folder existence and create it if it doesn't exist
		auto dir = std::filesystem::path(kenv.outGamePath) / fmt::format("LVL{:03d}", levelNum);
		bool doit = true;
		if (!std::filesystem::is_directory(dir)) {
			std::string msg = fmt::format("The following folder:\n\n{}\n\nis missing. Do you want to create it and then save as Level {}?\n\nNote: Language files won't be created.", dir.string(), levelNum);
			int r = MsgBox(g_window, msg.c_str(), MB_ICONWARNING | MB_YESNO);
			if (r == IDYES) {
				std::filesystem::create_directory(dir);
			} else {
				doit = false;
			}
		}
		// Do the save!
		if (doit)
			kenv.saveLevel(levelNum);
	}
	if (kenv.platform == kenv.PLATFORM_PC && !kenv.isRemaster && kenv.version <= KEnvironment::KVERSION_OLYMPIC) {
		ImGui::SameLine();
		if (ImGui::Button("Test")) {
			bool success = launcher.loadLevel(levelNum);
			if (!success) {
				MsgBox(g_window, "The GameModule could not be launched!\nBe sure the path to the GameModule is correctly set in the project file.", 16);
			}
		}
	}
	ImGui::Separator();
	ImGui::DragFloat3("Cam pos", &camera.position.x, 0.1f);
	ImGui::DragFloat3("Cam ori", &camera.orientation.x, 0.1f);
	ImGui::DragFloat("Cam speed", &_camspeed, 0.1f);
	ImGui::DragFloatRange2("Depth range", &camera.nearDist, &camera.farDist);
	ImGui::Checkbox("Orthographic", &camera.orthoMode); ImGui::SameLine();
	if (ImGui::Button("Top-down view")) {
		camera.orientation = Vector3(-1.5707f, 0.0f, 0.0f);
	}
	ImGui::Separator();
	ImGui::RadioButton("Move", &guizmoOperation, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Rotate", &guizmoOperation, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Scale", &guizmoOperation, 2);
	ImGui::DragFloat3("Cursor", &cursorPosition.x, 0.1f);
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted("Selected:");
	ImGui::SameLine();
	ImGui::BeginDisabled(!nearestRayHit || nearestRayHit->getTypeID() == 0);
	if (ImGui::Button("Details") && nearestRayHit) {
		nearestRayHit->onDetails();
	}
	ImGui::EndDisabled();
	if (nearestRayHit && nearestRayHit->getTypeID() != 0) {
		ImGui::TextUnformatted(nearestRayHit->getInfo().c_str());
	}
	else {
		ImGui::TextDisabled("nothing selected");
	}
	ImGui::Separator();
	ImGui::InputInt("Show sector", &showingSector);
	ImGui::Checkbox("Scene nodes", &showNodes); ImGui::SameLine();
	ImGui::Checkbox("Textures", &showTextures); ImGui::SameLine();
	ImGui::Checkbox("Alpha clip", &enableAlphaClip);
	ImGui::Checkbox("Invisible nodes", &showInvisibleNodes); ImGui::SameLine();
	ImGui::Checkbox("Clone nodes", &showClones);
	ImGui::Checkbox("Beacons", &showBeacons); ImGui::SameLine();
	ImGui::Checkbox("Beacon kluster bounds", &showBeaconKlusterBounds); //ImGui::SameLine();
	ImGui::Checkbox("Grounds", &showGrounds); ImGui::SameLine();
	ImGui::Checkbox("Ground bounds", &showGroundBounds); ImGui::SameLine();
	ImGui::Checkbox("Infinite walls", &showInfiniteWalls);
	ImGui::Checkbox("Sas bounds", &showSasBounds); ImGui::SameLine();
	ImGui::Checkbox("Lines & splines", &showLines); //ImGui::SameLine();
	ImGui::Checkbox("Squads + choreos", &showSquadChoreos); ImGui::SameLine();
	ImGui::Checkbox("Squad bounds", &showSquadBoxes);
	ImGui::Checkbox("MsgAction bounds", &showMsgActionBoxes);
	if (kenv.version >= kenv.KVERSION_XXL2) {
		ImGui::PushItemWidth(ImGui::CalcItemWidth() * 0.5f);
		ImGui::InputInt("##ShowingChoreo", &showingChoreography); ImGui::SameLine();
		ImGui::InputInt("Choreo/Key", &showingChoreoKey);
		ImGui::PopItemWidth();
	}
	else
		ImGui::InputInt("Choreo key", &showingChoreoKey);
	ImGui::Checkbox("Pathfinding graph", &showPFGraph); ImGui::SameLine();
	ImGui::Checkbox("Markers", &showMarkers); ImGui::SameLine();
	ImGui::Checkbox("Detectors", &showDetectors);
	ImGui::Checkbox("Lights", &showLights);
}

void EditorInterface::IGBeaconGraph()
{
	const auto getBeaconName = [this](int id) {return g_encyclo.getBeaconName(id).c_str(); };
	static const char* bonusNamesX1[] = { "?", "Helmet", "Golden Helmet", "Potion", "Shield", "Ham", "x3 Multiplier", "x10 Multiplier", "Laurel", "Boar", "Retro Coin", "Remaster Coin" };
	static const char* bonusNamesX2[] = { "?", "Potion", "Helmet", "Golden Helmet", "Diamond Helmet", "x3 Multiplier", "x10 Multiplier", "Ham", "Shield" };
	const auto getBonusName = [this](int bonusId) -> const char* {
		if (bonusId == -1)
			return "/";
		if (kenv.version == kenv.KVERSION_XXL1)
			return bonusNamesX1[bonusId];
		else if (kenv.version == kenv.KVERSION_XXL2)
			return bonusNamesX2[bonusId];
		return "?";
	};
	CKSrvBeacon* srvBeacon = kenv.levelObjects.getFirst<CKSrvBeacon>();
	if (ImGui::Button("Add beacon")) {
		ImGui::OpenPopup("AddBeacon");
	}
	ImGui::SameLine();
	static int spawnSector = -1, spawnPos = 1;
	ImGui::TextUnformatted("Sector:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(80.0f);
	ImGui::InputInt("##spawnSector", &spawnSector);
	if (spawnSector < 0) spawnSector = 0;
	if (spawnSector > (int)kenv.numSectors) spawnSector = (int)kenv.numSectors;
	ImGui::SameLine();
	ImGui::TextUnformatted("at:");
	ImGui::SameLine();
	ImGui::RadioButton("Camera", &spawnPos, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Cursor", &spawnPos, 1);

	if (ImGui::Button("New bonus handler")) {
		ImGui::OpenPopup("AddHandler");
	}
	ImGui::SameLine();
	if (ImGui::Button("Update all kluster sphere bounds")) {
		for (CKObject *bk : kenv.levelObjects.getClassType<CKBeaconKluster>().objects)
			srvBeacon->updateKlusterBounds(bk->cast<CKBeaconKluster>());
		for (auto& str : kenv.sectorObjects)
			for (CKObject* bk : str.getClassType<CKBeaconKluster>().objects)
				srvBeacon->updateKlusterBounds(bk->cast<CKBeaconKluster>());
	}
	if (ImGui::BeginPopup("AddBeacon")) {
		for (auto& hs : srvBeacon->handlers) {
			ImGui::PushID(hs.handlerId);
			if (ImGui::MenuItem(getBeaconName(hs.handlerId))) {
				SBeacon beacon;
				if (spawnPos)
					beacon.setPosition(cursorPosition);
				else
					beacon.setPosition(camera.position + camera.direction * 2.5f);

				if (auto* jsBeacon = g_encyclo.getBeaconJson(hs.handlerId); jsBeacon && jsBeacon->is_object() && jsBeacon->contains("defaultParams"))
					beacon.params = (uint16_t)std::stoi(jsBeacon->at("defaultParams").get_ref<const std::string&>(), nullptr, 16);
				else if (hs.object && hs.object->isSubclassOf<CKCrateCpnt>())
					beacon.params = 0b001'010;
				else
					beacon.params = 0;

				int klusterIndex;
				if (kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
					std::tie(klusterIndex, std::ignore) = srvBeacon->addBeaconToNearestKluster(kenv, spawnSector, hs.handlerIndex, beacon);
				}
				else {
					klusterIndex = srvBeacon->addKluster(kenv, spawnSector);
					srvBeacon->addBeacon(spawnSector, klusterIndex, hs.handlerIndex, beacon);
				}
				srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[spawnSector].beaconKlusters[klusterIndex].get());
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(%02X %02X %02X %02X %02X)", hs.unk2a, hs.numBits, hs.handlerIndex, hs.handlerId, hs.persistent);
			ImGui::PopID();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopup("AddHandler")) {
		CKGroup* grpBonus = (kenv.version >= kenv.KVERSION_XXL2) ? (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonusX2>() : (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonus>();
		if (grpBonus) {
			for (CKGroup* grp = grpBonus->childGroup.get(); grp; grp = grp->nextGroup.get()) {
				if (CKGrpBonusPool* pool = grp->dyncast<CKGrpBonusPool>()) {
					int hid = pool->handlerId;
					// do not show bonus pools that already have a handler 
					auto it = std::find_if(srvBeacon->handlers.begin(), srvBeacon->handlers.end(), [hid](const auto& h) {return h.handlerId == hid; });
					if (it != srvBeacon->handlers.end())
						continue;
					if (ImGui::MenuItem(getBeaconName(hid))) {
						srvBeacon->addHandler(pool, 1, hid, 0, 0);
						uint32_t numBonuses = 0;
						for (CKHook* hk = pool->childHook.get(); hk; hk = hk->next.get())
							numBonuses++;
						pool->maxBeaconBonusesOnScreen = std::max(1u, numBonuses / 2u);
					}
				}
			}
		}
		else {
			ImGui::TextUnformatted("No bonus pools, no handlers!");
		}
		ImGui::EndPopup();
	}
	const auto enumBeaconKluster = [this,&getBeaconName](CKBeaconKluster* bk) {
		if (ImGui::TreeNode(bk, "Cluster (%f, %f, %f) radius %f", bk->bounds.center.x, bk->bounds.center.y, bk->bounds.center.z, bk->bounds.radius)) {
			ImGui::DragFloat3("Center##beaconKluster", &bk->bounds.center.x, 0.1f);
			ImGui::DragFloat("Radius##beaconKluster", &bk->bounds.radius, 0.1f);
			int nbing = 0;
			for (auto &bing : bk->bings) {
				int boffi = bing.bitIndex;
				if(!bing.beacons.empty())
					ImGui::BulletText("%s %02X %02X %02X %02X %02X %02X %04X %08X", getBeaconName(bing.handlerId), bing.unk2a, bing.numBits, bing.handlerId, bing.sectorIndex, bing.klusterIndex, bing.handlerIndex, bing.bitIndex, bing.unk6);
				int nbeac = 0;
				for (auto &beacon : bing.beacons) {
					ImGui::PushID(&beacon);
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					bool tn_open = ImGui::TreeNodeEx("beacon", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf, "(%i,%i) %f %f %f 0x%04X", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z, beacon.params);
					//if (ImGui::Selectable("##beacon")) {
					if (ImGui::IsItemClicked()) {
						camera.position = pos - camera.direction * 5.0f;
						selBeaconSector = bing.sectorIndex;
						selBeaconKluster = bing.klusterIndex;
						selBeaconBing = nbing;
						selBeaconIndex = nbeac;
					}
					if (tn_open) {
						ImGui::TreePop();
					}
					//ImGui::SameLine();
					//ImGui::Text("(%i,%i) %f %f %f", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z);
					ImGui::PopID();
					boffi += bing.numBits;
					nbeac++;
				}
				nbing++;
			}
			ImGui::TreePop();
		}
	};
	ImGui::Columns(2);
	ImGui::BeginChild("BeaconGraph");
	if (ImGui::TreeNode("Level")) {
		for (CKBeaconKluster *bk = kenv.levelObjects.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
			enumBeaconKluster(bk);
		ImGui::TreePop();
	}
	int i = 1;
	for (auto &str : kenv.sectorObjects) {
		if (ImGui::TreeNode(&str, "Sector %i", i)) {
			if (str.getClassType<CKBeaconKluster>().objects.size())
				for (CKBeaconKluster *bk = str.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
					enumBeaconKluster(bk);
			ImGui::TreePop();
		}
		i++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("BeaconInfo");
	bool removal = false; int remSector, remKluster, remBing, remBeacon;
	if (selBeaconSector != -1) {
		CKBeaconKluster* bk = srvBeacon->beaconSectors[selBeaconSector].beaconKlusters[selBeaconKluster].get();
		CKBeaconKluster::Bing& bing = bk->bings[selBeaconBing];
		SBeacon& beacon = bing.beacons[selBeaconIndex];

		if (ImGui::Button("Remove")) {
			removal = true;
			remSector = bing.sectorIndex;
			remKluster = bing.klusterIndex;
			remBing = selBeaconBing;
			remBeacon = selBeaconIndex;
		}

		ImGui::Text("%s (%02X, %s)", getBeaconName(bing.handlerId), bing.handlerId, bing.handler->getClassName());
		ImGui::Text("Bits:");
		std::vector<bool>::iterator bitIterator = srvBeacon->getBeaconBitIterator(selBeaconSector, selBeaconKluster, selBeaconBing, selBeaconIndex);
		for (int i = 0; i < bing.numBits; i++) {
			ImGui::SameLine();
			ImGui::Text("%i", *(bitIterator + i) ? 1 : 0);
		}
		bool mod = false;
		mod |= ImGui::DragScalarN("Position##beacon", ImGuiDataType_S16, &beacon.posx, 3, 0.1f);
		mod |= ImGui::InputScalar("Params##beacon", ImGuiDataType_U16, &beacon.params, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		if (CKCrateCpnt *crateCpnt = bing.handler->dyncast<CKCrateCpnt>()) {
			ImGui::Separator();
			int cc = beacon.params & 7;
			if (ImGui::InputInt("Num crates", &cc)) {
				cc = std::clamp(cc, 1, 7);
				beacon.params &= ~7;
				beacon.params |= (cc & 7);
				mod = true;
			}
			cc = (beacon.params >> 3) & 7;
			if (ImGui::InputInt("Num bonuses", &cc)) {
				cc = std::clamp(cc, 0, 7);
				beacon.params &= ~(7 << 3);
				beacon.params |= (cc & 7) << 3;
				mod = true;
			}
			cc = (beacon.params >> 6) & 7;
			if (ImGui::InputInt("Arrangement", &cc)) {
				beacon.params &= ~(7 << 6);
				beacon.params |= (cc & 7) << 6;
				mod = true;
			}
			cc = (beacon.params >> 9) & 3;
			if (ImGui::BeginListBox("Bonus", ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing()*4.0f + ImGui::GetStyle().FramePadding.y * 2.0f))) {
				ImGui::PushItemWidth(-1.0f);
				for (int bon = 0; bon < 4; bon++) {
					ImGui::PushID(bon);
					if (ImGui::RadioButton("##bonusRadio", &cc, bon)) {
						cc = bon;
						beacon.params &= ~(3 << 9);
						beacon.params |= (cc & 3) << 9;
						//mod = true;
					}
					ImGui::SameLine();
					int& bonusId = crateCpnt->bonuses[bon];
					if (ImGui::BeginCombo("##combo", getBonusName(bonusId))) {
						if (ImGui::Selectable("/", bonusId == -1))
							bonusId = -1;
						static std::vector<int> fndBonuses;
						fndBonuses.clear();
						CKGroup* grpBonus = (kenv.version >= kenv.KVERSION_XXL2) ? (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonusX2>() : (CKGroup*)kenv.levelObjects.getFirst<CKGrpBonus>();
						for (CKGroup* grp = grpBonus->childGroup.get(); grp; grp = grp->nextGroup.get())
							if (!grp->isSubclassOf<CKGrpWildBoarPool>()) // sorry, no wild boars inside crates :(
								fndBonuses.push_back(grp->cast<CKGrpBonusPool>()->bonusType);
						std::sort(fndBonuses.begin(), fndBonuses.end());
						for (int bid : fndBonuses)
							if (ImGui::Selectable(getBonusName(bid), bonusId == bid))
								bonusId = bid;
						ImGui::EndCombo();
					}
					ImGui::PopID();
				}
				ImGui::PopItemWidth();
				ImGui::EndListBox();
			}
		}
		else if (auto* jsBeaconInfo = g_encyclo.getBeaconJson(bing.handlerId)) {
			if (jsBeaconInfo->is_object()) {
				bool isOrientable = jsBeaconInfo->is_object() && jsBeaconInfo->value<bool>("orientable", false);
				if (isOrientable) {
					float angle = decode8bitAngle(beacon.params & 255) * 180.0f / M_PI;
					if (ImGui::SliderFloat("Orientation", &angle, 0.0f, 360.0f, u8"%.1f\u00B0")) {
						beacon.params = (beacon.params & 0xFF00) | (uint8_t)std::round(angle * 256.0f / 360.0f);
					}
				}
				if (auto itParams = jsBeaconInfo->find("params"); itParams != jsBeaconInfo->end()) {
					unsigned int modParams = beacon.params;
					if (PropFlagsEditor(modParams, itParams.value()))
						beacon.params = (uint16_t)modParams;
				}
			}
		}
		if (mod) {
			if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
				CKSrvBeacon* srvBeacon = kenv.levelObjects.getFirst<CKSrvBeacon>();
				auto bitIterator = srvBeacon->getBeaconBitIterator(selBeaconSector, selBeaconKluster, selBeaconBing, selBeaconIndex);
				for (int i = 0; i < 6; i++)
					*(bitIterator + i) = beacon.params & (1 << i);
				*(bitIterator + 6) = false;
			}
			if (kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
				SBeacon beaconCopy = beacon;
				srvBeacon->removeBeacon(selBeaconSector, selBeaconKluster, selBeaconBing, selBeaconIndex);
				srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[selBeaconSector].beaconKlusters[selBeaconKluster].get());
				srvBeacon->cleanEmptyKlusters(kenv, selBeaconSector);
				std::tie(selBeaconKluster, selBeaconIndex) = srvBeacon->addBeaconToNearestKluster(kenv, selBeaconSector, selBeaconBing, beaconCopy);
				srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[selBeaconSector].beaconKlusters[selBeaconKluster].get());
			}
			else {
				srvBeacon->updateKlusterBounds(bk);
			}
		}
	}
	ImGui::EndChild();
	ImGui::Columns();

	if (removal) {
		srvBeacon->removeBeacon(remSector, remKluster, remBing, remBeacon);
		if (kenv.version <= maxGameSupportingAdvancedBeaconEditing) {
			srvBeacon->updateKlusterBounds(srvBeacon->beaconSectors[remSector].beaconKlusters[remKluster].get());
			srvBeacon->cleanEmptyKlusters(kenv, remSector);
		}
		selBeaconSector = -1;
		rayHits.clear();
		nearestRayHit = nullptr;
	}
}

void EditorInterface::IGEnumNode(CKSceneNode *node, const char *description, bool isAnimBranch)
{
	if (!node)
		return;
	bool hassub = false;
	if (CSGBranch *branch = node->dyncast<CSGBranch>())
		hassub |= (bool)branch->child;
	if (CAnyAnimatedNode *anyanimnode = node->dyncast<CAnyAnimatedNode>())
		hassub |= (bool)anyanimnode->branchs;
	if (isAnimBranch) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
	bool open = ImGui::TreeNodeEx(node, (hassub ? 0 : ImGuiTreeNodeFlags_Leaf) | ((selNode == node) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick,
		"%s %s", node->getClassName(), description ? description : kenv.getObjectName(node));
	if (isAnimBranch) ImGui::PopStyleColor();
	if (ImGui::IsItemClicked()) {
		selNode = node;
	}
	if (!node->isSubclassOf<CSGRootNode>() && !node->isSubclassOf<CSGSectorRoot>()) {
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
			ImGui::SetDragDropPayload("SceneGraphNode", &node, sizeof(node));
			ImGui::Text("%s", node->getClassName());
			ImGui::EndDragDropSource();
		}
	}
	if (CSGBranch *branch = node->dyncast<CSGBranch>()) {
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SceneGraphNode")) {
				CKSceneNode *sub = *(CKSceneNode**)payload->Data;
				// don't allow target nodes parented to source node
				bool parentedToSource = false;
				for (CKSceneNode *pc = node->parent.get(); pc; pc = pc->parent.get())
					parentedToSource |= pc == sub;
				if (!parentedToSource) {
					sub->parent->cast<CSGBranch>()->removeChild(sub);
					branch->insertChild(sub);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
	if (open) {
		if (hassub) {
			if (CSGBranch *branch = node->dyncast<CSGBranch>()) {
				IGEnumNode(branch->child.get());
				if (CAnyAnimatedNode *anyanimnode = node->dyncast<CAnyAnimatedNode>())
					IGEnumNode(anyanimnode->branchs.get(), nullptr, true);
			}
		}
		ImGui::TreePop();
	}
	IGEnumNode(node->next.get(), nullptr, isAnimBranch);
}

void EditorInterface::IGSceneGraph()
{
	if (ImGui::Button("Add Node")) {
		CKSceneNode* par = this->selNode.get();
		while (par && !par->isSubclassOf<CSGSectorRoot>())
			par = par->parent.get();
		if (par) {
			int sector = kenv.getObjectSector(par);
			CNode* node = kenv.createAndInitObject<CNode>(sector);
			par->cast<CSGSectorRoot>()->insertChild(node);
			node->transform.setTranslation(cursorPosition);
			kenv.setObjectName(node, "New_Node");
		}
		else {
			MsgBox(g_window, "Select the sector root node (or any of its children) where you want to add the node.", 48);
		}
	}
	CSGSectorRoot* lvlroot = kenv.levelObjects.getObject<CSGSectorRoot>(0);
	IGEnumNode(lvlroot, "Common sector");
	for (int i = 0; i < (int)kenv.numSectors; i++) {
		CSGSectorRoot *strroot = kenv.sectorObjects[i].getObject<CSGSectorRoot>(0);
		char buf[40];
		sprintf_s(buf, "Sector %i", i+1);
		IGEnumNode(strroot, buf);
	}
}

void EditorInterface::IGSceneNodeProperties()
{
	CKSceneNode* selNode = this->selNode.get();
	if (!selNode) {
		ImGui::Text("No node selected!");
		return;
	}

	ImGui::Text("%p %s : %s", selNode, selNode->getClassName(), kenv.getObjectName(selNode));
	IGObjectDragDropSource(*this, selNode);
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
			if (payload->IsDataType("CKObject")) {
				CKObject* obj = *(CKObject**)payload->Data;
				if (obj->isSubclassOf<CKSceneNode>())
					if (const ImGuiPayload* acceptedPayload = ImGui::AcceptDragDropPayload("CKObject"))
						this->selNode = *(CKSceneNode**)payload->Data;
			}
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::Separator();

	Matrix globalMat = selNode->getGlobalMatrix();
	if (ImGui::Button("Place camera there")) {
		Matrix& m = globalMat;
		camera.position = Vector3(m._41, m._42, m._43) - camera.direction * 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Find hook")) {
		wndShowHooks = true;
		for (auto& hkclass : kenv.levelObjects.categories[CKHook::CATEGORY].type) {
			for (CKObject* obj : hkclass.objects) {
				CKHook* hook = obj->cast<CKHook>();
				if (hook->node.bound) {
					if (hook->node.get() == selNode) {
						selectedHook = hook;
						viewGroupInsteadOfHook = false;
					}
				}
			}
		}
	}

	IGObjectNameInput("Name", selNode, kenv);

	Vector3 locTranslation, locRotation, locScale;
	bool recompose = false;
	ImGuizmo::DecomposeMatrixToComponents(selNode->transform.v, &locTranslation.x, &locRotation.x, &locScale.x);
	ImGui::BeginDisabled();
	if (ImGui::DragFloat3("Global Position", &globalMat._41, 0.1f)) {
		//selNode->transform = globalMat * (selNode->transform.getInverse4x3() * globalMat).getInverse4x3();
	}
	ImGui::EndDisabled();
	ImGui::DragFloat3("Local Position", &selNode->transform._41, 0.1f);
	recompose |= ImGui::DragFloat3("Local Rotation (deg)", &locRotation.x);
	recompose |= ImGui::DragFloat3("Local Scale", &locScale.x, 0.1f);
	if (recompose) {
		ImGuizmo::RecomposeMatrixFromComponents(&locTranslation.x, &locRotation.x, &locScale.x, selNode->transform.v);
	}
	ImGui::InputScalar("Flags1", ImGuiDataType_U32, &selNode->unk1, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputScalar("Flags2", ImGuiDataType_U8, &selNode->unk2, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
	if (selNode->isSubclassOf<CNode>()) {
		CNode *geonode = selNode->cast<CNode>();
		if (ImGui::Button("Import geometry from DFF")) {
			auto filepath = OpenDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				std::unique_ptr<RwClump> impClump = LoadDFF(filepath.c_str());
				std::vector<RwGeometry*> rwgeos;
				for (RwAtomic& atom : impClump->atomics) {
					RwGeometry *rwgeotot = impClump->geoList.geometries[atom.geoIndex].get();
					rwgeos.push_back(rwgeotot);
				}
				ChangeNodeGeometry(kenv, geonode, rwgeos.data(), rwgeos.size());

				progeocache.clear();
			}
		}
		if (!geonode->geometry) {
			ImGui::Text("No geometry");
		}
		else {
			if (ImGui::Button("Export geometry to DFF")) {
				auto filepath = SaveDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
				if (!filepath.empty()) {
					CKAnyGeometry* wgeo = geonode->geometry.get();
					CKAnyGeometry* kgeo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
					auto sharedRwgeo = std::make_shared<RwGeometry>(*kgeo->clump->atomic.geometry.get());
					RwGeometry& rwgeo = *sharedRwgeo;
					rwgeo.nativeVersion.reset();

					auto flushClump = [&]()
						{
							RwExtHAnim* hanim = nullptr;
							if (geonode->isSubclassOf<CAnimatedNode>()) {
								RwFrameList* framelist = geonode->cast<CAnimatedNode>()->frameList.get();
								hanim = (RwExtHAnim*)framelist->extensions[0].find(0x11E);
								assert(hanim);
							}

							RwClump clump = CreateClumpFromGeo(sharedRwgeo, hanim);

							IOFile dff(filepath.c_str(), "wb");
							clump.serialize(&dff);
						};

					int splitPartNumber = 1;
					const auto baseName = filepath.stem().u8string();
					while (wgeo = wgeo->nextGeo.get()) {
						CKAnyGeometry* kgeo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
						if (rwgeo.numVerts + kgeo->clump->atomic.geometry->numVerts <= 65536) {
							rwgeo.merge(*kgeo->clump->atomic.geometry);
						}
						else {
							// too many vertices, we need to split
							filepath = filepath.parent_path() / std::filesystem::u8path(fmt::format("{}_Part{}{}", baseName, splitPartNumber, filepath.extension().u8string()));
							flushClump();
							splitPartNumber += 1;
							rwgeo = *kgeo->clump->atomic.geometry;
							filepath = filepath.parent_path() / std::filesystem::u8path(fmt::format("{}_Part{}{}", baseName, splitPartNumber, filepath.extension().u8string()));
						}
					}
					flushClump();
				}
			}
			CKAnyGeometry *kgeo = geonode->geometry.get();
			if (geonode->geometry->flags & 8192) {
				static const char * const costumeNames[4] = { "Gaul", "Roman", "Pirate", "Swimsuit" };
				geonode->geometry->costumes;
				if (ImGui::BeginListBox("Costume", ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 5.0f))) {
					for (size_t i = 0; i < kgeo->costumes.size(); i++) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##costumeEntry", kgeo->clump == kgeo->costumes[i])) {
							for (CKAnyGeometry *gp = kgeo; gp; gp = gp->nextGeo.get())
								gp->clump = gp->costumes[i];
						}
						ImGui::SameLine();
						if (i >= 4) ImGui::TextUnformatted("???");
						else ImGui::TextUnformatted(costumeNames[i]);
						ImGui::PopID();
					}
					ImGui::EndListBox();
				}
			}
			if (ImGui::CollapsingHeader("Materials")) {
				for (CKAnyGeometry* wgeo = geonode->geometry.get(); wgeo; wgeo = wgeo->nextGeo.get()) {
					CKAnyGeometry* geo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
					if (!geo->clump) {
						ImGui::BulletText("(Geometry with no clump)");
						continue;
					}
					const char* matname = "(no texture)";
					RwGeometry* rwgeo = geo->clump->atomic.geometry.get();
					if (!rwgeo->materialList.materials.empty())
						if (rwgeo->materialList.materials[0].isTextured)
							matname = geo->clump->atomic.geometry->materialList.materials[0].texture.name.c_str();
					ImGui::PushID(geo);
					ImGui::BulletText("%s%s", matname, (geo != wgeo) ? " (duplicated geo)" : "");
					ImGui::InputScalar("Flags 1", ImGuiDataType_U32, &geo->flags, 0, 0, "%X", ImGuiInputTextFlags_CharsHexadecimal);
					ImGui::InputScalar("Flags 2", ImGuiDataType_U32, &geo->flags2, 0, 0, "%X", ImGuiInputTextFlags_CharsHexadecimal);
					if (geo->clump) {
						auto& flags = geo->clump->atomic.geometry->flags;
						ImGui::LabelText("RwGeo Flags", "%08X", flags);
						ImGui::CheckboxFlags("Rw Light", &flags, RwGeometry::RWGEOFLAG_LIGHT); ImGui::SameLine();
						ImGui::CheckboxFlags("Rw Modulate Colors", &flags, RwGeometry::RWGEOFLAG_MODULATECOLOR);
					}
					if (geo->material && kenv.hasClass<CMaterial>()) {
						ImGuiMemberListener ml{ kenv, *this };
						ml.setPropertyInfoList(g_encyclo, geo->material.get());
						geo->material->reflectMembers2(ml, &kenv);
					}
					ImGui::PopID();
					ImGui::Separator();
				}
			}
		}
	}
	if (CFogBoxNodeFx *fogbox = selNode->dyncast<CFogBoxNodeFx>()) {
		if (ImGui::CollapsingHeader("Fog box")) {
			ImGuiMemberListener ml(kenv, *this);
			ml.setPropertyInfoList(g_encyclo, fogbox);
			fogbox->reflectFog(ml, &kenv);
		}
	}
}

void EditorInterface::IGGroundEditor()
{
	static bool hideDynamicGrounds = true;
	ImGui::Checkbox("Hide dynamic", &hideDynamicGrounds);
	ImGui::SameLine();
	if (ImGui::Button("Autoname")) {
		auto gnstr = [this](KObjectList& objlist, int strnum) {
			int next = 0;
			for (CKObject* obj : objlist.getClassType<CGround>().objects) {
				char buf[32];
				sprintf_s(buf, "GndS%02u_%04u", strnum, next++);
				kenv.setObjectName(obj, buf);
			}
		};
		gnstr(kenv.levelObjects, 0);
		for (int strnum = 0; strnum < (int)kenv.numSectors; ++strnum)
			gnstr(kenv.sectorObjects[strnum], strnum + 1);
	}
	ImGui::SameLine();
	if (ImGui::Button("Make Scene geometry from Grounds")) {
		for (int i = -1; i < (int)kenv.numSectors; i++) {
			KObjectList& objlist = (i == -1) ? kenv.levelObjects : kenv.sectorObjects[i];

			CKMeshKluster* mkluster = objlist.getFirst<CKMeshKluster>();
			RwGeometry mergedGeo; bool firstGeo = true;
			for (kobjref<CGround>& gnd : mkluster->grounds) {
				if (gnd->isSubclassOf<CDynamicGround>())
					continue;
				auto optGndGeo = GroundGeo::generateGroundGeo(gnd.get(), false);
				if (optGndGeo) {
					auto& gndGeo = *optGndGeo;
					// sphere calculation
					BoundingSphere bs(gndGeo.positions[0], 0.0f);
					for (Vector3& pos : gndGeo.positions)
						bs.mergePoint(pos);

					// Default geometry values
					RwGeometry rwgeo;
					rwgeo.flags = RwGeometry::RWGEOFLAG_POSITIONS;
					rwgeo.numMorphs = 1;
					rwgeo.hasVertices = 1;
					rwgeo.hasNormals = 0;
					rwgeo.materialList.slots = { 0xFFFFFFFF };
					RwMaterial mat;
					mat.flags = 0;
					mat.color = 0xFFFFFFFF;
					mat.unused = 0;
					mat.isTextured = 0;
					mat.ambient = mat.specular = mat.diffuse = 1.0f;
					rwgeo.materialList.materials = { std::move(mat) };

					rwgeo.numVerts = gndGeo.positions.size();
					rwgeo.numTris = gndGeo.triangles.size();
					rwgeo.verts = std::move(gndGeo.positions);
					rwgeo.tris.resize(rwgeo.numTris);
					for (size_t i = 0; i < rwgeo.tris.size(); ++i)
						rwgeo.tris[i].indices = std::move(gndGeo.triangles[i]);
					rwgeo.spherePos = bs.center;
					rwgeo.sphereRadius = bs.radius;
					rwgeo.flags |= RwGeometry::RWGEOFLAG_PRELIT;
					rwgeo.colors = std::move(gndGeo.colors);

					if (firstGeo) {
						mergedGeo = std::move(rwgeo);
						firstGeo = false;
					}
					else {
						mergedGeo.merge(rwgeo);
					}
				}
			}
			if (firstGeo)
				continue;

			CSGSectorRoot* sgsr = objlist.getFirst<CSGSectorRoot>();
			RwGeometry* rwgeo = &mergedGeo;
			ChangeNodeGeometry(kenv, sgsr, &rwgeo, 1);
			if (kenv.version >= kenv.KVERSION_XXL2) {
				for (auto* geo = sgsr->geometry.get(); geo; geo = geo->nextGeo.get()) {
					geo->flags = 5;
					geo->flags2 = 6;
					geo->material->flags = 0;
				}
			}
		}
		progeocache.clear();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Replace all rendering sector root geometries by ground models,\nso that you can see the ground collision ingame.\n/!\\ You will lose all your current sector geometries!");
	ImGui::Columns(2);
	ImGui::BeginChild("GroundTree");
	auto feobjlist = [this](KObjectList &objlist, const char *desc, int sectorNumber) {
		if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
			ImGui::PushID(mkluster);
			bool tropen = ImGui::TreeNode(mkluster, "%s", desc);
			ImGui::SameLine();
			if (ImGui::SmallButton("Import")) {
				auto filepath = OpenDialogBox(g_window, "Wavefront OBJ file\0*.OBJ\0\0", "obj");
				if (!filepath.empty()) {
					ImportGroundOBJ(kenv, filepath, sectorNumber);
					gndmdlcache.clear();
				}
			}
			//if (ImGui::IsItemHovered())
			//	ImGui::SetTooltip("No walls yet! Corruption risk!");
			ImGui::SameLine();
			if (ImGui::SmallButton("Export")) {
				auto filepath = SaveDialogBox(g_window, "Wavefront OBJ file\0*.OBJ\0\0", "obj");
				if (!filepath.empty()) {
					FILE *obj;
					fsfopen_s(&obj, filepath, "wt");
					uint16_t gndx = 0;
					uint32_t basevtx = 1;
					for (const auto &gnd : mkluster->grounds) {
						if (gnd->isSubclassOf<CDynamicGround>() && hideDynamicGrounds)
							continue;
						fprintf(obj, "o %s\n", kenv.getObjectName(gnd.get()));
						for (auto &vtx : gnd->vertices) {
							fprintf(obj, "v %f %f %f\n", vtx.x, vtx.y, vtx.z);
						}
						for (auto &fac : gnd->triangles) {
							fprintf(obj, "f %u %u %u\n", basevtx + fac.indices[0], basevtx + fac.indices[1], basevtx + fac.indices[2]);
						}
						uint32_t wallvtx = basevtx + gnd->vertices.size();
						for (auto &wall : gnd->finiteWalls) {
							for (int p = 0; p < 2; p++) {
								Vector3 v = gnd->vertices[wall.baseIndices[p]];
								fprintf(obj, "v %f %f %f\n", v.x, v.y + wall.heights[p], v.z);
							}
							uint32_t faceIndices[4] = { basevtx + wall.baseIndices[0], basevtx + wall.baseIndices[1], wallvtx + 1, wallvtx };
							// flip face except for walls with negative heights
							if (!(wall.heights[0] < 0.0f && wall.heights[1] < 0.0f))
								std::swap(faceIndices[1], faceIndices[3]);
							fprintf(obj, "f %u %u %u %u\n", faceIndices[0], faceIndices[1], faceIndices[2], faceIndices[3]);
							wallvtx += 2;
						}
						for (auto &infwall : gnd->infiniteWalls) {
							/*
							for (int p = 0; p < 2; p++) {
								Vector3 v = gnd->vertices[infwall.baseIndices[p]];
								fprintf(obj, "v %f %f %f\n", v.x, 10001.0f, v.z);
							}
							fprintf(obj, "f %u %u %u %u\n", basevtx + infwall.baseIndices[0], basevtx + infwall.baseIndices[1], wallvtx + 1, wallvtx);
							wallvtx += 2;
							*/
							fprintf(obj, "l %u %u\n", basevtx + infwall.baseIndices[0], basevtx + infwall.baseIndices[1]);
						}
						basevtx = wallvtx;
					}
					fclose(obj);
				}
			}
			if (tropen) {
				for (auto &gnd : mkluster->grounds) {
					if (gnd->isSubclassOf<CDynamicGround>() && hideDynamicGrounds)
						continue;
					const char *type = "(G)";
					if (gnd->isSubclassOf<CDynamicGround>())
						type = "(D)";
					if (gnd->getRefCount() > 1)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
					bool p = ImGui::TreeNodeEx(gnd.get(), ImGuiTreeNodeFlags_Leaf | ((gnd.get() == selGround.get()) ? ImGuiTreeNodeFlags_Selected : 0), "%s %s (%02u,%02u)", type, kenv.getObjectName(gnd.get()), gnd->param1, gnd->param2);
					IGObjectDragDropSource(*this, gnd.get());
					if (ImGui::IsItemClicked())
						selGround = gnd.get();
					if (p)
						ImGui::TreePop();
					if (gnd->getRefCount() > 1)
						ImGui::PopStyleColor();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	};
	feobjlist(kenv.levelObjects, "Level", -1);
	int x = 0;
	for (auto &str : kenv.sectorObjects) {
		char lol[64];
		sprintf_s(lol, "Sector %i", x+1);
		feobjlist(str, lol, x);
		x++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("SelGroundInfo");
	if (selGround) {
		if (ImGui::Button("Delete")) {
			ImGui::OpenPopup("GroundDelete");
		}
		IGObjectNameInput("Name", selGround.get(), kenv);
		auto CheckboxFlags16 = [](const char *label, uint16_t *flags, unsigned int val) {
			unsigned int up = *flags;
			if (ImGui::CheckboxFlags(label, &up , val))
				*flags = up;
		};
		ImGui::InputScalar("Flags 1", ImGuiDataType_U16, &selGround->param1, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("Flags 2", ImGuiDataType_U16, &selGround->param2, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("Negative height", ImGuiDataType_Float, &selGround->param3);
		ImGui::InputScalar("Parameter", ImGuiDataType_Float, &selGround->param4);
		if (auto* jsGround = g_encyclo.getClassJson(CGround::FULL_ID)) {
			if (auto itProps = jsGround->find("properties"); itProps != jsGround->end()) {
				for (auto& [paramName, param] : { std::tie("param1", selGround->param1), std::tie("param2", selGround->param2) })
				{
					try {
						auto& jsFlags = itProps->at(paramName).at("bitFlags");
						ImGui::Separator();
						unsigned int flags = param;
						if (PropFlagsEditor(flags, jsFlags))
							param = (uint16_t)flags;
					}
					catch (const nlohmann::json::exception&) {}
				}
			}
		}

		if (ImGui::BeginPopup("GroundDelete")) {
			ImGui::Text("DO NOT delete if there are grounds referenced by scripting events,\nor else weird things and crashes will happen,\ncorrupting your level forever!\n(Hopefully this can be improved and prevented\nin future versions of the editor.)");
			if (ImGui::Button("I don't care, just do it!")) {
				for (int i = -1; i < (int)kenv.numSectors; i++) {
					KObjectList &objlist = (i == -1) ? kenv.levelObjects : kenv.sectorObjects[i];
					if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
						auto it = std::find_if(mkluster->grounds.begin(), mkluster->grounds.end(), [this](const kobjref<CGround> &ref) {return ref.get() == selGround.get(); });
						if (it != mkluster->grounds.end())
							mkluster->grounds.erase(it);
					}
				}
				if (selGround->getRefCount() == 0) {
					kenv.removeObject(selGround.get());
					selGround = nullptr;
					rayHits.clear();
					nearestRayHit = nullptr;
				}
			}
			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGEventEditor()
{
	CKSrvEvent *srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>();
	if (!srvEvent) return;

	if (ImGui::Button("Find targets...")) {
		ImGui::OpenPopup("EvtFindTargets");
	}
	if (ImGui::BeginPopup("EvtFindTargets")) {
		static int tgCat = 0, tgId = 0;
		ImGui::InputInt("Category", &tgCat);
		ImGui::InputInt("ID", &tgId);
		if (ImGui::Button("Find")) {
			size_t ev = 0, s = 0;
			for (auto &bee : srvEvent->sequences) {
				for (int i = 0; i < bee.numActions; i++) {
					if ((srvEvent->objs[ev + i].id & 0x1FFFF) == (tgCat | (tgId << 6)))
						printf("class (%i, %i) found at seq %i ev 0x%04X\n", tgCat, tgId, s, srvEvent->objInfos[ev + i]);
				}
				ev += bee.numActions;
				s++;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Export TSV")) {
		auto filename = SaveDialogBox(g_window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
		if (!filename.empty()) {
			std::map<std::pair<int, int>, std::set<uint16_t>> evtmap;
			size_t ev = 0, s = 0;
			for (auto &bee : srvEvent->sequences) {
				for (int i = 0; i < bee.numActions; i++) {
					auto &obj = srvEvent->objs[ev + i];
					evtmap[std::make_pair(obj.id & 63, (obj.id >> 6) & 2047)].insert(srvEvent->objInfos[ev + i]);
				}
				ev += bee.numActions;
				s++;
			}
			FILE *file;
			fsfopen_s(&file, filename, "wt");
			for (auto &mapentry : evtmap) {
				for (uint16_t evt : mapentry.second) {
					fprintf(file, "%i\t%i\t%04X\n", mapentry.first.first, mapentry.first.second, evt);
				}
			}
			fclose(file);
		}
	}
	ImGui::SameLine();
	ImGui::Text("Decoded: %i/%i", std::count_if(srvEvent->sequences.begin(), srvEvent->sequences.end(), [](CKSrvEvent::EventSequence &bee) {return bee.userFound; }), srvEvent->sequences.size());

	ImGui::Columns(2);
	if (ImGui::BeginTabBar("EvtSeqTypeBar")) {
		size_t ev = 0, i = 0;
		for (int et = 0; et < 3; ++et) {
			static const char* etNameArr[3] = { "Or", "And", "Simultaneous" };
			if (ImGui::BeginTabItem(etNameArr[et])) {
				if (ImGui::Button("New")) {
					srvEvent->sequences.emplace(srvEvent->sequences.begin() + i);
					srvEvent->evtSeqIDs.insert(srvEvent->evtSeqIDs.begin() + i, srvEvent->nextSeqID++);
					srvEvent->evtSeqNames.emplace(srvEvent->evtSeqNames.begin() + i, "New sequence");
					srvEvent->numSeqs[et] += 1;
				}
				ImGui::SameLine();
				bool pleaseRemove = false;
				if (ImGui::Button("Remove")) {
					pleaseRemove = true;
				}
				ImGui::BeginChild("EventSeqList");
				for (int j = 0; j < srvEvent->numSeqs[et]; ++j) {
					if (pleaseRemove && i == selectedEventSequence) {
						size_t actFirst = ev, actLast = ev + srvEvent->sequences[i].numActions;
						srvEvent->objs.erase(srvEvent->objs.begin() + actFirst, srvEvent->objs.begin() + actLast);
						srvEvent->objInfos.erase(srvEvent->objInfos.begin() + actFirst, srvEvent->objInfos.begin() + actLast);
						srvEvent->sequences.erase(srvEvent->sequences.begin() + i);
						srvEvent->evtSeqIDs.erase(srvEvent->evtSeqIDs.begin() + i);
						srvEvent->evtSeqNames.erase(srvEvent->evtSeqNames.begin() + i);
						srvEvent->numSeqs[et] -= 1;
						pleaseRemove = false;
						continue; // do not increment i nor ev
					}
					auto& bee = srvEvent->sequences[i];
					ImGui::PushID(i);
					if (ImGui::Selectable("##EventSeqEntry", i == selectedEventSequence, 0, ImVec2(0, ImGui::GetTextLineHeight() * 2.0f)))
						selectedEventSequence = i;
					if (ImGui::BeginDragDropSource()) {
						const auto& seqid = srvEvent->evtSeqIDs[i];
						ImGui::SetDragDropPayload("EventSeq", &seqid, sizeof(seqid));
						ImGui::Text("Event sequence\n%s", srvEvent->evtSeqNames[i].c_str());
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EventNodeX1")) {
							EventNodeX1* node = *(EventNodeX1**)payload->Data;
							node->seqIndex = srvEvent->evtSeqIDs[i];
							node->bit = 0; // TODO: Take available free bit
						}
						ImGui::EndDragDropTarget();
					}
					ImGui::SameLine();
					ImGui::BulletText("%s (%i, %02X)\nUsed by %s", srvEvent->evtSeqNames[i].c_str(), bee.numActions, bee.bitMask, bee.users.size() ? bee.users[0]->getClassName() : "?");
					ImGui::PopID();
					ev += bee.numActions;
					i++;
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			else {
				for (int j = 0; j < srvEvent->numSeqs[et]; ++j) {
					ev += srvEvent->sequences[i].numActions;
					i++;
				}
			}
		}
		ImGui::EndTabBar();
	}
	ImGui::NextColumn();

	ImGui::BeginChild("EventEditor");
	if (selectedEventSequence >= 0 && selectedEventSequence < (int)srvEvent->sequences.size()) {
		auto &bee = srvEvent->sequences[selectedEventSequence];
		int ev = 0;
		for (int i = 0; i < selectedEventSequence; i++) {
			ev += srvEvent->sequences[i].numActions;
		}
		ImGui::Text("Used by %s", bee.users.size() ? bee.users[0]->getClassName() : "?");
		for (size_t u = 1; u < bee.users.size(); u++) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), ", %s", bee.users[u]->getClassName());
		}
		auto& name = srvEvent->evtSeqNames[selectedEventSequence];
		ImGui::InputText("Name", (char*)name.c_str(), name.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &name);
		ImGui::InputScalar("Bitmask", ImGuiDataType_U8, &bee.bitMask, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
		char checkboxName[] = "?##BitmaskBit";
		for (int i = 0; i < 8; ++i) {
			uint8_t flag = (uint8_t)(1 << i);
			if (i != 0) ImGui::SameLine();
			bool b = (bee.bitMask & flag) != 0;
			checkboxName[0] = (char)('0' + i);
			if (ImGui::Checkbox(checkboxName, &b))
				bee.bitMask = (bee.bitMask & ~flag) | (uint8_t)((b ? 1 : 0) << i);
		}
		if (ImGui::Button("Add")) {
			auto it = srvEvent->objs.emplace(srvEvent->objs.begin() + ev + bee.numActions);
			it->bound = true;
			srvEvent->objInfos.emplace(srvEvent->objInfos.begin() + ev + bee.numActions);
			bee.numActions += 1;
		}
		int deleteEvent = -1;
		for (uint8_t i = 0; i < bee.numActions; i++) {
			ImGui::PushID(ev + i);

			if (srvEvent->objs[ev + i].bound) {
				ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
				IGObjectSelectorRef(*this, "##evtTargetObj", srvEvent->objs[ev + i].ref);
			}
			else {
				uint32_t& encref = srvEvent->objs[ev + i].id;
				uint32_t clcat = encref & 63;
				uint32_t clid = (encref >> 6) & 2047;
				uint32_t objnb = encref >> 17;
				ImGui::PushItemWidth(48.0f);
				ImGui::InputScalar("##EvtObjClCat", ImGuiDataType_U32, &clcat); ImGui::SameLine();
				ImGui::InputScalar("##EvtObjClId", ImGuiDataType_U32, &clid); ImGui::SameLine();
				ImGui::InputScalar("##EvtObjNumber", ImGuiDataType_U32, &objnb); //ImGui::SameLine();
				encref = clcat | (clid << 6) | (objnb << 17);
				ImGui::PopItemWidth();
			}
			ImGui::SameLine();
			if (ImGui::Button("X##EvtRemove")) {
				deleteEvent = ev + i;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Remove");

			ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
			IGEventMessageSelector(*this, "##EventMessage", srvEvent->objInfos[ev + i], srvEvent->objs[ev + i].ref.get());

			ImGui::PopID();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		}
		if (deleteEvent >= 0) {
			srvEvent->objs.erase(srvEvent->objs.begin() + deleteEvent);
			srvEvent->objInfos.erase(srvEvent->objInfos.begin() + deleteEvent);
			bee.numActions -= 1;
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGSoundEditor()
{
	static auto massByteSwap = [](void* data, size_t numBytes) {
		assert((numBytes & 1) == 0);
		size_t numShorts = numBytes / 2;
		uint16_t* data16 = (uint16_t*)data;
		for (size_t i = 0; i < numShorts; ++i)
			data16[i] = ((data16[i] & 255) << 8) | (data16[i] >> 8);
	};
	static auto exportSound = [](RwSound &snd, const std::filesystem::path& path, KEnvironment& kenv) {
		WavDocument wav;
		wav.formatTag = 1;
		wav.numChannels = 1;
		wav.samplesPerSec = snd.info.dings[0].sampleRate;
		wav.avgBytesPerSec = wav.samplesPerSec * 2;
		wav.pcmBitsPerSample = 16;
		wav.blockAlign = ((wav.pcmBitsPerSample + 7) / 8) * wav.numChannels;
		wav.data = snd.data.data;
		if (kenv.platform == KEnvironment::PLATFORM_X360 || kenv.platform == KEnvironment::PLATFORM_PS3) {
			massByteSwap(wav.data.data(), wav.data.size());
		}
		IOFile out = IOFile(path.c_str(), "wb");
		wav.write(&out);
	};
	auto enumDict = [this](CKSoundDictionary *sndDict, int strnum) {
		if (sndDict->sounds.empty())
			return;
		if (ImGui::TreeNode(sndDict, (strnum == 0) ? "Level" : "Sector %i", strnum)) {
			//if (ImGui::Button("Random shuffle")) {
			//	std::random_shuffle(sndDict->rwSoundDict.list.sounds.begin(), sndDict->rwSoundDict.list.sounds.end());
			//}
			//ImGui::SameLine();
			if (ImGui::Button("Export all")) {
				char dirname[MAX_PATH + 1], pname[MAX_PATH + 1];
				BROWSEINFOA bri;
				memset(&bri, 0, sizeof(bri));
				bri.hwndOwner = (HWND)g_window->getNativeWindow();
				bri.pszDisplayName = dirname;
				bri.lpszTitle = "Export all the sounds to folder:";
				bri.ulFlags = BIF_USENEWUI | BIF_RETURNONLYFSDIRS;
				PIDLIST_ABSOLUTE pid = SHBrowseForFolderA(&bri);
				if (pid != NULL) {
					SHGetPathFromIDListA(pid, dirname);
					for (auto &snd : sndDict->rwSoundDict.list.sounds) {
						char *np = strrchr((char*)snd.info.name.data(), '\\');
						if (!np) np = (char*)snd.info.name.data();
						sprintf_s(pname, "%s/%s", dirname, np);
						exportSound(snd, pname, kenv);
					}
				}
			}
			for (int sndid = 0; sndid < (int)sndDict->sounds.size(); sndid++) {
				auto &snd = sndDict->rwSoundDict.list.sounds[sndid];
				ImGui::PushID(sndid);
				ImGui::AlignTextToFramePadding();
				ImGui::BeginGroup();
				ImGui::Text("%3i", sndid);
				ImGui::SameLine();
				if (ImGui::ArrowButton("PlaySound", ImGuiDir_Right))
					PlaySnd(kenv, snd);
				if(ImGui::IsItemHovered()) ImGui::SetTooltip("Play");
				ImGui::SameLine();
				if (ImGui::Button("I")) {
					auto filepath = OpenDialogBox(g_window, "WAV audio file\0*.WAV\0\0", "wav");
					if (!filepath.empty()) {
						IOFile wf = IOFile(filepath.c_str(), "rb");
						WavDocument wav;
						wav.read(&wf);
						WavSampleReader wsr(&wav);
						if (wav.formatTag == 1 || wav.formatTag == 3) {
							if (wav.numChannels != 1) {
								MsgBox(g_window, "The WAV contains multiple channels (e.g. stereo).\nOnly the first channel will be imported.", 48);
							}

							size_t numSamples = wav.getNumSamples();
							auto &ndata = snd.data.data;
							ndata.resize(numSamples * 2);
							int16_t *pnt = (int16_t*)ndata.data();
							for (size_t i = 0; i < numSamples; i++)
								*(pnt++) = (int16_t)(wsr.nextSample() * 32767);
							if (kenv.platform == KEnvironment::PLATFORM_X360 || kenv.platform == KEnvironment::PLATFORM_PS3)
								massByteSwap(ndata.data(), ndata.size());

							for (auto &ding : snd.info.dings) {
								ding.sampleRate = wav.samplesPerSec;
								ding.dataSize = ndata.size();
							}
							sndDict->sounds[sndid].sampleRate = wav.samplesPerSec;
						}
						else {
							MsgBox(g_window, "The WAV file doesn't contain uncompressed mono 8/16-bit PCM wave data.\nPlease convert it to this format first.", 48);
						}
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import");
				ImGui::SameLine();
				if (ImGui::Button("E")) {
					const char *name = strrchr((const char *)snd.info.name.data(), '\\');
					if (name) name++;
					auto filepath = SaveDialogBox(g_window, "WAV audio file\0*.WAV\0\0", "wav", name);
					if (!filepath.empty()) {
						exportSound(snd, filepath, kenv);
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export");
				ImGui::SameLine();
				const char *name = (const char*)snd.info.name.data();
				ImGui::Text("%s", latinToUtf8(name).c_str());
				//ImGui::Text("%u %u", snd.info.dings[0].sampleRate, snd.info.dings[1].sampleRate);
				ImGui::EndGroup();
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
					int32_t fullIndex = (strnum << 24) | sndid;
					ImGui::SetDragDropPayload("SoundIndex", &fullIndex, sizeof(fullIndex));
					ImGui::Text("Sound Sector %i ID %i", strnum, sndid);
					ImGui::EndDragDropSource();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	};
	enumDict(kenv.levelObjects.getFirst<CKSoundDictionary>(), 0);
	for (int i = 0; i < (int)kenv.numSectors; i++)
		enumDict(kenv.sectorObjects[i].getFirst<CKSoundDictionary>(), i+1);
}

void EditorInterface::IGSquadEditor()
{
	ImGui::Columns(2);
	ImGui::BeginChild("SquadList");
	int wantToAdd = -1;
	if (ImGui::Button("New"))
		wantToAdd = CKGrpSquadEnemy::FULL_ID;
	ImGui::SameLine();
	if (ImGui::Button("New JetPack"))
		wantToAdd = CKGrpSquadJetPack::FULL_ID;
	if (wantToAdd != -1) {
		CKGrpSquadEnemy* squad = (CKGrpSquadEnemy*)kenv.createObject((uint32_t)wantToAdd, -1);
		squad->bundle = kenv.createObject<CKBundle>(-1);
		squad->msgAction = kenv.createObject<CKMsgAction>(-1);

		CKGrpEnemy* grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>();
		grpEnemy->addGroup(squad);

		squad->bundle->flags = 0x7F;
		CKServiceLife* svcLife = kenv.levelObjects.getFirst<CKServiceLife>();
		svcLife->addBundle(squad->bundle.get());

		squad->msgAction->states = { CKMsgAction::MAState{ { CKMsgAction::MAMessage{0xD16, { {14, {} } } } } } };
		squad->mat1 = Matrix::getTranslationMatrix(cursorPosition);
		squad->mat2 = squad->mat1;
		squad->sqUnk2 = cursorPosition;
		squad->sqUnk3 = { cursorPosition, {20.0f, 10.0f, 20.0f} };
		squad->sqUnk4 = { cursorPosition, {20.0f, 10.0f, 20.0f} };

		CKChoreography* choreo = kenv.createObject<CKChoreography>(-1);
		choreo->numKeys = 1;
		CKChoreoKey* key = kenv.createObject<CKChoreoKey>(-1);
		squad->choreographies = { choreo };
		squad->choreoKeys = { key };
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(!selectedSquad);
	if (ImGui::Button("Duplicate") && selectedSquad) {
		CKGrpSquadEnemy* squad = selectedSquad.get();
		CKGrpSquadEnemy* clone;
		if (CKGrpSquadJetPack* jpsquad = squad->dyncast<CKGrpSquadJetPack>()) {
			CKGrpSquadJetPack* jpclone = kenv.createObject<CKGrpSquadJetPack>(-1);
			*jpclone = *jpsquad;
			clone = jpclone;
		}
		else {
			clone = kenv.createObject<CKGrpSquadEnemy>(-1);
			*clone = *squad;
		}

		CKGrpEnemy* grpEnemy = squad->parentGroup->cast<CKGrpEnemy>();
		grpEnemy->addGroup(clone);

		clone->bundle = kenv.createObject<CKBundle>(-1);
		clone->bundle->flags = 0x7F;
		CKServiceLife* svcLife = kenv.levelObjects.getFirst<CKServiceLife>();
		//clone->bundle->next = svcLife->firstBundle;
		//svcLife->firstBundle = clone->bundle;
		svcLife->addBundle(clone->bundle.get());

		clone->msgAction = kenv.createObject<CKMsgAction>(-1);
		*clone->msgAction = *squad->msgAction;

		for (auto& ref : clone->choreographies) {
			CKChoreography* oriChoreo = ref.get();
			ref = kenv.createObject<CKChoreography>(-1);
			*ref = *oriChoreo;
		}
		for (auto& ref : clone->choreoKeys) {
			CKChoreoKey* ori = ref.get();
			ref = kenv.createObject<CKChoreoKey>(-1);
			*ref = *ori;
		}
		for (auto& pool : clone->pools) {
			pool.cpnt = kenv.cloneObject(pool.cpnt.get());
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Delete") && selectedSquad) {
		CKGrpSquadEnemy* squad = selectedSquad.get();
		if (squad->getRefCount() > 1) {
			MsgBox(g_window, "The squad is still being referenced.", 16);
		}
		else {
			auto removeObj = [this](kanyobjref& ref) {
				CKObject* obj = ref.get();
				ref.anyreset();
				kenv.removeObject(obj);
			};
			removeObj(squad->msgAction);
			for (auto& ref : squad->choreographies)
				removeObj(ref);
			for (auto& ref : squad->choreoKeys)
				removeObj(ref);
			for (auto& pool : squad->pools)
				removeObj(pool.cpnt);
			squad->parentGroup->removeGroup(squad);
			kenv.levelObjects.getFirst<CKServiceLife>()->removeBundle(squad->bundle.get());
			removeObj(squad->bundle);
			kenv.removeObject(squad);
		}
	}
	ImGui::EndDisabled();
	auto enumSquad = [this](CKObject *osquad, int si, bool jetpack) {
		CKGrpSquadEnemy *squad = osquad->cast<CKGrpSquadEnemy>();
		int numEnemies = 0;
		for (auto &pool : squad->pools) {
			numEnemies += pool.numEnemies;
		}
		ImGui::PushID(squad);
		if (ImGui::SmallButton("View")) {
			camera.position = squad->mat1.getTranslationVector() - camera.direction * 15.0f;
		}
		ImGui::SameLine();
		if (ImGui::Selectable("##SquadItem", selectedSquad == squad)) {
			selectedSquad = squad;
		}
		IGObjectDragDropSource(*this, squad);
		ImGui::SameLine();
		ImGui::Text("%s %i (%i): %s", jetpack ? "JetPack Squad" : "Squad", si, numEnemies, kenv.getObjectName(squad));
		ImGui::PopID();
	};
	int si = 0;
	for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
		enumSquad(osquad, si++, false);
	}
	si = 0;
	for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadJetPack>().objects) {
		enumSquad(osquad, si++, true);
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	if(selectedSquad) {
		CKGrpSquadEnemy *squad = selectedSquad.get();
		if (ImGui::BeginTabBar("SquadInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::BeginChild("SquadMain");
				IGObjectNameInput("Name", squad, kenv);
				ImGuiMemberListener ml(kenv, *this);
				ml.setPropertyInfoList(g_encyclo, squad);
				if (ImGui::Button("Fix all position vectors")) {
					Vector3 pos = squad->mat1.getTranslationVector();
					squad->mat2 = squad->mat1;
					squad->sqUnk2 = pos;
					squad->sqUnk3[0] = pos;
					squad->sqUnk4[0] = pos;
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("according to mat1");
				ml.reflect(squad->bsUnk1, "bsUnk1");
				ml.reflect(*(Vector3*)&squad->mat1._41, "mat1");
				ml.reflect(*(Vector3*)&squad->mat2._41, "mat2");
				ml.reflect(squad->sqUnk1, "sqUnk1");
				ml.reflect(squad->sqUnk2, "sqUnk2");
				ml.reflectContainer(squad->sqUnk3, "sqUnk3");
				ml.reflectContainer(squad->sqUnk4, "sqUnk4");
				ml.reflect(squad->sqUnk5, "sqUnk5");
				ml.reflectContainer(squad->fings, "fings");
				ml.reflectContainer(squad->sqUnk6, "sqUnk6");
				ml.reflect(squad->sqUnk6b, "sqUnk6b");
				ml.reflect(squad->sqUnk7, "sqUnk7");
				ml.reflect(squad->sqUnk8, "sqUnk8");
				ml.reflect(squad->sqUnkB, "sqUnkB");
				ml.reflect(squad->sqUnkA, "Event 1", squad);
				ml.reflect(squad->sqUnkC, "Event 2", squad);
				ml.reflect(squad->seUnk1, "seUnk1");
				ml.reflect(squad->seUnk2, "seUnk2");
				if (kenv.isRemaster) {
					bool isChallenge = squad->sqRomasterValue != 0;
					ImGui::Checkbox("Part of Romaster Challenge", &isChallenge);
					squad->sqRomasterValue = isChallenge ? 1 : 0;
				}
				if (CKGrpSquadJetPack *jpsquad = squad->dyncast<CKGrpSquadJetPack>()) {
					ImGui::Spacing();
					ml.reflectSize<uint16_t>(jpsquad->hearths, "Num hearths");
					ml.reflectContainer(jpsquad->hearths, "hearths");
					ml.reflect(jpsquad->sjpUnk1, "sjpUnk1");
					ml.reflect(jpsquad->sjpUnk2, "sjpUnk2");
					ml.reflect(jpsquad->sjpUnk3, "sjpUnk3");
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Markers")) {
				ImGui::BeginChild("MarkersChild");
				for (auto [name, list] : { std::make_pair("Spawning points", &squad->spawnMarkers), std::make_pair("Guard points",&squad->guardMarkers) }) {
					ImGui::PushID(name);
					if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen)) {
						if (ImGui::Button("Add")) {
							list->emplace_back();
						}
						ImGui::SameLine();
						if (ImGui::Button("Clear")) {
							list->clear();
						}
						for (auto& pnt : *list) {
							ImGui::Separator();
							ImGui::PushID(&pnt);
							IGMarkerSelector(*this, "Marker Index", pnt.markerIndex);
							ImGui::InputScalar("Byte", ImGuiDataType_U8, &pnt.b);
							// TODO: Modify marker properties directly here
							ImGui::PopID();
						}
					}
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("MsgAction")) {
				static std::map<int, nlohmann::json> actionMap;
				static bool actionMapRead = false;
				if (!actionMapRead) {
					auto [smaPtr, smaSize] = GetResourceContent("SquadMsgActions.json");
					assert(smaPtr && smaSize > 0);
					nlohmann::json actionJson = nlohmann::json::parse((const char*)smaPtr, (const char*)smaPtr + smaSize);
					for (auto& elem : actionJson.at("actions")) {
						actionMap.insert_or_assign(elem.at("number").get<int>(), std::move(elem));
					}
					actionMapRead = true;
				}
				auto getActionName = [](int num) -> std::string {
					auto it = actionMap.find(num);
					if (it != actionMap.end()) {
						return it->second.at("name").get<std::string>();
					}
					else {
						return "Unknown action " + std::to_string(num);
					}
				};
				auto getParameterJson = [](int action, int param) -> const nlohmann::json* {
					auto it = actionMap.find(action);
					if (it != actionMap.end()) {
						auto& plist = it->second.at("parameters");
						if (plist.is_array() && param >= 0 && param < (int)plist.size())
							return &plist.at(param);
					}
					return nullptr;
				};

				CKMsgAction *msgAction = squad->msgAction->cast<CKMsgAction>();
				static size_t stateIndex = 0;
				if (ImGui::Button("New state"))
					msgAction->states.emplace_back();
				ImGui::SameLine();
				if (ImGui::Button("Duplicate") && stateIndex < msgAction->states.size())
					msgAction->states.push_back(msgAction->states[stateIndex]);
				ImGui::SameLine();
				if (ImGui::ArrowButton("StateDown", ImGuiDir_Down) && stateIndex + 1 < msgAction->states.size()) {
					std::swap(msgAction->states[stateIndex], msgAction->states[stateIndex + 1]);
					++stateIndex;
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move selected state down");
				ImGui::SameLine();
				if (ImGui::ArrowButton("StateUp", ImGuiDir_Up) && stateIndex >= 1 && stateIndex < msgAction->states.size()) {
					std::swap(msgAction->states[stateIndex], msgAction->states[stateIndex - 1]);
					--stateIndex;
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move selected state up");
				ImGui::SameLine();
				if (ImGui::Button("Remove") && stateIndex < msgAction->states.size())
					msgAction->states.erase(msgAction->states.begin() + stateIndex);
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
					msgAction->states.clear();
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::BeginListBox("##StateList", ImVec2(0.0f, 80.0f))) {
					int i = 0;
					for (auto& a : msgAction->states) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##StateEntry", stateIndex == (size_t)i)) {
							stateIndex = i;
						}
						ImGui::SameLine();
						ImGui::Text("%i: %s (%zi Msgs)", i, a.name.c_str(), a.messageHandlers.size());
						ImGui::PopID();
						++i;
					}
					ImGui::EndListBox();
				}
				if (stateIndex >= 0 && stateIndex < (int)msgAction->states.size()) {
					auto& a = msgAction->states[stateIndex];
					IGStringInput("Name", a.name);
					if (ImGui::Button("New message handler"))
						a.messageHandlers.emplace_back();
					ImGui::BeginChild("MsgActionWnd");
					CKMsgAction::MAMessage* removeMsg = nullptr;
					CKMsgAction::MAAction* removeAct = nullptr;
					for (auto &b : a.messageHandlers) {
						bool wannaKeepMessage = true;
						ImGui::PushID(&b);
						ImGui::SetNextItemOpen(true, ImGuiCond_Once);
						bool hopen = ImGui::CollapsingHeader("##MessageHeader", &wannaKeepMessage);
						ImGui::SameLine();
						const auto squadClassIds = kenv.factories.at(squad->getClassFullID()).hierarchy;
						ImGui::Text("Message %s", g_encyclo.getEventName(g_encyclo.getEventJson(squadClassIds, b.event), b.event).c_str());
						if (hopen) {
							ImGui::Indent();
							uint16_t msg = (uint16_t)b.event;
							if (IGEventMessageSelector(*this, "Message", msg, squad, true))
								b.event = msg;

							if (ImGui::Button("New action"))
								b.actions.emplace_back().num = 14;
							for (auto &c : b.actions) {
								//if (&c != &b.actions.front())
								ImGui::Separator();
								ImGui::PushID(&c);
								auto itActInfo = actionMap.find(c.num);
								if (ImGui::BeginCombo("Action", getActionName(c.num).c_str())) {
									for (auto& [num, info] : actionMap) {
										if (ImGui::Selectable(getActionName(num).c_str(), c.num == num)) {
											c.num = num;
											c.parameters.clear();
											auto it = actionMap.find(c.num);
											if (it != actionMap.end()) {
												for (auto& param : it->second.at("parameters")) {
													auto& typestr = param.at("type").get_ref<std::string&>();
													if (typestr == "int0") c.parameters.emplace_back(std::in_place_index<0>);
													else if (typestr == "int1") c.parameters.emplace_back(std::in_place_index<1>);
													else if (typestr == "float") c.parameters.emplace_back(std::in_place_index<2>);
													else if (typestr == "ref") c.parameters.emplace_back(std::in_place_index<3>);
													else if (typestr == "marker") c.parameters.emplace_back(std::in_place_index<4>);

													if (auto it = param.find("content"); it != param.end() && *it == "comparatorFloat") {
														std::get<1>(c.parameters.back()) = 2;
													}
												}
											}
										}
									}
									ImGui::EndCombo();
								}
								ImGui::PushItemWidth(ImGui::CalcItemWidth() - 32.0f);
								int i = 0;
								for (auto &d : c.parameters) {
									const nlohmann::json* paramJson = getParameterJson(c.num, i);
									assert(paramJson);
									const std::string& paramName = paramJson->at("name").get_ref<const std::string&>();
									const char* tbuf = paramName.c_str();
									ImGui::PushID(&d);
									ImGui::SetNextItemWidth(32.0f);
									int type = (int)d.index();
									static const std::array typeAbbr = { "Int0", "Int1", "Flt", "Ref", "Mark" };
									ImGui::LabelText("##Type", typeAbbr.at(type));
									ImGui::SameLine();
									switch (d.index()) {
									case 0: {
										uint32_t& num0 = std::get<0>(d);
										if (auto it = paramJson->find("content"); it != paramJson->end() && it->get_ref<const std::string&>() == "event") {
											uint16_t msg = (uint16_t)num0;
											if (IGEventMessageSelector(*this, tbuf, msg, GameX1::CKHkEnemy::FULL_ID))
												num0 = msg;
										}
										else {
											ImGui::InputInt(tbuf, (int*)&num0);
										}
										break;
									}
									case 1: {
										uint32_t& num1 = std::get<1>(d);
										std::span<const std::pair<int, const char*>> comboEnums;
										if (auto it = paramJson->find("content"); it != paramJson->end()) {
											const std::string& content = it->get_ref<const std::string&>();
											if (content == "comparatorInt") {
												static const std::pair<int, const char*> cmpIntEnums[] = {
													{0, "=="}, {1, "!="}, {2, "<"}, {3, "<="}, {4, ">"}, {5, ">="}
												};
												comboEnums = cmpIntEnums;
											}
											else if (content == "comparatorFloat") {
												static const std::pair<int, const char*> cmpFloatEnums[] = {
													{2, "<"}, {4, ">"}
												};
												comboEnums = cmpFloatEnums;
											}
										}
										if (!comboEnums.empty()) {
											const char* preview = "?";
											auto itEnum = std::ranges::find(comboEnums, (int)num1, [](const auto& pair) {return pair.first; });
											if (itEnum != comboEnums.end())
												preview = itEnum->second;
											if (ImGui::BeginCombo(tbuf, preview)) {
												for (const auto& [enumNum, enumName] : comboEnums) {
													if (ImGui::Selectable(enumName, num1 == enumNum))
														num1 = enumNum;
												}
												ImGui::EndCombo();
											}
										}
										else {
											ImGui::InputInt(tbuf, (int*)&num1);
										}
										break;
									}
									case 2:
										ImGui::InputFloat(tbuf, &std::get<2>(d)); break;
									case 3:
										IGObjectSelectorRef(*this, tbuf, std::get<kobjref<CKObject>>(d)); break;
									case 4:
										IGMarkerSelector(*this, tbuf, std::get<MarkerIndex>(d)); break;
									}
									ImGui::PopID();
									++i;
								}
								ImGui::PopItemWidth();
								if (ImGui::SmallButton("Remove")) {
									removeMsg = &b; removeAct = &c;
								}
								ImGui::PopID();
							}
							ImGui::Unindent();
							ImGui::Spacing();
						}
						if (!wannaKeepMessage)
							removeMsg = &b;
						ImGui::PopID();
					}
					if (removeAct)
						removeMsg->actions.erase(removeMsg->actions.begin() + (removeAct - removeMsg->actions.data()));
					else if (removeMsg)
						a.messageHandlers.erase(a.messageHandlers.begin() + (removeMsg - a.messageHandlers.data()));
						
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Choreographies")) {
				ImGui::BeginChild("SquadChoreos");
				ImGui::Text("Num choreo: %i, Num choreo keys: %i", squad->choreographies.size(), squad->choreoKeys.size());
				auto getChoreo = [squad](int key) -> int {
					int cindex = 0, kindex = 0;
					for (auto &choreo : squad->choreographies) {
						if (key >= kindex && key < kindex + choreo->numKeys) {
							return cindex;
						}
						kindex += choreo->numKeys;
						cindex++;
					}
					return -1;
				};
				auto getFirstKey = [squad](int choreoIndex) -> int {
					int cindex = 0, kindex = 0;
					for (auto& choreo : squad->choreographies) {
						if (cindex == choreoIndex) {
							return kindex;
						}
						kindex += choreo->numKeys;
						cindex++;
					}
					return -1;
				};
				static int removeChoreography = -1;
				bool pleaseOpenRemoveChoreoPopup = false;
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::BeginListBox("##ChoreoList")) {
					int cindex = 0;
					int kindex = 0;
					for (auto& choreo : squad->choreographies) {
						ImGui::PushID(choreo.get());
						ImGui::BulletText("Choreo %i: %s", cindex, kenv.getObjectName(choreo.get()));
						ImGui::SameLine();
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::GetTextLineHeightWithSpacing());
						if (ImGui::SmallButton("X")) {
							pleaseOpenRemoveChoreoPopup = true;
							removeChoreography = cindex;
						}
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Remove");
						for (int k = kindex; k < kindex + choreo->numKeys; ++k) {
							ImGui::PushID(k);
							ImGui::Indent();
							if (ImGui::Selectable("##ChoreoEntry", showingChoreoKey == k))
								showingChoreoKey = k;
							ImGui::SameLine();
							ImGui::Text("Key %i: %s", k, kenv.getObjectName(squad->choreoKeys[k].get()));
							ImGui::Unindent();
							ImGui::PopID();
						}
						ImGui::PopID();
						cindex++;
						kindex += choreo->numKeys;
					}
					ImGui::EndListBox();
				}

				if(pleaseOpenRemoveChoreoPopup)
					ImGui::OpenPopup("DeleteChoreography");
				if (ImGui::BeginPopup("DeleteChoreography")) {
					CKChoreography* kchoreo = squad->choreographies[removeChoreography].get();
					ImGui::Text("Remove choreography %i\nand its %i keys?", removeChoreography, kchoreo->numKeys);
					if (ImGui::Button("Yes")) {
						int firstindex = getFirstKey(removeChoreography);
						auto rembegin = squad->choreoKeys.begin() + firstindex;
						auto remend = rembegin + kchoreo->numKeys;
						for (auto it = rembegin; it != remend; ++it) {
							CKChoreoKey* key = it->get();
							*it = nullptr;
							kenv.removeObject(key);
						}
						squad->choreoKeys.erase(rembegin, remend);

						squad->choreographies.erase(squad->choreographies.begin() + removeChoreography);
						kenv.removeObject(kchoreo);
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				bool keyInRange = showingChoreoKey >= 0 && showingChoreoKey < (int)squad->choreoKeys.size();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Choreo key:  ");
				ImGui::SameLine();
				if (ImGui::Button("New##Choreokey") && !squad->choreographies.empty()) {
					CKChoreoKey* newkey = kenv.createObject<CKChoreoKey>(-1);
					selectedSquad->choreoKeys.push_back(newkey);
					selectedSquad->choreographies.back()->numKeys += 1;
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate##Choreokey") && keyInRange) {
					int cindex = getChoreo(showingChoreoKey);
					CKChoreoKey* original = selectedSquad->choreoKeys[showingChoreoKey].get();
					CKChoreoKey* clone = kenv.cloneObject(original);
					selectedSquad->choreoKeys.emplace(selectedSquad->choreoKeys.begin() + showingChoreoKey + 1, clone);
					selectedSquad->choreographies[cindex]->numKeys += 1;
					kenv.setObjectName(clone, std::string("Copy of ") + kenv.getObjectName(original));
				}
				ImGui::SameLine();
				if (ImGui::ArrowButton("MoveChoreoKeyDown", ImGuiDir_Down) && keyInRange) {
					int kindex = showingChoreoKey;
					int cindex = getChoreo(kindex);
					int firstkeyindex = getFirstKey(cindex);
					int lastkeyindex = firstkeyindex + squad->choreographies[cindex]->numKeys - 1;
					if (kindex == lastkeyindex) {
						if (cindex < (int)squad->choreographies.size() - 1) {
							squad->choreographies[cindex]->numKeys -= 1;
							squad->choreographies[cindex+1]->numKeys += 1;
						}
					}
					else {
						if (kindex < (int)squad->choreoKeys.size() - 1) {
							std::swap(squad->choreoKeys[kindex], squad->choreoKeys[kindex + 1]);
							showingChoreoKey += 1;
						}
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Move selected Choreokey down");
				ImGui::SameLine();
				if (ImGui::ArrowButton("MoveChoreoKeyUp", ImGuiDir_Up) && keyInRange) {
					int kindex = showingChoreoKey;
					int cindex = getChoreo(kindex);
					int firstkeyindex = getFirstKey(cindex);
					if (kindex == firstkeyindex) {
						if (cindex >= 1) {
							squad->choreographies[cindex]->numKeys -= 1;
							squad->choreographies[cindex-1]->numKeys += 1;
						}
					}
					else {
						if (kindex >= 1) {
							std::swap(squad->choreoKeys[kindex], squad->choreoKeys[kindex - 1]);
							showingChoreoKey -= 1;
						}
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Move selected Choreokey up");
				ImGui::SameLine();
				if (ImGui::Button("X") && keyInRange) {
					int cindex = getChoreo(showingChoreoKey);
					CKChoreoKey* key = squad->choreoKeys[showingChoreoKey].get();
					squad->choreoKeys.erase(squad->choreoKeys.begin() + showingChoreoKey);
					squad->choreographies[cindex]->numKeys -= 1;
					kenv.removeObject(key);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Remove selected Choreokey");

				ImGui::AlignTextToFramePadding();
				ImGui::Text("Choreography:");
				ImGui::SameLine();
				if (ImGui::Button("New##Choreography")) {
					CKChoreography* ref = kenv.createObject<CKChoreography>(-1);
					selectedSquad->choreographies.push_back(ref);
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate##ChoreoGraphy") && keyInRange) {
					int cindex = getChoreo(showingChoreoKey);
					int firstkey = getFirstKey(cindex);
					CKChoreography* ori = selectedSquad->choreographies[cindex].get();;
					CKChoreography* ref = kenv.createObject<CKChoreography>(-1);
					*ref = *ori;
					selectedSquad->choreographies.push_back(ref);
					kenv.setObjectName(ref, std::string("Copy of ") + kenv.getObjectName(ori));

					for (int currentKey = 0; currentKey < int(ori->numKeys); currentKey++) {
						CKChoreoKey* ori = selectedSquad->choreoKeys[firstkey + currentKey].get();
						CKChoreoKey* ref = kenv.createObject<CKChoreoKey>(-1);
						*ref = *ori;
						selectedSquad->choreoKeys.push_back(ref);
						kenv.setObjectName(ref, std::string("Copy of ") + kenv.getObjectName(ori));
					}
				}

				//ImGui::InputInt("ChoreoKey", &showingChoreoKey);
				int ckeyindex = showingChoreoKey;
				if (ckeyindex >= 0 && ckeyindex < (int)squad->choreoKeys.size()) {
					CKChoreoKey* ckey = squad->choreoKeys[ckeyindex].get();
					ImGui::Separator();
					IGObjectNameInput("Choreo name", squad->choreographies[getChoreo(ckeyindex)].get(), kenv);
					IGObjectNameInput("Key name", ckey, kenv);
					ImGui::DragFloatRange2("Duration min/max", &ckey->unk1, &ckey->unk2, 0.2f, 0.0f, 10000.0f);
					float percentage = ckey->unk3 * 100.0f;
					if (ImGui::DragFloat("Enemies needed at position to start timer", &percentage, 0.2f, 0.0f, 100.0f, "%.1f%%"))
						ckey->unk3 = percentage / 100.0f;
					
					unsigned int iflags = ckey->flags;
					if (const auto* jsChoreo = g_encyclo.getClassJson(CKChoreoKey::FULL_ID)) {
						try {
							const auto& bitFlags = jsChoreo->at("properties").at("flags").at("bitFlags");
							if (PropFlagsEditor(iflags, bitFlags))
								ckey->flags = (uint16_t)iflags;
						}
						catch (const nlohmann::json::exception&) {}
					}

					if (ImGui::Button("Add spot")) {
						ckey->slots.emplace_back();
					}
					ImGui::SameLine();
					if (ImGui::Button("Randomize directions")) {
						for (auto &slot : ckey->slots) {
							float angle = (rand() & 255) * 3.1416f / 128.0f;
							slot.direction = Vector3(cos(angle), 0, sin(angle));
						}
					}
					ImGui::SameLine();
					ImGui::Text("%zu spots", ckey->slots.size());

					ImGui::BeginChild("ChoreoSlots", ImVec2(0, 0), true);
					for (auto &slot : ckey->slots) {
						ImGui::PushID(&slot);
						ImGui::DragFloat3("Position", &slot.position.x, 0.1f);
						ImGui::DragFloat3("Direction", &slot.direction.x, 0.1f);
						ImGui::InputScalar("Enemy pool", ImGuiDataType_S16, &slot.enemyGroup);
						ImGui::PopID();
						ImGui::Separator();
					}
					ImGui::EndChild();
				}		
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Pools")) {
				static size_t currentPoolInput = 0;
				if (ImGui::Button("Add")) {
					ImGui::OpenPopup("AddPool");
					//CKGrpSquadEnemy::PoolEntry pe;
				}
				if (ImGui::BeginPopup("AddPool")) {
					CKGrpEnemy* grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>();
					for (CKGroup* grp = grpEnemy->childGroup.get(); grp; grp = grp->nextGroup.get()) {
						if (CKGrpPoolSquad* pool = grp->dyncast<CKGrpPoolSquad>()) {
							CKHook* enemyHook = pool->childHook.get();
							if (enemyHook) {
								ImGui::PushID(pool);
								if (ImGui::Selectable("##AddPoolEntry")) {
									// find corresponding component class
									using namespace GameX1;
									static constexpr std::pair<int, int> hookCpntMap[] = {
										{CKHkBasicEnemy::FULL_ID, CKBasicEnemyCpnt::FULL_ID},
										{CKHkBasicEnemyLeader::FULL_ID, CKBasicEnemyLeaderCpnt::FULL_ID},
										{CKHkJumpingRoman::FULL_ID, CKJumpingRomanCpnt::FULL_ID},
										{CKHkRomanArcher::FULL_ID, CKRomanArcherCpnt::FULL_ID},
										{CKHkRocketRoman::FULL_ID, CKRocketRomanCpnt::FULL_ID},
										{CKHkJetPackRoman::FULL_ID, CKJetPackRomanCpnt::FULL_ID},
										{CKHkMobileTower::FULL_ID, CKMobileTowerCpnt::FULL_ID},
										{CKHkTriangularTurtle::FULL_ID, CKTriangularTurtleCpnt::FULL_ID},
										{CKHkSquareTurtle::FULL_ID, CKSquareTurtleCpnt::FULL_ID},
										{CKHkDonutTurtle::FULL_ID, CKDonutTurtleCpnt::FULL_ID},
										{CKHkPyramidalTurtle::FULL_ID, CKPyramidalTurtleCpnt::FULL_ID}
									};
									int ehookClassFid = (int)enemyHook->getClassFullID();
									auto it = std::find_if(std::begin(hookCpntMap), std::end(hookCpntMap), [ehookClassFid](auto& p) {return p.first == ehookClassFid; });
									assert(it != std::end(hookCpntMap));
									int cpntClassFid = it->second;

									auto& pe = squad->pools.emplace_back();
									pe.pool = pool;
									pe.cpnt = kenv.createObject((uint32_t)cpntClassFid, -1)->cast<CKEnemyCpnt>();
									pe.u1 = 1;
									pe.numEnemies = 1;
									pe.u2 = 1;
								}
								ImGui::SameLine();
								ImGui::Text("%s (%s)", kenv.getObjectName(pool), enemyHook->getClassName());
								ImGui::PopID();
							}
							else {
								ImGui::PushID(pool);
								ImGui::Selectable("##AddPoolEntry");
								ImGui::SameLine();
								ImGui::Text("%s (empty)", kenv.getObjectName(pool));
								ImGui::PopID();
							}
						}
					}
					ImGui::EndPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Duplicate")) {
					if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
						CKGrpSquadEnemy::PoolEntry duppe = squad->pools[currentPoolInput];
						duppe.cpnt = kenv.cloneObject(duppe.cpnt.get());
						squad->pools.push_back(duppe);
					}
				}
				ImGui::SameLine();
				if (ImGui::ArrowButton("PoolDown", ImGuiDir_Down)) {
					if (currentPoolInput >= 0 && currentPoolInput + 1 < squad->pools.size()) {
						std::swap(squad->pools[currentPoolInput], squad->pools[currentPoolInput + 1]);
						currentPoolInput += 1;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move pool down");
				ImGui::SameLine();
				if (ImGui::ArrowButton("PoolUp", ImGuiDir_Up)) {
					if (currentPoolInput >= 1 && currentPoolInput < squad->pools.size()) {
						std::swap(squad->pools[currentPoolInput - 1], squad->pools[currentPoolInput]);
						currentPoolInput -= 1;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move pool up");
				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
						auto &cpntref = squad->pools[currentPoolInput].cpnt;
						CKEnemyCpnt *cpnt = cpntref.get();
						cpntref.reset();
						kenv.removeObject(cpnt);
						squad->pools.erase(squad->pools.begin() + currentPoolInput);
					}
				}
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::BeginListBox("##PoolList")) {
					for (int i = 0; i < (int)squad->pools.size(); i++) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
							currentPoolInput = i;
						ImGui::SameLine();
						auto &pe = squad->pools[i];
						ImGui::Text("%s %u %u", pe.cpnt->getClassName(), pe.numEnemies, pe.u1);
						ImGui::PopID();
					}
					ImGui::EndListBox();
				}
				if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
					auto &pe = squad->pools[currentPoolInput];
					ImGui::BeginChild("SquadPools");
					ImGui::BulletText("%s %u %u %u", pe.cpnt->getClassName(), pe.u1, pe.u2, pe.u3.get() ? 1 : 0);
					IGObjectSelectorRef(*this, "Pool", pe.pool);
					ImGui::InputScalar("Enemy Count", ImGuiDataType_U16, &pe.numEnemies);
					ImGui::InputScalar("U1", ImGuiDataType_U8, &pe.u1);
					ImGui::InputScalar("U2", ImGuiDataType_U8, &pe.u2);
					if (pe.cpnt->isSubclassOf<CKEnemyCpnt>()) {
						CKEnemyCpnt *cpnt = pe.cpnt->cast<CKEnemyCpnt>();
						if (ImGui::Button("Import")) {
							auto path = OpenDialogBox(g_window, "Enemy Component file\0*.XEC-ENM-CPNT\0", "xec-enm-cpnt");
							if (!path.empty()) {
								KEnvironment kfab;
								CKObject* obj = KFab::loadKFab(kfab, path);
								CKEnemyCpnt* impCpnt = obj->dyncast<CKEnemyCpnt>();
								if (!impCpnt) {
									MsgBox(g_window, "This is not an enemy component!", 16);
								}
								else if (impCpnt->getClassFullID() != cpnt->getClassFullID()) {
									std::string msg = std::string("The components types don't match!\nThe selected pool's component is:\n ") + cpnt->getClassName() + "\nbut the imported file's component is:\n " + impCpnt->getClassName();
									MsgBox(g_window, msg.c_str(), 16);
								}
								else {
									kenv.factories.at(impCpnt->getClassFullID()).copy(impCpnt, cpnt);
								}
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Export")) {
							auto path = SaveDialogBox(g_window, "Enemy Component file\0*.XEC-ENM-CPNT\0", "xec-enm-cpnt");
							if (!path.empty()) {
								int fid = (int)cpnt->getClassFullID();
								KEnvironment kfab = KFab::makeSimilarKEnv(kenv);
								CKObject* clone = kfab.createObject((int)fid, -1);
								kfab.factories.at(fid).copy(cpnt, clone);
								if (GameX1::CKRocketRomanCpnt* rockman = clone->dyncast<GameX1::CKRocketRomanCpnt>())
									rockman->rrUnk9 = nullptr;
								KFab::saveKFab(kfab, clone, path);
							}
						}
						ImGui::SameLine(0.0f, 16.0f);
						static KWeakRef<CKEnemyCpnt> cpntToCopy;
						if (ImGui::Button("Copy")) {
							cpntToCopy = cpnt;
						}
						ImGui::SameLine();
						if (ImGui::Button("Paste") && cpntToCopy) {
							if (cpntToCopy->getClassFullID() != cpnt->getClassFullID()) {
								std::string msg = std::string("The components types don't match!\nThe selected pool's component is:\n ") + cpnt->getClassName() + "\nbut the copied component is:\n " + cpntToCopy->getClassName();
								MsgBox(g_window, msg.c_str(), 16);
							}
							else {
								kenv.factories.at(cpnt->getClassFullID()).copy(cpntToCopy.get(), cpnt);
							}
						}
						IGComponentEditor(cpnt);
					}
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::Columns();
}

void EditorInterface::IGX2SquadEditor()
{
	using namespace GameX2;
	ImGui::Columns(2);
	ImGui::BeginChild("SquadList");
	auto enumSquad = [this](CKObject* osquad, int si, bool jetpack) {
		CKGrpSquadX2* squad = osquad->cast<CKGrpSquadX2>();
		int numEnemies = 0;
		for (auto& pool : squad->fightData.pools) { // only works with XXL2
			numEnemies += pool.numEnemies;
		}
		ImGui::PushID(squad);
		if (ImGui::SmallButton("View")) {
			camera.position = squad->phases[0].mat.getTranslationVector() - camera.direction * 15.0f;
		}
		ImGui::SameLine();
		if (ImGui::Selectable("##SquadItem", selectedX2Squad == squad)) {
			selectedX2Squad = squad;
			viewFightZoneInsteadOfSquad = false;
		}
		ImGui::SameLine();
		ImGui::Text("[%i] (%i) %s", si, numEnemies, kenv.getObjectName(squad));
		ImGui::PopID();
	};
	auto enumFightZone = [&](CKObject* ozone) {
		CKGrpFightZone* zone = ozone->cast<CKGrpFightZone>();
		bool open = ImGui::TreeNodeEx(zone, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick, "Zone %s", kenv.getObjectName(zone));
		if (ImGui::IsItemActivated()) {
			selectedX2FightZone = zone;
			viewFightZoneInsteadOfSquad = true;
		}
		if (open) {
			int si = 0;
			for (CKGroup* osquad = zone->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
				enumSquad(osquad, si++, false);
			}
			ImGui::TreePop();
		}
	};
	for (CKObject* ozone : kenv.levelObjects.getClassType<CKGrpFightZone>().objects) {
		enumFightZone(ozone);
	}
	ImGui::EndChild();
	ImGui::NextColumn();

	auto poolEditor = [&](X2FightData& fightData) {
		static size_t currentPoolInput = 0;
		if (ImGui::Button("Duplicate")) {
			if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
				const X2FightData::PoolEntry& duppe = fightData.pools[currentPoolInput];
				fightData.pools.push_back(duppe);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete")) {
			if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
				fightData.pools.erase(fightData.pools.begin() + currentPoolInput);
			}
		}
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginListBox("##PoolList")) {
			for (int i = 0; i < (int)fightData.pools.size(); i++) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
					currentPoolInput = i;
				ImGui::SameLine();
				auto& pe = fightData.pools[i];
				ImGui::Text("%s Cpnt%u %u", kenv.getObjectName(pe.pool.get()), pe.componentIndex, pe.numEnemies);
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
			auto& pe = fightData.pools[currentPoolInput];
			ImGui::BeginChild("SquadPools");
			//ImGui::BulletText("%s %u %u", "TODO", pe.u1, pe.u2);
			IGObjectSelectorRef(*this, "Pool", pe.pool);
			ImGui::InputScalar("Component Index", ImGuiDataType_U8, &pe.componentIndex);
			ImGui::InputScalar("Enemy Total Count", ImGuiDataType_U16, &pe.numEnemies);
			ImGui::InputScalar("Enemy Initial Count", ImGuiDataType_U8, &pe.numInitiallySpawned);
			ImGui::EndChild();
		}
		};

	if (!viewFightZoneInsteadOfSquad && selectedX2Squad) {
		CKGrpSquadX2* squad = selectedX2Squad.get();
		//if (ImGui::Button("Duplicate")) {
		//	// TODO
		//}
		if (ImGui::BeginTabBar("SquadInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::BeginChild("SquadReflection");
				ImGuiMemberListener ml(kenv, *this);
				ml.setPropertyInfoList(g_encyclo, squad);
				squad->reflectMembers2(ml, &kenv);
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Phases")) {
				ImGui::BeginChild("SquadChoreos");
				ImGui::Text("Num phases: %i", squad->phases.size());
				ImGui::InputInt("Squad Phase", &showingChoreography);
				if (showingChoreography >= 0 && showingChoreography < (int)squad->phases.size()) {
					auto& phase = squad->phases[showingChoreography];
					ImGui::Separator();
					ImGui::DragFloat3("Position", &phase.mat._41, 0.1f);
					ImGui::Separator();
					ImGui::InputInt("ChoreoKey", &showingChoreoKey);
					const int &ckeyindex = showingChoreoKey;
					if (ckeyindex >= 0 && ckeyindex < (int)phase.choreography->keys.size()) {
						CKChoreoKey* ckey = phase.choreography->keys[ckeyindex].get();
						ImGui::Separator();
						ImGui::DragFloat("Duration", &ckey->x2unk1);

						if (ImGui::Button("Add spot")) {
							ckey->slots.emplace_back();
						}
						ImGui::SameLine();
						if (ImGui::Button("Randomize orientations")) {
							for (auto& slot : ckey->slots) {
								float angle = (rand() & 255) * 3.1416f / 128.0f;
								slot.direction = Vector3(cos(angle), 0, sin(angle));
							}
						}
						ImGui::BeginChild("ChoreoSlots", ImVec2(0, 0), true);
						for (auto& slot : ckey->slots) {
							ImGui::PushID(&slot);
							ImGui::DragFloat3("Position", &slot.position.x, 0.1f);
							ImGui::DragFloat3("Direction", &slot.direction.x, 0.1f);
							ImGui::InputScalar("Enemy pool", ImGuiDataType_S16, &slot.enemyGroup);
							ImGui::PopID();
							ImGui::Separator();
						}
						ImGui::EndChild();
					}
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if ((kenv.version == kenv.KVERSION_XXL2) && ImGui::BeginTabItem("Pools")) {
				poolEditor(squad->fightData);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	else if (viewFightZoneInsteadOfSquad && selectedX2FightZone) {
		CKGrpFightZone* zone = selectedX2FightZone.get();
		if (ImGui::BeginTabBar("FightZoneInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGui::BeginChild("FightZoneReflection");
				ImGuiMemberListener ml(kenv, *this);
				ml.setPropertyInfoList(g_encyclo, zone);
				zone->reflectMembers2(ml, &kenv);
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (kenv.version >= kenv.KVERSION_ARTHUR && ImGui::BeginTabItem("Pools")) {
				poolEditor(zone->fightData);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::Columns();
}

void EditorInterface::IGEnumGroup(CKGroup *group)
{
	if (!group)
		return;
	bool gopen = ImGui::TreeNodeEx(group, (selectedGroup == group && viewGroupInsteadOfHook) ? ImGuiTreeNodeFlags_Selected : 0, "%s %s", group->getClassName(), kenv.getObjectName(group));
	IGObjectDragDropSource(*this, group);
	if (ImGui::IsItemClicked()) {
		selectedGroup = group;
		viewGroupInsteadOfHook = true;
	}
	if (gopen) {
		IGEnumGroup(group->childGroup.get());
		for (CKHook *hook = group->childHook.get(); hook; hook = hook->next.get()) {
			bool b = ImGui::TreeNodeEx(hook, ImGuiTreeNodeFlags_Leaf | ((selectedHook == hook && !viewGroupInsteadOfHook) ? ImGuiTreeNodeFlags_Selected : 0), "%s %s", hook->getClassName(), kenv.getObjectName(hook));
			IGObjectDragDropSource(*this, hook);
			if (ImGui::IsItemClicked()) {
				selectedHook = hook;
				viewGroupInsteadOfHook = false;
			}
			if(b)
				ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	IGEnumGroup(group->nextGroup.get());
}

void EditorInterface::IGHookEditor()
{
	ImGui::Columns(2);
	ImGui::BeginChild("HookTree");
	IGEnumGroup(kenv.levelObjects.getFirst<CKGroupRoot>());
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("HookInfo");
	CKHook* selectedHook = this->selectedHook.get();
	CKGroup* selectedGroup = this->selectedGroup.get();
	if (selectedHook && !viewGroupInsteadOfHook) {
		ImGui::Text("%p %s", selectedHook, selectedHook->getClassName());
		IGObjectDragDropSource(*this, selectedHook);
		ImGui::Separator();
		IGObjectNameInput("Name", selectedHook, kenv);
		ImGui::InputScalar("Hook flags", ImGuiDataType_U32, &selectedHook->unk1, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		if (selectedHook->life) {
			// NOTE: Currently modifying life sector is dangerous
			// (e.g. makes PostRef decoding fail since it relies on the
			// life sectors)
			ImGui::LabelText("Life value", "%08X", selectedHook->life->unk1);
			int lifeSector = selectedHook->life->unk1 >> 2;
			int lifeFlags = selectedHook->life->unk1 & 3;
			bool lifeChanged = false;
			lifeChanged |= ImGui::InputInt("Life sector", &lifeSector);
			lifeChanged |= ImGui::InputInt("Life flags", &lifeFlags);
			if (lifeChanged) {
				selectedHook->life->unk1 = (lifeFlags & 3) | (lifeSector << 2);
			}
		}
		if (auto* node = selectedHook->node.get()) {
			ImGui::BeginGroup();
			ImGui::AlignTextToFramePadding();
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - 2*ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
			ImGui::LabelText("##Node", "%s %s (%p)", node->getClassName(), kenv.getObjectName(node), node);
			ImGui::SameLine();
			if (ImGui::ArrowButton("GoToNode", ImGuiDir_Right)) {
				selNode = node;
				camera.position = node->getGlobalMatrix().getTranslationVector() - camera.direction * 5.0f;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Select and show me where it is");
			}
			ImGui::SameLine();
			ImGui::Text("Node");
			ImGui::EndGroup();
			IGObjectDragDropSource(*this, node);
		}
		ImGui::Separator();
		const auto* clsInfo = g_encyclo.getClassJson(selectedHook->getClassFullID());
		if (clsInfo) {
			if (auto it = clsInfo->find("hookFlags"); it != clsInfo->end()) {
				PropFlagsEditor(selectedHook->unk1, it.value());
			}
		}
		ImGuiMemberListener ml(kenv, *this);
		ml.setPropertyInfoList(g_encyclo, selectedHook);
		selectedHook->virtualReflectMembers(ml, &kenv);

		if (ImGui::Button("Duplicate (UNSTABLE!)")) {
			HookMemberDuplicator hmd{ kenv, this };
			hmd.doClone(selectedHook);
		}
		if (ImGui::Button("Export (UNSTABLE!)")) {
			auto fpath = SaveDialogBox(g_window, "Hook file (*.xechook)\0*.XECHOOK\0", "xechook");
			if (!fpath.empty()) {
				HookMemberDuplicator hmd{ kenv, this };
				hmd.doExport(selectedHook, fpath);
			}
		}

		if (ImGui::Button("Update"))
			selectedHook->update();
	}
	else if (selectedGroup && viewGroupInsteadOfHook) {
		if (ImGui::Button("Import Hook (UNSTABLE!)")) {
			auto fpath = OpenDialogBox(g_window, "Hook (*.xechook)\0*.XECHOOK\0", "xechook");
			if (!fpath.empty()) {
				HookMemberDuplicator hmd{ kenv, this };
				hmd.doImport(fpath, selectedGroup);
				protexdict.reset(kenv.levelObjects.getFirst<CTextureDictionary>());
			}
		}
		IGObjectNameInput("Name", selectedGroup, kenv);
		if (CKGroup* rgroup = selectedGroup->dyncast<CKGroup>()) {
			ImGuiMemberListener ml(kenv, *this);
			ml.setPropertyInfoList(g_encyclo, rgroup);
			rgroup->virtualReflectMembers(ml, &kenv);
		}
		if (CKGrpLight* grpLight = selectedGroup->dyncast<CKGrpLight>()) {
			CKParticleGeometry* geo = grpLight->node->dyncast<CNode>()->geometry->dyncast<CKParticleGeometry>();
			if (ImGui::Button("Add light")) {
				CKHkLight* light = kenv.createAndInitObject<CKHkLight>();
				light->lightGrpLight = selectedGroup;
				light->activeSector = -1; // TEMP
				selectedGroup->addHook(light);
				geo->pgPoints.emplace(geo->pgPoints.begin(), cursorPosition);
				geo->pgHead2 += 1; geo->pgHead3 += 1;
			}
			int index = 0;
			for (CKHook* objLight = grpLight->childHook.get(); objLight; objLight = objLight->next.get()) {
				ImGui::PushID(objLight);
				ImGui::DragFloat3("##LightPos", &geo->pgPoints[index].x, 0.1f);
				ImGui::PopID();
				++index;
			}
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGCloneEditor()
{
	CCloneManager *cloneMgr = kenv.levelObjects.getFirst<CCloneManager>();
	if (!cloneMgr) return;
	if (cloneMgr->_numClones == 0) return;

	ImGui::DragFloat3("Preview pos", &selgeoPos.x, 0.1f);
	if (ImGui::Button("Move preview to front"))
		selgeoPos = camera.position + camera.direction * 3;

	if (!selClones.empty()) {
		ImGui::SameLine();
		if (ImGui::Button("Import DFF")) {
			auto filepath = OpenDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				std::unique_ptr<RwClump> impClump = LoadDFF(filepath.c_str());
				std::vector<std::unique_ptr<RwGeometry>> geos = impClump->geoList.geometries[0]->splitByMaterial();

				int p = 0;
				for (uint32_t x : selClones) {
					if (x == 0xFFFFFFFF)
						continue;
					auto &geo = cloneMgr->_teamDict._bings[x]._clump.atomic.geometry;
					if(p < (int)geos.size())
						geo = std::move(geos[p++]);
					else
						*geo = createEmptyGeo();
				}
				selGeometry = nullptr;

				progeocache.clear();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Export DFF")) {
			auto filepath = SaveDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				RwTeam::Dong *seldong = nullptr;
				for (auto &dong : cloneMgr->_team.dongs)
					if (dong.bongs == selClones)
						seldong = &dong;

				if (!seldong) {
					MsgBox(g_window, "Sorry, I couldn't find back the team entry with the selected team dict indices :(", 16);
				}
				else {
					RwFrameList *framelist = &seldong->clump.frameList;
					RwExtHAnim *hanim = (RwExtHAnim*)framelist->extensions[1].find(0x11E);	// null if not found

					// merge clone geos
					auto sharedMergedGeo = std::make_shared<RwGeometry>();
					RwGeometry& mergedGeo = *sharedMergedGeo; bool first = true;
					for (auto td : seldong->bongs) {
						if (td == 0xFFFFFFFF)
							continue;
						const RwGeometry &tdgeo = *cloneMgr->_teamDict._bings[td]._clump.atomic.geometry.get();
						if (first) {
							mergedGeo = tdgeo;
							first = false;
						}
						else {
							mergedGeo.merge(tdgeo);
						}
					}

					RwClump clump = CreateClumpFromGeo(sharedMergedGeo, hanim);
					IOFile out(filepath.c_str(), "wb");
					clump.serialize(&out);
					out.close();
				}
			}
		}
	}

	ImGui::BeginChild("CloneList");
	for (auto &clone : cloneSet) {
		std::string lol;
		for (size_t i = 0; i < clone.size(); i++) {
			uint32_t de = clone[i];
			if (de != 0xFFFFFFFF) {
				std::string texname = "?";
				const auto &matlist = cloneMgr->_teamDict._bings[de]._clump.atomic.geometry->materialList.materials;
				if (!matlist.empty())
					texname = matlist[0].texture.name;
				if (i != 0) lol.append(", ");
				lol.append(texname);
			}
		}
		ImGui::PushID(&clone);
		if (ImGui::Selectable(lol.c_str(), selClones == clone))
			selClones = clone;
		ImGui::PopID();
	}
	ImGui::EndChild();
}

void EditorInterface::IGComponentEditor(CKEnemyCpnt *cpnt)
{
	ImGui::PushItemWidth(130.0f);

	ImGuiMemberListener igml(kenv, *this);
	igml.setPropertyInfoList(g_encyclo, cpnt);
	cpnt->virtualReflectMembers(igml, &kenv);

	ImGui::PopItemWidth();
}

void EditorInterface::IGLocaleEditor()
{
	if (!g_localeEditor)
		g_localeEditor = std::make_unique<LocaleEditor>(kenv, gfx, g_window);
	g_localeEditor->gui();
}

void EditorInterface::IGTriggerEditor()
{
	CKSrvTrigger *srvTrigger = kenv.levelObjects.getFirst<CKSrvTrigger>();
	if (!srvTrigger) return;
	ImGui::Columns(2);
	ImGui::BeginChild("TriggerTree");
	auto enumDomain = [this](CKTriggerDomain *domain, const auto &rec) -> void {
		bool open = ImGui::TreeNode(domain, "%s", kenv.getObjectName(domain));
		if (open) {
			for (const auto &subdom : domain->subdomains)
				rec(subdom.get(), rec);
			for (const auto &trigger : domain->triggers) {
				bool open = ImGui::TreeNodeEx(trigger.get(), ImGuiTreeNodeFlags_Leaf, "%s", kenv.getObjectName(trigger.get()));
				if (ImGui::IsItemClicked())
					selectedTrigger = trigger.get();
				if (open)
					ImGui::TreePop();
			}
			if (ImGui::SmallButton("Add")) {
				domain->triggers.emplace_back(kenv.createAndInitObject<CKTrigger>());
			}
			ImGui::TreePop();
		}
	};
	enumDomain(srvTrigger->rootDomain.get(), enumDomain);
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("TriggerView");
	if (selectedTrigger) {
		static const char* combinerNames[5] = {
			"All (and)",
			"Not all (nand)",
			"At least one (ori)",
			"None (nori)",
			"Only one (xor)"
		};
		static const char* comparatorNames[6] = {
			"<=  Less than or equal",
			"<   Less than",
			">=  Greater than or equal",
			">   Greater than",
			"==  Equal",
			"!=  Different"
		};
		IGObjectNameInput("Name", selectedTrigger.get(), kenv);
		auto trimPath = [](const char* name) -> std::string {
			std::string str = name;
			size_t pathIndex = str.find("(/Domaine racine");
			if (pathIndex != str.npos) str.resize(pathIndex);
			return str;
		};
		auto addButton = [this]() -> CKConditionNode* {
			if (ImGui::Button("Add")) {
				ImGui::OpenPopup("AddCondNodeMenu");
			}
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EventNodeX2")) {
					auto& [eventNode, name] = *(const EventNodePayload*)payload->Data;
					CKComparator* cmp = kenv.createAndInitObject<CKComparator>();
					cmp->condNodeType = 8; // equal
					kenv.setObjectName(cmp, comparatorNames[4]);

					cmp->leftCmpData = kenv.createAndInitObject<CKComparedData>();
					cmp->leftCmpData->cmpdatType = 2 << 2;
					auto& cden = cmp->leftCmpData->cmpdatValue.emplace<CKComparedData::CmpDataEventNode>();
					cden.cmpdatT2Trigger = selectedTrigger.get();
					kenv.setObjectName(cmp->leftCmpData.get(), name);

					cmp->rightCmpData = kenv.createAndInitObject<CKComparedData>();
					cmp->rightCmpData->cmpdatType = 1 << 2;
					auto& cdc = cmp->rightCmpData->cmpdatValue.emplace<CKComparedData::CmpDataConstant>();
					cdc.value.emplace<uint8_t>(1);
					kenv.setObjectName(cmp->rightCmpData.get(), "True");

					eventNode->datas.emplace_back(cmp->leftCmpData.get());

					ImGui::EndDragDropTarget();
					return cmp;
				}
				ImGui::EndDragDropTarget();
			}
			if (ImGui::BeginPopup("AddCondNodeMenu")) {
				int addComparator = -1, addCombiner = -1;
				for (int i = 0; i < (int)std::size(comparatorNames); ++i) {
					if (ImGui::Selectable(comparatorNames[i])) {
						addComparator = i;
					}
				}
				ImGui::Separator();
				for (int i = 0; i < (int)std::size(combinerNames); ++i) {
					if (ImGui::Selectable(combinerNames[i])) {
						addCombiner = i;
					}
				}
				ImGui::EndPopup();
				if (addComparator != -1) {
					CKComparator* cmp = kenv.createAndInitObject<CKComparator>();
					cmp->condNodeType = addComparator << 1;
					kenv.setObjectName(cmp, comparatorNames[addComparator]);
					return cmp;
				}
				else if (addCombiner != -1) {
					CKCombiner* comb = kenv.createAndInitObject<CKCombiner>();
					comb->condNodeType = addCombiner << 1;
					kenv.setObjectName(comb, combinerNames[addCombiner]);
					return comb;
				}
			}
			return nullptr;
		};
		auto walkCondNode = [this, trimPath, addButton](CKConditionNode* node, auto rec) -> void {
			const char* name = "?";
			if (CKCombiner* c = node->dyncast<CKCombiner>())
				name = combinerNames[c->condNodeType >> 1];
			else if (CKComparator* c = node->dyncast<CKComparator>())
				name = comparatorNames[c->condNodeType >> 1];
			if (ImGui::TreeNodeEx(node, ImGuiTreeNodeFlags_DefaultOpen, "%s", name)) { //trimPath(kenv.getObjectName(node)).c_str()
				if (CKCombiner* comb = node->dyncast<CKCombiner>()) {
					for (auto& child : comb->condNodeChildren)
						rec(child.get(), rec);
					if (CKConditionNode* added = addButton())
						comb->condNodeChildren.emplace_back(added);
				}
				else if (CKComparator* cmp = node->dyncast<CKComparator>()) {
					//ImGuiMemberListener ml{ kenv, *this };
					ImGui::Bullet(); ImGui::TextUnformatted(trimPath(kenv.getObjectName(cmp->leftCmpData.get())).c_str());
					//cmp->leftCmpData->virtualReflectMembers(ml, &kenv);
					ImGui::Bullet(); ImGui::TextUnformatted(trimPath(kenv.getObjectName(cmp->rightCmpData.get())).c_str());
					//cmp->rightCmpData->virtualReflectMembers(ml, &kenv);
				}
				ImGui::TreePop();
			}
		};
		auto removeCondNode = [this](CKConditionNode* node, const auto& rec) -> void {
			if (CKCombiner* comb = node->dyncast<CKCombiner>()) {
				for (auto& ref : comb->condNodeChildren) {
					CKConditionNode* sub = ref.get();
					ref = nullptr;
					rec(sub, rec);
				}
			}
			else if (CKComparator* cmp = node->dyncast<CKComparator>()) {
				CKComparedData* left = cmp->leftCmpData.get();
				CKComparedData* right = cmp->rightCmpData.get();
				cmp->leftCmpData = nullptr;
				cmp->rightCmpData = nullptr;
				kenv.removeObject(left);
				kenv.removeObject(right);
			}
			kenv.removeObject(node);
		};
		if (ImGui::Button("Clear condition")) {
			if (CKConditionNode* cnd = selectedTrigger->condition.get()) {
				selectedTrigger->condition = nullptr;
				removeCondNode(cnd, removeCondNode);
			}
		}
		if (selectedTrigger->condition) {
			walkCondNode(selectedTrigger->condition.get(), walkCondNode);
		}
		else {
			if (CKConditionNode* added = addButton()) {
				selectedTrigger->condition = added;
			}
		}
		int acttodelete = -1;
		for (size_t actindex = 0; actindex < selectedTrigger->actions.size(); actindex++) {
			auto& act = selectedTrigger->actions[actindex];
			ImGui::Separator();
			ImGui::PushID(&act);
			ImGui::SetNextItemWidth(48.0f);
			ImGui::InputScalar("##EventID", ImGuiDataType_U16, &act.event, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text("->");
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::SetNextItemWidth(-1.0f);
			IGObjectSelectorRef(*this, "##EventObj", act.target);
			ImGui::TextUnformatted("Value:");
			ImGui::SameLine();
			int valType = (int)act.value.index();
			static const char* typeNames[4] = { "int8", "int32", "float", "kobjref" };
			ImGui::SetNextItemWidth(76.0f);
			if (ImGui::Combo("##ValueType", &valType, typeNames, 4))
				changeVariantType(act.value, (size_t)valType);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-32.0f);
			switch (act.value.index()) {
			case 0: ImGui::InputScalar("##Value", ImGuiDataType_U8, &std::get<uint8_t>(act.value)); break;
			case 1: ImGui::InputScalar("##Value", ImGuiDataType_U32, &std::get<uint32_t>(act.value)); break;
			case 2: ImGui::InputScalar("##Value", ImGuiDataType_Float, &std::get<float>(act.value)); break;
			case 3: IGObjectSelectorRef(*this, "##Value", std::get<KPostponedRef<CKObject>>(act.value)); break;
			}
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				acttodelete = actindex;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Delete");
			}
			ImGui::PopID();
		}
		if(acttodelete != -1)
			selectedTrigger->actions.erase(selectedTrigger->actions.begin() + acttodelete);
		if (ImGui::Button("New action")) {
			selectedTrigger->actions.emplace_back();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::checkNodeRayCollision(CKSceneNode * node, const Vector3 &rayDir, const Matrix &matrix)
{
	for(; node; node = node->next.get()) {
		Matrix nodeTransform = node->transform;
		nodeTransform.m[0][3] = nodeTransform.m[1][3] = nodeTransform.m[2][3] = 0.0f;
		nodeTransform.m[3][3] = 1.0f;
		Matrix globalTransform = nodeTransform * matrix;

		auto checkGeo = [this,node,&rayDir,&globalTransform](RwGeometry *rwgeo) {
			Vector3 sphereSize = globalTransform.getScalingVector() * rwgeo->sphereRadius;
			if (rayIntersectsSphere(camera.position, rayDir, rwgeo->spherePos.transform(globalTransform), std::max({ sphereSize.x, sphereSize.y, sphereSize.z }))) {
				for (auto &tri : rwgeo->tris) {
					std::array<Vector3, 3> trverts;
					for (int i = 0; i < 3; i++)
						trverts[i] = rwgeo->verts[tri.indices[i]].transform(globalTransform);
					auto ixres = getRayTriangleIntersection(camera.position, rayDir, trverts[0], trverts[1], trverts[2]);
					if (ixres.first) {
						CKSceneNode *tosel = node;
						if (!tosel->isSubclassOf<CSGSectorRoot>()) {
							while (!tosel->parent->isSubclassOf<CSGSectorRoot>() && !tosel->parent->isSubclassOf<CZoneNode>())
								tosel = tosel->parent.get();
							rayHits.push_back(std::make_unique<NodeSelection>(*this, ixres.second, tosel));
						}
						else { // non-editable selection for sector root to still let the user to set cursor position there
							rayHits.push_back(std::make_unique<UISelection>(*this, ixres.second));
						}
					}
				}
			}
		};

		if (showInvisibleNodes || !IsNodeInvisible(node, kenv.version >= 2)) {
			if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
				if (showClones) {
					CCloneManager *clm = kenv.levelObjects.getFirst<CCloneManager>();
					//auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const kobjref<CSGBranch> &ref) {return ref.get() == node; });
					//assert(it != clm->_clones.end());
					//size_t clindex = it - clm->_clones.begin();
					int clindex = nodeCloneIndexMap.at((CSGBranch*)node);
					for (uint32_t part : clm->_team.dongs[clindex].bongs)
						if (part != 0xFFFFFFFF) {
							RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump.atomic.geometry.get();
							checkGeo(rwgeo);
						}
				}
			}
			else if (node->isSubclassOf<CNode>() /*&& !node->isSubclassOf<CSGSectorRoot>()*/) {
				for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
					CKAnyGeometry *rgeo = kgeo->duplicateGeo ? kgeo->duplicateGeo.get() : kgeo;
					if (auto& clump = rgeo->clump)
						if (RwGeometry *rwgeo = clump->atomic.geometry.get())
							checkGeo(rwgeo);
				}
			}
			if (node->isSubclassOf<CSGBranch>()) {
				checkNodeRayCollision(node->cast<CSGBranch>()->child.get(), rayDir, globalTransform);
			}
			if (CAnyAnimatedNode *anyanimnode = node->dyncast<CAnyAnimatedNode>()) {
				checkNodeRayCollision(anyanimnode->branchs.get(), rayDir, globalTransform);
			}
		}
	}
}

void EditorInterface::checkMouseRay()
{
	Vector3 rayDir = getRay(camera, g_window);
	numRayHits = 0;
	rayHits.clear();
	nearestRayHit = nullptr;

	auto checkOnSector = [this,&rayDir](KObjectList &objlist) {
		// Nodes
		if(showNodes && kenv.hasClass<CSGSectorRoot>() && kenv.hasClass<CKGeometry>())
			checkNodeRayCollision(objlist.getFirst<CSGSectorRoot>(), rayDir, Matrix::getIdentity());

		// Beacons
		if (showBeacons && kenv.hasClass<CKBeaconKluster>()) {
			for (CKBeaconKluster *kluster = objlist.getFirst<CKBeaconKluster>(); kluster; kluster = kluster->nextKluster.get()) {
				int nbing = 0;
				for (auto &bing : kluster->bings) {
					if (bing.active) {
						int nbeac = 0;
						for (auto &beacon : bing.beacons) {
							Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
							pos.y += 0.5f;
							std::pair<bool, Vector3> rsi;
							if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
								Vector3 lc = pos - Vector3(0.5f, 0.5f, 0.5f);
								Vector3 hc = pos + Vector3(0.5f, (float)(beacon.params & 7) - 0.5f, 0.5f);
								rsi = getRayAABBIntersection(camera.position, rayDir, hc, lc);
							}
							else {
								rsi = getRaySphereIntersection(camera.position, rayDir, pos, 0.5f);
							}
							if (rsi.first) {
								rayHits.push_back(std::make_unique<BeaconSelection>(*this, rsi.second, bing.sectorIndex, bing.klusterIndex, nbing, nbeac));
							}
							nbeac++;
						}
					}
					nbing++;
				}
			}
		}

		// Grounds
		if ((showGroundBounds || showGrounds) && kenv.hasClass<CKMeshKluster>()) {
			if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
				for (auto &ground : mkluster->grounds) {
					auto rbi = getRayAABBIntersection(camera.position, rayDir, ground->aabb.highCorner, ground->aabb.lowCorner);
					if (rbi.first) {
						for (auto &tri : ground->triangles) {
							Vector3 &v0 = ground->vertices[tri.indices[0]];
							Vector3 &v1 = ground->vertices[tri.indices[1]];
							Vector3 &v2 = ground->vertices[tri.indices[2]];
							auto rti = getRayTriangleIntersection(camera.position, rayDir, v0, v2, v1);
							if(rti.first)
								rayHits.push_back(std::make_unique<GroundSelection>(*this, rti.second, ground.get()));
						}
					}
				}
			}
		}
	};

	checkOnSector(kenv.levelObjects);
	int showingStream = showingSector - 1;
	if (showingStream < 0)
		for (auto &str : kenv.sectorObjects)
			checkOnSector(str);
	else if (showingStream < (int)kenv.numSectors)
		checkOnSector(kenv.sectorObjects[showingStream]);

	// Squads
	if (showSquadChoreos && kenv.hasClass<CKGrpEnemy>()) {
		if (CKGrpEnemy *grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>()) {
			for (CKGroup *grp = grpEnemy->childGroup.get(); grp; grp = grp->nextGroup.get()) {
				if (CKGrpSquadEnemy *squad = grp->dyncast<CKGrpSquadEnemy>()) {
					Vector3 sqpos = squad->mat1.getTranslationVector();
					RwGeometry *rwgeo = swordModel->geoList.geometries[0].get();
					if (rayIntersectsSphere(camera.position, rayDir, rwgeo->spherePos.transform(squad->mat1), rwgeo->sphereRadius)) {
						for (auto &tri : rwgeo->tris) {
							std::array<Vector3, 3> trverts;
							for (int i = 0; i < 3; i++)
								trverts[i] = rwgeo->verts[tri.indices[i]].transform(squad->mat1);
							auto ixres = getRayTriangleIntersection(camera.position, rayDir, trverts[0], trverts[1], trverts[2]);
							if (ixres.first) {
								rayHits.push_back(std::make_unique<SquadSelection>(*this, ixres.second, squad));
								break;
							}
						}
					}
					if (showingChoreoKey >= 0 && showingChoreoKey < (int)squad->choreoKeys.size()) {
						int spotIndex = 0;
						for (auto &slot : squad->choreoKeys[showingChoreoKey]->slots) {
							Vector3 trpos = slot.position.transform(squad->mat1);
							auto rbi = getRayAABBIntersection(camera.position, rayDir, trpos + Vector3(1, 2, 1), trpos - Vector3(1, 0, 1));
							if (rbi.first) {
								rayHits.push_back(std::make_unique<ChoreoSpotSelection>(*this, rbi.second, squad, spotIndex));
							}
							spotIndex++;
						}
					}
				}
			}
		}
	}

	// Markers
	if (showMarkers && kenv.hasClass<CKSrvMarker>()) {
		if (CKSrvMarker *srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>()) {
			for (auto &list : srvMarker->lists) {
				int markerIndex = 0;
				for (auto &marker : list) {
					auto rsi = getRaySphereIntersection(camera.position, rayDir, marker.position, 0.5f);
					if (rsi.first) {
						rayHits.push_back(std::make_unique<MarkerSelection>(*this, rsi.second, markerIndex));
					}
					markerIndex += 1;
				}
			}
		}
	}

	// XXL1 Light hooks
	if (showLights && kenv.hasClass<CKGrpLight>()) {
		if (CKGrpLight* grpLight = kenv.levelObjects.getFirst<CKGrpLight>()) {
			auto& points = grpLight->node->cast<CNode>()->geometry->cast<CKParticleGeometry>()->pgPoints;
			int lightIndex = 0;
			for (const Vector3& pnt : points) {
				auto rsi = getRaySphereIntersection(camera.position, rayDir, pnt, 0.5f);
				if (rsi.first) {
					rayHits.push_back(std::make_unique<HkLightSelection>(*this, rsi.second, grpLight, lightIndex));
				}
				lightIndex += 1;
			}
		}
	}

	// XXL1 Detectors
	if (showDetectors && kenv.version == kenv.KVERSION_XXL1 && kenv.hasClass<CKSrvDetector>()) {
		if (CKSrvDetector* srvDetector = kenv.levelObjects.getFirst<CKSrvDetector>()) {
			size_t i = 0;
			for (auto& aabb : srvDetector->aaBoundingBoxes) {
				Vector3 center = (aabb.highCorner + aabb.lowCorner) * 0.5f;
				auto rsi = getRaySphereIntersection(camera.position, rayDir, center, 0.5f);
				if (rsi.first) {
					rayHits.push_back(std::make_unique<X1DetectorSelection>(*this, rsi.second, X1DetectorSelection::BOUNDINGBOX, i));
				}
				++i;
			}
			i = 0;
			for (auto& sph : srvDetector->spheres) {
				auto rsi = getRaySphereIntersection(camera.position, rayDir, sph.center, 0.5f);
				if (rsi.first) {
					rayHits.push_back(std::make_unique<X1DetectorSelection>(*this, rsi.second, X1DetectorSelection::SPHERE, i));
				}
				++i;
			}
			i = 0;
			for (auto& rect : srvDetector->rectangles) {
				auto rsi = getRaySphereIntersection(camera.position, rayDir, rect.center, 0.5f);
				if (rsi.first) {
					rayHits.push_back(std::make_unique<X1DetectorSelection>(*this, rsi.second, X1DetectorSelection::RECTANGLE, i));
				}
				++i;
			}
		}
	}

	// XXL2 Detectors
	if (showDetectors && kenv.hasClass<CKSectorDetector>()) {
		int strid = -2;
		int showingStream = showingSector - 1;
		for (CKObject* osector : kenv.levelObjects.getClassType<CKSectorDetector>().objects) {
			++strid;
			if (!(showingStream < 0 || strid == -1 || strid == showingStream))
				continue;
			CKSectorDetector* sector = osector->cast<CKSectorDetector>();
			for (auto& detector : sector->sdDetectors) {
				CMultiGeometry* geo = detector->dbGeometry.get();
				Vector3 center;
				if (auto* aabb = std::get_if<AABoundingBox>(&geo->mgShape)) {
					center = (aabb->highCorner + aabb->lowCorner) * 0.5f;
				}
				else if (auto* sph = std::get_if<BoundingSphere>(&geo->mgShape)) {
					center = sph->center;
				}
				else if (auto* rect = std::get_if<AARectangle>(&geo->mgShape)) {
					center = rect->center;
				}
				auto rsi = getRaySphereIntersection(camera.position, rayDir, center, 0.5f);
				if (rsi.first) {
					rayHits.push_back(std::make_unique<X2DetectorSelection>(*this, rsi.second, geo, detector.get()));
				}
			}
		}
	}

	if (!rayHits.empty()) {
		auto comp = [this](const std::unique_ptr<UISelection> &a, const std::unique_ptr<UISelection> &b) -> bool {
			return (camera.position - a->hitPosition).len3() < (camera.position - b->hitPosition).len3();
		};
		nearestRayHit = std::min_element(rayHits.begin(), rayHits.end(), comp)->get();
		cursorPosition = nearestRayHit->hitPosition;
	}
}

}
