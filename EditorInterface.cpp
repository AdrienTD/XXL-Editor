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
#include "CKDictionary.h"
#include "CKNode.h"
#include "CKGraphical.h"
#include "CKLogic.h"
#include "CKComponent.h"
#include "CKGroup.h"
#include "CKHook.h"
#include <shlobj_core.h>
#include <stb_image_write.h>
#include "rwext.h"
#include <stack>
#include "imgui/ImGuizmo.h"
#include "GameLauncher.h"
#include "Shape.h"
#include "CKService.h"
#include <INIReader.h>
#include "rw.h"
#include "WavDocument.h"
#include "CKCinematicNode.h"
#include "KLocalObject.h"
#include "CKLocalObjectSubs.h"
#include <io.h>
#include "GuiUtils.h"
#include "LocaleEditor.h"
#include "Duplicator.h"
#include <nlohmann/json.hpp>
#include <charconv>
#include <fmt/format.h>
#include <shellapi.h>
#include "imgui/imnodes.h"
#include "CKManager.h"
#include "GameClasses/CKGameX1.h"
#include "GameClasses/CKGameX2.h"
#include "GameClasses/CKGameOG.h"
#include "Encyclopedia.h"

using namespace GuiUtils;

namespace {
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

	void UpdateBeaconKlusterBounds(CKBeaconKluster *kluster) {
		BoundingSphere bounds = kluster->bounds;
		bool first = true;
		for (auto &bing : kluster->bings) {
			for (auto &beacon : bing.beacons) {
				BoundingSphere beaconSphere;
				if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
					Vector3 lc = beacon.getPosition() + Vector3(-0.5f, 0.0f, -0.5f);
					Vector3 hc = lc + Vector3(1.0f, (float)(beacon.params & 7), 1.0f);
					beaconSphere = BoundingSphere((lc + hc) * 0.5f, (hc - lc).len3() * 0.5f);
				}
				else
					beaconSphere = BoundingSphere(beacon.getPosition(), 1.0f);
				if (first) {
					bounds = beaconSphere;
					first = false;
				}
				else
					bounds.merge(beaconSphere);
			}
		}
		kluster->bounds = bounds;
	}

	void GimmeTheRocketRomans(KEnvironment &kenv) {
		using namespace GameX1;
		std::map<CKHkBasicEnemy*, CKHkRocketRoman*> hkmap;
		for (CKObject *obj : kenv.levelObjects.getClassType<CKHkBasicEnemy>().objects) {
			CKHkBasicEnemy *hbe = obj->cast<CKHkBasicEnemy>();
			CKHkRocketRoman *hrr = kenv.createObject<CKHkRocketRoman>(-1);
			hkmap[hbe] = hrr;
			for (auto &ref : hbe->boundingShapes)
				ref->object = hrr;
			hbe->beBoundNode->object = hrr;
			hbe->life->hook = hrr;

			// copy
			*static_cast<CKHkBasicEnemy*>(hrr) = *hbe;

			// Rocket-specific values
			CKAACylinder *rrsphere = kenv.createObject<CKAACylinder>(-1);
			rrsphere->transform = kenv.levelObjects.getFirst<CSGSectorRoot>()->transform;
			rrsphere->radius = 2.0f;
			rrsphere->cylinderHeight = 2.0f;
			rrsphere->cylinderRadius = 2.0f;
			assert(hrr->romanAnimatedClone == hrr->romanAnimatedClone2 && hrr->romanAnimatedClone3 == hrr->node && hrr->node == hrr->romanAnimatedClone);
			hrr->romanAnimatedClone2->insertChild(rrsphere);
			//hrr->rrCylinderNode = rrsphere;

			hrr->rrCylinderNode = hrr->boundingShapes[3]->cast<CKAACylinder>();
			hrr->boundingShapes[3] = rrsphere;

			CKSoundDictionaryID *sdid = kenv.createObject<CKSoundDictionaryID>(-1);
			sdid->soundEntries.resize(32); // add 32 default (empty) sounds
			int sndid = 0;
			for (auto& se : sdid->soundEntries) {
				se.active = true;
				se.id = sndid++;
				se.flags = 16;
				se.obj = hrr->node.get();
			}
			hrr->rrSoundDictID = sdid;

			hrr->rrParticleNode = kenv.levelObjects.getFirst<CKCrateCpnt>()->particleNode.get();

			CAnimationDictionary* animDict = kenv.createAndInitObject<CAnimationDictionary>();
			hrr->rrAnimDict = animDict;
			animDict->numAnims = 4;
			animDict->animIndices.resize(animDict->numAnims);
			for (int i = 0; i < animDict->numAnims; ++i)
				animDict->animIndices[i] = hrr->beAnimDict->animIndices[i];
		}
		for (CKObject *obj : kenv.levelObjects.getClassType<CKHkBasicEnemy>().objects) {
			CKHkBasicEnemy *hbe = obj->cast<CKHkBasicEnemy>();
			CKHkRocketRoman *hrr = hkmap[hbe];
			if(hbe->next.get())
				hrr->next = hkmap[(CKHkBasicEnemy*)hbe->next.get()];
			hbe->next.reset();
		}
		CKSrvCollision *col = kenv.levelObjects.getFirst<CKSrvCollision>();
		for (auto &ref : col->objs2)
			if (ref->getClassFullID() == CKHkBasicEnemy::FULL_ID)
				ref = hkmap[ref->cast<CKHkBasicEnemy>()];
		for (CKObject *obj : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
			CKGrpSquadEnemy *gse = obj->cast<CKGrpSquadEnemy>();
			for (auto &pe : gse->pools) {
				if (pe.cpnt->getClassFullID() == CKBasicEnemyCpnt::FULL_ID) {
					CKBasicEnemyCpnt *becpnt = pe.cpnt->cast<CKBasicEnemyCpnt>();
					CKRocketRomanCpnt *rrcpnt = kenv.createObject<CKRocketRomanCpnt>(-1);
					*(CKBasicEnemyCpnt*)rrcpnt = *becpnt;
					//....
					rrcpnt->rrCylinderRadius = 1.0f;
					rrcpnt->rrCylinderHeight = 1.0f;
					rrcpnt->rrUnk3 = Vector3(1.0f, 1.0f, 1.0f);
					rrcpnt->rrUnk4 = 0;
					rrcpnt->rrFireDistance = 3.0f;
					rrcpnt->rrUnk6 = 0;
					rrcpnt->rrFlySpeed = 5.0f;
					rrcpnt->rrRomanAimFactor = 10.0f;
					rrcpnt->rrUnk9 = kenv.levelObjects.getClassType(2, 28).objects[0]; // Asterix Hook
					//
					pe.cpnt = rrcpnt;
					kenv.removeObject(becpnt);
				}
			}
		}
		for (CKObject *obj : kenv.levelObjects.getClassType<CKGrpPoolSquad>().objects) {
			CKGrpPoolSquad *pool = obj->cast<CKGrpPoolSquad>();
			if (pool->childHook.get())
				if(pool->childHook->getClassFullID() == CKHkBasicEnemy::FULL_ID)
					pool->childHook = hkmap[pool->childHook->cast<CKHkBasicEnemy>()];
		}
		for (auto &ent : hkmap) {
			if (ent.first)
				kenv.removeObject(ent.first);
		}
		//col->objs.clear();
		//col->objs2.clear();
		//col->bings.clear();
		//col->unk1 = 0;
		//col->unk2 = 0;
		kenv.levelObjects.getClassType<CKHkRocketRoman>().info = kenv.levelObjects.getClassType<CKHkBasicEnemy>().info;
		kenv.levelObjects.getClassType<CKHkBasicEnemy>().info = 0;
		kenv.levelObjects.getClassType<CKRocketRomanCpnt>().info = kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().info;
		kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().info = 0;
	}

	bool audioInitDone = false;
	SDL_AudioDeviceID audiodevid;
	int audioLastFreq = 0;

	void InitSnd(int freq, bool byteSwapped) {
		if (audioInitDone && audioLastFreq == freq) {
			SDL_ClearQueuedAudio(audiodevid);
			return;
		}
		if (audioInitDone) {
			SDL_ClearQueuedAudio(audiodevid);
			SDL_CloseAudioDevice(audiodevid);
		}
		SDL_AudioSpec spec, have;
		memset(&spec, 0, sizeof(spec));
		spec.freq = freq;
		spec.format = byteSwapped ? AUDIO_S16MSB : AUDIO_S16LSB;
		spec.channels = 1;
		spec.samples = 4096;
		audiodevid = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);
		assert(audiodevid);
		SDL_PauseAudioDevice(audiodevid, 0);
		audioInitDone = true;
		audioLastFreq = freq;
	}

	void PlaySnd(KEnvironment &kenv, RwSound &snd) {
		InitSnd(snd.info.dings[0].sampleRate, kenv.platform == KEnvironment::PLATFORM_X360 || kenv.platform == KEnvironment::PLATFORM_PS3);
		SDL_QueueAudio(audiodevid, snd.data.data.data(), snd.data.data.size());
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

	ImVec4 getPFCellColor(uint8_t val) {
		ImVec4 color(1, 0, 1, 1);
		switch (val) {
		case 0: color = ImVec4(0, 1, 1, 1); break; // enemy
		case 1: color = ImVec4(1, 1, 1, 1); break; // partner + enemy
		case 4: color = ImVec4(1, 1, 0, 1); break; // partner
		case 7: color = ImVec4(1, 0, 0, 1); break; // wall
		}
		return color;
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

	void IGObjectNameInput(const char* label, CKObject* obj, KEnvironment& kenv) {
		std::string* pstr = nullptr;
		auto it = kenv.globalObjNames.dict.find(obj);
		if (it != kenv.globalObjNames.dict.end())
			pstr = &it->second.name;
		else {
			it = kenv.levelObjNames.dict.find(obj);
			if (it != kenv.levelObjNames.dict.end())
				pstr = &it->second.name;
			else {
				for (auto& str : kenv.sectorObjNames) {
					it = str.dict.find(obj);
					if (it != str.dict.end())
						pstr = &it->second.name;
				}
			}
		}
		if (pstr) {
			ImGui::InputText(label, pstr->data(), pstr->capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, pstr);
		}
		else {
			char test[2] = { 0,0 };
			void* user[2] = { obj, &kenv };
			ImGui::InputText(label, test, 1, ImGuiInputTextFlags_CallbackResize,
				[](ImGuiInputTextCallbackData* data) -> int {
					if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
						void** user = (void**)data->UserData;
						CKObject* obj = (CKObject*)user[0];
						KEnvironment* kenv = (KEnvironment*)user[1];
						auto& info = kenv->makeObjInfo(obj);
						info.name.resize(data->BufTextLen);
						data->Buf = info.name.data();
					}
					return 0;
				}, user);
		}
	}

	void IGStringInput(const char* label, std::string& str) {
		ImGui::InputText(label, str.data(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
	}

	void IGLink(const char* text, const wchar_t* url, Window* window = nullptr) {
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 box = ImGui::CalcTextSize(text);
		uint32_t color = 0xFFFA870F;
		if (ImGui::InvisibleButton(text, box)) {
			ShellExecuteW(window ? (HWND)window->getNativeWindow() : nullptr, NULL, url, NULL, NULL, SW_SHOWNORMAL);
		}
		if (ImGui::IsItemHovered()) {
			color = 0xFFFFC74F;
			ImGui::SetTooltip("%S", url);
		}

		float ly = pos.y + box.y;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddText(pos, color, text);
		drawList->AddLine(ImVec2(pos.x, ly), ImVec2(pos.x + box.x, ly), color);
	};

	static const char* beaconX1Names[] = {
		// 0x00
		"*", "*", "*", "Wooden Crate", "Metal Crate", "Simple", "Helmet", "Golden Helmet",
		// 0x08
		"Potion", "Shield", "Ham", "x3 Multiplier",	"x10 Multiplier", "Laurel", "Boar", "Water flow",
		// 0x10
		"Merchant", "*", "*", "*", "*", "Save point", "Respawn point", "Hero respawn pos",
		// 0x18
		"?", "?", "X2 Potion", "X2 Helmet", "X2 x3 Multiplier", "X2 x10 Multiplier", "X2 Ham", "X2 Shield",
		// 0x20
		"X2 Golden Helmet", "X2 Diamond Helmet", "?", "?", "?", "?", "X2 Enemy spawn", "X2 Marker",
		// 0x28
		"?", "?", "X2 Food Basket", "?", "?", "Ar Egg", "Ar Burried Surprise", "?",
		// 0x30
		"?", "?", "?", "?", "?", "?", "?", "?",
		// 0x38
		"?", "?", "?", "?", "?", "Ar Eggbag", "?", "?",
		// 0x40
		"OG Helmet", "OG Golden Helmet", "OG Glue", "OG Powder", "OG x3 Multiplier", "OG x10 Multiplier", "OG Ham", "OG Shield",
		// 0x48
		"OG Potion", "OG Bird Cage", "?", "?", "?", "?", "?", "?",
	};
	static const char* beaconX1RomasterNames[] = {
		// 0x00
		"*", "*", "*", "Wooden Crate", "Metal Crate", "?", "Helmet", "Golden Helmet",
		// 0x08
		"Potion", "Shield", "Ham", "x3 Multiplier",	"x10 Multiplier", "Laurel", "Boar", "Water flow",
		// 0x10
		"Merchant", "Retro Coin", "Remaster Coin", "*", "*", "Save point", "Respawn point", "Hero respawn pos",
		// 0x18
		"?", "?", "Freeze Crate 1", "Freeze Crate 3", "Freeze Crate 5",
	};
	static auto getBeaconName = [](KEnvironment& kenv, int handlerId) -> const char* {
		if (kenv.version <= kenv.KVERSION_XXL1 && kenv.isRemaster && handlerId < std::extent<decltype(beaconX1RomasterNames)>::value)
			return beaconX1RomasterNames[handlerId];
		if (handlerId < std::extent<decltype(beaconX1Names)>::value)
			return beaconX1Names[handlerId];
		return "!";
	};

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

	void InitImNodes() {
		static bool ImNodesInitialized = false;
		if (!ImNodesInitialized) {
			ImNodes::CreateContext();
			ImNodes::GetIO().AltMouseButton = ImGuiMouseButton_Right;
			ImNodes::GetIO().EmulateThreeButtonMouse.Modifier = &ImGui::GetIO().KeyAlt;
			ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
			ImNodesInitialized = true;
		}
	}

	bool IGU32Color(const char* name, uint32_t& color) {
		ImVec4 cf = ImGui::ColorConvertU32ToFloat4(color);
		if (ImGui::ColorEdit4(name, &cf.x)) {
			color = ImGui::ColorConvertFloat4ToU32(cf);
			return true;
		}
		return false;
	}

	void AnimDictEditor(EditorInterface& ui, CAnimationDictionary* animDict) {
		CAnimationManager* animMgr = ui.kenv.levelObjects.getFirst<CAnimationManager>();
		ImGui::PushID(animDict);
		ImGui::Indent();
		if (ImGui::CollapsingHeader("Animation Dictionary")) {
			ImGui::BeginChild("AnimDictEdit", ImVec2(0, 120.0f), true);
			ImGui::Columns(animDict->numSets); // TODO: correct order for Arthur sets
			for (size_t i = 0; i < animDict->animIndices.size(); ++i) {
				ImGui::PushID(i);
				ImGui::AlignTextToFramePadding();
				uint32_t animFullIndex = animDict->animIndices[i];
				uint32_t animSector = animFullIndex >> 24;
				uint32_t animIndex = animFullIndex & 0xFFFFFF;
				CSectorAnimation* secAnim = (animFullIndex == -1) ? nullptr : (ui.kenv.version < KEnvironment::KVERSION_ARTHUR) ? &animMgr->commonAnims : animMgr->arSectors[animSector].get();
				if ((i % animDict->numSets) == 0) {
					ImGui::Text("%2i:", (uint32_t)i / animDict->numSets);
					ImGui::SameLine();
				}
				if (ImGui::Button("A")) {
					auto anmpath = GuiUtils::OpenDialogBox(ui.g_window, "Renderware Animation (*.anm)\0*.ANM\0\0", "anm");
					if (!anmpath.empty()) {
						IOFile file = IOFile(anmpath.c_str(), "rb");
						RwAnimAnimation rwAnim;
						auto rwVerBackup = HeaderWriter::rwver; // TODO: Remove hack
						rwCheckHeader(&file, 0x1B);
						rwAnim.deserialize(&file);
						HeaderWriter::rwver = rwVerBackup;
						int32_t newIndex = animMgr->addAnimation(rwAnim, animDict->arSector);
						animDict->animIndices[i] = newIndex;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import from .ANM (unique)");
				ImGui::SameLine();
				ImGui::BeginDisabled(animFullIndex == -1);
				if (ImGui::Button("I")) {
					auto anmpath = GuiUtils::OpenDialogBox(ui.g_window, "Renderware Animation (*.anm)\0*.ANM\0\0", "anm");
					if (!anmpath.empty()) {
						IOFile file = IOFile(anmpath.c_str(), "rb");
						RwAnimAnimation& rwAnim = secAnim->anims[animIndex].rwAnim;
						rwAnim = {};
						auto rwVerBackup = HeaderWriter::rwver; // TODO: Remove hack
						rwCheckHeader(&file, 0x1B);
						rwAnim.deserialize(&file);
						HeaderWriter::rwver = rwVerBackup;
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import from .ANM (shared)");
				ImGui::SameLine();
				if (ImGui::Button("E")) {
					auto anmpath = GuiUtils::SaveDialogBox(ui.g_window, "Renderware Animation (*.anm)\0*.ANM\0\0", "anm");
					if (!anmpath.empty()) {
						IOFile file = IOFile(anmpath.c_str(), "wb");
						RwAnimAnimation& rwAnim = secAnim->anims[animIndex].rwAnim;
						rwAnim.serialize(&file);
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export to .ANM");
				ImGui::SameLine();
				if (animFullIndex != -1)
					ImGui::Text("Sec %2u, Index %3u", animSector, animIndex);
				else
					ImGui::TextUnformatted("None");
				ImGui::EndDisabled();
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			if (ImGui::Button("New slot")) {
				for (uint32_t i = 0; i < animDict->numSets; ++i) {
					animDict->animIndices.emplace_back(-1);
				}
				animDict->numAnims += 1;
			}
			ImGui::EndChild();
		}
		ImGui::Unindent();
		ImGui::PopID();
	}

	bool PropFlagsEditor(unsigned int& flagsValue, const nlohmann::json& flagsInfo) {
		bool modified = false;
		for (auto& [key, jsobj] : flagsInfo.items()) {
			auto sep = key.find('-');
			int bitStartIndex = 0, bitEndIndex = 0;
			if (sep == key.npos) {
				bitStartIndex = bitEndIndex = std::stoi(key);
			}
			else {
				std::from_chars(key.data(), key.data() + sep, bitStartIndex);
				std::from_chars(key.data() + sep + 1, key.data() + key.size(), bitEndIndex);
			}

			unsigned int mask = ((1 << (bitEndIndex - bitStartIndex + 1)) - 1) << bitStartIndex;
			if (jsobj.is_string()) {
				const auto& name = jsobj.get_ref<const std::string&>();
				if (bitStartIndex == bitEndIndex) {
					modified |= ImGui::CheckboxFlags(name.c_str(), &flagsValue, 1 << bitStartIndex);
				}
				else {
					unsigned int v = (flagsValue & mask) >> bitStartIndex;
					ImGui::SetNextItemWidth(48.0f);
					bool b = ImGui::InputScalar(name.c_str(), ImGuiDataType_U32, &v);
					if (b) {
						modified = true;
						flagsValue = (flagsValue & ~mask) | ((v << bitStartIndex) & mask);
					}
				}
			}
			else if (jsobj.is_object()) {
				unsigned int v = (flagsValue & mask) >> bitStartIndex;
				const auto& name = jsobj.at("name").get_ref<const std::string&>();
				std::string preview = std::to_string(v);
				if (auto it = jsobj.find(preview); it != jsobj.end())
					preview = it->get_ref<const std::string&>();
				bool b = false;
				if (ImGui::BeginCombo(name.c_str(), preview.c_str())) {
					b |= ImGui::InputScalar("Value", ImGuiDataType_U32, &v);
					for (auto& [ek, ev] : jsobj.items()) {
						if (ek != "name") {
							if (ImGui::Selectable(ev.get_ref<const std::string&>().c_str())) {
								v = std::stoi(ek);
								b = true;
							}
						}
					}
					ImGui::EndCombo();
				}
				if (b) {
					modified = true;
					flagsValue = (flagsValue & ~mask) | ((v << bitStartIndex) & mask);
				}

			}
		}
		return modified;
	}
}

// Manages the Event names JSON
static Encyclopedia g_encyclo;

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

	CKBeaconKluster::Beacon* getBeaconPtr() const {
		if (CKBeaconKluster* kluster = getKluster())
			if (bingIndex >= 0 && bingIndex < (int)kluster->bings.size())
				if (beaconIndex >= 0 && beaconIndex < (int)kluster->bings[bingIndex].beacons.size())
					return &kluster->bings[bingIndex].beacons[beaconIndex];
		return nullptr;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return getBeaconPtr() != nullptr; }
	Matrix getTransform() override { return Matrix::getTranslationMatrix(getBeaconPtr()->getPosition()); }
	void setTransform(const Matrix &mat) override {
		getBeaconPtr()->setPosition(mat.getTranslationVector());
		UpdateBeaconKlusterBounds(getKluster());
	}

	void duplicate() override {
		if (!hasTransform()) return;
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		const CKBeaconKluster::Beacon* originalBeacon = getBeaconPtr();
		int dupKlusterIndex = srvBeacon->addKluster(ui.kenv, sectorIndex);
		srvBeacon->addBeacon(sectorIndex, dupKlusterIndex, bingIndex, originalBeacon);
		UpdateBeaconKlusterBounds(srvBeacon->beaconSectors[sectorIndex].beaconKlusters[dupKlusterIndex].get());
	}
	bool remove() override {
		if (!hasTransform()) return false;
		CKSrvBeacon* srvBeacon = ui.kenv.levelObjects.getFirst<CKSrvBeacon>();
		srvBeacon->removeBeacon(sectorIndex, klusterIndex, bingIndex, beaconIndex);
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
		return fmt::format("Beacon {}", getBeaconName(ui.kenv, srvBeacon->handlers[bingIndex].handlerId));
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

	CKSrvMarker::Marker *marker;

	MarkerSelection(EditorInterface &ui, Vector3 &hitpos, CKSrvMarker::Marker *marker) : UISelection(ui, hitpos), marker(marker) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return true; }
	Matrix getTransform() override { return Matrix::getTranslationMatrix(marker->position); }
	void setTransform(const Matrix &mat) override { marker->position = mat.getTranslationVector(); }
	void onSelected() override { ui.selectedMarker = marker; }
	std::string getInfo() override { return fmt::format("Marker {}: {}", marker - ui.kenv.levelObjects.getFirst<CKSrvMarker>()->lists.front().data(), marker->name); }
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


// Creates ImGui editing widgets for every member in a member-reflected object
struct ImGuiMemberListener : NamedMemberListener {
	KEnvironment &kenv; EditorInterface &ui;
	MemberFlags currentFlags = MemberFlags::MF_NONE;
	std::stack<bool> scopeExpanded;
	
	ImGuiMemberListener(KEnvironment &kenv, EditorInterface &ui) : kenv(kenv), ui(ui) {}

	bool icon(const char *label, const char *desc = nullptr) {
		if (!scopeExpanded.empty() && scopeExpanded.top() == false)
			return false;
		bool hidden = (int)currentFlags & (int)MemberFlags::MF_EDITOR_HIDDEN;
		if (hidden)
			return false;
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(ImVec4(0, 1, 1, 1), label);
		if (desc && ImGui::IsItemHovered())
			ImGui::SetTooltip(desc);
		ImGui::SameLine();
		return true;
	}
	void reflect(uint8_t& ref, const char* name) override { if (icon(" 8", "Unsigned 8-bit integer")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_U8, &ref); }
	void reflect(uint16_t& ref, const char* name) override { if (icon("16", "Unsigned 16-bit integer")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_U16, &ref); }
	void reflect(uint32_t& ref, const char* name) override { if (icon("32", "Unsigned 32-bit integer")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_U32, &ref); }
	void reflect(int8_t& ref, const char* name) override { if (icon(" 8", "Signed 8-bit integer")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_S8, &ref); }
	void reflect(int16_t& ref, const char* name) override { if (icon("16", "Signed 16-bit integer")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_S16, &ref); }
	void reflect(int32_t& ref, const char* name) override { if (icon("32", "Signed 32-bit integer")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_S32, &ref); }
	void reflect(float& ref, const char* name) override { if (icon("Fl", "IEEE 754 Single floating-point number")) ImGui::InputScalar(getShortName(name).c_str(), ImGuiDataType_Float, &ref); }
	void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override { if (icon("Rf", "Object reference")) { ui.IGObjectSelector(kenv, getShortName(name).c_str(), ref, clfid); compositionEditor(ref.get(), clfid, name); } }
	void reflect(Vector3& ref, const char* name) override { if (icon("V3", "3D Floating-point vector")) ImGui::InputFloat3(getShortName(name).c_str(), &ref.x, "%.2f"); }
	void reflect(Matrix& ref, const char* name) override {
		if (icon("Mx", "4x4 transformation matrix")) {
			std::string fullName = getShortName(name);
			for (int i = 0; i < 4; ++i) {
				if (i != 0)
					icon("..", "Matrix continuation");
				ImGui::InputFloat3((fullName + ".Row" + (char)('0' + i)).c_str(), &ref.v[4 * i], "%.2f");
			}
		}
	}
	void reflect(EventNode& ref, const char* name, CKObject* user) override {
		if (icon("Ev", "Event sequence node")) {
			auto fullName = getShortName(name);
			ui.IGEventSelector(fullName.c_str(), ref);
		}
	}
	void reflect(MarkerIndex& ref, const char* name) override {
		if (icon("Mk", "Marker")) {
			ui.IGMarkerSelector(getShortName(name).c_str(), ref);
		}
	}
	void reflectPostRefTuple(uint32_t& tuple, const char* name) override {
		if (icon("PR", "Undecoded object reference (Postponed reference)")) {
			int igtup[3] = { tuple & 63, (tuple >> 6) & 2047, tuple >> 17 };
			if (ImGui::InputInt3(getShortName(name).c_str(), igtup)) {
				tuple = (igtup[0] & 63) | ((igtup[1] & 2047) << 6) | ((igtup[2] & 32767) << 17);
			}
		}
	}
	void reflect(std::string& ref, const char* name) override {
		if (icon("St", "Character string")) {
			ImGui::InputText(getShortName(name).c_str(), (char*)ref.c_str(), ref.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &ref);
		}
	}

	void setNextFlags(MemberFlags flags) override { currentFlags = flags; }

	void enterArray(const char* name) override {
		if (icon("[]", "Array") && ImGui::TreeNodeEx(getShortName(name).c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			scopeExpanded.push(true);
		else
			scopeExpanded.push(false);
		NamedMemberListener::enterArray(name);
	}

	void leaveArray() override {
		NamedMemberListener::leaveArray();
		if (scopeExpanded.top())
			ImGui::TreePop();
		scopeExpanded.pop();
	}

	void enterStruct(const char* name) override {
		if (icon("{}", "Structure") && ImGui::TreeNodeEx(getShortName(name).c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
			scopeExpanded.push(true);
		else
			scopeExpanded.push(false);
		NamedMemberListener::enterStruct(name);
	}

	void leaveStruct() override {
		NamedMemberListener::leaveStruct();
		if (scopeExpanded.top())
			ImGui::TreePop();
		scopeExpanded.pop();
	}

	void compositionEditor(CKObject* obj, int clfid, const char* name) {
		if (!obj) return;
		if (obj->getClassFullID() == CAnimationDictionary::FULL_ID) {
			AnimDictEditor(ui, (CAnimationDictionary*)obj);
		}
		else if (obj->getClassFullID() == CKSoundDictionaryID::FULL_ID) {
			SoundDictIDEditor(ui, (CKSoundDictionaryID*)obj);
		}
	}

	static void SoundDictIDEditor(EditorInterface& ui, CKSoundDictionaryID* sndDictID) {
		if (ui.kenv.version >= KEnvironment::KVERSION_XXL2 && !ui.kenv.hasClass<CKSound>())
			return;
		ImGui::PushID(sndDictID);
		ImGui::Indent();
		if (ImGui::CollapsingHeader("Sound ID Dictionary")) {
			ImGui::BeginChild("SndDictIDEdit", ImVec2(0, 500.0f/*120.0f*/), true);
			ImGui::Text("Ref count: %i", sndDictID->getRefCount());
			int numSlots = (ui.kenv.version >= KEnvironment::KVERSION_XXL2) ? sndDictID->x2Sounds.size() : sndDictID->soundEntries.size();
			for (int i = 0; i < numSlots; ++i) {
				bool slotActive;
				int sectorIndex = 0;
				int soundId = -1;
				if (ui.kenv.version >= KEnvironment::KVERSION_ARTHUR) {
					CKSound* snd = sndDictID->x2Sounds[i]->cast<CKSound>();
					slotActive = snd->sndWaveObj.get();
					if (slotActive) {
						// we need to find the CKWave's corresponding sector+id
						CKLevel* level = ui.kenv.levelObjects.getFirst<CKLevel>();
						for (int str = 0; str < level->sectors.size() && soundId == -1; ++str) {
							CKSoundDictionary* dict = level->sectors[str]->soundDictionary.get() ? level->sectors[str]->soundDictionary->cast<CKSoundDictionary>() : nullptr;
							if (dict) {
								for (int id = 0; id < dict->sounds.size() && soundId == -1; ++id) {
									if (dict->sounds[id].waveObj.get() == snd->sndWaveObj.get()) {
										// found
										sectorIndex = str;
										soundId = id;
									}
								}
							}
						}
						assert(soundId != -1);
					}
				}
				else if (ui.kenv.version >= KEnvironment::KVERSION_XXL2) {
					CKSound* snd = sndDictID->x2Sounds[i]->cast<CKSound>();
					slotActive = (snd->sndIndex & 0xFFFFFF) != 0xFFFFFF;
					if (slotActive) {
						sectorIndex = snd->sndIndex >> 24;
						soundId = snd->sndIndex & 0xFFFFFF;
					}
				}
				else {
					auto& snd = sndDictID->soundEntries[i];
					slotActive = snd.active;
					sectorIndex = snd.id >> 24;
					soundId = snd.id & 0xFFFFFF;
				}
				CKSoundDictionary* sndDict = nullptr;
				if (slotActive)
					sndDict = ui.kenv.levelObjects.getFirst<CKLevel>()->sectors[sectorIndex]->soundDictionary->cast<CKSoundDictionary>();
				ImGui::PushID(i);
				ImGui::AlignTextToFramePadding();
				ImGui::BeginGroup();
				bool enabled = slotActive && soundId >= 0 && soundId < sndDict->sounds.size();
				ImGui::BeginDisabled(!enabled);
				if (ImGui::ArrowButton("PlaySound", ImGuiDir_Right) && enabled) {
					PlaySnd(ui.kenv, sndDict->rwSoundDict.list.sounds[soundId]);
				}
				ImGui::EndDisabled();
				ImGui::SameLine();
				ImGui::Text("Slot %2i:", i);
				ImGui::SameLine();
				if (slotActive) {
					ImGui::Text("Sector %i ID %i", sectorIndex, soundId);
				}
				else {
					ImGui::TextDisabled("No sound");
				}
				ImGui::EndGroup();
				if (ImGui::BeginDragDropTarget()) {
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SoundIndex");
					if (payload) {
						int32_t fullIndex = *(int32_t*)payload->Data;
						if (ui.kenv.version >= KEnvironment::KVERSION_ARTHUR) {
							int sector = fullIndex >> 24;
							int id = fullIndex & 0xFFFFFF;
							sndDictID->x2Sounds[i]->cast<CKSound>()->sndWaveObj = ui.kenv.levelObjects.getFirst<CKLevel>()->sectors[sector]->soundDictionary->cast<CKSoundDictionary>()->sounds[id].waveObj.get();
						}
						else if (ui.kenv.version >= KEnvironment::KVERSION_XXL2)
							sndDictID->x2Sounds[i]->cast<CKSound>()->sndIndex = fullIndex;
						else
							sndDictID->soundEntries[i].id = fullIndex;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				if (ImGui::TreeNode("More")) {
					ImGuiMemberListener igml(ui.kenv, ui);
					MemberListener& ml = igml;
					if (ui.kenv.version >= KEnvironment::KVERSION_XXL2) {
						igml.setPropertyInfoList(g_encyclo, sndDictID->x2Sounds[i].get());
						sndDictID->x2Sounds[i]->cast<CKSound>()->virtualReflectMembers(ml, &ui.kenv);
					}
					else {
						auto& e = sndDictID->soundEntries[i];
						ImGui::Checkbox("Active", &e.active);
						ml.reflect(e.id, "id");
						ml.reflect(e.flags, "flags");
						bool onSceneNode = e.flags & 16;
						ImGui::CheckboxFlags("Playing", &e.flags, 8);
						ImGui::TextUnformatted("Play on:");
						ImGui::SameLine();
						if (ImGui::RadioButton("Position", !onSceneNode))
							e.flags &= ~16;
						ImGui::SameLine();
						if (ImGui::RadioButton("Scene node", onSceneNode))
							e.flags |= 16;
						if (e.flags & 16)
							ml.reflect(e.obj, "obj");
						else
							ml.reflect(e.refalt, "position");
						ml.reflect(e.volume, "volume");
						ml.reflect(e.speed, "speed");
						ml.reflect(e.replayAfterMin, "replayAfterMin");
						ml.reflect(e.replayAfterMax, "replayAfterMax");
						ml.reflect(e.boxHigh, "boxHigh");
						ml.reflect(e.boxLow, "boxLow");
						bool pil = e.playInLoop;
						if (ImGui::Checkbox("Play in loop", &pil))
							e.playInLoop = pil;
					}
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			if (true) {
				if (ImGui::Button("New slot")) {
					if (ui.kenv.version == KEnvironment::KVERSION_XXL1) {
						sndDictID->soundEntries.emplace_back();
					}
					else {
						CKSound* ksound = ui.kenv.createAndInitObject<CKSound>();
						sndDictID->x2Sounds.emplace_back() = ksound;
					}
				}
			}
			ImGui::EndChild();
		}
		ImGui::Unindent();
		ImGui::PopID();
	}
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
	camera.aspect = (float)g_window->getWidth() / g_window->getHeight();
	camera.updateMatrix();
	float camspeed = _camspeed;
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

	if (nearestRayHit) {
		const auto &selection = nearestRayHit;
		if (selection->hasTransform()) {
			Matrix gzmat = selection->getTransform();
			Matrix originalMat = gzmat;
			Matrix delta;
			const float snapAngle = 15.0f;
			const float* snap = (gzoperation == ImGuizmo::ROTATE && g_window->isAltPressed()) ? &snapAngle : nullptr;
			ImGuizmo::Manipulate(camera.viewMatrix.v, camera.projMatrix.v, (ImGuizmo::OPERATION)gzoperation, ImGuizmo::WORLD, gzmat.v, delta.v, snap);
			if (gzmat != originalMat)
				selection->setTransform(gzmat);
		}
	}

	// ImGuizmo View Cube
	const float viewControlSize = 96.0f;
	Matrix mmoriginal = camera.viewMatrix;
	Matrix mmcopy = mmoriginal;
	ImGuizmo::ViewManipulate(camera.viewMatrix.v, 10.0f, ImVec2(g_window->getWidth() - viewControlSize, g_window->getHeight() - viewControlSize), ImVec2(viewControlSize, viewControlSize), 0x40000000);
	// new matrix, need to decompose to position + orientation
	// actually obtain position + direction, then obtain orientation from direction (see Camera::update)
	Matrix invView = camera.viewMatrix.getInverse4x4();
	Vector3 newPos = Vector3(0, 0, 0).transform(invView);
	Vector3 newDir = -(Vector3(0, 0, 1).transform(invView) - newPos).normal();
	float newAngleX = std::asin(newDir.y);
	Vector3 newOri;
	float cosAX = std::cos(newAngleX);
	float mar = 1.5707f;
	if(!(-mar <= newAngleX && newAngleX <= mar))
		newOri = { std::clamp(newAngleX, -mar + 0.0005f, mar - 0.0005f), 0, 0 };
	else
		newOri = { newAngleX, std::atan2(-newDir.x / cosAX, newDir.z / cosAX) /*std::asin(-newDir.x / cosAX)*/ };
	camera.position = newPos;
	camera.orientation = newOri;

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
		if (ImGui::ImageButton(tbTexture, ImVec2(BUTTON_SIZE, BUTTON_SIZE), ImVec2(tx, ty), ImVec2(tx + delta, ty + delta))) {
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
		if (kenv.version <= kenv.KVERSION_XXL1)
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
		igwindow("Textures", &wndShowTextures, [](EditorInterface *ui) { ui->IGTextureEditor(); });
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
		igwindow("Pathfinding", &wndShowPathfinding, [](EditorInterface *ui) { ui->IGPathfindingEditor(); });
	if (kenv.hasClass<CKSrvMarker>())
		igwindow("Markers", &wndShowMarkers, [](EditorInterface *ui) { ui->IGMarkerEditor(); });
	if (kenv.hasClass<CKSrvDetector>())
		igwindow("Detectors", &wndShowDetectors, [](EditorInterface *ui) { ui->IGDetectorEditor(); });
	if (kenv.hasClass<CKSectorDetector>())
		igwindow("X2 Detectors", &wndShowDetectors, [](EditorInterface* ui) { ui->IGX2DetectorEditor(); });
	if (kenv.hasClass<CKSrvCinematic>())
		igwindow("Cinematic", &wndShowCinematic, [](EditorInterface *ui) { ui->IGCinematicEditor(); });
	if (kenv.hasClass<CKSrvCollision>())
		igwindow("Collision", &wndShowCollision, [](EditorInterface *ui) { ui->IGCollisionEditor(); });
	if (kenv.hasClass<CKLine>())
		igwindow("Lines", &wndShowLines, [](EditorInterface* ui) { ui->IGLineEditor(); });
	igwindow("Localization", &wndShowLocale, [](EditorInterface *ui) { ui->IGLocaleEditor(); });
	igwindow("Objects", &wndShowObjects, [](EditorInterface *ui) { ui->IGObjectTree(); });
	igwindow("Level", &wndShowLevel, [](EditorInterface* ui) { ui->IGLevelEditor(); });
	igwindow("Misc", &wndShowMisc, [](EditorInterface *ui) { ui->IGMiscTab(); });
	igwindow("About", &wndShowAbout, [](EditorInterface* ui) { ui->IGAbout(); });
	if (kenv.hasClass<CKSrvCamera>())
		igwindow("Camera", &wndShowCamera, [](EditorInterface* ui) { ui->IGCamera(); });
	if (kenv.hasClass<CKSrvCounter>())
		igwindow("Counters", &wndShowCounters, [](EditorInterface* ui) { ui->IGCounters(); });
	if (kenv.hasClass<CKSrvMusic>() && kenv.hasClass<CKMusicPlayList>())
		igwindow("Music", &wndShowMusic, [](EditorInterface* ui) {ui->IGMusic(); });
	if (kenv.hasClass<CKSrvSekensor>() && kenv.hasClass<CKSekens>())
		igwindow("Sekens (dialogue)", &wndShowSekens, [](EditorInterface* ui) {ui->IGSekens(); });
	igwindow("Object inspector", &wndShowObjInspector, [](EditorInterface* ui) {ui->IGObjectInspector(); });

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
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					pos.y += 0.5f;
					if (handlerFID == CKCrateCpnt::FULL_ID && kenv.hasClass<CSGRootNode>()) {
						int numCrates = beacon.params & 7;

						CKCrateCpnt *cratecpnt = bing.handler->cast<CKCrateCpnt>();
						if (!cratecpnt->crateNode) // happens in Romaster
							goto drawFallbackSphere;
						size_t clindex = getCloneIndex(cratecpnt->crateNode->cast<CClone>());

						for (int c = 0; c < numCrates; c++) {
							gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos + Vector3(0, (float)c, 0)) * camera.sceneMatrix);
							drawClone(clindex);
						}
					}
					else if (bing.handler->isSubclassOf<CKGrpBonusPool>() && kenv.hasClass<CSGRootNode>()) {
						CKGrpBonusPool *pool = bing.handler->cast<CKGrpBonusPool>();
						CKHook *hook = pool->childHook.get();

						gfx->setTransformMatrix(rotmat * Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
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
						gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
						gfx->setBlendColor(0xFF000000 | fallbackSphereColor);
						progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
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

			if (showingChoreoKey < (int)squad->choreoKeys.size()) {
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
						uint8_t poolindex = (slot.enemyGroup != -1) ? slot.enemyGroup : defaultpool;
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
				if (showingChoreoKey < (int)choreo->keys.size()) {
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
							uint8_t poolindex = (slot.enemyGroup != -1) ? slot.enemyGroup : defaultpool;
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
					gfx->setTransformMatrix(Matrix::getTranslationMatrix(marker.position) * camera.sceneMatrix);
					progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
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
}

void EditorInterface::IGObjectSelector(KEnvironment &kenv, const char * name, kanyobjref & ptr, uint32_t clfid)
{
	auto className = [](CKObject* obj) -> const char* {
		static char unkbuf[32];
		if (dynamic_cast<CKUnknown*>(obj)) {
			sprintf_s(unkbuf, "(%i, %i)", obj->getClassCategory(), obj->getClassID());
			return unkbuf;
		}
		else
			return obj->getClassName();
	};
	char tbuf[128] = "(null)";
	if(CKObject *obj = ptr._pointer)
		_snprintf_s(tbuf, _TRUNCATE, "%s : %s (%p)", className(obj), kenv.getObjectName(obj), obj);
	if (ImGui::BeginCombo(name, tbuf, 0)) {
		if (ImGui::Selectable("(null)", ptr._pointer == nullptr))
			ptr.anyreset();
		for (uint32_t clcatnum = 0; clcatnum < 15; clcatnum++) {
			if (clfid != 0xFFFFFFFF && (clfid & 63) != clcatnum)
				continue;
			auto &clcat = kenv.levelObjects.categories[clcatnum];
			for (uint32_t clid = 0; clid < clcat.type.size(); clid++) {
				auto &cl = clcat.type[clid];
				for (CKObject *eo : cl.objects) {
					if (clfid != 0xFFFFFFFF && !eo->isSubclassOfID(clfid))
						continue;
					ImGui::PushID(eo);
					if (ImGui::Selectable("##objsel", eo == ptr._pointer)) {
						ptr.anyreset(eo);
					}
					if (ImGui::IsItemVisible()) {
						ImGui::SameLine(0.0f, 0.0f);
						ImGui::Text("%s : %s (%p)", className(eo), kenv.getObjectName(eo), eo);
					}
					ImGui::PopID();
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ptr) {
		IGObjectDragDropSource(kenv, ptr._pointer);
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload *payload = ImGui::GetDragDropPayload()) {
			if (payload->IsDataType("CKObject")) {
				CKObject *obj = *(CKObject**)payload->Data;
				if (clfid == 0xFFFFFFFF || obj->isSubclassOfID(clfid))
					if (const ImGuiPayload *acceptedPayload = ImGui::AcceptDragDropPayload("CKObject"))
						ptr.anyreset(*(CKObject**)payload->Data);
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void EditorInterface::IGObjectDragDropSource(KEnvironment& kenv, CKObject* obj)
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		ImGui::SetDragDropPayload("CKObject", &obj, sizeof(obj));
		ImGui::Text("%p : %i %i %s : %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName(), kenv.getObjectName(obj));
		ImGui::EndDragDropSource();
	}
}

void EditorInterface::IGObjectSelector(KEnvironment& kenv, const char* name, KAnyPostponedRef& postref, uint32_t clfid)
{
	if (postref.bound)
		IGObjectSelector(kenv, name, postref.ref, clfid);
	else {
		uint32_t& tuple = postref.id;
		int igtup[3] = { tuple & 63, (tuple >> 6) & 2047, tuple >> 17 };
		if (ImGui::InputInt3(name, igtup)) {
			tuple = (igtup[0] & 63) | ((igtup[1] & 2047) << 6) | ((igtup[2] & 32767) << 17);
		}
	}
}

void EditorInterface::IGEventSelector(const char* name, EventNode& ref) {
	if (kenv.version < KEnvironment::KVERSION_XXL2)
		IGEventSelector(name, ref.enx1);
	else
		IGEventSelector(name, ref.enx2);
}

void EditorInterface::IGEventSelector(const char* name, EventNodeX1& ref) {
	ImGui::PushID(name);
	ImGui::BeginGroup();
	int igtup[2] = { ref.seqIndex, ref.bit };
	float itemwidth = ImGui::CalcItemWidth();
	ImGui::SetNextItemWidth(itemwidth - ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
	if (ImGui::InputInt2("##HkEventBox", igtup)) {
		ref.seqIndex = (int16_t)igtup[0]; ref.bit = (uint8_t)igtup[1] & 7;
	}
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	if (ImGui::ArrowButton("HkSelectEvent", ImGuiDir_Right)) {
		CKSrvEvent* srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>();
		auto& seqids = srvEvent->evtSeqIDs;
		auto it = std::find(seqids.begin(), seqids.end(), ref.seqIndex);
		if (it != seqids.end()) {
			selectedEventSequence = (int)(it - seqids.begin());
			wndShowEvents = true;
		}
		else
			ImGui::OpenPopup("EventNodeNotFound");
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Select event sequence");
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text(name);
	ImGui::EndGroup();
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		EventNodeX1* data = &ref;
		ImGui::SetDragDropPayload("EventNodeX1", &data, sizeof(data));
		ImGui::Text("Event node");
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EventSeq")) {
			ref.seqIndex = *(uint16_t*)payload->Data;
			ref.bit = 0;
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::BeginPopup("EventNodeNotFound")) {
		ImGui::Text("Event sequence of ID %i no longer exists.", ref.seqIndex);
		ImGui::EndPopup();
	}
	ImGui::PopID();
}

using EventNodePayload = std::pair<EventNodeX2*, char[16]>;

void EditorInterface::IGEventSelector(const char* name, EventNodeX2& ref) {
	ref.clean();
	ImGui::LabelText(name, "%zi CmpDatas", ref.datas.size());
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		EventNodePayload data;
		data.first = &ref;
		strncpy_s(data.second, name, sizeof(data.second));
		ImGui::SetDragDropPayload("EventNodeX2", &data, sizeof(data));
		ImGui::Text("Event node %s", name);
		ImGui::EndDragDropSource();
	}
}

void EditorInterface::IGMarkerSelector(const char* name, MarkerIndex& ref)
{
	ImGui::PushID(name);
	float itemwidth = ImGui::CalcItemWidth();
	ImGui::SetNextItemWidth(itemwidth - ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
	if (kenv.version == KEnvironment::KVERSION_XXL1)
		ImGui::InputInt("##MarkerInput", &ref.index);
	else if (kenv.version >= KEnvironment::KVERSION_XXL2)
		ImGui::InputScalarN("##MarkerInput", ImGuiDataType_U8, &ref.index, 4);
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	if (ImGui::ArrowButton("SelectMarker", ImGuiDir_Right)) {
		if (kenv.version == KEnvironment::KVERSION_XXL1) {
			CKSrvMarker* srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>();
			if (srvMarker && !srvMarker->lists.empty()) {
				auto& list = srvMarker->lists.front();
				if (ref.index >= 0 && ref.index < (int)list.size()) {
					selectedMarker = &list[ref.index];
					wndShowMarkers = true;
				}
			}
		}
		else if(kenv.version >= KEnvironment::KVERSION_XXL2) {
			selBeaconSector = ref.index & 255;
			selBeaconKluster = (ref.index >> 8) & 255;
			selBeaconBing = (ref.index >> 16) & 255;
			selBeaconIndex = (ref.index >> 24) & 255;
			wndShowBeacons = true;
		}
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Select marker");
	ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::TextUnformatted(name);
	ImGui::PopID();
}

bool EditorInterface::IGEventMessageSelector(const char* label, uint16_t& message, int fid, bool isCallback)
{
	static const std::string msgActionCallbacksSetName = "Squad MsgAction Callbacks";
	bool modified = false;
	auto* eventJson = g_encyclo.getEventJson(fid, message);
	std::string eventDesc = g_encyclo.getEventName(eventJson, message);
	bool hasIndex = eventJson && eventJson->at("id").get_ref<const std::string&>().size() == 9;
	float original_x = ImGui::GetCursorPosX();
	float width = ImGui::CalcItemWidth();
	ImGui::SetNextItemWidth(width - (hasIndex ? 32.0f + ImGui::GetStyle().ItemSpacing.x : 0.0f));
	if (ImGui::BeginCombo("##EventCombo", eventDesc.c_str())) {
		modified |= ImGui::InputScalar("##EventID", ImGuiDataType_U16, &message, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);

		auto lookAtList = [&message, &modified](const nlohmann::json& evlist) {
			for (auto& ev : evlist) {
				auto& strId = ev.at("id").get_ref<const std::string&>();
				auto& strName = ev.at("name").get_ref<const std::string&>();
				std::string desc = strId + ": " + strName;
				auto [idStart, idEnd] = Encyclopedia::decodeRange(strId);
				if (ImGui::Selectable(desc.c_str(), idStart <= message && message <= idEnd)) {
					message = idStart;
					modified = true;
				}
			}
		};

		if (isCallback) {
			lookAtList(g_encyclo.eventSets.at(msgActionCallbacksSetName).at("events"));
		}
		else if (fid != -1) {
			if (auto cit = g_encyclo.kclasses.find(fid); cit != g_encyclo.kclasses.end()) {
				if (auto itEvents = cit->second.find("events"); itEvents != cit->second.end())
					lookAtList(itEvents.value());
				if (auto itIncludes = cit->second.find("includeSets"); itIncludes != cit->second.end()) {
					for (auto& incName : itIncludes.value()) {
						auto& strIncName = incName.get_ref<const std::string&>();
						if (strIncName == msgActionCallbacksSetName)
							continue;
						auto& incSet = g_encyclo.eventSets.at(strIncName);
						lookAtList(incSet.at("events"));
					}
				}
			}
		}
		ImGui::EndCombo();
	}
	if (hasIndex) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(32.0f);
		auto [idStart, idEnd] = Encyclopedia::decodeRange(eventJson->at("id").get_ref<const std::string&>());
		int index = (int)message - idStart;
		if (ImGui::InputInt("##EventIndex", &index, 0, 0)) {
			int newIndex = std::clamp(idStart + index, idStart, idEnd);
			message = (uint16_t)newIndex;
			modified = true;
		}
	}
	ImGui::SameLine(original_x);
	ImGui::LabelText(label, "");
	return modified;
}
bool EditorInterface::IGEventMessageSelector(const char* label, uint16_t& message, CKObject* kobj, bool isCallback)
{
	return IGEventMessageSelector(label, message, kobj ? (int)kobj->getClassFullID() : -1, isCallback);
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
			selectedMarker = nullptr;
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

void EditorInterface::IGMiscTab()
{
#ifndef XEC_RELEASE
	ImGui::Checkbox("Show ImGui Demo", &showImGuiDemo);
#endif
	if (kenv.version == kenv.KVERSION_XXL1) {
		if (ImGui::Button("Rocket Romans \\o/"))
			GimmeTheRocketRomans(kenv);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Transform all Basic Enemies to Rocket Romans");
		if (ImGui::Button("Export Basic Enemy Cpnt Values to TXT")) {
			using namespace GameX1;
			CKBasicEnemyCpnt* firstcpnt = kenv.levelObjects.getFirst<CKBasicEnemyCpnt>();
			if (firstcpnt) {
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
				auto fname = SaveDialogBox(g_window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
				if (!fname.empty()) {
					FILE* csv;
					fsfopen_s(&csv, fname, "w");
					NameListener nl(csv);
					ValueListener vl(csv);
					fprintf(csv, "Index\t");
					firstcpnt->reflectMembers2(nl, &kenv);
					int index = 0;
					for (CKObject* obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
						fprintf(csv, "\n%i\t", index);
						obj->cast<CKBasicEnemyCpnt>()->reflectMembers2(vl, &kenv);
						index++;
					}
					fclose(csv);
				}
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Export the values of every CKBasicEnemyCpnt to a Tab Separated Values (TSV) text file");
	}

	if (kenv.version == kenv.KVERSION_XXL1 && kenv.isRemaster && ImGui::Button("Convert Romaster -> Original")) {
		// Truncate CParticlesNodeFx for compatibility with original
		for (CKObject* obj : kenv.levelObjects.getClassType<CParticlesNodeFx>().objects) {
			CParticlesNodeFx *fx = obj->cast<CParticlesNodeFx>();
			fx->unkPart.resize(0x5D - 0x53);
		}
		for (auto &str : kenv.sectorObjects) {
			for (CKObject* obj : str.getClassType<CParticlesNodeFx>().objects) {
				CParticlesNodeFx *fx = obj->cast<CParticlesNodeFx>();
				fx->unkPart.resize(0x5D - 0x53);
			}
		}
		// Remove all events sent to parkour stele hooks
		if (CKSrvEvent *srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>()) {
			int ev = 0;
			for (auto &bee : srvEvent->sequences) {
				for (int j = 0; j < bee.numActions; j++) {
					if(CKObject *obj = srvEvent->objs[ev].get())
						if (obj->isSubclassOf<GameX1::CKHkParkourSteleAsterix>()) {
							srvEvent->objs.erase(srvEvent->objs.begin() + ev);
							srvEvent->objInfos.erase(srvEvent->objInfos.begin() + ev);
							bee.numActions -= 1;
							j -= 1; // Don't increment j for next iteration
							ev -= 1;
						}
					ev++;
				}
			}
		}
		// remove the parkour hooks
		if (auto *grpMeca = kenv.levelObjects.getFirst<GameX1::CKGrpMeca>()) {
			CKHook *prev = nullptr, *next;
			for (CKHook *hook = grpMeca->childHook.get(); hook; hook = next) {
				next = hook->next.get();
				if (hook->isSubclassOf<GameX1::CKHkParkourSteleAsterix>()) {
					if (prev)
						prev->next = hook->next;
					else // meaning the parkour hook is the first child of the group
						grpMeca->childHook = hook->next;
					auto *life = hook->life.get();

					// remove life
					CKBundle *bundle = grpMeca->bundle.get();
					CKHookLife *prevlife = nullptr, *nextlife;
					for (CKHookLife *cndpnt = bundle->firstHookLife.get(); cndpnt; cndpnt = nextlife) {
						nextlife = cndpnt->nextLife.get();
						if (cndpnt == life) {
							if (prevlife)
								prevlife->nextLife = cndpnt->nextLife;
							else
								bundle->firstHookLife = cndpnt->nextLife;
							hook->life = nullptr;
							kenv.removeObject(life);
							break;
						}
						else
							prevlife = cndpnt;
					}

					kenv.removeObject(hook);
				}
				else
					prev = hook;
			}
		}
		// remove romaster-specific cinematic nodes, substitute them with NOP ones
		for (CKObject *obj : kenv.levelObjects.getClassType<CKCinematicScene>().objects) {
			CKCinematicScene* scene = obj->cast<CKCinematicScene>();
			for (auto &noderef : scene->cineNodes) {
				CKCinematicNode *node = noderef.get();
				if (node->isSubclassOf<CKPauseCinematicBloc>() || node->isSubclassOf<CKTeleportCinematicBloc>()) {
					CKCinematicBloc *bloc = node->cast<CKCinematicBloc>();
					CKStartEventCinematicBloc *sub = kenv.createObject<CKStartEventCinematicBloc>(-1);
					*(CKCinematicBloc*)sub = *bloc;
					noderef = sub;
					kenv.removeObject(node);
				}
				else if (node->isSubclassOf<CKEndDoor>()) {
					CKCinematicDoor *door = node->cast<CKCinematicDoor>();
					CKLogicalAnd *sub = kenv.createObject<CKLogicalAnd>(-1);
					*(CKCinematicDoor*)sub = *door;
					noderef = sub;
					kenv.removeObject(node);
				}
			}
		}
		// remove remaining romaster-specific cinematic nodes that were not referenced
		for (int clid : {CKPauseCinematicBloc::FULL_ID, CKTeleportCinematicBloc::FULL_ID, CKEndDoor::FULL_ID}) {
			auto &cls = kenv.levelObjects.getClassType(clid);
			auto veccopy = cls.objects;
			for (CKObject *obj : veccopy)
				kenv.removeObject(obj);
			cls.info = 0;
		}
		// shorten class list for hooks + logic misc
		kenv.levelObjects.categories[CKHook::CATEGORY].type.resize(208);
		kenv.levelObjects.categories[CKLogic::CATEGORY].type.resize(133);
		for (auto &str : kenv.sectorObjects) {
			str.categories[CKHook::CATEGORY].type.resize(208);
			str.categories[CKLogic::CATEGORY].type.resize(133);
		}
		kenv.isRemaster = false;
	}

	if (ImGui::Button("Reload Encyclopedia"))
		g_encyclo.clear();

	if (ImGui::Button("Save GAME file"))
		kenv.saveGameFile();

	if (ImGui::CollapsingHeader("Ray Hits")) {
		ImGui::Columns(2);
		for (auto &hit : rayHits) {
			ImGui::BulletText("%f", (camera.position - hit->hitPosition).len3());
			ImGui::NextColumn();
			if (hit->is<NodeSelection>()) {
				NodeSelection *ns = (NodeSelection*)hit.get();
				ImGui::Text("%i %p %s", hit->getTypeID(), ns->node, ns->node->getClassName());
			}
			else
				ImGui::Text("%i", hit->getTypeID());
			ImGui::NextColumn();
		}
		ImGui::Columns();
	}
	if (ImGui::CollapsingHeader("Unknown classes")) {
		for (auto &cl : CKUnknown::hits) {
			ImGui::BulletText("%i %i", cl.first, cl.second);
		}
	}
}

void EditorInterface::IGObjectTree()
{
	static const char *catnames[15] = { "Managers", "Services", "Hooks",
		"Hook Lives", "Groups", "Group Lives", "Components", "Camera",
		"Cinematic blocs", "Dictionaries", "Geometries", "Scene nodes",
		"Logic stuff", "Graphical stuff", "Errors"
	};
	auto enumObjList = [this](KObjectList &objlist) {
		for (int i = 0; i < 15; i++) {
			if (ImGui::TreeNode(catnames[i])) {
				for (auto &cl : objlist.categories[i].type) {
					if (!cl.objects.empty()) {
						CKObject* first = cl.objects[0];
						if (ImGui::TreeNode(&cl, "%s (%i, %i), %zu objects", first->getClassName(), first->getClassCategory(), first->getClassID(), cl.objects.size())) {
							int n = 0;
							for (CKObject* obj : cl.objects) {
								bool b = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%i, refCount=%i, %s", n, obj->getRefCount(), kenv.getObjectName(obj));
								IGObjectDragDropSource(kenv, obj);
								if (b)
									ImGui::TreePop();
								n++;
							}
							ImGui::TreePop();
						}
					}
				}
				ImGui::TreePop();
			}
		}
	};
	if (ImGui::TreeNode("Global (GAME)")) {
		for (CKObject* obj : kenv.globalObjects) {
			bool b = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%s (%i, %i), refCount=%i, %s", obj->getClassName(), obj->getClassCategory(), obj->getClassID(), obj->getRefCount(), kenv.getObjectName(obj));
			IGObjectDragDropSource(kenv, obj);
			if (b)
				ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Level (LVL)")) {
		enumObjList(kenv.levelObjects);
		ImGui::TreePop();
	}
	int i = 0;
	for (auto &str : kenv.sectorObjects) {
		if (ImGui::TreeNode(&str, "Sector %i (STR %02i)", i + 1, i)) {
			enumObjList(str);
			ImGui::TreePop();
		}
		i++;
	}
}

void EditorInterface::IGBeaconGraph()
{
	static auto getBeaconName = [this](int id) {return ::getBeaconName(kenv, id); };
	static const char* bonusNamesX1[] = { "?", "Helmet", "Golden Helmet", "Potion", "Shield", "Ham", "x3 Multiplier", "x10 Multiplier", "Laurel", "Boar", "Retro Coin", "Remaster Coin" };
	static const char* bonusNamesX2[] = { "?", "Potion", "Helmet", "Golden Helmet", "Diamond Helmet", "x3 Multiplier", "x10 Multiplier", "Ham", "Shield" };
	static auto getBonusName = [this](int bonusId) -> const char* {
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
			UpdateBeaconKlusterBounds(bk->cast<CKBeaconKluster>());
		for(auto &str : kenv.sectorObjects)
			for (CKObject *bk : str.getClassType<CKBeaconKluster>().objects)
				UpdateBeaconKlusterBounds(bk->cast<CKBeaconKluster>());
	}
	if(ImGui::BeginPopup("AddBeacon")) {
		CKSrvBeacon *srv = kenv.levelObjects.getFirst<CKSrvBeacon>();
		for (auto &hs : srv->handlers) {
			if (ImGui::MenuItem(getBeaconName(hs.handlerId))) {
				int klusterIndex = srvBeacon->addKluster(kenv, spawnSector);
				CKBeaconKluster::Beacon beacon;
				if (spawnPos)
					beacon.setPosition(cursorPosition);
				else
					beacon.setPosition(camera.position + camera.direction * 2.5f);
				beacon.params = 0xA;
				srvBeacon->addBeacon(spawnSector, klusterIndex, hs.handlerIndex, &beacon);
				UpdateBeaconKlusterBounds(srvBeacon->beaconSectors[spawnSector].beaconKlusters[klusterIndex].get());
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(%02X %02X %02X %02X %02X)", hs.unk2a, hs.numBits, hs.handlerIndex, hs.handlerId, hs.persistent);
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
	auto enumBeaconKluster = [this](CKBeaconKluster* bk) {
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
		CKBeaconKluster *bk = srvBeacon->beaconSectors[selBeaconSector].beaconKlusters[selBeaconKluster].get();
		CKBeaconKluster::Beacon &beacon = bk->bings[selBeaconBing].beacons[selBeaconIndex];

		// find bing + boffi
		CKBeaconKluster::Bing *fndbing = nullptr;
		int boffi, bingIndex = 0, beaconIndex = 0;
		for (CKBeaconKluster::Bing &cing : bk->bings) {
			boffi = cing.bitIndex;
			for (CKBeaconKluster::Beacon &ceacon : cing.beacons) {
				if (&beacon == &ceacon) {
					fndbing = &cing; break;
				}
				boffi += cing.numBits;
				beaconIndex++;
			}
			if (fndbing) break;
			bingIndex++;
			beaconIndex = 0;
		}
		assert(fndbing);
		CKBeaconKluster::Bing &bing = *fndbing;

		if (ImGui::Button("Remove")) {
			removal = true;
			remSector = bing.sectorIndex;
			remKluster = bing.klusterIndex;
			remBing = bingIndex;
			remBeacon = beaconIndex;
		}

		ImGui::Text("%s (%02X, %s)", getBeaconName(bing.handlerId), bing.handlerId, bing.handler->getClassName());
		ImGui::Text("Bits:");
		for (int i = 0; i < bing.numBits; i++) {
			ImGui::SameLine();
			ImGui::Text("%i", srvBeacon->beaconSectors[bing.sectorIndex].bits[boffi + i] ? 1 : 0);
		}
		bool mod = false;
		mod |= ImGui::DragScalarN("Position##beacon", ImGuiDataType_S16, &beacon.posx, 3, 0.1f);
		mod |= ImGui::InputScalar("Params##beacon", ImGuiDataType_U16, &beacon.params, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		if (CKCrateCpnt *crateCpnt = bing.handler->dyncast<CKCrateCpnt>()) {
			ImGui::Separator();
			int cc = beacon.params & 7;
			if (ImGui::InputInt("Num crates", &cc)) {
				beacon.params &= ~7;
				beacon.params |= (cc & 7);
				mod = true;
			}
			cc = (beacon.params >> 3) & 7;
			if (ImGui::InputInt("Num bonuses", &cc)) {
				beacon.params &= ~(7 << 3);
				beacon.params |= (cc & 7) << 3;
				mod = true;
			}
			cc = (beacon.params >> 9) & 3;
			if (ImGui::ListBoxHeader("Bonus", ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing()*4.0f + ImGui::GetStyle().FramePadding.y * 2.0f))) {
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
				ImGui::ListBoxFooter();
			}
		}
		if (mod) {
			if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
				CKSrvBeacon *srvBeacon = kenv.levelObjects.getFirst<CKSrvBeacon>();
				int boff = bing.bitIndex;
				for (auto &beacon2 : bing.beacons) {
					for (int i = 0; i < 6; i++)
						srvBeacon->beaconSectors[bing.sectorIndex].bits[boff++] = beacon2.params & (1 << i);
					srvBeacon->beaconSectors[bing.sectorIndex].bits[boff++] = false;
				}
				assert(boff - bing.bitIndex == bing.numBits * bing.beacons.size());
			}
			UpdateBeaconKlusterBounds(bk);
		}
	}
	ImGui::EndChild();
	ImGui::Columns();

	if (removal) {
		srvBeacon->removeBeacon(remSector, remKluster, remBing, remBeacon);
		selBeaconSector = -1;
		rayHits.clear();
		nearestRayHit = nullptr;
	}
}

void EditorInterface::IGTextureEditor()
{
	static int currentTexDictSector = 0;
	ImGui::InputInt("Sector", &currentTexDictSector);
	int currentTexDict = currentTexDictSector - 1;
	CTextureDictionary *texDict;
	ProTexDict *cur_protexdict;
	if (currentTexDict >= 0 && currentTexDict < (int)kenv.numSectors) {
		texDict = kenv.sectorObjects[currentTexDict].getFirst<CTextureDictionary>();
		cur_protexdict = &str_protexdicts[currentTexDict];
	}
	else {
		texDict = kenv.levelObjects.getObject<CTextureDictionary>(0);
		cur_protexdict = &protexdict;
		currentTexDictSector = 0;
		currentTexDict = -1;
	}
	if (selTexID >= (int)texDict->piDict.textures.size())
		selTexID = (int)texDict->piDict.textures.size() - 1;
	if (ImGui::Button("Insert")) {
		auto filepaths = MultiOpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
		for (const auto& filepath : filepaths) {
			std::string name = filepath.stem().string().substr(0, 31);
			size_t index = texDict->piDict.findTexture(name);
			if (index == -1) {
				RwPITexDict::PITexture& tex = texDict->piDict.textures.emplace_back();
				tex.images.push_back(RwImage::loadFromFile(filepath.c_str()));
				tex.texture.name = name;
				tex.texture.filtering = 2;
				tex.texture.uAddr = 1;
				tex.texture.vAddr = 1;
			}
			else {
				RwPITexDict::PITexture& tex = texDict->piDict.textures[index];
				tex.images.clear();
				tex.images.push_back(RwImage::loadFromFile(filepath.c_str()));
			}
		}
		if (!filepaths.empty())
			cur_protexdict->reset(texDict);
	}
	ImGui::SameLine();
	if ((selTexID != -1) && ImGui::Button("Replace")) {
		auto filepath = OpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
		if (!filepath.empty()) {
			texDict->piDict.textures[selTexID].images = { RwImage::loadFromFile(filepath.c_str()) };
			texDict->piDict.textures[selTexID].nativeVersion.reset();
			cur_protexdict->reset(texDict);
		}
	}
	ImGui::SameLine();
	if ((selTexID != -1) && ImGui::Button("Remove")) {
		texDict->piDict.textures.erase(texDict->piDict.textures.begin() + selTexID);
		cur_protexdict->reset(texDict);
		if (selTexID >= (int)texDict->piDict.textures.size())
			selTexID = -1;
	}
	ImGui::SameLine();
	if ((selTexID != -1) && ImGui::Button("Export")) {
		auto &tex = texDict->piDict.textures[selTexID];
		auto filepath = SaveDialogBox(g_window, "PNG Image\0*.PNG\0\0", "png", tex.texture.name.c_str());
		if (!filepath.empty()) {
			RwImage cimg = tex.images[0].convertToRGBA32();
			FILE* file; fsfopen_s(&file, filepath, "wb");
			auto callback = [](void* context, void* data, int size) {fwrite(data, size, 1, (FILE*)context); };
			stbi_write_png_to_func(callback, file, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
			fclose(file);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Export all")) {
		auto dirname = SelectFolderDialogBox(g_window, "Export all the textures to folder:");
		if (!dirname.empty()) {
			for (auto &tex : texDict->piDict.textures) {
				auto pname = dirname / (std::string(tex.texture.name.c_str()) + ".png");
				RwImage cimg = tex.images[0].convertToRGBA32();
				FILE* file; fsfopen_s(&file, pname, "wb");
				auto callback = [](void* context, void* data, int size) {fwrite(data, size, 1, (FILE*)context); };
				stbi_write_png_to_func(callback, file, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
				fclose(file);
			}
		}
	}
	//ImGui::SameLine();
	//if (ImGui::Button("Invert all")) {
	//	InvertTextures(kenv);
	//	protexdict.reset(kenv.levelObjects.getFirst<CTextureDictionary>());
	//	for (int i = 0; i < (int)str_protexdicts.size(); i++)
	//		str_protexdicts[i].reset(kenv.sectorObjects[i].getFirst<CTextureDictionary>());
	//}
	if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableSetupColumn("TexLeft", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("TexRight", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		static ImGuiTextFilter search;
		search.Draw();
		ImGui::BeginChild("TexSeletion");
		for (int i = 0; i < (int)texDict->piDict.textures.size(); i++) {
			auto& tex = texDict->piDict.textures[i];
			if (!search.PassFilter(tex.texture.name.c_str()))
				continue;
			ImGui::PushID(i);
			if (ImGui::Selectable("##texsel", i == selTexID, 0, ImVec2(0, 32))) {
				selTexID = i;
			}
			if (ImGui::IsItemVisible()) {
				ImGui::SameLine();
				ImGui::Image(cur_protexdict->find(tex.texture.name.c_str()).second, ImVec2(32, 32));
				ImGui::SameLine();
				ImGui::Text("%s\n%i*%i*%i", tex.texture.name.c_str(), tex.images[0].width, tex.images[0].height, tex.images[0].bpp);
			}
			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::TableNextColumn();
		ImGui::BeginChild("TexViewer", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (selTexID != -1) {
			auto& tex = texDict->piDict.textures[selTexID];
			ImGui::Image(cur_protexdict->find(tex.texture.name.c_str()).second, ImVec2((float)tex.images[0].width, (float)tex.images[0].height));
		}
		ImGui::EndChild();
		ImGui::EndTable();
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
	IGObjectDragDropSource(kenv, selNode);
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
					while (wgeo = wgeo->nextGeo.get()) {
						CKAnyGeometry* kgeo = wgeo->duplicateGeo ? wgeo->duplicateGeo.get() : wgeo;
						rwgeo.merge(*kgeo->clump->atomic.geometry);
					}

					RwExtHAnim *hanim = nullptr;
					if (geonode->isSubclassOf<CAnimatedNode>()) {
						RwFrameList* framelist = geonode->cast<CAnimatedNode>()->frameList.get();
						hanim = (RwExtHAnim*)framelist->extensions[0].find(0x11E);
						assert(hanim);
					}

					RwClump clump = CreateClumpFromGeo(sharedRwgeo, hanim);

					IOFile dff(filepath.c_str(), "wb");
					clump.serialize(&dff);
				}
			}
			CKAnyGeometry *kgeo = geonode->geometry.get();
			if (geonode->geometry->flags & 8192) {
				static const char * const costumeNames[4] = { "Gaul", "Roman", "Pirate", "Swimsuit" };
				geonode->geometry->costumes;
				if (ImGui::ListBoxHeader("Costume", ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 5.0f))) {
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
					ImGui::ListBoxFooter();
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
					IGObjectDragDropSource(kenv, gnd.get());
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
		ImGui::Separator();
		CheckboxFlags16("Bouncing", &selGround->param1, 1);
		CheckboxFlags16("Death", &selGround->param1, 4);
		CheckboxFlags16("Slide", &selGround->param1, 8);
		CheckboxFlags16("Hurt 1", &selGround->param1, 0x10);
		CheckboxFlags16("Hurt 2", &selGround->param1, 0x20);
		CheckboxFlags16("Hurt 3", &selGround->param1, 0x40);
		ImGui::Separator();
		CheckboxFlags16("Walkable", &selGround->param2, 1);
		CheckboxFlags16("Below water", &selGround->param2, 2);
		CheckboxFlags16("Ceiling", &selGround->param2, 8);
		CheckboxFlags16("High grass", &selGround->param2, 0x20);
		CheckboxFlags16("???", &selGround->param2, 0x80);

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
						ImGui::Text("Event sequence %i", seqid);
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
					ImGui::BulletText("%i: %s (%i, %02X)\nUsed by %s", srvEvent->evtSeqIDs[i], srvEvent->evtSeqNames[i].c_str(), bee.numActions, bee.bitMask, bee.users.size() ? bee.users[0]->getClassName() : "?");
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
		for (uint8_t i = 0; i < bee.numActions; i++) {
			ImGui::PushID(ev + i);

			if (srvEvent->objs[ev + i].bound) {
				ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
				IGObjectSelectorRef(kenv, "##evtTargetObj", srvEvent->objs[ev + i].ref);
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
				srvEvent->objs.erase(srvEvent->objs.begin() + ev + i);
				srvEvent->objInfos.erase(srvEvent->objInfos.begin() + ev + i);
				bee.numActions -= 1;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Remove");

			ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
			IGEventMessageSelector("##EventMessage", srvEvent->objInfos[ev + i], srvEvent->objs[ev + i].ref.get());

			ImGui::PopID();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
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
		IGObjectDragDropSource(kenv, squad);
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
				MemberListener& gml = ml;
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
				gml.reflect(squad->sqBizObj1, "sqBizObj1");
				gml.reflect(squad->sqBizMarker1, "sqBizMarker1");
				gml.reflect(squad->sqBizObj2, "sqBizObj2");
				gml.reflect(squad->sqBizMarker2, "sqBizMarker2");
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
							IGMarkerSelector("Marker Index", pnt.markerIndex);
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
				if (ImGui::ListBoxHeader("##StateList", ImVec2(0.0f, 80.0f))) {
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
					ImGui::ListBoxFooter();
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
						ImGui::Text("Message %s", g_encyclo.getEventName(g_encyclo.getEventJson(squad->getClassFullID(), b.event), b.event).c_str());
						if (hopen) {
							ImGui::Indent();
							uint16_t msg = (uint16_t)b.event;
							if (IGEventMessageSelector("Message", msg, squad, true))
								b.event = msg;

							if (ImGui::Button("New action"))
								b.actions.emplace_back().num = 14;
							for (auto &c : b.actions) {
								//if (&c != &b.actions.front())
								ImGui::Separator();
								ImGui::PushID(&c);
								ImGui::PushItemWidth(ImGui::CalcItemWidth() - 48.0f);
								ImGui::SetNextItemWidth(48.0f);
								ImGui::InputScalar("##Action Num", ImGuiDataType_U8, &c.num);
								ImGui::SameLine();
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
												}
											}
										}
									}
									ImGui::EndCombo();
								}
								int i = 0;
								for (auto &d : c.parameters) {
									const nlohmann::json* paramJson = getParameterJson(c.num, i);
									const std::string& paramName = paramJson->at("name").get_ref<const std::string&>();
									const char* tbuf = paramName.c_str();
									ImGui::PushID(&d);
									ImGui::SetNextItemWidth(48.0f);
									int type = (int)d.index();
									if (ImGui::Combo("##Type", &type, "I0\0I1\0Flt\0Ref\0Mark"))
										changeVariantType(d, type);
									ImGui::SameLine();
									switch (d.index()) {
									case 0: {
										uint32_t& num0 = std::get<0>(d);
										if (auto it = paramJson->find("content"); it != paramJson->end() && it->get_ref<const std::string&>() == "event") {
											uint16_t msg = (uint16_t)num0;
											if (IGEventMessageSelector(tbuf, msg, GameX1::CKHkEnemy::FULL_ID))
												num0 = msg;
										}
										else {
											ImGui::InputInt(tbuf, (int*)&num0);
										}
										break;
									}
									case 1: {
										uint32_t& num1 = std::get<1>(d);
										const char** comboNames = nullptr; int comboSize = 0;
										if (auto it = paramJson->find("content"); it != paramJson->end()) {
											const std::string& content = it->get_ref<const std::string&>();
											if (content == "comparatorInt") {
												static const char* cmpIntNames[] = { "==", "!=", "<", "<=", ">", ">=" };
												comboNames = cmpIntNames;
												comboSize = 6;
											}
											else if (content == "comparatorFloat") {
												static const char* cmpFltNames[] = { "<", ">" };
												comboNames = cmpFltNames;
												comboSize = 2;
											}
										}
										if (comboNames) {
											ImGui::Combo(tbuf, (int*)&num1, comboNames, comboSize);
										}
										else {
											ImGui::InputInt(tbuf, (int*)&num1);
										}
										break;
									}
									case 2:
										ImGui::InputFloat(tbuf, &std::get<2>(d)); break;
									case 3:
										IGObjectSelectorRef(kenv, tbuf, std::get<kobjref<CKObject>>(d)); break;
									case 4:
										IGMarkerSelector(tbuf, std::get<MarkerIndex>(d)); break;
									}
									ImGui::PopID();
									++i;
								}
								ImGui::PopItemWidth();
								if (ImGui::SmallButton("New parameter"))
									c.parameters.emplace_back();
								ImGui::SameLine();
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
				if (ImGui::ListBoxHeader("##ChoreoList")) {
					int cindex = 0;
					int kindex = 0;
					for (auto& choreo : squad->choreographies) {
						ImGui::PushID(choreo.get());
						ImGui::BulletText("Choreo %i: %s", cindex, kenv.getObjectName(choreo.get()));
						ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - ImGui::GetTextLineHeightWithSpacing());
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
					ImGui::ListBoxFooter();
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
					ImGui::DragFloat("Duration", &ckey->unk1);
					ImGui::DragFloat("Unk2", &ckey->unk2);
					ImGui::DragFloat("Unk3", &ckey->unk3);
					ImGui::InputInt("Default Pool", &defaultpool);
					
					if (ImGui::Button("Flags")) {
						ImGui::OpenPopup("ChoreoKey flags");
					}
					if (ImGui::BeginPopup("ChoreoKey flags")) {
						unsigned int iflags = (int)ckey->flags;
						ImGui::CheckboxFlags("Don't rotate around player", &iflags, 1<<0);
						ImGui::CheckboxFlags("Bit 2", &iflags, 1<<1);
						ImGui::CheckboxFlags("Bit 3", &iflags, 1<<2);
						ImGui::CheckboxFlags("Formations always have spears out", &iflags, 1<<3);
						ImGui::CheckboxFlags("Look at player", &iflags, 1<<4);
						ImGui::CheckboxFlags("Bit 6", &iflags, 1<<5);
						ImGui::CheckboxFlags("Enemies can run", &iflags, 1<<6);
						ImGui::CheckboxFlags("Bit 8", &iflags, 1<<7);
						ckey->flags = (uint16_t)iflags;
						ImGui::EndPopup();
					}
					ImGui::SameLine();
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
				if (ImGui::ListBoxHeader("##PoolList")) {
					for (int i = 0; i < (int)squad->pools.size(); i++) {
						ImGui::PushID(i);
						if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
							currentPoolInput = i;
						ImGui::SameLine();
						auto &pe = squad->pools[i];
						ImGui::Text("%s %u %u", pe.cpnt->getClassName(), pe.numEnemies, pe.u1);
						ImGui::PopID();
					}
					ImGui::ListBoxFooter();
				}
				if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
					auto &pe = squad->pools[currentPoolInput];
					ImGui::BeginChild("SquadPools");
					ImGui::BulletText("%s %u %u %u", pe.cpnt->getClassName(), pe.u1, pe.u2, pe.u3.get() ? 1 : 0);
					IGObjectSelectorRef(kenv, "Pool", pe.pool);
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
		if (ImGui::ListBoxHeader("##PoolList")) {
			for (int i = 0; i < (int)fightData.pools.size(); i++) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
					currentPoolInput = i;
				ImGui::SameLine();
				auto& pe = fightData.pools[i];
				ImGui::Text("%s Cpnt%u %u", kenv.getObjectName(pe.pool.get()), pe.componentIndex, pe.numEnemies);
				ImGui::PopID();
			}
			ImGui::ListBoxFooter();
		}
		if (currentPoolInput >= 0 && currentPoolInput < fightData.pools.size()) {
			auto& pe = fightData.pools[currentPoolInput];
			ImGui::BeginChild("SquadPools");
			//ImGui::BulletText("%s %u %u", "TODO", pe.u1, pe.u2);
			IGObjectSelectorRef(kenv, "Pool", pe.pool);
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
	IGObjectDragDropSource(kenv, group);
	if (ImGui::IsItemClicked()) {
		selectedGroup = group;
		viewGroupInsteadOfHook = true;
	}
	if (gopen) {
		IGEnumGroup(group->childGroup.get());
		for (CKHook *hook = group->childHook.get(); hook; hook = hook->next.get()) {
			bool b = ImGui::TreeNodeEx(hook, ImGuiTreeNodeFlags_Leaf | ((selectedHook == hook && !viewGroupInsteadOfHook) ? ImGuiTreeNodeFlags_Selected : 0), "%s %s", hook->getClassName(), kenv.getObjectName(hook));
			IGObjectDragDropSource(kenv, hook);
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
		IGObjectDragDropSource(kenv, selectedHook);
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
			ImGui::Separator();
		}
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
		if (CKSceneNode* sn = selectedHook->node.get()) {
			if (ImGui::Button("Where is it?")) {
				selNode = sn;
				camera.position = sn->getGlobalMatrix().getTranslationVector() - camera.direction * 5.0f;
			}
		}

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

void EditorInterface::IGPathfindingEditor()
{
	CKSrvPathFinding *srvpf = kenv.levelObjects.getFirst<CKSrvPathFinding>();
	if (!srvpf) return;
	if (ImGui::Button("New PF node")) {
		CKPFGraphNode *pfnode = kenv.createObject<CKPFGraphNode>(-1);
		srvpf->nodes.emplace_back(pfnode);
		pfnode->numCellsX = 20;
		pfnode->numCellsZ = 20;
		pfnode->cells = std::vector<uint8_t>(pfnode->numCellsX * pfnode->numCellsZ, 1);
		pfnode->lowBBCorner = cursorPosition;
		pfnode->highBBCorner = pfnode->lowBBCorner + Vector3((float)pfnode->numCellsX * 2.0f, 50.0f, (float)pfnode->numCellsZ * 2.0f);
	}

	ImGui::Columns(2);
	ImGui::BeginChild("PFNodeList");
	int nid = 0;
	for (auto &pfnode : srvpf->nodes) {
		ImGui::PushID(&pfnode);
		if (ImGui::Selectable("##PFNodeEntry", selectedPFGraphNode == pfnode.get())) {
			selectedPFGraphNode = pfnode.get();
		}
		ImGui::SameLine();
		ImGui::Text("Graph node %i (%u*%u)", nid, pfnode->numCellsX, pfnode->numCellsZ);
		ImGui::PopID();
		nid++;
	}
	ImGui::EndChild();

	ImGui::NextColumn();
	ImGui::BeginChild("PFNodeInfo");
	if (CKPFGraphNode *pfnode = selectedPFGraphNode.get()) {
		float oldcw = pfnode->getCellWidth();
		float oldch = pfnode->getCellHeight();
		if (ImGui::DragFloat3("BB Low", &pfnode->lowBBCorner.x, 0.1f)) {
			pfnode->highBBCorner = pfnode->lowBBCorner + Vector3(pfnode->numCellsX * oldcw, 50, pfnode->numCellsZ * oldch);
		}
		//ImGui::DragFloat3("BB High", &pfnode->highBBCorner.x, 0.1f);
		if (ImGui::Button("Place camera there")) {
			camera.position.x = (pfnode->lowBBCorner.x + pfnode->highBBCorner.x) * 0.5f;
			camera.position.z = (pfnode->lowBBCorner.z + pfnode->highBBCorner.z) * 0.5f;
		}

		int tid = 0;
		for (auto &pftrans : pfnode->transitions) {
			if (ImGui::TreeNode(&pftrans, "Transition %i", tid)) {
				for (auto &thing : pftrans->things) {
					ImGui::Bullet();
					ImGui::Indent();
					for (int i = 0; i < 12; i += 3)
						ImGui::Text("%f %f %f", thing.matrix[i], thing.matrix[i + 1], thing.matrix[i + 2]);
					ImGui::Text("%i", thing.unk);
					ImGui::Unindent();
				}
				ImGui::TreePop();
			}
			tid++;
		}

		ImGui::Text("Grid size: %u * %u", pfnode->numCellsX, pfnode->numCellsZ);
		ImGui::Text("Cell size: %f * %f", pfnode->getCellWidth(), pfnode->getCellHeight());

		static uint8_t resizeX, resizeZ;
		static float recellWidth, recellHeight;
		if (ImGui::Button("Resize")) {
			ImGui::OpenPopup("PFGridResize");
			resizeX = pfnode->numCellsX;
			resizeZ = pfnode->numCellsZ;
			recellWidth = pfnode->getCellWidth();
			recellHeight = pfnode->getCellHeight();
		}
		if (ImGui::BeginPopup("PFGridResize")) {
			ImGui::InputScalar("Grid Width", ImGuiDataType_U8, &resizeX);
			ImGui::InputScalar("Grid Height", ImGuiDataType_U8, &resizeZ);
			ImGui::InputFloat("Cell Width", &recellWidth);
			ImGui::InputFloat("Cell Height", &recellHeight);
			if (ImGui::Button("OK")) {
				std::vector<uint8_t> res(resizeX * resizeZ, 1);
				int cx = std::min(pfnode->numCellsX, resizeX);
				int cz = std::min(pfnode->numCellsZ, resizeZ);
				for (uint8_t x = 0; x < cx; x++)
					for (uint8_t z = 0; z < cz; z++)
						res[z*resizeX + x] = pfnode->cells[z*pfnode->numCellsX + x];
				pfnode->numCellsX = resizeX;
				pfnode->numCellsZ = resizeZ;
				pfnode->cells = res;
				pfnode->highBBCorner = pfnode->lowBBCorner + Vector3(recellWidth * resizeX, 50, recellHeight * resizeZ);
			}
			ImGui::EndPopup();
		}

		static uint8_t paintval = 7;
		for (uint8_t val : {1, 4, 7, 0}) {
			char buf[8];
			sprintf_s(buf, "%X", val);
			ImGui::PushStyleColor(ImGuiCol_Text, getPFCellColor(val));
			if (ImGui::Button(buf))
				paintval = val;
			ImGui::PopStyleColor();
			ImGui::SameLine();
		}
		//ImGui::SameLine();
		ImGui::InputScalar("Value", ImGuiDataType_U8, &paintval);
		paintval &= 15;

		int c = 0;
		ImGui::BeginChild("PFGrid", ImVec2(0, 16 * 0), true, ImGuiWindowFlags_NoMove);
		for (int y = 0; y < pfnode->numCellsZ; y++) {
			for (int x = 0; x < pfnode->numCellsX; x++) {
				uint8_t &val = pfnode->cells[c++];
				ImVec4 color = getPFCellColor(val);

				ImGui::TextColored(color, "%X", val);
				//ImGui::Image(nullptr, ImVec2(8, 8), ImVec2(0, 0), ImVec2(0, 0), color);
				//ImVec2 curpos = ImGui::GetCursorScreenPos();
				//ImGui::GetWindowDrawList()->AddRectFilled(curpos, ImVec2(curpos.x + 8, curpos.y + 8), ImGui::GetColorU32(color));
				//ImGui::Dummy(ImVec2(8, 8));

				if (ImGui::IsItemHovered()) {
					if (ImGui::IsMouseDown(0))
						val = paintval;
					else if (ImGui::IsMouseDown(1))
						paintval = val;
				}
				ImGui::SameLine();
			}
			ImGui::NewLine();
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGMarkerEditor()
{
	CKSrvMarker *marker = kenv.levelObjects.getFirst<CKSrvMarker>();
	if (!marker) return;
	ImGui::Columns(2);
	ImGui::BeginChild("MarkerTree");
	int lx = 0;
	for (auto &list : marker->lists) {
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode(&list, "List %i", lx)) {
			if (ImGui::Button("Add")) {
				auto& marker = list.emplace_back();
				marker.position = cursorPosition;
			}
			int mx = 0;
			for (auto &marker : list) {
				ImGui::PushID(&marker);
				if (ImGui::Selectable("##MarkerEntry", selectedMarker == &marker)) {
					selectedMarker = &marker;
				}
				ImGui::SameLine();
				ImGui::Text("%3i: %s (%u)", mx, marker.name.c_str(), marker.val3);
				ImGui::PopID();
				mx++;
			}
			ImGui::TreePop();
		}
		lx++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("MarkerInfo");
	if (selectedMarker) {
		CKSrvMarker::Marker *marker = (CKSrvMarker::Marker*)selectedMarker;
		if (ImGui::Button("Place camera there")) {
			camera.position = marker->position - camera.direction * 5.0f;
		}
		IGStringInput("Name", marker->name);
		ImGui::DragFloat3("Position", &marker->position.x, 0.1f);
		ImGui::InputScalar("Orientation 1", ImGuiDataType_U8, &marker->orientation1);
		ImGui::InputScalar("Orientation 2", ImGuiDataType_U8, &marker->orientation2);
		ImGui::InputScalar("Val3", ImGuiDataType_U16, &marker->val3);
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGDetectorEditor()
{
	CKSrvDetector *srvDetector = kenv.levelObjects.getFirst<CKSrvDetector>();
	if (!srvDetector) return;
	auto coloredTreeNode = [](const char *label, const ImVec4 &color = ImVec4(1,1,1,1)) {
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		bool res = ImGui::TreeNode(label);
		ImGui::PopStyleColor();
		return res;
	};
	auto enumdctlist = [this, &coloredTreeNode](std::vector<CKSrvDetector::Detector>& dctlist, const char* name, const ImVec4& color = ImVec4(1, 1, 1, 1), bool isInsideDetector = false, int filterShape = -1) {
		if (filterShape != -1)
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (coloredTreeNode(name, color)) {
			
			for (size_t i = 0; i < dctlist.size(); ++i) {
				auto& dct = dctlist[i];
				if (filterShape != -1 && (int)dct.shapeIndex != filterShape)
					continue;
				ImGui::PushID(&dct);
				ImGui::BulletText("#%zi", i);
				if (filterShape == -1)
					ImGui::InputScalar("Shape index", ImGuiDataType_U16, &dct.shapeIndex);
				ImGui::InputScalar("Node index", ImGuiDataType_U16, &dct.nodeIndex);
				ImGui::InputScalar("Flags", ImGuiDataType_U16, &dct.flags, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
				unsigned int modflags = dct.flags;
				bool flagsModified = false;
				flagsModified |= ImGui::CheckboxFlags("Enabled on start", &modflags, 2);
				modflags = (modflags & ~3) | ((modflags & 2) ? 3 : 0);
				if (flagsModified)
					dct.flags = (uint16_t)modflags;
				IGEventSelector("Event sequence", dct.eventSeqIndex);
				ImGui::Separator();
				ImGui::PopID();
			}
			if (ImGui::Button("New")) {
				dctlist.emplace_back();
				dctlist.back().flags = isInsideDetector ? 0x0133 : 0x010F;
				if (filterShape != -1)
					dctlist.back().shapeIndex = filterShape;
			}

			ImGui::TreePop();
		}
	};

	Vector3 creationPosition = cursorPosition;

	if (ImGui::BeginTabBar("DetectorTabBar")) {
		if (ImGui::BeginTabItem("Shapes")) {
			ImGui::Columns(2);
			ImGui::BeginChild("DctShapesList");
			if (coloredTreeNode("Bounding boxes", ImVec4(0, 1, 0, 1))) {
				int i = 0;
				for (auto& bb : srvDetector->aaBoundingBoxes) {
					bool selected = selectedShapeType == 0 && selectedShapeIndex == i;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0);
					if (ImGui::TreeNodeEx(&bb, flags, "%3i: %s (%.2f,%.2f,%.2f) - (%.2f,%.2f,%.2f)", i, srvDetector->aabbNames[i].c_str(), bb.highCorner.x, bb.highCorner.y, bb.highCorner.z, bb.lowCorner.x, bb.lowCorner.y, bb.lowCorner.z))
						ImGui::TreePop();
					if (ImGui::IsItemActivated()) {
						selectedShapeType = 0; selectedShapeIndex = i;
					}
					++i;
				}
				if (ImGui::Button("New")) {
					srvDetector->aaBoundingBoxes.emplace_back(creationPosition + Vector3(1, 1, 1), creationPosition - Vector3(1, 1, 1));
					srvDetector->aabbNames.emplace_back("New bounding box");
				}
				ImGui::TreePop();
			}
			if (coloredTreeNode("Spheres", ImVec4(1, 0.5f, 0, 1))) {
				int i = 0;
				for (auto& sph : srvDetector->spheres) {
					bool selected = selectedShapeType == 1 && selectedShapeIndex == i;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0);
					if (ImGui::TreeNodeEx(&sph, flags, "%3i: %s c=(%.2f,%.2f,%.2f) r=%f", i, srvDetector->sphNames[i].c_str(), sph.center.x, sph.center.y, sph.center.z, sph.radius))
						ImGui::TreePop();
					if (ImGui::IsItemActivated()) {
						selectedShapeType = 1; selectedShapeIndex = i;
					}
					++i;
				}
				if (ImGui::Button("New")) {
					srvDetector->spheres.emplace_back(creationPosition, 1.0f);
					srvDetector->sphNames.emplace_back("New sphere");
				}
				ImGui::TreePop();
			}
			if (coloredTreeNode("Rectangles", ImVec4(1, 0, 1, 1))) {
				int i = 0;
				for (auto& rect : srvDetector->rectangles) {
					bool selected = selectedShapeType == 2 && selectedShapeIndex == i;
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | (selected ? ImGuiTreeNodeFlags_Selected : 0);
					if (ImGui::TreeNodeEx(&rect, flags, "%3i: %s c=(%.2f,%.2f,%.2f) dir=%i", i, srvDetector->rectNames[i].c_str(), rect.center.x, rect.center.y, rect.center.z, rect.direction))
						ImGui::TreePop();
					if (ImGui::IsItemActivated()) {
						selectedShapeType = 2; selectedShapeIndex = i;
					}
					++i;
				}
				if (ImGui::Button("New")) {
					srvDetector->rectangles.emplace_back();
					srvDetector->rectangles.back().center = creationPosition;
					srvDetector->rectNames.emplace_back("New rectangle");
				}
				ImGui::TreePop();
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("DctShapeInfo");
			if (selectedShapeType == 0 && selectedShapeIndex >= 0 && selectedShapeIndex < srvDetector->aaBoundingBoxes.size()) {
				auto& bb = srvDetector->aaBoundingBoxes[selectedShapeIndex];
				IGStringInput("Name", srvDetector->aabbNames[selectedShapeIndex]);
				ImGui::DragFloat3("High corner", &bb.highCorner.x, 0.1f);
				ImGui::DragFloat3("Low corner", &bb.lowCorner.x, 0.1f);
				if (ImGui::Button("See OvO"))
					camera.position = Vector3(bb.highCorner.x, camera.position.y, bb.highCorner.z);
				enumdctlist(srvDetector->aDetectors, "On Enter Box", ImVec4(0, 1, 0, 1), false, selectedShapeIndex);
				enumdctlist(srvDetector->dDetectors, "While Inside Box", ImVec4(0, 1, 0, 1), true, selectedShapeIndex);
			}
			else if (selectedShapeType == 1 && selectedShapeIndex >= 0 && selectedShapeIndex < srvDetector->spheres.size()) {
				auto& sph = srvDetector->spheres[selectedShapeIndex];
				IGStringInput("Name", srvDetector->sphNames[selectedShapeIndex]);
				ImGui::DragFloat3("Center", &sph.center.x, 0.1f);
				ImGui::DragFloat("Radius", &sph.radius, 0.1f);
				if (ImGui::Button("See OvO"))
					camera.position = Vector3(sph.center.x, camera.position.y, sph.center.z);
				enumdctlist(srvDetector->bDetectors, "On Enter Sphere", ImVec4(1, 0.5f, 0, 1), false, selectedShapeIndex);
				enumdctlist(srvDetector->eDetectors, "While Inside Sphere", ImVec4(1, 0.5f, 0, 1), true, selectedShapeIndex);
			}
			else if (selectedShapeType == 2 && selectedShapeIndex >= 0 && selectedShapeIndex < srvDetector->rectangles.size()) {
				auto& rect = srvDetector->rectangles[selectedShapeIndex];
				IGStringInput("Name", srvDetector->rectNames[selectedShapeIndex]);
				ImGui::DragFloat3("Center", &rect.center.x, 0.1f);
				ImGui::DragFloat("Length 1", &rect.length1, 0.1f);
				ImGui::DragFloat("Length 2", &rect.length2, 0.1f);
				ImGui::InputScalar("Direction", ImGuiDataType_U8, &rect.direction);
				if (ImGui::Button("See OvO"))
					camera.position = Vector3(rect.center.x, camera.position.y, rect.center.z);
				enumdctlist(srvDetector->cDetectors, "On Cross Rectangle", ImVec4(1, 0, 1, 1), false, selectedShapeIndex);
			}
			ImGui::EndChild();
			ImGui::Columns(1);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Checklist")) {
			ImGui::BeginChild("DetectorChecklist");
			ImGui::PushID("checklist");
			enumdctlist(srvDetector->aDetectors, "Entering Bounding boxes", ImVec4(0, 1, 0, 1), false);
			enumdctlist(srvDetector->bDetectors, "Entering Spheres", ImVec4(1, 0.5f, 0, 1), false);
			enumdctlist(srvDetector->cDetectors, "Crossing Rectangles", ImVec4(1, 0, 1, 1), false);
			enumdctlist(srvDetector->dDetectors, "Being in Bounding boxes", ImVec4(0, 1, 0, 1), true);
			enumdctlist(srvDetector->eDetectors, "Being in Spheres", ImVec4(1, 0.5f, 0, 1), true);
			ImGui::PopID();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Scene Nodes")) {
			ImGui::BeginChild("DetectorSceneNodes");
			int i = 0;
			ImGui::PushItemWidth(-32.0f);
			for (auto& node : srvDetector->nodes) {
				IGObjectSelectorRef(kenv, std::to_string(i++).c_str(), node);
			}
			ImGui::PopItemWidth();
			if (ImGui::Button("New")) {
				srvDetector->nodes.emplace_back();
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void EditorInterface::IGCinematicEditor()
{
	CKSrvCinematic *srvCine = kenv.levelObjects.getFirst<CKSrvCinematic>();
	static int selectedCinematicSceneIndex = -1;
	static KWeakRef<CKCinematicNode> selectedCineNode;
	auto getSceneName = [this,srvCine](int index) -> std::string {
		if(index >= 0 && index < (int)srvCine->cineScenes.size())
			return std::to_string(index) + ": " + kenv.getObjectName(srvCine->cineScenes[index].get());
		return "?";
	};
	if (ImGui::BeginCombo("Cinematic Scene", getSceneName(selectedCinematicSceneIndex).c_str())) {
		for (int i = 0; i < (int)srvCine->cineScenes.size(); ++i)
			if (ImGui::Selectable(getSceneName(i).c_str()))
				selectedCinematicSceneIndex = i;
		ImGui::EndCombo();
	}
	if (ImGui::Button("New scene")) {
		CKCinematicScene* newScene = kenv.createAndInitObject<CKCinematicScene>();
		srvCine->cineScenes.emplace_back(newScene);
		selectedCinematicSceneIndex = (int)srvCine->cineScenes.size() - 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("Corrupt")) {
		for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
			for (CKObject* obj : cncls.objects) {
				CKCinematicNode* knode = obj->cast<CKCinematicNode>();
				if (knode->isSubclassOf<CKCinematicDoor>())
					knode->cast<CKCinematicDoor>()->cnFlags &= 0xFA;
				else
					knode->cast<CKCinematicBloc>()->cnFlags &= 0xFFFA;
			}
		}
	}
	if (selectedCinematicSceneIndex >= 0 && selectedCinematicSceneIndex < (int)srvCine->cineScenes.size()) {
		CKCinematicScene *scene = srvCine->cineScenes[selectedCinematicSceneIndex].get();

		ImGui::SameLine();
		if (ImGui::Button("Export TGF")) {
			auto filename = SaveDialogBox(g_window, "Trivial Graph Format (*.tgf)\0*.TGF\0", "tgf");
			if (!filename.empty()) {
				std::map<CKCinematicNode*, int> gfNodes;
				std::map<std::array<CKCinematicNode*, 2>, std::string> gfEdges;
				int nextNodeId = 1;

				// Find all nodes from the scene
				for (auto &cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject *obj : cncls.objects) {
						CKCinematicNode *knode = obj->cast<CKCinematicNode>();
						if (knode->cnScene.get() == scene)
							gfNodes.insert({ knode, nextNodeId++ });
					}
				}

				// Constructing edges
				std::function<void(CKCinematicNode *node)> visit;
				visit = [&scene, &gfNodes, &gfEdges, &visit](CKCinematicNode *node) {
					uint16_t start = (node->cnStartOutEdge != 0xFFFF) ? node->cnStartOutEdge : node->cnFinishOutEdge;
					for (int i = 0; i < node->cnNumOutEdges; i++) {
						CKCinematicNode *subnode = scene->cineNodes[start + i].get();
						gfEdges[{ node, subnode }].append(std::to_string(i)).append(",");
					}
					if (node->cnFinishOutEdge != 0xFFFF) {
						for (int i = node->cnFinishOutEdge; i < start + node->cnNumOutEdges; ++i) {
							CKCinematicNode* subnode = scene->cineNodes[i].get();
							gfEdges[{ node, subnode }].append("cond,");
						}
					}
					if (CKGroupBlocCinematicBloc* grpbloc = node->dyncast<CKGroupBlocCinematicBloc>()) {
						gfEdges[{node, grpbloc->gbFirstNode.get()}].append("grpHead,");
					}
				};
				//visit(scene->startDoor.get());
				for (auto &gnode : gfNodes) {
					visit(gnode.first);
				}

				//// Node labeling
				//for (auto &edge : gfEdges) {
				//	for (CKCinematicNode *node : edge.first) {
				//		if (gfNodes[node] == 0)
				//			gfNodes[node] = nextNodeId++;
				//	}
				//}

				FILE *tgf;
				fsfopen_s(&tgf, filename, "wt");
				for (auto &gnode : gfNodes) {
					fprintf(tgf, "%i %s (%p)\n", gnode.second, gnode.first->getClassName(), gnode.first);
				}
				fprintf(tgf, "#\n");
				for (auto &edge : gfEdges) {
					fprintf(tgf, "%i %i %s\n", gfNodes[edge.first[0]], gfNodes[edge.first[1]], edge.second.c_str());
				}
				fclose(tgf);
			}
		}

		ImGui::Columns(2);

		if (ImGui::BeginTabBar("GraphBar")) {
			if (ImGui::BeginTabItem("Edge list")) {
				ImGui::BeginChild("CineNodes");

				bool b = ImGui::TreeNodeEx("Start door", ImGuiTreeNodeFlags_Leaf | ((selectedCineNode == scene->startDoor.get()) ? ImGuiTreeNodeFlags_Selected : 0));
				if (ImGui::IsItemClicked()) {
					selectedCineNode = scene->startDoor.get();
				}
				if (b) ImGui::TreePop();

				struct CineNodeEnumerator {
					static void enumNode(CKCinematicNode* node, int i) {
						bool isGroup = node->isSubclassOf<CKGroupBlocCinematicBloc>();
						ImGuiTreeNodeFlags tflags = 0;
						if (selectedCineNode == node) tflags |= ImGuiTreeNodeFlags_Selected;
						if (!isGroup) tflags |= ImGuiTreeNodeFlags_Leaf;
						bool b = ImGui::TreeNodeEx(node, tflags, "%i: %s", i++, node->getClassName());
						if (ImGui::IsItemClicked()) {
							selectedCineNode = node;
						}
						if (b) {
							if (isGroup) {
								int i = 0;
								for (auto& sub : node->cast<CKGroupBlocCinematicBloc>()->gbSubnodes)
									enumNode(sub.get(), i++);
							}
							ImGui::TreePop();
						}
					}
				};

				int i = 0;
				for (auto& node : scene->cineNodes) {
					CineNodeEnumerator::enumNode(node.get(), i++);
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Block graph", nullptr, ImGuiTabItemFlags_Leading)) {
				static ImNodesEditorContext* cineNodesContext = nullptr;
				if (!cineNodesContext) {
					InitImNodes();
					cineNodesContext = ImNodes::EditorContextCreate();
				}
				ImNodes::EditorContextSet(cineNodesContext);

				static int ImNodesCurrentScene = -1;
				static std::unordered_map<CKCinematicNode*, int> blockIdMap;
				static std::unordered_map<int, CKCinematicNode*> idBlockMap;
				static int nextId = 0;
				
				if (ImNodesCurrentScene != selectedCinematicSceneIndex) {
					ImNodes::ClearNodeSelection();
					ImNodes::ClearLinkSelection();
					
					// Compute the layout
					std::set<CKCinematicNode*> reachedSet;
					std::map<CKGroupBlocCinematicBloc*, std::vector<std::vector<CKCinematicNode*>>> layout;
					auto visit = [scene,&reachedSet,&layout](CKCinematicNode* node, int depth, const auto& rec) -> void {
						if (reachedSet.count(node))
							return;
						reachedSet.insert(node);
						
						auto& layoutGroup = layout[node->cnGroupBloc.get()];
						if (depth >= (int)layoutGroup.size())
							layoutGroup.resize(depth + 1);
						layoutGroup[depth].push_back(node);

						uint16_t start = (node->cnStartOutEdge != 0xFFFF) ? node->cnStartOutEdge : node->cnFinishOutEdge;
						uint16_t end = (node->cnFinishOutEdge != 0xFFFF) ? node->cnFinishOutEdge : (start + node->cnNumOutEdges);
						for (int i = start; i < end; i++) {
							CKCinematicNode* subnode = scene->cineNodes[i].get();
							rec(subnode, depth + 1, rec);
						}
					};
					
					// Visit from the Start Door, as well as Group Starting Node
					visit(scene->startDoor.get(), 0, visit);
					for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
						for (CKObject* obj : cncls.objects) {
							if (CKGroupBlocCinematicBloc* group = obj->dyncast<CKGroupBlocCinematicBloc>()) {
								if (group->cnScene.get() == scene) {
									visit(group->gbFirstNode.get(), 0, visit);
								}
							}
						}
					}

					for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
						for (CKObject* obj : cncls.objects) {
							CKCinematicNode* knode = obj->cast<CKCinematicNode>();
							if (knode->cnScene.get() == scene) {
								int id;
								if (auto it = blockIdMap.find(knode); it != blockIdMap.end()) {
									id = it->second;
								}
								else {
									blockIdMap.insert_or_assign(knode, nextId);
									idBlockMap.insert_or_assign(nextId, knode);
									id = nextId;
									++nextId;
								}
							}
						}
					}
					static constexpr float nodeWidth = 240.0f;
					static constexpr float nodeHeight = 150.0f;
					std::vector<int> groupPosY(layout.size(), 0);
					size_t lg = 0;
					for (auto& [group, layoutGroup] : layout) {
						int rows = 0;
						for (int x = 0; x < (int)layoutGroup.size(); ++x) {
							rows = std::max(rows, (int)layoutGroup[x].size());
						}

						groupPosY[lg] = rows;
						if (lg > 0)
							groupPosY[lg] += groupPosY[lg - 1];
						lg += 1;
					}
					lg = 0;
					for (auto& [group, layoutGroup] : layout) {
						int startRow = (lg > 0) ? groupPosY[lg - 1] : 0;
						lg += 1;
						for (int x = 0; x < (int)layoutGroup.size(); ++x) {
							for (int y = 0; y < (int)layoutGroup[x].size(); ++y) {
								ImNodes::SetNodeGridSpacePos(blockIdMap.at(layoutGroup[x][y]), ImVec2(x * nodeWidth, (startRow + y) * nodeHeight));
							}
						}
					}
					ImNodesCurrentScene = selectedCinematicSceneIndex;
				}
				if (ImGui::Button("Add node")) {
					ImGui::OpenPopup("AddCineNode");
				}
				if (ImGui::BeginPopup("AddCineNode")) {
					int toadd = -1;
					auto door = [&toadd](int id, const char* name) {
						if (ImGui::Selectable(name))
							toadd = id;
					};
					door(CKLogicalAnd::FULL_ID, "CKLogicalAnd");
					door(CKLogicalOr::FULL_ID, "CKLogicalOr");
					if (kenv.version == KEnvironment::KVERSION_XXL1)
						door(CKRandLogicalDoor::FULL_ID, "CKRandLogicalDoor");
					if (kenv.version >= KEnvironment::KVERSION_XXL2 || kenv.isRemaster)
						door(CKEndDoor::FULL_ID, "CKEndDoor");

					ImGui::Separator();

					auto block = [&toadd](int id, const char* name) {
						if (ImGui::Selectable(name))
							toadd = id;
					};
					block(CKPlayAnimCinematicBloc::FULL_ID, "CKPlayAnimCinematicBloc");
					block(CKPathFindingCinematicBloc::FULL_ID, "CKPathFindingCinematicBloc");
					block(CKFlaggedPathCinematicBloc::FULL_ID, "CKFlaggedPathCinematicBloc");
					block(CKGroupBlocCinematicBloc::FULL_ID, "CKGroupBlocCinematicBloc");
					block(CKAttachObjectsCinematicBloc::FULL_ID, "CKAttachObjectsCinematicBloc");
					block(CKParticleCinematicBloc::FULL_ID, "CKParticleCinematicBloc");
					block(CKSekensorCinematicBloc::FULL_ID, "CKSekensorCinematicBloc");
					block(CKDisplayPictureCinematicBloc::FULL_ID, "CKDisplayPictureCinematicBloc");
					block(CKManageCameraCinematicBloc::FULL_ID, "CKManageCameraCinematicBloc");
					block(CKStartEventCinematicBloc::FULL_ID, "CKStartEventCinematicBloc");
					block(CKLightningCinematicBloc::FULL_ID, "CKLightningCinematicBloc");
					block(CKPlaySoundCinematicBloc::FULL_ID, "CKPlaySoundCinematicBloc");
					ImGui::Separator();
					if (kenv.version >= KEnvironment::KVERSION_XXL2 || kenv.isRemaster) {
						block(CKPauseCinematicBloc::FULL_ID, "CKPauseCinematicBloc");
						block(CKTeleportCinematicBloc::FULL_ID, "CKTeleportCinematicBloc");
					}
					if (kenv.version == KEnvironment::KVERSION_XXL1) {
						block(CKStreamCinematicBloc::FULL_ID, "CKStreamCinematicBloc");
						block(CKStreamAloneCinematicBloc::FULL_ID, "CKStreamAloneCinematicBloc");
						block(CKStreamGroupBlocCinematicBloc::FULL_ID, "CKStreamGroupBlocCinematicBloc");
						block(CKManageEventCinematicBloc::FULL_ID, "CKManageEventCinematicBloc");
						block(CKManagerEventStopCinematicBloc::FULL_ID, "CKManagerEventStopCinematicBloc");
						block(CKSkyCinematicBloc::FULL_ID, "CKSkyCinematicBloc");
					}
					else if (kenv.version >= KEnvironment::KVERSION_XXL2) {
						block(CKPlayVideoCinematicBloc::FULL_ID, "CKPlayVideoCinematicBloc");
						block(CKFlashUICinematicBloc::FULL_ID, "CKFlashUICinematicBloc");
						if (kenv.version >= KEnvironment::KVERSION_OLYMPIC) {
							block(CKLockUnlockCinematicBloc::FULL_ID, "CKLockUnlockCinematicBloc");
						}
					}

					if (toadd != -1) {
						kenv.levelObjects.getClassType(toadd).info = 1;
						CKCinematicNode* added = kenv.createObject((uint32_t)toadd, -1)->cast<CKCinematicNode>();
						added->init(&kenv);
						added->cnScene = scene;
						if (kenv.version >= KEnvironment::KVERSION_XXL2) {
							added->cnFlags = 0x12;
						}
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();
				ImGui::BeginDisabled(ImNodes::NumSelectedNodes() <= 0);
				if (ImGui::Button("Remove nodes")) {
					int numSelNodes = ImNodes::NumSelectedNodes();
					if (numSelNodes > 0) {
						std::vector<int> selNodes;
						selNodes.resize(numSelNodes);
						ImNodes::GetSelectedNodes(selNodes.data());
						bool someImpossible = false;
						for (int id : selNodes) {
							CKCinematicNode* node = idBlockMap.at(id);
							int refCount = node->getRefCount();
							if (refCount == 0 && node->cnNumOutEdges == 0) {
								kenv.removeObject(node);
								idBlockMap.erase(id);
								blockIdMap.erase(node);
							}
							else
								someImpossible = true;
						}
						ImNodes::ClearNodeSelection();
						if (someImpossible)
							MsgBox(g_window, "Some nodes could not be removed as they are still referenced or have edges.", 48);
					}
				}
				ImGui::EndDisabled();

				ImNodes::BeginNodeEditor();
				// Nodes
				for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject* obj : cncls.objects) {
						CKCinematicNode *knode = obj->cast<CKCinematicNode>();
						if (knode->cnScene.get() == scene) {
							int id;
							if (auto it = blockIdMap.find(knode); it != blockIdMap.end()) {
								id = it->second;
							}
							else {
								blockIdMap.insert_or_assign(knode, nextId);
								idBlockMap.insert_or_assign(nextId, knode);
								id = nextId;
								++nextId;
							}
							ImNodes::BeginNode(id);
							ImNodes::BeginNodeTitleBar();
							ImGui::TextUnformatted(knode->getClassName());
							ImGui::SetNextItemWidth(128.0f);
							IGObjectNameInput("##Name", knode, kenv);
							ImNodes::EndNodeTitleBar();
							ImNodes::BeginInputAttribute(3 * id);
							ImGui::TextUnformatted("Start");
							ImNodes::EndInputAttribute();
							ImNodes::BeginInputAttribute(3 * id + 1);
							ImGui::TextUnformatted("Finish");
							ImNodes::EndInputAttribute();
							ImNodes::BeginOutputAttribute(3 * id + 2);
							ImGui::TextUnformatted("Done");
							ImNodes::EndOutputAttribute();							
							ImNodes::EndNode();
						}
					}
				}
				// Edges
				for (auto& cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject* obj : cncls.objects) {
						CKCinematicNode* knodeSrc = obj->cast<CKCinematicNode>();
						if (knodeSrc->cnScene.get() == scene) {
							int srcId = blockIdMap.at(knodeSrc);
							uint16_t start = (knodeSrc->cnStartOutEdge != 0xFFFF) ? knodeSrc->cnStartOutEdge : knodeSrc->cnFinishOutEdge;
							uint16_t end = start + knodeSrc->cnNumOutEdges;
							uint16_t trans = (knodeSrc->cnFinishOutEdge != 0xFFFF) ? knodeSrc->cnFinishOutEdge : end;
							for (int i = start; i < end; i++) {
								bool isFinish = i >= trans;
								int destId = blockIdMap.at(scene->cineNodes[i].get());
								ImNodes::PushColorStyle(ImNodesCol_Link, isFinish ? 0xFF0000FF : 0xFF00FF00);
								ImNodes::Link(i, 3 * srcId + 2, 3 * destId + (isFinish ? 1 : 0));
								ImNodes::PopColorStyle();
							}
						}
					}
				}
				ImNodes::MiniMap(0.2f, ImNodesMiniMapLocation_BottomRight);
				ImNodes::EndNodeEditor();

				int numSelNodes = ImNodes::NumSelectedNodes();
				if (numSelNodes > 0) {
					std::vector<int> selNodes;
					selNodes.resize(numSelNodes);
					ImNodes::GetSelectedNodes(selNodes.data());
					selectedCineNode = idBlockMap.at(selNodes[0]);
				}
				int creaSrcNode, creaSrcAttrib, creaDestNode, creaDestAttrib;
				if (ImNodes::IsLinkCreated(&creaSrcNode, &creaSrcAttrib, &creaDestNode, &creaDestAttrib)) {
					scene->addEdge(idBlockMap.at(creaSrcNode), idBlockMap.at(creaDestNode), (creaDestAttrib % 3) != 0, &kenv);
				}
				int destroyedLink;
				if (ImNodes::IsLinkDestroyed(&destroyedLink)) {
					auto [src, dest, fin] = scene->getEdgeInfo(destroyedLink, &kenv);
					scene->removeEdge(src, dest, fin, &kenv);
				}
				
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::NextColumn();
		if (ImGui::BeginTabBar("CineRight")) {
			if (ImGui::BeginTabItem("Node")) {
				ImGui::Text("(%i refs)", selectedCineNode->getRefCount());
				if (selectedCineNode) {
					ImGui::BeginChild("CineSelectedNode");
					uint32_t& flags = selectedCineNode->cnFlags;
					for (uint32_t i = 0; i < 16; ++i) {
						ImGui::CheckboxFlags(fmt::format("{:X}", i).c_str(), &flags, 1 << i);
						if ((i%8) != 7)
							ImGui::SameLine();
					}
					struct CineBlocMemberListener : ImGuiMemberListener {
						using ImGuiMemberListener::ImGuiMemberListener;
						void message(const char* msg) {
							static const std::string_view target = "End of Cinematic Node Base";
							if (msg == target)
								ImGui::Separator();
						}
					};
					CineBlocMemberListener ml(kenv, *this);
					ml.setPropertyInfoList(g_encyclo, selectedCineNode.get());
					selectedCineNode->virtualReflectMembers(ml, &kenv);

					if (CKDisplayPictureCinematicBloc* dp = selectedCineNode->dyncast<CKDisplayPictureCinematicBloc>()) {
						ImVec4 color = ImGui::ColorConvertU32ToFloat4(dp->ckdpcbColor);
						if (ImGui::ColorEdit4("Color", &color.x))
							dp->ckdpcbColor = ImGui::ColorConvertFloat4ToU32(color);
						IGStringInput("Texture", dp->ckdpcbBillboard->texture);
					}
				
					ImGui::EndChild();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Scene")) {
				ImGui::BeginChild("CineSceneProperties");
				IGObjectNameInput("Name", scene, kenv);
				ImGuiMemberListener iml{ kenv, *this };
				iml.setPropertyInfoList(g_encyclo, scene);
				MemberListener& ml = iml;
				ml.reflect(scene->csFlags, "csFlags");
				if (kenv.version == KEnvironment::KVERSION_XXL1) {
					int flags = scene->csFlags;
					bool mod = ImGui::CheckboxFlags("Only play once", &flags, 2);
					mod |= ImGui::CheckboxFlags("Play on start", &flags, 0x10);
					mod |= ImGui::CheckboxFlags("Disable HUD & Pause", &flags, 0x100);
					mod |= ImGui::CheckboxFlags("Reset camera at end", &flags, 0x400);
					mod |= ImGui::CheckboxFlags("End on pressing ENTER", &flags, 0x1000);
					mod |= ImGui::CheckboxFlags("Black bars", &flags, 0x8000);
					if (mod)
						scene->csFlags = (uint16_t)flags;
				}
				ml.reflect(scene->csUnk2, "csUnk2");
				ml.reflect(scene->csBarsColor, "csBarsColor");
				ImVec4 color = ImGui::ColorConvertU32ToFloat4(scene->csBarsColor);
				if (ImGui::ColorEdit4("Color", &color.x))
					scene->csBarsColor = ImGui::ColorConvertFloat4ToU32(color);
				ml.reflect(scene->csUnk4, "csUnk4");
				ml.reflect(scene->csUnk5, "csUnk5");
				ml.reflect(scene->csUnk6, "csUnk6");
				ml.reflect(scene->csUnk7, "csUnk7");
				ml.reflect(scene->csUnk8, "csUnk8");
				ml.reflect(scene->csUnk9, "csUnk9");
				ml.reflect(scene->csUnkA, "csUnkA");
				ml.reflect(scene->onSceneEnded, "onSceneEnded", nullptr);
				ml.reflect(scene->groups, "groups");
				ml.reflect(scene->sndDict, "sndDict");
				ml.reflect(scene->csUnkF, "csUnkF");
				if (kenv.version >= KEnvironment::KVERSION_XXL2) {
					ml.reflect(scene->arcsUnk1a, "arcsUnk1a");
					ml.reflect(scene->ogOnSceneStart, "ogOnSceneStart", nullptr);
					ml.reflect(scene->spyroOnSceneSkipped, "spyroOnSceneSkipped", nullptr);
					ml.reflect(scene->x2CameraEndDuration, "x2CameraEndDuration");
					ml.reflect(scene->arthurOnlyByte, "arthurOnlyByte");
					ml.reflect(scene->spyroSkipScene, "spyroSkipScene");
				}
				else if (kenv.isRemaster) {
					ml.reflect(scene->otherUnkFromRomaster, "otherUnkFromRomaster");
				}
				ImGui::SetNextItemOpen(true, ImGuiCond_Once);
				if (ImGui::CollapsingHeader("Scene Data")) {
					for (auto& dataRef : scene->cineDatas) {
						CKCinematicSceneData* data = dataRef.get();
						ImGui::PushID(data);
						ml.reflect(data->hook, "hook");
						ml.reflect(data->animDict, "animDict");
						ml.reflect(data->csdUnkA, "csdUnkA");
						ml.reflect(data->csdUnkB, "csdUnkB");
						ImGui::PopID();
						if (&dataRef != &scene->cineDatas.back()) ImGui::Separator();
					}
					if (ImGui::Button("Add data")) {
						CKCinematicSceneData* data = kenv.createAndInitObject<CKCinematicSceneData>();
						data->animDict = kenv.createAndInitObject<CAnimationDictionary>();
						scene->cineDatas.emplace_back(data);
					}
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::Columns();
	}
}

std::unique_ptr<LocaleEditor> g_localeEditor;

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
			IGObjectSelectorRef(kenv, "##EventObj", act.target);
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
			case 3: IGObjectSelectorRef(kenv, "##Value", std::get<KPostponedRef<CKObject>>(act.value)); break;
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

void EditorInterface::IGX2DetectorEditor()
{
	if (kenv.version < kenv.KVERSION_XXL2)
		return;
	int strid = 0;
	ImGui::Columns(2);
	ImGui::BeginChild("DtcList");
	for (CKObject* osector : kenv.levelObjects.getClassType<CKSectorDetector>().objects) {
		CKSectorDetector* sector = osector->cast<CKSectorDetector>();
		if (ImGui::TreeNode(sector, "Sector %i", strid)) {
			for (auto& detector : sector->sdDetectors) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
				if (selectedX2Detector.get() == detector.get())
					flags |= ImGuiTreeNodeFlags_Selected;
				if (ImGui::TreeNodeEx(detector.get(), flags, "%s", kenv.getObjectName(detector.get())))
					ImGui::TreePop();
				if (ImGui::IsItemClicked())
					selectedX2Detector = detector.get();
				IGObjectDragDropSource(kenv, detector.get());
			}
			CKDetectorBase* addedDetector = nullptr;
			if (ImGui::Button("Add Event detector")) {
				addedDetector = kenv.createAndInitObject<CKDetectorEvent>();
			}
			ImGui::SameLine();
			if (ImGui::Button("Add Music detector")) {
				addedDetector = kenv.createAndInitObject<CKDetectorMusic>();
			}
			if (addedDetector) {
				addedDetector->dbFlags = 113;
				addedDetector->dbMovable = kenv.levelObjects.getFirst<CKDetectedMovable>();
				addedDetector->dbGeometry = kenv.createAndInitObject<CMultiGeometry>();
				addedDetector->dbSectorIndex = strid;
				addedDetector->dbGeometry->mgShape.emplace<AABoundingBox>(cursorPosition + Vector3(1.0f, 1.0f, 1.0f), cursorPosition - Vector3(1.0f, 1.0f, 1.0f));
				sector->sdDetectors.emplace_back(addedDetector);
			}
			ImGui::TreePop();
		}
		++strid;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("DtcProps");
	if (CKDetectorBase* detector = selectedX2Detector.get()) {
		IGObjectNameInput("Name", detector, kenv);
		ImGuiMemberListener igml(kenv, *this);
		igml.setPropertyInfoList(g_encyclo, detector);
		detector->virtualReflectMembers(igml, &kenv);
		if (ImGui::CollapsingHeader("Geometry") && detector->dbGeometry) {
			auto& mgShape = detector->dbGeometry->mgShape;
			static const char* shapeTypeNames[3] = { "Box", "Sphere", "Rectangle" };
			int shapeType = mgShape.index();
			if (ImGui::Combo("Shape Type", &shapeType, shapeTypeNames, 3)) {
				// obtain center from old shape
				Vector3 center;
				if (auto* aabb = std::get_if<AABoundingBox>(&mgShape)) {
					center = (aabb->highCorner + aabb->lowCorner) * 0.5f;
				}
				else if (auto* sph = std::get_if<BoundingSphere>(&mgShape)) {
					center = sph->center;
				}
				else if (auto* rect = std::get_if<AARectangle>(&mgShape)) {
					center = rect->center;
				}
				// then create new shape
				if (shapeType == 0)
					mgShape.emplace<AABoundingBox>(center + Vector3(1.0f, 1.0f, 1.0f), center - Vector3(1.0f, 1.0f, 1.0f));
				else if (shapeType == 1)
					mgShape.emplace<BoundingSphere>(center, 1.0f);
				else if (shapeType == 2)
					mgShape.emplace<AARectangle>(center);
			}
			detector->dbGeometry->reflectMembers2(igml, &kenv);
			ImGui::Text("%i references", detector->dbGeometry->getRefCount());
		}
		if (ImGui::CollapsingHeader("Movables") && detector->dbMovable) {
			detector->dbMovable->reflectMembers2(igml, &kenv);
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGCollisionEditor()
{
	CKSrvCollision* srvcoll = kenv.levelObjects.getFirst<CKSrvCollision>();
	if (!srvcoll)
		return;
	ImGuiMemberListener igml{ kenv, *this };
	igml.setPropertyInfoList(g_encyclo, srvcoll);
	MemberListener* ml = &igml;
	if (ImGui::BeginTabBar("ColliTabBar")) {
		if (ImGui::BeginTabItem("Head")) {
			//ml->reflect(srvcoll->numWhat, "numWhat");
			ml->reflect(srvcoll->huh, "huh");
			//ml->reflect(srvcoll->unk1, "unk1");
			//ml->reflect(srvcoll->unk2, "unk2");
			ml->reflect(srvcoll->activeList, "activeList");
			ml->reflect(srvcoll->inactiveList, "inactiveList");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("objs")) {
			int i = 0;
			for (auto& vec : srvcoll->objs) {
				int j = 0;
				for (auto& ref : vec) {
					std::string str = std::to_string(i) + ',' + std::to_string(j);
					IGObjectSelectorRef(kenv, str.c_str(), ref);
					j++;
				}
				ImGui::Separator();
				i++;
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("objs2")) {
			int i = 0;
			for (auto& ref : srvcoll->objs2)
				IGObjectSelectorRef(kenv, std::to_string(i++).c_str(), ref);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("bings")) {
			static size_t selectedBing = -1;
			auto getActorName = [this,srvcoll](uint16_t id) -> const char* {
				if (id != 0xFFFF && srvcoll->objs2[id]) {
					if (kenv.version >= kenv.KVERSION_XXL2)
						return kenv.getObjectName(srvcoll->objs2[id].get());
					else
						return srvcoll->objs2[id]->getClassName();
				}
				return "/";
			};
			ImGui::Columns(2);
			static std::vector<int> bingTags;
			if (ImGui::Button("Tag")) {
				bingTags = std::vector<int>(srvcoll->bings.size(), 0);
				std::vector<int> nodes = { srvcoll->activeList };
				std::vector<int> nodes2;
				bingTags[srvcoll->activeList] |= 1;
				while (!nodes.empty()) {
					nodes2.clear();
					for (int n : nodes) {
						for (int a : srvcoll->bings[n].aa) {
							if (a != 0xFFFF && (bingTags[a] & 1) == 0) {
								bingTags[a] |= 1;
								nodes2.push_back(a);
							}
						}
					}
					std::swap(nodes, nodes2);
				}
				nodes = { srvcoll->inactiveList };
				nodes2.clear();
				bingTags[srvcoll->inactiveList] |= 2;
				while (!nodes.empty()) {
					nodes2.clear();
					for (int n : nodes) {
						for (int a : {srvcoll->bings[n].aa[0], srvcoll->bings[n].aa[1]}) {
							if (a != 0xFFFF && (bingTags[a] & 2) == 0) {
								bingTags[a] |= 2;
								nodes2.push_back(a);
							}
						}
					}
					std::swap(nodes, nodes2);
				}
			}
			ImGui::BeginChild("CollBingList");
			int b = 0;
			for (auto& bing : srvcoll->bings) {
				ImGui::PushID(b);
				if (ImGui::Selectable("##sel", b == selectedBing))
					selectedBing = b;
				if (ImGui::IsItemVisible()) {
					ImGui::SameLine();
					int tag = (b < bingTags.size()) ? bingTags[b] : 7;
					ImGui::Text("%i:%i, %s : %s", b, tag, getActorName(bing.b1), getActorName(bing.b2));
				}
				ImGui::PopID();
				b++;
			}
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("CollBingInfo");
			if (selectedBing < srvcoll->bings.size()) {
				auto& bing = srvcoll->bings[selectedBing];
				ImGui::InputScalar("v1", ImGuiDataType_U16, &bing.v1, nullptr, nullptr, "%04X");
				ml->reflect(bing.obj1, "obj1");
				ml->reflect(bing.obj2, "obj2");
				ml->reflect(bing.b1, "b1");
				ImGui::Text("%s", getActorName(bing.b1));
				ml->reflect(bing.b2, "b2");
				ImGui::Text("%s", getActorName(bing.b2));
				ml->reflect(bing.v2, "v2");
				ml->reflect(bing.aa, "aa");
				ImGui::Separator();
				if (bing.obj1) {
					ml->reflect(bing.obj1->cast<CKBoundingShape>()->unk1, "obj1_unk1");
					ml->reflect(bing.obj1->cast<CKBoundingShape>()->unk2, "obj1_unk2");
				}
				if (bing.obj2) {
					ml->reflect(bing.obj2->cast<CKBoundingShape>()->unk1, "obj2_unk1");
					ml->reflect(bing.obj2->cast<CKBoundingShape>()->unk2, "obj2_unk2");
				}
			}
			ImGui::EndChild();
			ImGui::Columns();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void EditorInterface::IGLineEditor()
{
	static KWeakRef<CKLogic> lineObject;
	kobjref<CKLogic> lineTempRef = lineObject.get();
	IGObjectSelectorRef(kenv, "Line", lineTempRef);
	lineObject = lineTempRef.get();
	if (!lineObject) return;
	if (CKLine* line = lineObject->dyncast<CKLine>()) {
		bool update = false;
		for (size_t i = 0; i < line->points.size(); ++i) {
			ImGui::PushID((int)i);
			ImGui::DragFloat3("##LinePoint", &line->points[i].x, 0.1f);
			ImGui::SameLine();
			if (ImGui::Button("D")) {
				line->points.insert(line->points.begin() + i, line->points[i]);
				update = true;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate");
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				line->points.erase(line->points.begin() + i);
				update = true;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove");
			ImGui::PopID();
		}
		if (update) {
			line->numSegments = (uint8_t)(line->points.size() - 1);
			line->segmentLengths.resize(line->numSegments);
			float total = 0.0f;
			for (size_t i = 0; i < (size_t)line->numSegments; ++i) {
				line->segmentLengths[i] = (line->points[i + 1] - line->points[i]).len3();
				total += line->segmentLengths[i];
			}
			line->totalLength = total;
		}
	}
	if (CKFlaggedPath* path = lineObject->dyncast<CKFlaggedPath>()) {
		IGObjectSelectorRef(kenv, "Path's line", path->line);
		for (size_t i = 0; i < path->numPoints; ++i) {
			ImGui::PushID((int)i);
			ImGui::SetNextItemWidth(64.0f);
			ImGui::DragFloat("##PathElemValue", &path->pntValues[i], 0.1f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(128.0f);
			IGEventSelector("", path->pntEvents[i]);
			ImGui::SameLine();
			if (ImGui::Button("D")) {
				path->pntValues.insert(path->pntValues.begin() + i, path->pntValues[i]);
				path->pntEvents.insert(path->pntEvents.begin() + i, path->pntEvents[i]);
				path->numPoints += 1;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Duplicate");
			ImGui::SameLine();
			if (ImGui::Button("X")) {
				path->pntValues.erase(path->pntValues.begin() + i);
				path->pntEvents.erase(path->pntEvents.begin() + i);
				path->numPoints -= 1;
				--i;
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Remove");
			ImGui::PopID();
		}
	}
}

void EditorInterface::IGLevelEditor()
{
	bool doDynGroundFix = false;
	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::Button("Add new sector")) {
		int strNumber = kenv.sectorObjects.size();
		auto& str = kenv.sectorObjects.emplace_back();
		kenv.sectorObjNames.emplace_back();
		int clcat = 0;
		for (auto& cat : str.categories) {
			cat.type.resize(kenv.levelObjects.categories[clcat].type.size());
			int clid = 0;
			for (auto& kcl : cat.type) {
				auto& lvltype = kenv.levelObjects.categories[clcat].type[clid];
				kcl.startId = (uint16_t)lvltype.objects.size();
				if (lvltype.info != 2) {
					for (int p = 0; p < strNumber; p++)
						kcl.startId += (uint16_t)kenv.sectorObjects[p].categories[clcat].type[clid].objects.size();
				}
				clid++;
			}
			clcat++;
		}
		kenv.numSectors++;

		// CKSector
		CKSector* ksector = kenv.createObject<CKSector>(-1);
		ksector->sgRoot = kenv.createObject<CSGSectorRoot>(strNumber);
		ksector->strId = strNumber + 1;
		ksector->unk1 = 2;
		ksector->soundDictionary = kenv.createObject<CKSoundDictionary>(strNumber);
		ksector->soundDictionary->cast<CKSoundDictionary>()->inactive = strNumber + 1;
		ksector->meshKluster = kenv.createObject<CKMeshKluster>(strNumber);

		// beacons
		auto& bs = kenv.levelObjects.getFirst<CKSrvBeacon>()->beaconSectors.emplace_back();

		// sgroot
		CTextureDictionary* texDict = kenv.createObject<CTextureDictionary>(strNumber);
		ksector->sgRoot->cast<CSGSectorRoot>()->texDictionary = texDict;
		texDict->piDict.nativeVersionPlatform = kenv.levelObjects.getFirst<CTextureDictionary>()->piDict.nativeVersionPlatform;
		//ksector->sgRoot->cast<CSGSectorRoot>()->sectorNum = strNumber+1;

		doDynGroundFix = true;

		// Lvl
		CKLevel* klevel = kenv.levelObjects.getFirst<CKLevel>();
		klevel->sectors.emplace_back(ksector);

		// editor
		progeocache.clear();
		gndmdlcache.clear();
		prepareLevelGfx();
	}
	ImGui::SameLine();
	ImGui::Text("%i sectors", kenv.numSectors);

	if (ImGui::Button("Fix last sector's dyngrounds") || doDynGroundFix) {
		// add common dynamic grounds in MeshKluster
		for (auto& str : kenv.sectorObjects) {
			auto& strGrounds = str.getFirst<CKMeshKluster>()->grounds;
			const auto& lvlGrounds = kenv.levelObjects.getFirst<CKMeshKluster>()->grounds;
			for (auto& ref : lvlGrounds) {
				if (ref && ref->isSubclassOf<CDynamicGround>()) {
					if (kenv.sectorObjects.size() >= 1) {
						auto& firstGrounds = kenv.sectorObjects[0].getFirst<CKMeshKluster>()->grounds;
						if (std::find(firstGrounds.begin(), firstGrounds.end(), ref) == firstGrounds.end())
							continue;
					}
					if (std::find(strGrounds.begin(), strGrounds.end(), ref) == strGrounds.end())
						strGrounds.push_back(ref);
				}
			}
		}
	}

	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::CollapsingHeader("Level Start")) {
		using namespace GameX1;
		CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
		static int cheatIndex = 0;
		if (!kenv.isRemaster) cheatIndex = 0;

		auto getCheatDesc = [level](int index) -> std::string {
			return std::to_string(index) + ": " + level->lvlRemasterCheatSpawnNames[index];
		};

		if (kenv.isRemaster) {
			if (ImGui::BeginCombo("Cheat", getCheatDesc(cheatIndex).c_str())) {
				for (int i = 0; i < 20; ++i) {
					ImGui::PushID(i);
					if (ImGui::Selectable("##cheatentry")) {
						cheatIndex = i;
					}
					ImGui::SameLine();
					ImGui::TextUnformatted(getCheatDesc(i).c_str());
					ImGui::PopID();
				}
				ImGui::EndCombo();
			}
			IGStringInput("Cheat name", level->lvlRemasterCheatSpawnNames[cheatIndex]);
		}
		ImGui::InputScalar("Initial sector", ImGuiDataType_U32, &level->initialSector[cheatIndex]);
		CKHkHero* heroes[3] = { kenv.levelObjects.getFirst<CKHkAsterix>(), kenv.levelObjects.getFirst<CKHkObelix>(), kenv.levelObjects.getFirst<CKHkIdefix>() };
		static constexpr const char* heroNames[3] = { "Asterix", "Obelix", "Dogmatix" };
		if (heroes[0] && heroes[1] && heroes[2]) {
			for (size_t i = 0; i < 3; ++i) {
				ImGui::InputFloat3(heroNames[i], &heroes[i]->heroUnk53[cheatIndex].x);
			}
			if (ImGui::Button("Teleport heroes to cursor and update start positions")) {
				Vector3 oriAstPos = heroes[0]->node->transform.getTranslationVector();
				Vector3 vec = cursorPosition - oriAstPos;
				for (size_t i = 0; i < 3; ++i) {
					CKHkHero* hero = heroes[i];
					auto& mat = hero->node->transform;
					mat.setTranslation(mat.getTranslationVector() + vec);
					heroes[i]->heroUnk53[cheatIndex] = heroes[i]->node->transform.getTranslationVector();
				}
			}
			if (ImGui::Button("Update hero start positions from nodes")) {
				for (size_t i = 0; i < 3; ++i) {
					heroes[i]->heroUnk53[cheatIndex] = heroes[i]->node->transform.getTranslationVector();
				}
			}
		}
	}

	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::CollapsingHeader("Sky colors")) {
		if (CKHkSkyLife* hkSkyLife = kenv.levelObjects.getFirst<CKHkSkyLife>()) {
			ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(hkSkyLife->skyColor);
			ImGui::ColorEdit4("Sky color", &c1.x);
			hkSkyLife->skyColor = ImGui::ColorConvertFloat4ToU32(c1);
			ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(hkSkyLife->cloudColor);
			ImGui::ColorEdit4("Cloud color", &c2.x);
			hkSkyLife->cloudColor = ImGui::ColorConvertFloat4ToU32(c2);
		}
	}
	if (kenv.version == kenv.KVERSION_XXL1 && ImGui::CollapsingHeader("Level-handled objects")) {
		CKLevel* level = kenv.levelObjects.getFirst<CKLevel>();
		ImGui::PushID("LevelObjs");
		int i = 0;
		for (auto& kref : level->objs) {
			IGObjectSelectorRef(kenv, std::to_string(i++).c_str(), kref);
		}
		if (ImGui::Button("Add"))
			level->objs.emplace_back();
		ImGui::PopID();
	}
}

void EditorInterface::IGAbout()
{
	static bool loaded = false;
	static texture_t logo = nullptr;
	static int logoWidth, logoHeight;
	if (!loaded) {
		auto [ptr, len] = GetResourceContent("logo.png");
		RwImage img = RwImage::loadFromMemory(ptr, len);
		logo = gfx->createTexture(img);
		logoWidth = img.width;
		logoHeight = img.height;
		loaded = true;
	}

	ImGui::Image(logo, ImVec2(400.0f, 400.0f * (float)logoHeight / (float)logoWidth));
#ifdef XEC_APPVEYOR
	static const char* version = "Version " XEC_APPVEYOR;
#else
	static const char* version = "Development version";
#endif
	ImGui::Text("XXL Editor\n%s\nbuilt on " __DATE__, version);
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	ImGui::Text("Developed by AdrienTD\nThanks to S.P.Q.R");
	ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	IGLink("Wiki", L"https://github.com/AdrienTD/XXL-Editor/wiki", g_window);
	ImGui::TextUnformatted("for documentation, tutorials, and links to Discord servers");
	IGLink("GitHub repo", L"https://github.com/AdrienTD/XXL-Editor", g_window);
	ImGui::TextUnformatted("for source code and stable releases");
	IGLink("AppVeyor", L"https://ci.appveyor.com/project/AdrienTD/xxl-editor", g_window);
	ImGui::TextUnformatted("for the latest development build");
}

void EditorInterface::IGCamera()
{
	static KWeakRef<CKCameraBase> selectedCamera;
	CKSrvCamera* srvCamera = kenv.levelObjects.getFirst<CKSrvCamera>();
	auto viewCamera = [this](CKCameraBase* kcamera) {
		camera.position = kcamera->kcamPosition;
		Vector3 newDir = (kcamera->kcamLookAt - camera.position).normal();
		float newAngleX = std::asin(newDir.y);
		Vector3 newOri;
		float cosAX = std::cos(newAngleX);
		float mar = 1.5707f;
		if (!(-mar <= newAngleX && newAngleX <= mar))
			newOri = { std::clamp(newAngleX, -mar + 0.0005f, mar - 0.0005f), 0, 0 };
		else
			newOri = { newAngleX, std::atan2(-newDir.x / cosAX, newDir.z / cosAX) };
		camera.orientation = newOri;
	};
	if (ImGui::BeginTable("CameraTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if (ImGui::Button("New camera")) {
			ImGui::OpenPopup("NewCamera");
		}
		if (ImGui::BeginPopup("NewCamera")) {
			int toadd = -1;
			auto cls = [&toadd](int id, const char* name) {
				if (ImGui::Selectable(name))
					toadd = id;
			};
			cls(CKCamera::FULL_ID, "CKCamera");
			cls(CKCameraRigidTrack::FULL_ID, "CKCameraRigidTrack");
			if (kenv.version == KEnvironment::KVERSION_XXL1)
				cls(CKCameraClassicTrack::FULL_ID, "CKCameraClassicTrack");
			cls(CKCameraPathTrack::FULL_ID, "CKCameraPathTrack");
			cls(CKCameraFixTrack::FULL_ID, "CKCameraFixTrack");
			cls(CKCameraAxisTrack::FULL_ID, "CKCameraAxisTrack");
			cls(CKCameraSpyTrack::FULL_ID, "CKCameraSpyTrack");
			cls(CKCameraPassivePathTrack::FULL_ID, "CKCameraPassivePathTrack");
			if (kenv.version >= KEnvironment::KVERSION_XXL2) {
				cls(CKCameraBalistTrack::FULL_ID, "CKCameraBalistTrack");
				cls(CKCameraClassicTrack2::FULL_ID, "CKCameraClassicTrack2");
				cls(CKCameraFirstPersonTrack::FULL_ID, "CKCameraFirstPersonTrack");
			}
			if (toadd != -1) {
				kenv.levelObjects.getClassType(toadd).info = 1;
				CKCameraBase* added = kenv.createObject((uint32_t)toadd, -1)->cast<CKCameraBase>();
				added->init(&kenv);
				added->kcamNextCam = srvCamera->scamCam;
				srvCamera->scamCam = added;
			}
			ImGui::EndPopup();
		}
		ImGui::BeginChild("CameraList");
		for (CKCameraBase* camera = srvCamera->scamCam.get(); camera; camera = camera->kcamNextCam.get()) {
			ImGui::PushID(camera);
			if (ImGui::Selectable("##CamSel", selectedCamera == camera)) {
				selectedCamera = camera;
			}
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					viewCamera(camera);
				}
			}
			ImGui::SameLine();
			ImGui::Text("%s %s", camera->getClassName(), kenv.getObjectName(camera));
			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::TableNextColumn();
		if (CKCameraBase* kcamera = selectedCamera.get()) {
			ImGui::BeginChild("CameraProps");
			IGObjectNameInput("Name", kcamera, kenv);
			bool mod = false;
			mod |= ImGui::DragFloat("FOV", &kcamera->kcamFOV);
			mod |= ImGui::DragFloat3("Position", &kcamera->kcamPosition.x);
			mod |= ImGui::DragFloat3("Look at", &kcamera->kcamLookAt.x);
			mod |= ImGui::DragFloat3("Up vector", &kcamera->kcamUpVector.x);
			if (ImGui::Button("View")) {
				viewCamera(kcamera);
			}
			ImGui::SameLine();
			if (ImGui::Button("Set")) {
				kcamera->kcamPosition = camera.position;
				kcamera->kcamLookAt = camera.position + camera.direction;
				mod = true;
			}
			if (mod) {
				kcamera->kcamFarDistance_dup = kcamera->kcamFarDistance;
				kcamera->kcamUnk3_dup = kcamera->kcamUnk3;
				kcamera->kcamFOV_dup = kcamera->kcamFOV;
				kcamera->kcamPosition_dup = kcamera->kcamPosition;
				kcamera->kcamLookAt_dup = kcamera->kcamLookAt;
				kcamera->kcamUpVector_dup = kcamera->kcamUpVector;
			}
			if (kcamera->ogFogData && ImGui::CollapsingHeader("Fog data")) {
				CKCameraFogDatas* fogData = kcamera->ogFogData.get();
				IGU32Color("Color 1", fogData->color1);
				ImGui::InputFloat("unk2", &fogData->unk2);
				ImGui::InputFloat("unk3", &fogData->unk3);
				IGU32Color("Color 2", fogData->color2);
				ImGui::InputFloat("unk4", &fogData->unk4);
				ImGui::InputFloat("unk5", &fogData->unk5);
				ImGui::InputFloat("unk6", &fogData->unk6);
			}
			struct CameraEditMemberListener : FilterMemberListener<CameraEditMemberListener, ImGuiMemberListener> {
				using FilterMemberListener::FilterMemberListener;
				bool _allow = true;
				bool cond(const char* name) {
					return _allow;
				}
				virtual void message(const char* msg) override {
					if (msg == std::string_view("End of CKCamera")) {
						_allow = true;
					}
				}
			};
			bool showBaseMembers = false;
			if (ImGui::CollapsingHeader("Base members"))
				showBaseMembers = true;
			CameraEditMemberListener iml{ kenv, *this };
			iml.setPropertyInfoList(g_encyclo, kcamera);
			iml._allow = showBaseMembers;
			kcamera->virtualReflectMembers(iml, &kenv);
			ImGui::EndChild();
		}
		ImGui::EndTable();
	}

}

void EditorInterface::IGCounters()
{
	static KWeakRef<CKObject> selectedCounter;
	CKSrvCounter* srvCounter = kenv.levelObjects.getFirst<CKSrvCounter>();
	if (ImGui::BeginTable("CountersTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoHostExtendY, ImGui::GetContentRegionAvail())) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		if (ImGui::Button("New integer")) {
			kenv.levelObjects.getClassType<CKIntegerCounter>().info = 1;
			srvCounter->integerCounters.emplace_back(kenv.createAndInitObject<CKIntegerCounter>());
		}
		ImGui::SameLine();
		if (ImGui::Button("New timer")) {
			kenv.levelObjects.getClassType<CKTimeCounter>().info = 1;
			srvCounter->timeCounters.emplace_back(kenv.createAndInitObject<CKTimeCounter>());
		}
		ImGui::BeginChild("CounterList");
		for (auto& ref : srvCounter->integerCounters) {
			CKIntegerCounter* intCounter = ref.get();
			ImGui::PushID(intCounter);
			if (ImGui::Selectable("##sel", selectedCounter == intCounter))
				selectedCounter = intCounter;
			ImGui::SameLine();
			ImGui::Text("Integer %s", kenv.getObjectName(intCounter));
			ImGui::PopID();
		}
		ImGui::Separator();
		for (auto& ref : srvCounter->timeCounters) {
			CKTimeCounter* timeCounter = ref.get();
			ImGui::PushID(timeCounter);
			if (ImGui::Selectable("##sel", selectedCounter == timeCounter))
				selectedCounter = timeCounter;
			ImGui::SameLine();
			ImGui::Text("Timer %s", kenv.getObjectName(timeCounter));
			ImGui::PopID();
		}
		ImGui::EndChild();

		ImGui::TableNextColumn();

		if (selectedCounter) {
			IGObjectNameInput("Name", selectedCounter.get(), kenv);
			ImGuiMemberListener ml{ kenv, *this };
			ml.setPropertyInfoList(g_encyclo, selectedCounter.get());
			if (CKIntegerCounter* intCounter = selectedCounter->dyncast<CKIntegerCounter>()) {
				intCounter->reflectMembers2(ml, &kenv);
			}
			else if (CKTimeCounter* timeCounter = selectedCounter->dyncast<CKTimeCounter>()) {
				timeCounter->reflectMembers2(ml, &kenv);
			}
		}

		ImGui::EndTable();
	}
}

void EditorInterface::IGMusic()
{
	static KWeakRef<CKMusicPlayList> selectedPlayList;
	static int selectedTrackIndex = -1;
	CKSrvMusic* srvMusic = kenv.levelObjects.getFirst<CKSrvMusic>();
	if (ImGui::Button("New playlist")) {
		srvMusic->playLists.emplace_back(kenv.createAndInitObject<CKMusicPlayList>());
	}
	ImGui::SetNextItemWidth(-1.0f);
	if (ImGui::BeginListBox("##PlayListListBox")) {
		for (auto& playList : srvMusic->playLists) {
			ImGui::PushID(playList.get());
			if (ImGui::Selectable("##PlayListSelect", playList.get() == selectedPlayList.get()))
				selectedPlayList = playList.get();
			ImGui::SameLine();
			ImGui::TextUnformatted(kenv.getObjectName(playList.get()));
			ImGui::PopID();
		}
		ImGui::EndListBox();
	}
	ImGui::Separator();
	bool streamsSeparated = kenv.version >= KEnvironment::KVERSION_OLYMPIC;
	if (selectedPlayList) {
		IGObjectNameInput("Playlist Name", selectedPlayList.get(), kenv);
		if (ImGui::Button("New stream")) {
			if (streamsSeparated)
				selectedPlayList->ogStreams.emplace_back(kenv.createAndInitObject<CKStreamObject>(), 0);
			else
				selectedPlayList->x2Streams.emplace_back();
		}
		int numStreams = (int)(streamsSeparated ? selectedPlayList->ogStreams.size() : selectedPlayList->x2Streams.size());
		ImGui::SameLine();
		ImGui::BeginDisabled(!(0 <= selectedTrackIndex && selectedTrackIndex < numStreams));
		if (ImGui::ArrowButton("TrackDown", ImGuiDir_Down)) {
			if (selectedTrackIndex < numStreams - 1) {
				if (streamsSeparated)
					std::swap(selectedPlayList->ogStreams[selectedTrackIndex], selectedPlayList->ogStreams[selectedTrackIndex + 1]);
				else
					std::swap(selectedPlayList->x2Streams[selectedTrackIndex], selectedPlayList->x2Streams[selectedTrackIndex + 1]);
				selectedTrackIndex += 1;
			}

		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move stream down");
		ImGui::SameLine();
		if (ImGui::ArrowButton("TrackUp", ImGuiDir_Up)) {
			if (selectedTrackIndex > 0) {
				if (streamsSeparated)
					std::swap(selectedPlayList->ogStreams[selectedTrackIndex], selectedPlayList->ogStreams[selectedTrackIndex - 1]);
				else
					std::swap(selectedPlayList->x2Streams[selectedTrackIndex], selectedPlayList->x2Streams[selectedTrackIndex - 1]);
				selectedTrackIndex -= 1;
			}
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Move stream up");
		ImGui::SameLine();
		if (ImGui::Button("Delete stream")) {
			if (0 <= selectedTrackIndex && selectedTrackIndex < numStreams) {
				if (streamsSeparated) {
					CKObject* obj = selectedPlayList->ogStreams[selectedTrackIndex].first.get();
					selectedPlayList->ogStreams.erase(selectedPlayList->ogStreams.begin() + selectedTrackIndex);
					if (obj)
						kenv.removeObject(obj);
				}
				else {
					selectedPlayList->x2Streams.erase(selectedPlayList->x2Streams.begin() + selectedTrackIndex);
				}
				numStreams -= 1;
			}
		}
		ImGui::EndDisabled();
		ImGui::SetNextItemWidth(-1.0f);
		if (ImGui::BeginListBox("##PlayListTrackList")) {
			for (int i = 0; i < numStreams; ++i) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##Track", selectedTrackIndex == i))
					selectedTrackIndex = i;
				ImGui::SameLine();
				if (streamsSeparated)
					ImGui::Text("%2i: %s", i, kenv.getObjectName(selectedPlayList->ogStreams[i].first.get()));
				else
					ImGui::Text("%2i: Stream %2i", i, selectedPlayList->x2Streams[i].streamIndex);
				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
		ImGui::Separator();
		if (0 <= selectedTrackIndex && selectedTrackIndex < numStreams) {
			ImGuiMemberListener ml{ kenv, *this };
			if (streamsSeparated) {
				CKStreamObject* stream = selectedPlayList->ogStreams[selectedTrackIndex].first.get();
				ml.setPropertyInfoList(g_encyclo, stream);
				IGObjectNameInput("Entry Name", stream, kenv);
				IGObjectSelectorRef(kenv, "track", stream->streamPointer);
				if (stream->streamPointer)
					ImGui::Text("File %s, %f seconds", stream->streamPointer->wavePath.c_str(), stream->streamPointer->waveDurationSec);
				ml.reflect(stream->param1, "param1");
				ml.reflect(stream->param2, "param2");
				ml.reflect(stream->param3, "param3");
				ml.reflect(stream->param4, "param4");
				ml.reflect(stream->ogUnk1, "ogUnk1");
				ml.reflect(stream->ogUnk2, "ogUnk2");
			}
			else {
				auto& stream = selectedPlayList->x2Streams[selectedTrackIndex];
				ml.reflect(stream.streamIndex, "streamIndex");
				ml.reflect(stream.param1, "param1");
				ml.reflect(stream.param2, "param2");
				ml.reflect(stream.param3, "param3");
				ml.reflect(stream.param4, "param4");
				ml.reflect(stream.x2Unk1, "x2Unk1");
			}
		}
	}
}

void EditorInterface::IGSekens()
{
	static KWeakRef<CKSekens> selectedSekens;
	CKSrvSekensor* srvSekensor = kenv.levelObjects.getFirst<CKSrvSekensor>();
	CLocManager* locManager = kenv.getGlobal<CLocManager>();
	ImGui::Columns(2);
	ImGui::BeginChild("SekensList");
	int sekIndex = 0;
	for (auto& sek : srvSekensor->sekens) {
		ImGui::PushID(sek.get());
		if (ImGui::Selectable("##SekensSel", sek.get() == selectedSekens.get()))
			selectedSekens = sek.get();
		ImGui::SameLine();
		ImGui::Text("%2i: %s", sekIndex++, kenv.getObjectName(sek.get()));
		ImGui::PopID();
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("SekensInfo");
	if (selectedSekens) {
		auto selectedSekensIterator = std::find_if(srvSekensor->sekens.begin(), srvSekensor->sekens.end(), [](kanyobjref& ref) {return ref.get() == selectedSekens.get(); });
		int selectedSekensIndex = selectedSekensIterator - srvSekensor->sekens.begin();
		if (ImGui::BeginTabBar("SekensBar")) {
			if (ImGui::BeginTabItem("Simple")) {
				static int selectedLanguage = 0;
				ImGui::InputInt("Language", &selectedLanguage);
				IGObjectNameInput("Name", selectedSekens.get(), kenv);
				if (ImGui::BeginTable("LineTable", 5)) {
					ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("LocDuration", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("Unk3", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("Text ID", ImGuiTableColumnFlags_WidthFixed, 54.0f);
					ImGui::TableSetupColumn("Localized text");
					ImGui::TableHeadersRow();
					size_t numLines = (kenv.version < kenv.KVERSION_OLYMPIC) ? selectedSekens->sekLines.size() : selectedSekens->ogLines.size();
					float totalLocDuration = 0.0f;
					int ogStdTextIndex = 0;
					for (size_t i = 0; i < numLines; ++i) {
						int* pIndex = nullptr;
						float* pDuration = nullptr;
						float* pUnk = nullptr;
						if (kenv.version < kenv.KVERSION_OLYMPIC) {
							pIndex = (int*)&selectedSekens->sekLines[i].mUnk0;
							pDuration = &selectedSekens->sekLines[i].mUnk1;
							pUnk = &selectedSekens->sekLines[i].mUnk2;
						}
						else if (kenv.version >= kenv.KVERSION_OLYMPIC) {
							pDuration = &selectedSekens->ogLines[i]->skbkUnk1;
							if (auto* block = selectedSekens->ogLines[i]->dyncast<CKSekensBlock>()) {
								pIndex = &ogStdTextIndex;
								pUnk = &block->skbkUnk2;
								auto& strList = locManager->stdStringRefs;
								if (auto it = std::find(strList.begin(), strList.end(), block->skbkTextRef); it != strList.end())
									ogStdTextIndex = it - strList.begin();
								else
									ogStdTextIndex = -1;
							}
							
						}
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::PushID(i);
						ImGui::SetNextItemWidth(48.0f);
						ImGui::InputScalar("##duration", ImGuiDataType_Float, pDuration);
						ImGui::TableNextColumn();
						if (g_localeEditor) {
							try {
								float locDuration = g_localeEditor->documents.at(selectedLanguage).lvlLocpacks.at(levelNum).get<Loc_CKSrvSekensor>()->locSekens.at(selectedSekensIndex).locLines.at(i).duration;
								ImGui::Text("%.3f", locDuration);
								totalLocDuration += locDuration;
							}
							catch (const std::out_of_range&) {
								ImGui::TextUnformatted("/");
							}
						}
						ImGui::TableNextColumn();
						if (pUnk) {
							ImGui::SetNextItemWidth(48.0f);
							ImGui::InputScalar("##unk", ImGuiDataType_Float, pUnk);
						}
						ImGui::TableNextColumn();
						if (pIndex) {
							ImGui::SetNextItemWidth(48.0f);
							bool modified = ImGui::InputScalar("##LineID", ImGuiDataType_S32, pIndex);
							if (modified && kenv.version >= kenv.KVERSION_OLYMPIC) {
								CKObject* newPtr = (0 <= *pIndex && (size_t)*pIndex < locManager->stdStringRefs.size()) ? locManager->stdStringRefs[*pIndex].get() : nullptr;
								selectedSekens->ogLines[i]->dyncast<CKSekensBlock>()->skbkTextRef = newPtr;
							}
						}
						ImGui::TableNextColumn();
						if (pIndex) {
							const char* text = "Please open the Localization window first.";
							if (*pIndex == -1) {
								text = "/";
							}
							else if (g_localeEditor) {
								try {
									text = g_localeEditor->documents.at(selectedLanguage).stdTextU8.at(*pIndex).c_str();
								}
								catch (const std::out_of_range&) {
									text = "(not found)";
								}
							}
							ImGui::TextUnformatted(text);
						}
						ImGui::PopID();
					}
					ImGui::EndTable();
					ImGui::Text("Total localized duration: %f", totalLocDuration);
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Advanced")) {
				ImGuiMemberListener igml{ kenv, *this };
				igml.setPropertyInfoList(g_encyclo, selectedSekens.get());
				selectedSekens->virtualReflectMembers(igml, &kenv);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
}

template<typename Func, typename First, typename ... Rest> void EI_ReflectAnyReflectableObject(const Func& f, CKObject* obj)
{
	if (auto* s = obj->dyncast<First>())
		f(s);
	else if constexpr (sizeof...(Rest) > 0)
		EI_ReflectAnyReflectableObject<Func, Rest...>(f, obj);
}

void EditorInterface::IGObjectInspector()
{
	static KWeakRef<CKObject> selectedObjectRef;
	kobjref<CKObject> comboObject = selectedObjectRef.get();
	IGObjectSelectorRef(kenv, "Object", comboObject);
	if (comboObject.get() != selectedObjectRef.get())
		selectedObjectRef = comboObject.get();
	ImGui::Separator();
	if (CKObject* obj = selectedObjectRef.get()) {
		ImGui::BeginChild("ObjReflection");
		ImGuiMemberListener ml{ kenv, *this };
		ml.setPropertyInfoList(g_encyclo, obj);
		auto f = [&](auto s) {s->virtualReflectMembers(ml, &kenv); };
		EI_ReflectAnyReflectableObject<decltype(f),
			CKReflectableManager, CKReflectableService, CKHook, CKGroup, CKReflectableComponent,
			CKCameraBase, CKCinematicNode, CKReflectableLogic, CKReflectableGraphical>(f, obj);
		if (auto* s = obj->dyncast<CKReflectableGameDef>())
			s->reflectLevel(ml, &kenv);
		ImGui::EndChild();
	}
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
				for (auto &marker : list) {
					auto rsi = getRaySphereIntersection(camera.position, rayDir, marker.position, 0.5f);
					if (rsi.first) {
						rayHits.push_back(std::make_unique<MarkerSelection>(*this, rsi.second, &marker));
					}
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
