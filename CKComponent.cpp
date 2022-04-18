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

void CKEnemyCpnt::reflectMembers2(MemberListener &r, KEnvironment *kenv)
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

void CKSeizableEnemyCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, seUnk1);
	RREFLECT(r, seUnk2);
	RREFLECT(r, seUnk3);
	RREFLECT(r, seBBoxSize);
	RREFLECT(r, seFortitude);
	RREFLECT(r, seUnk6);
	RREFLECT(r, seBBoxOffset);
	RREFLECT(r, seUnk8);
	RREFLECT(r, seUnk9);
	RREFLECT(r, seColScale);
	RREFLECT(r, seUnk11);
	RREFLECT(r, seUnk12);
	RREFLECT(r, seColOffset);
	RREFLECT(r, seStunTime);
	RREFLECT(r, seUnk15);
	RREFLECT(r, seKnockback);
	RREFLECT(r, seUnk17);
	RREFLECT(r, seUnk18);
	RREFLECT(r, seUnk19);
	RREFLECT(r, seUnk20);
	RREFLECT(r, seUnk21);
	RREFLECT(r, seUnk22);
	RREFLECT(r, seUnk23);
	RREFLECT(r, seComboStunTime);
	RREFLECT(r, seDeathSpeed);
	RREFLECT(r, seDeathFlySpeed);
	RREFLECT(r, seShieldPoints);
	RREFLECT(r, seUnk28);
	RREFLECT(r, seCoverTime);
	RREFLECT(r, seKnockbackSpeed);
	RREFLECT(r, seKnockbackResistance);
}

void CKSquadSeizableEnemyCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSeizableEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, sqseUnk1);
}

void CKBasicEnemyCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSquadSeizableEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, beUnk1);
	RREFLECT(r, beUnk2);
	RREFLECT(r, beUnk3);
	RREFLECT(r, beRange);
	RREFLECT(r, beUnk5);
	RREFLECT(r, beChargeDuration);
	RREFLECT(r, beAttackTime1);
	RREFLECT(r, beAttackTime2);
	RREFLECT(r, beAttackTime3);
	RREFLECT(r, beAttackTime4);
	RREFLECT(r, beAttackTime5);
	RREFLECT(r, beUnk12);
	RREFLECT(r, beUnk13);
}

void CKRocketRomanCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKBasicEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, rrCylinderRadius);
	RREFLECT(r, rrCylinderHeight);
	RREFLECT(r, rrUnk3);
	RREFLECT(r, rrUnk4);
	RREFLECT(r, rrFireDistance);
	RREFLECT(r, rrUnk6);
	RREFLECT(r, rrFlySpeed);
	RREFLECT(r, rrRomanAimFactor);
	RREFLECT(r, rrUnk9);
}

void CKRomanArcherCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSquadSeizableEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, raUnk1);
	RREFLECT(r, raUnk2);
	RREFLECT(r, raUnk3);
	RREFLECT(r, raUnk4);
	RREFLECT(r, raUnk5);
	RREFLECT(r, raUnk6);
	RREFLECT(r, raUnk7);
	RREFLECT(r, raNumArrowsPerAttack);
	RREFLECT(r, raArrowTimeInterval);
}

void CKTurtleCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSquadEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, ttUnk1);
	RREFLECT(r, ttUnk2);
	RREFLECT(r, ttUnk3);
	RREFLECT(r, ttUnk4);
	RREFLECT(r, ttUnk5);
	RREFLECT(r, ttUnk6);
	RREFLECT(r, ttUnk7);
	RREFLECT(r, ttUnk8);
	//RREFLECT(r, ttUnk9);
	r.reflectSize<uint16_t>(ttSpearStates, "ttNumSpearStates");
	r.reflectContainer(ttSpearStates, "ttSpearStates");
	RREFLECT(r, ttUnk11);
	RREFLECT(r, ttUnk12);
	RREFLECT(r, ttUnk13);
	RREFLECT(r, ttUnk14);
}

void CKJumpingRomanCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSquadSeizableEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, jrUnk1);
	RREFLECT(r, jrUnk2);
	RREFLECT(r, jrUnk3);
	RREFLECT(r, jrUnk4);
	RREFLECT(r, jrUnk5);
	RREFLECT(r, jrUnk6);
	RREFLECT(r, jrUnk7);
	RREFLECT(r, jrUnk8);
	RREFLECT(r, jrUnk9);
	RREFLECT(r, jrUnk10);
	RREFLECT(r, jrUnk11);
	RREFLECT(r, jrUnk12);
	RREFLECT(r, jrUnk13);
	RREFLECT(r, jrUnk14);
	RREFLECT(r, jrUnk15);
	RREFLECT(r, jrUnk16);
	RREFLECT(r, jrUnk17);
	RREFLECT(r, jrUnk18);
	RREFLECT(r, jrUnk19);
}

void CKMobileTowerCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSquadEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, mtUnk1);
	RREFLECT(r, mtUnk2);
	RREFLECT(r, mtUnk3);
	RREFLECT(r, mtUnk4);
	RREFLECT(r, mtUnk5);
	RREFLECT(r, mtUnk6);
	RREFLECT(r, mtUnk7);
	RREFLECT(r, mtUnk8);
	RREFLECT(r, mtUnk9);
	RREFLECT(r, mtUnk10);
	RREFLECT(r, mtUnk11);
	RREFLECT(r, mtUnk12);
	RREFLECT(r, mtUnk13);
	RREFLECT(r, mtUnk14);
	RREFLECT(r, mtUnk15);
	RREFLECT(r, mtUnk16);
	RREFLECT(r, mtUnk17);
	RREFLECT(r, mtUnk18);
	RREFLECT(r, mtUnk19);
	RREFLECT(r, mtUnk20);
	RREFLECT(r, mtUnk21);
	RREFLECT(r, mtUnk22);
	RREFLECT(r, mtUnk23);
	RREFLECT(r, mtUnk24);
	RREFLECT(r, mtUnk25);
	RREFLECT(r, mtUnk26);
	RREFLECT(r, mtUnk27);
}

void CKJetPackRomanCpnt::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	CKSquadEnemyCpnt::reflectMembers2(r, kenv);
	RREFLECT(r, jpUnk1);
	RREFLECT(r, jpUnk2);
	RREFLECT(r, jpUnk3);
	RREFLECT(r, jpUnk4);
	RREFLECT(r, jpUnk5);
	RREFLECT(r, jpUnk6);
	RREFLECT(r, jpUnk7);
	RREFLECT(r, jpUnk8);
	RREFLECT(r, jpUnk9);
	RREFLECT(r, jpUnk10);
	RREFLECT(r, jpUnk11);
	RREFLECT(r, jpUnk12);
	RREFLECT(r, jpUnk13);
	RREFLECT(r, jpUnk14);
	RREFLECT(r, jpUnk15);
	RREFLECT(r, jpUnk16);
	RREFLECT(r, jpUnk17);
	RREFLECT(r, jpUnk18);
	RREFLECT(r, jpUnk19);
	RREFLECT(r, jpUnk20);
	RREFLECT(r, jpUnk21);
	RREFLECT(r, jpUnk22);
	RREFLECT(r, jpUnk23);
}

void CKGrpMecaCpntAsterix::reflectMembers2(MemberListener & r, KEnvironment * kenv)
{
	//CKComponent::reflectMembers(r);
	r.reflect(cpmecWoodenCrateCpnt, "cpmecWoodenCrateCpnt");
	r.reflect(cpmecMetalCrateCpnt, "cpmecMetalCrateCpnt");
	if (kenv->isRemaster) {
		r.reflect(cpmecPark1CrateCpnt, "cpmecPark1CrateCpnt");
		r.reflect(cpmecPark3CrateCpnt, "cpmecPark3CrateCpnt");
		r.reflect(cpmecPark5CrateCpnt, "cpmecPark5CrateCpnt");
	}
	r.reflect(cpmecOtherRefs, "cpmecOtherRefs");
	r.reflect(cpmecUnk6, "cpmecUnk6");
	r.reflect(cpmecSndDictID, "cpmecSndDictID");
}

void CKShadowCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(scpValues, "scpValues");
	r.reflect(scpBytes, "scpBytes");
}

void CKWildBoarCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(ckwbcUnk0, "ckwbcUnk0");
	r.reflect(ckwbcUnk1, "ckwbcUnk1");
	r.reflect(ckwbcUnk2, "ckwbcUnk2");
	r.reflect(ckwbcUnk3, "ckwbcUnk3");
	r.reflect(ckwbcUnk4, "ckwbcUnk4");
	r.reflect(ckwbcUnk5, "ckwbcUnk5");
};

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
