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
#include "CKCinematicNode.h"
#include "KLocalObject.h"
#include "CKLocalObjectSubs.h"
#include <io.h>

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

	bool IsNodeInvisible(CKSceneNode *node, bool isXXL2) {
		return isXXL2 ? ((node->unk1 & 4) && !(node->unk1 & 0x10)) : (node->unk1 & 2);
	}

	void DrawSceneNode(CKSceneNode *node, const Matrix &transform, Renderer *gfx, ProGeoCache &geocache, ProTexDict *texdict, CCloneManager *clm, bool showTextures, bool showInvisibles, bool showClones, std::map<CSGBranch*, int> &nodeCloneIndexMap, bool isXXL2)
	{
		if (!node)
			return;
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
							RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump->atomic.geometry.get();
							geocache.getPro(rwgeo, texdict)->draw(showTextures);
						}
				}
			}
			else if (node->isSubclassOf<CNode>()) {
				gfx->setTransformMatrix(globalTransform);
				for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
					CKAnyGeometry *rgeo = kgeo->duplicateGeo ? kgeo->duplicateGeo.get() : kgeo;
					if (RwMiniClump *rwminiclp = rgeo->clump)
						if (RwGeometry *rwgeo = rwminiclp->atomic.geometry.get())
							geocache.getPro(rwgeo, texdict)->draw(showTextures);
				}
			}
			if (node->isSubclassOf<CSGBranch>())
				DrawSceneNode(node->cast<CSGBranch>()->child.get(), globalTransform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones, nodeCloneIndexMap, isXXL2);
			if (CAnyAnimatedNode *anyanimnode = node->dyncast<CAnyAnimatedNode>())
				DrawSceneNode(anyanimnode->branchs.get(), globalTransform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones, nodeCloneIndexMap, isXXL2);
		}
		DrawSceneNode(node->next.get(), transform, gfx, geocache, texdict, clm, showTextures, showInvisibles, showClones, nodeCloneIndexMap, isXXL2);
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

	std::string wcharToUtf8(const wchar_t *text) {
		// UTF-16 -> UTF-8
		int u8size = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
		char *u8name = (char*)alloca(u8size);
		WideCharToMultiByte(CP_UTF8, 0, text, -1, u8name, u8size, NULL, NULL);
		return std::string(u8name);
	}

	std::wstring utf8ToWchar(const char *text) {
		int widesize = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
		wchar_t *widename = (wchar_t*)alloca(2 * widesize);
		MultiByteToWideChar(CP_UTF8, 0, text, -1, widename, widesize);
		return std::wstring(widename);
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

	void ImportGroundOBJ(KEnvironment &kenv, const char *filename, int sector) {
		FILE *wobj;
		fopen_s(&wobj, filename, "rt");
		if (!wobj) return;
		char line[512]; char *context; const char * const spaces = " \t";
		std::vector<Vector3> positions;
		std::vector<CGround::Triangle> triangles; // change int16 to int32 ??
		auto flushTriangles = [&positions, &triangles, &kenv,&sector]() {
			if (!triangles.empty()) {
				CGround *gnd = kenv.createObject<CGround>(sector);
				KObjectList &objlist = (sector == -1) ? kenv.levelObjects : kenv.sectorObjects[sector];
				CKMeshKluster *kluster = objlist.getFirst<CKMeshKluster>();
				CKSector *ksector = kenv.levelObjects.getClassType<CKSector>().objects[sector + 1]->cast<CKSector>();
				kluster->grounds.emplace_back(gnd);
				std::map<Vector3, int> posmap;
				uint16_t nextIndex = 0;
				for (auto &tri : triangles) {
					CGround::Triangle cvtri;
					for (int c = 0; c < 3; c++) {
						int objIndex = tri.indices[c];
						int cvIndex;
						const Vector3 &objPos = positions[objIndex];
						auto pmit = posmap.find(objPos);
						if (pmit == posmap.end()) {
							posmap[objPos] = nextIndex;
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
				gnd->param1 = 0; gnd->param2 = 1;
				gnd->param3 = gnd->param4 = 0.0f;
				gnd->aabb = AABoundingBox(gnd->vertices[0]);
				for (Vector3 &vec : gnd->vertices) {
					gnd->aabb.mergePoint(vec);
				}
				AABoundingBox safeaabb = gnd->aabb;
				safeaabb.highCorner.y += 10.0f;
				safeaabb.lowCorner.y -= 5.0f;
				kluster->aabb.merge(safeaabb);
				ksector->boundaries.merge(safeaabb);
				triangles.clear();
			}
		};
		while (!feof(wobj)) {
			fgets(line, 511, wobj);
			std::string word = strtok_s(line, spaces, &context);
			if (word == "o") {
				flushTriangles();
			}
			else if (word == "v") {
				Vector3 vec;
				for (float &coord : vec) {
					coord = atof(strtok_s(NULL, spaces, &context));
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
				for (int i = 2; i < face.size(); i++) {
					CGround::Triangle tri;
					tri.indices = { face[0], face[i - 1], face[i] };
					triangles.push_back(std::move(tri));
				}

			}
		}
		flushTriangles();
		fclose(wobj);
	}

	// ImGui InputCallback for std::string
	static const auto IGStdStringInputCallback = [](ImGuiInputTextCallbackData *data) -> int {
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			std::string *str = (std::string*)data->UserData;
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->data();
		}
		return 0;
	};
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

// Creates ImGui editing widgets for every member in a member-reflected object
struct ImGuiMemberListener : MemberListener {
	KEnvironment &kenv; EditorInterface &ui;
	ImGuiMemberListener(KEnvironment &kenv, EditorInterface &ui) : kenv(kenv), ui(ui) {}
	void icon(const char *label, const char *desc = nullptr) {
		ImGui::AlignTextToFramePadding();
		ImGui::TextColored(ImVec4(0, 1, 1, 1), label);
		if (desc && ImGui::IsItemHovered())
			ImGui::SetTooltip(desc);
		ImGui::SameLine();
	}
	void reflect(uint8_t &ref, const char *name) override { icon(" 8", "Unsigned 8-bit integer"); ImGui::InputScalar(name, ImGuiDataType_U8, &ref); }
	void reflect(uint16_t &ref, const char *name) override { icon("16", "Unsigned 16-bit integer"); ImGui::InputScalar(name, ImGuiDataType_U16, &ref); }
	void reflect(uint32_t &ref, const char *name) override { icon("32", "Unsigned 32-bit integer"); ImGui::InputScalar(name, ImGuiDataType_U32, &ref); }
	void reflect(float &ref, const char *name) override { icon("Fl", "IEEE 754 Single floating-point number"); ImGui::InputScalar(name, ImGuiDataType_Float, &ref); }
	void reflectAnyRef(kanyobjref &ref, int clfid, const char *name) override { icon("Rf", "Object reference"); ui.IGObjectSelector(kenv, name, ref, clfid); /*ImGui::Text("%s: %p", name, ref._pointer);*/ }
	void reflect(Vector3 &ref, const char *name) override { icon("V3", "3D Floating-point vector"); ImGui::InputFloat3(name, &ref.x, 2); }
	void reflect(EventNode &ref, const char *name, CKObject *user) override {
		icon("Ev", "Event sequence node");
		ImGui::PushID(name);
		int igtup[2] = { ref.seqIndex, ref.bit };
		float itemwidth = ImGui::CalcItemWidth();
		ImGui::SetNextItemWidth(itemwidth - ImGui::GetStyle().ItemInnerSpacing.x - ImGui::GetFrameHeight());
		if (ImGui::InputInt2("##HkEventBox", igtup)) {
			ref.seqIndex = (int16_t)igtup[0]; ref.bit = (uint8_t)igtup[1] & 7;
		}
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		if (ImGui::ArrowButton("HkSelectEvent", ImGuiDir_Right)) {
			ui.selectedEventSequence = ref.seqIndex;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Select event sequence");
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text(name);
		ImGui::PopID();
	}
	void reflectPostRefTuple(uint32_t &tuple, const char *name) override {
		icon("PR", "Undecoded object reference (Postponed reference)");
		int igtup[3] = { tuple & 63, (tuple >> 6) & 2047, tuple >> 17 };
		if (ImGui::InputInt3(name, igtup)) {
			tuple = (igtup[0] & 63) | ((igtup[1] & 2047) << 6) | ((igtup[2] & 32767) << 17);
		}
	}
	void reflect(std::string &ref, const char *name) override {
		icon("St", "Character string");
		ImGui::InputText(name, (char*)ref.c_str(), ref.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &ref);
	}
};

EditorInterface::EditorInterface(KEnvironment & kenv, Window * window, Renderer * gfx, INIReader &config)
	: kenv(kenv), g_window(window), gfx(gfx), protexdict(gfx), progeocache(gfx), gndmdlcache(gfx),
	launcher(config.Get("XXL-Editor", "gamemodule", "./GameModule_MP_windowed.exe"), kenv.outGamePath)
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
	if (kenv.hasClass<CTextureDictionary>()) {
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
	nodeCloneIndexMap.clear();
	cloneSet.clear();
	if (CCloneManager *cloneMgr = kenv.levelObjects.getFirst<CCloneManager>())
		if (cloneMgr->_numClones > 0) {
			for (int i = 0; i < cloneMgr->_clones.size(); i++)
				nodeCloneIndexMap.insert({cloneMgr->_clones[i].get(), i});
			for (auto &dong : cloneMgr->_team.dongs)
				cloneSet.insert(dong.bongs);
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
	if (!ImGuizmo::IsUsing())
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

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Window")) {
		ImGui::MenuItem("Main", nullptr, &wndShowMain);
		ImGui::MenuItem("Textures", nullptr, &wndShowTextures);
		ImGui::MenuItem("Clones", nullptr, &wndShowClones);
		ImGui::MenuItem("Scene graph", nullptr, &wndShowSceneGraph);
		ImGui::MenuItem("Beacons", nullptr, &wndShowBeacons);
		ImGui::MenuItem("Grounds", nullptr, &wndShowGrounds);
		if(kenv.version <= kenv.KVERSION_XXL1)
			ImGui::MenuItem("Events", nullptr, &wndShowEvents);
		else
			ImGui::MenuItem("Triggers", nullptr, &wndShowTriggers);
		ImGui::MenuItem("Sounds", nullptr, &wndShowSounds);
		ImGui::MenuItem("Squads", nullptr, &wndShowSquads);
		ImGui::MenuItem("Hooks", nullptr, &wndShowHooks);
		ImGui::MenuItem("Pathfinding", nullptr, &wndShowPathfinding);
		if(kenv.version <= kenv.KVERSION_XXL1)
			ImGui::MenuItem("Markers", nullptr, &wndShowMarkers);
		ImGui::MenuItem("Detectors", nullptr, &wndShowDetectors);
		ImGui::MenuItem("Cinematic", nullptr, &wndShowCinematic);
		ImGui::MenuItem("Localization", nullptr, &wndShowLocale);
		ImGui::MenuItem("Objects", nullptr, &wndShowObjects);
		ImGui::MenuItem("Misc", nullptr, &wndShowMisc);
		ImGui::EndMenu();
	}
	ImGui::Spacing();
#ifdef XEC_APPVEYOR
	ImGui::Text("XXL Editor v" XEC_APPVEYOR " (" __DATE__ ") by AdrienTD, FPS %i", lastFps);
#else
	ImGui::Text("XXL Editor Development version, by AdrienTD, FPS: %i", lastFps);
#endif
	ImGui::EndMainMenuBar();

	//ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

	auto igwindow = [this](const char *name, bool *flag, void(*func)(EditorInterface *ui)) {
		if (*flag) {
			if (ImGui::Begin(name, flag))
				func(this);
			ImGui::End();
		}
	};
	igwindow("Main", &wndShowMain, [](EditorInterface *ui) { ui->IGMain(); });
	if (kenv.hasClass<CTextureDictionary>())
		igwindow("Textures", &wndShowTextures, [](EditorInterface *ui) { ui->IGTextureEditor(); });
	if (kenv.hasClass<CCloneManager>())
		igwindow("Clones", &wndShowClones, [](EditorInterface *ui) { ui->IGCloneEditor(); });
	if (kenv.hasClass<CSGSectorRoot>())
		igwindow("Scene graph", &wndShowSceneGraph, [](EditorInterface *ui) {
			ImGui::Columns(2);
			ImGui::BeginChild("SceneNodeTree");
			ui->IGSceneGraph();
			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild("SceneNodeProperties");
			ui->IGSceneNodeProperties();
			ImGui::EndChild();
			ImGui::Columns();
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
	if (kenv.hasClass<CKGrpEnemy>())
		igwindow("Squads", &wndShowSquads, [](EditorInterface *ui) { ui->IGSquadEditor(); });
	if (kenv.hasClass<CKGroupRoot>())
		igwindow("Hooks", &wndShowHooks, [](EditorInterface *ui) { ui->IGHookEditor(); });
	if (kenv.hasClass<CKSrvPathFinding>())
		igwindow("Pathfinding", &wndShowPathfinding, [](EditorInterface *ui) { ui->IGPathfindingEditor(); });
	if (kenv.hasClass<CKSrvMarker>())
		igwindow("Markers", &wndShowMarkers, [](EditorInterface *ui) { ui->IGMarkerEditor(); });
	if (kenv.hasClass<CKSrvDetector>())
		igwindow("Detectors", &wndShowDetectors, [](EditorInterface *ui) { ui->IGDetectorEditor(); });
	if (kenv.hasClass<CKSrvCinematic>())
		igwindow("Cinematic", &wndShowCinematic, [](EditorInterface *ui) { ui->IGCinematicEditor(); });
	igwindow("Localization", &wndShowLocale, [](EditorInterface *ui) { ui->IGLocaleEditor(); });
	igwindow("Objects", &wndShowObjects, [](EditorInterface *ui) { ui->IGObjectTree(); });
	igwindow("Misc", &wndShowMisc, [](EditorInterface *ui) { ui->IGMiscTab(); });

#ifndef XEC_RELEASE
	if (showImGuiDemo)
		ImGui::ShowDemoWindow(&showImGuiDemo);
#endif
}

void EditorInterface::render()
{
	if (kenv.version == kenv.KVERSION_XXL1) {
		if (CKHkSkyLife* hkSkyLife = kenv.levelObjects.getFirst<CKHkSkyLife>()) {
			gfx->setBackgroundColor(hkSkyLife->skyColor);
		}
	}

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

	if (showNodes && kenv.hasClass<CSGSectorRoot>()) {
		CSGSectorRoot *rootNode = kenv.levelObjects.getObject<CSGSectorRoot>(0);
		bool isXXL2 = kenv.version >= 2;
		DrawSceneNode(rootNode, camera.sceneMatrix, gfx, progeocache, &protexdict, clm, showTextures, showInvisibleNodes, showClones, nodeCloneIndexMap, isXXL2);
		if (showingSector < 0) {
			for (int str = 0; str < kenv.numSectors; str++) {
				CSGSectorRoot * strRoot = kenv.sectorObjects[str].getObject<CSGSectorRoot>(0);
				DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, &str_protexdicts[str], clm, showTextures, showInvisibleNodes, showClones, nodeCloneIndexMap, isXXL2);
			}
		} else if(showingSector < kenv.numSectors) {
			CSGSectorRoot * strRoot = kenv.sectorObjects[showingSector].getObject<CSGSectorRoot>(0);
			DrawSceneNode(strRoot, camera.sceneMatrix, gfx, progeocache, &str_protexdicts[showingSector], clm, showTextures, showInvisibleNodes, showClones, nodeCloneIndexMap, isXXL2);
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
				RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump->atomic.geometry.get();
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
				if (!bing.active)
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
							gfx->setTransformMatrix(Matrix::getTranslationMatrix(pos + Vector3(0, c, 0)) * camera.sceneMatrix);
							drawClone(clindex);
						}
					}
					else if (bing.handler->isSubclassOf<CKGrpBonusPool>() && kenv.hasClass<CSGRootNode>()) {
						CKGrpBonusPool *pool = bing.handler->cast<CKGrpBonusPool>();
						CKHook *hook = pool->childHook.get();

						size_t clindex = getCloneIndex(hook->node->cast<CSGBranch>());

						gfx->setTransformMatrix(rotmat * Matrix::getTranslationMatrix(pos) * camera.sceneMatrix);
						drawClone(clindex);
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
		if(showingSector < 0)
			for (auto &str : kenv.sectorObjects)
				for (CKBeaconKluster *bk = str.getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
					drawBeaconKluster(bk);
		else if(showingSector < kenv.numSectors)
			for (CKBeaconKluster *bk = kenv.sectorObjects[showingSector].getFirst<CKBeaconKluster>(); bk; bk = bk->nextKluster.get())
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
	//float alpha = SDL_GetTicks() * 3.1416f / 1000.f;
	//Vector3 v1(-cos(alpha), -1, -sin(alpha)), v2(cos(alpha), -1, sin(alpha)), v3(0, 1, 0);
	//Vector3 rayDir = getRay(camera, g_window);
	//auto res = getRayTriangleIntersection(camera.position, rayDir, v3, v1, v2);
	//uint32_t color = res.first ? 0xFF0000FF : 0xFFFFFFFF;
	//gfx->drawLine3D(v1, v2, color);
	//gfx->drawLine3D(v2, v3, color);
	//gfx->drawLine3D(v3, v1, color);

	if (nearestRayHit) {
		const Vector3 rad = Vector3(1, 1, 1) * 0.1f;
		drawBox(nearestRayHit->hitPosition + rad, nearestRayHit->hitPosition - rad);
	}

	if (showGroundBounds && kenv.hasClass<CGround>()) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		gfx->unbindTexture(0);
		auto drawGroundBounds = [this,&drawBox](CGround* gnd) {
			auto &b = gnd->aabb;
			drawBox(b.highCorner, b.lowCorner, (selGround == gnd) ? 0xFF00FF00 : 0xFFFFFFFF);
		};
		for (CKObject* obj : kenv.levelObjects.getClassType<CGround>().objects)
			drawGroundBounds(obj->cast<CGround>());
		if (showingSector < 0)
			for (auto &str : kenv.sectorObjects)
				for (CKObject *obj : str.getClassType<CGround>().objects)
					drawGroundBounds(obj->cast<CGround>());
		else if(showingSector < kenv.numSectors)
			for (CKObject *obj : kenv.sectorObjects[showingSector].getClassType<CGround>().objects)
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
		if (showingSector < 0)
			for (auto &str : kenv.sectorObjects)
				for (CKObject *obj : str.getClassType<CGround>().objects)
					drawGround(obj->cast<CGround>());
		else if (showingSector < kenv.numSectors)
			for (CKObject *obj : kenv.sectorObjects[showingSector].getClassType<CGround>().objects)
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

	CKGroup *grpEnemy = kenv.hasClass<CKGrpEnemy>() ? kenv.levelObjects.getFirst<CKGrpEnemy>() : nullptr;

	if (grpEnemy) {
		gfx->setTransformMatrix(camera.sceneMatrix);
		//for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
		for (CKGroup* osquad = grpEnemy->childGroup.get(); osquad; osquad = osquad->nextGroup.get()) {
			if (!osquad->isSubclassOf<CKGrpSquadEnemy>()) continue;
			CKGrpSquadEnemy* squad = osquad->cast<CKGrpSquadEnemy>();
			if (showSquadBoxes) {
				for (const auto& bb : { squad->sqUnk3, squad->sqUnk4 }) {
					Vector3 v1(bb[0], bb[1], bb[2]);
					Vector3 v2(bb[3], bb[4], bb[5]);
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

			if (showingChoreoKey < squad->choreoKeys.size()) {
				CKChoreoKey *ckey = squad->choreoKeys[showingChoreoKey].get();
				const Matrix &gmat = squad->mat1;
				for (auto& slot : ckey->slots) {
					Vector3 spos = slot.position.transform(gmat);

					switch (slot.enemyGroup) {
						case 255:
							gfx->setBlendColor(0xFF0000FF);
							gfx->setTransformMatrix(camera.sceneMatrix);
							break;
						default:
							gfx->setBlendColor(0xFFFFFFFF);
							gfx->setTransformMatrix(camera.sceneMatrix);
							break;
						}
					gfx->unbindTexture(0);
					drawBox(spos + Vector3(0, 1, 0) - Vector3(1, 1, 1), spos + Vector3(1, 2, 1));
					gfx->setBlendColor(0xFFFFFFFF);

					if (slot.enemyGroup == 255 && squad->pools.size() > defaultpool) {
						auto hook = squad->pools[defaultpool].pool->childHook;
						auto nodegeo = hook->node->cast<CAnimatedClone>();
						size_t clindex = getCloneIndex(nodegeo);
						float angle = (std::atan2(slot.direction.x, slot.direction.z));
						gfx->setTransformMatrix(Matrix::getRotationYMatrix(angle)* Matrix::getTranslationMatrix(slot.position)* squad->mat1* camera.sceneMatrix);

						if ((hook->getClassID() == 110) || (hook->getClassID() == 90) ||
							(hook->getClassID() == 124) || (hook->getClassID() == 125)) {
							drawClone(clindex - 2);
						}
						else if (hook->getClassID() == 176) {
							drawClone(clindex - 11);
						}

						drawClone(clindex);
					}


					for  (int poolindex = 0; poolindex < squad->pools.size(); poolindex++) {
						if (poolindex == slot.enemyGroup) {
							auto hook = squad->pools[poolindex].pool->childHook;

							auto nodegeo = hook->node->cast<CAnimatedClone>();
							size_t clindex = getCloneIndex(nodegeo);
							float angle = (std::atan2(slot.direction.x, slot.direction.z));
							gfx->setTransformMatrix(Matrix::getRotationYMatrix(angle) * Matrix::getTranslationMatrix(slot.position) * squad->mat1 * camera.sceneMatrix);

							if ((hook->getClassID() == 110) || (hook->getClassID() == 90) ||
								(hook->getClassID() == 124) || (hook->getClassID() == 125)) {
								drawClone(clindex - 2);
							}
							if (hook->getClassID() == 176) {
								drawClone(clindex - 11);
							}

							drawClone(clindex);
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

	if (showMarkers && kenv.hasClass<CKSrvMarker>()) {
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

	if (showDetectors && kenv.hasClass<CKSrvDetector>()) {
		if (CKSrvDetector *srvDetector = kenv.levelObjects.getFirst<CKSrvDetector>()) {
			gfx->setTransformMatrix(camera.sceneMatrix);
			gfx->setBlendColor(0xFF00FF00); // green
			for (auto &aabb : srvDetector->aaBoundingBoxes)
				drawBox(aabb.highCorner, aabb.lowCorner);
			gfx->setBlendColor(0xFF0080FF); // orange
			for (auto &sph : srvDetector->spheres)
				drawBox(sph.center + Vector3(1, 1, 1) * sph.radius, sph.center - Vector3(1, 1, 1) * sph.radius);
			gfx->setBlendColor(0xFFFF00FF); // pink
			for (auto &h : srvDetector->rectangles) {
				Vector3 dir, side1, side2;
				switch (h.direction | 1) {
				case 1: dir = Vector3(1, 0, 0); side1 = Vector3(0, 1, 0); side2 = Vector3(0, 0, 1); break;
				case 3: dir = Vector3(0, 1, 0); side1 = Vector3(0, 0, 1); side2 = Vector3(1, 0, 0); break;
				case 5: dir = Vector3(0, 0, 1); side1 = Vector3(1, 0, 0); side2 = Vector3(0, 1, 0); break;
				}
				if (h.direction & 1)
					dir *= -1.0f;
				gfx->setTransformMatrix(Matrix::getTranslationMatrix(h.center) * camera.sceneMatrix);
				progeocache.getPro(sphereModel->geoList.geometries[0], &protexdict)->draw();
				gfx->drawLine3D(Vector3(0, 0, 0), dir * 4.0f);
				Vector3 corner = side1 * h.length1 + side2 * h.length2;
				drawBox(corner, -corner);
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
	char tbuf[128] = "(null)";
	if(CKObject *obj = ptr._pointer)
		_snprintf_s(tbuf, _TRUNCATE, "%p : %i %i %s : %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName(), kenv.getObjectName(obj));
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
					ImGui::SameLine(0.0f, 0.0f);
					ImGui::Text("%p : %i %i %s", eo, eo->getClassCategory(), eo->getClassID(), eo->getClassName());
					ImGui::PopID();
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ptr) {
		if (ImGui::BeginDragDropSource()) {
			CKObject *obj = ptr._pointer;
			ImGui::SetDragDropPayload("CKObject", &obj, sizeof(obj));
			ImGui::Text("%p : %i %i %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName());
			ImGui::EndDragDropSource();
		}
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
		selectedHook = nullptr;
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
	if (kenv.version == kenv.KVERSION_XXL1 && kenv.platform == kenv.PLATFORM_PC) {
		ImGui::SameLine();
		if (ImGui::Button("Test")) {
			launcher.loadLevel(levelNum);
		}
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
	ImGui::InputInt("Show sector", &showingSector);
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
	ImGui::Checkbox("MsgAction bounds", &showMsgActionBoxes);
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
	if (ImGui::Button("Rocket Romans \\o/"))
		GimmeTheRocketRomans(kenv);
	if(ImGui::IsItemHovered())
		ImGui::SetTooltip("Transform all Basic Enemies to Rocket Romans");
	if (ImGui::Button("Export Basic Enemy Cpnt Values to TXT")) {
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
				void reflect(EventNode &ref, const char *name, CKObject *user) override { write(name); };
				void reflect(std::string &ref, const char *name) override { abort(); } // TODO
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
				void reflect(EventNode &ref, const char *name, CKObject *user) override { fprintf(csv, "(%i,%i)\t", ref.seqIndex, ref.bit); };
				void reflect(std::string &ref, const char *name) override { abort(); } // TODO
			};
			std::string fname = SaveDialogBox(g_window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
			if (!fname.empty()) {
				FILE *csv;
				fopen_s(&csv, fname.c_str(), "w");
				NameListener nl(csv);
				ValueListener vl(csv);
				fprintf(csv, "Index\t");
				firstcpnt->reflectMembers2(nl, &kenv);
				int index = 0;
				for (CKObject *obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
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
	//if (ImGui::Button("Export info for Beta Rome")) {
	//	FILE *wobj;
	//	fopen_s(&wobj, "betah.obj", "wt");
	//	int inx = 0, nextVertex = 1;
	//	for (CKObject *ko : kenv.levelObjects.getClassType<CKLine>().objects) {
	//		CKLine *line = ko->cast<CKLine>();
	//		fprintf(wobj, "o CKLine_%04u\n", inx++);
	//		for (Vector3 &vert : line->points)
	//			fprintf(wobj, "v %f %f %f\n", vert.x, vert.y, vert.z);
	//		fprintf(wobj, "l");
	//		for (int i = 0; i < line->points.size(); i++)
	//			fprintf(wobj, " %i", nextVertex + i);
	//		fprintf(wobj, "\n");
	//		nextVertex += line->points.size();
	//	}
	//	inx = 0;
	//	for (CKObject *ko : kenv.levelObjects.getClassType<CKSpline4L>().objects) {
	//		CKSpline4L *spline = ko->cast<CKSpline4L>();
	//		fprintf(wobj, "o CKSpline4L_%04u\n", inx++);
	//		for (Vector3 &vert : spline->dings)
	//			fprintf(wobj, "v %f %f %f\n", vert.x, vert.y, vert.z);
	//		fprintf(wobj, "l");
	//		for (int i = 0; i < spline->dings.size(); i++)
	//			fprintf(wobj, " %i", nextVertex + i);
	//		fprintf(wobj, "\n");
	//		nextVertex += spline->dings.size();
	//	}

	//	fclose(wobj);
	//}
	if (ImGui::Button("Make scene geometry from ground collision")) {
		for (int i = -1; i < (int)kenv.numSectors; i++) {
			KObjectList &objlist = (i == -1) ? kenv.levelObjects : kenv.sectorObjects[i];

			// get vertices + triangles
			std::vector<Vector3> positions;
			std::vector<RwGeometry::Triangle> triangles;
			std::vector<uint32_t> colors;

			CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>();
			for (kobjref<CGround> &gnd : mkluster->grounds) {
				if (gnd->isSubclassOf<CDynamicGround>())
					continue;
				uint16_t startIndex = positions.size();
				for (auto &tri : gnd->triangles) {
					std::array<Vector3, 3> tv;
					for (int i = 0; i < 3; i++)
						tv[i] = gnd->vertices[tri.indices[2-i]];
					Vector3 crs = (tv[2] - tv[0]).cross(tv[1] - tv[0]).normal();
					float dp = std::max(crs.dot(Vector3(1,1,1).normal()), 0.0f);
					uint8_t ll = (uint8_t)(dp * 255.0f);
					uint32_t clr = 0xFF000000 + ll * 0x010101;

					positions.insert(positions.end(), tv.begin(), tv.end());
					colors.insert(colors.end(), 3, clr);
					RwGeometry::Triangle rwtri;
					rwtri.indices = { startIndex, (uint16_t)(startIndex + 1), (uint16_t)(startIndex + 2) };
					rwtri.materialId = 0;
					startIndex += 3;
					triangles.push_back(std::move(rwtri));
				}
			}

			if (triangles.empty() || positions.empty())
				continue;

			// sphere calculation
			BoundingSphere bs(positions[0], 0.0f);
			for (Vector3 &pos : positions)
				bs.mergePoint(pos);

			// make geometry
			std::unique_ptr<RwGeometry> rwgeo(new RwGeometry(createEmptyGeo()));
			rwgeo->numVerts = positions.size();
			rwgeo->numTris = triangles.size();
			rwgeo->verts = std::move(positions);
			rwgeo->tris = std::move(triangles);
			rwgeo->spherePos = bs.center;
			rwgeo->sphereRadius = bs.radius;

			rwgeo->flags |= RwGeometry::RWGEOFLAG_PRELIT;
			rwgeo->colors = std::move(colors);

			CKGeometry *kgeo = kenv.createObject<CKGeometry>(i);
			kgeo->flags = 1;
			kgeo->flags2 = 0;
			kgeo->clump = new RwMiniClump;
			kgeo->clump->atomic.flags = 5;
			kgeo->clump->atomic.unused = 0;
			kgeo->clump->atomic.geometry = std::move(rwgeo);

			// replace sector node's geometry
			CSGSectorRoot *sgsr = objlist.getFirst<CSGSectorRoot>();
			CKAnyGeometry *ag = sgsr->geometry.get();
			sgsr->geometry.reset();
			while (ag) {
				CKAnyGeometry *nxt = ag->nextGeo.get();
				ag->nextGeo.reset();
				kenv.removeObject(ag);
				ag = nxt;
			}
			sgsr->geometry = kgeo;
		}
		progeocache.clear();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Replace all rendering sector root geometries by ground models,\nso that you can see the ground collision ingame.");

	if (kenv.isRemaster && ImGui::Button("Convert Romaster -> Original")) {
		// Truncate CParticlesNodeFx for compatibility with original
		for (CKObject* obj : kenv.levelObjects.getClassType<CParticlesNodeFx>().objects) {
			CParticlesNodeFx *fx = obj->cast<CParticlesNodeFx>();
			fx->unkPartSize = 0x5D - 0x53;
		}
		for (auto &str : kenv.sectorObjects) {
			for (CKObject* obj : str.getClassType<CParticlesNodeFx>().objects) {
				CParticlesNodeFx *fx = obj->cast<CParticlesNodeFx>();
				fx->unkPartSize = 0x5D - 0x53;
			}
		}
		// Remove all events sent to parkour stele hooks
		if (CKSrvEvent *srvEvent = kenv.levelObjects.getFirst<CKSrvEvent>()) {
			int ev = 0;
			for (auto &bee : srvEvent->bees) {
				for (int j = 0; j < bee._1; j++) {
					if(CKObject *obj = srvEvent->objs[ev].get())
						if (obj->isSubclassOf<CKHkParkourSteleAsterix>()) {
							srvEvent->objs.erase(srvEvent->objs.begin() + ev);
							srvEvent->objInfos.erase(srvEvent->objInfos.begin() + ev);
							bee._1 -= 1;
							j -= 1; // Don't increment j for next iteration
							ev -= 1;
						}
					ev++;
				}
			}
		}
		// remove the parkour hooks
		if (auto *grpMeca = kenv.levelObjects.getFirst<CKGrpMeca>()) {
			CKHook *prev = nullptr, *next;
			for (CKHook *hook = grpMeca->childHook.get(); hook; hook = next) {
				next = hook->next.get();
				if (hook->isSubclassOf<CKHkParkourSteleAsterix>()) {
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
				if (node->isSubclassOf<CKRomaOnly1CinematicBloc>() || node->isSubclassOf<CKRomaOnly2CinematicBloc>()) {
					CKCinematicBloc *bloc = node->cast<CKCinematicBloc>();
					CKStartEventCinematicBloc *sub = kenv.createObject<CKStartEventCinematicBloc>(-1);
					*(CKCinematicBloc*)sub = *bloc;
					noderef = sub;
					kenv.removeObject(node);
				}
				else if (node->isSubclassOf<CKLogicalRomaOnly>()) {
					CKCinematicDoor *door = node->cast<CKCinematicDoor>();
					CKLogicalAnd *sub = kenv.createObject<CKLogicalAnd>(-1);
					*(CKCinematicDoor*)sub = *door;
					noderef = sub;
					kenv.removeObject(node);
				}
			}
		}
		// remove remaining romaster-specific cinematic nodes that were not referenced
		for (int clid : {CKRomaOnly1CinematicBloc::FULL_ID, CKRomaOnly2CinematicBloc::FULL_ID, CKLogicalRomaOnly::FULL_ID}) {
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

	if (ImGui::CollapsingHeader("Sky colors")) {
		if (CKHkSkyLife *hkSkyLife = kenv.levelObjects.getFirst<CKHkSkyLife>()) {
			ImVec4 c1 = ImGui::ColorConvertU32ToFloat4(hkSkyLife->skyColor);
			ImGui::ColorEdit4("Sky color", &c1.x);
			hkSkyLife->skyColor = ImGui::ColorConvertFloat4ToU32(c1);
			ImVec4 c2 = ImGui::ColorConvertU32ToFloat4(hkSkyLife->cloudColor);
			ImGui::ColorEdit4("Cloud color", &c2.x);
			hkSkyLife->cloudColor = ImGui::ColorConvertFloat4ToU32(c2);
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
						bool b = ImGui::TreeNodeEx(obj, ImGuiTreeNodeFlags_Leaf, "%s (%i, %i) %i, refCount=%i, %s", obj->getClassName(), obj->getClassCategory(), obj->getClassID(), n, obj->getRefCount(), latinToUtf8(kenv.getObjectName(obj)).c_str());
						if (ImGui::BeginDragDropSource()) {
							ImGui::SetDragDropPayload("CKObject", &obj, sizeof(obj));
							ImGui::Text("%p : %i %i %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName());
							ImGui::EndDragDropSource();
						}
						if(b)
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
		// 0x00
		"*", "*", "*", "Wooden Crate", "Metal Crate", "?", "Helmet", "Golden Helmet",
		// 0x08
		"Potion", "Shield", "Ham", "x3 Multiplier",	"x10 Multiplier", "Laurel", "Boar", "Water flow",
		// 0x10
		"Merchant", "*", "*", "*", "*", "Save point", "Respawn point", "Hero respawn pos",
		// 0x18
		"?", "?", "?", "X2 Helmet", "X2 x3 Multiplier", "X2 x10 Multiplier", "?", "X2 Shield",
		// 0x20
		"X2 Golden Helmet", "X2 Diamond Helmet", "?", "?", "?", "?", "X2 Enemy spawn", "X2 Marker",
		// 0x28
		"?", "?", "X2 Food Basket", "?", "?", "?", "OG Surprise", "?",
		// 0x30
		"?", "?", "?", "?", "?", "?", "?", "?",
		// 0x38
		"?", "?", "?", "?", "?", "?", "?", "?",
		// 0x40
		"?", "?", "OG Glue", "OG Powder", "?", "?", "?", "OG Shield",
		// 0x48
		"OG Potion", "OG Bird Cage", "?", "?", "?", "?", "?", "?",
	};
	static const char *beaconX1RomasterNames[] = {
		// 0x00
		"*", "*", "*", "Wooden Crate", "Metal Crate", "?", "Helmet", "Golden Helmet",
		// 0x08
		"Potion", "Shield", "Ham", "x3 Multiplier",	"x10 Multiplier", "Laurel", "Boar", "Water flow",
		// 0x10
		"Merchant", "Original Coin", "Remaster Coin", "*", "*", "Save point", "Respawn point", "Hero respawn pos",
		// 0x18
		"?", "?", "Freeze Crate 1", "Freeze Crate 3", "Freeze Crate 5",
	};
	static auto getBeaconName = [this](int handlerId) -> const char * {
		if (kenv.version <= kenv.KVERSION_XXL1 && kenv.isRemaster && handlerId < std::extent<decltype(beaconX1RomasterNames)>::value)
			return beaconX1RomasterNames[handlerId];
		if (handlerId < std::extent<decltype(beaconX1Names)>::value)
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
		if (ImGui::TreeNode(bk, "Cluster (%f, %f, %f) radius %f", bk->bounds.center.x, bk->bounds.center.y, bk->bounds.center.z, bk->bounds.radius)) {
			ImGui::DragFloat3("Center##beaconKluster", &bk->bounds.center.x, 0.1f);
			ImGui::DragFloat("Radius##beaconKluster", &bk->bounds.radius, 0.1f);
			for (auto &bing : bk->bings) {
				int boffi = bing.bitIndex;
				if(!bing.beacons.empty())
					ImGui::BulletText("%s %02X %02X %02X %02X %02X %02X %04X %08X", getBeaconName(bing.handlerId), bing.unk2a, bing.numBits, bing.handlerId, bing.sectorIndex, bing.klusterIndex, bing.handlerIndex, bing.bitIndex, bing.unk6);
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
		if (selTexID >= texDict->textures.size())
			selTexID = -1;
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
		protexdict.reset(kenv.levelObjects.getFirst<CTextureDictionary>());
		for (int i = 0; i < str_protexdicts.size(); i++)
			str_protexdicts[i].reset(kenv.sectorObjects[i].getFirst<CTextureDictionary>());
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
		"%s %s", node->getClassName(), description);
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
					IGEnumNode(anyanimnode->branchs.get(), "", true);
			}
		}
		ImGui::TreePop();
	}
	IGEnumNode(node->next.get(), "", isAnimBranch);
}

void EditorInterface::IGSceneGraph()
{
	CSGSectorRoot *lvlroot = kenv.levelObjects.getObject<CSGSectorRoot>(0);
	if (ImGui::Button("Add node to LVL")) {
		CNode *node = kenv.createAndInitObject<CNode>();
		lvlroot->insertChild(node);
	}
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

	ImGui::Text("%p %s : %s", selNode, selNode->getClassName(), kenv.getObjectName(selNode));
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
		ImGui::SetDragDropPayload("CKObject", &selNode, sizeof(selNode));
		ImGui::Text("%p %s", selNode, selNode->getClassName());
		ImGui::EndDragDropSource();
	}
	ImGui::Separator();

	ImGui::DragFloat3("Local Position", &selNode->transform._41, 0.1f);
	Matrix globalMat = selNode->getGlobalMatrix();
	if (ImGui::DragFloat3("Global Position", &globalMat._41, 0.1f)) {
		//selNode->transform = globalMat * (selNode->transform.getInverse4x3() * globalMat).getInverse4x3();
	}
	if (ImGui::Button("Place camera there")) {
		Matrix &m = globalMat;
		camera.position = Vector3(m._41, m._42, m._43) - camera.direction * 5.0f;
	}
	if (ImGui::Button("Find hook")) {
		for (auto &hkclass : kenv.levelObjects.categories[CKHook::CATEGORY].type) {
			for (CKObject *obj : hkclass.objects) {
				CKHook *hook = obj->cast<CKHook>();
				if (hook->node.bound)
					if (hook->node.get() == selNode)
						selectedHook = hook;
			}
		}
	}
	ImGui::InputScalar("Flags1", ImGuiDataType_U32, &selNode->unk1, nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::InputScalar("Flags2", ImGuiDataType_U8, &selNode->unk2, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
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
		if (!geonode->geometry) {
			ImGui::Text("No geometry");
		}
		else {
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
			ImGui::Text("Materials:");
			for (CKAnyGeometry *geo = geonode->geometry.get(); geo; geo = geo->nextGeo.get()) {
				if (!geo->clump) {
					ImGui::BulletText("(Geometry with no clump)");
					continue;
				}
				const char *matname = "(no texture)";
				RwGeometry *rwgeo = geo->clump->atomic.geometry.get();
				if(!rwgeo->materialList.materials.empty())
					if(rwgeo->materialList.materials[0].isTextured)
						matname = geo->clump->atomic.geometry->materialList.materials[0].texture.name.c_str();
				ImGui::PushID(geo);
				ImGui::BulletText("%s", matname);
				ImGui::InputScalar("Flags 1", ImGuiDataType_U32, &geo->flags, 0, 0, "%X", ImGuiInputTextFlags_CharsHexadecimal);
				ImGui::InputScalar("Flags 2", ImGuiDataType_U32, &geo->flags2, 0, 0, "%X", ImGuiInputTextFlags_CharsHexadecimal);
				ImGui::PopID();
			}
		}
	}
	if (CFogBoxNodeFx *fogbox = selNode->dyncast<CFogBoxNodeFx>()) {
		ImGuiMemberListener ml(kenv, *this);
		fogbox->reflectFog(ml, &kenv);
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
	auto feobjlist = [this](KObjectList &objlist, const char *desc, int sectorNumber) {
		if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
			ImGui::PushID(mkluster);
			bool tropen = ImGui::TreeNode(mkluster, "%s", desc);
			ImGui::SameLine();
			if (ImGui::SmallButton("Import")) {
				std::string filepath = OpenDialogBox(g_window, "Wavefront OBJ file\0*.OBJ\0\0", "obj");
				if (!filepath.empty()) {
					ImportGroundOBJ(kenv, filepath.c_str(), sectorNumber);
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("No walls yet! Corruption risk!");
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
	feobjlist(kenv.levelObjects, "Level", -1);
	int x = 0;
	for (auto &str : kenv.sectorObjects) {
		char lol[64];
		sprintf_s(lol, "Sector %i", x);
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

		if (ImGui::BeginPopup("GroundDelete")) {
			ImGui::Text("DO NOT delete if there are grounds referenced by scripting events,\nor else weird things and crashes will happen,\ncorrupting your level forever!\n(Hopefully this can be improved and prevented\nin future versions of the editor.)");
			if (ImGui::Button("I don't care, just do it!")) {
				for (int i = -1; i < (int)kenv.numSectors; i++) {
					KObjectList &objlist = (i == -1) ? kenv.levelObjects : kenv.sectorObjects[i];
					if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
						auto it = std::find_if(mkluster->grounds.begin(), mkluster->grounds.end(), [this](const kobjref<CGround> &ref) {return ref.get() == selGround; });
						if (it != mkluster->grounds.end())
							mkluster->grounds.erase(it);
					}
				}
				if (selGround->getRefCount() == 0) {
					kenv.removeObject(selGround);
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
			for (auto &bee : srvEvent->bees) {
				for (int i = 0; i < bee._1; i++) {
					if ((srvEvent->objs[ev + i].id & 0x1FFFF) == (tgCat | (tgId << 6)))
						printf("class (%i, %i) found at seq %i ev 0x%04X\n", tgCat, tgId, s, srvEvent->objInfos[ev + i]);
				}
				ev += bee._1;
				s++;
			}
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Export TSV")) {
		std::string filename = SaveDialogBox(g_window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
		if (!filename.empty()) {
			std::map<std::pair<int, int>, std::set<uint16_t>> evtmap;
			size_t ev = 0, s = 0;
			for (auto &bee : srvEvent->bees) {
				for (int i = 0; i < bee._1; i++) {
					auto &obj = srvEvent->objs[ev + i];
					evtmap[std::make_pair(obj.id & 63, (obj.id >> 6) & 2047)].insert(srvEvent->objInfos[ev + i]);
				}
				ev += bee._1;
				s++;
			}
			FILE *file;
			fopen_s(&file, filename.c_str(), "wt");
			for (auto &mapentry : evtmap) {
				for (uint16_t evt : mapentry.second) {
					fprintf(file, "%i\t%i\t%04X\n", mapentry.first.first, mapentry.first.second, evt);
				}
			}
			fclose(file);
		}
	}
	ImGui::SameLine();
	ImGui::Text("Decoded: %i/%i", std::count_if(srvEvent->bees.begin(), srvEvent->bees.end(), [](CKSrvEvent::StructB &bee) {return bee.userFound; }), srvEvent->bees.size());

	ImGui::Columns(2);
	ImGui::BeginChild("EventSeqList");
	size_t ev = 0, i = 0;
	for (auto &bee : srvEvent->bees) {
		if (i == srvEvent->numA || i == srvEvent->numA + srvEvent->numB)
			ImGui::Separator();
		ImGui::PushID(i);
		if (ImGui::Selectable("##EventSeqEntry", i == selectedEventSequence, 0, ImVec2(0, ImGui::GetTextLineHeight()*2.0f)))
			selectedEventSequence = i;
		ImGui::SameLine();
		ImGui::BulletText("Sequence %i (%i, %i)\nUsed by %s", i, bee._1, bee._2, bee.users.size() ? bee.users[0]->getClassName() : "?");
		ImGui::PopID();
		ev += bee._1;
		i++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();

	ImGui::BeginChild("EventEditor");
	if (selectedEventSequence >= 0 && selectedEventSequence < srvEvent->bees.size()) {
		auto &bee = srvEvent->bees[selectedEventSequence];
		ev = 0;
		for (int i = 0; i < selectedEventSequence; i++) {
			ev += srvEvent->bees[i]._1;
		}
		ImGui::Text("Used by %s", bee.users.size() ? bee.users[0]->getClassName() : "?");
		for (size_t u = 1; u < bee.users.size(); u++) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 1, 0, 1), ", %s", bee.users[u]->getClassName());
		}
		ImGui::InputScalar("Bitmask", ImGuiDataType_U8, &bee._2);
		if (ImGui::Button("Add")) {
			auto it = srvEvent->objs.emplace(srvEvent->objs.begin() + ev + bee._1);
			it->bound = true;
			srvEvent->objInfos.emplace(srvEvent->objInfos.begin() + ev + bee._1);
			bee._1 += 1;
		}
		for (uint8_t i = 0; i < bee._1; i++) {
			ImGui::PushID(ev + i);
			ImGui::SetNextItemWidth(48.0f);
			ImGui::InputScalar("##EventID", ImGuiDataType_U16, &srvEvent->objInfos[ev + i], nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text("->");
			ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
			//ImGui::SetNextItemWidth(-1.0f);
			if (srvEvent->objs[ev + i].bound) {
				//CKObject *obj = srvEvent->objs[ev + i].get();
				//ImGui::Text("-> %p (%i, %i) %s", obj, obj->getClassCategory(), obj->getClassID(), obj->getClassName());
				ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.x);
				IGObjectSelectorRef(kenv, "##evtTargetObj", srvEvent->objs[ev + i].ref);
			}
			else {
				//ImGui::Text("-> Undecoded ref %08X", srvEvent->objs[ev + i].id);
				uint32_t &encref = srvEvent->objs[ev + i].id;
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
				bee._1 -= 1;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Remove");
			ImGui::PopID();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();
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
		if (ImGui::Button("Duplicate")) {
			CKGrpSquadEnemy *clone;
			if (CKGrpSquadJetPack *jpsquad = squad->dyncast<CKGrpSquadJetPack>()) {
				CKGrpSquadJetPack *jpclone = kenv.createObject<CKGrpSquadJetPack>(-1);
				*jpclone = *jpsquad;
				clone = jpclone;
			} else {
				clone = kenv.createObject<CKGrpSquadEnemy>(-1);
				*clone = *squad;
			}

			CKGrpEnemy *grpEnemy = squad->parentGroup->cast<CKGrpEnemy>();
			CKGroup *fstSquad = grpEnemy->childGroup.get();

			clone->nextGroup = fstSquad;
			grpEnemy->childGroup = clone;

			clone->bundle = kenv.createObject<CKBundle>(-1);
			//clone->bundle->next = fstSquad->bundle;
			clone->bundle->flags = 0x7F;
			auto &bundles = kenv.levelObjects.getClassType<CKBundle>().objects;
			bundles[bundles.size() - 2]->cast<CKBundle>()->next = clone->bundle;

			clone->msgAction = kenv.createObject<CKMsgAction>(-1);
			*clone->msgAction = *squad->msgAction;

			for (auto &ref : clone->choreographies) {
				CKChoreography *oriChoreo = ref.get();
				ref = kenv.createObject<CKChoreography>(-1);
				*ref = *oriChoreo;
			}
			for (auto &ref : clone->choreoKeys) {
				CKChoreoKey *ori = ref.get();
				ref = kenv.createObject<CKChoreoKey>(-1);
				*ref = *ori;
			}
			for (auto &pool : clone->pools) {
				pool.cpnt = pool.cpnt->clone(kenv, -1)->cast<CKEnemyCpnt>();
			}
		}
		if (ImGui::BeginTabBar("SquadInfoBar")) {
			if (ImGui::BeginTabItem("Main")) {
				ImGuiMemberListener ml(kenv, *this);
				ml.reflect(*(Vector3*)&squad->mat1._41, "mat1");
				ml.reflect(*(Vector3*)&squad->mat2._41, "mat2");
				ml.reflect(squad->sqUnk1, "sqUnk1");
				ml.reflect(squad->sqUnk2, "sqUnk2");
				ml.reflectContainer(squad->refs, "refs");
				ml.reflectContainer(squad->sqUnk3, "sqUnk3");
				ml.reflectContainer(squad->sqUnk4, "sqUnk4");
				ml.reflect(squad->sqUnk5, "sqUnk5");
				ml.reflectContainer(squad->fings, "fings");
				ml.reflectContainer(squad->sqUnk6, "sqUnk6");
				ml.reflect(squad->sqUnk7, "sqUnk7");
				ml.reflect(squad->sqUnk8, "sqUnk8");
				ml.reflect(squad->sqUnkB, "sqUnkB");
				ml.reflect(squad->sqUnkA, "Event 1", squad);
				ml.reflect(squad->sqUnkC, "Event 2", squad);
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
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("MsgAction")) {
				ImGui::BeginChild("MsgActionWnd");
				CKMsgAction *msgAction = squad->msgAction->cast<CKMsgAction>();
				for (auto &a : msgAction->mas1) {
					if (ImGui::TreeNodeEx(&a, ImGuiTreeNodeFlags_DefaultOpen, "%i", a.mas2.size())) {
						for (auto &b : a.mas2) {
							if (ImGui::TreeNodeEx(&b, ImGuiTreeNodeFlags_DefaultOpen, "%04X %i", b.event, b.mas3.size())) {
								for (auto &c : b.mas3) {
									if (ImGui::TreeNodeEx(&c, ImGuiTreeNodeFlags_DefaultOpen, "%i %i", c.num, c.mas4.size())) {
										int i = 0;
										for (auto &d : c.mas4) {
											char tbuf[64];
											sprintf_s(tbuf, "#%i t%i", i, d.type);
											ImGui::PushID(&d);
											switch (d.type) {
											case 2:
												ImGui::InputFloat(tbuf, &d.valFloat); break;
											case 3:
												IGObjectSelectorRef(kenv, tbuf, d.ref); break;
											default:
												ImGui::InputInt(tbuf, (int*)&d.valU32); break;
											}
											ImGui::PopID();
										}
										ImGui::TreePop();
									}
								}
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				}
				ImGui::EndChild();
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

				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.2f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.2f, 1.0f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1.2f, 1.0f, 1.05f));
				ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(3.59f, 1.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_TextDisabled, (ImVec4)ImColor::HSV(3.59f, 1.0f, 0.0f));
				if (ImGui::Button("New ChoreoKey")) {
					if (selectedSquad) {
						IsDoingHomeWork = true; // Extremely Important! PogChamp
						CKChoreoKey* newkey = kenv.createObject<CKChoreoKey>(-1);

						// Default values
						newkey->unk1 = 0.0f;
						newkey->unk2 = 0.0f;
						newkey->unk3 = 0.0f;
						newkey->flags = 0;

						selectedSquad->choreoKeys.push_back(newkey);
						selectedSquad->choreographies.back()->numKeys = selectedSquad->choreographies.back()->numKeys + 1;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("New ChoreoGraphy")) {
					if (selectedSquad) {
						IsDoingHomeWork = true; // Extremely Important! PogChamp
						CKChoreography* ref = kenv.createObject<CKChoreography>(-1);
							
						ref->unkfloat = 0.0f;
						ref->unk2 = 0;
						ref->numKeys = 0;

						selectedSquad->choreographies.push_back(ref);
						
					}
				}
				ImGui::PopStyleColor(5);

				ImGui::InputInt("ChoreoKey", &showingChoreoKey);
				int ckeyindex = showingChoreoKey;
				if (ckeyindex >= 0 && ckeyindex < squad->choreoKeys.size()) {
					auto &ckey = squad->choreoKeys[ckeyindex];
					//if (ImGui::TreeNode(&ckey, "Key %u %f %f %f %u", ckey->slots.size(), ckey->unk1, ckey->unk2, ckey->unk3, ckey->flags)) {
					ImGui::Separator();
					ImGui::DragFloat("Duration", &ckey->unk1);
					ImGui::DragFloat("Unk2", &ckey->unk2);
					ImGui::DragFloat("Unk3", &ckey->unk3);
					ImGui::InputInt("Default Pool", &defaultpool);

					if (ImGui::Button("Duplicate ChoreoGraphy")) {
						if (selectedSquad) {
							IsDoingHomeWork = true; // Extremely Important! PogChamp
							CKChoreography* ori = selectedSquad->choreographies[getChoreo(showingChoreoKey)].get();;
							CKChoreography* ref = kenv.createObject<CKChoreography>(-1);
							*ref = *ori;
							selectedSquad->choreographies.push_back(ref);

							for (int currentKey = 0; currentKey < int(selectedSquad->choreographies[getChoreo(showingChoreoKey)]->numKeys); currentKey++) {
								CKChoreoKey* ori = selectedSquad->choreoKeys[showingChoreoKey + currentKey].get();
								CKChoreoKey* ref = kenv.createObject<CKChoreoKey>(-1);
								*ref = *ori;
								selectedSquad->choreoKeys.push_back(ref);
							}
						}
					}
					ImGui::SameLine();
					static int ChoreoInsertNum = 0;
					static int ChoreoKInsertNum = 0;
					if (ImGui::Button("Duplicate ChoreoKey")) {
						ImGui::OpenPopup("Duplicate Options");
					}
					if (ImGui::BeginPopup("Duplicate Options")) {

						ImGui::InputInt("Insert at ChoreoGraphy:", &ChoreoInsertNum);
						ImGui::InputInt("Insert at ChoreoKey:", &ChoreoKInsertNum);

						if (ImGui::Button("OK")) {
							if (selectedSquad) {
								IsDoingHomeWork = true; // Extremely Important! PogChamp
								CKChoreoKey* ori = selectedSquad->choreoKeys[showingChoreoKey].get();;
								CKChoreoKey* ref = kenv.createObject<CKChoreoKey>(-1);
								*ref = *ori;
								auto ChoreoInsertPos = selectedSquad->choreoKeys.begin() + ChoreoKInsertNum;
								selectedSquad->choreoKeys.insert(ChoreoInsertPos, 1, ref);
								selectedSquad->choreographies[ChoreoInsertNum]->numKeys = selectedSquad->choreographies[ChoreoInsertNum]->numKeys + 1;
							}
						}
						ImGui::EndPopup();
					}

					static auto get_bit = [](int16_t integer, int16_t N) {

						int16_t constant = 1 << (N - 1);

						if (integer & constant) {
							return true;
						}

						return false;
					};

					static auto toggle_bit = [](int16_t integer, size_t N, bool is1) {
						if (is1 == true) integer = integer |= 1 << N;
						else integer = integer &= ~(1 << N);

						return integer;
					};



					static bool ckflags[8] = { get_bit(ckey->flags, 1), get_bit(ckey->flags, 2), get_bit(ckey->flags, 3), get_bit(ckey->flags, 4),
											   get_bit(ckey->flags, 5), get_bit(ckey->flags, 6), get_bit(ckey->flags, 7), get_bit(ckey->flags, 8) };

					
					if (ImGui::Button("Flags")) {
						ImGui::OpenPopup("ChoreoKey flags");
						ckflags[0] = get_bit(ckey->flags, 1);
						ckflags[1] = get_bit(ckey->flags, 2);
						ckflags[2] = get_bit(ckey->flags, 3);
						ckflags[3] = get_bit(ckey->flags, 4);
						ckflags[4] = get_bit(ckey->flags, 5);
						ckflags[5] = get_bit(ckey->flags, 6);
						ckflags[6] = get_bit(ckey->flags, 7);
						ckflags[7] = get_bit(ckey->flags, 8);
					}
					ImGui::SameLine();
					static int ChoreoDeleteNum = -1;
					static int ChoreoKDeleteNum = -1;
					static bool AdjustChoreo = false;
					static bool AdjustPrevChoreoSlots = false;
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.8f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 1.0f, 0.9f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 1.0f, 1.0f));
					if (ImGui::Button("Delete Tools")) {
						ImGui::OpenPopup("Delete Options");
					}
					ImGui::PopStyleColor(3);
					if (ImGui::BeginPopup("ChoreoKey flags")) {

						ImGui::Checkbox("Don't rotate around asterix", &ckflags[0]);
						ImGui::Checkbox("Bit 2", &ckflags[1]);
						ImGui::Checkbox("Bit 3", &ckflags[2]);
						ImGui::Checkbox("Formations always have spears out", &ckflags[3]);
						ImGui::Checkbox("Look at asterix", &ckflags[4]);
						ImGui::Checkbox("Bit 6", &ckflags[5]);
						ImGui::Checkbox("Enemies can run", &ckflags[6]);
						ImGui::Checkbox("Bit 8", &ckflags[7]);

						if (ImGui::Button("OK")) {
							for (int8_t index = 0; index < 8; index++) {
								ckey->flags = toggle_bit(ckey->flags, index, ckflags[index]);
							}
						}
						ImGui::EndPopup();
					}

					if (ImGui::BeginPopup("Delete Options")) {

						ImGui::InputInt("Delete num ChoreoGraphy:", &ChoreoDeleteNum);
						ImGui::Checkbox("Merge Choreokeys to previous ChoreoGraphy", &AdjustPrevChoreoSlots);
						//ImGui::Checkbox("Adjust Number of Choreokeys", &AdjustChoreo);
						ImGui::InputInt("Delete num ChoreoKey:", &ChoreoKDeleteNum);
						ImGui::Checkbox("Adjust Number of Choreokeys", &AdjustChoreo);

						if (ImGui::Button("OK")) {
							if (selectedSquad) {
								IsDoingHomeWork = true; // Extremely Important! PogChamp

								if (ChoreoKDeleteNum > -1) {
									//ChoreoKey
									auto* Keytoremove = selectedSquad->choreoKeys[ChoreoKDeleteNum].get();
									auto ChoreoKDeletePos = selectedSquad->choreoKeys.begin() + ChoreoKDeleteNum;
									selectedSquad->choreoKeys.erase(ChoreoKDeletePos);
									kenv.removeObject(Keytoremove);
									if (AdjustChoreo == true) {
										selectedSquad->choreographies[getChoreo(ChoreoKDeleteNum)]->numKeys = selectedSquad->choreographies[getChoreo(ChoreoKDeleteNum)]->numKeys - 1;
									}
								}
									
								if (ChoreoDeleteNum > -1) {
									// ChoreoGraphy
									auto* Choreographytoremove = selectedSquad->choreographies[ChoreoDeleteNum].get();
									auto ChoreoDeletePos = selectedSquad->choreographies.begin() + ChoreoDeleteNum;
									selectedSquad->choreographies.erase(ChoreoDeletePos);
									kenv.removeObject(Choreographytoremove);
									if (AdjustPrevChoreoSlots == true) {
										selectedSquad->choreographies[getChoreo(ChoreoDeleteNum - 1)]->numKeys = selectedSquad->choreographies[getChoreo(ChoreoDeleteNum - 1)]->numKeys + selectedSquad->choreographies[getChoreo(ChoreoDeleteNum)]->numKeys;
									}
								}
							}
						}
						ImGui::EndPopup();
					}
					

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
					ImGui::BeginChild("ChoreoSlots", ImVec2(0, 0), true);
					if (IsDoingHomeWork == false) {
						for (auto &slot : ckey->slots) {
							ImGui::PushID(&slot);
							ImGui::DragFloat3("Position", &slot.position.x, 0.1f);
							ImGui::DragFloat3("Direction", &slot.direction.x, 0.1f);
							ImGui::InputScalar("Enemy pool", ImGuiDataType_S8, &slot.enemyGroup);
							ImGui::PopID();
							ImGui::Separator();
						}
					}
					IsDoingHomeWork = false;
					ImGui::EndChild();
				}		
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Pools")) {
				static size_t currentPoolInput = 0;
				if (ImGui::Button("Duplicate")) {
					if (currentPoolInput >= 0 && currentPoolInput < squad->pools.size()) {
						CKGrpSquadEnemy::PoolEntry duppe = squad->pools[currentPoolInput];
						duppe.cpnt = duppe.cpnt->clone(kenv, -1)->cast<CKEnemyCpnt>();
						squad->pools.push_back(duppe);
					}
				}
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

void EditorInterface::IGEnumGroup(CKGroup *group)
{
	if (!group)
		return;
	if (ImGui::TreeNode(group, "%s", group->getClassName())) {
		IGEnumGroup(group->childGroup.get());
		for (CKHook *hook = group->childHook.get(); hook; hook = hook->next.get()) {
			bool b = ImGui::TreeNodeEx(hook, ImGuiTreeNodeFlags_Leaf | (selectedHook == hook ? ImGuiTreeNodeFlags_Selected : 0), "%s", hook->getClassName());
			if (ImGui::IsItemClicked())
				selectedHook = hook;
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
	if (selectedHook) {
		ImGui::Text("%p %s", selectedHook, selectedHook->getClassName());
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
			ImGui::SetDragDropPayload("CKObject", &selectedHook, sizeof(selectedHook));
			ImGui::Text("%p %s", selectedHook, selectedHook->getClassName());
			ImGui::EndDragDropSource();
		}
		ImGui::Separator();
		ImGuiMemberListener ml(kenv, *this);
		selectedHook->virtualReflectMembers(ml, &kenv);
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
	ImGui::PushItemWidth(130.0f);

	ImGuiMemberListener igml(kenv, *this);
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
		pfnode->highBBCorner = pfnode->lowBBCorner + Vector3(pfnode->numCellsX * 2, 50, pfnode->numCellsZ * 2);
	}
	//ImGui::SameLine();
	//if (ImGui::Button("Examine")) {
	//	std::map<uint8_t, int> counts;
	//	int pid = 0;
	//	for (auto &pfnode : srvpf->nodes) {
	//		for (uint8_t &cell : pfnode->cells) {
	//			counts[cell]++;
	//			if (cell == 0)
	//				printf("found 0 at %i\n", pid);
	//			//if (cell != 7) cell = 4;
	//		}
	//		pid++;
	//	}
	//	for (auto &me : counts) {
	//		printf("%u: %i\n", me.first, me.second);
	//	}
	//}

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

void EditorInterface::IGDetectorEditor()
{
	ImGui::BeginChild("DetectorEditor");
	CKSrvDetector *srvDetector = kenv.levelObjects.getFirst<CKSrvDetector>();
	if (!srvDetector) return;
	auto coloredTreeNode = [](const char *label, const ImVec4 &color = ImVec4(1,1,1,1)) {
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		bool res = ImGui::TreeNode(label);
		ImGui::PopStyleColor();
		return res;
	};
	if (ImGui::CollapsingHeader("Shapes", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (coloredTreeNode("Bounding boxes", ImVec4(0, 1, 0, 1))) {
			int i = 0;
			for (auto &bb : srvDetector->aaBoundingBoxes) {
				ImGui::PushID(&bb);
				ImGui::BulletText("#%i", i++);
				ImGui::DragFloat3("High corner", &bb.highCorner.x, 0.1f);
				ImGui::DragFloat3("Low corner", &bb.lowCorner.x, 0.1f);
				if (ImGui::Button("See OvO"))
					camera.position = Vector3(bb.highCorner.x, camera.position.y, bb.highCorner.z);
				ImGui::PopID();
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
		if (coloredTreeNode("Spheres", ImVec4(1, 0.5f, 0, 1))) {
			int i = 0;
			for (auto &sph : srvDetector->spheres) {
				ImGui::PushID(&sph);
				ImGui::BulletText("#%i", i++);
				ImGui::DragFloat3("Center", &sph.center.x, 0.1f);
				ImGui::DragFloat("Radius", &sph.radius, 0.1f);
				if (ImGui::Button("See OvO"))
					camera.position = Vector3(sph.center.x, camera.position.y, sph.center.z);
				ImGui::PopID();
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
		if (coloredTreeNode("Rectangles", ImVec4(1, 0, 1, 1))) {
			int i = 0;
			for (auto &rect : srvDetector->rectangles) {
				ImGui::PushID(&rect);
				ImGui::BulletText("#%i", i++);
				ImGui::DragFloat3("Center", &rect.center.x, 0.1f);
				ImGui::DragFloat("Length 1", &rect.length1);
				ImGui::DragFloat("Length 2", &rect.length2);
				ImGui::InputScalar("Direction", ImGuiDataType_U8, &rect.direction);
				if (ImGui::Button("See OvO"))
					camera.position = Vector3(rect.center.x, camera.position.y, rect.center.z);
				ImGui::PopID();
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
	}
	if (ImGui::CollapsingHeader("Checklist", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID("checklist");
		auto enumdctlist = [&coloredTreeNode](std::vector<CKSrvDetector::Detector> &dctlist, const char *name, const ImVec4 &color = ImVec4(1,1,1,1)) {
			if (coloredTreeNode(name, color)) {
				int i = 0;
				for (auto &dct : dctlist) {
					ImGui::PushID(&dct);
					ImGui::BulletText("#%i", i++);
					ImGui::InputScalar("Shape index", ImGuiDataType_U16, &dct.shapeIndex);
					ImGui::InputScalar("Node index", ImGuiDataType_U16, &dct.nodeIndex);
					ImGui::InputScalar("Flags", ImGuiDataType_U16, &dct.flags);
					ImGui::InputScalar("Event sequence", ImGuiDataType_S16, &dct.eventSeqIndex.seqIndex);
					ImGui::InputScalar("Event sequence bit", ImGuiDataType_U8, &dct.eventSeqIndex.bit);
					ImGui::Separator();
					ImGui::PopID();
				}
				ImGui::TreePop();
			}
		};
		enumdctlist(srvDetector->aDetectors, "Bounding boxes", ImVec4(0, 1, 0, 1));
		enumdctlist(srvDetector->bDetectors, "Spheres", ImVec4(1, 0.5f, 0, 1));
		enumdctlist(srvDetector->cDetectors, "Rectangles", ImVec4(1, 0, 1, 1));
		enumdctlist(srvDetector->dDetectors, "D Detectors");
		enumdctlist(srvDetector->eDetectors, "E Detectors");
		ImGui::PopID();
	}
	if (ImGui::CollapsingHeader("Scene Nodes")) {
		int i = 0;
		ImGui::PushItemWidth(-32.0f);
		for (auto &node : srvDetector->nodes) {
			IGObjectSelectorRef(kenv, std::to_string(i++).c_str(), node);
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
}

void EditorInterface::IGCinematicEditor()
{
	CKSrvCinematic *srvCine = kenv.levelObjects.getFirst<CKSrvCinematic>();
	static int selectedCinematicSceneIndex = -1;
	static CKCinematicNode *selectedCineNode = nullptr;
	ImGui::InputInt("Cinematic Scene", &selectedCinematicSceneIndex);
	if (selectedCinematicSceneIndex >= 0 && selectedCinematicSceneIndex < srvCine->cineScenes.size()) {
		CKCinematicScene *scene = srvCine->cineScenes[selectedCinematicSceneIndex].get();

		if (ImGui::Button("Export TGF")) {
			std::string filename = SaveDialogBox(g_window, "Trivial Graph Format (*.tgf)\0*.TGF\0", "tgf");
			if (!filename.empty()) {
				std::map<CKCinematicNode*, int> gfNodes;
				std::map<std::array<CKCinematicNode*, 2>, std::string> gfEdges;
				int nextNodeId = 1;

				// Find all nodes from the scene
				for (auto &cncls : kenv.levelObjects.categories[CKCinematicNode::CATEGORY].type) {
					for (CKObject *obj : cncls.objects) {
						//CKCinematicNode *knode = obj->cast<CKCinematicNode>();
						if (CKCinematicDoor *kdoor = obj->dyncast<CKCinematicDoor>()) {
							if (kdoor->cdScene.get() == scene)
								gfNodes.insert({ kdoor, nextNodeId++ });
						}
						else if (CKCinematicBloc *kbloc = obj->dyncast<CKCinematicBloc>()) {
							if (kbloc->cbScene.get() == scene)
								gfNodes.insert({ kbloc, nextNodeId++ });
						}

					}
				}

				// Constructing edges
				std::function<void(CKCinematicNode *node)> visit;
				visit = [&scene, &gfNodes, &gfEdges, &visit](CKCinematicNode *node) {
					if (CKCinematicDoor *door = node->dyncast<CKCinematicDoor>()) {
						//assert((door->cdUnk1 == 0xFFFF) || (door->cdUnk2 == 0xFFFF));
						if (door->cdUnk1 != 0xFFFF) {
							for (int i = 0; i < door->cdUnk3; i++) {
								CKCinematicNode *subnode = scene->cineNodes[door->cdUnk1 + i].get();
								//if (door->cdUnk2 != door->cdUnk1 + i)
								//	gfEdges.insert({ { node, subnode }, std::to_string(i) });
								gfEdges[{ node, subnode }].append(std::to_string(i)).append(",");
								//visit(subnode);
							}
						}
						if (door->cdUnk2 != 0xFFFF) {
							CKCinematicNode *subnode = scene->cineNodes[door->cdUnk2].get();
							gfEdges[{ node, subnode }].append("cond,");
						}
					}
					else if (CKCinematicBloc *bloc = node->dyncast<CKCinematicBloc>()) {
						//assert((bloc->cbUnk0 == 0xFFFF) || (bloc->cbUnk1 == 0xFFFF));
						//assert(bloc->cbUnk1 == 0xFFFF);
						if (bloc->cbUnk0 != 0xFFFF) {
							for (int i = 0; i < bloc->cbUnk2; i++) {
								CKCinematicNode *subnode = scene->cineNodes[bloc->cbUnk0 + i].get();
								gfEdges[{ node, subnode }].append(std::to_string(i)).append(",");
								//visit(subnode);
							}
						}
						if (bloc->cbUnk1 != 0xFFFF) {
							CKCinematicNode *subnode = scene->cineNodes[bloc->cbUnk1].get();
							gfEdges[{ node, subnode }].append("cond,");
						}
						if (CKGroupBlocCinematicBloc *grpbloc = node->dyncast<CKGroupBlocCinematicBloc>()) {
							gfEdges[{node, grpbloc->gbFirstNode.get()}].append("grpHead,");
						}
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
				fopen_s(&tgf, filename.c_str(), "wt");
				for (auto &gnode : gfNodes) {
					fprintf(tgf, "%i %s (%p)\n", gnode.second, gnode.first->getClassName() + 7, gnode.first);
				}
				fprintf(tgf, "#\n");
				for (auto &edge : gfEdges) {
					fprintf(tgf, "%i %i %s\n", gfNodes[edge.first[0]], gfNodes[edge.first[1]], edge.second.c_str());
				}
				fclose(tgf);
			}
		}

		ImGui::Columns(2);
		ImGui::BeginChild("CineNodes");

		bool b = ImGui::TreeNodeEx("Start door", ImGuiTreeNodeFlags_Leaf | ((scene->startDoor.get() == selectedCineNode) ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked()) {
			selectedCineNode = scene->startDoor.get();
		}
		if (b) ImGui::TreePop();

		struct CineNodeEnumerator {
			static void enumNode(CKCinematicNode *node, int i) {
				bool isGroup = node->isSubclassOf<CKGroupBlocCinematicBloc>();
				ImGuiTreeNodeFlags tflags = 0;
				if (node == selectedCineNode) tflags |= ImGuiTreeNodeFlags_Selected;
				if (!isGroup) tflags |= ImGuiTreeNodeFlags_Leaf;
				bool b = ImGui::TreeNodeEx(node, tflags, "%i: %s", i++, node->getClassName());
				if (ImGui::IsItemClicked()) {
					selectedCineNode = node;
				}
				if (b) {
					if (isGroup) {
						int i = 0;
						for (auto &sub : node->cast<CKGroupBlocCinematicBloc>()->gbSubnodes)
							enumNode(sub.get(), i++);
					}
					ImGui::TreePop();
				}
			}
		};

		int i = 0;
		for (auto &node : scene->cineNodes) {
			CineNodeEnumerator::enumNode(node.get(), i++);
		}
		ImGui::EndChild();
		ImGui::NextColumn();
		if (selectedCineNode) {
			ImGui::BeginChild("CineSelectedNode");
			ImGuiMemberListener ml(kenv, *this);
			selectedCineNode->virtualReflectMembers(ml, &kenv);
			ImGui::EndChild();
		}
		ImGui::Columns();
	}
}

void EditorInterface::IGLocaleEditor()
{
	if (kenv.version > KEnvironment::KVERSION_XXL1 || kenv.platform == KEnvironment::PLATFORM_PS2) {
		ImGui::Text("Only available for XXL1 PC/GCN");
		return;
	}

	// Converts loc file text to editable Utf8 format and vice versa, also transforms special button icons to \a \b ...
	struct TextConverter {
		static std::string decode(const std::wstring &wstr) {
			//return wcharToUtf8(wstr.c_str());
			std::string dec;
			for (wchar_t wc : wstr) {
				if (wc == '\\')
					dec.append("\\\\");
				else if (wc >= 0xE000 && wc <= 0xE000 + 'z' - 'a') {
					dec.push_back('\\');
					dec.push_back(wc - 0xE000 + 'a');
				}
				else {
					dec.append(wcharToUtf8(std::wstring(1, wc).c_str()));
				}
			}
			return dec;
		}
		static std::wstring encode(const std::string &str) {
			//return utf8ToWchar(str.c_str());
			std::wstring enc;
			int j = 0, i;
			for (i = 0; i < str.size(); ) {
				if (str[i] == '\\') {
					if (str[i + 1] == '\\') {
						enc.append(utf8ToWchar(std::string(str, j, i - j).c_str()));
						enc.push_back('\\');
						i += 2;
						j = i;
					}
					else if (str[i + 1] >= 'a' && str[i + 1] <= 'z') {
						enc.append(utf8ToWchar(std::string(str, j, i - j).c_str()));
						enc.push_back(0xE000 + str[i + 1] - 'a');
						i += 2;
						j = i;
					}
					else
						i++;
				}
				else i++;
			}
			enc.append(utf8ToWchar(std::string(str, j, i - j).c_str()));
			return enc;
		}
	};

	// Instance of GLOC file loaded in memory
	struct LocalDocument {
		KLocalPack locpack;
		std::vector<texture_t> fontTextures;
		std::map<std::string, int> fntTexMap;
		std::vector<std::string> trcTextU8, stdTextU8;
		uint32_t langStrIndex, langID;
		std::map<int, KLocalPack> lvlLocpacks;
		std::map<int, std::vector<texture_t>> lvlTextures;
	};

	static std::vector<LocalDocument> documents;
	static bool langLoaded = false;

	static std::vector<int> fndLevels = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	// TODO: Find all levels by searching for LVL*** folders in the gamepath
	
	if (!langLoaded) {
		for (auto &doc : documents) {
			for (texture_t &tex : doc.fontTextures)
				gfx->deleteTexture(tex);
			for (auto &lt : doc.lvlTextures)
				for (texture_t &tex : lt.second)
					gfx->deleteTexture(tex);
		}
		documents.clear();

		bool missingLlocWarningShown = false;

		int numLang = 1;
		for (int langid = 0; langid < numLang; langid++) {
			documents.emplace_back();
			auto &doc = documents.back();
			auto &locpack = doc.locpack;
			//locpack = KLocalPack();
			locpack.addFactory<Loc_CLocManager>();
			locpack.addFactory<Loc_CManager2d>();
			char tbuf[512];
			sprintf_s(tbuf, "%s/%02uGLOC.%s", kenv.gamePath.c_str(), langid, KEnvironment::platformExt[kenv.platform]);
			IOFile gloc = IOFile(tbuf, "rb");
			locpack.deserialize(&kenv, &gloc);

			if (Loc_CManager2d *lmgr = locpack.get<Loc_CManager2d>()) {
				int i = 0;
				for (auto &tex : lmgr->piTexDict.textures) {
					doc.fontTextures.push_back(gfx->createTexture(tex.image));
					doc.fntTexMap[tex.texture.name] = i++;
				}
			}

			if (Loc_CLocManager *loc = locpack.get<Loc_CLocManager>()) {
				numLang = loc->numLanguages;
				doc.langStrIndex = loc->langStrIndices[langid];
				doc.langID = loc->langIDs[langid];
				//static const std::wstring zerostr = std::wstring(1, '\0');
				for (auto &trc : loc->trcStrings) {
					//assert(trc.second != zerostr);
					doc.trcTextU8.emplace_back(TextConverter::decode(trc.second));
				}
				for (std::wstring &str : loc->stdStrings) {
					//assert(str != zerostr);
					doc.stdTextU8.emplace_back(TextConverter::decode(str));
				}
			}

			// LLOCs
			for (int lvl : fndLevels) {
				KLocalPack &llpack = doc.lvlLocpacks[lvl];
				llpack.addFactory<Loc_CKGraphic>();
				sprintf_s(tbuf, "%s/LVL%03u/%02uLLOC%02u.%s", kenv.gamePath.c_str(), lvl, langid, lvl, KEnvironment::platformExt[kenv.platform]);
				if (_access(tbuf, 0) == -1) {
					// LLOC file missing... Just duplicate another one with same lang id
					if (!missingLlocWarningShown) {
						missingLlocWarningShown = true;
						MessageBox((HWND)g_window->getNativeWindow(), "Some LLOC files are missing!\nThe editor will instead duplicate another LLOC file as a replacement.\nPlease check in the Level textures that the editor chose the correct language to duplicate!", "XXL Editor", 48);
					}
					bool fnd = false;
					for (auto &dd : documents) {
						if (dd.langStrIndex == doc.langStrIndex) {
							auto it = dd.lvlLocpacks.find(lvl);
							if (it != dd.lvlLocpacks.end()) {
								llpack = it->second;
								fnd = true;
								break;
							}
						}
					}
					assert(fnd && "Missing LLOC file, no similar found!");
				}
				else {
					IOFile llocfile(tbuf, "rb");
					llpack.deserialize(&kenv, &llocfile);
				}

				if (Loc_CKGraphic *kgfx = llpack.get<Loc_CKGraphic>()) {
					auto &texvec = doc.lvlTextures[lvl];
					for (auto &ktex : kgfx->textures)
						texvec.push_back(gfx->createTexture(ktex.img));
				}
			}
		}
		langLoaded = true;
	}

	if (documents.empty())
		return;

	static int langid = 0;
	static unsigned int selfont = 0, selglyph = 0;

	char tbuf[256];
	sprintf_s(tbuf, "%i: %s", langid, documents[langid].stdTextU8[documents[langid].langStrIndex].c_str());
	if (ImGui::BeginCombo("Language", tbuf)) {
		for (int i = 0; i < documents.size(); i++) {
			sprintf_s(tbuf, "%i: %s", i, documents[i].stdTextU8[documents[i].langStrIndex].c_str());
			if (ImGui::Selectable(tbuf)) {
				langid = i;
				selglyph = 0;
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Save all")) {
		std::vector<uint32_t> globStrIndices, globIDs;
		for (auto &doc : documents) {
			globStrIndices.push_back(doc.langStrIndex);
			globIDs.push_back(doc.langID);
		}
		for (int langid = 0; langid < documents.size(); langid++) {
			LocalDocument &doc = documents[langid];
			if (Loc_CLocManager *loc = doc.locpack.get<Loc_CLocManager>()) {
				loc->numLanguages = documents.size();
				loc->langStrIndices = globStrIndices;
				loc->langIDs = globIDs;
				for (int i = 0; i < loc->trcStrings.size(); i++) {
					std::wstring &lstr = loc->trcStrings[i].second;
					lstr = TextConverter::encode(doc.trcTextU8[i]);
					if (!lstr.empty())
						lstr.push_back(0);
				}
				for (int i = 0; i < loc->stdStrings.size(); i++) {
					std::wstring &lstr = loc->stdStrings[i];
					lstr = TextConverter::encode(doc.stdTextU8[i]);
					if (!lstr.empty())
						lstr.push_back(0);
				}
			}

			char tbuf[512];
			sprintf_s(tbuf, "%s/%02uGLOC.%s", kenv.outGamePath.c_str(), langid, KEnvironment::platformExt[kenv.platform]);
			IOFile gloc = IOFile(tbuf, "wb");
			doc.locpack.serialize(&kenv, &gloc);

			// LLOCs
			for (int lvl : fndLevels) {
				sprintf_s(tbuf, "%s/LVL%03u/%02uLLOC%02u.%s", kenv.outGamePath.c_str(), lvl, langid, lvl, KEnvironment::platformExt[kenv.platform]);
				IOFile lloc = IOFile(tbuf, "wb");
				doc.lvlLocpacks.at(lvl).serialize(&kenv, &lloc);
			}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Duplicate language")) {
		documents.emplace_back(documents[langid]);
		documents.back().langID = 0xFFFFFFFF;
		documents.back().fontTextures.clear();
		if (Loc_CManager2d *lmgr = documents.back().locpack.get<Loc_CManager2d>()) {
			for (auto &tex : lmgr->piTexDict.textures) {
				documents.back().fontTextures.push_back(gfx->createTexture(tex.image));
			}
		}
		documents.back().lvlTextures.clear();
		for (auto &e : documents.back().lvlLocpacks) {
			if (Loc_CKGraphic *kgfx = e.second.get<Loc_CKGraphic>()) {
				auto &texvec = documents.back().lvlTextures[e.first];
				for (auto &ktex : kgfx->textures)
					texvec.push_back(gfx->createTexture(ktex.img));
			}
		}
		langid = documents.size() - 1;
	}
	if (documents.size() > 1) {
		ImGui::SameLine();
		if (ImGui::Button("Remove language")) {
			for (texture_t tex : documents[langid].fontTextures)
				gfx->deleteTexture(tex);
			for (auto &lt : documents[langid].lvlTextures)
				for (texture_t &tex : lt.second)
					gfx->deleteTexture(tex);
			documents.erase(documents.begin() + langid);
			if (langid >= documents.size())
				langid = documents.size() - 1;
		}
	}

	auto &doc = documents[langid];

	if (ImGui::BeginTabBar("LangTabBar")) {
		if (Loc_CLocManager *loc = doc.locpack.get<Loc_CLocManager>()) {
			if (ImGui::BeginTabItem("TRC Text")) {
				if (ImGui::Button("Export all")) {
					std::string filepath = SaveDialogBox(g_window, "Tab-separated values file (*.txt)\0*.TXT\0\0", "txt");
					if (!filepath.empty()) {
						FILE *tsv;
						if (!fopen_s(&tsv, filepath.c_str(), "w")) {
							for (int i = 0; i < loc->stdStrings.size(); i++) {
								fprintf(tsv, "/\t%s\n", doc.stdTextU8[i].c_str());
							}
							for (int i = 0; i < loc->trcStrings.size(); i++) {
								fprintf(tsv, "%i\t%s\n", loc->trcStrings[i].first, doc.trcTextU8[i].c_str());
							}
							fclose(tsv);
						}
					}
				}
				ImGui::BeginChild("TRCTextWnd");
				for (int i = 0; i < loc->trcStrings.size(); i++) {
					auto &trc = loc->trcStrings[i];
					ImGui::Text("%u", trc.first);
					ImGui::SameLine(48.0f);
					auto &str = doc.trcTextU8[i];
					ImGui::PushID(&str);
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::InputText("##Text", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Standard Text")) {
				ImGui::BeginChild("StdTextWnd");
				for (int i = 0; i < loc->stdStrings.size(); i++) {
					ImGui::Text("%i", i);
					ImGui::SameLine(48.0f);
					auto &str = doc.stdTextU8[i];
					ImGui::PushID(&str);
					ImGui::SetNextItemWidth(-1.0f);
					ImGui::InputText("##Text", (char*)str.c_str(), str.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &str);
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
		}

		if (Loc_CManager2d *lmgr = doc.locpack.get<Loc_CManager2d>()) {
			if (ImGui::BeginTabItem("Font textures")) {
				ImGui::BeginChild("FontTexWnd");
				for (int i = 0; i < doc.fontTextures.size(); i++) {
					auto &tex = lmgr->piTexDict.textures[i];
					ImGui::PushID(&tex);
					ImGui::BulletText("%s (%i*%i*%i)", tex.texture.name.c_str(), tex.image.width, tex.image.height, tex.image.bpp);
					if (ImGui::Button("Export")) {
						std::string filepath = SaveDialogBox(g_window, "PNG Image\0*.PNG\0\0", "png", tex.texture.name.c_str());
						if (!filepath.empty()) {
							RwImage cimg = tex.image.convertToRGBA32();
							stbi_write_png(filepath.c_str(), cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Replace")) {
						std::string filepath = OpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
						if (!filepath.empty()) {
							tex.image = RwImage::loadFromFile(filepath.c_str());
							gfx->deleteTexture(doc.fontTextures[i]);
							doc.fontTextures[i] = gfx->createTexture(tex.image);
						}
					}
					ImGui::SameLine();
					if (ImGui::Button("Fix")) {
						if (tex.image.bpp == 32) {
							uint32_t *pix = (uint32_t*)tex.image.pixels.data();
							int sz = tex.image.width * tex.image.height;
							for (int p = 0; p < sz; p++)
								if (pix[p] == 0xFF00FF00 || pix[p] == 0xFF8000FF)
									pix[p] &= 0x00FFFFFF;
							gfx->deleteTexture(doc.fontTextures[i]);
							doc.fontTextures[i] = gfx->createTexture(tex.image);
						}
					}
					ImGui::Image(doc.fontTextures[i], ImVec2(tex.image.width, tex.image.height));
					ImGui::PopID();
				}
				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Font glyphs")) {
				if (ImGui::InputInt("Font", (int*)&selfont))
					selglyph = 0;
				selfont %= lmgr->fonts.size();
				RwFont2D &font = lmgr->fonts[selfont].second;
				ImGui::Columns(2);

				if (ImGui::Button("Add")) {
					ImGui::OpenPopup("AddGlyphPopup");
				}
				if (ImGui::BeginPopup("AddGlyphPopup")) {
					static uint16_t chToAdd = 0;
					static std::string charPreview;
					ImGui::TextUnformatted("Add character:");
					bool mod = false;
					mod |= ImGui::InputScalar("Dec", ImGuiDataType_U16, &chToAdd);
					mod |= ImGui::InputScalar("Hex", ImGuiDataType_U16, &chToAdd, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
					if (ImGui::InputText("Char", (char*)charPreview.c_str(), charPreview.capacity() + 1, ImGuiInputTextFlags_CallbackResize, IGStdStringInputCallback, &charPreview)) {
						chToAdd = utf8ToWchar(charPreview.c_str())[0];
					}
					ImGui::PushButtonRepeat(true);
					if (ImGui::ArrowButton("PreviousChar", ImGuiDir_Left)) { chToAdd--; mod = true; }
					ImGui::SameLine();
					if (ImGui::ArrowButton("NextChar", ImGuiDir_Right)) { chToAdd++; mod = true; }
					//ImGui::SameLine();
					ImGui::PopButtonRepeat();
					if (mod) {
						wchar_t wcs[2] = { chToAdd, 0 };
						charPreview = wcharToUtf8(wcs);
					}
					uint16_t *slotdest = nullptr;
					if (chToAdd < 128)
						slotdest = &font.charGlyphTable[chToAdd];
					else if (chToAdd >= font.firstWideChar && chToAdd < (font.firstWideChar + font.wideGlyphTable.size()))
						slotdest = &font.wideGlyphTable[chToAdd - font.firstWideChar];
					if (!(slotdest && *slotdest != 0xFFFF)) {
						if (ImGui::Button("OK")) {
							uint16_t glindex = (uint16_t)font.glyphs.size();
							font.glyphs.emplace_back();
							auto &glyph = font.glyphs.back();
							glyph.coords = { 0.0f, 0.0f, 0.25f, 0.25f };
							glyph.glUnk1 = 1.0f;
							glyph.texIndex = 0;
							if (slotdest) {
								*slotdest = glindex;
							}
							else {
								// need to resize widechar glyph table
								if (chToAdd < font.firstWideChar) {
									size_t origsize = font.wideGlyphTable.size();
									size_t origstart = font.firstWideChar - chToAdd;
									font.wideGlyphTable.resize(origsize + (font.firstWideChar - chToAdd));
									std::move(font.wideGlyphTable.begin(), font.wideGlyphTable.begin() + origsize, font.wideGlyphTable.begin() + origstart);
									font.wideGlyphTable[0] = glindex;
									for (int i = 1; i < origstart; i++)
										font.wideGlyphTable[i] = 0xFFFF;
									font.firstWideChar = chToAdd;
								}
								else if (chToAdd >= font.firstWideChar + font.wideGlyphTable.size()) {
									size_t origsize = font.wideGlyphTable.size();
									font.wideGlyphTable.resize(chToAdd - font.firstWideChar + 1);
									std::fill(font.wideGlyphTable.begin() + origsize, font.wideGlyphTable.end() - 1, 0xFFFF);
									font.wideGlyphTable.back() = glindex;
								}
								else assert(nullptr && "nani?!");
							}
						}
					}
					else {
						ImGui::AlignTextToFramePadding();
						ImGui::TextUnformatted("Character already has a glyph!");
					}
					ImGui::EndPopup();
				}

				ImGui::BeginChild("GlyphList");

				auto enumchar = [&doc, &font](int c, int g) {
					auto &glyph = font.glyphs[g];
					ImGui::PushID(&glyph);
					if (ImGui::Selectable("##glyph", selglyph == g, 0, ImVec2(0.0f, 32.0f)))
						selglyph = g;

					if (ImGui::BeginDragDropSource()) {
						ImGui::SetDragDropPayload("Glyph", &g, sizeof(g));
						wchar_t wbuf[2] = { (wchar_t)c, 0 };
						ImGui::Text("Glyph #%i\nChar %s (0x%04X)", g, wcharToUtf8(wbuf).c_str(), c);
						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Glyph")) {
							glyph = font.glyphs[*(int*)payload->Data];
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					auto &ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
					float ratio = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
					ImGui::Image(doc.fontTextures[ti], ImVec2(ratio * 32, 32), ImVec2(glyph.coords[0], glyph.coords[1]), ImVec2(glyph.coords[2], glyph.coords[3]));

					ImGui::SameLine(48.0f);
					wchar_t wbuf[2] = { (wchar_t)c, 0 };
					ImGui::Text("Glyph #%i\nChar %s (0x%04X)", g, wcharToUtf8(wbuf).c_str(), c);
					ImGui::PopID();
				};

				for (int c = 0; c < 128; c++) {
					uint16_t g = font.charGlyphTable[c];
					if (g != 0xFFFF)
						enumchar(c, g);
				}
				for (int c = 0; c < font.wideGlyphTable.size(); c++) {
					uint16_t g = font.wideGlyphTable[c];
					if (g != 0xFFFF)
						enumchar(c + font.firstWideChar, g);
				}
				ImGui::EndChild();

				ImGui::NextColumn();

				auto &glyph = font.glyphs[selglyph];
				bool mod = false;
				mod |= ImGui::DragFloat2("UV Low", &glyph.coords[0], 0.01f);
				mod |= ImGui::DragFloat2("UV High", &glyph.coords[2], 0.01f);
				ImGui::DragFloat("w/h", &glyph.glUnk1);
				ImGui::InputScalar("Texture", ImGuiDataType_U8, &glyph.texIndex);
				glyph.texIndex %= font.texNames.size();

				if (mod) {
					glyph.glUnk1 = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
				}

				ImGui::Spacing();

				static bool hasBorders = false;
				ImGui::Checkbox("Has Borders (to set V coord. correctly)", &hasBorders);
				ImGui::InputFloat("Auto height", &font.glyphHeight);

				ImGui::BeginChild("FontTexPreview", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
				ImDrawList *drawList = ImGui::GetWindowDrawList();
				auto &ti = doc.fntTexMap[font.texNames[glyph.texIndex]];
				auto &img = lmgr->piTexDict.textures[ti].image;
				ImVec2 pos = ImGui::GetCursorScreenPos();
				drawList->AddImage(doc.fontTextures[ti], pos, ImVec2(pos.x + img.width, pos.y + img.height));
				ImVec2 pmin = ImVec2(std::floor(pos.x + glyph.coords[0] * img.width), std::floor(pos.y + glyph.coords[1] * img.height));
				ImVec2 pmax = ImVec2(std::floor(pos.x + glyph.coords[2] * img.width), std::floor(pos.y + glyph.coords[3] * img.height));
				drawList->AddRect(pmin, pmax, ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0, 1)));

				ImVec2 spos = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("FontTexPreview", ImVec2(img.width, img.height), ImGuiButtonFlags_MouseButtonLeft);
				if (ImGui::IsItemClicked()) {
					glyph.coords[0] = glyph.coords[2] = (ImGui::GetMousePos().x - spos.x + 0.5f) / img.width;
					int rhi = (int)(font.glyphHeight + (hasBorders ? 3 : 1));
					int row = (int)(ImGui::GetMousePos().y - spos.y) / rhi;
					float off = hasBorders ? 2.5f : 0.5f;
					glyph.coords[1] = ((float)(row * rhi) + off) / img.height;
					glyph.coords[3] = ((float)(row * rhi) + off + font.glyphHeight) / img.height;
					glyph.glUnk1 = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
				}
				if (ImGui::IsItemActive() && ImGui::IsItemHovered()) {
					glyph.coords[2] = (ImGui::GetMousePos().x - spos.x + 0.5f) / img.width;
					glyph.glUnk1 = std::abs((glyph.coords[2] - glyph.coords[0]) / (glyph.coords[3] - glyph.coords[1]));
				}
				ImGui::EndChild();

				ImGui::Columns();
				ImGui::EndTabItem();
			}
		}

		if (ImGui::BeginTabItem("Level textures")) {
			static int sellvl = 0;
			ImGui::InputInt("Level", &sellvl);
			auto it = doc.lvlLocpacks.find(sellvl);
			if (it != doc.lvlLocpacks.end()) {
				KLocalPack &llpack = it->second;
				if (Loc_CKGraphic *kgfx = llpack.get<Loc_CKGraphic>()) {
					for (int t = 0; t < kgfx->textures.size(); t++) {
						auto &tex = kgfx->textures[t];
						ImGui::BulletText("%s", tex.name.c_str());
						if (ImGui::Button("Export")) {
							std::string filepath = SaveDialogBox(g_window, "PNG Image\0*.PNG\0\0", "png", tex.name.c_str());
							if (!filepath.empty()) {
								RwImage cimg = tex.img.convertToRGBA32();
								stbi_write_png(filepath.c_str(), cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Replace")) {
							std::string filepath = OpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
							if (!filepath.empty()) {
								tex.img = RwImage::loadFromFile(filepath.c_str());
								gfx->deleteTexture(doc.lvlTextures[sellvl][t]);
								doc.lvlTextures[sellvl][t] = gfx->createTexture(tex.img);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Replace in all levels")) {
							std::string filepath = OpenDialogBox(g_window, "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0", nullptr);
							if (!filepath.empty()) {
								RwImage newimg = RwImage::loadFromFile(filepath.c_str());
								for (auto &e : doc.lvlLocpacks) {
									if (Loc_CKGraphic *kgfx = e.second.get<Loc_CKGraphic>()) {
										int c = 0;
										for (auto &cand : kgfx->textures) {
											if (cand.name == tex.name) {
												cand.img = newimg;
												gfx->deleteTexture(doc.lvlTextures[e.first][c]);
												doc.lvlTextures[e.first][c] = gfx->createTexture(tex.img);
											}
											c++;
										}
									}
								}
							}
						}
						ImGui::Image(doc.lvlTextures[sellvl][t], ImVec2(tex.img.width, tex.img.height));
					}
				}
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Misc")) {
			ImGui::InputScalar("Lang text index", ImGuiDataType_U32, &doc.langStrIndex);
			ImGui::InputScalar("Lang ID", ImGuiDataType_S32, &doc.langID);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void EditorInterface::IGTriggerEditor()
{
	CKSrvTrigger *srvTrigger = kenv.levelObjects.getFirst<CKSrvTrigger>();
	if (!srvTrigger) return;
	auto enumDomain = [this](CKTriggerDomain *domain, const auto &rec) -> void {
		bool open = ImGui::TreeNode(domain, "%s", kenv.getObjectName(domain));
		if (open) {
			for (const auto &subdom : domain->subdomains)
				rec(subdom.get(), rec);
			for (const auto &trigger : domain->triggers) {
				bool open = ImGui::TreeNode(trigger.get(), "%s", kenv.getObjectName(trigger.get()));
				if (open) {
					IGObjectSelectorRef(kenv, "Condition", trigger->condition);
					for (auto &act : trigger->actions) {
						ImGui::Separator();
						ImGui::PushID(&act);
						ImGui::SetNextItemWidth(48.0f);
						ImGui::InputScalar("##EventID", ImGuiDataType_U16, &act.event, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
						ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
						ImGui::Text("->");
						ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
						ImGui::SetNextItemWidth(-1.0f);
						IGObjectSelectorRef(kenv, "##EventObj", act.target);
						ImGui::TextUnformatted("Value type:");
						ImGui::SameLine(); ImGui::RadioButton("int8", (int*)&act.valType, 0);
						ImGui::SameLine(); ImGui::RadioButton("int32", (int*)&act.valType, 1);
						ImGui::SameLine(); ImGui::RadioButton("float", (int*)&act.valType, 2);
						ImGui::SameLine(); ImGui::RadioButton("kobjref", (int*)&act.valType, 3);
						switch (act.valType) {
						case 0: ImGui::InputScalar("int8", ImGuiDataType_U8, &act.valU8); break;
						case 1: ImGui::InputScalar("int32", ImGuiDataType_U32, &act.valU32); break;
						case 2: ImGui::InputScalar("float", ImGuiDataType_Float, &act.valFloat); break;
						case 3: IGObjectSelectorRef(kenv, "kobjref", act.valRef); break;
						}
						ImGui::PopID();
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	};
	enumDomain(srvTrigger->rootDomain.get(), enumDomain);
}

void EditorInterface::checkNodeRayCollision(CKSceneNode * node, const Vector3 &rayDir, const Matrix &matrix)
{
	if (!node) return;
	
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
						while (!tosel->parent->isSubclassOf<CSGSectorRoot>())
							tosel = tosel->parent.get();
						rayHits.push_back(std::make_unique<NodeSelection>(*this, ixres.second, tosel));
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
						RwGeometry *rwgeo = clm->_teamDict._bings[part]._clump->atomic.geometry.get();
						checkGeo(rwgeo);
					}
			}
		}
		else if (node->isSubclassOf<CNode>() /*&& !node->isSubclassOf<CSGSectorRoot>()*/) {
			for (CKAnyGeometry *kgeo = node->cast<CNode>()->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
				CKAnyGeometry *rgeo = kgeo->duplicateGeo ? kgeo->duplicateGeo.get() : kgeo;
				if (RwMiniClump *clump = rgeo->clump)
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
		if(showNodes && kenv.hasClass<CSGSectorRoot>())
			checkNodeRayCollision(objlist.getFirst<CSGSectorRoot>(), rayDir, Matrix::getIdentity());

		// Beacons
		if (showBeacons && kenv.hasClass<CKBeaconKluster>()) {
			for (CKBeaconKluster *kluster = objlist.getFirst<CKBeaconKluster>(); kluster; kluster = kluster->nextKluster.get()) {
				for (auto &bing : kluster->bings) {
					if (bing.active) {
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
								rayHits.push_back(std::make_unique<BeaconSelection>(*this, rsi.second, &beacon, kluster));
							}
						}
					}
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
	if (showingSector < 0)
		for (auto &str : kenv.sectorObjects)
			checkOnSector(str);
	else if (showingSector < kenv.numSectors)
		checkOnSector(kenv.sectorObjects[showingSector]);

	// Squads
	if (showSquadChoreos && kenv.hasClass<CKGrpEnemy>()) {
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
	if (showMarkers && kenv.hasClass<CKSrvMarker>()) {
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

	if (!rayHits.empty()) {
		auto comp = [this](const std::unique_ptr<UISelection> &a, const std::unique_ptr<UISelection> &b) -> bool {
			return (camera.position - a->hitPosition).len3() < (camera.position - b->hitPosition).len3();
		};
		nearestRayHit = std::min_element(rayHits.begin(), rayHits.end(), comp)->get();
	}
}
