#include "CKGroup.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKHook.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKComponent.h"
#include "CKDictionary.h"

void CKGroup::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.setNextFlags(MemberListener::MemberFlags::MF_HOOK_INTERNAL);
	if (kenv->version < kenv->KVERSION_XXL2) {
		r.reflect(nextGroup, "nextGroup");
		r.reflect(parentGroup, "parentGroup");
		r.reflect(life, "life");
		r.reflect(bundle, "bundle");
		r.reflect(unk2, "unk2");
		r.reflect(childGroup, "childGroup");
		r.reflect(childHook, "childHook");
	}
	else {
		r.reflect(x2UnkA, "x2UnkA");
		r.reflect(unk2, "unk2");
		uint32_t x2ref = 0xFFFFFFFF;
		r.reflect(x2ref, "x2ref");
		assert(x2ref == 0xFFFFFFFF);
		r.reflect(nextGroup, "nextGroup");
		r.reflect(parentGroup, "parentGroup");
		r.reflect(life, "life");
		r.reflect(bundle, "bundle");
		r.reflect(childGroup, "childGroup");
		r.reflect(childHook, "childHook");
	}
	r.setNextFlags(MemberListener::MemberFlags::MF_NONE);
}

void CKGroup::addHook(CKHook* hook)
{
	hook->next = this->childHook;
	this->childHook = hook;
	if (hook->life) {
		const bool disabled = hook->life->unk1 & 1;
		auto& lifeLinkedList = disabled ? this->bundle->otherHookLife : this->bundle->firstHookLife;
		hook->life->nextLife = lifeLinkedList;
		lifeLinkedList = hook->life;
	}

	// XXL2+
	if (this->bundle) {
		// TODO: if disabled use "other" hook pointer
		hook->x2next = this->bundle->firstHook;
		this->bundle->firstHook = hook;
	}
}

void CKGroup::addGroup(CKGroup* group)
{
	group->nextGroup = this->childGroup;
	this->childGroup = group;
	group->parentGroup = this;
}

void CKGroup::removeGroup(CKGroup* group)
{
	bool found = false;
	if (childGroup.get() == group) {
		childGroup = group->nextGroup;
		found = true;
	}
	else {
		for (CKGroup* g = childGroup.get(); g; g = g->nextGroup.get()) {
			if (g->nextGroup.get() == group) {
				g->nextGroup = group->nextGroup;
				found = true;
				break;
			}
		}
	}
	if (found) {
		group->nextGroup = nullptr;
		group->parentGroup = nullptr;
	}
}

void CKGroupLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	unk = kenv->readObjRef<CKObject>(file);
	group = kenv->readObjRef<CKGroup>(file);
	unk2 = file->readUint32();
}

void CKGroupLife::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, unk);
	kenv->writeObjRef(file, group);
	file->writeUint32(unk2);
}

void CKGrpBonusPool::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(bonusType, "bonusType");
	r.reflect(handlerId, "handlerId");
	r.reflect(maxBeaconBonusesOnScreen, "maxBeaconBonusesOnScreen");
	if (kenv->version >= kenv->KVERSION_XXL2)
		r.reflect(x2UnkFlt, "x2UnkFlt");
	if (kenv->version >= kenv->KVERSION_ARTHUR)
		r.reflect(arUnkByte, "arUnkByte");
	r.reflect(unk3, "unk3");
	r.reflect(unk4, "unk4");
	//assert(!unk3 && !unk4);
	r.reflect(nextBonusHook, "nextBonusHook");
	r.reflect(bonusCpnt, "bonusCpnt");
	r.reflect(particleNode1, "particleNode1");
	r.reflect(particleNode2, "particleNode2");
	if (kenv->version < kenv->KVERSION_XXL2)
		r.reflect(secondBonusCpnt, "secondBonusCpnt");
	if (kenv->version >= kenv->KVERSION_OLYMPIC)
		r.reflect(ogSekensLauncherCpnt, "ogSekensLauncherCpnt");
}

void CKGrpBaseSquad::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(bsUnk1, "bsUnk1");
	r.reflect(msgAction, "msgAction");
}

void CKGrpSquad::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGrpBaseSquad::reflectMembers2(r, kenv);
	r.enterArray("matrices");
	for (auto mat : { &mat1, &mat2 }) {
		r.enterArray("mat");
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				r.reflect(mat->m[i][j], "mval");
				r.incrementIndex();
			}
		}
		r.leaveArray();
		r.incrementIndex();
	}
	r.leaveArray();
	r.reflect(sqUnk1, "sqUnk1");
	r.reflect(sqUnk2, "sqUnk2");
	r.reflect(sqBizObj1, "sqBizObj1");
	r.reflect(sqBizMarker1, "sqBizMarker1");
	r.reflect(sqBizObj2, "sqBizObj2");
	r.reflect(sqBizMarker2, "sqBizMarker2");
	r.reflect(sqUnk3, "sqUnk3");
	r.reflect(sqUnk4, "sqUnk4");
	r.reflect(sqUnk5, "sqUnk5");

	r.reflectSize<uint32_t>(choreographies, "choreographies_size");
	r.reflect(choreographies, "choreographies");
	r.reflectSize<uint32_t>(choreoKeys, "choreoKeys_size");
	r.reflect(choreoKeys, "choreoKeys");

	auto bingRefl = [&](Bing& s) {
		r.reflect(s.markerIndex, "markerIndex");
		r.reflect(s.b, "b");
		};
	r.reflectSize<uint32_t>(guardMarkers, "guardMarkers_size");
	r.foreachElement(guardMarkers, "guardMarkers", bingRefl);
	r.reflectSize<uint32_t>(spawnMarkers, "spawnMarkers_size");
	r.foreachElement(spawnMarkers, "spawnMarkers", bingRefl);
	
	r.reflectSize<uint32_t>(fings, "fings_size");
	r.reflect(fings, "fings");
	r.reflect(sqUnk6, "sqUnk6");
	r.reflect(sqUnk6b, "sqUnk6b");
	r.reflect(sqUnk7, "sqUnk7");
	r.reflect(sqUnk8, "sqUnk8");
	
	r.reflectSize<uint32_t>(pools, "pools_size");
	r.foreachElement(pools, "pools", [&](PoolEntry& pe) {
		r.reflect(pe.pool, "pool");
		r.reflect(pe.cpnt, "cpnt");
		r.reflect(pe.u1, "u1");
		r.reflect(pe.numEnemies, "numEnemies");
		r.reflect(pe.u2, "u2");
		r.reflect(pe.u3, "u3");
		});

	r.reflect(sqUnkA, "sqUnkA", this);
	r.reflect(sqUnkB, "sqUnkB");
	if (kenv->isRemaster)
		r.reflect(sqRomasterValue, "sqRomasterValue");
	r.reflect(sqUnkC, "sqUnkC", this);
}

void CKGrpSquadEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGrpSquad::reflectMembers2(r, kenv);
	r.reflect(seUnk1, "seUnk1");
	r.reflect(seUnk2, "seUnk2");
}

void CKGrpPoolSquad::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	if (kenv->version == kenv->KVERSION_XXL1) {
		r.reflect(somenum, "somenum");
		r.reflect(shadowCpnt, "shadowCpnt");
	}
	else if (kenv->version >= kenv->KVERSION_XXL2) {
		r.reflectSize<uint32_t>(components, "components_size");
		r.reflect(components, "components");
		r.reflect(enemyType, "enemyType");
	}
}

void CKGrpSquadJetPack::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGrpSquadEnemy::reflectMembers2(r, kenv);
	r.reflectSize<uint16_t>(hearths, "hearths_size");
	r.reflect(hearths, "hearths");
	r.reflect(sjpUnk1, "sjpUnk1");
	r.reflect(sjpUnk2, "sjpUnk2");
	r.reflect(sjpUnk3, "sjpUnk3");
	r.reflect(particleNodes, "particleNodes");
}

void CKGrpLight::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(node, "node");
	r.reflect(texname, "texname");
}

void CKGrpSquadX2::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(phases, "size_phases");
	r.foreachElement(phases, "phases", [&](Phase& phase) {
		phase.reflectMembers(r, kenv);
		});
	if (kenv->version == kenv->KVERSION_XXL2) {
		r.reflectSize<uint8_t>(fightData.pools, "size_pools");
		r.foreachElement(fightData.pools, "pools", [&](X2FightData::PoolEntry& p) {
			r.reflect(p.pool, "pool");
			r.reflect(p.componentIndex, "componentIndex");
			r.reflect(p.numEnemies, "numEnemies");
			r.reflect(p.numInitiallySpawned, "numInitiallySpawned");
			});
		r.reflectSize<uint32_t>(fightData.slots, "size_slots");
		r.foreachElement(fightData.slots, "slots", [&](X2FightData::Slot& s) {
			r.reflect(s.pos, "pos");
			r.reflect(s.dir, "dir");
			r.reflect(s.index, "index");
			});
		r.reflectSize<uint32_t>(fightData.slots2, "size_slots2");
		r.foreachElement(fightData.slots2, "slots2", [&](X2FightData::Slot2& s) {
			r.reflect(s.pos, "pos");
			r.reflect(s.dir, "dir");
			r.reflect(s.us1, "us1");
			r.reflect(s.us2, "us2");
			r.reflect(s.us3, "us3");
			r.reflect(s.us4, "us4");
			});
		r.reflectSize<uint32_t>(vecVec, "size_vecVec");
		r.reflectContainer(vecVec, "vecVec");
		r.reflect(x2sqUnk1, "x2sqUnk1");
		r.reflect(x2sqUnk2, "x2sqUnk2");
		r.reflect(x2sqUnk3, "x2sqUnk3");
		r.reflect(x2sqUnk4, "x2sqUnk4");
	}
	else if (kenv->version >= kenv->KVERSION_ARTHUR) {
		r.reflectSize<uint32_t>(ogThings, "size_ogThings");
		r.foreachElement(ogThings, "ogThings", [&](std::vector<OgThing>& vec1) {
			r.reflectSize<uint32_t>(vec1, "size_ogThingsL1");
			r.reflectContainer(vec1, "ogThingsL1");
			});
		r.reflectSize<uint32_t>(ogBytes, "size_ogBytes");
		r.reflectContainer(ogBytes, "ogBytes");
		r.reflect(ogVeryUnk, "ogVeryUnk");
	}
	r.reflectSize<uint32_t>(x2sqObjList1, "size_x2sqObjList1");
	r.reflectContainer(x2sqObjList1, "x2sqObjList1");
	r.reflectSize<uint32_t>(x2sqObjList2, "size_x2sqObjList2");
	r.reflectContainer(x2sqObjList2, "x2sqObjList2");
	r.reflectSize<uint32_t>(x2sqObjList3, "size_x2sqObjList3");
	r.reflectContainer(x2sqObjList3, "x2sqObjList3");
}
