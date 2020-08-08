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

#define RREFLECT(r, var) r.reflect(var, #var)

void CKEnemyCpnt::reflectMembers(MemberListener &r)
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
}

void CKSeizableEnemyCpnt::reflectMembers(MemberListener & r)
{
	CKEnemyCpnt::reflectMembers(r);
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

void CKSquadSeizableEnemyCpnt::reflectMembers(MemberListener & r)
{
	CKSeizableEnemyCpnt::reflectMembers(r);
	RREFLECT(r, sqseUnk1);
}

void CKBasicEnemyCpnt::reflectMembers(MemberListener & r)
{
	CKSquadSeizableEnemyCpnt::reflectMembers(r);
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

void CKRocketRomanCpnt::reflectMembers(MemberListener & r)
{
	CKBasicEnemyCpnt::reflectMembers(r);
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

void CKRomanArcherCpnt::reflectMembers(MemberListener & r)
{
	CKSquadSeizableEnemyCpnt::reflectMembers(r);
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

void CKTurtleCpnt::reflectMembers(MemberListener & r)
{
	CKSquadEnemyCpnt::reflectMembers(r);
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

void CKJumpingRomanCpnt::reflectMembers(MemberListener & r)
{
	CKSquadSeizableEnemyCpnt::reflectMembers(r);
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

void CKMobileTowerCpnt::reflectMembers(MemberListener & r)
{
	CKSquadEnemyCpnt::reflectMembers(r);
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

void CKJetPackRomanCpnt::reflectMembers(MemberListener & r)
{
	CKSquadEnemyCpnt::reflectMembers(r);
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
