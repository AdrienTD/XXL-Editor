#include "CKHook.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"

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
