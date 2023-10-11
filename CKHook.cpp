#include "CKHook.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"
#include "CKGroup.h"
#include "CKDictionary.h"
#include "CKService.h"
#include "CKLogic.h"
#include "CKComponent.h"
#include "CKGraphical.h"
#include "CKGameX1.h"

void CKHook::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.setNextFlags(MemberListener::MemberFlags::MF_HOOK_INTERNAL);
	if (kenv->version < kenv->KVERSION_XXL2) {
		r.reflect(next, "next");
		r.reflect(unk1, "unk1");
		r.reflect(life, "life");
		r.reflect(node, "node");
	}
	else {
		r.reflect(x2UnkA, "x2UnkA");
		r.reflect(x2Sector, "x2Sector");
		r.reflect(x2next, "x2next");
		r.reflect(next, "next");
		r.reflect(life, "life");
		r.reflect(node, "node");
	}
	r.setNextFlags(MemberListener::MemberFlags::MF_NONE);
}

void CKHook::onLevelLoaded(KEnvironment * kenv)
{
	// The goal here is to find the hook's sector to obtain
	// the correct references to some objects stored in STR.

	using namespace GameX1;

	int str = -1;
	if (kenv->version >= KEnvironment::KVERSION_XXL2) {
		str = (int)x2Sector - 1;
	}
	else if (activeSector >= -1) {
		str = activeSector;
	}
	else if (CKHkAnimatedCharacter* hkanim = this->dyncast<CKHkAnimatedCharacter>()) {
		str = (int)hkanim->sector - 1;
	}
	else if (CKHkBoss* boss = this->dyncast<CKHkBoss>()) {
		// We assume the boss machine is at the last sector,
		// but we can't keep that assumption for longer
		// if we want bosses in other sectors.
		str = kenv->numSectors - 1;
	}
	else if (CKHkTrioCatapult* tc = this->dyncast<CKHkTrioCatapult>()) {
		str = (int)tc->ckhtcSector - 1;
	}
	else if (CKHkWater* water = this->dyncast<CKHkWater>()) {
		// Check in the sector bit array for a single 1 bit
		int32_t strBitmask = water->ckhwSectorsBitArray & ((2 << kenv->numSectors) - 1);
		int realStr = -1;
		bool reliable = true;
		for (int i = 1; i < 32; ++i) {
			if ((strBitmask & (1 << i)) != 0) {
				if (realStr != -1) {
					reliable = false;
					break;
				}
				realStr = i - 1;
			}
		}
		
		// If only one active sector in the bit array, use that sector
		if (reliable) {
			str = realStr;
		}
		else {
			// The basic idea is to get the water's plane, find grounds matching
			// the Postref id in the ckhwGrounds vector, and take sector that has
			// the closest grounds to the water plane.

			static constexpr bool debug = false;
			Vector3 pos = water->ckhwUnk0->transform.getTranslationVector();
			AABoundingBox bb1{ pos + Vector3(water->ckhwSizeX, 0.0f, water->ckhwSizeZ), pos };
			if (debug)
				printf("   WaterBox  AABB: %10f %10f %10f, %10f %10f %10f\n", bb1.highCorner.x, bb1.highCorner.y, bb1.highCorner.z, bb1.lowCorner.x, bb1.lowCorner.y, bb1.lowCorner.z);

			if (!water->ckhwGrounds.empty()) {
				std::map<int, int> strHitFrequency;
				for (auto& postref : water->ckhwGrounds) {
					int gid = postref.id;
					int clcat = gid & 63;
					int clid = (gid >> 6) & 2047;
					assert(clcat == CGround::CATEGORY);
					assert(clid == CGround::CLASS_ID);
					int objid = gid >> 17;
					int bestSector = -1;
					float bestDist = std::numeric_limits<float>::infinity();
					for (int cand = 0; cand < kenv->numSectors; ++cand) {
						auto& cl = kenv->sectorObjects[cand].getClassType<CGround>();
						int objIndex = objid - cl.startId;
						if (objIndex >= 0 && objIndex < (int)cl.objects.size()) {
							CGround* gnd = (CGround*)cl.objects[objIndex];
							AABoundingBox& bb2 = gnd->aabb;

							// Shortest Manhattan distance between both bounding boxes
							float x = std::max(bb1.lowCorner.x - bb2.highCorner.x, 0.0f) + std::max(bb2.lowCorner.x - bb1.highCorner.x, 0.0f);
							float y = std::max(bb1.lowCorner.y - bb2.highCorner.y, 0.0f) + std::max(bb2.lowCorner.y - bb1.highCorner.y, 0.0f);
							float z = std::max(bb1.lowCorner.z - bb2.highCorner.z, 0.0f) + std::max(bb2.lowCorner.z - bb1.highCorner.z, 0.0f);
							float dist = x + y + z;

							if (debug) {
								printf(" - Sector %2i\n", cand);
								printf("   Ground    AABB: %10f %10f %10f, %10f %10f %10f\n", bb2.highCorner.x, bb2.highCorner.y, bb2.highCorner.z, bb2.lowCorner.x, bb2.lowCorner.y, bb2.lowCorner.z);
								printf("   Distance:  %f\n", dist);
							}
							if (dist < bestDist) {
								//assert(bestDist > 0.01f);
								bestDist = dist;
								bestSector = cand;
							}
						}
					}
					strHitFrequency[bestSector]++;
				}
				if (debug) {
					printf("ckhwSectorsBitArray = 0x%08X:\n", water->ckhwSectorsBitArray);
					printf("strHitFrequency:\n");
					for (auto& kv : strHitFrequency)
						printf(" %i: %i\n", kv.first, kv.second);
				}
				str = std::max_element(strHitFrequency.begin(), strHitFrequency.end(), [](auto& a, auto& b) {return a.second < b.second; })->first;
			}
		}
	}
	else if(this->life) {
		str = (this->life->unk1 >> 2) - 1;
	}
	//printf("bind %s's node to sector %i\n", this->getClassName(), str);
	//node.bind(kenv, str);

	// This will walk through every KPostponedRef in the hook
	// and bind them to the sector str.
	struct PostrefBinder : MemberListener {
		KEnvironment* kenv; int sector; CKHook* hook;
		PostrefBinder(KEnvironment* kenv, int sector, CKHook* hook) : kenv(kenv), sector(sector), hook(hook) {}

		virtual void reflect(uint8_t& ref, const char* name) override {}
		virtual void reflect(uint16_t& ref, const char* name) override {}
		virtual void reflect(uint32_t& ref, const char* name) override {}
		virtual void reflect(float& ref, const char* name) override {}
		virtual void reflect(Vector3& ref, const char* name) override {}
		virtual void reflect(EventNode& ref, const char* name, CKObject* user) override {}
		virtual void reflect(MarkerIndex& ref, const char* name) override {}
		virtual void reflect(std::string& ref, const char* name) override {}

		virtual void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override {
			onObject(ref.get());
		}
		virtual void reflectAnyPostRef(KAnyPostponedRef& postref, int clfid, const char* name) override {
			//printf("bind %s :: %s postref to sector %i\n", hook->getClassName(), name, sector);
			postref.bind(kenv, sector);
			onObject(postref.ref.get());
		}
		using MemberListener::reflect;

		void onObject(CKObject* obj) {
			// If an object referenced by the hook is a CSGHotSpot (XXL2+),
			// then bind its Node postref to the hook's sector.
			if (obj)
				if (CSGHotSpot* hs = obj->dyncast<CSGHotSpot>())
					hs->csghsUnk0.bind(kenv, sector);
		}
	};
	PostrefBinder binder{ kenv, str, this };
	//binder.reflect(this->node, "node");
	this->virtualReflectMembers(binder, kenv);
	activeSector = str;
}

int CKHook::getAddendumVersion()
{
	return 1;
}

void CKHook::deserializeAddendum(KEnvironment* kenv, File* file, int version)
{
	activeSector = file->readInt32();
}

void CKHook::serializeAddendum(KEnvironment* kenv, File* file)
{
	assert(activeSector >= -1);
	file->writeInt32(activeSector);
}

void CKHookLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	hook = kenv->readObjRef<CKHook>(file);
	nextLife = kenv->readObjRef<CKHookLife>(file);
	unk1 = file->readUint32();
}

void CKHookLife::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, hook);
	kenv->writeObjRef(file, nextLife);
	file->writeUint32(unk1);
}

void CKHkBasicBonus::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHook::reflectMembers2(r, kenv);
	r.reflect(nextBonus, "nextBonus");
	r.reflect(pool, "pool");
	r.reflect(cpnt, "cpnt");
	r.reflect(hero, "hero");
	r.reflect(somenums, "somenums");
}

void CKHkCrate::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhcUnk0, "ckhcUnk0");
	r.reflect(ckhcUnk1, "ckhcUnk1");
};

void CKHkLight::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(lightGrpLight, "lightGrpLight");
	r.reflect(lightEvtSeq1, "lightEvtSeq1", this);
	r.reflect(lightEvtSeq2, "lightEvtSeq2", this);
	r.reflect(lightEvtSeq3, "lightEvtSeq3", this);
	r.reflect(lightEvtSeq4, "lightEvtSeq4", this);
}

void CKHkSkyLife::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CKHookLife::deserialize(kenv, file, length);
	skyColor = file->readUint32();
	cloudColor = file->readUint32();
}

void CKHkSkyLife::serialize(KEnvironment* kenv, File* file)
{
	CKHookLife::serialize(kenv, file);
	file->writeUint32(skyColor);
	file->writeUint32(cloudColor);
}
