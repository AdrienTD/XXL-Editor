#include "IGGroundEditor.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"
#include "GuiUtils.h"
#include "PropFlagsEditor.h"
#include "GeoUtils.h"

#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"

#include "rw.h"

#include <imgui/imgui.h>

namespace
{
	void ImportGroundOBJ(KEnvironment& kenv, const std::filesystem::path& filename, int sector) {
		KObjectList& objlist = (sector == -1) ? kenv.levelObjects : kenv.sectorObjects[sector];
		CKMeshKluster* kluster = objlist.getFirst<CKMeshKluster>();
		CKSector* ksector = kenv.levelObjects.getClassType<CKSector>().objects[sector + 1]->cast<CKSector>();

		std::map<std::string, CGround*> groundMap;
		for (auto& gnd : kluster->grounds) {
			if (!gnd->isSubclassOf<CDynamicGround>()) {
				groundMap[kenv.getObjectName(gnd.get())] = gnd.get();
			}
		}

		FILE* wobj;
		GuiUtils::fsfopen_s(&wobj, filename, "rt");
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
				for (auto& tri : triangles) {
					CGround::Triangle cvtri;
					for (int c = 0; c < 3; c++) {
						int objIndex = tri.indices[c];
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
				for (Vector3& vec : gnd->vertices) {
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
				for (float& coord : vec) {
					coord = std::strtof(strtok_s(NULL, spaces, &context), nullptr);
				}
				positions.push_back(vec);
			}
			else if (word == "f") {
				std::vector<uint16_t> face;
				while (char* arg = strtok_s(NULL, spaces, &context)) {
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
}

void EditorUI::IGGroundEditor(EditorInterface& ui)
{
	using namespace GuiUtils;

	static bool hideDynamicGrounds = true;

	auto& kenv = ui.kenv;
	ImGui::Checkbox("Hide dynamic", &hideDynamicGrounds);
	ImGui::SameLine();
	if (ImGui::Button("Autoname")) {
		auto gnstr = [&ui, &kenv](KObjectList& objlist, int strnum) {
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
			GeoUtils::ChangeNodeGeometry(kenv, sgsr, &rwgeo, 1);
			if (kenv.version >= kenv.KVERSION_XXL2) {
				for (auto* geo = sgsr->geometry.get(); geo; geo = geo->nextGeo.get()) {
					geo->flags = 5;
					geo->flags2 = 6;
					geo->material->flags = 0;
				}
			}
		}
		ui.progeocache.clear();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Replace all rendering sector root geometries by ground models,\nso that you can see the ground collision ingame.\n/!\\ You will lose all your current sector geometries!");
	ImGui::Columns(2);
	ImGui::BeginChild("GroundTree");
	auto feobjlist = [&ui, &kenv](KObjectList& objlist, const char* desc, int sectorNumber) {
		if (CKMeshKluster* mkluster = objlist.getFirst<CKMeshKluster>()) {
			ImGui::PushID(mkluster);
			bool tropen = ImGui::TreeNode(mkluster, "%s", desc);
			ImGui::SameLine();
			if (ImGui::SmallButton("Import")) {
				auto filepath = OpenDialogBox(ui.g_window, "Wavefront OBJ file\0*.OBJ\0\0", "obj");
				if (!filepath.empty()) {
					ImportGroundOBJ(kenv, filepath, sectorNumber);
					ui.gndmdlcache.clear();
				}
			}
			//if (ImGui::IsItemHovered())
			//	ImGui::SetTooltip("No walls yet! Corruption risk!");
			ImGui::SameLine();
			if (ImGui::SmallButton("Export")) {
				auto filepath = SaveDialogBox(ui.g_window, "Wavefront OBJ file\0*.OBJ\0\0", "obj");
				if (!filepath.empty()) {
					FILE* obj;
					fsfopen_s(&obj, filepath, "wt");
					uint16_t gndx = 0;
					uint32_t basevtx = 1;
					for (const auto& gnd : mkluster->grounds) {
						if (gnd->isSubclassOf<CDynamicGround>() && hideDynamicGrounds)
							continue;
						fprintf(obj, "o %s\n", kenv.getObjectName(gnd.get()));
						for (auto& vtx : gnd->vertices) {
							fprintf(obj, "v %f %f %f\n", vtx.x, vtx.y, vtx.z);
						}
						for (auto& fac : gnd->triangles) {
							fprintf(obj, "f %u %u %u\n", basevtx + fac.indices[0], basevtx + fac.indices[1], basevtx + fac.indices[2]);
						}
						uint32_t wallvtx = basevtx + gnd->vertices.size();
						for (auto& wall : gnd->finiteWalls) {
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
						for (auto& infwall : gnd->infiniteWalls) {
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
				for (auto& gnd : mkluster->grounds) {
					if (gnd->isSubclassOf<CDynamicGround>() && hideDynamicGrounds)
						continue;
					const char* type = "(G)";
					if (gnd->isSubclassOf<CDynamicGround>())
						type = "(D)";
					if (gnd->getRefCount() > 1)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
					bool p = ImGui::TreeNodeEx(gnd.get(), ImGuiTreeNodeFlags_Leaf | ((gnd.get() == ui.selGround.get()) ? ImGuiTreeNodeFlags_Selected : 0), "%s %s (%02u,%02u)", type, kenv.getObjectName(gnd.get()), gnd->param1, gnd->param2);
					IGObjectDragDropSource(ui, gnd.get());
					if (ImGui::IsItemClicked())
						ui.selGround = gnd.get();
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
	for (auto& str : kenv.sectorObjects) {
		char lol[64];
		sprintf_s(lol, "Sector %i", x + 1);
		feobjlist(str, lol, x);
		x++;
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("SelGroundInfo");
	if (ui.selGround) {
		if (ImGui::Button("Delete")) {
			ImGui::OpenPopup("GroundDelete");
		}
		IGObjectNameInput("Name", ui.selGround.get(), kenv);
		auto CheckboxFlags16 = [](const char* label, uint16_t* flags, unsigned int val) {
			unsigned int up = *flags;
			if (ImGui::CheckboxFlags(label, &up, val))
				*flags = up;
			};
		ImGui::InputScalar("Flags 1", ImGuiDataType_U16, &ui.selGround->param1, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("Flags 2", ImGuiDataType_U16, &ui.selGround->param2, nullptr, nullptr, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::InputScalar("Negative height", ImGuiDataType_Float, &ui.selGround->param3);
		ImGui::InputScalar("Parameter", ImGuiDataType_Float, &ui.selGround->param4);

		auto* ground = ui.selGround.get();
		ImGui::Separator();
		if (auto* dynGround = ground->dyncast<CDynamicGround>()) {
			IGObjectSelectorRef(ui, "Node", dynGround->node);
		}
		else {
			if (ground->editing) {
				bool makeFixed = false;
				if (ImGui::Button("Make fixed"))
					makeFixed = true;
				Vector3 position = ground->editing->transform.getTranslationVector();
				if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
					Matrix newTransfrom = ground->editing->transform;
					newTransfrom.setTranslation(position);
					ground->setTransform(newTransfrom);
				}
				if (makeFixed)
					ground->makeFixed();
			}
			else {
				if (ImGui::Button("Make movable")) {
					// Take vertex average / center of gravity as origin.
					// So origin taken is the same regardless of the orientation of the ground.
					Vector3 average(0.0f, 0.0f, 0.0f);
					for (const auto& vec : ground->vertices)
						average += vec;
					average /= ground->vertices.size();
					ground->makeMovable(Matrix::getTranslationMatrix(average));
				}
			}
		}

		if (auto* jsGround = ui.g_encyclo.getClassJson(CGround::FULL_ID)) {
			if (auto itProps = jsGround->find("properties"); itProps != jsGround->end()) {
				for (auto& [paramName, param] : { std::tie("param1", ui.selGround->param1), std::tie("param2", ui.selGround->param2) })
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
					KObjectList& objlist = (i == -1) ? kenv.levelObjects : kenv.sectorObjects[i];
					if (CKMeshKluster* mkluster = objlist.getFirst<CKMeshKluster>()) {
						auto it = std::find_if(mkluster->grounds.begin(), mkluster->grounds.end(),
							[&ui](const kobjref<CGround>& ref) {return ref.get() == ui.selGround.get(); }
						);
						if (it != mkluster->grounds.end())
							mkluster->grounds.erase(it);
					}
				}
				if (ui.selGround->getRefCount() == 0) {
					kenv.removeObject(ui.selGround.get());
					ui.selGround = nullptr;
					ui.rayHits.clear();
					ui.nearestRayHit = nullptr;
				}
			}
			ImGui::EndPopup();
		}
	}
	ImGui::EndChild();
	ImGui::Columns();

}
