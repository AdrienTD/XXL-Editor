#include "EditorInterface.h"
#include "KEnvironment.h"
#include "imgui/imgui.h"
#include "imguiimpl.h"
#include <SDL2/SDL.h>
#define NOMINMAX
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
#include "imgui/ImGuizmo.h"
#include "GameLauncher.h"
#include "Shape.h"
#include "CKService.h"
#include <INIReader.h>
#include "rw.h"
#include "WavDocument.h"

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

	void DrawSceneNode(CKSceneNode *node, const Matrix &transform, Renderer *gfx, ProGeoCache &geocache, ProTexDict *texdict, CCloneManager *clm, bool showTextures, bool showInvisibles, bool showClones)
	{
		if (!node)
			return;
		Matrix nodeTransform = node->transform;
		nodeTransform.m[0][3] = nodeTransform.m[1][3] = nodeTransform.m[2][3] = 0.0f;
		nodeTransform.m[3][3] = 1.0f;
		Matrix globalTransform = nodeTransform * transform;
		if (showInvisibles || !(node->unk1 & 2)) {
			if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
				if (showClones) {
					auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const kobjref<CSGBranch> &ref) {return ref.get() == node; });
					assert(it != clm->_clones.end());
					size_t clindex = it - clm->_clones.begin();
					gfx->setTransformMatrix(globalTransform);
					for (uint32_t part : clm->_team.dongs[clindex].bongs)
						if (part != 0xFFFFFFFF) {
							RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump->atomic.geometry.get();
							geocache.getPro(rwgeo, texdict)->draw(showTextures);
						}
				}
			}
			else if (node->isSubclassOf<CNode>()) {
				gfx->setTransformMatrix(globalTransform);
				for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
					if (RwMiniClump *rwminiclp = kgeo->clump)
						if (RwGeometry *rwgeo = rwminiclp->atomic.geometry.get())
							geocache.getPro(rwgeo, texdict)->draw(showTextures);
				}
			}
		}
		if (node->isSubclassOf<CSGBranch>())
			DrawSceneNode(node->cast<CSGBranch>()->child.get(), globalTransform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones);
		DrawSceneNode(node->next.get(), transform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones);
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
				for (const std::pair<const Vector3 &, float> pe : { std::make_pair(highCorner,1), std::make_pair(lowCorner,-1) }) {
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
					Vector3 hc = lc + Vector3(1.0f, beacon.params & 7, 1.0f);
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
			hrr->unk1 = hbe->unk1;
			hrr->life = hbe->life;
			hrr->node = hbe->node;

			hrr->unk1 = hbe->unk1;
			hrr->unk2 = hbe->unk2;
			hrr->unk3 = hbe->unk3;
			hrr->unk4 = hbe->unk4;
			hrr->unk5 = hbe->unk5;
			hrr->squad = hbe->squad;
			hrr->unk7 = hbe->unk7;
			hrr->unk8 = hbe->unk8;
			hrr->unk9 = hbe->unk9;
			hrr->unkA = hbe->unkA;
			hrr->shadowCpnt = hbe->shadowCpnt;
			hrr->hkWaterFx = hbe->hkWaterFx;

			hrr->sunk1 = hbe->sunk1;
			hrr->sunk2 = hbe->sunk2;
			hrr->sunk3 = hbe->sunk3;
			hrr->sunk4 = hbe->sunk4;
			hrr->boundingShapes = hbe->boundingShapes;
			hrr->particlesNodeFx1 = hbe->particlesNodeFx1;
			hrr->particlesNodeFx2 = hbe->particlesNodeFx2;
			hrr->particlesNodeFx3 = hbe->particlesNodeFx3;
			hrr->fogBoxNode = hbe->fogBoxNode;
			hrr->sunused = hbe->sunused;
			hrr->hero = hbe->hero;
			hrr->romanAnimatedClone = hbe->romanAnimatedClone;
			hrr->sunk5 = hbe->sunk5;
			hrr->sunk6 = hbe->sunk6;

			hrr->matrix33 = hbe->matrix33;
			hrr->sunk7 = hbe->sunk7;

			hrr->beClone1 = hbe->beClone1;
			hrr->beClone2 = hbe->beClone2;
			hrr->beClone3 = hbe->beClone3;
			hrr->beClone4 = hbe->beClone4;
			hrr->beParticleNode1 = hbe->beParticleNode1;
			hrr->beParticleNode2 = hbe->beParticleNode2;
			hrr->beParticleNode3 = hbe->beParticleNode3;
			hrr->beParticleNode4 = hbe->beParticleNode4;
			hrr->beAnimDict = hbe->beAnimDict;
			hrr->beSoundDict = hbe->beSoundDict;
			hrr->beBoundNode = hbe->beBoundNode;

			hrr->romanAnimatedClone2 = hbe->romanAnimatedClone2;
			hrr->beUnk1 = hbe->beUnk1;
			hrr->beUnk2 = hbe->beUnk2;
			hrr->romanAnimatedClone3 = hbe->romanAnimatedClone3;
			hrr->beUnk3 = hbe->beUnk3;
			hrr->beUnk4 = hbe->beUnk4;
			hrr->beUnk5 = hbe->beUnk5;
			hrr->beUnk6 = hbe->beUnk6;

			// TODO: Rocket-specific values
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
			hrr->rrSoundDictID = sdid;

			hrr->rrParticleNode = kenv.levelObjects.getFirst<CKCrateCpnt>()->particleNode.get();
			hrr->rrAnimDict = hrr->beAnimDict.get();
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

	void InitSnd(int freq) {
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
		spec.format = AUDIO_S16;
		spec.channels = 1;
		spec.samples = 4096;
		audiodevid = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);
		assert(audiodevid);
		SDL_PauseAudioDevice(audiodevid, 0);
		audioInitDone = true;
		audioLastFreq = freq;
	}

	void PlaySnd(KEnvironment &kenv, RwSound &snd) {
		InitSnd(snd.info.dings[0].sampleRate);
		SDL_QueueAudio(audiodevid, snd.data.data.data(), snd.data.data.size());
	}

	RwClump CreateClumpFromGeo(RwGeometry &rwgeo, RwExtHAnim *hanim = nullptr) {
		RwClump clump;

		RwFrame frame;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 3; j++)
				frame.matrix[i][j] = (i == j) ? 1.0f : 0.0f;
		frame.index = 0xFFFFFFFF;
		frame.flags = 0;
		clump.frameList.frames.push_back(frame);
		clump.frameList.extensions.emplace_back();

		clump.geoList.geometries.push_back(&rwgeo);

		RwAtomic *atom = new RwAtomic;
		atom->frameIndex = 0;
		atom->geoIndex = 0;
		atom->flags = 5;
		atom->unused = 0;
		clump.atomics.push_back(atom);

		if (hanim) {
			frame.index = 0;
			clump.frameList.frames.push_back(frame);
			RwsExtHolder freh;
			RwExtHAnim *haclone = (RwExtHAnim*)hanim->clone();
			haclone->nodeId = 0;
			freh.exts.push_back(haclone);
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

				RwExtHAnim *bha = new RwExtHAnim;
				bha->version = 0x100;
				bha->nodeId = bn.first;
				RwsExtHolder reh;
				reh.exts.push_back(bha);
				clump.frameList.extensions.push_back(std::move(reh));
			}
		}
		return clump;
	}

	std::string OpenDialogBox(Window *window, const char *filter, const char *defExt) {
		char filepath[MAX_PATH + 1] = "\0";
		OPENFILENAME ofn = {};
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)window->getNativeWindow();
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = sizeof(filepath);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = defExt;
		if (GetOpenFileNameA(&ofn))
			return filepath;
		return std::string();
	}

	std::string SaveDialogBox(Window *window, const char *filter, const char *defExt, const char *defName = nullptr) {
		char filepath[MAX_PATH + 1] = "\0";
		if(defName)
			strcpy_s(filepath, defName);
		OPENFILENAME ofn = {};
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)window->getNativeWindow();
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = sizeof(filepath);
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = defExt;
		if (GetSaveFileNameA(&ofn))
			return filepath;
		return std::string();
	}

	std::string latinToUtf8(const char *text) {
		// latin -> UTF-16
		int widesize = MultiByteToWideChar(1252, 0, text, -1, NULL, 0);
		wchar_t *widename = (wchar_t*)alloca(2*widesize);
		MultiByteToWideChar(1252, 0, text, -1, widename, widesize);

		// UTF-16 -> UTF-8
		int u8size = WideCharToMultiByte(CP_UTF8, 0, widename, widesize, NULL, 0, NULL, NULL);
		char *u8name = (char*)alloca(u8size);
		WideCharToMultiByte(CP_UTF8, 0, widename, widesize, u8name, u8size, NULL, NULL);

		return std::string(u8name);
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
}

// Selection classes

struct NodeSelection : UISelection {
	static const int ID = 1;

	CKSceneNode *node;

	NodeSelection(EditorInterface &ui, Vector3 &hitpos, CKSceneNode *node) : UISelection(ui, hitpos), node(node) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return true; }
	Matrix getTransform() override {
		Matrix mat = node->transform;
		for (int i = 0; i < 4; i++)
			mat.m[i][3] = (i == 3) ? 1.0f : 0.0f;
		return mat;
	}
	void setTransform(const Matrix &mat) override { node->transform = mat; }
};

struct BeaconSelection : UISelection {
	static const int ID = 2;

	CKBeaconKluster::Beacon *beacon;
	CKBeaconKluster *kluster;

	BeaconSelection(EditorInterface &ui, Vector3 &hitpos, CKBeaconKluster::Beacon *beacon, CKBeaconKluster *kluster) : UISelection(ui, hitpos), beacon(beacon), kluster(kluster) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return true; }
	Matrix getTransform() override { return Matrix::getTranslationMatrix(beacon->getPosition()); }
	void setTransform(const Matrix &mat) override {
		beacon->setPosition(mat.getTranslationVector());
		UpdateBeaconKlusterBounds(kluster);
	}
};

struct GroundSelection : UISelection {
	static const int ID = 3;

	CGround *ground;

	GroundSelection(EditorInterface &ui, Vector3 &hitpos, CGround *gnd) : UISelection(ui, hitpos), ground(gnd) {}
	
	int getTypeID() override { return ID; }
};

struct SquadSelection : UISelection {
	static const int ID = 4;

	CKGrpSquadEnemy *squad;

	SquadSelection(EditorInterface &ui, Vector3 &hitpos, CKGrpSquadEnemy *squad) : UISelection(ui, hitpos), squad(squad) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return true; }
	Matrix getTransform() override { return squad->mat1; }
	void setTransform(const Matrix &mat) override { squad->mat1 = mat; }
};

struct ChoreoSpotSelection : UISelection {
	static const int ID = 5;

	CKGrpSquadEnemy *squad; int spotIndex;

	ChoreoSpotSelection(EditorInterface &ui, Vector3 &hitpos, CKGrpSquadEnemy *squad, int spotIndex) : UISelection(ui, hitpos), squad(squad), spotIndex(spotIndex) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override {
		if (ui.showingChoreoKey < 0 || ui.showingChoreoKey >= squad->choreoKeys.size()) return false;
		if (spotIndex < 0 || spotIndex >= squad->choreoKeys[ui.showingChoreoKey]->slots.size()) return false;
		return true;
	}
	Matrix getTransform() override {
		return Matrix::getTranslationMatrix(squad->choreoKeys[ui.showingChoreoKey]->slots[spotIndex].position) * squad->mat1;
	}
	void setTransform(const Matrix &mat) override {
		Matrix inv = squad->mat1.getInverse4x3();
		squad->choreoKeys[ui.showingChoreoKey]->slots[spotIndex].position = (mat * inv).getTranslationVector();
	}
};

struct MarkerSelection : UISelection {
	static const int ID = 6;

	CKSrvMarker::Marker *marker;

	MarkerSelection(EditorInterface &ui, Vector3 &hitpos, CKSrvMarker::Marker *marker) : UISelection(ui, hitpos), marker(marker) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return true; }
	Matrix getTransform() override { return Matrix::getTranslationMatrix(marker->position); }
	void setTransform(const Matrix &mat) override { marker->position = mat.getTranslationVector(); }
};

EditorInterface::EditorInterface(KEnvironment & kenv, Window * window, Renderer * gfx, INIReader &config)
	: kenv(kenv), g_window(window), gfx(gfx), protexdict(gfx), progeocache(gfx), gndmdlcache(gfx),
	launcher(config.Get("XXL-Editor", "gamemodule", "./GameModule_MP_windowed.exe"), kenv.gamePath)
{
	lastFpsTime = SDL_GetTicks() / 1000;

	auto loadModel = [](const char *fn) -> RwClump * {
		RwClump *clp = new RwClump;
		File *dff = GetResourceFile(fn);
		rwCheckHeader(dff, 0x10);
		clp->deserialize(dff);
		delete dff;
		return clp;
	};

	sphereModel.reset(loadModel("sphere.dff"));
	swordModel.reset(loadModel("sword.dff"));
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
	cloneSet.clear();
	if (CCloneManager *cloneMgr = kenv.levelObjects.getFirst<CCloneManager>())
		if (cloneMgr->_numClones > 0)
			for (auto &dong : cloneMgr->_team.dongs)
				cloneSet.insert(dong.bongs);
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
	if (g_window->getKeyDown(SDL_SCANCODE_UP) || g_window->getKeyDown(SDL_SCANCODE_W))
		camera.position += camera.direction * camspeed;
	if (g_window->getKeyDown(SDL_SCANCODE_DOWN) || g_window->getKeyDown(SDL_SCANCODE_S))
		camera.position -= camera.direction * camspeed;
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
		//selNode = nullptr;
		//selBeacon = nullptr;
		//selBeaconKluster = nullptr;
		//selGround = nullptr;
		checkMouseRay();
		if (nearestRayHit) {
			if (NodeSelection *ns = nearestRayHit->cast<NodeSelection>()) {
				selNode = ns->node;
			}
			else if (BeaconSelection *bs = nearestRayHit->cast<BeaconSelection>()) {
				selBeacon = bs->beacon;
				selBeaconKluster = bs->kluster;
			}
			else if (GroundSelection *gs = nearestRayHit->cast<GroundSelection>()) {
				selGround = gs->ground;
			}
			else if (SquadSelection *ss = nearestRayHit->cast<SquadSelection>()) {
				selectedSquad = ss->squad;
			}
			else if (ChoreoSpotSelection *css = nearestRayHit->cast<ChoreoSpotSelection>()) {
				selectedSquad = css->squad;
			}
			else if (MarkerSelection *ms = nearestRayHit->cast<MarkerSelection>()) {
				selectedMarker = ms->marker;
			}
		}
	}

	static int gzoperation = ImGuizmo::TRANSLATE;
	if(!ImGuizmo::IsUsing())
		gzoperation = g_window->isCtrlPressed() ? ImGuizmo::ROTATE : (g_window->isShiftPressed() ? ImGuizmo::SCALE : ImGuizmo::TRANSLATE);
	ImGuizmo::BeginFrame();
	ImGuizmo::SetRect(0, 0, g_window->getWidth(), g_window->getHeight());

	if (nearestRayHit) {
		const auto &selection = nearestRayHit;
		if (selection->hasTransform()) {
			Matrix gzmat = selection->getTransform();
			Matrix originalMat = gzmat;
			Matrix delta;
			ImGuizmo::Manipulate(camera.viewMatrix.v, camera.projMatrix.v, (ImGuizmo::OPERATION)gzoperation, ImGuizmo::WORLD, gzmat.v, delta.v);
			if (gzmat != originalMat)
				selection->setTransform(gzmat);
		}
	}

	ImGui::Begin("Main");
#ifdef XEC_APPVEYOR
	ImGui::Text("AppVeyor Build %i, by AdrienTD, FPS %i", XEC_APPVEYOR, lastFps);
#else
	ImGui::Text("Version 0.0.0.3 by AdrienTD, FPS: %i", lastFps);
#endif
	ImGui::BeginTabBar("MainTabBar", 0);
	if (ImGui::BeginTabItem("Main")) {
		IGMain();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Textures")) {
		IGTextureEditor();
		ImGui::EndTabItem();
	}
	//if (ImGui::BeginTabItem("Geo")) {
	//	IGGeometryViewer();
	//	ImGui::EndTabItem();
	//}
	if (ImGui::BeginTabItem("Clones")) {
		IGCloneEditor();
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
		IGBeaconGraph();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Grounds")) {
		IGGroundEditor();
		ImGui::EndTabItem();
	}
	if (kenv.hasClass<CKSrvEvent>()) {
		if (ImGui::BeginTabItem("Events")) {
			IGEventEditor();
			ImGui::EndTabItem();
		}
	}
	if (ImGui::BeginTabItem("Sounds")) {
		IGSoundEditor();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Squads")) {
		IGSquadEditor();
		ImGui::EndTabItem();
	}
	//if (ImGui::BeginTabItem("Hooks")) {
	//	IGHookEditor();
	//	ImGui::EndTabItem();
	//}
	if (ImGui::BeginTabItem("Pathfinding")) {
		IGPathfindingEditor();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Markers")) {
		IGMarkerEditor();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Objects")) {
		IGObjectTree();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Misc")) {
		IGMiscTab();
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

	CCloneManager *clm = kenv.levelObjects.getFirst<CCloneManager>();

	if (clm && !selClones.empty()) {
		gfx->setTransformMatrix(Matrix::getTranslationMatrix(selgeoPos) * camera.sceneMatrix);
		for (uint32_t ci : selClones)
			if(ci != 0xFFFFFFFF)
				progeocache.getPro(clm->_teamDict._bings[ci]._clump->atomic.geometry.get(), &protexdict)->draw();
	}

	if (showNodes) {
		CSGSectorRoot *rootNode = kenv.levelObjects.getObject<CSGSectorRoot>(0);
		DrawSceneNode(rootNode, camera.sceneMatrix, gfx, progeocache, &protexdict, clm, showTextures, showInvisibleNodes, showClones);
		for (int str = 0; str < kenv.numSectors; str++) {
			CSGSectorRoot * strRoot = kenv.sectorObjects[str].getObject<CSGSectorRoot>(0);
			DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, &str_protexdicts[str], clm, showTextures, showInvisibleNodes, showClones);
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
		auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const kobjref<CSGBranch> &ref) {return ref.get() == node; });
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

	auto drawBeaconKluster = [this, clm, &getCloneIndex, &drawClone, &drawBox](CKBeaconKluster* bk) {
		if (showBeaconKlusterBounds) {
			gfx->setTransformMatrix(camera.sceneMatrix);
			gfx->unbindTexture(0);
			float rd = bk->bounds.radius;
			drawBox(bk->bounds.center + Vector3(rd, rd, rd), bk->bounds.center - Vector3(rd, rd, rd));
		}
		if (showBeacons) {
			for (auto &bing : bk->bings) {
				if (!bing.active)
					continue;
				uint32_t handlerFID = bing.handler->getClassFullID();
				for (auto &beacon : bing.beacons) {
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					if (handlerFID == CKCrateCpnt::FULL_ID) {
						int numCrates = beacon.params & 7;

						CKCrateCpnt *cratecpnt = bing.handler->cast<CKCrateCpnt>();
						size_t clindex = getCloneIndex(cratecpnt->crateNode->cast<CClone>());

						for (int c = 0; c < numCrates; c++) {
							gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos + Vector3(0, 0.5f + c, 0)) * camera.sceneMatrix);
							drawClone(clindex);
						}
					}
					else if (bing.handler->isSubclassOf<CKGrpBonusPool>()) {
						CKGrpBonusPool *pool = bing.handler->cast<CKGrpBonusPool>();
						CKHook *hook = pool->childHook.get();

						size_t clindex = getCloneIndex(hook->node->cast<CSGBranch>());

						// rotation
						Matrix rotmat = Matrix::getRotationYMatrix(SDL_GetTicks() * 3.1415f / 1000.0f);

						gfx->setTransformMatrix(rotmat * Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
						drawClone(clindex);
					}
					else {
						gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
						int c = 255 - (SDL_GetTicks() % 1000) * 128 / 1000;
						gfx->setBlendColor(0xFF000000 | c);
						progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
						gfx->setBlendColor(0xFFFFFFFF);
					}
				}
			}
		}
	};
	for (CKBeaconKluster *bk = kenv.levelObjects.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
		drawBeaconKluster(bk);
	for (auto &str : kenv.sectorObjects)
		if (str.getClassType<CKBeaconKluster>().objects.size())
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

	float alpha = SDL_GetTicks() * 3.1416f / 1000.f;
	Vector3 v1(-cos(alpha), -1, -sin(alpha)), v2(cos(alpha), -1, sin(alpha)), v3(0, 1, 0);
	Vector3 rayDir = getRay(camera, g_window);
	auto res = getRayTriangleIntersection(camera.position, rayDir, v3, v1, v2);
	uint32_t color = res.first ? 0xFF0000FF : 0xFFFFFFFF;
	gfx->setTransformMatrix(camera.sceneMatrix);
	gfx->unbindTexture(0);
	gfx->drawLine3D(v1, v2, color);
	gfx->drawLine3D(v2, v3, color);
	gfx->drawLine3D(v3, v1, color);

	if (nearestRayHit) {
		const Vector3 rad = Vector3(1, 1, 1) * 0.1f;
		drawBox(nearestRayHit->hitPosition + rad, nearestRayHit->hitPosition - rad);
	}

	if (showGroundBounds) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		auto drawGroundBounds = [this,&drawBox](CGround* gnd) {
			auto &b = gnd->aabb;
			drawBox(Vector3(b[0], b[1], b[2]), Vector3(b[3], b[4], b[5]), (selGround == gnd) ? 0xFF00FF00 : 0xFFFFFFFF);
		};
		for (CKObject* obj : kenv.levelObjects.getClassType<CGround>().objects)
			drawGroundBounds(obj->cast<CGround>());
		for (auto &str : kenv.sectorObjects)
			for (CKObject *obj : str.getClassType<CGround>().objects)
				drawGroundBounds(obj->cast<CGround>());
	}

	if (showGrounds) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		auto drawGround = [this](CGround* gnd) {
			if (selGround == gnd) gfx->setBlendColor(0xFF00FF00);
			gndmdlcache.getModel(gnd)->draw(showInfiniteWalls);
			if (selGround == gnd) gfx->setBlendColor(0xFFFFFFFF);
		};
		for (CKObject* obj : kenv.levelObjects.getClassType<CGround>().objects)
			drawGround(obj->cast<CGround>());
		for (auto &str : kenv.sectorObjects)
			for (CKObject *obj : str.getClassType<CGround>().objects)
				drawGround(obj->cast<CGround>());
	}

	// CKLine
	if (showLines) {
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
	if (showLines) {
		auto drawSpline = [this](CKSpline4L* kl) {
			for (size_t i = 0; i < kl->dings.size()-1; i++)
				gfx->drawLine3D(kl->dings[i], kl->dings[i + 1]);
		};
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		for (CKObject* obj : kenv.levelObjects.getClassType<CKSpline4L>().objects)
			drawSpline(obj->cast<CKSpline4L>());
		for (auto &str : kenv.sectorObjects)
			for (CKObject *obj : str.getClassType<CKSpline4L>().objects)
				drawSpline(obj->cast<CKSpline4L>());
	}

	CKGroup *grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>();

	if (showSquadBoxes && grpEnemy) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		//for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
		for(CKGroup *osquad = grpEnemy->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
			if (!osquad->isSubclassOf<CKGrpSquadEnemy>()) continue;
			CKGrpSquadEnemy *squad = osquad->cast<CKGrpSquadEnemy>();
			for (const auto &bb : { squad->sqUnk3, squad->sqUnk4 }) {
				Vector3 v1(bb[0], bb[1], bb[2]);
				Vector3 v2(bb[3], bb[4], bb[5]);
				drawBox(v1-v2*0.5f, v1+v2*0.5f);
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
			if (showingChoreoKey < squad->choreoKeys.size()) {
				CKChoreoKey *ckey = squad->choreoKeys[showingChoreoKey].get();
				const Matrix &gmat = squad->mat1;
				for (auto &slot : ckey->slots) {
					Vector3 spos = slot.position.transform(gmat);
					drawBox(spos - Vector3(1, 1, 1), spos + Vector3(1, 1, 1));
				}
			}
		}
	}

	if (showPFGraph) {
		if (CKSrvPathFinding *srvpf = kenv.levelObjects.getFirst<CKSrvPathFinding>()) {
			gfx->setTransformMatrix(camera.sceneMatrix);
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
							gfx->drawLine3D(pfnode->lowBBCorner + Vector3(x, h, z)*cellsize, pfnode->lowBBCorner + Vector3(x + 1, h, z + 1)*cellsize, ddcolor);
							gfx->drawLine3D(pfnode->lowBBCorner + Vector3(x + 1, h, z)*cellsize, pfnode->lowBBCorner + Vector3(x, h, z + 1)*cellsize, ddcolor);
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

	if (showMarkers) {
		if (CKSrvMarker *srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>()) {
			gfx->setBlendColor(0xFFFFFF00);
			for (auto &list : srvMarker->lists) {
				for (auto &marker : list) {
					gfx->setTransformMatrix(Matrix::getTranslationMatrix(marker.position) * camera.sceneMatrix);
					progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
				}
			}
		}
	}
}

void EditorInterface::IGObjectSelector(const char * name, CKObject ** ptr, uint32_t clfid)
{
	char tbuf[80] = "(null)";
	CKObject *obj = *ptr;
	if(obj)
		sprintf_s(tbuf, "%p : %i %i %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName());
	if (ImGui::BeginCombo(name, tbuf, 0)) {
		for (uint32_t clcatnum = 0; clcatnum < 15; clcatnum++) {
			if (clfid != 0xFFFFFFFF && (clfid & 63) != clcatnum)
				continue;
			auto &clcat = kenv.levelObjects.categories[clcatnum];
			for (uint32_t clid = 0; clid < clcat.type.size(); clid++) {
				if (clfid != 0xFFFFFFFF && (clfid >> 6) != clid)
					continue;
				auto &cl = clcat.type[clid];
				for (CKObject *eo : cl.objects) {
					ImGui::PushID(eo);
					if (ImGui::Selectable("##objsel", eo == obj)) {
						obj->release();
						eo->addref();
						*ptr = eo;
					}
					ImGui::SameLine();
					ImGui::Text("%p : %i %i %s", eo, eo->getClassCategory(), eo->getClassID(), eo->getClassName());
					ImGui::PopID();
				}
			}
		}
		ImGui::EndCombo();
	}
}

void EditorInterface::IGMain()
{
	static int levelNum = 8;
	ImGui::InputInt("Level number##LevelNum", &levelNum);
	if (ImGui::Button("Load")) {
		selGeometry = nullptr;
		selNode = nullptr;
		selBeacon = nullptr;
		selBeaconKluster = nullptr;
		selGround = nullptr;
		selectedSquad = nullptr;
		selectedPFGraphNode = nullptr;
		selectedMarker = nullptr;
		selClones.clear();
		rayHits.clear();
		nearestRayHit = nullptr;

		progeocache.clear();
		gndmdlcache.clear();
		kenv.loadLevel(levelNum);
		prepareLevelGfx();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) {
		kenv.saveLevel(levelNum);
	}
	ImGui::SameLine();
	if (ImGui::Button("Test")) {
		launcher.loadLevel(levelNum);
	}
	ImGui::Separator();
	ImGui::DragFloat3("Cam pos", &camera.position.x, 0.1f);
	ImGui::DragFloat3("Cam ori", &camera.orientation.x, 0.1f);
	ImGui::DragFloat("Cam speed", &_camspeed, 0.1f);
	ImGui::DragFloatRange2("Depth range", &camera.nearDist, &camera.farDist);
	ImGui::Checkbox("Orthographic", &camera.orthoMode); ImGui::SameLine();
	if (ImGui::Button("Top-down view")) {
		camera.orientation = Vector3(-1.5707f, 3.1416f, 0.0f);
	}
	ImGui::Checkbox("Show scene nodes", &showNodes); ImGui::SameLine();
	ImGui::Checkbox("Show textures", &showTextures);
	ImGui::Checkbox("Show invisibles", &showInvisibleNodes); ImGui::SameLine();
	ImGui::Checkbox("Show clones", &showClones);
	ImGui::Checkbox("Beacons", &showBeacons); ImGui::SameLine();
	ImGui::Checkbox("Beacon kluster bounds", &showBeaconKlusterBounds); //ImGui::SameLine();
	ImGui::Checkbox("Grounds", &showGrounds); ImGui::SameLine();
	ImGui::Checkbox("Ground bounds", &showGroundBounds); ImGui::SameLine();
	ImGui::Checkbox("Infinite walls", &showInfiniteWalls);
	ImGui::Checkbox("Sas bounds", &showSasBounds); ImGui::SameLine();
	ImGui::Checkbox("Lines & splines", &showLines); //ImGui::SameLine();
	ImGui::Checkbox("Squads + choreos", &showSquadChoreos); ImGui::SameLine();
	ImGui::Checkbox("Squad bounds", &showSquadBoxes);
	ImGui::InputInt("Choreo key", &showingChoreoKey);
	ImGui::Checkbox("Pathfinding graph", &showPFGraph); ImGui::SameLine();
	ImGui::Checkbox("Markers", &showMarkers);
}

void EditorInterface::IGMiscTab()
{
	ImGui::Checkbox("Show ImGui Demo", &showImGuiDemo);
	if (ImGui::Button("Rocket Romans \\o/"))
		GimmeTheRocketRomans(kenv);
	if(ImGui::IsItemHovered())
		ImGui::SetTooltip("Transform all Basic Enemies to Rocket Romans");
	if (ImGui::Button("Export Component Values to CSV")) {
		CKBasicEnemyCpnt *firstcpnt = kenv.levelObjects.getFirst<CKBasicEnemyCpnt>();
		if (firstcpnt) {
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
			fprintf(csv, "Index\t");
			firstcpnt->reflectMembers(nl);
			int index = 0;
			for (CKObject *obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
				fprintf(csv, "\n%i\t", index);
				obj->cast<CKBasicEnemyCpnt>()->reflectMembers(vl);
				index++;
			}
			fclose(csv);
		}
	}
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
					int n = 0;
					for (CKObject *obj : cl.objects) {
						if (ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%s (%i, %i) %i, refCount=%i", obj->getClassName(), obj->getClassCategory(), obj->getClassID(), n, obj->getRefCount()))
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
}

void EditorInterface::IGBeaconGraph()
{
	static const char *beaconX1Names[] = {
		"*",				// 00
		"*",				// 01
		"*",				// 02
		"Wooden Crate",		// 03
		"Metal Crate",		// 04
		"?",				// 05
		"Helmet",			// 06
		"Golden Helmet",	// 07
		"Potion",			// 08
		"Shield",			// 09
		"Ham",				// 0a
		"x3 Multiplier",	// 0b
		"x10 Multiplier",	// 0c
		"Laurel",			// 0d
		"Boar",				// 0e
		"Water flow",		// 0f
		"Merchant",			// 10
		"*",				// 11
		"*",				// 12
		"*",				// 13
		"*",				// 14
		"Save point",		// 15
		"Respawn point",	// 16
		"Hero respawn pos",	// 17
	};
	static auto getBeaconName = [](int handlerId) -> const char * {
		if (handlerId < 0x18)
			return beaconX1Names[handlerId];
		return "!";
	};
	if (ImGui::Button("Add beacon")) {
		ImGui::OpenPopup("AddBeacon");
	}
	ImGui::SameLine();
	if (ImGui::Button("Update all kluster sphere bounds")) {
		for (CKObject *bk : kenv.levelObjects.getClassType<CKBeaconKluster>().objects)
			UpdateBeaconKlusterBounds(bk->cast<CKBeaconKluster>());
		for(auto &str : kenv.sectorObjects)
			for (CKObject *bk : str.getClassType<CKBeaconKluster>().objects)
				UpdateBeaconKlusterBounds(bk->cast<CKBeaconKluster>());
	}
	/*
	if (ImGui::Button("List beacon sectors")) {
		CKSrvBeacon *srv = kenv.levelObjects.getFirst<CKSrvBeacon>();
		int i = 0;
		for (auto &bs : srv->beaconSectors) {
			printf("-------- BS %i --------\n", i);
			printf("nusedbings:%u, nbings:%u, unk:%u, nbits:%u\n", bs.numUsedBings, bs.numBings, bs.beaconArraySize, bs.numBits);
			int totUsedBings = 0, totBings = 0, totBeacons = 0, totBits = 0;
			for (auto &bk : bs.beaconKlusters) {
				totUsedBings += bk->numUsedBings;
				totBings += bk->bings.size();
				for (auto &bing : bk->bings) {
					totBeacons += bing.beacons.size();
					totBits += bing.numBits * bing.beacons.size();
				}
			}
			printf("totUsedBings:%u totBings:%u totBeacons:%u totBits:%u\n", totUsedBings, totBings, totBeacons, totBits);
		}
	}
	*/
	if(ImGui::BeginPopup("AddBeacon")) {
		CKSrvBeacon *srv = kenv.levelObjects.getFirst<CKSrvBeacon>();
		for (auto &hs : srv->handlers) {
			//char buf[128];
			//sprintf_s(buf, "%s (%02X %02X %02X %02X %02X)", name, hs.unk2a, hs.numBits, hs.handlerIndex, hs.handlerId, hs.persistent);
			if (ImGui::MenuItem(getBeaconName(hs.handlerId))) {
				int bki = kenv.levelObjects.getClassType<CKBeaconKluster>().objects.size();
				CKBeaconKluster *kluster = kenv.createObject<CKBeaconKluster>(-1);

				srv->beaconSectors[0].beaconKlusters.push_back(kluster);
				srv->beaconSectors[0].numUsedBings++;
				srv->beaconSectors[0].numBings += srv->handlers.size();
				srv->beaconSectors[0].beaconArraySize += 8;
				int bx = srv->beaconSectors[0].bits.size();
				for(int i = 0; i < hs.numBits; i++)
					srv->beaconSectors[0].bits.push_back(true /*i == 0*/);
				CKSector *str = kenv.levelObjects.getFirst<CKSector>();
				CKBeaconKluster *prev = nullptr;
				for(CKBeaconKluster *pk = kenv.levelObjects.getFirst<CKBeaconKluster>(); pk; pk = pk->nextKluster.get())
					prev = pk;
				if (prev)
					prev->nextKluster = kluster;
				else
					str->beaconKluster = kluster;

				kluster->numUsedBings = 1;
				kluster->bings.resize(srv->handlers.size());
				for (auto &bing : kluster->bings)
					bing.active = false;
				auto &bing = kluster->bings[hs.handlerIndex];
				bing.active = true;
				bing.handler = hs.object.get();
				bing.unk2a = hs.unk2a;
				bing.numBits = hs.numBits;
				bing.handlerId = hs.handlerId;
				bing.sectorIndex = 0;
				bing.klusterIndex = bki;
				bing.handlerIndex = hs.handlerIndex;
				bing.bitIndex = bx;
				bing.unk6 = 0x128c;	// some class id?
				CKBeaconKluster::Beacon beacon;
				beacon.setPosition(camera.position + camera.direction * 2.5f);
				beacon.params = 0xA;
				bing.beacons.push_back(beacon);
				UpdateBeaconKlusterBounds(kluster);
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(%02X %02X %02X %02X %02X)", hs.unk2a, hs.numBits, hs.handlerIndex, hs.handlerId, hs.persistent);
		}
		ImGui::EndPopup();
	}
	auto enumBeaconKluster = [this](CKBeaconKluster* bk) {
		if (ImGui::TreeNode(bk, "pos (%f, %f, %f) radius %f", bk->bounds.center.x, bk->bounds.center.y, bk->bounds.center.z, bk->bounds.radius)) {
			ImGui::DragFloat3("Center##beaconKluster", &bk->bounds.center.x, 0.1f);
			ImGui::DragFloat("Radius##beaconKluster", &bk->bounds.radius, 0.1f);
			for (auto &bing : bk->bings) {
				int boffi = bing.bitIndex;
				if(!bing.beacons.empty())
					ImGui::Text("%02X %02X %02X %02X %02X %02X %04X %08X", bing.unk2a, bing.numBits, bing.handlerId, bing.sectorIndex, bing.klusterIndex, bing.handlerIndex, bing.bitIndex, bing.unk6);
				for (auto &beacon : bing.beacons) {
					ImGui::PushID(&beacon);
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					bool tn_open = ImGui::TreeNodeEx("beacon", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf, "(%i,%i) %f %f %f 0x%04X", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z, beacon.params);
					//if (ImGui::Selectable("##beacon")) {
					if (ImGui::IsItemClicked()) {
						camera.position = pos - camera.direction * 5.0f;
						selBeacon = &beacon;
						selBeaconKluster = bk;
					}
					if (tn_open) {
						ImGui::TreePop();
					}
					//ImGui::SameLine();
					//ImGui::Text("(%i,%i) %f %f %f", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z);
					ImGui::PopID();
					boffi += bing.numBits;
				}
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
	int i = 0;
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
	if (selBeacon && selBeaconKluster) {
		CKBeaconKluster *bk = (CKBeaconKluster*)selBeaconKluster;
		CKBeaconKluster::Beacon &beacon = *(CKBeaconKluster::Beacon*)selBeacon;

		// find bing + boffi
		CKBeaconKluster::Bing *fndbing = nullptr;
		int boffi;
		for (CKBeaconKluster::Bing &cing : bk->bings) {
			boffi = cing.bitIndex;
			for (CKBeaconKluster::Beacon &ceacon : cing.beacons) {
				if (&beacon == &ceacon) {
					fndbing = &cing; break;
				}
				boffi += cing.numBits;
			}
			if (fndbing) break;
		}
		assert(fndbing);
		CKBeaconKluster::Bing &bing = *fndbing;

		CKSrvBeacon *srvBeacon = kenv.levelObjects.getFirst<CKSrvBeacon>();
		ImGui::Text("%s (%02X, %s)", getBeaconName(bing.handlerId), bing.handlerId, bing.handler->getClassName());
		ImGui::Text("Bits:");
		for (int i = 0; i < bing.numBits; i++) {
			ImGui::SameLine();
			ImGui::Text("%i", srvBeacon->beaconSectors[bing.sectorIndex].bits[boffi + i] ? 1 : 0);
		}
		bool mod = false;
		mod |= ImGui::DragScalarN("Position##beacon", ImGuiDataType_S16, &beacon.posx, 3, 0.1f);
		mod |= ImGui::InputScalar("Params##beacon", ImGuiDataType_U16, &beacon.params, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
			int cc = beacon.params & 7;
			if (ImGui::InputInt("Num crates", &cc)) {
				beacon.params &= ~7;
				beacon.params |= (cc & 7);
				mod = true;
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
}

void EditorInterface::IGGeometryViewer()
{
	ImGui::DragFloat3("Geo pos", &selgeoPos.x, 0.1f);
	if (ImGui::Button("Move geo to front"))
		selgeoPos = camera.position + camera.direction * 3;
	if (selGeometry) {
		ImGui::SameLine();
		if (ImGui::Button("Import DFF")) {
			std::string filepath = OpenDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				RwClump *impClump = LoadDFF(filepath.c_str());
				if (selGeoCloneIndex == -1)
					*selGeometry = *impClump->geoList.geometries[0];
				else {
					std::vector<std::unique_ptr<RwGeometry>> geos = impClump->geoList.geometries[0]->splitByMaterial();
					CCloneManager *cloneMgr = kenv.levelObjects.getFirst<CCloneManager>();
					std::vector<uint32_t> dictIndices;
					for (auto &dong : cloneMgr->_team.dongs) {
						if (std::find(dong.bongs.begin(), dong.bongs.end(), selGeoCloneIndex) != dong.bongs.end()) {
							dictIndices = dong.bongs;
							break;
						}
					}
					int p = 0;
					for (uint32_t x : dictIndices) {
						if (x == 0xFFFFFFFF)
							continue;
						cloneMgr->_teamDict._bings[x]._clump->atomic.geometry = std::move(geos[p++]);
						if (p >= geos.size())
							break;
					}
					selGeometry = nullptr;
				}
				progeocache.clear();
			}
		}
		ImGui::SameLine();
		if (selGeometry && ImGui::Button("Export DFF")) {
			CCloneManager *cloneMgr = kenv.levelObjects.getFirst<CCloneManager>();
			for (auto &dong : cloneMgr->_team.dongs) {
				if (std::find(dong.bongs.begin(), dong.bongs.end(), selGeoCloneIndex) != dong.bongs.end()) {
					RwFrameList *framelist = &dong.clump.frameList;
					RwExtHAnim *hanim = (RwExtHAnim*)framelist->extensions[1].find(0x11E);	// null if not found

					// merge clone geos
					RwGeometry mergedGeo; bool first = true;
					for (auto td : dong.bongs) {
						if (td == 0xFFFFFFFF)
							continue;
						RwGeometry &tdgeo = *cloneMgr->_teamDict._bings[td]._clump->atomic.geometry.get();
						if (first) {
							mergedGeo = tdgeo;
							first = false;
						}
						else {
							mergedGeo.merge(tdgeo);
						}
					}

					RwClump clump = CreateClumpFromGeo(mergedGeo, hanim);
					IOFile out("clone.dff", "wb");
					clump.serialize(&out);
					out.close();
					break;
				}
			}
		}
	}

	ImGui::BeginChild("RwGeoSelection");
	auto enumRwGeo = [this](RwGeometry *rwgeo, int i, bool isClone = false) {
		std::string fndname = "?";
		if (rwgeo->materialList.materials.size())
			fndname = rwgeo->materialList.materials[0].texture.name;
		ImGui::PushID(i);
		if (ImGui::Selectable("##rwgeo", selGeometry == rwgeo)) {
			selGeometry = rwgeo;
			selGeoCloneIndex = isClone ? i : -1;
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
					enumRwGeo(bing._clump->atomic.geometry.get(), i++, true);
				}
				ImGui::PopID();
				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();
}

void EditorInterface::IGTextureEditor()
{
	static int currentTexDict = -1;
	ImGui::InputInt("Sector", &currentTexDict);
	CTextureDictionary *texDict;
	ProTexDict *cur_protexdict;
	if (currentTexDict >= 0 && currentTexDict < kenv.numSectors) {
		texDict = kenv.sectorObjects[currentTexDict].getFirst<CTextureDictionary>();
		cur_protexdict = &str_protexdicts[currentTexDict];
	}
	else {
		texDict = kenv.levelObjects.getObject<CTextureDictionary>(0);
		cur_protexdict = &protexdict;
		currentTexDict = -1;
	}
	if (selTexID >= texDict->textures.size())
		selTexID = texDict->textures.size() - 1;
	if (ImGui::Button("Insert")) {
		std::string filepath = OpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
		if (!filepath.empty()) {
			AddTexture(kenv, filepath.c_str());
			cur_protexdict->reset(texDict);
		}
	}
	ImGui::SameLine();
	if ((selTexID != -1) && ImGui::Button("Replace")) {
		std::string filepath = OpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
		if (!filepath.empty()) {
			texDict->textures[selTexID].image = RwImage::loadFromFile(filepath.c_str());
			cur_protexdict->reset(texDict);
		}
	}
	ImGui::SameLine();
	if ((selTexID != -1) && ImGui::Button("Remove")) {
		texDict->textures.erase(texDict->textures.begin() + selTexID);
		cur_protexdict->reset(texDict);
	}
	ImGui::SameLine();
	if ((selTexID != -1) && ImGui::Button("Export")) {
		auto &tex = texDict->textures[selTexID];
		std::string filepath = SaveDialogBox(g_window, "PNG Image\0*.PNG\0\0", "png", tex.name);
		if (!filepath.empty()) {
			RwImage cimg = tex.image.convertToRGBA32();
			stbi_write_png(filepath.c_str(), cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Export all")) {
		char dirname[MAX_PATH + 1], pname[MAX_PATH + 1];
		BROWSEINFOA bri;
		memset(&bri, 0, sizeof(bri));
		bri.hwndOwner = (HWND)g_window->getNativeWindow();
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
		if (ImGui::Selectable("##texsel", i == selTexID, 0, ImVec2(0, 32))) {
			selTexID = i;
		}
		ImGui::SameLine();
		ImGui::Image(cur_protexdict->find(texDict->textures[i].name).second, ImVec2(32, 32));
		ImGui::SameLine();
		ImGui::Text("%s\n%i*%i*%i", tex.name, tex.image.width, tex.image.height, tex.image.bpp);
		ImGui::PopID();
		i++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("TexViewer", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	if (selTexID != -1) {
		auto &tex = texDict->textures[selTexID];
		ImGui::Image(cur_protexdict->find(tex.name).second, ImVec2(tex.image.width, tex.image.height));
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGEnumNode(CKSceneNode *node, const char *description)
{
	if (!node)
		return;
	bool hassub = false;
	if (node->isSubclassOf<CSGBranch>())
		hassub = node->cast<CSGBranch>()->child.get();
	bool open = ImGui::TreeNodeEx(node, (hassub ? 0 : ImGuiTreeNodeFlags_Leaf) | ((selNode == node) ? ImGuiTreeNodeFlags_Selected : 0), "%s %s", node->getClassName(), description);
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
	IGEnumNode(lvlroot, "Common sector");
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
			std::string filepath = OpenDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				RwClump *impClump = LoadDFF(filepath.c_str());

				// Remove current geometry
				CKAnyGeometry *kgeo = geonode->geometry.get();
				geonode->geometry.reset();
				while (kgeo) {
					CKAnyGeometry *next = kgeo->nextGeo.get();
					kenv.removeObject(kgeo);
					kgeo = next;
				}

				// Create new geometry
				CKAnyGeometry *prevgeo = nullptr;
				for (RwAtomic *atom : impClump->atomics) {
					RwGeometry *rwgeotot = impClump->geoList.geometries[atom->geoIndex];
					auto splitgeos = rwgeotot->splitByMaterial();
					for (auto &rwgeo : splitgeos) {
						if (rwgeo->tris.empty())
							continue;
						rwgeo->flags &= ~0x60;
						rwgeo->materialList.materials[0].color = 0xFFFFFFFF;
						CKAnyGeometry *newgeo;
						if (geonode->isSubclassOf<CAnimatedNode>())
							newgeo = kenv.createObject<CKSkinGeometry>(-1);
						else
							newgeo = kenv.createObject<CKGeometry>(-1);
						if (prevgeo) prevgeo->nextGeo = kobjref<CKAnyGeometry>(newgeo);
						else geonode->geometry.reset(newgeo);
						prevgeo = newgeo;
						newgeo->flags = 1;
						newgeo->flags2 = 0;
						newgeo->clump = new RwMiniClump;
						newgeo->clump->atomic.flags = 5;
						newgeo->clump->atomic.unused = 0;
						newgeo->clump->atomic.geometry = std::move(rwgeo);
					}
				}

				progeocache.clear();
			}
		}
		if (ImGui::Button("Export geometry to DFF")) {
			std::string filepath = SaveDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				CKAnyGeometry *kgeo = geonode->geometry.get();
				RwGeometry rwgeo = *kgeo->clump->atomic.geometry.get();
				kgeo = kgeo->nextGeo.get();
				while (kgeo) {
					rwgeo.merge(*kgeo->clump->atomic.geometry);
					kgeo = kgeo->nextGeo.get();
				}

				RwExtHAnim *hanim = nullptr;
				if (geonode->isSubclassOf<CAnimatedNode>()) {
					RwFrameList *framelist = geonode->cast<CAnimatedNode>()->frameList;
					hanim = (RwExtHAnim*)framelist->extensions[0].find(0x11E);
					assert(hanim);
				}

				RwClump clump = CreateClumpFromGeo(rwgeo, hanim);

				printf("done\n");
				IOFile dff(filepath.c_str(), "wb");
				clump.serialize(&dff);
			}
		}
	}
}

void EditorInterface::IGGroundEditor()
{
	//if (ImGui::Button("Find duplicates in klusters")) {
	//	std::vector<KObjectList*> olvec;
	//	olvec.push_back(&kenv.levelObjects);
	//	for (auto &str : kenv.sectorObjects)
	//		olvec.push_back(&str);
	//	for (int i = 0; i < olvec.size(); i++) {
	//		CKMeshKluster *mk1 = olvec[i]->getFirst<CKMeshKluster>();
	//		for (int j = i + 1; j < olvec.size(); j++) {
	//			CKMeshKluster *mk2 = olvec[j]->getFirst<CKMeshKluster>();
	//			int k = 0;
	//			for (auto &gnd : mk2->grounds) {
	//				if (gnd->isSubclassOf<CDynamicGround>())
	//					continue;
	//				auto it = std::find(mk1->grounds.begin(), mk1->grounds.end(), gnd);
	//				if (it != mk1->grounds.end()) {
	//					printf("str_%i[%i] == str_%i[%i]\n", i, it - mk1->grounds.begin(), j, k);
	//				}
	//				k++;
	//			}
	//		}
	//	}
	//}
	//ImGui::SameLine();
	static bool hideDynamicGrounds = true;
	ImGui::Checkbox("Hide dynamic", &hideDynamicGrounds);
	ImGui::Columns(2);
	ImGui::BeginChild("GroundTree");
	auto feobjlist = [this](KObjectList &objlist, const char *desc) {
		if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
			ImGui::PushID(mkluster);
			bool tropen = ImGui::TreeNode(mkluster, "%s", desc);
			ImGui::SameLine();
			if (ImGui::SmallButton("Export")) {
				std::string filepath = SaveDialogBox(g_window, "Wavefront OBJ file\0*.OBJ\0\0", "obj");
				if (!filepath.empty()) {
					FILE *obj;
					fopen_s(&obj, filepath.c_str(), "wt");
					uint16_t gndx = 0;
					uint32_t basevtx = 1;
					for (const auto &gnd : mkluster->grounds) {
						if (gnd->isSubclassOf<CDynamicGround>() && hideDynamicGrounds)
							continue;
						fprintf(obj, "o Gnd%04u/flags\n", gndx++);
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
							fprintf(obj, "f %u %u %u %u\n", basevtx + wall.baseIndices[0], basevtx + wall.baseIndices[1], wallvtx + 1, wallvtx);
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
					bool p = ImGui::TreeNodeEx(gnd.get(), ImGuiTreeNodeFlags_Leaf | ((gnd.get() == selGround) ? ImGuiTreeNodeFlags_Selected : 0), "%s %u %u %f %f", type, gnd->param1, gnd->param2, gnd->param3, gnd->param4);
					if (ImGui::IsItemClicked())
						selGround = gnd.get();
					if (p)
						ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
	};
	feobjlist(kenv.levelObjects, "Level");
	int x = 0;
	for (auto &str : kenv.sectorObjects) {
		char lol[64];
		sprintf_s(lol, "Sector %i", x++);
		feobjlist(str, lol);
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("SelGroundInfo");
	if (selGround) {
		auto CheckboxFlags16 = [](const char *label, uint16_t *flags, unsigned int val) {
			unsigned int up = *flags;
			if (ImGui::CheckboxFlags(label, &up , val))
				*flags = up;
		};
		ImGui::InputScalar("param1", ImGuiDataType_U16, &selGround->param1, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("param2", ImGuiDataType_U16, &selGround->param2, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("param3", ImGuiDataType_Float, &selGround->param3);
		ImGui::InputScalar("param4", ImGuiDataType_Float, &selGround->param4);
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
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::IGEventEditor()
{
	CKSrvEvent *srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>();
	if (!srvEvent) return;

	size_t ev = 0;
	for (auto &bee : srvEvent->bees) {
		if (ImGui::TreeNodeEx(&bee, ImGuiTreeNodeFlags_DefaultOpen, "%02X %02X", bee._1, bee._2)) {
			for (uint8_t i = 0; i < bee._1; i++) {
				CKObject *obj = srvEvent->objs[ev + i].get();
				ImGui::Text("%04X -> %p (%i, %i) %s", srvEvent->objInfos[ev+i], obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName());
			}
			ImGui::TreePop();
		}
		ev += bee._1;
	}
}

void EditorInterface::IGSoundEditor()
{
	static auto exportSound = [](RwSound &snd, const char *path) {
		WavDocument wav;
		wav.formatTag = 1;
		wav.numChannels = 1;
		wav.samplesPerSec = snd.info.dings[0].sampleRate;
		wav.avgBytesPerSec = wav.samplesPerSec * 2;
		wav.pcmBitsPerSample = 16;
		wav.blockAlign = ((wav.pcmBitsPerSample + 7) / 8) * wav.numChannels;
		wav.data = snd.data.data;
		IOFile out = IOFile(path, "wb");
		wav.write(&out);
	};
	auto enumDict = [this](CKSoundDictionary *sndDict, int strnum) {
		if (sndDict->sounds.empty())
			return;
		if (ImGui::TreeNode(sndDict, (strnum == -1) ? "Level" : "Sector %i", strnum)) {
			if (ImGui::Button("Random shuffle")) {
				CKSoundDictionary *sndDict = kenv.levelObjects.getFirst<CKSoundDictionary>();
				std::random_shuffle(sndDict->rwSoundDict.list.sounds.begin(), sndDict->rwSoundDict.list.sounds.end());
			}
			ImGui::SameLine();
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
						exportSound(snd, pname);
					}
				}
			}
			for (int sndid = 0; sndid < sndDict->sounds.size(); sndid++) {
				auto &snd = sndDict->rwSoundDict.list.sounds[sndid];
				ImGui::PushID(sndid);
				if (ImGui::ArrowButton("PlaySound", ImGuiDir_Right))
					PlaySnd(kenv, snd);
				if(ImGui::IsItemHovered()) ImGui::SetTooltip("Play");
				ImGui::SameLine();
				if (ImGui::Button("I")) {
					std::string filepath = OpenDialogBox(g_window, "WAV audio file\0*.WAV\0\0", "wav");
					if (!filepath.empty()) {
						IOFile wf = IOFile(filepath.c_str(), "rb");
						WavDocument wav;
						wav.read(&wf);
						WavSampleReader wsr(&wav);
						if (wav.formatTag == 1 || wav.formatTag == 3) {
							if (wav.numChannels != 1) {
								MessageBox((HWND)g_window->getNativeWindow(), "The WAV contains multiple channels (e.g. stereo).\nOnly the first channel will be imported.", "XXL Editor", 48);
							}

							size_t numSamples = wav.getNumSamples();
							auto &ndata = snd.data.data;
							ndata.resize(numSamples * 2);
							int16_t *pnt = (int16_t*)ndata.data();
							for (int i = 0; i < numSamples; i++)
								*(pnt++) = (int16_t)(wsr.nextSample() * 32767);

							for (auto &ding : snd.info.dings) {
								ding.sampleRate = wav.samplesPerSec;
								ding.dataSize = ndata.size();
							}
							sndDict->sounds[sndid].sampleRate = wav.samplesPerSec;
						}
						else {
							MessageBox((HWND)g_window->getNativeWindow(), "The WAV file doesn't contain uncompressed mono 8/16-bit PCM wave data.\nPlease convert it to this format first.", "XXL Editor", 48);
						}
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Import");
				ImGui::SameLine();
				if (ImGui::Button("E")) {
					const char *name = strrchr((const char *)snd.info.name.data(), '\\');
					if (name) name++;
					std::string filepath = SaveDialogBox(g_window, "WAV audio file\0*.WAV\0\0", "wav", name);
					if (!filepath.empty()) {
						exportSound(snd, filepath.c_str());
					}
				}
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("Export");
				ImGui::SameLine();
				const char *name = (const char*)snd.info.name.data();
				ImGui::Text("%s", latinToUtf8(name).c_str());
				//ImGui::Text("%u %u", snd.info.dings[0].sampleRate, snd.info.dings[1].sampleRate);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	};
	enumDict(kenv.levelObjects.getFirst<CKSoundDictionary>(), -1);
	for (int i = 0; i < kenv.numSectors; i++)
		enumDict(kenv.sectorObjects[i].getFirst<CKSoundDictionary>(), i);
}

void EditorInterface::IGSquadEditor()
{
	ImGui::Columns(2);
	ImGui::BeginChild("SquadList");
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
		ImGui::SameLine();
		ImGui::Text("%s %i (%i)", jetpack ? "JetPack Squad" : "Squad", si, numEnemies);
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
		CKGrpSquadEnemy *squad = selectedSquad;
		ImGui::BeginTabBar("SquadInfoBar");
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
			auto choreoString = [squad](int key) -> std::string {
				int cindex = 0, kindex = 0;
				for (auto &choreo : squad->choreographies) {
					if (key >= kindex && key < kindex + choreo->numKeys) {
						char tbuf[48];
						sprintf_s(tbuf, "Choreo %i (key %i-%i)", cindex, kindex, kindex + choreo->numKeys - 1);
						return std::string(tbuf);
					}
					kindex += choreo->numKeys;
					cindex++;
				}
				return "(Invalid choreo key)";
			};
			if (ImGui::BeginCombo("Choreography", choreoString(showingChoreoKey).c_str())) {
				int cindex = 0;
				int kindex = 0;
				for (auto &choreo : squad->choreographies) {
					ImGui::PushID(&choreo);
					if (ImGui::Selectable("##ChoreoEntry"))
						showingChoreoKey = kindex;
					ImGui::SameLine();
					ImGui::TextUnformatted(choreoString(kindex).c_str());
					ImGui::PopID();
					cindex++;
					kindex += choreo->numKeys;
				}
				ImGui::EndCombo();
			}
			ImGui::InputInt("ChoreoKey", &showingChoreoKey);
			int ckeyindex = showingChoreoKey;
			if (ckeyindex >= 0 && ckeyindex < squad->choreoKeys.size()) {
				auto &ckey = squad->choreoKeys[ckeyindex];
				//if (ImGui::TreeNode(&ckey, "Key %u %f %f %f %u", ckey->slots.size(), ckey->unk1, ckey->unk2, ckey->unk3, ckey->flags)) {
				ImGui::Separator();
				ImGui::DragFloat("Duration", &ckey->unk1);
				ImGui::DragFloat("Unk2", &ckey->unk2);
				ImGui::DragFloat("Unk3", &ckey->unk3);
				if (ImGui::Button("Add spot")) {
					ckey->slots.emplace_back();
				}
				ImGui::SameLine();
				if (ImGui::Button("Randomize orientations")) {
					for (auto &slot : ckey->slots) {
						float angle = (rand() & 255) * 3.1416f / 128.0f;
						slot.direction = Vector3(cos(angle), 0, sin(angle));
					}
				}
				ImGui::BeginChild("ChoreoSlots", ImVec2(0,0), true);
				for (auto &slot : ckey->slots) {
					ImGui::PushID(&slot);
					ImGui::DragFloat3("Position", &slot.position.x, 0.1f);
					ImGui::DragFloat3("Direction", &slot.direction.x, 0.1f);
					ImGui::InputScalar("Enemy pool", ImGuiDataType_S8, &slot.enemyGroup);
					ImGui::PopID();
					ImGui::Separator();
				}
				ImGui::EndChild();
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Pools")) {
			static size_t currentPoolInput = 0;
			if (ImGui::Button("Duplicate")) {
				if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
					CKGrpSquadEnemy::PoolEntry duppe = squad->pools[currentPoolInput];
					duppe.cpnt = duppe.cpnt->clone(kenv, -1)->cast<CKEnemyCpnt>();
					squad->pools.push_back(duppe);
				}
			}
			ImGui::SetNextItemWidth(-1.0f);
			ImGui::ListBoxHeader("##PoolList");
			for (int i = 0; i < squad->pools.size(); i++) {
				ImGui::PushID(i);
				if (ImGui::Selectable("##PoolSel", i == currentPoolInput))
					currentPoolInput = i;
				ImGui::SameLine();
				auto &pe = squad->pools[i];
				ImGui::Text("%s %u %u", pe.cpnt->getClassName(), pe.numEnemies, pe.u1);
				ImGui::PopID();
			}
			ImGui::ListBoxFooter();
			if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
				auto &pe = squad->pools[currentPoolInput];
				ImGui::BeginChild("SquadPools");
				ImGui::BulletText("%s %u %u %u", pe.cpnt->getClassName(), pe.u1, pe.u2, pe.u3.get() ? 1 : 0);
				IGObjectSelectorRef("Pool", pe.pool);
				ImGui::InputScalar("Enemy Count", ImGuiDataType_U16, &pe.numEnemies);
				ImGui::InputScalar("U1", ImGuiDataType_U8, &pe.u1);
				ImGui::InputScalar("U2", ImGuiDataType_U8, &pe.u2);
				if (pe.cpnt->isSubclassOf<CKEnemyCpnt>()) {
					CKEnemyCpnt *cpnt = pe.cpnt->cast<CKEnemyCpnt>();
					IGComponentEditor(cpnt);
				}
				ImGui::EndChild();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::Columns();
}

void EditorInterface::IGEnumGroup(CKGroup *group)
{
	if (!group)
		return;
	if (ImGui::TreeNode(group, "%s", group->getClassName())) {
		IGEnumGroup(group->childGroup.get());
		//for (CKHook *hook = group->childHook.get(); hook; hook = hook->next.get())
		//	if (ImGui::TreeNodeEx(hook, ImGuiTreeNodeFlags_Leaf, "%s", hook->getClassName()))
		//		ImGui::TreePop();
		ImGui::TreePop();
	}
	IGEnumGroup(group->nextGroup.get());
}

void EditorInterface::IGHookEditor()
{
	IGEnumGroup(kenv.levelObjects.getFirst<CKGroupRoot>());
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
			std::string filepath = OpenDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				RwClump *impClump = LoadDFF(filepath.c_str());
				std::vector<std::unique_ptr<RwGeometry>> geos = impClump->geoList.geometries[0]->splitByMaterial();

				int p = 0;
				for (uint32_t x : selClones) {
					if (x == 0xFFFFFFFF)
						continue;
					auto &geo = cloneMgr->_teamDict._bings[x]._clump->atomic.geometry;
					if(p < geos.size())
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
			std::string filepath = SaveDialogBox(g_window, "Renderware Clump\0*.DFF\0\0", "dff");
			if (!filepath.empty()) {
				RwTeam::Dong *seldong = nullptr;
				for (auto &dong : cloneMgr->_team.dongs)
					if (dong.bongs == selClones)
						seldong = &dong;

				if (!seldong) {
					MessageBox((HWND)g_window->getNativeWindow(), "Sorry, I couldn't find back the team entry with the selected team dict indices :(", "XXL Editor", 16);
				}
				else {
					RwFrameList *framelist = &seldong->clump.frameList;
					RwExtHAnim *hanim = (RwExtHAnim*)framelist->extensions[1].find(0x11E);	// null if not found

					// merge clone geos
					RwGeometry mergedGeo; bool first = true;
					for (auto td : seldong->bongs) {
						if (td == 0xFFFFFFFF)
							continue;
						const RwGeometry &tdgeo = *cloneMgr->_teamDict._bings[td]._clump->atomic.geometry.get();
						if (first) {
							mergedGeo = tdgeo;
							first = false;
						}
						else {
							mergedGeo.merge(tdgeo);
						}
					}

					RwClump clump = CreateClumpFromGeo(mergedGeo, hanim);
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
				const auto &matlist = cloneMgr->_teamDict._bings[de]._clump->atomic.geometry->materialList.materials;
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
	struct ImGuiMemberListener : MemberListener {
		void reflect(uint8_t &ref, const char *name) override { ImGui::InputScalar(name, ImGuiDataType_U8, &ref); }
		void reflect(uint16_t &ref, const char *name) override { ImGui::InputScalar(name, ImGuiDataType_U16, &ref); }
		void reflect(uint32_t &ref, const char *name) override { ImGui::InputScalar(name, ImGuiDataType_U32, &ref); }
		void reflect(float &ref, const char *name) override { ImGui::InputScalar(name, ImGuiDataType_Float, &ref); }
		void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { ImGui::Text("%s: %p", name, ref._pointer); }
		void reflect(Vector3 &ref, const char *name) override { ImGui::InputFloat3(name, &ref.x, 2); }
	};

	ImGui::PushItemWidth(130.0f);

	ImGuiMemberListener igml;
	cpnt->virtualReflectMembers(igml);

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
		pfnode->highBBCorner = pfnode->lowBBCorner + Vector3(pfnode->numCellsX * 2, 50, pfnode->numCellsZ * 2);
	}
	ImGui::SameLine();
	if (ImGui::Button("Examine")) {
		std::map<uint8_t, int> counts;
		int pid = 0;
		for (auto &pfnode : srvpf->nodes) {
			for (uint8_t &cell : pfnode->cells) {
				counts[cell]++;
				if (cell == 0)
					printf("found 0 at %i\n", pid);
				//if (cell != 7) cell = 4;
			}
			pid++;
		}
		for (auto &me : counts) {
			printf("%u: %i\n", me.first, me.second);
		}
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
	if (CKPFGraphNode *pfnode = selectedPFGraphNode) {
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
		if (ImGui::TreeNode(&list, "List %i", lx)) {
			int mx = 0;
			for (auto &marker : list) {
				ImGui::PushID(&marker);
				if (ImGui::Selectable("##MarkerEntry", selectedMarker == &marker)) {
					selectedMarker = &marker;
				}
				ImGui::SameLine();
				ImGui::Text("Marker %3i (%3u, %3u, %u)", mx, marker.orientation1, marker.orientation2, marker.val3);
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
		ImGui::DragFloat3("Position", &marker->position.x, 0.1f);
		ImGui::InputScalar("Orientation 1", ImGuiDataType_U8, &marker->orientation1);
		ImGui::InputScalar("Orientation 2", ImGuiDataType_U8, &marker->orientation2);
		ImGui::InputScalar("Val3", ImGuiDataType_U16, &marker->val3);
	}
	ImGui::EndChild();
	ImGui::Columns();
}

void EditorInterface::checkNodeRayCollision(CKSceneNode * node, const Vector3 &rayDir, const Matrix &matrix)
{
	if (!node) return;
	
	Matrix nodeTransform = node->transform;
	nodeTransform.m[0][3] = nodeTransform.m[1][3] = nodeTransform.m[2][3] = 0.0f;
	nodeTransform.m[3][3] = 1.0f;
	Matrix globalTransform = nodeTransform * matrix;

	auto checkGeo = [this,node,&rayDir,&globalTransform](RwGeometry *rwgeo) {
		if (rayIntersectsSphere(camera.position, rayDir, rwgeo->spherePos.transform(globalTransform), rwgeo->sphereRadius)) {
			for (auto &tri : rwgeo->tris) {
				std::array<Vector3, 3> trverts;
				for (int i = 0; i < 3; i++)
					trverts[i] = rwgeo->verts[tri.indices[i]].transform(globalTransform);
				auto ixres = getRayTriangleIntersection(camera.position, rayDir, trverts[0], trverts[1], trverts[2]);
				if (ixres.first) {
					CKSceneNode *tosel = node;
					if (!tosel->isSubclassOf<CSGSectorRoot>()) {
						while (!tosel->parent->isSubclassOf<CSGSectorRoot>())
							tosel = tosel->parent.get();
						rayHits.push_back(std::make_unique<NodeSelection>(*this, ixres.second, tosel));
					}
				}
			}
		}
	};

	if (showInvisibleNodes || !(node->unk1 & 2)) {
		if (node->isSubclassOf<CClone>() || node->isSubclassOf<CAnimatedClone>()) {
			if (showClones) {
				CCloneManager *clm = kenv.levelObjects.getFirst<CCloneManager>();
				auto it = std::find_if(clm->_clones.begin(), clm->_clones.end(), [node](const kobjref<CSGBranch> &ref) {return ref.get() == node; });
				assert(it != clm->_clones.end());
				size_t clindex = it - clm->_clones.begin();
				gfx->setTransformMatrix(globalTransform);
				for (uint32_t part : clm->_team.dongs[clindex].bongs)
					if (part != 0xFFFFFFFF) {
						RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump->atomic.geometry.get();
						checkGeo(rwgeo);
					}
			}
		}
		else if (node->isSubclassOf<CNode>() /*&& !node->isSubclassOf<CSGSectorRoot>()*/) {
			for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get())
				if (RwMiniClump *clump = kgeo->clump)
					if (RwGeometry *rwgeo = clump->atomic.geometry.get())
						checkGeo(rwgeo);
		}
	}

	if (node->isSubclassOf<CSGBranch>()) {
		checkNodeRayCollision(node->cast<CSGBranch>()->child.get(), rayDir, globalTransform);
	}
	checkNodeRayCollision(node->next.get(), rayDir, matrix);
}

void EditorInterface::checkMouseRay()
{
	Vector3 rayDir = getRay(camera, g_window);
	numRayHits = 0;
	rayHits.clear();
	nearestRayHit = nullptr;

	auto checkOnSector = [this,&rayDir](KObjectList &objlist) {
		// Nodes
		if(showNodes)
			checkNodeRayCollision(objlist.getFirst<CSGSectorRoot>(), rayDir, Matrix::getIdentity());

		// Beacons
		if (showBeacons) {
			for (CKBeaconKluster *kluster = objlist.getFirst<CKBeaconKluster>(); kluster; kluster = kluster->nextKluster.get()) {
				for (auto &bing : kluster->bings) {
					if (bing.active) {
						for (auto &beacon : bing.beacons) {
							Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
							auto rsi = getRaySphereIntersection(camera.position, rayDir, pos, 0.5f);
							if (rsi.first) {
								rayHits.push_back(std::make_unique<BeaconSelection>(*this, rsi.second, &beacon, kluster));
							}
						}
					}
				}
			}
		}

		// Grounds
		if (showGroundBounds || showGrounds) {
			if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
				for (auto &ground : mkluster->grounds) {
					auto rbi = getRayAABBIntersection(camera.position, rayDir, Vector3(ground->aabb[0], ground->aabb[1], ground->aabb[2]), Vector3(ground->aabb[3], ground->aabb[4], ground->aabb[5]));
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

		// Squads
		if (showSquadChoreos) {
			if (CKGrpEnemy *grpEnemy = kenv.levelObjects.getFirst<CKGrpEnemy>()) {
				for (CKGroup *grp = grpEnemy->childGroup.get(); grp; grp = grp->nextGroup.get()) {
					if (CKGrpSquadEnemy *squad = grp->dyncast<CKGrpSquadEnemy>()) {
						Vector3 sqpos = squad->mat1.getTranslationVector();
						RwGeometry *rwgeo = swordModel->geoList.geometries[0];
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
						if (showingChoreoKey >= 0 && showingChoreoKey < squad->choreoKeys.size()) {
							int spotIndex = 0;
							for (auto &slot : squad->choreoKeys[showingChoreoKey]->slots) {
								Vector3 trpos = slot.position.transform(squad->mat1);
								auto rbi = getRayAABBIntersection(camera.position, rayDir, trpos + Vector3(1, 1, 1), trpos - Vector3(1, 1, 1));
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
		if (showMarkers) {
			if (CKSrvMarker *srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>()) {
				for (auto &list : srvMarker->lists) {
					for (auto &marker : list) {
						auto rsi = getRaySphereIntersection(camera.position, rayDir, marker.position, 0.5f);
						if (rsi.first) {
							rayHits.emplace_back(new MarkerSelection(*this, rsi.second, &marker));
						}
					}
				}
			}
		}
	};

	checkOnSector(kenv.levelObjects);
	for (auto &str : kenv.sectorObjects)
		checkOnSector(str);

	if (!rayHits.empty()) {
		auto comp = [this](const std::unique_ptr<UISelection> &a, const std::unique_ptr<UISelection> &b) -> bool {
			return (camera.position - a->hitPosition).len3() < (camera.position - b->hitPosition).len3();
		};
		nearestRayHit = std::min_element(rayHits.begin(), rayHits.end(), comp)->get();
	}
}
