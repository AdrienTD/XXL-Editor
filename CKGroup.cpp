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
