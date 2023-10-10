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
	r.setNextFlags(MemberListener::MemberFlags::MF_EDITOR_INTERNAL);
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
		hook->life->nextLife = this->bundle->firstHookLife;
		this->bundle->firstHookLife = hook->life;
	}

	// XXL2+
	if (this->bundle) {
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
	//r.reflectContainer(phases, "phases");
	for (auto& phase : phases) {
		phase.reflectMembers(r, kenv);
	}
	if (kenv->version == kenv->KVERSION_XXL2) {
		r.reflectSize<uint8_t>(pools, "size_pools");
		r.reflectContainer(pools, "pools");
		r.reflectSize<uint32_t>(slots, "size_slots");
		r.reflectContainer(slots, "slots");
		r.reflectSize<uint32_t>(slots2, "size_slots2");
		r.reflectContainer(slots2, "slots2");
		r.reflectSize<uint32_t>(vecVec, "size_vecVec");
		r.reflectContainer(vecVec, "vecVec");
		r.reflect(x2sqUnk1, "x2sqUnk1");
		r.reflect(x2sqUnk2, "x2sqUnk2");
		r.reflect(x2sqUnk3, "x2sqUnk3");
		r.reflect(x2sqUnk4, "x2sqUnk4");
	}
	else if (kenv->version >= kenv->KVERSION_ARTHUR) {
		r.reflectSize<uint32_t>(ogThings, "size_ogThings");
		for (auto& vec1 : ogThings) {
			r.reflectSize<uint32_t>(vec1, "size_ogThingsL1");
			r.reflectContainer(vec1, "ogThingsL1");
		}
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

void CKGrpMeca::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(ckgmUnk5, "ckgmUnk5");
	r.reflect(ckgmUnk6, "ckgmUnk6");
	r.reflect(ckgmUnk7, "ckgmUnk7");
	r.reflect(ckgmUnk8, "ckgmUnk8");
	r.reflect(ckgmUnk9, "ckgmUnk9");
	r.reflect(ckgmUnk10, "ckgmUnk10");
	r.reflect(ckgmUnk11, "ckgmUnk11");
	r.reflect(ckgmUnk12, "ckgmUnk12");
	r.reflect(ckgmUnk13, "ckgmUnk13");
	r.reflect(ckgmUnk14, "ckgmUnk14");
	r.reflect(ckgmUnk15, "ckgmUnk15");
	r.reflect(ckgmUnk16, "ckgmUnk16");
	r.reflect(ckgmUnk17, "ckgmUnk17");
	r.reflect(ckgmUnk18, "ckgmUnk18");
	r.reflect(ckgmUnk19, "ckgmUnk19");
	r.reflect(ckgmUnk20, "ckgmUnk20");
	r.reflect(ckgmUnk21, "ckgmUnk21");
	r.reflect(ckgmUnk22, "ckgmUnk22");
	r.reflect(ckgmUnk23, "ckgmUnk23");
};

void CKGrpTrio::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(ckgtUnk5, "ckgtUnk5");
	r.reflect(ckgtUnk6, "ckgtUnk6");
	r.reflect(ckgtUnk7, "ckgtUnk7");
	r.reflect(ckgtUnk8, "ckgtUnk8");
	r.reflect(ckgtUnk9, "ckgtUnk9");
	r.reflect(ckgtUnk10, "ckgtUnk10");
	r.reflect(ckgtUnk11, "ckgtUnk11");
	r.reflect(ckgtUnk12, "ckgtUnk12");
	r.reflect(ckgtUnk13, "ckgtUnk13");
	r.reflect(ckgtUnk14, "ckgtUnk14");
	r.reflect(ckgtUnk15, "ckgtUnk15");
	r.reflect(ckgtUnk16, "ckgtUnk16");
	r.reflect(ckgtUnk17, "ckgtUnk17");
	r.reflect(ckgtUnk18, "ckgtUnk18");
	r.reflect(ckgtUnk19, "ckgtUnk19");
	r.reflect(ckgtUnk20, "ckgtUnk20");
	r.reflect(ckgtUnk21, "ckgtUnk21");
	r.reflect(ckgtUnk22, "ckgtUnk22");
	r.reflect(ckgtUnk23, "ckgtUnk23");
	r.reflect(ckgtUnk24, "ckgtUnk24");
	r.reflect(ckgtUnk25, "ckgtUnk25");
	r.reflect(ckgtUnk26, "ckgtUnk26");
	r.reflect(ckgtUnk27, "ckgtUnk27");
	r.reflect(ckgtUnk28, "ckgtUnk28");
	r.reflect(ckgtUnk29, "ckgtUnk29");
	r.reflect(ckgtUnk30, "ckgtUnk30");
	r.reflect(ckgtUnk31, "ckgtUnk31");
	r.reflect(ckgtUnk32, "ckgtUnk32");
	r.reflect(ckgtUnk33, "ckgtUnk33");
	r.reflect(ckgtUnk34, "ckgtUnk34");
	r.reflect(ckgtUnk35, "ckgtUnk35");
	r.reflect(ckgtUnk36, "ckgtUnk36");
	r.reflect(ckgtUnk37, "ckgtUnk37");
	r.reflect(ckgtUnk38, "ckgtUnk38");
	r.reflect(ckgtUnk39, "ckgtUnk39");
	r.reflect(ckgtUnk40, "ckgtUnk40");
	r.reflect(ckgtUnk41, "ckgtUnk41");
	r.reflect(ckgtUnk42, "ckgtUnk42");
	r.reflect(ckgtUnk43, "ckgtUnk43");
	r.reflect(ckgtUnk44, "ckgtUnk44");
	r.reflect(ckgtUnk45, "ckgtUnk45");
	r.reflect(ckgtUnk46, "ckgtUnk46");
	r.reflect(ckgtUnk47, "ckgtUnk47");
	r.reflect(ckgtUnk48, "ckgtUnk48");
	r.reflect(ckgtUnk49, "ckgtUnk49");
	r.reflect(ckgtUnk50, "ckgtUnk50");
	r.reflect(ckgtUnk51, "ckgtUnk51");
	r.reflect(ckgtUnk52, "ckgtUnk52");
	r.reflect(ckgtUnk53, "ckgtUnk53");
	r.reflect(ckgtUnk54, "ckgtUnk54");
	r.reflect(ckgtUnk55, "ckgtUnk55");
	r.reflect(ckgtUnk56, "ckgtUnk56");
	r.reflect(ckgtUnk57, "ckgtUnk57");
	r.reflect(ckgtUnk58, "ckgtUnk58");
	r.reflect(ckgtUnk59, "ckgtUnk59");
	r.reflect(ckgtUnk60, "ckgtUnk60");
	r.reflect(ckgtUnk61, "ckgtUnk61");
	r.reflect(ckgtUnk62, "ckgtUnk62");
	r.reflect(ckgtUnk63, "ckgtUnk63");
	r.reflect(ckgtUnk64, "ckgtUnk64");
	r.reflect(ckgtUnk65, "ckgtUnk65");
	r.reflect(ckgtUnk66, "ckgtUnk66");
	r.reflect(ckgtUnk67, "ckgtUnk67");
	r.reflect(ckgtUnk68, "ckgtUnk68");
	r.reflect(ckgtUnk69, "ckgtUnk69");
	r.reflect(ckgtUnk70, "ckgtUnk70");
	r.reflect(ckgtUnk71, "ckgtUnk71");
	r.reflect(ckgtUnk72, "ckgtUnk72");
	r.reflect(ckgtUnk73, "ckgtUnk73");
	r.reflect(ckgtUnk74, "ckgtUnk74");
	r.reflect(ckgtUnk75, "ckgtUnk75");
	r.reflect(ckgtUnk76, "ckgtUnk76");
	r.reflect(ckgtUnk77, "ckgtUnk77");
	r.reflect(ckgtUnk78, "ckgtUnk78");
	r.reflect(ckgtUnk79, "ckgtUnk79", this);
	r.reflect(ckgtUnk80, "ckgtUnk80", this);
	r.reflect(ckgtUnk81, "ckgtUnk81", this);
	r.reflect(ckgtUnk82, "ckgtUnk82", this);
	r.reflect(ckgtUnk83, "ckgtUnk83", this);
	r.reflect(ckgtUnk84, "ckgtUnk84", this);
	r.reflect(ckgtUnk85, "ckgtUnk85", this);
	r.reflect(ckgtUnk86, "ckgtUnk86", this);
	r.reflect(ckgtUnk87, "ckgtUnk87", this);
	r.reflect(ckgtUnk88, "ckgtUnk88", this);
	r.reflect(ckgtUnk89, "ckgtUnk89", this);
	r.reflect(ckgtUnk90, "ckgtUnk90", this);
	r.reflect(ckgtUnk91, "ckgtUnk91", this);
	r.reflect(ckgtUnk92, "ckgtUnk92", this);
	r.reflect(ckgtUnk93, "ckgtUnk93", this);
	r.reflect(ckgtUnk94, "ckgtUnk94", this);
	r.reflect(ckgtUnk95, "ckgtUnk95", this);
	r.reflect(ckgtUnk96, "ckgtUnk96", this);
	r.reflect(ckgtUnk97, "ckgtUnk97", this);
	r.reflectSize<uint8_t>(ckgtUnk99, "ckgtUnk98");
	r.reflect(ckgtUnk99, "ckgtUnk99");
	r.reflectSize<uint8_t>(ckgtUnk107, "ckgtUnk106");
	r.reflect(ckgtUnk107, "ckgtUnk107");
	r.reflectSize<uint8_t>(ckgtUnk109, "ckgtUnk108");
	r.reflect(ckgtUnk109, "ckgtUnk109");
	r.reflect(ckgtUnk111, "ckgtUnk111");
	r.reflect(ckgtUnk112, "ckgtUnk112");
	r.reflect(ckgtUnk113, "ckgtUnk113");
	r.reflect(ckgtUnk114, "ckgtUnk114");
	r.reflect(ckgtUnk115, "ckgtUnk115");
	r.reflect(ckgtUnk116, "ckgtUnk116");
	r.reflect(ckgtUnk117, "ckgtUnk117");
	r.reflect(ckgtUnk118, "ckgtUnk118");
	r.reflect(ckgtUnk119, "ckgtUnk119");
	r.reflect(ckgtUnk120, "ckgtUnk120");
	r.reflect(ckgtUnk121, "ckgtUnk121");
	r.reflect(ckgtUnk122, "ckgtUnk122");
	r.reflect(ckgtUnk123, "ckgtUnk123");
	r.reflect(ckgtUnk124, "ckgtUnk124");
	r.reflect(ckgtUnk125, "ckgtUnk125");
	r.reflect(ckgtUnk126, "ckgtUnk126");
	r.reflect(ckgtUnk127, "ckgtUnk127");
	r.reflect(ckgtUnk128, "ckgtUnk128");
	r.reflect(ckgtUnk129, "ckgtUnk129");
	r.reflect(ckgtUnk130, "ckgtUnk130");
	r.reflect(ckgtUnk131, "ckgtUnk131");
	r.reflect(ckgtUnk132, "ckgtUnk132");
	r.reflect(ckgtUnk133, "ckgtUnk133");
	r.reflect(ckgtUnk134, "ckgtUnk134");
	r.reflect(ckgtUnk135, "ckgtUnk135");
	r.reflect(ckgtUnk136, "ckgtUnk136");
	r.reflect(ckgtUnk137, "ckgtUnk137");
	r.reflect(ckgtUnk138, "ckgtUnk138");
	r.reflect(ckgtUnk139, "ckgtUnk139");
	r.reflect(ckgtUnk140, "ckgtUnk140");
	r.reflect(ckgtUnk141, "ckgtUnk141");
	r.reflect(ckgtUnk142, "ckgtUnk142");
	r.reflect(ckgtUnk143, "ckgtUnk143");
	r.reflect(ckgtUnk144, "ckgtUnk144");
	r.reflect(ckgtUnk145, "ckgtUnk145");
	r.reflect(ckgtUnk146, "ckgtUnk146");
	r.reflect(ckgtUnk147, "ckgtUnk147");
	r.reflect(ckgtUnk148, "ckgtUnk148");
	r.reflect(ckgtUnk149, "ckgtUnk149");
	r.reflect(ckgtUnk150, "ckgtUnk150");
	r.reflect(ckgtUnk151, "ckgtUnk151");
	r.reflect(ckgtUnk152, "ckgtUnk152");
	r.reflect(ckgtUnk153, "ckgtUnk153");
	r.reflect(ckgtUnk154, "ckgtUnk154");
};

void CKGrpFrontEnd::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(ckgfeSoundDict, "ckgfeSoundDict");
};

void CKGrpCatapult::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(ckgcShadowCpnt, "ckgcShadowCpnt");
}

void CKGrpAsterixCheckpoint::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(astCheckpointValue, "astCheckpointValue");
}

void CKGrpBonusSpitter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(ckgbsUnk5, "ckgbsUnk5");
	r.reflect(ckgbsUnk6, "ckgbsUnk6");
	r.reflect(ckgbsUnk7, "ckgbsUnk7");
	r.reflect(ckgbsUnk8, "ckgbsUnk8");
};

void CKGrpMap::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKGroup::reflectMembers2(r, kenv);
	r.reflect(ckgmUnk5, "ckgmUnk5");
	r.reflect(ckgmUnk6, "ckgmUnk6");
	r.reflect(ckgmUnk7, "ckgmUnk7");
	r.reflect(ckgmUnk8, "ckgmUnk8");
	r.reflect(ckgmUnk9, "ckgmUnk9");
	r.reflect(ckgmUnk10, "ckgmUnk10");
	r.reflect(ckgmUnk11, "ckgmUnk11");
	r.reflect(ckgmUnk12, "ckgmUnk12");
	r.reflect(ckgmUnk13, "ckgmUnk13");
	r.reflect(ckgmUnk14, "ckgmUnk14");
	r.reflect(ckgmUnk15, "ckgmUnk15");
	r.reflect(ckgmUnk16, "ckgmUnk16");
	r.reflect(ckgmUnk17, "ckgmUnk17");
	r.reflect(ckgmUnk18, "ckgmUnk18");
	r.reflect(ckgmUnk19, "ckgmUnk19");
	r.reflect(ckgmUnk20, "ckgmUnk20");
	r.reflect(ckgmUnk21, "ckgmUnk21");
	r.reflect(ckgmUnk22, "ckgmUnk22");
	r.reflect(ckgmUnk23, "ckgmUnk23");
	r.reflect(ckgmUnk24, "ckgmUnk24");
	r.reflect(ckgmUnk25, "ckgmUnk25");
	r.reflect(ckgmUnk26, "ckgmUnk26");
	r.reflect(ckgmUnk27, "ckgmUnk27");
	r.reflect(ckgmUnk28, "ckgmUnk28");
	r.reflect(ckgmUnk29, "ckgmUnk29");
	r.reflect(ckgmUnk30, "ckgmUnk30");
	r.reflect(ckgmUnk31, "ckgmUnk31");
};
