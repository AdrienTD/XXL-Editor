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

void CKHook::reflectMembers(MemberListener & r)
{
	//r.reflect(next, "next");
	//r.reflect(unk1, "unk1");
	//r.reflect(life, "life");
	//r.reflect(node, "node");
}

void CKHook::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
		next = kenv->readObjRef<CKHook>(file);
		unk1 = file->readUint32();
		life = kenv->readObjRef<CKHookLife>(file);
		node.read(file);
	}
	else {
		x2UnkA = file->readUint32();
		x2Sector = file->readUint32();
		x2next = kenv->readObjRef<CKHook>(file);
		next = kenv->readObjRef<CKHook>(file);
		//assert(x2next == next);
		life = kenv->readObjRef<CKHookLife>(file);
		node.read(file);
	}
	activeSector = -2;
}

void CKHook::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
		kenv->writeObjRef(file, next);
		file->writeUint32(unk1);
		kenv->writeObjRef(file, life);
		node.write(kenv, file);
	}
	else {
		file->writeUint32(x2UnkA);
		file->writeUint32(x2Sector);
		kenv->writeObjRef(file, x2next);
		kenv->writeObjRef(file, next);
		kenv->writeObjRef(file, life);
		node.write(kenv, file);
	}
}

void CKHook::onLevelLoaded(KEnvironment * kenv)
{
	// The goal here is to find the hook's sector to obtain
	// the correct references to some objects stored in STR.

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
			printf("bind %s :: %s postref to sector %i\n", hook->getClassName(), name, sector);
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
	binder.reflect(this->node, "node");
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

void CKHkWildBoar::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHook::reflectMembers2(r, kenv);
	r.reflect(nextBoar, "nextBoar");
	r.reflect(boundingSphere, "boundingSphere");
	r.reflect(animationDictionary, "animationDictionary");
	r.reflect(cpnt, "cpnt");
	r.reflect(pool, "pool");
	r.reflect(somenums, "somenums");
	r.reflect(shadowCpnt, "shadowCpnt");
}

void CKHkEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHook::reflectMembers2(r, kenv);
	r.reflect(unk1, "unk1");
	r.reflect(unk2, "unk2");
	r.reflect(unk3, "unk3");
	r.reflect(unk4, "unk4");
	r.reflect(unk5, "unk5");
	r.reflect(squad, "squad");
	r.reflect(unk7, "unk7");
	r.reflect(unk8, "unk8");
	r.reflect(unk9, "unk9");
	r.reflect(unkA, "unkA");
	r.reflect(shadowCpnt, "shadowCpnt");
	r.reflect(hkWaterFx, "hkWaterFx");
}

void CKHkSeizableEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHkEnemy::reflectMembers2(r, kenv);
	r.reflect(sunk1, "sunk1");
	r.reflect(sunk2, "sunk2");
	r.reflect(sunk3, "sunk3");
	r.reflect(sunk4, "sunk4");
	r.reflect(boundingShapes, "boundingShapes");
	r.reflect(particlesNodeFx1, "particlesNodeFx1");
	r.reflect(particlesNodeFx2, "particlesNodeFx2");
	r.reflect(particlesNodeFx3, "particlesNodeFx3");
	r.reflect(fogBoxNode, "fogBoxNode");
	r.reflect(sunused, "sunused");
	r.reflect(hero, "hero");
	r.reflect(romanAnimatedClone, "romanAnimatedClone");
	r.reflect(sunk5, "sunk5");
	r.reflect(sunk6, "sunk6");
}

void CKHkSquadSeizableEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHkSeizableEnemy::reflectMembers2(r, kenv);
	r.reflect(matrix33, "matrix33");
	r.reflect(sunk7, "sunk7");
}

void CKHkBasicEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHkSquadSeizableEnemy::reflectMembers2(r, kenv);
	r.reflect(beClone1, "beClone1");
	r.reflect(beClone2, "beClone2");
	r.reflect(beClone3, "beClone3");
	r.reflect(beClone4, "beClone4");
	r.reflect(beParticleNode1, "beParticleNode1");
	r.reflect(beParticleNode2, "beParticleNode2");
	r.reflect(beParticleNode3, "beParticleNode3");
	r.reflect(beParticleNode4, "beParticleNode4");
	r.reflect(beAnimDict, "beAnimDict");
	r.reflect(beSoundDict, "beSoundDict");
	r.reflect(beBoundNode, "beBoundNode");
	r.reflect(romanAnimatedClone2, "romanAnimatedClone2");
	r.reflect(beUnk1, "beUnk1");
	r.reflect(beUnk2, "beUnk2");
	r.reflect(romanAnimatedClone3, "romanAnimatedClone3");
	r.reflect(beUnk3, "beUnk3");
	r.reflect(beUnk4, "beUnk4");
	r.reflect(beUnk5, "beUnk5");
	r.reflect(beUnk6, "beUnk6");
}

void CKHkRocketRoman::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHkBasicEnemy::reflectMembers2(r, kenv);
	r.reflect(rrAnimDict, "rrAnimDict");
	r.reflect(rrParticleNode, "rrParticleNode");
	r.reflect(rrCylinderNode, "rrCylinderNode");
	r.reflect(rrSoundDictID, "rrSoundDictID");
}

void CKHkSkyLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHookLife::deserialize(kenv, file, length);
	skyColor = file->readUint32();
	cloudColor = file->readUint32();
}

void CKHkSkyLife::serialize(KEnvironment * kenv, File * file)
{
	CKHookLife::serialize(kenv, file);
	file->writeUint32(skyColor);
	file->writeUint32(cloudColor);
}

void CKHkBoatLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHookLife::deserialize(kenv, file, length);
	boatHook = kenv->readObjRef<CKHook>(file);
}

void CKHkBoatLife::serialize(KEnvironment * kenv, File * file)
{
	CKHookLife::serialize(kenv, file);
	kenv->writeObjRef(file, boatHook);
}

void CKHkAnimatedCharacter::reflectMembers(MemberListener& r)
{
	CKHook::reflectMembers(r);
	r.reflect(animDict, "animDict");
	r.reflect(shadowCpnt, "shadowCpnt");
	r.reflect(unkRef1, "unkRef1");
	r.reflect(matrix, "matrix");
	r.reflect(position, "position");
	r.reflect(orientation, "orientation");
	r.reflect(unkFloatArray, "unkFloatArray");
	r.reflect(unkFloat1, "unkFloat1");
	r.reflect(unkFloat2, "unkFloat2");
	r.reflect(unkFloat3, "unkFloat3");
	r.reflect(unkFloat4, "unkFloat4");
	r.reflect(sector, "sector");
}

void CKHkAnimatedCharacter::update()
{
	matrix = node->transform;
	position = matrix.getTranslationVector();
}

void CKHkActivator::reflectMembers(MemberListener & r)
{
	CKHook::reflectMembers(r);
	r.reflect(actAnimDict, "actAnimDict");
	r.reflect(actSndDict, "actSndDict");
	r.reflect(actSphere1, "actSphere1");
	r.reflect(actSphere2, "actSphere2");
	r.reflect(actUnk4, "actUnk4");
	r.reflect(actEvtSeq1, "actEvtSeq1", this);
	r.reflect(actEvtSeq2, "actEvtSeq2", this);
}

void CKHkActivator::update()
{
	Vector3 pos = node->transform.getTranslationVector();
	actSphere1->transform.setTranslation(pos);
	actSphere2->transform.setTranslation(pos);
}

void CKHkTorch::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(torchSndDict, "torchSndDict");
	r.reflect(torchBranch, "torchBranch");
	r.reflect(torchUnk2, "torchUnk2");
	r.reflect(torchUnk3, "torchUnk3");
	r.reflect(torchUnk4, "torchUnk4");
	r.reflect(torchUnk5, "torchUnk5");
	r.reflect(torchUnk6, "torchUnk6");
	r.reflect(torchUnk7, "torchUnk7");
	r.reflect(torchUnk8, "torchUnk8");
	r.reflect(torchEvtSeq1, "torchEvtSeq1", this);
	r.reflect(torchEvtSeq2, "torchEvtSeq2", this);
	r.reflect(torchEvtSeq3, "torchEvtSeq3", this);
}
void CKHkHearth::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(hearthSndDict, "hearthSndDict");
	r.reflect(hearthDynGround, "hearthDynGround");
	r.reflect(hearthEvtSeq1, "hearthEvtSeq1", this);
	r.reflect(hearthEvtSeq2, "hearthEvtSeq2", this);
	r.reflect(hearthEvtSeq3, "hearthEvtSeq3", this);
	r.reflect(hearthEvtSeq4, "hearthEvtSeq4", this);
	r.reflect(hearthEvtSeq5, "hearthEvtSeq5", this);
	r.reflect(hearthUnk7, "hearthUnk7");
	r.reflect(hearthUnk8, "hearthUnk8");
	r.reflect(hearthUnk9, "hearthUnk9");
	r.reflect(hearthUnk10, "hearthUnk10");
}
void CKHkLight::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(lightGrpLight, "lightGrpLight");
	r.reflect(lightEvtSeq1, "lightEvtSeq1", this);
	r.reflect(lightEvtSeq2, "lightEvtSeq2", this);
	r.reflect(lightEvtSeq3, "lightEvtSeq3", this);
	r.reflect(lightEvtSeq4, "lightEvtSeq4", this);
}

void CKHkPressionStone::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(psSquad, "psSquad");
	r.reflect(psDynGround, "psDynGround");
	r.reflect(psSndDict, "psSndDict");
	r.reflect(psEvtSeq1, "psEvtSeq1", this);
	r.reflect(psEvtSeq2, "psEvtSeq2", this);
	r.reflect(psUnk5, "psUnk5");
	r.reflect(psUnk6, "psUnk6");
	r.reflect(psUnk7, "psUnk7");
	r.reflect(psUnk8, "psUnk8");
}
void CKHkHero::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKHook::reflectMembers(r);
	r.reflect(heroGrpTrio, "heroGrpTrio");
	r.reflect(heroUnk1, "heroUnk1");
	r.reflect(heroUnk2, "heroUnk2");
	r.reflect(heroUnk3, "heroUnk3");
	r.reflect(heroUnk4, "heroUnk4");
	r.reflect(heroUnk5, "heroUnk5");
	r.reflect(heroUnk6, "heroUnk6");
	r.reflect(heroUnk7, "heroUnk7");
	r.reflect(heroUnk8, "heroUnk8");
	r.reflect(heroUnk9, "heroUnk9");
	r.reflect(heroUnk10, "heroUnk10");
	r.reflect(heroUnk11, "heroUnk11");
	r.reflect(heroBranch1, "heroBranch1");
	r.reflect(heroUnk13, "heroUnk13");
	r.reflect(heroUnk14, "heroUnk14");
	r.reflect(heroUnk15, "heroUnk15");
	r.reflect(heroUnk16, "heroUnk16");
	r.reflect(heroUnk17, "heroUnk17");
	r.reflect(heroUnk18, "heroUnk18");
	r.reflect(heroUnk19, "heroUnk19");
	r.reflect(heroUnk20, "heroUnk20");
	r.reflect(heroUnk21, "heroUnk21");
	r.reflect(heroUnk22, "heroUnk22");
	r.reflect(heroUnk23, "heroUnk23");
	r.reflect(heroAnimDict, "heroAnimDict");
	r.reflect(heroUnk25, "heroUnk25");
	r.reflect(heroUnk26, "heroUnk26");
	r.reflect(heroUnk27, "heroUnk27");
	r.reflect(heroUnk28, "heroUnk28");
	r.reflect(heroUnk29, "heroUnk29");
	r.reflect(heroBranch2, "heroBranch2");
	r.reflect(heroUnk31, "heroUnk31");
	r.reflect(heroUnk32, "heroUnk32");
	r.reflect(heroUnk33, "heroUnk33");
	r.reflect(heroUnk34, "heroUnk34");
	r.reflect(heroUnk35, "heroUnk35");
	r.reflect(heroUnk36, "heroUnk36");
	r.reflect(heroUnk37, "heroUnk37");
	r.reflect(heroUnk38, "heroUnk38");
	r.reflect(heroUnk39, "heroUnk39");
	r.reflect(heroSphere, "heroSphere");
	r.reflect(heroUnk41, "heroUnk41");
	r.reflect(heroUnk42, "heroUnk42");
	r.reflect(heroDynSphere, "heroDynSphere");
	r.reflect(heroCylinder, "heroCylinder");
	r.reflect(heroUnk45, "heroUnk45");
	r.reflect(heroUnk46, "heroUnk46");
	r.reflect(heroParticleNode, "heroParticleNode");
	r.reflect(heroTrailNode, "heroTrailNode");
	r.reflect(heroSndDict, "heroSndDict");
	r.reflect(heroUnk50, "heroUnk50");
	r.reflect(heroUnk51, "heroUnk51");
	r.reflect(heroUnk52, "heroUnk52");
	const size_t numElem = kenv->isRemaster ? 20 : 1;
	heroUnk53.resize(numElem);
	heroUnk54.resize(numElem);
	heroUnk55.resize(numElem);
	heroUnk56.resize(numElem);
	heroUnk57.resize(numElem);
	r.reflect(heroUnk53, "heroUnk53");
	r.reflect(heroUnk54, "heroUnk54");
	r.reflect(heroUnk55, "heroUnk55");
	r.reflect(heroUnk56, "heroUnk56");
	r.reflect(heroUnk57, "heroUnk57");
	r.reflect(heroUnk58, "heroUnk58");
	r.reflect(heroUnk59, "heroUnk59");
	r.reflect(heroUnk60, "heroUnk60");
	r.reflect(heroUnk61, "heroUnk61");
	r.reflect(heroUnk62, "heroUnk62");
	r.reflect(heroUnk63, "heroUnk63");
	r.reflect(heroUnk64, "heroUnk64");
	r.reflect(heroUnk65, "heroUnk65");
	r.reflect(heroUnk66, "heroUnk66");
	r.reflect(heroUnk67, "heroUnk67");
	r.reflect(heroUnk68, "heroUnk68");
	r.reflect(heroUnk69, "heroUnk69");
	r.reflect(heroUnk70, "heroUnk70");
	r.reflect(heroUnk71, "heroUnk71");
	r.reflect(heroUnk72, "heroUnk72");
	r.reflect(heroUnk73, "heroUnk73");
	r.reflect(heroUnk74, "heroUnk74");
	r.reflect(heroUnk75, "heroUnk75");
	r.reflect(heroUnk76, "heroUnk76");
	r.reflect(heroUnk77, "heroUnk77");
	r.reflect(heroUnk78, "heroUnk78");
	r.reflect(heroUnk79, "heroUnk79");
	r.reflect(heroUnk80, "heroUnk80");
	r.reflect(heroUnk81, "heroUnk81");
	r.reflect(heroUnk82, "heroUnk82");
	r.reflect(heroUnk83, "heroUnk83");
	r.reflect(heroBranch3, "heroBranch3");
	r.reflect(heroUnk85, "heroUnk85");
	r.reflect(heroUnk86, "heroUnk86");
	r.reflect(heroUnk87, "heroUnk87");
	r.reflect(heroUnk88, "heroUnk88");
	r.reflect(heroUnk89, "heroUnk89");
	r.reflect(heroUnk90, "heroUnk90");
	r.reflect(heroUnk91, "heroUnk91");
	r.reflect(heroUnk92, "heroUnk92");
	r.reflect(heroUnk93, "heroUnk93");
	r.reflect(heroUnk94, "heroUnk94");
	r.reflect(heroUnk95, "heroUnk95");
	r.reflect(heroUnk96, "heroUnk96");
	r.reflect(heroUnk97, "heroUnk97");
	r.reflect(heroUnk98, "heroUnk98");
	r.reflect(heroUnk99, "heroUnk99");
	r.reflect(heroUnk100, "heroUnk100");
	r.reflect(heroUnk101, "heroUnk101");
	r.reflect(heroUnk102, "heroUnk102");
	r.reflect(heroUnk103, "heroUnk103");
	r.reflect(heroUnk104, "heroUnk104");
	r.reflect(heroUnk105, "heroUnk105");
	r.reflect(heroUnk106, "heroUnk106");
	r.reflect(heroUnk107, "heroUnk107");
	r.reflect(heroUnk108, "heroUnk108");
	r.reflect(heroUnk109, "heroUnk109");
	r.reflect(heroUnk110, "heroUnk110");
	r.reflect(heroUnk111, "heroUnk111");
	r.reflect(heroUnk112, "heroUnk112");
	r.reflect(heroUnk113, "heroUnk113");
	r.reflect(heroUnk114, "heroUnk114");
	r.reflect(heroUnk115, "heroUnk115");
	r.reflect(heroUnk116, "heroUnk116");
	r.reflect(heroUnk117, "heroUnk117");
	r.reflect(heroUnk118, "heroUnk118");
	r.reflect(heroUnk119, "heroUnk119");
	r.reflect(heroUnk120, "heroUnk120");
	r.reflect(heroUnk121, "heroUnk121");
	r.reflect(heroUnk122, "heroUnk122");
	r.reflect(heroUnk123, "heroUnk123");
	r.reflect(heroUnk124, "heroUnk124");
	r.reflect(heroUnk125, "heroUnk125");
	r.reflect(heroUnk126, "heroUnk126");
	r.reflect(heroUnk127, "heroUnk127");
	r.reflect(heroUnk128, "heroUnk128");
	r.reflect(heroUnk129, "heroUnk129");
	r.reflect(heroUnk130, "heroUnk130");
	r.reflect(heroUnk131, "heroUnk131");
	r.reflect(heroUnk132, "heroUnk132");
	r.reflect(heroUnk133, "heroUnk133");
	r.reflect(heroUnk134, "heroUnk134");
	r.reflect(heroUnk135, "heroUnk135");
	r.reflect(heroUnk136, "heroUnk136");
	r.reflect(heroUnk137, "heroUnk137");
	r.reflect(heroUnk138, "heroUnk138");
	r.reflect(heroUnk139, "heroUnk139");
	r.reflect(heroUnk140, "heroUnk140");
	r.reflect(heroUnk141, "heroUnk141");
	r.reflect(heroUnk142, "heroUnk142");
	r.reflect(heroUnk143, "heroUnk143");
	r.reflect(heroUnk144, "heroUnk144");
	r.reflect(heroUnk145, "heroUnk145");
	r.reflect(heroUnk146, "heroUnk146");
	r.reflect(heroUnk147, "heroUnk147");
	r.reflect(heroUnk148, "heroUnk148");
	r.reflect(heroUnk149, "heroUnk149");
	r.reflect(heroUnk150, "heroUnk150");
	r.reflect(heroUnk151, "heroUnk151");
	r.reflect(heroUnk152, "heroUnk152");
	r.reflect(heroUnk153, "heroUnk153");
	r.reflect(heroUnk154, "heroUnk154");
	r.reflect(heroUnk155, "heroUnk155");
	r.reflect(heroUnk156, "heroUnk156");
	r.reflect(heroUnk157, "heroUnk157");
	r.reflect(heroUnk158, "heroUnk158");
	r.reflect(heroUnk159, "heroUnk159");
	r.reflect(heroUnk160, "heroUnk160");
	r.reflect(heroUnk161, "heroUnk161");
	r.reflect(heroUnk162, "heroUnk162");
	r.reflect(heroUnk163, "heroUnk163");
	r.reflect(heroUnk164, "heroUnk164");
	r.reflect(heroUnk165, "heroUnk165");
	r.reflect(heroUnk166, "heroUnk166");
	r.reflect(heroUnk167, "heroUnk167");
	r.reflect(heroUnk168, "heroUnk168");
	r.reflect(heroUnk169, "heroUnk169");
	r.reflect(heroUnk170, "heroUnk170");
	r.reflect(heroUnk171, "heroUnk171");
	r.reflect(heroUnk172, "heroUnk172");
	r.reflect(heroUnk173, "heroUnk173");
	r.reflect(heroUnk174, "heroUnk174");
	r.reflect(heroUnk175, "heroUnk175");
	r.reflect(heroShadowCpnt, "heroShadowCpnt");
	r.reflect(heroWaterFxHook, "heroWaterFxHook");
	r.reflect(heroUnk178, "heroUnk178");
}
void CKHkAsterix::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKHkHero::reflectMembers2(r, kenv);
	r.reflect(asteUnk0, "asteUnk0");
	r.reflect(asteUnk1, "asteUnk1");
	r.reflect(asteUnk2, "asteUnk2");
	r.reflect(asteUnk3, "asteUnk3");
	r.reflect(asteUnk4, "asteUnk4");
	r.reflect(asteUnk5, "asteUnk5");
	r.reflect(asteUnk6, "asteUnk6");
	r.reflect(asteUnk7, "asteUnk7");
	r.reflect(asteUnk8, "asteUnk8");
	r.reflect(asteUnk9, "asteUnk9");
	r.reflect(asteUnk10, "asteUnk10");
	r.reflect(asteUnk11, "asteUnk11");
	r.reflect(asteUnk12, "asteUnk12");
	r.reflect(asteUnk13, "asteUnk13");
	r.reflect(asteUnk14, "asteUnk14");
	r.reflect(asteUnk15, "asteUnk15");
	r.reflect(asteUnk16, "asteUnk16");
	r.reflect(asteUnk17, "asteUnk17");
	r.reflect(asteUnk18, "asteUnk18");
	r.reflect(asteUnk19, "asteUnk19");
	r.reflect(asteUnk20, "asteUnk20");
	r.reflect(asteUnk21, "asteUnk21");
	r.reflect(asteUnk22, "asteUnk22");
	r.reflect(asteUnk23, "asteUnk23");
	r.reflect(asteUnk24, "asteUnk24");
	r.reflect(asteDynSphere1, "asteDynSphere1");
	r.reflect(asteDynSphere2, "asteDynSphere2");
	r.reflect(asteBranch1, "asteBranch1");
	r.reflect(asteBranch2, "asteBranch2");
	r.reflect(asteUnk29, "asteUnk29");
	r.reflect(asteUnk30, "asteUnk30");
	r.reflect(asteSphere1, "asteSphere1");
	r.reflect(asteSphere2, "asteSphere2");
	r.reflect(asteBranch3, "asteBranch3");
	r.reflect(asteBranch4, "asteBranch4");
	r.reflect(asteParticleNode0, "asteParticleNode0");
	r.reflect(asteParticleNode1, "asteParticleNode1");
	r.reflect(asteParticleNode2, "asteParticleNode2");
	r.reflect(asteParticleNode3, "asteParticleNode3");
	r.reflect(asteParticleNode4, "asteParticleNode4");
	r.reflect(asteParticleNode5, "asteParticleNode5");
	r.reflect(asteParticleNode6, "asteParticleNode6");
	r.reflect(asteParticleNode7, "asteParticleNode7");
	r.reflect(asteParticleNode8, "asteParticleNode8");
	r.reflect(asteParticleNode9, "asteParticleNode9");
	r.reflect(asteParticleNode10, "asteParticleNode10");
	r.reflect(asteParticleNode11, "asteParticleNode11");
	r.reflect(asteParticleNode12, "asteParticleNode12");
	r.reflect(asteParticleNode13, "asteParticleNode13");
	r.reflect(asteParticleNode14, "asteParticleNode14");
	r.reflect(asteParticleNode15, "asteParticleNode15");
	r.reflect(asteParticleNode16, "asteParticleNode16");
	r.reflect(asteBranch5, "asteBranch5");
	r.reflect(asteBranch6, "asteBranch6");
}
void CKHkObelix::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKHkHero::reflectMembers2(r, kenv);
	r.reflect(obeUnk0, "obeUnk0");
	r.reflect(obeDynSphere1, "obeDynSphere1");
	r.reflect(obeDynSphere2, "obeDynSphere2");
	r.reflect(obeBranchA1, "obeBranchA1");
	r.reflect(obeBranchA2, "obeBranchA2");
	r.reflect(obeUnk5, "obeUnk5");
	r.reflect(obeUnk6, "obeUnk6");
	r.reflect(obeUnk7, "obeUnk7");
	r.reflect(obeSphere1, "obeSphere1");
	r.reflect(obeSphere2, "obeSphere2");
	r.reflect(obeBranchB1, "obeBranchB1");
	r.reflect(obeBranchB2, "obeBranchB2");
	r.reflect(obeParticleNode0, "obeParticleNode0");
	r.reflect(obeParticleNode1, "obeParticleNode1");
	r.reflect(obeParticleNode2, "obeParticleNode2");
	r.reflect(obeParticleNode3, "obeParticleNode3");
	r.reflect(obeParticleNode4, "obeParticleNode4");
	r.reflect(obeParticleNode5, "obeParticleNode5");
	r.reflect(obeParticleNode6, "obeParticleNode6");
	r.reflect(obeParticleNode7, "obeParticleNode7");
	r.reflect(obeParticleNode8, "obeParticleNode8");
	r.reflect(obeParticleNode9, "obeParticleNode9");
	r.reflect(obeParticleNode10, "obeParticleNode10");
	r.reflect(obeParticleNode11, "obeParticleNode11");
	r.reflect(obeParticleNode12, "obeParticleNode12");
	r.reflect(obeParticleNode13, "obeParticleNode13");
	r.reflect(obeParticleNode14, "obeParticleNode14");
	r.reflect(obeParticleNode15, "obeParticleNode15");
	r.reflect(obeParticleNode16, "obeParticleNode16");
	r.reflect(obeParticleNode17, "obeParticleNode17");
	r.reflect(obeBranchE1, "obeBranchE1");
	r.reflect(obeBranchE2, "obeBranchE2");
	r.reflect(obeBranchE3, "obeBranchE3");
	r.reflect(obeBranchE4, "obeBranchE4");
	r.reflect(obeBranchE5, "obeBranchE5");
	r.reflect(obeBranchE6, "obeBranchE6");
}
void CKHkIdefix::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKHkHero::reflectMembers2(r, kenv);
	r.reflect(ideUnk0, "ideUnk0");
	r.reflect(ideSphere, "ideSphere");
	r.reflect(ideBranch, "ideBranch");
	r.reflect(ideUnk3, "ideUnk3");
	r.reflect(ideUnk4, "ideUnk4");
	r.reflect(ideUnk5, "ideUnk5");
	r.reflect(ideUnk6, "ideUnk6");
	r.reflect(ideUnk7, "ideUnk7");
	r.reflect(ideParticleNode1, "ideParticleNode1");
	r.reflect(ideParticleNode2, "ideParticleNode2");
}
void CKHkMachinegun::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKHook::reflectMembers(r);
	r.reflect(mgunUnk0, "mgunUnk0");
	r.reflect(mgunUnk1, "mgunUnk1");
	r.reflect(mgunSpline, "mgunSpline");
	r.reflect(mgunUnk3, "mgunUnk3");
	r.reflect(mgunUnk4, "mgunUnk4");
	r.reflect(mgunUnk5, "mgunUnk5");
	r.reflect(mgunUnk6, "mgunUnk6");
	r.reflect(mgunUnk7, "mgunUnk7");
	r.reflect(mgunUnk8, "mgunUnk8");
	r.reflect(mgunUnk9, "mgunUnk9");
	r.reflect(mgunUnk10, "mgunUnk10");
	r.reflect(mgunUnk11, "mgunUnk11");
	r.reflect(mgunUnk12, "mgunUnk12");
	r.reflect(mgunUnk13, "mgunUnk13");
	r.reflect(mgunUnk14, "mgunUnk14");
	r.reflect(mgunUnk15, "mgunUnk15");
	r.reflect(mgunUnk16, "mgunUnk16");
	r.reflect(mgunUnk17, "mgunUnk17");
	r.reflect(mgunUnk18, "mgunUnk18");
	r.reflect(mgunUnk19, "mgunUnk19");
	r.reflect(mgunUnk20, "mgunUnk20");
	r.reflect(mgunUnk21, "mgunUnk21");
	r.reflect(mgunAnimDict, "mgunAnimDict");
	r.reflect(mgunSndDict, "mgunSndDict");
	r.reflect(mgunAnimNode, "mgunAnimNode");
	r.reflect(mgunNode, "mgunNode");
	r.reflect(mgunUnk26, "mgunUnk26");
	r.reflect(mgunParticles1, "mgunParticles1");
	r.reflect(mgunParticles2, "mgunParticles2");
	r.reflect(mgunParticles3, "mgunParticles3");
	r.reflect(mgunUnk30, "mgunUnk30");
	r.reflect(mgunUnk31, "mgunUnk31");
	r.reflect(mgunUnk32, "mgunUnk32");
	r.reflect(mgunUnk33, "mgunUnk33");
	r.reflect(mgunClones, "mgunClones");
	r.reflect(mgunUnk35, "mgunUnk35");
	r.reflect(mgunUnk36, "mgunUnk36");
	r.reflect(mgunUnk37, "mgunUnk37");
	r.reflect(mgunUnk38, "mgunUnk38");
	r.reflect(mgunUnk39, "mgunUnk39");
	r.reflect(mgunUnk40, "mgunUnk40");
	r.reflect(mgunBranch1, "mgunBranch1");
	r.reflect(mgunBranch2, "mgunBranch2");
	r.reflect(mgunBranch3, "mgunBranch3");
	r.reflect(mgunBranch4, "mgunBranch4");
	r.reflect(mgunUnk45, "mgunUnk45");
	r.reflect(mgunUnk46, "mgunUnk46", this);
	r.reflect(mgunUnk47, "mgunUnk47", this);
	r.reflect(mgunDynGround1, "mgunDynGround1");
	r.reflect(mgunDynGround2, "mgunDynGround2");
	r.reflect(mgunProjectile, "mgunProjectile");
	r.reflect(mgunUnk51, "mgunUnk51");
	r.reflect(mgunUnk52, "mgunUnk52");
	r.reflect(mgunShadowCpnt, "mgunShadowCpnt");
	r.reflect(mgunUnk54, "mgunUnk54");
	r.reflect(mgunUnk55, "mgunUnk55");
}
void CKHkDrawbridge::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(dbStaticGround, "dbStaticGround");
	r.reflect(dbDynGround, "dbDynGround");
	r.reflect(dbSndDict, "dbSndDict");
	r.reflect(dbEvtSeq, "dbEvtSeq", this);
	r.reflect(dbUnk4, "dbUnk4");
	r.reflect(dbUnk5, "dbUnk5");
	r.reflect(dbUnk6, "dbUnk6");
	r.reflect(dbUnk7, "dbUnk7");
	r.reflect(dbUnk8, "dbUnk8");
}
void CKHkMegaAshtray::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(maAnimDict, "maAnimDict");
	r.reflect(maSndDict, "maSndDict");
	r.reflect(maDynGround1, "maDynGround1");
	r.reflect(maDynGround2, "maDynGround2");
	r.reflect(maUnk4, "maUnk4");
	r.reflect(maUnk5, "maUnk5");
}
void CKHkCorkscrew::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(cswDynGround, "cswDynGround");
	r.reflect(cswSndDict, "cswSndDict");
	r.reflect(cswUnk2, "cswUnk2");
	r.reflect(cswUnk3, "cswUnk3", this);
	r.reflect(cswUnk4, "cswUnk4", this);
	r.reflect(cswUnk5, "cswUnk5");
	r.reflect(cswUnk6, "cswUnk6");
	r.reflect(cswUnk7, "cswUnk7");
	r.reflect(cswUnk8, "cswUnk8");
	r.reflect(cswUnk9, "cswUnk9");
	r.reflect(cswUnk10, "cswUnk10");
	r.reflect(cswUnk11, "cswUnk11");
}
void CKHkCorkscrew::update()
{
	cswDynGround->node = node->cast<CSGBranch>()->child;
}
void CKHkTurnstile::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(tsDynGround, "tsDynGround");
	r.reflect(tsSndDict, "tsSndDict");
	r.reflect(tsCylinder, "tsCylinder");
	r.reflect(tsUnk3, "tsUnk3", this);
	r.reflect(tsUnk4, "tsUnk4", this);
	r.reflect(tsUnk5, "tsUnk5", this);
	r.reflect(tsUnk6, "tsUnk6");
	r.reflect(tsUnk7, "tsUnk7");
	r.reflect(tsUnk8, "tsUnk8");
}
void CKHkTurnstile::update()
{
	tsDynGround->node = node->cast<CSGBranch>()->child;
}
void CKHkLifter::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(liftSquad, "liftSquad");
	r.reflect(liftDynGround, "liftDynGround");
	r.reflect(liftStaticGround, "liftStaticGround");
	r.reflect(liftNode1, "liftNode1");
	r.reflect(liftNode2, "liftNode2");
	r.reflect(liftSndDict, "liftSndDict");
	r.reflect(liftUnk6, "liftUnk6");
	r.reflect(liftUnk7, "liftUnk7");
	r.reflect(liftUnk8, "liftUnk8");
	r.reflect(liftUnk9, "liftUnk9");
	r.reflect(liftUnk10, "liftUnk10");
	r.reflect(liftUnk11, "liftUnk11");
	r.reflect(liftUnk12, "liftUnk12", this);
	r.reflect(liftUnk13, "liftUnk13", this);
	r.reflect(liftUnk14, "liftUnk14", this);
	r.reflect(liftUnk15, "liftUnk15", this);
	r.reflect(liftUnk16, "liftUnk16", this);
	r.reflect(liftUnk17, "liftUnk17", this);
}
void CKHkRotaryBeam::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKHook::reflectMembers(r);
	r.reflect(rbAnimDict, "rbAnimDict");
	r.reflect(rbSndDict, "rbSndDict");
	r.reflect(rbUnk2, "rbUnk2", this);
	r.reflect(rbUnk3, "rbUnk3", this);
	r.reflect(rbUnk4, "rbUnk4");
	r.reflect(rbUnk5, "rbUnk5");
	r.reflect(rbUnk6, "rbUnk6");
	r.reflect(rbUnk7, "rbUnk7");
	r.reflect(rbUnk8, "rbUnk8");
	r.reflect(rbUnk9, "rbUnk9");
	r.reflect(rbUnk10, "rbUnk10");
	r.reflect(rbUnk11, "rbUnk11");
	r.reflect(rbUnk12, "rbUnk12");
}
void CKHkLightPillar::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
}
void CKHkWind::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(windSndDict, "windSndDict");
	r.reflect(windFogBoxes, "windFogBoxes");
	r.reflect(windUnk2, "windUnk2");
	r.reflect(windUnk3, "windUnk3");
	r.reflect(windUnk4, "windUnk4");
	r.reflect(windUnk5, "windUnk5");
	r.reflect(windUnk6, "windUnk6");
	r.reflect(windUnk7, "windUnk7", this);
	r.reflect(windUnk8, "windUnk8", this);
}
void CKHkPowderKeg::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(pkGround, "pkGround");
	r.reflect(pkCylinder, "pkCylinder");
	r.reflect(pkSndDict, "pkSndDict");
	r.reflect(pkUnk3, "pkUnk3");
	r.reflect(pkUnk4, "pkUnk4", this);
	r.reflect(pkUnk5, "pkUnk5", this);
	r.reflect(pkUnk6, "pkUnk6");
	r.reflect(pkUnk7, "pkUnk7");
	r.reflect(pkUnk8, "pkUnk8");
	r.reflect(pkUnk9, "pkUnk9");
}
void CKHkSwingDoor::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(swdSndDict, "swdSndDict");
	r.reflect(swdEvtSeq1, "swdEvtSeq1", this);
	r.reflect(swdEvtSeq2, "swdEvtSeq2", this);
	r.reflect(swdUnk3, "swdUnk3");
	r.reflect(swdUnk4, "swdUnk4");
	r.reflect(swdUnk5, "swdUnk5");
	r.reflect(swdUnk6, "swdUnk6");
	r.reflect(swdUnk7, "swdUnk7");
	r.reflect(swdNode1, "swdNode1");
	r.reflect(swdNode2, "swdNode2");
}
void CKHkSlideDoor::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKHook::reflectMembers(r);
	r.reflect(sldSndDict, "sldSndDict");
	r.reflect(sldEvtSeq1, "sldEvtSeq1", this);
	r.reflect(sldEvtSeq2, "sldEvtSeq2", this);
	r.reflect(sldUnk3, "sldUnk3");
	r.reflect(sldUnk4, "sldUnk4");
	r.reflect(sldUnk5, "sldUnk5");
	r.reflect(sldUnk6, "sldUnk6");
	r.reflect(sldUnk7, "sldUnk7");
	r.reflect(sldNode, "sldNode");
	r.reflect(sldDynGround, "sldDynGround");
	r.reflect(sldUnk10, "sldUnk10");
	r.reflect(sldUnk11, "sldUnk11");
	if (kenv->isRemaster)
		r.reflect(sldRomasterValue, "sldRomasterValue");
}
void CKHkCrumblyZone::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(czSndDict, "czSndDict");
	r.reflect(czGround, "czGround");
	r.reflect(czNode1, "czNode1");
	r.reflect(czNode2, "czNode2");
	r.reflect(czObb, "czObb");
	r.reflect(czProjectileScrap, "czProjectileScrap");
	r.reflect(czParticleNode, "czParticleNode");
	r.reflect(czUnk7, "czUnk7");
	r.reflect(czUnk8, "czUnk8");
	r.reflect(czEvtSeqMaybe, "czEvtSeqMaybe");
	r.reflect(czEvtSeq2, "czEvtSeq2", this);
}
void CKHkHelmetCage::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(hcNode1, "hcNode1");
	r.reflect(hcClone1, "hcClone1");
	r.reflect(hcClone2, "hcClone2");
	r.reflect(hcDynGround1, "hcDynGround1");
	r.reflect(hcDynGround2, "hcDynGround2");
	r.reflect(hcSndDict, "hcSndDict");
	r.reflect(hcUnk6, "hcUnk6");
	r.reflect(hcUnk7, "hcUnk7");
	r.reflect(hcUnk8, "hcUnk8");
	r.reflect(hcUnk9, "hcUnk9");
	r.reflect(hcUnk10, "hcUnk10");
	r.reflect(hcUnk11, "hcUnk11");
	r.reflect(hcUnk12, "hcUnk12");
	r.reflect(hcUnk13, "hcUnk13");
	r.reflect(hcUnk14, "hcUnk14");
	r.reflect(hcUnk15, "hcUnk15");
	r.reflect(hcObb1, "hcObb1");
	r.reflect(hcObb2, "hcObb2");
}
void CKHkRollingStone::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(rlstPath, "rlstPath");
	r.reflect(rlstSphere, "rlstSphere");
	r.reflect(rlstProjScrap, "rlstProjScrap");
	r.reflect(rlstSndDict, "rlstSndDict");
	r.reflect(rlstClone, "rlstClone");
	r.reflect(rlstUnk5, "rlstUnk5");
	r.reflect(rlstUnk6, "rlstUnk6");
	r.reflect(rlstUnk7, "rlstUnk7");
	r.reflect(rlstUnk8, "rlstUnk8");
	r.reflect(rlstUnk9, "rlstUnk9");
	r.reflect(rlstUnk10, "rlstUnk10");
	r.reflect(rlstUnk11, "rlstUnk11");
	r.reflect(rlstUnk12, "rlstUnk12");
	r.reflect(rlstUnk13, "rlstUnk13");
	r.reflect(rlstUnk14, "rlstUnk14", this);
}
void CKHkRollingStone::onLevelLoaded(KEnvironment * kenv)
{
	CKHook::onLevelLoaded(kenv);
	if(rlstPath)
		rlstPath->usingSector = (this->life->unk1 >> 2) - 1;
}
void CKHkPushPullAsterix::Special::reflectMembers(MemberListener &r) {
	r.reflect(mUnk0, "mUnk0");
	r.reflect(mUnk1, "mUnk1");
	r.reflect(mUnk2, "mUnk2");
	r.reflect(mUnk3, "mUnk3");
	r.reflect(mUnk4, "mUnk4");
	r.reflect(mUnk5, "mUnk5");
	r.reflect(mUnk6, "mUnk6");
	r.reflect(mSndDict, "mSndDict");
}
void CKHkPushPullAsterix::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(ppaUnk0, "ppaUnk0");
	r.reflect(ppaUnk1, "ppaUnk1");
	r.reflect(ppaUnk2, "ppaUnk2");
	r.reflect(ppaUnk3, "ppaUnk3");
	r.reflect(ppaUnk4, "ppaUnk4");
	r.reflect(ppaUnk5, "ppaUnk5");
	r.reflect(ppaUnk6, "ppaUnk6");
	r.reflect(ppaUnk7, "ppaUnk7");
	r.reflect(ppaUnk8, "ppaUnk8");
	r.reflect(ppaUnk9, "ppaUnk9");
	r.reflect(ppaUnk10, "ppaUnk10");
	r.reflect(ppaUnk11, "ppaUnk11");
	r.reflect(ppaUnk12, "ppaUnk12");
	r.reflect(ppaFlaggedPath, "ppaFlaggedPath");
	r.reflect(ppaLine, "ppaLine");
	r.reflect(ppaUnk15, "ppaUnk15");
	r.reflect(ppaUnk16, "ppaUnk16");
	r.reflect(ppaUnk17, "ppaUnk17");
	r.reflect(ppaUnk18, "ppaUnk18");
	r.reflect(ppaUnk19, "ppaUnk19");
	r.reflect(ppaUnk20, "ppaUnk20");
	r.reflect(ppaUnk21, "ppaUnk21");
	r.reflect(ppaObb, "ppaObb");
	r.reflect(ppaUnk23, "ppaUnk23");
	r.reflect(ppaUnk24, "ppaUnk24");
	r.reflect(ppaUnk25, "ppaUnk25");
	r.reflect(ppaUnk26, "ppaUnk26");
	r.reflect(ppaParticleNode1, "ppaParticleNode1");
	r.reflect(ppaParticleNode2, "ppaParticleNode2");
	r.reflect(ppaUnk29, "ppaUnk29");
	r.reflect(ppaSndDict, "ppaSndDict");
	r.reflect(ppaDynGround, "ppaDynGround");
	r.reflect(ppaUnk32, "ppaUnk32", this);
	r.reflect(ppaUnk33, "ppaUnk33", this);
	r.reflect(ppaUnk34, "ppaUnk34", this);
	r.reflect(ppaUnk35, "ppaUnk35", this);
	r.reflect(ppaUnk36, "ppaUnk36");
	r.reflect(ppaUnk37, "ppaUnk37");
	r.reflect(ppaUnk38, "ppaUnk38");
	r.reflect(ppaBranch1, "ppaBranch1");
	r.reflect(ppaBranch2, "ppaBranch2");
	r.reflect(ppaBranch3, "ppaBranch3");
	r.reflect(ppaBranch4, "ppaBranch4");
	r.reflect(ppaUnk43, "ppaUnk43");
	r.reflect(ppaUnk44, "ppaUnk44");
}
void CKHkPushPullAsterix::onLevelLoaded(KEnvironment * kenv)
{
	CKHook::onLevelLoaded(kenv);
	if(ppaFlaggedPath)
		ppaFlaggedPath->cast<CKFlaggedPath>()->usingSector = (this->life->unk1 >> 2) - 1;
}
void CKHkBumper::reflectMembers2(MemberListener &r, KEnvironment* kenv) {
	CKHook::reflectMembers(r);
	r.reflect(bmpBranch, "bmpBranch");
	r.reflect(bmpAnimnode, "bmpAnimnode");
	r.reflect(bmpAnimDict, "bmpAnimDict");
	r.reflect(bmpSndDict, "bmpSndDict");
	r.reflect(bmpObb, "bmpObb");
	r.reflect(bmpGround, "bmpGround");
	r.reflect(bmpUnk6, "bmpUnk6", this);
	r.reflect(bmpUnk7, "bmpUnk7");
	r.reflect(bmpUnk8, "bmpUnk8");
	r.reflect(bmpUnk9, "bmpUnk9");
	r.reflect(bmpUnk10, "bmpUnk10");
}
void CKHkClueMan::Ing::reflectMembers(MemberListener &r) {
	r.reflect(mUnk0, "mUnk0");
	r.reflect(mCamera, "mCamera");
	r.reflect(mUnk2, "mUnk2");
	r.reflect(mUnk3, "mUnk3");
}
void CKHkClueMan::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(cmUnk0, "cmUnk0");
	r.reflect(cmUnk1, "cmUnk1");
	r.reflect(cmUnk2, "cmUnk2");
	r.reflect(cmUnk3, "cmUnk3");
	r.reflect(cmUnk4, "cmUnk4");
	r.reflect(cmUnk5, "cmUnk5");
	r.reflect(cmUnk6, "cmUnk6");
	r.reflect(cmUnk7, "cmUnk7");
	r.reflectSize<uint16_t>(cmBings, "sizeFor_cmBings");
	r.reflect(cmBings, "cmBings");
	r.reflectSize<uint16_t>(cmDings, "sizeFor_cmDings");
	r.reflect(cmDings, "cmDings");
	r.reflectSize<uint16_t>(cmFings, "sizeFor_cmFings");
	r.reflect(cmFings, "cmFings");
	r.reflect(cmUnk14, "cmUnk14");
	r.reflect(cmUnk15, "cmUnk15");
	r.reflect(cmUnk16, "cmUnk16");
	r.reflect(cmUnk17, "cmUnk17");
	r.reflect(cmUnk18, "cmUnk18");
	r.reflect(cmUnk19, "cmUnk19", this);
	r.reflect(cmUnk20, "cmUnk20", this);
	r.reflect(cmUnk21, "cmUnk21", this);
	r.reflect(cmUnk22, "cmUnk22", this);
	r.reflect(cmUnk23, "cmUnk23", this);
	r.reflect(cmUnk24, "cmUnk24");
	r.reflect(cmUnk25, "cmUnk25");
	r.reflect(cmUnk26, "cmUnk26");
	r.reflect(cmUnk27, "cmUnk27");
	r.reflect(cmUnk28, "cmUnk28");
	r.reflect(cmUnk29, "cmUnk29");
	r.reflect(cmUnk30, "cmUnk30");
	r.reflect(cmUnk31, "cmUnk31");
	r.reflect(cmUnk32, "cmUnk32");
	r.reflect(cmUnk33, "cmUnk33");
	r.reflect(cmUnk34, "cmUnk34");
	r.reflect(cmUnk35, "cmUnk35");
	r.reflect(cmUnk36, "cmUnk36");
	r.reflect(cmUnk37, "cmUnk37");
	r.reflect(cmBillboard1, "cmBillboard1");
	r.reflect(cmBillboard2, "cmBillboard2");
	r.reflect(cmUnk40, "cmUnk40");
	r.reflect(cmUnk41, "cmUnk41");
	r.reflect(cmUnk42, "cmUnk42");
	r.reflect(cmUnk43, "cmUnk43");
	r.reflect(cmUnk44, "cmUnk44");
	r.reflect(cmUnk45, "cmUnk45");
	r.reflect(cmUnk46, "cmUnk46");
	r.reflect(cmUnk47, "cmUnk47");
	r.reflect(cmUnk48, "cmUnk48");
	r.reflect(cmUnk49, "cmUnk49");
	r.reflect(cmUnk50, "cmUnk50");
	r.reflect(cmUnk51, "cmUnk51");
	r.reflect(cmUnk52, "cmUnk52");
	r.reflect(cmUnk53, "cmUnk53");
	r.reflect(cmUnk54, "cmUnk54");
	r.reflect(cmUnk55, "cmUnk55");
	r.reflect(cmUnk56, "cmUnk56");
	r.reflect(cmUnk57, "cmUnk57");
	r.reflect(cmUnk58, "cmUnk58");
	r.reflect(cmUnk59, "cmUnk59");
	r.reflect(cmUnk60, "cmUnk60");
}
void CKHkSky::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
}
void CKHkAsterixShop::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(shopAnimNode2, "shopAnimNode2");
	r.reflect(shopAnimDict1, "shopAnimDict1");
	r.reflect(shopAnimDict2, "shopAnimDict2");
	r.reflect(shopSndDict, "shopSndDict");
	r.reflect(shopBillboardList, "shopBillboardList");
	r.reflect(shopText1, "shopText1");
	r.reflect(shopText2, "shopText2");
	r.reflect(shopText3, "shopText3");
	r.reflect(shopText4, "shopText4");
	r.reflect(shopBillboard1, "shopBillboard1");
	r.reflect(shopBillboard2, "shopBillboard2");
	r.reflect(shopUnk11, "shopUnk11");
	r.reflect(shopUnk12, "shopUnk12");
	r.reflect(shopUnk13, "shopUnk13");
	r.reflect(shopUnk14, "shopUnk14");
	r.reflect(shopUnk15, "shopUnk15");
	r.reflect(shopUnk16, "shopUnk16");
	r.reflect(shopUnk17, "shopUnk17");
	r.reflect(shopUnk18, "shopUnk18");
	r.reflect(shopCameraTrack, "shopCameraTrack");
	r.reflect(shopUnk20, "shopUnk20");
	r.reflect(shopUnk21, "shopUnk21");
	r.reflect(shopUnk22, "shopUnk22");
	r.reflect(shopUnk23, "shopUnk23");
	r.reflect(shopUnk24, "shopUnk24");
	r.reflect(shopDynGround, "shopDynGround");
	r.reflect(shopUnk26, "shopUnk26");
	r.reflect(shopUnk27, "shopUnk27");
	r.reflect(shopUnk28, "shopUnk28");
	r.reflect(shopUnk29, "shopUnk29");
	r.reflect(shopUnk30, "shopUnk30");
	r.reflect(shopUnk31, "shopUnk31");
}
void CKHkWaterFall::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(wfallBranch2, "wfallBranch2");
	r.reflect(wfallUnk1, "wfallUnk1");
	r.reflect(wfallUnk2, "wfallUnk2");
}
void CKHkAsterixCheckpoint::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(acpNode, "acpNode");
	r.reflect(acpAnimDict, "acpAnimDict");
	r.reflect(acpSndDict, "acpSndDict");
	r.reflect(acpSphere1, "acpSphere1");
	r.reflect(acpSphere2, "acpSphere2");
	r.reflect(acpSphere3, "acpSphere3");
	r.reflect(acpParticleNode1, "acpParticleNode1");
	r.reflect(acpParticleNode2, "acpParticleNode2");
	r.reflect(acpGrpCheckpoint, "acpGrpCheckpoint");
	r.reflect(acpUnk9, "acpUnk9");
}
void CKHkBonusSpitter::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(bsDynGround, "bsDynGround");
	r.reflect(bsNode, "bsNode");
	r.reflect(bsUnk2, "bsUnk2");
	r.reflect(bsUnk3, "bsUnk3");
	r.reflect(bsUnk4, "bsUnk4");
	r.reflect(bsBonusType, "bsBonusType");
}
void CKHkTeleBridge::Part::reflectMembers(MemberListener &r) {
	r.reflect(mClone1, "mClone1");
	r.reflect(mDynGround, "mDynGround");
	r.reflect(mClone2, "mClone2");
	r.reflect(mClone3, "mClone3");
}
void CKHkTeleBridge::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(tbStaticGround, "tbStaticGround");
	r.reflect(tbSndDict, "tbSndDict");
	r.reflect(tbUnk2, "tbUnk2");
	r.reflect(tbUnk3, "tbUnk3");
	r.reflect(tbUnk4, "tbUnk4");
	r.reflect(tbUnk5, "tbUnk5");
	r.reflect(tbUnk6, "tbUnk6");
	r.reflect(tbUnk7, "tbUnk7");
	r.reflect(tbUnk8, "tbUnk8");
	r.reflect(tbUnk9, "tbUnk9");
	r.reflect(tbUnk10, "tbUnk10", this);
	r.reflect(tbUnk11, "tbUnk11", this);
	r.reflect(tbParts, "tbParts");
}
void CKHkTelepher::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(telUnk0, "telUnk0");
	r.reflect(telUnk1, "telUnk1");
	r.reflect(telUnk2, "telUnk2");
	r.reflect(telUnk3, "telUnk3");
	r.reflect(telFlaggedPath, "telFlaggedPath");
	r.reflect(telDynGround, "telDynGround");
	r.reflect(telUnk6, "telUnk6");
	r.reflect(telUnk7, "telUnk7");
	r.reflect(telUnk8, "telUnk8");
	r.reflect(telUnk9, "telUnk9");
	r.reflect(telUnk10, "telUnk10");
	r.reflect(telUnk11, "telUnk11");
	r.reflect(telUnk12, "telUnk12");
	r.reflect(telUnk13, "telUnk13");
	r.reflect(telUnk14, "telUnk14");
	r.reflect(telUnk15, "telUnk15");
	r.reflect(telUnk16, "telUnk16");
	r.reflect(telUnk17, "telUnk17");
	r.reflect(telUnk18, "telUnk18");
	r.reflect(telUnk19, "telUnk19");
	r.reflect(telUnk20, "telUnk20");
	r.reflect(telUnk21, "telUnk21", this);
	r.reflect(telSndDict, "telSndDict");
}
void CKHkTowedTelepher::reflectMembers(MemberListener &r) {
	CKHkTelepher::reflectMembers(r);
	r.reflect(towtelTowNode1, "towtelTowNode1");
	r.reflect(towtelTowNode2, "towtelTowNode2");
	r.reflect(towtelParticleNode, "towtelParticleNode");
	r.reflect(towtelUnk3, "towtelUnk3");
	r.reflect(towtelUnk4, "towtelUnk4");
	r.reflect(towtelUnk5, "towtelUnk5");
	r.reflect(towtelUnk6, "towtelUnk6");
	r.reflect(towtelUnk7, "towtelUnk7");
	r.reflect(towtelUnk8, "towtelUnk8");
	r.reflect(towtelUnk9, "towtelUnk9");
	r.reflect(towtelUnk10, "towtelUnk10");
	r.reflect(towtelUnk11, "towtelUnk11");
	r.reflect(towtelSphere, "towtelSphere");
	r.reflect(towtelUnk13, "towtelUnk13");
	r.reflect(towtelUnk14, "towtelUnk14", this);
	r.reflect(towtelUnk15, "towtelUnk15", this);
}

void CKHkSquadEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHkEnemy::reflectMembers2(r, kenv);
	r.reflect(matrix33, "matrix33");
	r.reflect(hseUnk2, "hseUnk2");
}

void CKHkJetPackRoman::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	for (auto &ref : hjpUnk0)
		ref = kenv->readObjRef<CKObject>(file);
	hjpUnk1 = file->readUint8();
	for (float &f : hjpUnk2)
		f = file->readFloat();
	hjpUnk3 = kenv->readObjRef<CKObject>(file);
	hjpUnk4 = file->readUint8();
	for (float &f : hjpUnk5)
		f = file->readFloat();
	hjpUnk6 = kenv->readObjRef<CKObject>(file);
	hjpUnk7 = kenv->readObjRef<CKObject>(file);
	hjpUnk8 = kenv->readObjRef<CKObject>(file);
	CKHkSquadEnemy::deserialize(kenv, file, length);
}

void CKHkJetPackRoman::serialize(KEnvironment * kenv, File * file)
{
	for (auto &ref : hjpUnk0)
		kenv->writeObjRef(file, ref);
	file->writeUint8(hjpUnk1);
	for (float &f : hjpUnk2)
		file->writeFloat(f);
	kenv->writeObjRef(file, hjpUnk3);
	file->writeUint8(hjpUnk4);
	for (float &f : hjpUnk5)
		file->writeFloat(f);
	kenv->writeObjRef(file, hjpUnk6);
	kenv->writeObjRef(file, hjpUnk7);
	kenv->writeObjRef(file, hjpUnk8);
	CKHkSquadEnemy::serialize(kenv, file);
}

void CKHkJetPackRoman::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(hjpUnk0, "hjpUnk0");
	r.reflect(hjpUnk1, "hjpUnk1");
	r.reflect(hjpUnk2, "hjpUnk2");
	r.reflect(hjpUnk3, "hjpUnk3");
	r.reflect(hjpUnk4, "hjpUnk4");
	r.reflect(hjpUnk5, "hjpUnk5");
	r.reflect(hjpUnk6, "hjpUnk6");
	r.reflect(hjpUnk7, "hjpUnk7");
	r.reflect(hjpUnk8, "hjpUnk8");
	CKHkSquadEnemy::reflectMembers2(r, kenv);
}

void CKHkMobileTower::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(hmtObjs1, "hmtObjs1");
	r.reflect(hmtUnk1, "hmtUnk1");
	r.reflect(hmtUnk2, "hmtUnk2");
	r.foreachElement(parts, "parts", [&](Part& part) {
		r.reflect(part.obj, "obj");
		r.reflect(part.byteVal, "byteVal");
		r.reflect(part.fltValues, "fltValues");
		});
	r.reflect(hmtObjs2, "hmtObjs2");
	r.reflect(hmtUnk5, "hmtUnk5");
	r.reflect(hmtUnk6, "hmtUnk6");
	CKHkSquadEnemy::reflectMembers2(r, kenv);
}

void CKHkMobileTower::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	for (auto &ref : hmtObjs1)
		ref = kenv->readObjRef<CKObject>(file);
	hmtUnk1 = file->readUint8();
	for (auto &f : hmtUnk2)
		f = file->readFloat();
	for (Part &part : parts) {
		part.obj = kenv->readObjRef<CKObject>(file);
		part.byteVal = file->readUint8();
		for (auto &f : part.fltValues)
			f = file->readFloat();
	}
	for (auto &ref : hmtObjs2)
		ref = kenv->readObjRef<CKObject>(file);
	for (auto &f : hmtUnk5)
		f = file->readFloat();
	hmtUnk6 = kenv->readObjRef<CKObject>(file);
	CKHkSquadEnemy::deserialize(kenv, file, length);
}

void CKHkMobileTower::serialize(KEnvironment * kenv, File * file)
{
	for (auto &ref : hmtObjs1)
		kenv->writeObjRef(file, ref);
	file->writeUint8(hmtUnk1);
	for (auto &f : hmtUnk2)
		file->writeFloat(f);
	for (Part &part : parts) {
		kenv->writeObjRef(file, part.obj);
		file->writeUint8(part.byteVal);
		for (auto &f : part.fltValues)
			file->writeFloat(f);
	}
	for (auto &ref : hmtObjs2)
		kenv->writeObjRef(file, ref);
	for (auto &f : hmtUnk5)
		file->writeFloat(f);
	kenv->writeObjRef(file, hmtUnk6);
	CKHkSquadEnemy::serialize(kenv, file);
}

void CKHkInterfaceBase::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(uibFlags, "uibFlags");
	r.reflect(uibGrpFrontEnd, "uibGrpFrontEnd");
}
void CKHkInterfaceInGame::reflectMembers(MemberListener &r) {
	CKHkInterfaceBase::reflectMembers(r);
	r.reflect(uiigShields, "uiigShields");
	r.reflect(uiigUnk1, "uiigUnk1");
	r.reflect(uiigUnk2, "uiigUnk2");
	r.reflect(uiigUnk3, "uiigUnk3");
	r.reflect(uiigUnk4, "uiigUnk4");
	r.reflect(uiigUnk5, "uiigUnk5");
	r.reflect(uiigTextBoxes, "uiigTextBoxes");
	r.reflect(uiigSndDict, "uiigSndDict");
	r.reflect(uiigUnk8, "uiigUnk8");
	r.reflect(uiigActionTextIDs, "uiigActionTextIDs");
	r.reflect(uiigUnk10, "uiigUnk10");
	r.reflect(uiigUnk11, "uiigUnk11");
	r.reflect(uiigShieldValues, "uiigShieldValues");
	r.reflect(uiigPotionValues, "uiigPotionValues");
	r.reflect(uiigUnk14, "uiigUnk14");
	r.reflect(uiigUnk15, "uiigUnk15");
	r.reflect(uiigUnk16, "uiigUnk16");
	r.reflect(uiigHelmetValues, "uiigHelmetValues");
	r.reflect(uiigUnk18, "uiigUnk18");
	r.reflect(uiigUnk19, "uiigUnk19");
	r.reflect(uiigUnk20, "uiigUnk20");
	r.reflect(uiigLaurelValues, "uiigLaurelValues");
	r.reflect(uiigUnk22, "uiigUnk22");
	r.reflect(uiigUnk23, "uiigUnk23");
	r.reflect(uiigUnk24, "uiigUnk24");
	r.reflect(uiigUnk25, "uiigUnk25");
	r.reflect(uiigUnk26, "uiigUnk26");
	r.reflect(uiigUnk27, "uiigUnk27");
	r.reflect(uiigUnk28, "uiigUnk28");
	r.reflect(uiigUnk29, "uiigUnk29");
	r.reflect(uiigUnk30, "uiigUnk30");
}

void CKHkInterfaceOption::reflectMembers2(MemberListener & r, KEnvironment *kenv)
{
	CKHkInterfaceBase::reflectMembers(r);
	r.reflect(uiioptUnk0, "uiioptUnk0");
	uiioptUnk1.resize((kenv->isRemaster || (kenv->platform == kenv->PLATFORM_PS2)) ? 15 : 13);
	r.reflect(uiioptUnk1, "uiioptUnk1");
	r.reflect(uiioptUnk2, "uiioptUnk2");
	r.reflect(uiioptUnk3, "uiioptUnk3");
	r.reflect(uiioptUnk4, "uiioptUnk4");
	r.reflect(uiioptUnk5, "uiioptUnk5");
	r.reflect(uiioptUnk6, "uiioptUnk6");
	r.reflect(uiioptUnk7, "uiioptUnk7");
	r.reflect(uiioptUnk8, "uiioptUnk8");
	r.reflect(uiioptUnk9, "uiioptUnk9");
	r.reflect(uiioptUnk10, "uiioptUnk10");
	r.reflect(uiioptUnk11, "uiioptUnk11");
	r.reflect(uiioptUnk12, "uiioptUnk12");
	r.reflect(uiioptUnk13, "uiioptUnk13");
	r.reflect(uiioptUnk14, "uiioptUnk14");
	r.reflect(uiioptUnk15, "uiioptUnk15");
	if (kenv->platform != kenv->PLATFORM_PS2) {
		r.reflect(uiioptUnk16, "uiioptUnk16");
		r.reflect(uiioptUnk17, "uiioptUnk17");
		r.reflect(uiioptUnk18, "uiioptUnk18");
		r.reflect(uiioptUnk19, "uiioptUnk19");
		r.reflect(uiioptUnk20, "uiioptUnk20");
		r.reflect(uiioptUnk21, "uiioptUnk21");
		r.reflect(uiioptUnk22, "uiioptUnk22");
		r.reflect(uiioptUnk23, "uiioptUnk23");
		r.reflect(uiioptUnk24, "uiioptUnk24");
		r.reflect(uiioptUnk25, "uiioptUnk25");
		r.reflect(uiioptUnk26, "uiioptUnk26");
		r.reflect(uiioptUnk27, "uiioptUnk27");
		r.reflect(uiioptUnk28, "uiioptUnk28");
		r.reflect(uiioptUnk29, "uiioptUnk29");
		r.reflect(uiioptUnk30, "uiioptUnk30");
		r.reflect(uiioptUnk31, "uiioptUnk31");
		r.reflect(uiioptUnk32, "uiioptUnk32");
		r.reflect(uiioptUnk33, "uiioptUnk33");
		r.reflect(uiioptUnk34, "uiioptUnk34");
		r.reflect(uiioptUnk35, "uiioptUnk35");
		r.reflect(uiioptUnk36, "uiioptUnk36");
		r.reflect(uiioptUnk37, "uiioptUnk37");
		r.reflect(uiioptUnk38, "uiioptUnk38");
		r.reflect(uiioptUnk39, "uiioptUnk39");
		if (!kenv->isRemaster) {
			r.reflect(uiioptUnk40, "uiioptUnk40");
			r.reflect(uiioptUnk41, "uiioptUnk41");
			r.reflect(uiioptUnk42, "uiioptUnk42");
			r.reflect(uiioptUnk43, "uiioptUnk43");
			r.reflect(uiioptUnk44, "uiioptUnk44");
			r.reflect(uiioptUnk45, "uiioptUnk45");
			r.reflect(uiioptUnk46, "uiioptUnk46");
			r.reflect(uiioptUnk47, "uiioptUnk47");
		}
	}
}

void CKHkParkourSteleAsterix::reflectMembers(MemberListener& r)
{
	CKHook::reflectMembers(r);
	r.reflect(parkUnk0, "parkUnk0");
	r.reflect(parkUnk1, "parkUnk1");
	r.reflect(parkTimeLimit, "parkTimeLimit");
	r.reflect(parkUnk3, "parkUnk3");
	r.reflect(parkUnk4, "parkUnk4");
	r.reflect(parkUnk5, "parkUnk5");
	r.reflect(parkUnk6, "parkUnk6");
	r.reflect(parkOnStopped, "parkOnStopped", this);
	r.reflect(parkOnLaunched, "parkOnLaunched", this);
	r.reflect(parkNode1, "parkNode1");
	r.reflect(parkNode2, "parkNode2");
	r.reflect(parkUnk11, "parkUnk11");
	r.reflect(parkGameType, "parkGameType");
	r.reflect(parkBronzeTime, "parkBronzeTime");
	r.reflect(parkSilverTime, "parkSilverTime");
	r.reflect(parkGoldTime, "parkGoldTime");
	r.reflect(parkId, "parkId");
}

void CKHkBoat::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhbUnk3, "ckhbUnk3");
	r.reflect(ckhbUnk4, "ckhbUnk4");
	r.reflect(ckhbUnk5, "ckhbUnk5");
	r.reflect(ckhbUnk6, "ckhbUnk6");
	r.reflect(ckhbUnk7, "ckhbUnk7");
	r.reflect(ckhbUnk8, "ckhbUnk8");
	r.reflect(ckhbUnk9, "ckhbUnk9");
	r.reflect(ckhbUnk10, "ckhbUnk10");
	r.reflect(ckhbUnk11, "ckhbUnk11");
	r.reflect(ckhbUnk12, "ckhbUnk12");
	r.reflect(ckhbUnk13, "ckhbUnk13");
	r.reflect(ckhbUnk14, "ckhbUnk14");
	r.reflect(ckhbUnk15, "ckhbUnk15");
	r.reflect(ckhbUnk16, "ckhbUnk16");
	r.reflect(ckhbUnk17, "ckhbUnk17");
	r.reflect(ckhbUnk18, "ckhbUnk18");
	r.reflect(ckhbUnk19, "ckhbUnk19");
	r.reflect(ckhbUnk20, "ckhbUnk20");
	r.reflect(ckhbUnk21, "ckhbUnk21");
	r.reflect(ckhbUnk22, "ckhbUnk22");
	r.reflect(ckhbUnk23, "ckhbUnk23");
	r.reflect(ckhbUnk24, "ckhbUnk24");
	r.reflect(ckhbUnk25, "ckhbUnk25");
	r.reflect(ckhbUnk26, "ckhbUnk26");
	r.reflect(ckhbUnk27, "ckhbUnk27");
	r.reflect(ckhbUnk28, "ckhbUnk28");
	r.reflect(ckhbUnk29, "ckhbUnk29");
	r.reflect(ckhbUnk30, "ckhbUnk30");
	r.reflect(ckhbUnk31, "ckhbUnk31");
	r.reflect(ckhbUnk32, "ckhbUnk32");
	r.reflect(ckhbUnk33, "ckhbUnk33");
	r.reflect(ckhbUnk34, "ckhbUnk34");
	r.reflect(ckhbUnk35, "ckhbUnk35");
	r.reflect(ckhbUnk36, "ckhbUnk36");
	r.reflect(ckhbUnk37, "ckhbUnk37");
	r.reflect(ckhbUnk38, "ckhbUnk38");
	r.reflect(ckhbUnk39, "ckhbUnk39");
	r.reflect(ckhbUnk40, "ckhbUnk40");
	r.reflect(ckhbUnk41, "ckhbUnk41");
	r.reflect(ckhbUnk42, "ckhbUnk42");
	r.reflect(ckhbUnk43, "ckhbUnk43");
	r.reflect(ckhbUnk44, "ckhbUnk44");
	r.reflect(ckhbUnk45, "ckhbUnk45");
	r.reflect(ckhbUnk46, "ckhbUnk46");
	r.reflect(ckhbUnk47, "ckhbUnk47");
	r.reflect(ckhbUnk48, "ckhbUnk48");
	r.reflect(ckhbUnk49, "ckhbUnk49");
	r.reflect(ckhbUnk50, "ckhbUnk50");
	r.reflect(ckhbUnk51, "ckhbUnk51");
	r.reflect(ckhbUnk52, "ckhbUnk52");
	r.reflect(ckhbUnk53, "ckhbUnk53");
	r.reflect(ckhbUnk54, "ckhbUnk54");
	r.reflect(ckhbUnk55, "ckhbUnk55");
	r.reflect(ckhbUnk56, "ckhbUnk56");
	r.reflect(ckhbUnk57, "ckhbUnk57");
	r.reflect(ckhbUnk58, "ckhbUnk58");
	r.reflect(ckhbUnk59, "ckhbUnk59");
	r.reflect(ckhbUnk60, "ckhbUnk60");
	r.reflect(ckhbUnk61, "ckhbUnk61");
	r.reflect(ckhbUnk62, "ckhbUnk62");
	r.reflect(ckhbUnk63, "ckhbUnk63");
	r.reflect(ckhbUnk64, "ckhbUnk64");
	r.reflect(ckhbUnk65, "ckhbUnk65");
	r.reflect(ckhbUnk66, "ckhbUnk66");
	r.reflect(ckhbUnk67, "ckhbUnk67");
	r.reflect(ckhbUnk68, "ckhbUnk68");
	r.reflect(ckhbUnk69, "ckhbUnk69", this);
	r.reflect(ckhbUnk70, "ckhbUnk70", this);
	r.reflect(ckhbUnk71, "ckhbUnk71");
	r.reflect(ckhbUnk72, "ckhbUnk72");
	r.reflect(ckhbUnk73, "ckhbUnk73");
	r.reflect(ckhbUnk74, "ckhbUnk74");
	r.reflect(ckhbUnk75, "ckhbUnk75");
	r.reflect(ckhbUnk76, "ckhbUnk76");
	r.reflect(ckhbUnk77, "ckhbUnk77");
	r.reflect(ckhbUnk78, "ckhbUnk78");
	r.reflect(ckhbUnk79, "ckhbUnk79");
	r.reflect(ckhbUnk80, "ckhbUnk80");
	r.reflect(ckhbUnk81, "ckhbUnk81");
	r.reflect(ckhbUnk82, "ckhbUnk82");
	r.reflect(ckhbUnk83, "ckhbUnk83");
};

void CKHkJumpingRoman::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkSquadSeizableEnemy::reflectMembers2(r, kenv);
	r.reflect(ckhjrUnk31, "ckhjrUnk31");
	r.reflect(ckhjrUnk32, "ckhjrUnk32");
	r.reflect(ckhjrUnk33, "ckhjrUnk33");
	r.reflect(ckhjrUnk34, "ckhjrUnk34");
	r.reflect(ckhjrUnk35, "ckhjrUnk35");
	r.reflect(ckhjrUnk36, "ckhjrUnk36");
	r.reflect(ckhjrUnk37, "ckhjrUnk37");
	r.reflect(ckhjrUnk38, "ckhjrUnk38");
	r.reflect(ckhjrUnk39, "ckhjrUnk39");
	r.reflect(ckhjrUnk40, "ckhjrUnk40");
	r.reflect(ckhjrUnk41, "ckhjrUnk41");
	r.reflect(ckhjrUnk42, "ckhjrUnk42");
	r.reflect(ckhjrUnk43, "ckhjrUnk43");
	r.reflect(ckhjrUnk44, "ckhjrUnk44");
	r.reflect(ckhjrUnk45, "ckhjrUnk45");
};

void CKHkTurtle::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkSquadEnemy::reflectMembers2(r, kenv);
	r.reflect(ckhptUnk17, "ckhptUnk17");
	r.reflect(ckhptUnk18, "ckhptUnk18");
	r.reflect(ckhptUnk19, "ckhptUnk19");
	r.reflect(ckhptUnk20, "ckhptUnk20");
	r.reflect(ckhptUnk21, "ckhptUnk21");
	r.reflect(ckhptUnk22, "ckhptUnk22");
	r.reflect(ckhptUnk23, "ckhptUnk23");
	r.reflect(ckhptUnk24, "ckhptUnk24");
	r.reflect(ckhptUnk25, "ckhptUnk25");
	r.reflect(ckhptUnk26, "ckhptUnk26");
	r.reflect(ckhptUnk27, "ckhptUnk27");
	r.reflect(ckhptUnk28, "ckhptUnk28");
	r.reflect(ckhptUnk29, "ckhptUnk29");
	r.reflect(ckhptUnk30, "ckhptUnk30");
	r.reflect(ckhptUnk31, "ckhptUnk31");
	r.reflect(ckhptUnk32, "ckhptUnk32");
	r.reflect(ckhptUnk33, "ckhptUnk33");
	r.reflect(ckhptUnk34, "ckhptUnk34");
	r.reflect(ckhptUnk35, "ckhptUnk35");
	r.reflect(ckhptUnk36, "ckhptUnk36");
	r.reflect(ckhptUnk37, "ckhptUnk37");
	r.reflect(ckhptUnk38, "ckhptUnk38", this);
	r.reflect(ckhptUnk39, "ckhptUnk39", this);
	r.reflect(ckhptUnk40, "ckhptUnk40");
	r.reflect(ckhptUnk41, "ckhptUnk41");
};

void CKHkPyramidalTurtle::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkTurtle::reflectMembers2(r, kenv);
	r.reflect(ckhptUnk0, "ckhptUnk0");
	r.reflect(ckhptUnk1, "ckhptUnk1");
	r.reflect(ckhptUnk2, "ckhptUnk2");
	r.reflect(ckhptUnk3, "ckhptUnk3");
	r.reflect(ckhptUnk4, "ckhptUnk4");
	r.reflect(ckhptUnk5, "ckhptUnk5");
	r.reflect(ckhptUnk6, "ckhptUnk6");
	r.reflect(ckhptUnk7, "ckhptUnk7");
	r.reflect(ckhptUnk8, "ckhptUnk8");
	r.reflect(ckhptUnk9, "ckhptUnk9");
	r.reflect(ckhptUnk10, "ckhptUnk10");
	r.reflect(ckhptUnk11, "ckhptUnk11");
	r.reflect(ckhptUnk12, "ckhptUnk12");
	r.reflect(ckhptUnk13, "ckhptUnk13");
	r.reflect(ckhptUnk14, "ckhptUnk14");
	r.reflect(ckhptUnk15, "ckhptUnk15");
	r.reflect(ckhptUnk16, "ckhptUnk16");
	r.reflect(ckhptUnk17, "ckhptUnk17");
	r.reflect(ckhptUnk18, "ckhptUnk18");
	r.reflect(ckhptUnk19, "ckhptUnk19");
	r.reflect(ckhptUnk20, "ckhptUnk20");
	r.reflect(ckhptUnk21, "ckhptUnk21");
	r.reflect(ckhptUnk22, "ckhptUnk22");
	r.reflect(ckhptUnk23, "ckhptUnk23");
	r.reflect(ckhptUnk24, "ckhptUnk24");
	r.reflect(ckhptUnk25, "ckhptUnk25");
	r.reflect(ckhptUnk26, "ckhptUnk26");
	r.reflect(ckhptUnk27, "ckhptUnk27");
	r.reflect(ckhptUnk28, "ckhptUnk28");
	r.reflect(ckhptUnk29, "ckhptUnk29");
	r.reflect(ckhptUnk30, "ckhptUnk30");
	r.reflect(ckhptUnk31, "ckhptUnk31");
	r.reflect(ckhptUnk32, "ckhptUnk32");
	r.reflect(ckhptUnk33, "ckhptUnk33");
	r.reflect(ckhptUnk34, "ckhptUnk34");
	r.reflect(ckhptUnk35, "ckhptUnk35");
	r.reflect(ckhptUnk36, "ckhptUnk36");
	r.reflect(ckhptUnk37, "ckhptUnk37");
	r.reflect(ckhptUnk38, "ckhptUnk38");
	r.reflect(ckhptUnk39, "ckhptUnk39");
	r.reflect(ckhptUnk40, "ckhptUnk40");
	r.reflect(ckhptUnk41, "ckhptUnk41");
	r.reflect(ckhptUnk42, "ckhptUnk42");
	r.reflect(ckhptUnk43, "ckhptUnk43");
	r.reflect(ckhptUnk44, "ckhptUnk44");
	r.reflect(ckhptUnk45, "ckhptUnk45");
	r.reflect(ckhptUnk46, "ckhptUnk46");
	r.reflect(ckhptUnk47, "ckhptUnk47");
	r.reflect(ckhptUnk48, "ckhptUnk48");
	r.reflect(ckhptUnk49, "ckhptUnk49");
	r.reflect(ckhptUnk50, "ckhptUnk50");
	r.reflect(ckhptUnk51, "ckhptUnk51");
	r.reflect(ckhptUnk52, "ckhptUnk52");
	r.reflect(ckhptUnk53, "ckhptUnk53");
	r.reflect(ckhptUnk54, "ckhptUnk54");
	r.reflect(ckhptUnk55, "ckhptUnk55");
	r.reflect(ckhptUnk56, "ckhptUnk56");
	r.reflect(ckhptUnk57, "ckhptUnk57");
	r.reflect(ckhptUnk58, "ckhptUnk58");
	r.reflect(ckhptUnk59, "ckhptUnk59");
	r.reflect(ckhptUnk60, "ckhptUnk60");
	r.reflect(ckhptUnk61, "ckhptUnk61");
	r.reflect(ckhptUnk62, "ckhptUnk62");
	r.reflect(ckhptUnk63, "ckhptUnk63");
	r.reflect(ckhptUnk64, "ckhptUnk64");
	r.reflect(ckhptUnk65, "ckhptUnk65");
	r.reflect(ckhptUnk66, "ckhptUnk66");
	r.reflect(ckhptUnk67, "ckhptUnk67");
	r.reflect(ckhptUnk68, "ckhptUnk68");
	r.reflect(ckhptUnk69, "ckhptUnk69");
	r.reflect(ckhptUnk70, "ckhptUnk70");
	r.reflect(ckhptUnk71, "ckhptUnk71");
	r.reflect(ckhptUnk72, "ckhptUnk72");
	r.reflect(ckhptUnk73, "ckhptUnk73");
	r.reflect(ckhptUnk74, "ckhptUnk74");
	r.reflect(ckhptUnk75, "ckhptUnk75");
	r.reflect(ckhptUnk76, "ckhptUnk76");
	r.reflect(ckhptUnk77, "ckhptUnk77");
	r.reflect(ckhptUnk78, "ckhptUnk78");
	r.reflect(ckhptUnk79, "ckhptUnk79");
};

void CKHkSquareTurtle::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkTurtle::reflectMembers2(r, kenv);
	r.reflect(ckhstUnk0, "ckhstUnk0");
	r.reflect(ckhstUnk1, "ckhstUnk1");
	r.reflect(ckhstUnk2, "ckhstUnk2");
	r.reflect(ckhstUnk3, "ckhstUnk3");
	r.reflect(ckhstUnk4, "ckhstUnk4");
	r.reflect(ckhstUnk5, "ckhstUnk5");
	r.reflect(ckhstUnk6, "ckhstUnk6");
	r.reflect(ckhstUnk7, "ckhstUnk7");
	r.reflect(ckhstUnk8, "ckhstUnk8");
	r.reflect(ckhstUnk9, "ckhstUnk9");
	r.reflect(ckhstUnk10, "ckhstUnk10");
	r.reflect(ckhstUnk11, "ckhstUnk11");
	r.reflect(ckhstUnk12, "ckhstUnk12");
	r.reflect(ckhstUnk13, "ckhstUnk13");
	r.reflect(ckhstUnk14, "ckhstUnk14");
	r.reflect(ckhstUnk15, "ckhstUnk15");
	r.reflect(ckhstUnk16, "ckhstUnk16");
	r.reflect(ckhstUnk17, "ckhstUnk17");
	r.reflect(ckhstUnk18, "ckhstUnk18");
	r.reflect(ckhstUnk19, "ckhstUnk19");
	r.reflect(ckhstUnk20, "ckhstUnk20");
	r.reflect(ckhstUnk21, "ckhstUnk21");
	r.reflect(ckhstUnk22, "ckhstUnk22");
	r.reflect(ckhstUnk23, "ckhstUnk23");
	r.reflect(ckhstUnk24, "ckhstUnk24");
	r.reflect(ckhstUnk25, "ckhstUnk25");
	r.reflect(ckhstUnk26, "ckhstUnk26");
	r.reflect(ckhstUnk27, "ckhstUnk27");
	r.reflect(ckhstUnk28, "ckhstUnk28");
	r.reflect(ckhstUnk29, "ckhstUnk29");
	r.reflect(ckhstUnk30, "ckhstUnk30");
	r.reflect(ckhstUnk31, "ckhstUnk31");
	r.reflect(ckhstUnk32, "ckhstUnk32");
	r.reflect(ckhstUnk33, "ckhstUnk33");
	r.reflect(ckhstUnk34, "ckhstUnk34");
	r.reflect(ckhstUnk35, "ckhstUnk35");
	r.reflect(ckhstUnk36, "ckhstUnk36");
	r.reflect(ckhstUnk37, "ckhstUnk37");
	r.reflect(ckhstUnk38, "ckhstUnk38");
	r.reflect(ckhstUnk39, "ckhstUnk39");
	r.reflect(ckhstUnk40, "ckhstUnk40");
	r.reflect(ckhstUnk41, "ckhstUnk41");
	r.reflect(ckhstUnk42, "ckhstUnk42");
};

void CKHkDonutTurtle::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkTurtle::reflectMembers2(r, kenv);
	r.reflect(ckhdtUnk0, "ckhdtUnk0");
	r.reflect(ckhdtUnk1, "ckhdtUnk1");
	r.reflect(ckhdtUnk2, "ckhdtUnk2");
	r.reflect(ckhdtUnk3, "ckhdtUnk3");
	r.reflect(ckhdtUnk4, "ckhdtUnk4");
	r.reflect(ckhdtUnk5, "ckhdtUnk5");
	r.reflect(ckhdtUnk6, "ckhdtUnk6");
	r.reflect(ckhdtUnk7, "ckhdtUnk7");
	r.reflect(ckhdtUnk8, "ckhdtUnk8");
	r.reflect(ckhdtUnk9, "ckhdtUnk9");
	r.reflect(ckhdtUnk10, "ckhdtUnk10");
	r.reflect(ckhdtUnk11, "ckhdtUnk11");
	r.reflect(ckhdtUnk12, "ckhdtUnk12");
	r.reflect(ckhdtUnk13, "ckhdtUnk13");
	r.reflect(ckhdtUnk14, "ckhdtUnk14");
	r.reflect(ckhdtUnk15, "ckhdtUnk15");
	r.reflect(ckhdtUnk16, "ckhdtUnk16");
	r.reflect(ckhdtUnk17, "ckhdtUnk17");
	r.reflect(ckhdtUnk18, "ckhdtUnk18");
	r.reflect(ckhdtUnk19, "ckhdtUnk19");
	r.reflect(ckhdtUnk20, "ckhdtUnk20");
	r.reflect(ckhdtUnk21, "ckhdtUnk21");
	r.reflect(ckhdtUnk22, "ckhdtUnk22");
	r.reflect(ckhdtUnk23, "ckhdtUnk23");
	r.reflect(ckhdtUnk24, "ckhdtUnk24");
	r.reflect(ckhdtUnk25, "ckhdtUnk25");
	r.reflect(ckhdtUnk26, "ckhdtUnk26");
	r.reflect(ckhdtUnk27, "ckhdtUnk27");
	r.reflect(ckhdtUnk28, "ckhdtUnk28");
	r.reflect(ckhdtUnk29, "ckhdtUnk29");
	r.reflect(ckhdtUnk30, "ckhdtUnk30");
	r.reflect(ckhdtUnk31, "ckhdtUnk31");
	r.reflect(ckhdtUnk32, "ckhdtUnk32");
	r.reflect(ckhdtUnk33, "ckhdtUnk33");
	r.reflect(ckhdtUnk34, "ckhdtUnk34");
	r.reflect(ckhdtUnk35, "ckhdtUnk35");
	r.reflect(ckhdtUnk36, "ckhdtUnk36");
	r.reflect(ckhdtUnk37, "ckhdtUnk37");
	r.reflect(ckhdtUnk38, "ckhdtUnk38");
	r.reflect(ckhdtUnk39, "ckhdtUnk39");
	r.reflect(ckhdtUnk40, "ckhdtUnk40");
	r.reflect(ckhdtUnk41, "ckhdtUnk41");
	r.reflect(ckhdtUnk42, "ckhdtUnk42");
	r.reflect(ckhdtUnk43, "ckhdtUnk43");
	r.reflect(ckhdtUnk44, "ckhdtUnk44");
	r.reflect(ckhdtUnk45, "ckhdtUnk45");
	r.reflect(ckhdtUnk46, "ckhdtUnk46");
	r.reflect(ckhdtUnk47, "ckhdtUnk47");
	r.reflect(ckhdtUnk48, "ckhdtUnk48");
	r.reflect(ckhdtUnk49, "ckhdtUnk49");
	r.reflect(ckhdtUnk50, "ckhdtUnk50");
	r.reflect(ckhdtUnk51, "ckhdtUnk51");
	r.reflect(ckhdtUnk52, "ckhdtUnk52");
	r.reflect(ckhdtUnk53, "ckhdtUnk53");
	r.reflect(ckhdtUnk54, "ckhdtUnk54");
	r.reflect(ckhdtUnk55, "ckhdtUnk55");
	r.reflect(ckhdtUnk56, "ckhdtUnk56");
	r.reflect(ckhdtUnk57, "ckhdtUnk57");
	r.reflect(ckhdtUnk58, "ckhdtUnk58");
	r.reflect(ckhdtUnk59, "ckhdtUnk59");
	r.reflect(ckhdtUnk60, "ckhdtUnk60");
	r.reflect(ckhdtUnk61, "ckhdtUnk61");
	r.reflect(ckhdtUnk62, "ckhdtUnk62");
	r.reflect(ckhdtUnk63, "ckhdtUnk63");
	r.reflect(ckhdtUnk64, "ckhdtUnk64");
	r.reflect(ckhdtUnk65, "ckhdtUnk65");
	r.reflect(ckhdtUnk66, "ckhdtUnk66");
	r.reflect(ckhdtUnk67, "ckhdtUnk67");
	r.reflect(ckhdtUnk68, "ckhdtUnk68");
	r.reflect(ckhdtUnk69, "ckhdtUnk69");
	r.reflect(ckhdtUnk70, "ckhdtUnk70");
	r.reflect(ckhdtUnk71, "ckhdtUnk71");
	r.reflect(ckhdtUnk72, "ckhdtUnk72");
	r.reflect(ckhdtUnk73, "ckhdtUnk73");
	r.reflect(ckhdtUnk74, "ckhdtUnk74");
	r.reflect(ckhdtUnk75, "ckhdtUnk75");
	r.reflect(ckhdtUnk76, "ckhdtUnk76");
	r.reflect(ckhdtUnk77, "ckhdtUnk77");
	r.reflect(ckhdtUnk78, "ckhdtUnk78");
	r.reflect(ckhdtUnk79, "ckhdtUnk79");
	r.reflect(ckhdtUnk80, "ckhdtUnk80");
	r.reflect(ckhdtUnk81, "ckhdtUnk81");
	r.reflect(ckhdtUnk82, "ckhdtUnk82");
	r.reflect(ckhdtUnk83, "ckhdtUnk83");
	r.reflect(ckhdtUnk84, "ckhdtUnk84");
	r.reflect(ckhdtUnk85, "ckhdtUnk85");
	r.reflect(ckhdtUnk86, "ckhdtUnk86");
	r.reflect(ckhdtUnk87, "ckhdtUnk87");
	r.reflect(ckhdtUnk88, "ckhdtUnk88");
	r.reflect(ckhdtUnk89, "ckhdtUnk89");
	r.reflect(ckhdtUnk90, "ckhdtUnk90");
	r.reflect(ckhdtUnk91, "ckhdtUnk91");
	r.reflect(ckhdtUnk92, "ckhdtUnk92");
	r.reflect(ckhdtUnk93, "ckhdtUnk93");
	r.reflect(ckhdtUnk94, "ckhdtUnk94");
	r.reflect(ckhdtUnk95, "ckhdtUnk95");
	r.reflect(ckhdtUnk96, "ckhdtUnk96");
	r.reflect(ckhdtUnk97, "ckhdtUnk97");
	r.reflect(ckhdtUnk98, "ckhdtUnk98");
	r.reflect(ckhdtUnk99, "ckhdtUnk99");
	r.reflect(ckhdtUnk100, "ckhdtUnk100");
	r.reflect(ckhdtUnk101, "ckhdtUnk101");
	r.reflect(ckhdtUnk102, "ckhdtUnk102");
	r.reflect(ckhdtUnk103, "ckhdtUnk103");
	r.reflect(ckhdtUnk104, "ckhdtUnk104");
	r.reflect(ckhdtUnk105, "ckhdtUnk105");
	r.reflect(ckhdtUnk106, "ckhdtUnk106");
	r.reflect(ckhdtUnk107, "ckhdtUnk107");
	r.reflect(ckhdtUnk108, "ckhdtUnk108");
	r.reflect(ckhdtUnk109, "ckhdtUnk109");
	r.reflect(ckhdtUnk110, "ckhdtUnk110");
	r.reflect(ckhdtUnk111, "ckhdtUnk111");
	r.reflect(ckhdtUnk112, "ckhdtUnk112");
	r.reflect(ckhdtUnk113, "ckhdtUnk113");
	r.reflect(ckhdtUnk114, "ckhdtUnk114");
	r.reflect(ckhdtUnk115, "ckhdtUnk115");
	r.reflect(ckhdtUnk116, "ckhdtUnk116");
	r.reflect(ckhdtUnk117, "ckhdtUnk117");
	r.reflect(ckhdtUnk118, "ckhdtUnk118");
	r.reflect(ckhdtUnk119, "ckhdtUnk119");
	r.reflect(ckhdtUnk120, "ckhdtUnk120");
	r.reflect(ckhdtUnk121, "ckhdtUnk121");
	r.reflect(ckhdtUnk122, "ckhdtUnk122");
	r.reflect(ckhdtUnk123, "ckhdtUnk123");
	r.reflect(ckhdtUnk124, "ckhdtUnk124");
	r.reflect(ckhdtUnk125, "ckhdtUnk125");
	r.reflect(ckhdtUnk126, "ckhdtUnk126");
	r.reflect(ckhdtUnk127, "ckhdtUnk127");
	r.reflect(ckhdtUnk128, "ckhdtUnk128");
	r.reflect(ckhdtUnk129, "ckhdtUnk129");
	r.reflect(ckhdtUnk130, "ckhdtUnk130");
	r.reflect(ckhdtUnk131, "ckhdtUnk131");
	r.reflect(ckhdtUnk132, "ckhdtUnk132");
	r.reflect(ckhdtUnk133, "ckhdtUnk133");
	r.reflect(ckhdtUnk134, "ckhdtUnk134");
};

void CKHkTriangularTurtle::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkTurtle::reflectMembers2(r, kenv);
	r.reflect(ckhttUnk0, "ckhttUnk0");
	r.reflect(ckhttUnk1, "ckhttUnk1");
	r.reflect(ckhttUnk2, "ckhttUnk2");
	r.reflect(ckhttUnk3, "ckhttUnk3");
	r.reflect(ckhttUnk4, "ckhttUnk4");
	r.reflect(ckhttUnk5, "ckhttUnk5");
	r.reflect(ckhttUnk6, "ckhttUnk6");
	r.reflect(ckhttUnk7, "ckhttUnk7");
	r.reflect(ckhttUnk8, "ckhttUnk8");
	r.reflect(ckhttUnk9, "ckhttUnk9");
	r.reflect(ckhttUnk10, "ckhttUnk10");
	r.reflect(ckhttUnk11, "ckhttUnk11");
	r.reflect(ckhttUnk12, "ckhttUnk12");
	r.reflect(ckhttUnk13, "ckhttUnk13");
	r.reflect(ckhttUnk14, "ckhttUnk14");
	r.reflect(ckhttUnk15, "ckhttUnk15");
	r.reflect(ckhttUnk16, "ckhttUnk16");
	r.reflect(ckhttUnk17, "ckhttUnk17");
	r.reflect(ckhttUnk18, "ckhttUnk18");
	r.reflect(ckhttUnk19, "ckhttUnk19");
	r.reflect(ckhttUnk20, "ckhttUnk20");
	r.reflect(ckhttUnk21, "ckhttUnk21");
	r.reflect(ckhttUnk22, "ckhttUnk22");
	r.reflect(ckhttUnk23, "ckhttUnk23");
	r.reflect(ckhttUnk24, "ckhttUnk24");
	r.reflect(ckhttUnk25, "ckhttUnk25");
	r.reflect(ckhttUnk26, "ckhttUnk26");
	r.reflect(ckhttUnk27, "ckhttUnk27");
	r.reflect(ckhttUnk28, "ckhttUnk28");
	r.reflect(ckhttUnk29, "ckhttUnk29");
	r.reflect(ckhttUnk30, "ckhttUnk30");
	r.reflect(ckhttUnk31, "ckhttUnk31");
	r.reflect(ckhttUnk32, "ckhttUnk32");
	r.reflect(ckhttUnk33, "ckhttUnk33");
	r.reflect(ckhttUnk34, "ckhttUnk34");
	r.reflect(ckhttUnk35, "ckhttUnk35");
	r.reflect(ckhttUnk36, "ckhttUnk36");
	r.reflect(ckhttUnk37, "ckhttUnk37");
	r.reflect(ckhttUnk38, "ckhttUnk38");
	r.reflect(ckhttUnk39, "ckhttUnk39");
	r.reflect(ckhttUnk40, "ckhttUnk40");
	r.reflect(ckhttUnk41, "ckhttUnk41");
	r.reflect(ckhttUnk42, "ckhttUnk42");
	r.reflect(ckhttUnk43, "ckhttUnk43");
	r.reflect(ckhttUnk44, "ckhttUnk44");
	r.reflect(ckhttUnk45, "ckhttUnk45");
	r.reflect(ckhttUnk46, "ckhttUnk46");
	r.reflect(ckhttUnk47, "ckhttUnk47");
	r.reflect(ckhttUnk48, "ckhttUnk48");
	r.reflect(ckhttUnk49, "ckhttUnk49");
	r.reflect(ckhttUnk50, "ckhttUnk50");
	r.reflect(ckhttUnk51, "ckhttUnk51");
	r.reflect(ckhttUnk52, "ckhttUnk52");
	r.reflect(ckhttUnk53, "ckhttUnk53");
	r.reflect(ckhttUnk54, "ckhttUnk54");
	r.reflect(ckhttUnk55, "ckhttUnk55");
	r.reflect(ckhttUnk56, "ckhttUnk56");
	r.reflect(ckhttUnk57, "ckhttUnk57");
	r.reflect(ckhttUnk58, "ckhttUnk58");
	r.reflect(ckhttUnk59, "ckhttUnk59");
	r.reflect(ckhttUnk60, "ckhttUnk60");
	r.reflect(ckhttUnk61, "ckhttUnk61");
};

void CKHkBoss::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhbUnk3, "ckhbUnk3");
	r.reflect(ckhbUnk4, "ckhbUnk4");
	r.reflect(ckhbUnk5, "ckhbUnk5");
	r.reflect(ckhbUnk6, "ckhbUnk6");
	r.reflect(ckhbUnk7, "ckhbUnk7");
	r.reflect(ckhbUnk8, "ckhbUnk8");
	r.reflect(ckhbUnk9, "ckhbUnk9");
	r.reflect(ckhbUnk10, "ckhbUnk10");
	r.reflect(ckhbUnk11, "ckhbUnk11");
	r.reflect(ckhbUnk12, "ckhbUnk12");
	r.reflect(ckhbUnk13, "ckhbUnk13");
	r.reflect(ckhbUnk14, "ckhbUnk14");
	r.reflect(ckhbUnk15, "ckhbUnk15");
	r.reflect(ckhbUnk16, "ckhbUnk16");
	r.reflect(ckhbUnk17, "ckhbUnk17");
	r.reflect(ckhbUnk18, "ckhbUnk18");
	r.reflect(ckhbUnk19, "ckhbUnk19");
	r.reflect(ckhbUnk20, "ckhbUnk20");
	r.reflect(ckhbUnk21, "ckhbUnk21");
	r.reflect(ckhbUnk22, "ckhbUnk22");
	r.reflect(ckhbUnk23, "ckhbUnk23");
	r.reflect(ckhbUnk24, "ckhbUnk24");
	r.reflect(ckhbUnk25, "ckhbUnk25");
	r.reflect(ckhbUnk26, "ckhbUnk26");
	r.reflect(ckhbUnk27, "ckhbUnk27");
	r.reflect(ckhbUnk28, "ckhbUnk28");
	r.reflect(ckhbUnk29, "ckhbUnk29");
	r.reflect(ckhbUnk30, "ckhbUnk30");
	r.reflect(ckhbUnk31, "ckhbUnk31");
	r.reflect(ckhbUnk32, "ckhbUnk32");
	r.reflect(ckhbUnk33, "ckhbUnk33");
	r.reflect(ckhbUnk34, "ckhbUnk34");
	r.reflect(ckhbUnk35, "ckhbUnk35");
	r.reflect(ckhbUnk36, "ckhbUnk36");
	r.reflect(ckhbUnk37, "ckhbUnk37");
	r.reflect(ckhbUnk38, "ckhbUnk38");
	r.reflect(ckhbUnk39, "ckhbUnk39");
	r.reflect(ckhbUnk40, "ckhbUnk40");
	r.reflect(ckhbUnk41, "ckhbUnk41");
	r.reflect(ckhbUnk42, "ckhbUnk42");
	r.reflect(ckhbUnk43, "ckhbUnk43");
	r.reflect(ckhbUnk44, "ckhbUnk44");
	r.reflect(ckhbUnk45, "ckhbUnk45");
	r.reflect(ckhbUnk46, "ckhbUnk46");
	r.reflect(ckhbUnk47, "ckhbUnk47");
	r.reflect(ckhbUnk48, "ckhbUnk48");
	r.reflect(ckhbUnk49, "ckhbUnk49");
	r.reflect(ckhbUnk50, "ckhbUnk50");
	r.reflect(ckhbUnk51, "ckhbUnk51");
	r.reflect(ckhbUnk52, "ckhbUnk52");
	r.reflect(ckhbUnk53, "ckhbUnk53");
	r.reflect(ckhbUnk54, "ckhbUnk54");
	r.reflect(ckhbUnk55, "ckhbUnk55");
	r.reflect(ckhbUnk56, "ckhbUnk56");
	r.reflect(ckhbUnk57, "ckhbUnk57");
	r.reflect(ckhbUnk58, "ckhbUnk58");
	r.reflect(ckhbUnk59, "ckhbUnk59");
	r.reflect(ckhbUnk60, "ckhbUnk60");
	r.reflect(ckhbUnk61, "ckhbUnk61");
	r.reflect(ckhbUnk62, "ckhbUnk62");
	r.reflect(ckhbUnk63, "ckhbUnk63");
	r.reflect(ckhbUnk64, "ckhbUnk64");
	r.reflect(ckhbUnk65, "ckhbUnk65");
	r.reflect(ckhbUnk66, "ckhbUnk66");
	r.reflect(ckhbUnk67, "ckhbUnk67");
	r.reflect(ckhbUnk68, "ckhbUnk68");
	r.reflect(ckhbUnk69, "ckhbUnk69");
	for (auto& thing : ckhbSomethings) {
		r.reflect(thing.ckhbUnk70, "ckhbUnk70");
		r.reflect(thing.ckhbUnk71, "ckhbUnk71");
		r.reflect(thing.ckhbUnk72, "ckhbUnk72");
		r.reflect(thing.ckhbUnk73, "ckhbUnk73");
		r.reflect(thing.ckhbUnk74, "ckhbUnk74");
	}
	r.reflect(ckhbUnk110, "ckhbUnk110", this);
	r.reflect(ckhbUnk111, "ckhbUnk111", this);
	r.reflect(ckhbUnk112, "ckhbUnk112", this);
	r.reflect(ckhbUnk113, "ckhbUnk113", this);
	r.reflect(ckhbUnk114, "ckhbUnk114", this);
	r.reflect(ckhbUnk115, "ckhbUnk115", this);
	r.reflect(ckhbUnk116, "ckhbUnk116");
	r.reflect(ckhbUnk117, "ckhbUnk117");
	r.reflect(ckhbUnk118, "ckhbUnk118");
	r.reflect(ckhbUnk119, "ckhbUnk119");
	r.reflect(ckhbUnk120, "ckhbUnk120");
	r.reflect(ckhbUnk121, "ckhbUnk121");
	r.reflect(ckhbUnk122, "ckhbUnk122");
	r.reflect(ckhbUnk123, "ckhbUnk123");
	r.reflect(ckhbUnk124, "ckhbUnk124");
	r.reflect(ckhbUnk125, "ckhbUnk125");
	r.reflect(ckhbUnk126, "ckhbUnk126");
	r.reflect(ckhbUnk127, "ckhbUnk127");
	r.reflect(ckhbUnk128, "ckhbUnk128");
	r.reflect(ckhbUnk129, "ckhbUnk129");
};

void CKHkTrioCatapult::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhtcUnk0, "ckhtcUnk0");
	r.reflect(ckhtcUnk1, "ckhtcUnk1");
	r.reflect(ckhtcUnk2, "ckhtcUnk2");
	r.reflect(ckhtcUnk3, "ckhtcUnk3");
	r.reflect(ckhtcUnk4, "ckhtcUnk4");
	r.reflect(ckhtcUnk5, "ckhtcUnk5");
	r.reflect(ckhtcUnk6, "ckhtcUnk6");
	r.reflect(ckhtcUnk7, "ckhtcUnk7");
	r.reflect(ckhtcUnk8, "ckhtcUnk8");
	r.reflect(ckhtcUnk9, "ckhtcUnk9");
	r.reflect(ckhtcUnk10, "ckhtcUnk10");
	r.reflect(ckhtcUnk11, "ckhtcUnk11", this);
	r.reflect(ckhtcUnk12, "ckhtcUnk12", this);
	r.reflect(ckhtcUnk13, "ckhtcUnk13", this);
	r.reflect(ckhtcUnk14, "ckhtcUnk14", this);
	r.reflect(ckhtcUnk15, "ckhtcUnk15");
	r.reflect(ckhtcUnk16, "ckhtcUnk16");
	r.reflect(ckhtcSector, "ckhtcSector");
};

void CKHkObelixCatapult::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhocUnk0, "ckhocUnk0");
	r.reflect(ckhocUnk1, "ckhocUnk1");
	r.reflect(ckhocUnk2, "ckhocUnk2");
	r.reflect(ckhocUnk3, "ckhocUnk3");
	r.reflect(ckhocUnk4, "ckhocUnk4");
	r.reflect(ckhocUnk5, "ckhocUnk5");
	r.reflect(ckhocUnk6, "ckhocUnk6");
};

void CKHkRomanArcher::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkSquadSeizableEnemy::reflectMembers2(r, kenv);
	r.reflect(ckhraUnk0, "ckhraUnk0");
	r.reflect(ckhraUnk1, "ckhraUnk1");
	r.reflect(ckhraUnk2, "ckhraUnk2");
	r.reflect(ckhraUnk3, "ckhraUnk3");
	r.reflect(ckhraUnk4, "ckhraUnk4");
	r.reflect(ckhraUnk5, "ckhraUnk5");
	r.reflect(ckhraUnk6, "ckhraUnk6");
	r.reflect(ckhraUnk7, "ckhraUnk7");
	r.reflect(ckhraUnk8, "ckhraUnk8");
	r.reflect(ckhraUnk9, "ckhraUnk9");
	r.reflect(ckhraUnk10, "ckhraUnk10");
	r.reflect(ckhraUnk11, "ckhraUnk11");
	r.reflect(ckhraUnk12, "ckhraUnk12");
	r.reflect(ckhraUnk13, "ckhraUnk13");
	r.reflect(ckhraUnk14, "ckhraUnk14");
	r.reflect(ckhraUnk15, "ckhraUnk15");
	r.reflect(ckhraUnk16, "ckhraUnk16");
	r.reflect(ckhraUnk17, "ckhraUnk17");
};
void CKHkCrate::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhcUnk0, "ckhcUnk0");
	r.reflect(ckhcUnk1, "ckhcUnk1");
};

void CKHkCatapult::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhcUnk0, "ckhcUnk0");
	r.reflect(ckhcUnk1, "ckhcUnk1");
	r.reflect(ckhcUnk2, "ckhcUnk2");
	r.reflect(ckhcUnk3, "ckhcUnk3");
	r.reflect(ckhcUnk4, "ckhcUnk4");
	r.reflect(ckhcUnk5, "ckhcUnk5");
	r.reflect(ckhcUnk6, "ckhcUnk6");
	r.reflect(ckhcUnk7, "ckhcUnk7");
	r.reflect(ckhcUnk8, "ckhcUnk8");
	r.reflect(ckhcUnk9, "ckhcUnk9");
	r.reflect(ckhcUnk10, "ckhcUnk10");
	r.reflect(ckhcUnk11, "ckhcUnk11");
	r.reflect(ckhcUnk12, "ckhcUnk12");
	r.reflect(ckhcUnk13, "ckhcUnk13");
	r.reflect(ckhcUnk14, "ckhcUnk14");
	r.reflect(ckhcUnk15, "ckhcUnk15");
	r.reflect(ckhcUnk16, "ckhcUnk16");
	r.reflect(ckhcUnk17, "ckhcUnk17");
	r.reflect(ckhcUnk18, "ckhcUnk18");
	r.reflect(ckhcUnk19, "ckhcUnk19");
	r.reflect(ckhcUnk20, "ckhcUnk20");
	r.reflect(ckhcUnk21, "ckhcUnk21");
	r.reflect(ckhcUnk22, "ckhcUnk22");
	r.reflect(ckhcUnk23, "ckhcUnk23");
	r.reflect(ckhcUnk24, "ckhcUnk24");
	r.reflect(ckhcUnk25, "ckhcUnk25");
	r.reflect(ckhcUnk26, "ckhcUnk26");
	r.reflect(ckhcUnk27, "ckhcUnk27");
	r.reflect(ckhcUnk28, "ckhcUnk28");
	r.reflect(ckhcUnk29, "ckhcUnk29");
	r.reflect(ckhcUnk30, "ckhcUnk30");
	r.reflect(ckhcUnk31, "ckhcUnk31");
	r.reflect(ckhcUnk32, "ckhcUnk32");
	r.reflect(ckhcUnk33, "ckhcUnk33");
	r.reflect(ckhcUnk34, "ckhcUnk34");
	r.reflect(ckhcUnk35, "ckhcUnk35");
	r.reflect(ckhcUnk36, "ckhcUnk36");
	r.reflect(ckhcUnk37, "ckhcUnk37");
	r.reflect(ckhcUnk38, "ckhcUnk38");
	r.reflect(ckhcUnk39, "ckhcUnk39");
	r.reflect(ckhcUnk40, "ckhcUnk40");
	r.reflect(ckhcUnk41, "ckhcUnk41");
	r.reflect(ckhcUnk42, "ckhcUnk42");
};
void CKHkInterfaceEvolution::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhieUnk0, "ckhieUnk0");
	r.reflect(ckhieUnk1, "ckhieUnk1");
};
void CKHkInterfacePause::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhipUnk0, "ckhipUnk0");
	r.reflect(ckhipUnk1, "ckhipUnk1");
	r.reflect(ckhipUnk2, "ckhipUnk2");
	r.reflect(ckhipUnk3, "ckhipUnk3");
	r.reflect(ckhipUnk4, "ckhipUnk4");
	r.reflect(ckhipUnk5, "ckhipUnk5");
};
void CKHkInterfaceLoadSave::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhilsUnk0, "ckhilsUnk0");
	r.reflect(ckhilsUnk1, "ckhilsUnk1");
	r.reflect(ckhilsUnk2, "ckhilsUnk2");
	r.reflect(ckhilsUnk3, "ckhilsUnk3");
	r.reflect(ckhilsUnk4, "ckhilsUnk4");
	r.reflect(ckhilsUnk5, "ckhilsUnk5");
	r.reflect(ckhilsUnk6, "ckhilsUnk6");
	r.reflect(ckhilsUnk7, "ckhilsUnk7");
	r.reflect(ckhilsUnk8, "ckhilsUnk8");
	r.reflect(ckhilsUnk9, "ckhilsUnk9");
	r.reflect(ckhilsUnk10, "ckhilsUnk10");
	r.reflect(ckhilsUnk11, "ckhilsUnk11");
};
void CKHkInterfaceMain::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhimUnk0, "ckhimUnk0");
	r.reflect(ckhimUnk1, "ckhimUnk1");
	r.reflect(ckhimUnk2, "ckhimUnk2");
	r.reflect(ckhimUnk3, "ckhimUnk3");
	r.reflect(ckhimUnk4, "ckhimUnk4");
	r.reflect(ckhimUnk5, "ckhimUnk5");
	r.reflect(ckhimUnk6, "ckhimUnk6");
	r.reflect(ckhimUnk7, "ckhimUnk7");
	r.reflect(ckhimUnk8, "ckhimUnk8");
	r.reflect(ckhimUnk9, "ckhimUnk9");
	r.reflect(ckhimUnk10, "ckhimUnk10");
	r.reflect(ckhimUnk11, "ckhimUnk11");
	r.reflect(ckhimUnk12, "ckhimUnk12");
	r.reflect(ckhimUnk13, "ckhimUnk13");
	r.reflect(ckhimUnk14, "ckhimUnk14");
};
void CKHkInterfaceOpening::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhioUnk0, "ckhioUnk0");
	r.reflect(ckhioUnk1, "ckhioUnk1");
	r.reflect(ckhioUnk2, "ckhioUnk2");
	r.reflect(ckhioUnk3, "ckhioUnk3");
	r.reflect(ckhioUnk4, "ckhioUnk4");
	r.reflect(ckhioUnk5, "ckhioUnk5");
	r.reflect(ckhioUnk6, "ckhioUnk6");
	if(kenv->platform != kenv->PLATFORM_PS2 && !kenv->isRemaster)
		r.reflect(ckhioNintendo, "ckhioNintendo");
};
void CKHkInterfaceCloth::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhicUnk0, "ckhicUnk0");
	r.reflect(ckhicUnk1, "ckhicUnk1");
	r.reflect(ckhicUnk2, "ckhicUnk2");
	r.reflect(ckhicUnk3, "ckhicUnk3");
	r.reflect(ckhicUnk4, "ckhicUnk4");
	r.reflect(ckhicUnk5, "ckhicUnk5");
	r.reflect(ckhicUnk6, "ckhicUnk6");
	r.reflect(ckhicUnk7, "ckhicUnk7");
	r.reflect(ckhicUnk8, "ckhicUnk8");
	r.reflect(ckhicUnk9, "ckhicUnk9");
};
void CKHkInterfaceGallery::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkInterfaceBase::reflectMembers2(r, kenv);
	r.reflect(ckhigUnk0, "ckhigUnk0");
	r.reflect(ckhigUnk1, "ckhigUnk1");
	r.reflect(ckhigUnk2, "ckhigUnk2");
	r.reflect(ckhigUnk3, "ckhigUnk3");
	r.reflect(ckhigUnk4, "ckhigUnk4");
	r.reflect(ckhigUnk5, "ckhigUnk5");
	r.reflect(ckhigUnk6, "ckhigUnk6");
	r.reflect(ckhigUnk7, "ckhigUnk7");
	r.reflect(ckhigUnk8, "ckhigUnk8");
	r.reflect(ckhigUnk9, "ckhigUnk9");
	r.reflect(ckhigUnk10, "ckhigUnk10");
	r.reflect(ckhigUnk11, "ckhigUnk11");
	r.reflect(ckhigUnk12, "ckhigUnk12");
	r.reflect(ckhigUnk13, "ckhigUnk13");
	r.reflect(ckhigUnk14, "ckhigUnk14");
	r.reflect(ckhigUnk15, "ckhigUnk15");
	r.reflect(ckhigUnk16, "ckhigUnk16");
	r.reflect(ckhigUnk17, "ckhigUnk17");
	r.reflect(ckhigUnk18, "ckhigUnk18");
	r.reflect(ckhigUnk19, "ckhigUnk19");
	r.reflect(ckhigUnk20, "ckhigUnk20");
	r.reflect(ckhigUnk21, "ckhigUnk21");
	r.reflect(ckhigUnk22, "ckhigUnk22");
};
void CKHkWater::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhwUnk0, "ckhwUnk0");
	r.reflect(ckhwUnk1, "ckhwUnk1");
	r.reflect(ckhwSizeX, "ckhwSizeX");
	r.reflect(ckhwSizeZ, "ckhwSizeZ");
	r.reflect(ckhwUnk4, "ckhwUnk4");
	r.reflect(ckhwUnk5, "ckhwUnk5");
	r.reflect(ckhwUnk6, "ckhwUnk6");
	r.reflect(ckhwUnk7, "ckhwUnk7");
	r.reflect(ckhwUnk8, "ckhwUnk8");
	r.reflect(ckhwUnk9, "ckhwUnk9");
	r.reflect(ckhwUnk10, "ckhwUnk10");
	r.reflect(ckhwUnk11, "ckhwUnk11");
	r.reflect(ckhwUnk12, "ckhwUnk12");
	r.reflect(ckhwUnk13, "ckhwUnk13");
	r.reflect(ckhwUnk14, "ckhwUnk14");
	r.reflect(ckhwUnk15, "ckhwUnk15");
	r.reflect(ckhwUnk16, "ckhwUnk16");
	r.reflect(ckhwUnk17, "ckhwUnk17");
	r.reflect(ckhwUnk18, "ckhwUnk18");
	r.reflect(ckhwUnk19, "ckhwUnk19");
	r.reflect(ckhwUnk20, "ckhwUnk20");
	r.reflect(ckhwUnk21, "ckhwUnk21");
	r.reflect(ckhwUnk22, "ckhwUnk22");
	r.reflect(ckhwUnk23, "ckhwUnk23");
	r.reflect(ckhwUnk24, "ckhwUnk24");
	r.reflectSize<uint32_t>(ckhwGrounds, "size_ckhwGrounds");
	r.reflect(ckhwGrounds, "ckhwGrounds");
	r.reflect(ckhwSectorsBitArray, "ckhwSectorsBitArray");
	r.reflect(ckhwUnk30, "ckhwUnk30");
	r.enterArray("ckhwDings");
	for (auto& ding : ckhwDings) {
		r.enterStruct("ckhwDings");
		r.reflect(ding.wdUnk1, "wdUnk1");
		if (ding.wdUnk1 == 0)
			r.reflect(ding.wdNode, "wdNode");
		r.reflect(ding.wdUnk2, "wdUnk2");
		r.reflect(ding.wdUnk3, "wdUnk3");
		r.reflect(ding.wdUnk4, "wdUnk4");
		r.reflect(ding.wdUnk5, "wdUnk5");
		r.leaveStruct();
		r.incrementIndex();
	}
	r.leaveArray();
};
void CKHkWaterFx::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHook::reflectMembers2(r, kenv);
	r.reflect(ckhwfUnk0, "ckhwfUnk0");
	r.reflect(ckhwfUnk1, "ckhwfUnk1");
	r.reflect(ckhwfUnk2, "ckhwfUnk2");
	r.reflect(ckhwfUnk3, "ckhwfUnk3");
	r.reflect(ckhwfUnk4, "ckhwfUnk4");
	r.reflect(ckhwfUnk5, "ckhwfUnk5");
	r.reflect(ckhwfUnk6, "ckhwfUnk6");
	r.reflect(ckhwfUnk7, "ckhwfUnk7");
	r.reflect(ckhwfUnk8, "ckhwfUnk8");
	r.reflect(ckhwfUnk9, "ckhwfUnk9");
	r.reflect(ckhwfUnk10, "ckhwfUnk10");
	r.reflect(ckhwfUnk11, "ckhwfUnk11");
	r.reflect(ckhwfUnk12, "ckhwfUnk12");
	r.reflect(ckhwfUnk13, "ckhwfUnk13");
	r.reflect(ckhwfUnk14, "ckhwfUnk14");
	r.reflect(ckhwfUnk15, "ckhwfUnk15");
	r.reflect(ckhwfUnk16, "ckhwfUnk16");
	r.reflect(ckhwfUnk17, "ckhwfUnk17");
	r.reflect(ckhwfUnk18, "ckhwfUnk18");
	r.reflect(ckhwfUnk19, "ckhwfUnk19");
	r.reflect(ckhwfUnk20, "ckhwfUnk20");
	r.reflect(ckhwfUnk21, "ckhwfUnk21");
	r.reflect(ckhwfUnk22, "ckhwfUnk22");
	r.reflect(ckhwfUnk23, "ckhwfUnk23");
	r.reflect(ckhwfUnk24, "ckhwfUnk24");
	r.reflect(ckhwfUnk25, "ckhwfUnk25");
	r.reflect(ckhwfUnk26, "ckhwfUnk26");
	r.reflect(ckhwfUnk27, "ckhwfUnk27");
	r.reflect(ckhwfUnk28, "ckhwfUnk28");
	r.reflect(ckhwfUnk29, "ckhwfUnk29");
	r.reflect(ckhwfUnk30, "ckhwfUnk30");
	r.reflect(ckhwfUnk31, "ckhwfUnk31");
	r.reflect(ckhwfUnk32, "ckhwfUnk32");
	r.reflect(ckhwfUnk33, "ckhwfUnk33");
	r.reflect(ckhwfValueArray, "ckhwfValueArray");
	r.reflect(ckhwfUnk64, "ckhwfUnk64");
	r.reflect(ckhwfUnk65, "ckhwfUnk65");
	r.reflect(ckhwfUnk66, "ckhwfUnk66");
	r.reflect(ckhwfUnk67, "ckhwfUnk67");
	r.reflect(ckhwfUnk68, "ckhwfUnk68");
	r.reflect(ckhwfUnk69, "ckhwfUnk69");
	r.reflect(ckhwfUnk70, "ckhwfUnk70");
	r.reflect(ckhwfUnk71, "ckhwfUnk71");
	r.reflect(ckhwfUnk72, "ckhwfUnk72");
	r.reflect(ckhwfUnk73, "ckhwfUnk73");
	r.reflect(ckhwfUnk74, "ckhwfUnk74");
	r.reflect(ckhwfUnk75, "ckhwfUnk75");
	r.reflect(ckhwfUnk76, "ckhwfUnk76");
	r.reflect(ckhwfUnk77, "ckhwfUnk77");
	r.reflect(ckhwfUnk78, "ckhwfUnk78");
	r.reflect(ckhwfUnk79, "ckhwfUnk79");
	r.reflect(ckhwfUnk80, "ckhwfUnk80");
	r.reflect(ckhwfUnk81, "ckhwfUnk81");
	r.reflect(ckhwfUnk82, "ckhwfUnk82");
	r.reflect(ckhwfUnk83, "ckhwfUnk83");
	r.reflect(ckhwfUnk84, "ckhwfUnk84");
	r.reflect(ckhwfUnk85, "ckhwfUnk85");
	r.reflect(ckhwfUnk86, "ckhwfUnk86");
	r.reflect(ckhwfUnk87, "ckhwfUnk87");
	r.reflect(ckhwfUnk88, "ckhwfUnk88");
	r.reflect(ckhwfUnk89, "ckhwfUnk89");
	r.reflect(ckhwfUnk90, "ckhwfUnk90");
	r.reflect(ckhwfUnk91, "ckhwfUnk91");
	r.reflect(ckhwfUnk92, "ckhwfUnk92");
};
void CKHkBasicEnemyLeader::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKHkBasicEnemy::reflectMembers2(r, kenv);
	r.reflect(ckhbelUnk0, "ckhbelUnk0");
	r.reflect(ckhbelUnk1, "ckhbelUnk1");
};
