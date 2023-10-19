#include "CKComponent.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"
#include "CKDictionary.h"

void CKCrateCpnt::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	group = kenv->readObjRef<CKObject>(file);
	particleNode = kenv->readObjRef<CKSceneNode>(file);
	soundIds = kenv->readObjRef<CKObject>(file);
	projectiles = kenv->readObjRef<CKObject>(file);
	crateNode = kenv->readObjRef<CKSceneNode>(file);

	if (kenv->version >= kenv->KVERSION_XXL2) {
		for (auto &cqd : x2CamQuakeDatas) {
			uint32_t sz = file->readUint32();
			cqd.data1.resize(sz * 2);
			for (float &f : cqd.data1)
				f = file->readFloat();
			sz = file->readUint32();
			cqd.data2.resize(sz * 2);
			for (float &f : cqd.data2)
				f = file->readFloat();
			cqd.fnFloat = file->readFloat();
		}
		x2UnkFlt = file->readFloat();
	}

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

	if (kenv->version >= kenv->KVERSION_XXL2) {
		for (auto &cqd : x2CamQuakeDatas) {
			uint32_t cnt = cqd.data1.size() / 2;
			file->writeUint32(cnt);
			for (float &f : cqd.data1)
				file->writeFloat(f);
			cnt = cqd.data2.size() / 2;
			file->writeUint32(cnt);
			for (float &f : cqd.data2)
				file->writeFloat(f);
			file->writeFloat(cqd.fnFloat);
		}
		file->writeFloat(x2UnkFlt);
	}

	for (float &f : unk1) file->writeFloat(f);
	file->writeUint8(unk7);
	for (float &f : pack1) file->writeFloat(f);
	for (float &f : pack2) file->writeFloat(f);
	for (int &f : bonuses) file->writeUint32(f);
	file->writeUint16(unk8);
	file->writeUint8(unk9);
}

void CKCrateCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(group, "group");
	r.reflect(particleNode, "particleNode");
	r.reflect(soundIds, "soundIds");
	r.reflect(projectiles, "projectiles");
	r.reflect(crateNode, "crateNode");

	// TODO: XXL2

	r.reflect(unk1, "unk1");
	r.reflect(unk7, "unk7");
	r.reflect(pack1, "pack1");
	r.reflect(pack2, "pack2");
	r.reflect(bonuses, "bonuses");
	r.reflect(unk8, "unk8");
	r.reflect(unk9, "unk9");
}

#define RREFLECT(r, var) r.reflect(var, #var)

void CKEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	RREFLECT(r, flags);
	RREFLECT(r, health);
	RREFLECT(r, damage);
	RREFLECT(r, unk1);
	RREFLECT(r, walkAnimSpeed);
	RREFLECT(r, coverAnimSpeed);
	RREFLECT(r, speed1);
	RREFLECT(r, runSpeed1);
	RREFLECT(r, speed2);
	RREFLECT(r, speed3);
	RREFLECT(r, runSpeed2);
	RREFLECT(r, speed4);
	RREFLECT(r, speed5);
	RREFLECT(r, unkf1);
	RREFLECT(r, unkf2);
	RREFLECT(r, unkf3);
	RREFLECT(r, coverRange);
	RREFLECT(r, unkf5);
	RREFLECT(r, attackCooldown);
	RREFLECT(r, unkf7);
	RREFLECT(r, unkf8);
	RREFLECT(r, unkf9);
	RREFLECT(r, targeting);
	RREFLECT(r, unkLast);
	if (kenv->isRemaster) {
		RREFLECT(r, ecRoma1);
		RREFLECT(r, ecRoma2);
		RREFLECT(r, ecRoma3);
		RREFLECT(r, ecRoma4);
		RREFLECT(r, ecRoma5);
		RREFLECT(r, ecRoma6);
		RREFLECT(r, ecRoma7);
	}
}

void CKShadowCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(scpValues, "scpValues");
	r.reflect(scpBytes, "scpBytes");
}

void CKBonusCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(ckbcUnk0, "ckbcUnk0");
	r.reflect(ckbcUnk1, "ckbcUnk1");
	r.reflect(ckbcUnk2, "ckbcUnk2");
	r.reflect(ckbcUnk3, "ckbcUnk3");
	r.reflect(ckbcUnk4, "ckbcUnk4");
	r.reflect(ckbcUnk5, "ckbcUnk5");
	r.reflect(ckbcUnk6, "ckbcUnk6");
	r.reflect(ckbcUnk7, "ckbcUnk7");
	r.reflect(ckbcUnk8, "ckbcUnk8");
	r.reflect(ckbcUnk9, "ckbcUnk9");
	r.reflect(ckbcUnk10, "ckbcUnk10");
	r.reflect(ckbcUnk11, "ckbcUnk11");
	r.reflect(ckbcUnk12, "ckbcUnk12");
	r.reflect(ckbcUnk13, "ckbcUnk13");
	r.reflect(ckbcUnk14, "ckbcUnk14");
};
