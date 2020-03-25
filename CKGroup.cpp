#include "CKGroup.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKHook.h"

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
