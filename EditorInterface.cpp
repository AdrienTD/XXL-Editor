#include "EditorInterface.h"
#include "KEnvironment.h"
#include "imguiimpl.h"
#include "rwrenderer.h"
#include "rwext.h"
#include "GameLauncher.h"
#include "Shape.h"
#include "rw.h"
#include "Image.h"
#include "GuiUtils.h"
#include "LocaleEditor.h"
#include "Encyclopedia.h"

#include "CoreClasses/CKService.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKHook.h"
#include "GameClasses/CKGameX1.h"
#include "GameClasses/CKGameX2.h"
#include "GameClasses/CKGameOG.h"

#include "EditorUI/IGSceneNodeEditor.h"
#include "EditorUI/IGCloneEditor.h"
#include "EditorUI/IGTextureEditor.h"
#include "EditorUI/IGSoundEditor.h"
#include "EditorUI/IGBeaconEditor.h"
#include "EditorUI/IGGroundEditor.h"
#include "EditorUI/IGEventEditor.h"
#include "EditorUI/IGTriggerEditor.h"
#include "EditorUI/IGHookEditor.h"
#include "EditorUI/IGSquadEditor.h"
#include "EditorUI/IGEventEditor.h"
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
#include "EditorUI/GeoUtils.h"

#include <numbers>
#include <optional>

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include <SDL2/SDL.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace GuiUtils;

namespace EditorUI {

namespace {
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

	std::optional<Vector3> getRaySphereIntersection(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &spherePos, float sphereRadius) {
		Vector3 rayDir = _rayDir.normal();
		Vector3 rs2sp = spherePos - rayStart;
		float sphereRadiusSq = sphereRadius * sphereRadius;
		if (rs2sp.sqlen3() <= sphereRadiusSq)
			return rayStart;
		float dot = rayDir.dot(rs2sp);
		if (dot < 0.0f)
			return std::nullopt;
		Vector3 shortPoint = rayStart + rayDir * dot;
		float shortDistSq = (spherePos - shortPoint).sqlen3();
		if (shortDistSq > sphereRadiusSq)
			return std::nullopt;
		Vector3 ix = shortPoint - rayDir * sqrt(sphereRadiusSq - shortDistSq);
		return ix;
	}

	std::optional<Vector3> getRayTriangleIntersection(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3) {
		Vector3 rayDir = _rayDir.normal();
		Vector3 v2 = p2 - p1, v3 = p3 - p1;
		Vector3 trinorm = v2.cross(v3).normal(); // order?
		if (trinorm == Vector3(0, 0, 0))
			return std::nullopt;
		float rayDir_dot_trinorm = rayDir.dot(trinorm);
		if (rayDir_dot_trinorm < 0.0f)
			return std::nullopt;
		float p = p1.dot(trinorm);
		float alpha = (p - rayStart.dot(trinorm)) / rayDir_dot_trinorm;
		if (alpha < 0.0f)
			return std::nullopt;
		Vector3 sex = rayStart + rayDir * alpha;

		Vector3 c = sex - p1;
		float d = v2.sqlen3() * v3.sqlen3() - v2.dot(v3) * v2.dot(v3);
		//assert(d != 0.0f);
		float a = (c.dot(v2) * v3.sqlen3() - c.dot(v3) * v2.dot(v3)) / d;
		float b = (c.dot(v3) * v2.sqlen3() - c.dot(v2) * v3.dot(v2)) / d;
		if (a >= 0.0f && b >= 0.0f && (a + b) <= 1.0f)
			return sex;
		else
			return std::nullopt;
	}

	bool isPointInAABB(const Vector3 &point, const Vector3 &highCorner, const Vector3 &lowCorner) {
		for (int i = 0; i < 3; i++)
			if (point.coord[i] < lowCorner.coord[i] || point.coord[i] > highCorner.coord[i])
				return false;
		return true;
	}

	std::optional<Vector3> getRayAABBIntersection(const Vector3 &rayStart, const Vector3 &_rayDir, const Vector3 &highCorner, const Vector3 &lowCorner) {
		if (isPointInAABB(rayStart, highCorner, lowCorner))
			return rayStart;
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
						return candidate;
				}
			}
		}
		return std::nullopt;
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
					beacon.params = (beacon.params & 0xFF00) | (uint8_t)std::round(angle * 128.0f / std::numbers::pi);
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
					beacon->params = (beacon->params & 0xFF00) | (uint8_t)(int)(angle * 128.0f / std::numbers::pi);
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
	std::string getInfo() override {
		return fmt::format("{} {}",
			(ground && ground->isSubclassOf<CDynamicGround>()) ? "Dynamic Ground" : "Ground",
			ground ? ui.kenv.getObjectName(ground.get()) : "removed");
	}
	void onDetails() override { onSelected(); ui.wndShowGrounds = true; }

	bool hasTransform() override { return ground && ground->editing; }
	Matrix getTransform() override { return ground->editing->transform; }
	void setTransform(const Matrix& mat) override { ground->setTransform(mat); }
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

struct X2SquadSelection : UISelection {
	static const int ID = 204;

	KWeakRef<CKGrpSquadX2> squad;

	X2SquadSelection(EditorInterface& ui, Vector3& hitpos, CKGrpSquadX2* squad) : UISelection(ui, hitpos), squad(squad) {}

	int getTypeID() override { return ID; }
	bool hasTransform() override { return squad.get() && ui.showingChoreography < squad->phases.size(); }
	Matrix getTransform() override {
		Matrix mmm = squad->phases[ui.showingChoreography].mat;
		mmm._14 = 0.0f;
		mmm._24 = 0.0f;
		mmm._34 = 0.0f;
		mmm._44 = 1.0f;
		return mmm;
	}
	void setTransform(const Matrix& mat) override { squad->phases[ui.showingChoreography].mat = mat; }
	void onSelected() override { ui.selectedX2Squad = squad; }
	std::string getInfo() override { return fmt::format("Squad {}", squad ? ui.kenv.getObjectName(squad.get()) : "Removed"); }
	void onDetails() override { onSelected(); ui.wndShowSquads = true; }
};

struct BaseChoreoSpotSelection : UISelection {
	static const int ID = 5;

	int spotIndex;

	BaseChoreoSpotSelection(EditorInterface& ui, Vector3& hitpos, int spotIndex)
		: UISelection(ui, hitpos), spotIndex(spotIndex) {}

	virtual CKGroup* squadGroup() = 0;
	virtual CKChoreoKey* choreoKey() = 0;
	virtual Matrix squadMatrix() = 0;

	Matrix getTransform() override {
		auto& spot = choreoKey()->slots[spotIndex];
		Matrix mRot = Matrix::getIdentity();
		Vector3 v1 = spot.direction.normal();
		Vector3 v3 = v1.cross(Vector3(0.0f, 1.0f, 0.0f));
		const Vector3& v4 = spot.position;
		std::tie(mRot._11, mRot._12, mRot._13) = std::tie(v1.x, v1.y, v1.z);
		std::tie(mRot._31, mRot._32, mRot._33) = std::tie(v3.x, v3.y, v3.z);
		std::tie(mRot._41, mRot._42, mRot._43) = std::tie(v4.x, v4.y, v4.z);
		return mRot * squadMatrix();
	}
	void setTransform(const Matrix &mat) override {
		Matrix inv = squadMatrix().getInverse4x4();
		Matrix spotMat = mat * inv;
		auto& spot = choreoKey()->slots[spotIndex];
		spot.position = spotMat.getTranslationVector();
		spot.direction = Vector3(spotMat._11, spotMat._12, spotMat._13);
	}

	void duplicate() override {
		if (!hasTransform()) return;
		auto& slots = choreoKey()->slots;
		slots.push_back(slots[spotIndex]);
	}
	bool remove() override {
		if (!hasTransform()) return false;
		auto& slots = choreoKey()->slots;
		slots.erase(slots.begin() + spotIndex);
		return true;
	}
	std::string getInfo() override {
		auto* squad = squadGroup();
		return fmt::format("Choreo spot {} from Squad {}", spotIndex, squad ? ui.kenv.getObjectName(squad) : "removed");
	}
	void onDetails() override { onSelected(); ui.wndShowSquads = true; }
};

struct X1ChoreoSpotSelection : BaseChoreoSpotSelection {
	static const int ID = 5;

	KWeakRef<CKGrpSquadEnemy> squad;

	X1ChoreoSpotSelection(EditorInterface& ui, Vector3& hitpos, CKGrpSquadEnemy* squad, int spotIndex)
		: BaseChoreoSpotSelection(ui, hitpos, spotIndex), squad(squad) {}

	CKGroup* squadGroup() override { return squad.get(); }
	CKChoreoKey* choreoKey() override { return squad->choreoKeys[ui.showingChoreoKey].get(); }
	Matrix squadMatrix() override { return squad->mat1; }

	int getTypeID() override { return ID; }
	bool hasTransform() override {
		if (!squad) return false;
		if (ui.showingChoreoKey < 0 || ui.showingChoreoKey >= (int)squad->choreoKeys.size()) return false;
		if (spotIndex < 0 || spotIndex >= (int)squad->choreoKeys[ui.showingChoreoKey]->slots.size()) return false;
		return true;
	}
	void onSelected() override { ui.selectedSquad = squad; }
};

struct X2ChoreoSpotSelection : BaseChoreoSpotSelection {
	static const int ID = 205;

	KWeakRef<CKGrpSquadX2> squad;

	X2ChoreoSpotSelection(EditorInterface& ui, Vector3& hitpos, CKGrpSquadX2* squad, int spotIndex)
		: BaseChoreoSpotSelection(ui, hitpos, spotIndex), squad(squad) {}

	CKGroup* squadGroup() override { return squad.get(); }
	CKChoreoKey* choreoKey() override { return squad->phases[ui.showingChoreography].choreography->keys[ui.showingChoreoKey].get(); }
	Matrix squadMatrix() override {
		Matrix mat = squad->phases[ui.showingChoreography].mat;
		mat._14 = mat._24 = mat._34 = 0.0f;
		mat._44 = 1.0f;
		return mat;
	}

	int getTypeID() override { return ID; }
	bool hasTransform() override {
		if (!squad) return false;
		if (ui.showingChoreography < 0 || ui.showingChoreography >= (int)squad->phases.size()) return false;
		auto& phase = squad->phases[ui.showingChoreography];
		if (ui.showingChoreoKey < 0 || ui.showingChoreoKey >= (int)phase.choreography->keys.size()) return false;
		if (spotIndex < 0 || spotIndex >= (int)phase.choreography->keys[ui.showingChoreoKey]->slots.size()) return false;
		return true;
	}
	void onSelected() override { ui.selectedX2Squad = squad; }
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
		marker->orientation1 = (uint8_t)std::round(angle * 128.0f / std::numbers::pi);
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
		Image img = Image::loadFromMemory(ptr, siz);
		tbTexture = gfx->createTexture(img);
		std::tie(ptr, siz) = GetResourceContent("HelpMarker.png");
		img = Image::loadFromMemory(ptr, siz);
		helpTexture = gfx->createTexture(img);
		tbIconsLoaded = true;
	}

	ImGui::BeginMainMenuBar();
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
	ImGui::TextLinkOpenURL(needhelp, "https://github.com/AdrienTD/XXL-Editor/wiki");
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
		igwindow("Clones", &wndShowClones, [](EditorInterface *ui) { IGCloneEditor(*ui); });
	if (kenv.hasClass<CSGSectorRoot>())
		igwindow("Scene graph", &wndShowSceneGraph, [](EditorInterface* ui) { IGSceneNodeEditor(*ui); });
	if (kenv.hasClass<CKBeaconKluster>())
		igwindow("Beacons", &wndShowBeacons, [](EditorInterface *ui) { IGBeaconEditor(*ui); });
	if (kenv.hasClass<CKMeshKluster>())
		igwindow("Grounds", &wndShowGrounds, [](EditorInterface *ui) { IGGroundEditor(*ui); });
	if (kenv.hasClass<CKSrvEvent>())
		igwindow("Events", &wndShowEvents, [](EditorInterface *ui) { IGEventEditor(*ui); });
	if (kenv.hasClass<CKSrvTrigger>())
		igwindow("Triggers", &wndShowTriggers, [](EditorInterface *ui) { IGTriggerEditor(*ui); });
	if (kenv.hasClass<CKSoundDictionary>())
		igwindow("Sounds", &wndShowSounds, [](EditorInterface *ui) { IGSoundEditor(*ui); });
	if (kenv.hasClass<CKGrpEnemy>() || (kenv.version >= kenv.KVERSION_XXL2 && kenv.hasClass<CKGrpSquadX2>()))
		igwindow("Squads", &wndShowSquads, [](EditorInterface* ui) { if (ui->kenv.version >= KEnvironment::KVERSION_XXL2) IGSquadEditorXXL2Plus(*ui); else IGSquadEditorXXL1(*ui); });
	if (kenv.hasClass<CKGroupRoot>())
		igwindow("Hooks", &wndShowHooks, [](EditorInterface *ui) { IGHookEditor(*ui); });
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

		for (int sectorIndex = 0; sectorIndex <= kenv.numSectors; ++sectorIndex) {
			if (sectorIndex == 0 || showingSector == 0 || sectorIndex == showingSector) {
				auto& objlist = (sectorIndex == 0) ? kenv.levelObjects : kenv.sectorObjects[sectorIndex - 1];
				for (const auto& gnd : objlist.getClassType<CGround>().objects) {
					drawGround(gnd->cast<CGround>());
				}
				if (showDynamicGrounds) {
					for (const auto& gnd : objlist.getClassType<CDynamicGround>().objects) {
						drawGround(gnd->cast<CDynamicGround>());
					}
				}
			}
		}

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
			for (int sectorIndex = 0; sectorIndex <= kenv.numSectors; ++sectorIndex) {
				if (sectorIndex == 0 || showingSector == 0 || sectorIndex == showingSector) {
					for (const auto& wall : kenv.levelObjects.getFirst<CKLevel>()->sectors[sectorIndex]->meshKluster->walls) {
						drawWall(wall.get());
					}
				}
			}
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

	if (kenv.version == KEnvironment::KVERSION_XXL1 && grpEnemy && (showSquadBoxes || showMsgActionBoxes)) {
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
	
	if (kenv.version >= KEnvironment::KVERSION_XXL2 && showSquadBoxes) {
		if (auto* grpEnemy = getX2PlusEnemyGroup()) {
			for (int str = 0; str <= kenv.numSectors; ++str) {
				if (str == showingSector || str == 0 || showingSector == 0) {
					for (CKGroup* fightZoneBp = grpEnemy->fightZoneGroups[str]->childGroup.get(); fightZoneBp; fightZoneBp = fightZoneBp->nextGroup.get()) {
						auto* fightZone = fightZoneBp->cast<GameX2::CKGrpFightZone>();
						drawBox(fightZone->zonePos1 - fightZone->zoneSize1 * 0.5f, fightZone->zonePos1 + fightZone->zoneSize1 * 0.5f, 0xFFFF4080);
						drawBox(fightZone->zonePos2 - fightZone->zoneSize2 * 0.5f, fightZone->zonePos2 + fightZone->zoneSize2 * 0.5f, 0xFF80C0FF);
					}
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
							if (poolindex < fightData.pools.size() && fightData.pools[poolindex].pool) {
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
				const auto& lowBBCorner = pfnode->boundingBox.lowCorner;
				const auto& highBBCorner = pfnode->boundingBox.highCorner;
				drawBox(lowBBCorner, highBBCorner);

				float h = highBBCorner.y - lowBBCorner.y;
				float cw = pfnode->getCellWidth();
				float ch = pfnode->getCellHeight();
				for (float y : {h}) {
					for (int z = 1; z < pfnode->numCellsZ; z++) {
						gfx->drawLine3D(lowBBCorner + Vector3(0, y, z*ch), lowBBCorner + Vector3(pfnode->numCellsX*cw, y, z*ch), 0xFF00FFFF);
					}
					for (int x = 1; x < pfnode->numCellsX; x++) {
						gfx->drawLine3D(lowBBCorner + Vector3(x*cw, y, 0), lowBBCorner + Vector3(x*cw, y, pfnode->numCellsZ*ch), 0xFF00FFFF);
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
							gfx->drawLine3D(lowBBCorner + Vector3(fx, h, fz)*cellsize, lowBBCorner + Vector3(fx + 1, h, fz + 1)*cellsize, ddcolor);
							gfx->drawLine3D(lowBBCorner + Vector3(fx + 1, h, fz)*cellsize, lowBBCorner + Vector3(fx, h, fz + 1)*cellsize, ddcolor);
						}
					}
				}

				for (auto &pftrans : pfnode->transitions) {
					for (auto &door : pftrans->doors) {
						drawBox(door.sourceBox.lowCorner, door.sourceBox.highCorner, 0xFF00FF00);
						drawBox(door.destinationBox.lowCorner, door.destinationBox.highCorner, 0xFF00FFFF);
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

GameX2::IKGrpEnemy* EditorInterface::getX2PlusEnemyGroup()
{
	CKObject* group = nullptr;
	if (kenv.version == KEnvironment::KVERSION_XXL2) {
		group = kenv.levelObjects.getFirst<GameX2::CKGrpA2Enemy>();
	}
	else if (kenv.version == KEnvironment::KVERSION_ARTHUR) {
		auto& cltype = kenv.levelObjects.getClassType(4, 94);
		group = cltype.objects.empty() ? nullptr : cltype.objects[0];
	}
	else if (kenv.version == KEnvironment::KVERSION_OLYMPIC) {
		group = kenv.levelObjects.getFirst<GameOG::CKGrpA3Enemy>();
	}
	else if (kenv.version == KEnvironment::KVERSION_SPYRO) {
		auto& cltype = kenv.levelObjects.getClassType(4, 41);
		group = cltype.objects.empty() ? nullptr : cltype.objects[0];
	}
	else if (kenv.version == KEnvironment::KVERSION_ALICE) {
		auto& cltype = kenv.levelObjects.getClassType(4, 57);
		group = cltype.objects.empty() ? nullptr : cltype.objects[0];
	}
	return group ? group->dyncast<GameX2::IKGrpEnemy>() : nullptr;
}

void EditorInterface::IGMain()
{
	static uint32_t lastMessageTime;
	static std::string lastMessage;

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

			lastMessage = "Loaded!";
			lastMessageTime = SDL_GetTicks();
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
		if (doit) {
			kenv.saveLevel(levelNum);

			lastMessage = "Saved!";
			lastMessageTime = SDL_GetTicks();
		}
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
	if (!lastMessage.empty()) {
		ImGui::SameLine();
		const uint32_t timeout = 2000u;
		const uint32_t curTime = SDL_GetTicks();
		const float intensity = std::clamp(4.0f - 4.0f * (float)(curTime - lastMessageTime) / (float)timeout, 0.0f, 1.0f);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, intensity), "%s", lastMessage.c_str());
		if (curTime - lastMessageTime > timeout)
			lastMessage.clear();
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
	ImGui::Checkbox("Gnd bounds", &showGroundBounds); ImGui::SameLine();
	ImGui::Checkbox("Dynamic Gnd", &showDynamicGrounds); ImGui::SameLine();
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

void EditorInterface::IGLocaleEditor()
{
	if (!g_localeEditor)
		g_localeEditor = std::make_unique<LocaleEditor>(kenv, gfx, g_window);
	g_localeEditor->gui();
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
					if (ixres.has_value()) {
						CKSceneNode *tosel = node;
						if (!tosel->isSubclassOf<CSGSectorRoot>()) {
							while (!tosel->parent->isSubclassOf<CSGSectorRoot>() && !tosel->parent->isSubclassOf<CZoneNode>())
								tosel = tosel->parent.get();
							rayHits.push_back(std::make_unique<NodeSelection>(*this, *ixres, tosel));
						}
						else { // non-editable selection for sector root to still let the user to set cursor position there
							rayHits.push_back(std::make_unique<UISelection>(*this, *ixres));
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
							std::optional<Vector3> rsi;
							if (bing.handler->isSubclassOf<CKCrateCpnt>()) {
								Vector3 lc = pos - Vector3(0.5f, 0.5f, 0.5f);
								Vector3 hc = pos + Vector3(0.5f, (float)(beacon.params & 7) - 0.5f, 0.5f);
								rsi = getRayAABBIntersection(camera.position, rayDir, hc, lc);
							}
							else {
								rsi = getRaySphereIntersection(camera.position, rayDir, pos, 0.5f);
							}
							if (rsi.has_value()) {
								rayHits.push_back(std::make_unique<BeaconSelection>(*this, *rsi, bing.sectorIndex, bing.klusterIndex, nbing, nbeac));
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
					if (!showDynamicGrounds && ground->isSubclassOf<CDynamicGround>())
						continue;
					std::optional<Matrix> transform;
					if (auto* dynGround = ground->dyncast<CDynamicGround>()) {
						transform = dynGround->getTransform();
					}
					auto rbi = getRayAABBIntersection(camera.position, rayDir, ground->aabb.highCorner, ground->aabb.lowCorner);
					if (rbi.has_value() || transform) {
						for (auto &tri : ground->triangles) {
							Vector3 v0 = ground->vertices[tri.indices[0]];
							Vector3 v1 = ground->vertices[tri.indices[1]];
							Vector3 v2 = ground->vertices[tri.indices[2]];
							if (transform) {
								v0 = v0.transform(*transform);
								v1 = v1.transform(*transform);
								v2 = v2.transform(*transform);
							}
							auto rti = getRayTriangleIntersection(camera.position, rayDir, v0, v2, v1);
							if(rti.has_value())
								rayHits.push_back(std::make_unique<GroundSelection>(*this, *rti, ground.get()));
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
							if (ixres.has_value()) {
								rayHits.push_back(std::make_unique<SquadSelection>(*this, *ixres, squad));
								break;
							}
						}
					}
					if (showingChoreoKey >= 0 && showingChoreoKey < (int)squad->choreoKeys.size()) {
						int spotIndex = 0;
						for (auto &slot : squad->choreoKeys[showingChoreoKey]->slots) {
							Vector3 trpos = slot.position.transform(squad->mat1);
							auto rbi = getRayAABBIntersection(camera.position, rayDir, trpos + Vector3(1, 2, 1), trpos - Vector3(1, 0, 1));
							if (rbi.has_value()) {
								rayHits.push_back(std::make_unique<X1ChoreoSpotSelection>(*this, *rbi, squad, spotIndex));
							}
							spotIndex++;
						}
					}
				}
			}
		}
	}

	// Squads XXL2+
	if (showSquadChoreos) {
		if (auto* grpEnemy = getX2PlusEnemyGroup()) {
			for (int sectorIndex = 0; sectorIndex <= kenv.numSectors; ++sectorIndex) {
				if (sectorIndex == 0 || showingSector == 0 || sectorIndex == showingSector) {
					auto* fightZoneRoot = grpEnemy->fightZoneGroups.at(sectorIndex).get();
					for (CKGroup* grp1 = fightZoneRoot->childGroup.get(); grp1; grp1 = grp1->nextGroup.get()) {
						if (auto* fightZone = grp1->dyncast<GameX2::CKGrpFightZone>()) {
							for (CKGroup* grp2 = fightZone->childGroup.get(); grp2; grp2 = grp2->nextGroup.get()) {
								if (auto* squad = grp2->dyncast<CKGrpSquadX2>()) {
									if (showingChoreography < squad->phases.size()) {
										auto& phase = squad->phases[showingChoreography];
										Vector3 squadPosition = phase.mat.getTranslationVector();
										RwGeometry* rwgeo = swordModel->geoList.geometries[0].get();
										if (rayIntersectsSphere(camera.position, rayDir, rwgeo->spherePos.transform(phase.mat), rwgeo->sphereRadius)) {
											for (auto& tri : rwgeo->tris) {
												std::array<Vector3, 3> trverts;
												for (int i = 0; i < 3; i++)
													trverts[i] = rwgeo->verts[tri.indices[i]].transform(phase.mat);
												auto ixres = getRayTriangleIntersection(camera.position, rayDir, trverts[0], trverts[1], trverts[2]);
												if (ixres.has_value()) {
													rayHits.push_back(std::make_unique<X2SquadSelection>(*this, *ixres, squad));
													break;
												}
											}
										}
										if (showingChoreoKey >= 0 && showingChoreoKey < (int)phase.choreography->keys.size()) {
											int spotIndex = 0;
											for (auto& slot : phase.choreography->keys[showingChoreoKey]->slots) {
												Vector3 trpos = slot.position.transform(phase.mat);
												auto rbi = getRayAABBIntersection(camera.position, rayDir, trpos + Vector3(1, 2, 1), trpos - Vector3(1, 0, 1));
												if (rbi.has_value()) {
													rayHits.push_back(std::make_unique<X2ChoreoSpotSelection>(*this, *rbi, squad, spotIndex));
												}
												spotIndex++;
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
	}

	// Markers
	if (showMarkers && kenv.hasClass<CKSrvMarker>()) {
		if (CKSrvMarker *srvMarker = kenv.levelObjects.getFirst<CKSrvMarker>()) {
			for (auto &list : srvMarker->lists) {
				int markerIndex = 0;
				for (auto &marker : list) {
					auto rsi = getRaySphereIntersection(camera.position, rayDir, marker.position, 0.5f);
					if (rsi.has_value()) {
						rayHits.push_back(std::make_unique<MarkerSelection>(*this, *rsi, markerIndex));
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
				if (rsi.has_value()) {
					rayHits.push_back(std::make_unique<HkLightSelection>(*this, *rsi, grpLight, lightIndex));
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
				if (rsi.has_value()) {
					rayHits.push_back(std::make_unique<X1DetectorSelection>(*this, *rsi, X1DetectorSelection::BOUNDINGBOX, i));
				}
				++i;
			}
			i = 0;
			for (auto& sph : srvDetector->spheres) {
				auto rsi = getRaySphereIntersection(camera.position, rayDir, sph.center, 0.5f);
				if (rsi.has_value()) {
					rayHits.push_back(std::make_unique<X1DetectorSelection>(*this, *rsi, X1DetectorSelection::SPHERE, i));
				}
				++i;
			}
			i = 0;
			for (auto& rect : srvDetector->rectangles) {
				auto rsi = getRaySphereIntersection(camera.position, rayDir, rect.center, 0.5f);
				if (rsi.has_value()) {
					rayHits.push_back(std::make_unique<X1DetectorSelection>(*this, *rsi, X1DetectorSelection::RECTANGLE, i));
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
				if (rsi.has_value()) {
					rayHits.push_back(std::make_unique<X2DetectorSelection>(*this, *rsi, geo, detector.get()));
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
