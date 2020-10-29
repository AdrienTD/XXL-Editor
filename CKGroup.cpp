#include "CKGroup.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKHook.h"
#include "CKNode.h"
#include "CKLogic.h"
#include "CKComponent.h"

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

void CKGrpBonusPool::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGroup::deserialize(kenv, file, length);
	bonusType = file->readUint32();
	handlerId = file->readUint32();
	unk2 = file->readUint32();
	if (kenv->version >= kenv->KVERSION_XXL2)
		x2UnkFlt = file->readFloat();
	unk3 = file->readUint32();
	unk4 = file->readUint32();
	assert(unk3 == unk4 && unk4 == 0xFFFFFFFF);
	nextBonusHook = kenv->readObjRef<CKHkBasicBonus>(file);
	bonusCpnt = kenv->readObjRef<CKObject>(file);
	particleNode1 = kenv->readObjRef<CKSceneNode>(file);
	particleNode2 = kenv->readObjRef<CKSceneNode>(file);
	if (kenv->version < kenv->KVERSION_XXL2)
		secondBonusCpnt = kenv->readObjRef<CKObject>(file);
}

void CKGrpBonusPool::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	file->writeUint32(bonusType);
	file->writeUint32(handlerId);
	file->writeUint32(unk2);
	if (kenv->version >= kenv->KVERSION_XXL2)
		file->writeFloat(x2UnkFlt);
	file->writeUint32(unk3);
	file->writeUint32(unk4);
	kenv->writeObjRef(file, nextBonusHook);
	kenv->writeObjRef(file, bonusCpnt);
	kenv->writeObjRef(file, particleNode1);
	kenv->writeObjRef(file, particleNode2);
	if (kenv->version < kenv->KVERSION_XXL2)
		kenv->writeObjRef(file, secondBonusCpnt);
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
	for (auto &ref : refs)
		ref = kenv->readObjRef<CKObject>(file);
	for (auto arr : { &sqUnk3, &sqUnk4 })
		for (float &f : *arr)
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
			bing.markerIndex = file->readUint32();
			bing.b = file->readUint8();
		}
	}
	fings.resize(file->readUint32());
	for (auto &i : fings)
		i = file->readUint32();
	for (auto &f : sqUnk6)
		f = file->readFloat();
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
	for (auto &ref : refs)
		kenv->writeObjRef<CKObject>(file, ref);
	for (auto arr : { &sqUnk3, &sqUnk4 })
		for (float &f : *arr)
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
			file->writeUint32(bing.markerIndex);
			file->writeUint8(bing.b);
		}
	}
	file->writeUint32(fings.size());
	for (auto &i : fings)
		file->writeUint32(i);
	for (auto &f : sqUnk6)
		file->writeFloat(f);
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
	somenum = file->readUint32();
	shadowCpnt = kenv->readObjRef<CKObject>(file);
}

void CKGrpPoolSquad::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	file->writeUint32(somenum);
	kenv->writeObjRef(file, shadowCpnt);
}

void CKGrpSquadJetPack::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGrpSquadEnemy::deserialize(kenv, file, length);
	uint16_t numHearths = file->readUint16();
	hearths.resize(numHearths);
	for (auto &hook : hearths)
		hook = kenv->readObjRef<CKHook>(file);
	sjpUnk1 = file->readUint32();
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
	file->writeUint32(sjpUnk1);
	file->writeUint8(sjpUnk2);
	file->writeUint8(sjpUnk3);
	for (auto &pn : particleNodes)
		kenv->writeObjRef<CKSceneNode>(file, pn);
}

void CKGrpLight::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGroup::deserialize(kenv, file, length);
	node = kenv->readObjRef<CKSceneNode>(file);
	uint16_t namesize = file->readUint16();
	char *name = (char*)alloca(namesize);
	file->read(name, namesize);
	texname.assign(name, namesize);
}

void CKGrpLight::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	kenv->writeObjRef(file, node);
	file->writeUint16((uint16_t)texname.size());
	file->write(texname.data(), texname.size());
}
