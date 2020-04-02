#include "CKHook.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"
#include "CKGroup.h"

void CKHook::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	next = kenv->readObjRef<CKHook>(file);
	unk1 = file->readUint32();
	life = kenv->readObjRef<CKObject>(file);
	node = kenv->readObjRef<CKSceneNode>(file);
}

void CKHook::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, next);
	file->writeUint32(unk1);
	kenv->writeObjRef(file, life);
	kenv->writeObjRef(file, node);
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

void CKHkBasicBonus::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHook::deserialize(kenv, file, length);
	nextBonus = kenv->readObjRef<CKHkBasicBonus>(file);
	pool = kenv->readObjRef<CKGrpAsterixBonusPool>(file);
	cpnt = kenv->readObjRef<CKObject>(file);
	hero = kenv->readObjRef<CKObject>(file);
	for (float &f : somenums) f = file->readFloat();
}

void CKHkBasicBonus::serialize(KEnvironment * kenv, File * file)
{
	CKHook::serialize(kenv, file);
	kenv->writeObjRef(file, nextBonus);
	kenv->writeObjRef(file, pool);
	kenv->writeObjRef(file, cpnt);
	kenv->writeObjRef(file, hero);
	for (float &f : somenums) file->writeFloat(f);
}
