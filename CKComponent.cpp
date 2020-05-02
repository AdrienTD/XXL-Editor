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

void CKBasicEnemyCpnt::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	file->read(data.data(), data.size());
}

void CKBasicEnemyCpnt::serialize(KEnvironment * kenv, File * file)
{
	file->write(data.data(), data.size());
}

void CKRocketRomanCpnt::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKBasicEnemyCpnt::deserialize(kenv, file, length);
	rrCylinderRadius = file->readFloat();
	rrCylinderHeight = file->readFloat();
	for (float &f : runk3)
		f = file->readFloat();
	runk4 = file->readUint8();
	rrFireDistance = file->readFloat();
	runk6 = file->readUint8();
	rrFlySpeed = file->readFloat();
	rrRomanAimFactor = file->readFloat();
	runk9 = kenv->readObjRef<CKObject>(file);
}

void CKRocketRomanCpnt::serialize(KEnvironment * kenv, File * file)
{
	CKBasicEnemyCpnt::serialize(kenv, file);
	file->writeFloat(rrCylinderRadius);
	file->writeFloat(rrCylinderHeight);
	for (float &f : runk3)
		file->writeFloat(f);
	file->writeUint8(runk4);
	file->writeFloat(rrFireDistance);
	file->writeUint8(runk6);
	file->writeFloat(rrFlySpeed);
	file->writeFloat(rrRomanAimFactor);
	kenv->writeObjRef(file, runk9);
}
