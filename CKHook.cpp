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
	pool = kenv->readObjRef<CKGrpBonusPool>(file);
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

void CKHkWildBoar::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHook::deserialize(kenv, file, length);
	nextBoar = kenv->readObjRef<CKHkWildBoar>(file);
	boundingSphere = kenv->readObjRef<CKSceneNode>(file);
	animationDictionary = kenv->readObjRef<CKObject>(file);
	cpnt = kenv->readObjRef<CKObject>(file);
	pool = kenv->readObjRef<CKGrpWildBoarPool>(file);
	for (float &f : somenums)
		f = file->readFloat();
	shadowCpnt = kenv->readObjRef<CKObject>(file);
}

void CKHkWildBoar::serialize(KEnvironment * kenv, File * file)
{
	CKHook::serialize(kenv, file);
	kenv->writeObjRef(file, nextBoar);
	kenv->writeObjRef(file, boundingSphere);
	kenv->writeObjRef(file, animationDictionary);
	kenv->writeObjRef(file, cpnt);
	kenv->writeObjRef(file, pool);
	for (float &f : somenums)
		file->writeFloat(f);
	kenv->writeObjRef(file, shadowCpnt);
}
