#include "NodeCollisionUtils.h"

#include "File.h"
#include "KEnvironment.h"
#include "CKUtils.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKHook.h"
#include "CoreClasses/CKGroup.h"
#include "CoreClasses/CKComponent.h"
#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKLogic.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace {
	nlohmann::ordered_json g_jsCollisionInfo;

	struct ShapeMemberListener : NamedMemberListener {
		CKObject* owner;
		std::string_view memberName;
		std::vector<std::pair<CKObject*, CKBoundingShape*>> shapes;

		void reflect(uint8_t& ref, const char* name) override {}
		void reflect(uint16_t& ref, const char* name) override {}
		void reflect(uint32_t& ref, const char* name) override {}
		void reflect(float& ref, const char* name) override {}
		void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override {
			if (!ref)
				return;
			CKBoundingShape* shape = ref.get()->dyncast<CKBoundingShape>();
			if (!shape)
				return;
			if (getFullName(name) == memberName)
				shapes.push_back({ owner, shape });
		}
		void reflect(Vector3& ref, const char* name) override {}
		void reflect(EventNode& ref, const char* name, CKObject* user) override {}
		void reflect(MarkerIndex& ref, const char* name) override {}
		void reflect(std::string& ref, const char* name) override {}
	};

	std::vector<std::pair<CKObject*, CKBoundingShape*>> getAllShapesFromMember(KEnvironment& kenv, int ownerFullId, std::string_view memberName)
	{
		if (ownerFullId == CKCrateCpnt::FULL_ID && memberName == "!crateCloneOBB") {
			std::vector<std::pair<CKObject*, CKBoundingShape*>> result;
			for (CKObject* obj : kenv.levelObjects.getClassType<CKCrateCpnt>().objects) {
				CKCrateCpnt* cpnt = (CKCrateCpnt*)obj;
				for (CKSceneNode* crateNode = cpnt->crateNode.get(); crateNode; crateNode = crateNode->next.get()) {
					CSGBranch* crateBranch = crateNode->cast<CSGBranch>();
					CKBoundingShape* crateVolume = crateBranch->child->cast<CKBoundingShape>();
					result.push_back({ cpnt, crateVolume });
				}
			}
			return result;
		}

		ShapeMemberListener sml;
		sml.memberName = memberName;

		auto reflectAs = [&]<typename T>() {
			auto& cltype = kenv.levelObjects.getClassType(ownerFullId);
			if (cltype.objects.empty() || !cltype.objects[0]->isSubclassOf<T>())
				return;
			for (CKObject* obj : cltype.objects) {
				sml.owner = obj;
				T* tobj = obj->cast<T>();
				tobj->virtualReflectMembers(sml, &kenv);
			}
		};
		reflectAs.template operator() < CKReflectableService > ();
		reflectAs.template operator() < CKHook > ();
		reflectAs.template operator() < CKGroup > ();
		reflectAs.template operator() < CKComponent > ();
		reflectAs.template operator() < CKReflectableLogic > ();

		return std::move(sml.shapes);
	}

	CKBoundingShape* getShapeFromMember(KEnvironment& kenv, CKObject* owner, std::string_view memberName, CKBoundingShape* parentShape = nullptr)
	{
		if (owner->getClassFullID() == CKCrateCpnt::FULL_ID && memberName == "!crateCloneOBB") {
			return parentShape;
		}

		ShapeMemberListener sml;
		sml.owner = owner;
		sml.memberName = memberName;

		auto reflectAs = [&]<typename T>() {
			if (T* tobj = owner->dyncast<T>()) {
				tobj->virtualReflectMembers(sml, &kenv);
			}
		};
		reflectAs.template operator() < CKReflectableService > ();
		reflectAs.template operator() < CKHook > ();
		reflectAs.template operator() < CKGroup > ();
		reflectAs.template operator() < CKComponent > ();
		reflectAs.template operator() < CKReflectableLogic > ();

		assert(sml.shapes.size() < 2);
		return sml.shapes.empty() ? nullptr : sml.shapes[0].second;
	}
}

void NodeCollisionUtils::LoadNodeCollisionInfo(int gameVersion)
{
	if (!g_jsCollisionInfo.empty())
		return;
	const auto fileName = fmt::format("NodeCollisionTestInfo_{}.json", gameVersion);
	auto [fileData, fileSize] = GetResourceContent(fileName.c_str());
	g_jsCollisionInfo = nlohmann::json::parse(std::string_view((const char*)fileData, fileSize));
}

void NodeCollisionUtils::CreateCollisionsForObject(KEnvironment& kenv, CKObject* owner)
{
	LoadNodeCollisionInfo(kenv.version);

	CKSrvCollision* srvCollision = kenv.levelObjects.getFirst<CKSrvCollision>();

	auto jsTestList = g_jsCollisionInfo.at("collisionTests");
	const int classFullID = owner->getClassFullID();
	for (const auto& jsTest : jsTestList) {
		for (const std::string_view objkey : { "obj1", "obj2" }) {
			if (jsTest.at(objkey).at(0) == classFullID) {
				const std::string_view otherObjkey = objkey == "obj1" ? "obj2" : "obj1";
				const int otherClassFullID = jsTest.at(otherObjkey).at(0);

				auto* objShape = getShapeFromMember(kenv, owner, jsTest.at(objkey).at(1));
				if (!objShape) {
					break;
				}
				auto otherShapes = getAllShapesFromMember(kenv, otherClassFullID, jsTest.at(otherObjkey).at(1));

				for (auto [otherOwner, otherShape] : otherShapes) {
					auto testIndex = srvCollision->addCollision(owner, objShape, otherOwner, otherShape);

					if (jsTest.contains("children")) {
						for (const auto& jsChild : jsTest.at("children")) {
							assert(jsChild.at(objkey).at(0) == classFullID);
							assert(jsChild.at(otherObjkey).at(0) == otherClassFullID);

							auto* childShape = getShapeFromMember(kenv, owner, jsChild.at(objkey).at(1), objShape);
							auto* otherChildShape = getShapeFromMember(kenv, otherOwner, jsChild.at(otherObjkey).at(1), otherShape);
							assert(childShape && otherChildShape);

							auto childIndex = srvCollision->addCollision(owner, childShape, otherOwner, otherChildShape);
							srvCollision->setParent(childIndex, testIndex);
						}
					}
				}

				break;
			}
		}
	}
}
