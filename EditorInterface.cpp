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
#include "imgui/ImGuizmo.h"
#include "GameLauncher.h"
#include "Shape.h"
#include "CKService.h"

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
					rrcpnt->data = becpnt->data;
					//....
					rrcpnt->rrCylinderRadius = 1.0f;
					rrcpnt->rrCylinderHeight = 1.0f;
					rrcpnt->runk3 = Vector3(1.0f, 1.0f, 1.0f);
					rrcpnt->runk4 = 0;
					rrcpnt->rrFireDistance = 3.0f;
					rrcpnt->runk6 = 0;
					rrcpnt->rrFlySpeed = 5.0f;
					rrcpnt->rrRomanAimFactor = 10.0f;
					rrcpnt->runk9 = kenv.levelObjects.getClassType(2, 28).objects[0]; // Asterix Hook
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

	SDL_AudioDeviceID audiodevid;

	void InitSnd() {
		static bool initdone = false;
		if (initdone) return;
		SDL_AudioSpec spec, have;
		memset(&spec, 0, sizeof(spec));
		spec.freq = 22050;
		spec.format = AUDIO_S16;
		spec.channels = 1;
		spec.samples = 4096;
		audiodevid = SDL_OpenAudioDevice(NULL, 0, &spec, &have, 0);
		assert(audiodevid);
		SDL_PauseAudioDevice(audiodevid, 0);
		initdone = true;
	}

	void PlaySnd(KEnvironment &kenv, RwSound &snd) {
		InitSnd();
		SDL_ClearQueuedAudio(audiodevid);
		SDL_QueueAudio(audiodevid, snd.data.data.data(), snd.data.data.size());
	}
}

EditorInterface::EditorInterface(KEnvironment & kenv, Window * window, Renderer * gfx)
	: kenv(kenv), g_window(window), gfx(gfx), protexdict(gfx), progeocache(gfx), gndmdlcache(gfx)
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
	//selgeoPos = camera.position + getRay(camera, g_window).normal() * 10.0f;
	static bool clicking = false;
	if (g_window->getMouseDown(SDL_BUTTON_LEFT)) {
		if (!clicking) {
			clicking = true;
			selectionType = 0;
			selNode = nullptr;
			selBeacon = nullptr;
			selBeaconKluster = nullptr;
			selGround = nullptr;
			checkMouseRay();
			if (rayHits.size()) {
				selectionType = nearestRayHit.type;
				if (selectionType == 1) {
					CKSceneNode *obj = (CKSceneNode*)nearestRayHit.obj;
					if (!obj->isSubclassOf<CSGSectorRoot>()) {
						while (!obj->parent->isSubclassOf<CSGSectorRoot>())
							obj = obj->parent.get();
						selNode = obj;
					}
				}
				else if (selectionType == 2) {
					selBeacon = nearestRayHit.obj;
					selBeaconKluster = nearestRayHit.obj2;
				}
				else if (selectionType == 3) {
					selGround = (CGround*)nearestRayHit.obj;
				}
				else {
					selectionType = 0;
				}
			}
		}
	}
	else
		clicking = false;

	static Matrix gzmat = Matrix::getIdentity();
	static int gzoperation = ImGuizmo::TRANSLATE;
	if(!ImGuizmo::IsUsing())
		gzoperation = g_window->isCtrlPressed() ? ImGuizmo::ROTATE : (g_window->isShiftPressed() ? ImGuizmo::SCALE : ImGuizmo::TRANSLATE);
	ImGuizmo::BeginFrame();
	ImGuizmo::SetRect(0, 0, g_window->getWidth(), g_window->getHeight());
	if(selectionType == 1 && selNode) {
		for (int i = 0; i < 4; i++)
			selNode->transform.m[i][3] = (i == 3) ? 1.0f : 0.0f;
		ImGuizmo::Manipulate(camera.viewMatrix.v, camera.projMatrix.v, (ImGuizmo::OPERATION)gzoperation, ImGuizmo::WORLD, selNode->transform.v);

		//ImGui::Begin("Test ImGuizmo");
		//ImGui::RadioButton("Translate", &gzoperation, ImGuizmo::TRANSLATE); ImGui::SameLine();
		//ImGui::RadioButton("Rotate", &gzoperation, ImGuizmo::ROTATE); ImGui::SameLine();
		//ImGui::RadioButton("Scale", &gzoperation, ImGuizmo::SCALE);
		//ImGui::DragFloat4("1", &selNode->transform._11);
		//ImGui::DragFloat4("2", &selNode->transform._21);
		//ImGui::DragFloat4("3", &selNode->transform._31);
		//ImGui::DragFloat4("4", &selNode->transform._41);
		//ImGui::End();
	}
	else if (selectionType == 2 && selBeacon && selBeaconKluster) {
		CKBeaconKluster::Beacon *beacon = (CKBeaconKluster::Beacon *)selBeacon;
		Matrix mat = Matrix::getTranslationMatrix(beacon->getPosition());
		ImGuizmo::Manipulate(camera.viewMatrix.v, camera.projMatrix.v, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, mat.v);
		beacon->setPosition(Vector3(mat._41, mat._42, mat._43));
		UpdateBeaconKlusterBounds((CKBeaconKluster*)selBeaconKluster);
	}

	ImGui::Begin("Main");
	ImGui::Text("Hello to the Asterix Games Modding Discord!");
	ImGui::Text("FPS: %i", lastFps);
	ImGui::BeginTabBar("MainTabBar", 0);
	if (ImGui::BeginTabItem("Main")) {
		IGMain();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Textures")) {
		IGTextureEditor();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Geo")) {
		IGGeometryViewer();
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
	//if (ImGui::BeginTabItem("Events")) {
	//	IGEventEditor();
	//	ImGui::EndTabItem();
	//}
	if (ImGui::BeginTabItem("Sounds")) {
		IGSoundEditor();
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Squads")) {
		IGSquadEditor();
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

	if (!rayHits.empty()) {
		const Vector3 rad = Vector3(1, 1, 1) * 0.1f;
		drawBox(nearestRayHit.hitPos + rad, nearestRayHit.hitPos - rad);
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
		auto drawGroundBounds = [this](CGround* gnd) {
			gndmdlcache.getModel(gnd)->draw(showInfiniteWalls);
		};
		for (CKObject* obj : kenv.levelObjects.getClassType<CGround>().objects)
			drawGroundBounds(obj->cast<CGround>());
		for (auto &str : kenv.sectorObjects)
			for (CKObject *obj : str.getClassType<CGround>().objects)
				drawGroundBounds(obj->cast<CGround>());
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

	if (showSquadBoxes) {
		for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
			CKGrpSquadEnemy *squad = osquad->cast<CKGrpSquadEnemy>();
			for (const auto &bb : { squad->sqUnk3, squad->sqUnk4 }) {
				Vector3 v1(bb[0], bb[1], bb[2]);
				Vector3 v2(bb[3], bb[4], bb[5]);
				drawBox(v1-v2*0.5f, v1+v2*0.5f);
			}
		}
	}
}

void EditorInterface::IGMain()
{
	static int levelNum = 8;
	ImGui::InputInt("Level number##LevelNum", &levelNum);
	if (ImGui::Button("Load")) {
		selGeometry = nullptr;
		selectionType = 0;
		selNode = nullptr;
		selBeacon = nullptr;
		selBeaconKluster = nullptr;
		selGround = nullptr;

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
		static GameLauncher launcher;
		launcher.loadLevel(levelNum);
	}
	ImGui::Separator();
	ImGui::DragFloat3("Cam pos", &camera.position.x, 0.1f);
	ImGui::DragFloat3("Cam ori", &camera.orientation.x, 0.1f);
	ImGui::DragFloat("Cam speed", &_camspeed, 0.1f);
	ImGui::DragFloatRange2("Depth range", &camera.nearDist, &camera.farDist);
	ImGui::Checkbox("Orthographic", &camera.orthoMode);
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
	ImGui::Checkbox("Lines & splines", &showLines); ImGui::SameLine();
	ImGui::Checkbox("Squad bounds", &showSquadBoxes);
}

void EditorInterface::IGMiscTab()
{
	ImGui::Checkbox("Show ImGui Demo", &showImGuiDemo);
	if (ImGui::Button("Rocket Romans \\o/"))
		GimmeTheRocketRomans(kenv);
	if(ImGui::IsItemHovered())
		ImGui::SetTooltip("Transform all Basic Enemies to Rocket Romans");
	if (ImGui::CollapsingHeader("Ray Hits")) {
		ImGui::Columns(2);
		for (auto &hit : rayHits) {
			ImGui::BulletText("%f", (camera.position - hit.hitPos).len3());
			ImGui::NextColumn();
			if (hit.type == 1)
				ImGui::Text("%i %p %s", hit.type, hit.obj, ((CKSceneNode*)hit.obj)->getClassName());
			else
				ImGui::Text("%i %p", hit.type, hit.obj);
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
	if (ImGui::Button("Update all kluster sphere bounds")) {
		for (CKObject *bk : kenv.levelObjects.getClassType<CKBeaconKluster>().objects)
			UpdateBeaconKlusterBounds(bk->cast<CKBeaconKluster>());
		for(auto &str : kenv.sectorObjects)
			for (CKObject *bk : str.getClassType<CKBeaconKluster>().objects)
				UpdateBeaconKlusterBounds(bk->cast<CKBeaconKluster>());
	}
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
	if (ImGui::Button("Add beacon")) {
		ImGui::OpenPopup("AddBeacon");
	}
	if(ImGui::BeginPopup("AddBeacon")) {
		CKSrvBeacon *srv = kenv.levelObjects.getFirst<CKSrvBeacon>();
		for (auto &hs : srv->handlers) {
			char buf[128];
			sprintf_s(buf, "%s %02X %02X %02X %02X %02X", hs.object->getClassName(), hs.unk2a, hs.numBits, hs.handlerIndex, hs.handlerId, hs.persistent);
			if (ImGui::MenuItem(buf)) {
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
		}
		ImGui::EndPopup();
	}
	auto enumBeaconKluster = [this](CKBeaconKluster* bk) {
		if (ImGui::TreeNode(bk, "pos (%f, %f, %f) radius %f", bk->bounds.center.x, bk->bounds.center.y, bk->bounds.center.z, bk->bounds.radius)) {
			ImGui::DragFloat3("Center##beaconKluster", &bk->bounds.center.x, 0.1f);
			ImGui::DragFloat("Radius##beaconKluster", &bk->bounds.radius, 0.1f);
			for (auto &bing : bk->bings) {
				if(!bing.beacons.empty())
					ImGui::Text("%02X %02X %02X %02X %02X %02X %04X %08X", bing.unk2a, bing.numBits, bing.handlerId, bing.sectorIndex, bing.klusterIndex, bing.handlerIndex, bing.bitIndex, bing.unk6);
				for (auto &beacon : bing.beacons) {
					ImGui::PushID(&beacon);
					Vector3 pos = Vector3(beacon.posx, beacon.posy, beacon.posz) * 0.1f;
					bool tn_open = ImGui::TreeNodeEx("beacon", ImGuiTreeNodeFlags_OpenOnArrow, "(%i,%i) %f %f %f 0x%04X", bing.handler->getClassCategory(), bing.handler->getClassID(), pos.x, pos.y, pos.z, beacon.params);
					//if (ImGui::Selectable("##beacon")) {
					if (ImGui::IsItemClicked()) {
						camera.position = pos - camera.direction * 5.0f;
						selBeacon = &beacon;
						selBeaconKluster = bk;
					}
					if (tn_open) {
						bool mod = false;
						mod |= ImGui::DragScalarN("Position##beacon", ImGuiDataType_S16, &beacon.posx, 3, 0.1f);
						mod |= ImGui::InputScalar("Params##beacon", ImGuiDataType_U16, &beacon.params, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
						if (mod)
							UpdateBeaconKlusterBounds(bk);
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
}

void EditorInterface::IGGeometryViewer()
{
	ImGui::DragFloat3("Geo pos", &selgeoPos.x, 0.1f);
	if (ImGui::Button("Move geo to front"))
		selgeoPos = camera.position + camera.direction * 3;
	ImGui::SameLine();
	if (ImGui::Button("Import DFF")) {
		char filepath[300] = "\0";
		OPENFILENAME ofn = {};
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)g_window->getNativeWindow();
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFilter = "Renderware Clump\0*.DFF\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = sizeof(filepath);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = "dff";
		if (GetOpenFileNameA(&ofn)) {
			printf("%s\n", filepath);

			RwClump *impClump = LoadDFF(filepath); //"C:\\Users\\Adrien\\Desktop\\kthings\\xecpp_dff_test\\GameCube Hat\\gamecube.blend.dff"
												   //cloneManager->_teamDict._bings[39]._clump->atomic.geometry = std::unique_ptr<RwGeometry>(new RwGeometry(*pyra->geoList.geometries[0]));
			*selGeometry = *impClump->geoList.geometries[0];
			progeocache.clear();
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
}

void EditorInterface::IGTextureEditor()
{
	CTextureDictionary *texDict = kenv.levelObjects.getObject<CTextureDictionary>(0);
	if (selTexID >= texDict->textures.size())
		selTexID = texDict->textures.size() - 1;
	if (ImGui::Button("Insert")) {
		char filepath[300] = "\0";
		OPENFILENAME ofn = {};
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)g_window->getNativeWindow();
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFilter = "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = sizeof(filepath);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
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
		ofn.hwndOwner = (HWND)g_window->getNativeWindow();
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFilter = "Image\0*.PNG;*.BMP;*.TGA;*.GIF;*.HDR;*.PSD;*.JPG;*.JPEG\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = sizeof(filepath);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
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
	if (ImGui::Button("Export")) {
		char filepath[300];
		auto &tex = texDict->textures[selTexID];
		strcpy_s(filepath, tex.name);
		OPENFILENAME ofn = {};
		memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)g_window->getNativeWindow();
		ofn.hInstance = GetModuleHandle(NULL);
		ofn.lpstrFilter = "PNG Image\0*.PNG\0\0";
		ofn.nFilterIndex = 0;
		ofn.lpstrFile = filepath;
		ofn.nMaxFile = sizeof(filepath);
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		ofn.lpstrDefExt = "png";
		if (GetSaveFileNameA(&ofn)) {
			printf("%s\n", filepath);
			RwImage cimg = tex.image.convertToRGBA32();
			stbi_write_png(filepath, cimg.width, cimg.height, 4, cimg.pixels.data(), cimg.pitch);
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
		ImGui::Image(protexdict.find(texDict->textures[i].name).second, ImVec2(32, 32));
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
		ImGui::Image(protexdict.find(tex.name).second, ImVec2(tex.image.width, tex.image.height));
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
			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = (HWND)g_window->getNativeWindow();
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Renderware Clump\0*.DFF\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
			ofn.lpstrDefExt = "dff";
			if (GetOpenFileNameA(&ofn)) {
				RwClump *impClump = LoadDFF(filepath);

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
			else printf("GetOpenFileName fail: 0x%X\n", CommDlgExtendedError());
		}
		if (ImGui::Button("Export geometry to DFF")) {
			char filepath[300] = "\0";
			OPENFILENAME ofn = {};
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = (HWND)g_window->getNativeWindow();
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFilter = "Renderware Clump\0*.DFF\0\0";
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = filepath;
			ofn.nMaxFile = sizeof(filepath);
			ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
			ofn.lpstrDefExt = "dff";
			if (GetSaveFileNameA(&ofn)) {
				CKAnyGeometry *kgeo = geonode->geometry.get();
				RwGeometry rwgeo = *kgeo->clump->atomic.geometry.get();
				kgeo = kgeo->nextGeo.get();
				while (kgeo) {
					rwgeo.merge(*kgeo->clump->atomic.geometry);
					kgeo = kgeo->nextGeo.get();
				}

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

				if (geonode->isSubclassOf<CAnimatedNode>()) {
					RwFrameList *framelist = geonode->cast<CAnimatedNode>()->frameList;
					RwExtHAnim *hanim = (RwExtHAnim*)framelist->extensions[0].find(0x11E);
					assert(hanim);
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

				printf("done\n");
				IOFile dff(filepath, "wb");
				clump.serialize(&dff);
			}
		}
	}
}

void EditorInterface::IGGroundEditor()
{
	if (ImGui::Button("Find duplicates in klusters")) {
		std::vector<KObjectList*> olvec;
		olvec.push_back(&kenv.levelObjects);
		for (auto &str : kenv.sectorObjects)
			olvec.push_back(&str);
		for (int i = 0; i < olvec.size(); i++) {
			CKMeshKluster *mk1 = olvec[i]->getFirst<CKMeshKluster>();
			for (int j = i + 1; j < olvec.size(); j++) {
				CKMeshKluster *mk2 = olvec[j]->getFirst<CKMeshKluster>();
				int k = 0;
				for (auto &gnd : mk2->grounds) {
					auto it = std::find(mk1->grounds.begin(), mk1->grounds.end(), gnd);
					if (it != mk1->grounds.end()) {
						printf("str_%i[%i] == str_%i[%i]\n", i, it - mk1->grounds.begin(), j, k);
					}
					k++;
				}
			}
		}
	}
	ImGui::Columns(2);
	auto feobjlist = [this](KObjectList &objlist, const char *desc) {
		if (CKMeshKluster *mkluster = objlist.getFirst<CKMeshKluster>()) {
			if (ImGui::TreeNode(mkluster, "%s", desc)) {
				for (auto &gnd : mkluster->grounds) {
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
		}
	};
	feobjlist(kenv.levelObjects, "Level");
	int x = 0;
	for (auto &str : kenv.sectorObjects) {
		char lol[64];
		sprintf_s(lol, "Sector %i", x++);
		feobjlist(str, lol);
	}
	ImGui::NextColumn();
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
				ImGui::Text("%04X -> %p (%i, %i)", srvEvent->objInfos[ev+i], obj, obj->getClassCategory(), obj->getClassID());
			}
			ImGui::TreePop();
		}
		ev += bee._1;
	}
}

void EditorInterface::IGSoundEditor()
{
	if (ImGui::Button("Randomize LVL sounds")) {
		CKSoundDictionary *sndDict = kenv.levelObjects.getFirst<CKSoundDictionary>();
		std::random_shuffle(sndDict->rwSoundDict.list.sounds.begin(), sndDict->rwSoundDict.list.sounds.end());
	}
	auto enumDict = [this](CKSoundDictionary *sndDict, int strnum) {
		if (sndDict->sounds.empty())
			return;
		if (ImGui::TreeNode(sndDict, (strnum == -1) ? "Level" : "Sector %i", strnum)) {
			for (int sndid = 0; sndid < sndDict->sounds.size(); sndid++) {
				auto &snd = sndDict->rwSoundDict.list.sounds[sndid];
				ImGui::PushID(sndid);
				if (ImGui::ArrowButton("PlaySound", ImGuiDir_Right))
					PlaySnd(kenv, snd);
				ImGui::SameLine();
				const char *name = (const char*)snd.info.name.data();
				ImGui::Text("%s", name);
				ImGui::Text("%u %u", snd.info.dings[0].sampleRate, snd.info.dings[1].sampleRate);
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
	int si = 0;
	for (CKObject *osquad : kenv.levelObjects.getClassType<CKGrpSquadEnemy>().objects) {
		CKGrpSquadEnemy *squad = osquad->cast<CKGrpSquadEnemy>();
		if (ImGui::TreeNode(squad, "Squad %i", si)) {
			for (auto &bb : { squad->sqUnk3, squad->sqUnk4 }) {
				ImGui::Text("%f %f %f %f %f %f", bb[0], bb[1], bb[2], bb[3], bb[4], bb[5]);
			}
			ImGui::Text("Num choreo: %i, Num choreo keys: %i", squad->choreographies.size(), squad->choreoKeys.size());
			for (auto &pe : squad->pools) {
				ImGui::PushID(&pe);
				ImGui::BulletText("%s %u %u %u", pe.cpnt->getClassName(), pe.u1, pe.u2, pe.u3.get() ? 1 : 0);
				ImGui::InputScalar("Enemy Count", ImGuiDataType_U16, &pe.numEnemies);
				ImGui::InputScalar("U1", ImGuiDataType_U8, &pe.u1);
				ImGui::InputScalar("U2", ImGuiDataType_U8, &pe.u2);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		si++;
	}
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
					rayHits.emplace_back(ixres.second, 1, node);
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
								rayHits.emplace_back(rsi.second, 2, &beacon, kluster);
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
								rayHits.emplace_back(rti.second, 3, ground.get());
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
		auto comp = [this](const Selection &a, const Selection &b) -> bool {
			return (camera.position - a.hitPos).len3() < (camera.position - b.hitPos).len3();
		};
		nearestRayHit = *std::min_element(rayHits.begin(), rayHits.end(), comp);
	}
}
