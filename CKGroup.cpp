#include "CKGroup.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKHook.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKComponent.h"
#include "CKDictionary.h"

void CKGroup::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
		nextGroup = kenv->readObjRef<CKGroup>(file);
		parentGroup = kenv->readObjRef<CKGroup>(file);
		life = kenv->readObjRef<CKGroupLife>(file);
		bundle = kenv->readObjRef<CKBundle>(file);
		unk2 = file->readUint32();
		childGroup = kenv->readObjRef<CKGroup>(file);
		childHook = kenv->readObjRef<CKHook>(file);
	}
	else {
		x2UnkA = file->readUint32();
		unk2 = file->readUint32();
		uint32_t x2ref = file->readUint32();
		assert(x2ref == 0xFFFFFFFF);
		nextGroup = kenv->readObjRef<CKGroup>(file);
		parentGroup = kenv->readObjRef<CKGroup>(file);
		life = kenv->readObjRef<CKGroupLife>(file);
		bundle = kenv->readObjRef<CKBundle>(file);
		childGroup = kenv->readObjRef<CKGroup>(file);
		childHook = kenv->readObjRef<CKHook>(file);
	}
}

void CKGroup::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
		kenv->writeObjRef(file, nextGroup);
		kenv->writeObjRef(file, parentGroup);
		kenv->writeObjRef(file, life);
		kenv->writeObjRef(file, bundle);
		file->writeUint32(unk2);
		kenv->writeObjRef(file, childGroup);
		kenv->writeObjRef(file, childHook);
	}
	else {
		file->writeUint32(x2UnkA);
		file->writeUint32(unk2);
		file->writeUint32(0xFFFFFFFF);
		kenv->writeObjRef(file, nextGroup);
		kenv->writeObjRef(file, parentGroup);
		kenv->writeObjRef(file, life);
		kenv->writeObjRef(file, bundle);
		kenv->writeObjRef(file, childGroup);
		kenv->writeObjRef(file, childHook);
	}
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

void CKGrpBaseSquad::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGroup::deserialize(kenv, file, length);
	bsUnk1 = file->readUint32();
	msgAction = kenv->readObjRef<CKMsgAction>(file);
}

void CKGrpBaseSquad::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	file->writeUint32(bsUnk1);
	kenv->writeObjRef(file, msgAction);
}

void CKGrpSquad::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGrpBaseSquad::deserialize(kenv, file, length);
	for (auto mat : { &mat1, &mat2 }) {
		*mat = Matrix::getIdentity();
		for (int i = 0; i < 4; i++)
			for(int j = 0; j < 3; j++)
				mat->m[i][j] = file->readFloat();
	}
	sqUnk1 = file->readFloat();
	for (float &c : sqUnk2)
		c = file->readFloat();
	sqBizObj1 = kenv->readObjRef<CKObject>(file);
	sqBizMarker1.read(kenv, file);
	sqBizObj2 = kenv->readObjRef<CKObject>(file);
	sqBizMarker2.read(kenv, file);
	for (auto arr : { &sqUnk3, &sqUnk4 })
		for (Vector3& v : *arr)
			for (float& f : v)
				f = file->readFloat();
	sqUnk5 = file->readUint32();
	uint32_t numChoreographies = file->readUint32();
	choreographies.resize(numChoreographies);
	for (auto &ref : choreographies)
		ref = kenv->readObjRef<CKChoreography>(file);
	uint32_t numChoreoKeys = file->readUint32();
	choreoKeys.resize(numChoreoKeys);
	for (auto &ref : choreoKeys)
		ref = kenv->readObjRef<CKChoreoKey>(file);
	for (auto arr : { &guardMarkers, &spawnMarkers }) {
		arr->resize(file->readUint32());
		for (auto &bing : *arr) {
			bing.markerIndex.read(kenv, file);
			bing.b = file->readUint8();
		}
	}
	fings.resize(file->readUint32());
	for (auto &i : fings)
		i = file->readUint32();
	for (auto &f : sqUnk6)
		f = file->readFloat();
	sqUnk6b = file->readUint32();
	sqUnk7 = file->readUint16();
	sqUnk8 = file->readUint8();
	pools.resize(file->readUint32());
	for (PoolEntry &pe : pools) {
		pe.pool = kenv->readObjRef<CKGrpPoolSquad>(file);
		pe.cpnt = kenv->readObjRef<CKEnemyCpnt>(file);
		pe.u1 = file->readUint8();
		pe.numEnemies = file->readUint16();
		pe.u2 = file->readUint8();
		pe.u3 = kenv->readObjRef<CKObject>(file);
	}
	sqUnkA.read(kenv, file, this);
	sqUnkB = file->readFloat();
	if (kenv->isRemaster)
		sqRomasterValue = file->readUint8();
	sqUnkC.read(kenv, file, this);
}

void CKGrpSquad::serialize(KEnvironment * kenv, File * file)
{
	CKGrpBaseSquad::serialize(kenv, file);
	for (auto mat : { &mat1, &mat2 }) {
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 3; j++)
				file->writeFloat(mat->m[i][j]);
	}
	file->writeFloat(sqUnk1);
	for (float &c : sqUnk2)
		file->writeFloat(c);
	kenv->writeObjRef<CKObject>(file, sqBizObj1);
	sqBizMarker1.write(kenv, file);
	kenv->writeObjRef<CKObject>(file, sqBizObj2);
	sqBizMarker2.write(kenv, file);
	for (auto arr : { &sqUnk3, &sqUnk4 })
		for (Vector3& v : *arr)
			for (float& f : v)
				file->writeFloat(f);
	file->writeUint32(sqUnk5);
	file->writeUint32(choreographies.size());
	for (auto &ref : choreographies)
		kenv->writeObjRef(file, ref);
	file->writeUint32(choreoKeys.size());
	for (auto &ref : choreoKeys)
		kenv->writeObjRef(file, ref);
	for (auto arr : { &guardMarkers, &spawnMarkers }) {
		file->writeUint32(arr->size());
		for (auto &bing : *arr) {
			bing.markerIndex.write(kenv, file);
			file->writeUint8(bing.b);
		}
	}
	file->writeUint32(fings.size());
	for (auto &i : fings)
		file->writeUint32(i);
	for (auto &f : sqUnk6)
		file->writeFloat(f);
	file->writeUint32(sqUnk6b);
	file->writeUint16(sqUnk7);
	file->writeUint8(sqUnk8);
	file->writeUint32(pools.size());
	for (PoolEntry &pe : pools) {
		kenv->writeObjRef(file, pe.pool);
		kenv->writeObjRef(file, pe.cpnt);
		file->writeUint8(pe.u1);
		file->writeUint16(pe.numEnemies);
		file->writeUint8(pe.u2);
		kenv->writeObjRef<CKObject>(file, pe.u3);
	}
	sqUnkA.write(kenv, file);
	file->writeFloat(sqUnkB);
	if (kenv->isRemaster)
		file->writeUint8(sqRomasterValue);
	sqUnkC.write(kenv, file);
}

void CKGrpSquadEnemy::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGrpSquad::deserialize(kenv, file, length);
	seUnk1 = file->readFloat();
	seUnk2 = file->readFloat();
}

void CKGrpSquadEnemy::serialize(KEnvironment * kenv, File * file)
{
	CKGrpSquad::serialize(kenv, file);
	file->writeFloat(seUnk1);
	file->writeFloat(seUnk2);
}

void CKGrpPoolSquad::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGroup::deserialize(kenv, file, length);
	if (kenv->version == kenv->KVERSION_XXL1) {
		somenum = file->readUint32();
		shadowCpnt = kenv->readObjRef<CKObject>(file);
	}
	else if (kenv->version >= kenv->KVERSION_XXL2) {
		uint32_t ncpnt = file->readUint32();
		for (uint32_t i = 0; i < ncpnt; i++)
			components.push_back(kenv->readObjRef<CKObject>(file));
		enemyType = file->readUint8();
	}
}

void CKGrpPoolSquad::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	if (kenv->version == kenv->KVERSION_XXL1) {
		file->writeUint32(somenum);
		kenv->writeObjRef(file, shadowCpnt);
	}
	else if (kenv->version >= kenv->KVERSION_XXL2) {
		file->writeUint32(components.size());
		for (auto& cpnt : components)
			kenv->writeObjRef(file, cpnt);
		file->writeUint8(enemyType);
	}
}

void CKGrpSquadJetPack::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGrpSquadEnemy::deserialize(kenv, file, length);
	uint16_t numHearths = file->readUint16();
	hearths.resize(numHearths);
	for (auto &hook : hearths)
		hook = kenv->readObjRef<CKHook>(file);
	sjpUnk1 = file->readFloat();
	sjpUnk2 = file->readUint8();
	sjpUnk3 = file->readUint8();
	for (auto &pn : particleNodes)
		pn = kenv->readObjRef<CKSceneNode>(file);
}

void CKGrpSquadJetPack::serialize(KEnvironment * kenv, File * file)
{
	CKGrpSquadEnemy::serialize(kenv, file);
	file->writeUint16(hearths.size());
	for (auto &hook : hearths)
		kenv->writeObjRef<CKHook>(file, hook);
	file->writeFloat(sjpUnk1);
	file->writeUint8(sjpUnk2);
	file->writeUint8(sjpUnk3);
	for (auto &pn : particleNodes)
		kenv->writeObjRef<CKSceneNode>(file, pn);
}

void CKGrpLight::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGroup::deserialize(kenv, file, length);
	node = kenv->readObjRef<CKSceneNode>(file);
	texname = file->readSizedString<uint16_t>();
}

void CKGrpLight::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	kenv->writeObjRef(file, node);
	file->writeSizedString<uint16_t>(texname);
}

void CKGrpSquadX2::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
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
	r.reflect(ckgfeSoundDict, "ckgfeSoundDict");
};

void CKGrpCatapult::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(ckgcShadowCpnt, "ckgcShadowCpnt");
}

void CKGrpAsterixCheckpoint::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(astCheckpointValue, "astCheckpointValue");
}

void CKGrpBonusSpitter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(ckgbsUnk5, "ckgbsUnk5");
	r.reflect(ckgbsUnk6, "ckgbsUnk6");
	r.reflect(ckgbsUnk7, "ckgbsUnk7");
	r.reflect(ckgbsUnk8, "ckgbsUnk8");
};

void CKGrpMap::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
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
