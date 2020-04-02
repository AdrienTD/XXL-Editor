#include "CKComponent.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"

void CKCrateCpnt::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	group = kenv->readObjRef<CKObject>(file);
	particleNode = kenv->readObjRef<CKSceneNode>(file);
	soundIds = kenv->readObjRef<CKObject>(file);
	projectiles = kenv->readObjRef<CKObject>(file);
	crateNode = kenv->readObjRef<CKSceneNode>(file);
	for (float &f : unk1) f = file->readFloat();
	unk7 = file->readUint8();
	for (float &f : pack1) f = file->readFloat();
	for (float &f : pack2) f = file->readFloat();
	for (int &f : bonuses) f = file->readUint32();
	unk8 = file->readUint16();
	unk9 = file->readUint8();
}

void CKCrateCpnt::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, group);
	kenv->writeObjRef(file, particleNode);
	kenv->writeObjRef(file, soundIds);
	kenv->writeObjRef(file, projectiles);
	kenv->writeObjRef(file, crateNode);
	for (float &f : unk1) file->writeFloat(f);
	file->writeUint8(unk7);
	for (float &f : pack1) file->writeFloat(f);
	for (float &f : pack2) file->writeFloat(f);
	for (int &f : bonuses) file->writeUint32(f);
	file->writeUint16(unk8);
	file->writeUint8(unk9);
}
