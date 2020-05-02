#include "CKGroup.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKHook.h"
#include "CKNode.h"

void CKGroup::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	nextGroup = kenv->readObjRef<CKGroup>(file);
	parentGroup = kenv->readObjRef<CKGroup>(file);
	life = kenv->readObjRef<CKGroupLife>(file);
	bundle = kenv->readObjRef<CKObject>(file);
	unk2 = file->readUint32();
	childGroup = kenv->readObjRef<CKGroup>(file);
	childHook = kenv->readObjRef<CKHook>(file);
}

void CKGroup::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, nextGroup);
	kenv->writeObjRef(file, parentGroup);
	kenv->writeObjRef(file, life);
	kenv->writeObjRef(file, bundle);
	file->writeUint32(unk2);
	kenv->writeObjRef(file, childGroup);
	kenv->writeObjRef(file, childHook);
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
	unk3 = file->readUint32();
	unk4 = file->readUint32();
	assert(unk3 == unk4 && unk4 == 0xFFFFFFFF);
	nextBonusHook = kenv->readObjRef<CKHkBasicBonus>(file);
	bonusCpnt = kenv->readObjRef<CKObject>(file);
	particleNode1 = kenv->readObjRef<CKSceneNode>(file);
	particleNode2 = kenv->readObjRef<CKSceneNode>(file);
	secondBonusCpnt = kenv->readObjRef<CKObject>(file);
}

void CKGrpBonusPool::serialize(KEnvironment * kenv, File * file)
{
	CKGroup::serialize(kenv, file);
	file->writeUint32(bonusType);
	file->writeUint32(handlerId);
	file->writeUint32(unk2);
	file->writeUint32(unk3);
	file->writeUint32(unk4);
	kenv->writeObjRef(file, nextBonusHook);
	kenv->writeObjRef(file, bonusCpnt);
	kenv->writeObjRef(file, particleNode1);
	kenv->writeObjRef(file, particleNode2);
	kenv->writeObjRef(file, secondBonusCpnt);
}

void CKGrpBaseSquad::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGroup::deserialize(kenv, file, length);
	bsUnk1 = file->readUint32();
	msgAction = kenv->readObjRef<CKObject>(file);
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
		for (auto &f : *mat)
			f = file->readFloat();
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
		ref = kenv->readObjRef<CKObject>(file);
	uint32_t numChoreoKeys = file->readUint32();
	choreoKeys.resize(numChoreoKeys);
	for (auto &ref : choreoKeys)
		ref = kenv->readObjRef<CKObject>(file);
	for (auto arr : { &bings, &dings }) {
		arr->resize(file->readUint32());
		for (auto &bing : *arr) {
			bing.a = file->readUint32();
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
		pe.cpnt = kenv->readObjRef<CKObject>(file);
		pe.u1 = file->readUint8();
		pe.u2 = file->readUint8();
		pe.u3 = kenv->readObjRef<CKObject>(file);
	}
	sqUnkA = file->readUint16();
	sqUnkB = file->readFloat();
	sqUnkC = file->readUint16();
}

void CKGrpSquad::serialize(KEnvironment * kenv, File * file)
{
	CKGrpBaseSquad::serialize(kenv, file);
	for (auto mat : { &mat1, &mat2 }) {
		for (auto &f : *mat)
			file->writeFloat(f);
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
		kenv->writeObjRef<CKObject>(file, ref);
	file->writeUint32(choreoKeys.size());
	for (auto &ref : choreoKeys)
		kenv->writeObjRef<CKObject>(file, ref);
	for (auto arr : { &bings, &dings }) {
		file->writeUint32(arr->size());
		for (auto &bing : *arr) {
			file->writeUint32(bing.a);
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
		kenv->writeObjRef<CKObject>(file, pe.cpnt);
		file->writeUint8(pe.u1);
		file->writeUint8(pe.u2);
		kenv->writeObjRef<CKObject>(file, pe.u3);
	}
	file->writeUint16(sqUnkA);
	file->writeFloat(sqUnkB);
	file->writeUint16(sqUnkC);
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