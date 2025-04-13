#include "IGAnimationViewer.h"

#include "EditorInterface.h"
#include "EditorWidgets.h"

#include "KEnvironment.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKGraphical.h"

#include "rw.h"
#include "rwext.h"

#include <imgui/imgui.h>

#include <SDL2/SDL_timer.h> // TODO Put time as data member in EditorInterface, and remove SDL depend.

namespace
{
	using namespace EditorUI;

	struct AnimViewerModelInfo {
		const RwAnimAnimation* rwAnim = nullptr;
		const RwExtHAnim* hAnim = nullptr;
		std::vector<const RwGeometry*> rwGeos;
		int nodeNumBones = 0;
		int sector = -1;
	};

	std::optional<AnimViewerModelInfo> getAnimViewerModelInfo(EditorInterface& ui)
	{
		if (!ui.selectedAnimatedNode.get())
			return {};
		CAnimationManager* animMgr = ui.kenv.levelObjects.getFirst<CAnimationManager>();
		if (ui.selectedAnimationSector >= 0 && ui.selectedAnimationSector <= ui.kenv.numSectors) {
			const CSectorAnimation* sectorAnims =
				(ui.kenv.version >= KEnvironment::KVERSION_ARTHUR)
				? animMgr->arSectors[ui.selectedAnimationSector].get()
				: &animMgr->commonAnims;

			if (ui.selectedAnimationIndex >= 0 && ui.selectedAnimationIndex < sectorAnims->anims.size()) {
				AnimViewerModelInfo info;
				info.rwAnim = &sectorAnims->anims.at(ui.selectedAnimationIndex).rwAnim;

				if (CAnimatedNode* anmNode = ui.selectedAnimatedNode->dyncast<CAnimatedNode>()) {
					info.hAnim = (const RwExtHAnim*)anmNode->frameList->extensions[0].find(0x11E);
					info.rwGeos.reserve(8);
					for (CKAnyGeometry* kgeo = anmNode->geometry.get(); kgeo; kgeo = kgeo->nextGeo.get()) {
						CKAnyGeometry* actualkgeo = kgeo->duplicateGeo ? kgeo->duplicateGeo.get() : kgeo;
						info.rwGeos.push_back(actualkgeo->clump->atomic.geometry.get());
					}
					info.nodeNumBones = anmNode->numBones;
					info.sector = ui.kenv.getObjectSector(anmNode);
					return info;
				}
				else if (CAnimatedClone* anmClone = ui.selectedAnimatedNode->dyncast<CAnimatedClone>()) {
					CCloneManager* cloneMgr = ui.kenv.levelObjects.getFirst<CCloneManager>();
					int teamIndex = anmClone->cloneInfo;
					int clindex = ui.nodeCloneIndexMap.at(anmClone);
					const auto& dong = cloneMgr->_team.dongs[clindex];

					info.hAnim = (const RwExtHAnim*)dong.clump.frameList.extensions[1].find(0x11E);
					info.rwGeos.reserve(8);
					for (uint32_t part : dong.bongs) {
						if (part != 0xFFFFFFFF) {
							info.rwGeos.push_back(cloneMgr->_teamDict._bings[part]._clump.atomic.geometry.get());
						}
					}
					return info;
				}
			}
		}

		return {};
	}
}

void EditorUI::IGAnimationViewer(EditorInterface& ui)
{
	auto& kenv = ui.kenv;
	kobjref<CAnyAnimatedNode> ref = ui.selectedAnimatedNode.get();
	IGObjectSelectorRef(ui, "Node object", ref);
	ui.selectedAnimatedNode = ref.get();
	if (kenv.version >= KEnvironment::KVERSION_ARTHUR) {
		ImGui::InputInt("Anim sector", &ui.selectedAnimationSector);
	}
	ImGui::InputInt("Anim index", &ui.selectedAnimationIndex);
	ImGui::DragFloat3("Position", &ui.selectedAnimRenderPos.x);
	if (ImGui::Button("Move to camera")) {
		ui.selectedAnimRenderPos = ui.camera.position + ui.camera.direction * 5.0f;
	}
	ImGui::SameLine();
	if (ImGui::Button("Move to cursor")) {
		ui.selectedAnimRenderPos = ui.cursorPosition;
	}
	ImGui::Checkbox("Show stickman", &ui.showStickman);
	ImGui::Separator();

	if (auto animViewerInfo = getAnimViewerModelInfo(ui)) {
		auto& rwanim = *animViewerInfo->rwAnim;
		auto* hanim = animViewerInfo->hAnim;
		int rwGuessedNodes = rwanim.guessNumNodes();
		if (rwGuessedNodes != hanim->bones.size()) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid!\nNumber of bones do not match!");
		}
		ImGui::Text("Node   Num bones: %u", animViewerInfo->nodeNumBones);
		ImGui::Text("HAnim  Num bones: %u", hanim->bones.size());
		ImGui::Text("RwAnim Num bones: %i", rwGuessedNodes);
		ImGui::Text("HAnim  Frame size: %u", hanim->keyFrameSize);
		ImGui::Text("RwAnim Num frames: %zu", rwanim.numFrames());
		ImGui::Text("RwAnim Duration: %.4f sec", rwanim.duration);
		int geoIndex = 0;
		for (const RwGeometry* rwgeo : animViewerInfo->rwGeos) {
			auto* skin = (RwExtSkin*)rwgeo->extensions.find(0x116);
			if (skin) {
				ImGui::Text("Geo %i Skin Num bones: %u", geoIndex, skin->numBones);
				ImGui::Text("Geo %i Skin Num used bones: %u", geoIndex, skin->numUsedBones);
				ImGui::Text("Geo %i Skin Max weights per vertex: %u", geoIndex, skin->maxWeightPerVertex);
			}
			geoIndex += 1;
		}
	}
}

void EditorUI::RenderAnimation(EditorInterface& ui)
{
	auto* gfx = ui.gfx;
	if (auto animViewerInfo = getAnimViewerModelInfo(ui)) {
		gfx->unbindTexture(0);
		gfx->setTransformMatrix(Matrix::getTranslationMatrix(ui.selectedAnimRenderPos) * ui.camera.sceneMatrix);
		gfx->setBlendColor(0xFFFFFFFF); // white

		const int nodeSector = animViewerInfo->sector;
		ProTexDict* texDict = (nodeSector >= 0) ? &ui.str_protexdicts[nodeSector] : &ui.protexdict;

		const auto& rwanim = *animViewerInfo->rwAnim;
		const auto* hanim = animViewerInfo->hAnim;

		if (rwanim.guessNumNodes() != hanim->bones.size()) {
			gfx->setBlendColor(0xFF0000FF); // red
			ProGeometry* progeoSphere = ui.progeocache.getPro(ui.sphereModel->geoList.geometries[0], &ui.protexdict);
			progeoSphere->draw(false);
		}
		else {
			float time = (float)(SDL_GetTicks() % (int)(rwanim.duration * 1000.0f)) / 1000.0f;
			auto boneMatrices = rwanim.interpolateNodeTransforms(hanim->bones.size(), time);
			std::vector<Matrix> globalBoneMatrices(hanim->bones.size());

			std::stack<Matrix> matrixStack;
			matrixStack.push(Matrix::getIdentity());
			for (int b = 0; b < hanim->bones.size(); ++b) {
				auto& bone = hanim->bones[b];
				if (bone.flags & 2) {
					matrixStack.push(matrixStack.top());
				}

				auto prevMatrixPos = matrixStack.top().getTranslationVector();
				matrixStack.top() = boneMatrices[b] * matrixStack.top();
				globalBoneMatrices[b] = matrixStack.top();

				if (ui.showStickman) {
					gfx->drawLine3D(matrixStack.top().getTranslationVector(), prevMatrixPos);
				}

				if (bone.flags & 1) {
					matrixStack.pop();
				}
			}
			assert(matrixStack.empty());

			if (!ui.showStickman) {
				for (const RwGeometry* rwgeo : animViewerInfo->rwGeos) {
					RwGeometry tfGeo = *rwgeo;
					const RwExtSkin* skin = (RwExtSkin*)tfGeo.extensions.find(0x116);
					const int numWeights = std::clamp((int)skin->maxWeightPerVertex, 1, 4);
					for (int vtx = 0; vtx < tfGeo.verts.size(); ++vtx) {
						Vector3 tfVertex = { 0.0f,0.0f,0.0f };
						for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
							const int boneIndex = skin->vertexIndices[vtx][weightIndex];
							const float weight = skin->vertexWeights[vtx][weightIndex];
							tfVertex += tfGeo.verts[vtx].transform(skin->matrices[boneIndex]).transform(globalBoneMatrices[boneIndex]) * weight;
						}
						tfGeo.verts[vtx] = tfVertex;
					}
					ProGeometry progeo(gfx, &tfGeo, texDict);
					progeo.draw(true);
				}
			}
		}
	}
}
