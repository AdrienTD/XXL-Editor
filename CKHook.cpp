#include "CKHook.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"
#include "CKGroup.h"
#include "CKDictionary.h"

void CKHook::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	next = kenv->readObjRef<CKHook>(file);
	unk1 = file->readUint32();
	life = kenv->readObjRef<CKHookLife>(file);
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

void CKHkEnemy::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHook::deserialize(kenv, file, length);
	unk1 = file->readUint32();
	for (float &f : unk2)
		f = file->readFloat();
	unk3 = file->readFloat();
	unk4 = file->readUint8();
	unk5 = file->readUint8();
	squad = kenv->readObjRef<CKObject>(file);
	for (float &f : unk7)
		f = file->readFloat();
	unk8 = file->readFloat();
	unk9 = kenv->readObjRef<CKObject>(file);
	unkA = kenv->readObjRef<CKObject>(file);
	shadowCpnt = kenv->readObjRef<CKObject>(file);
	hkWaterFx = kenv->readObjRef<CKObject>(file);
}

void CKHkEnemy::serialize(KEnvironment * kenv, File * file)
{
	CKHook::serialize(kenv, file);
	file->writeUint32(unk1);
	for (float &f : unk2)
		file->writeFloat(f);
	file->writeFloat(unk3);
	file->writeUint8(unk4);
	file->writeUint8(unk5);
	kenv->writeObjRef(file, squad);
	for (float &f : unk7)
		file->writeFloat(f);
	file->writeFloat(unk8);
	kenv->writeObjRef(file, unk9);
	kenv->writeObjRef(file, unkA);
	kenv->writeObjRef(file, shadowCpnt);
	kenv->writeObjRef(file, hkWaterFx);
}

void CKHkSeizableEnemy::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHkEnemy::deserialize(kenv, file, length);
	sunk1 = file->readUint32();
	sunk2 = file->readUint8();
	sunk3 = file->readUint8();
	sunk4 = file->readUint8();
	for (auto &ref : boundingShapes)
		ref = kenv->readObjRef<CKBoundingShape>(file);
	particlesNodeFx1 = kenv->readObjRef<CKSceneNode>(file);
	particlesNodeFx2 = kenv->readObjRef<CKSceneNode>(file);
	particlesNodeFx3 = kenv->readObjRef<CKSceneNode>(file);
	fogBoxNode = kenv->readObjRef<CKSceneNode>(file);
	sunused = file->readUint32();
	hero = kenv->readObjRef<CKHook>(file);
	romanAnimatedClone = kenv->readObjRef<CKSceneNode>(file);
	sunk5 = file->readUint8();
	for (float &f : sunk6)
		f = file->readFloat();
}

void CKHkSeizableEnemy::serialize(KEnvironment * kenv, File * file)
{
	CKHkEnemy::serialize(kenv, file);
	file->writeUint32(sunk1);
	file->writeUint8(sunk2);
	file->writeUint8(sunk3);
	file->writeUint8(sunk4);
	for (auto &ref : boundingShapes)
		kenv->writeObjRef(file, ref);
	kenv->writeObjRef(file, particlesNodeFx1);
	kenv->writeObjRef(file, particlesNodeFx2);
	kenv->writeObjRef(file, particlesNodeFx3);
	kenv->writeObjRef(file, fogBoxNode);
	file->writeUint32(sunused);
	kenv->writeObjRef(file, hero);
	kenv->writeObjRef(file, romanAnimatedClone);
	file->writeUint8(sunk5);
	for (float &f : sunk6)
		file->writeFloat(f);
}

void CKHkSquadSeizableEnemy::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHkSeizableEnemy::deserialize(kenv, file, length);
	for (float &f : matrix33)
		f = file->readFloat();
	sunk7 = file->readUint32();
}

void CKHkSquadSeizableEnemy::serialize(KEnvironment * kenv, File * file)
{
	CKHkSeizableEnemy::serialize(kenv, file);
	for (float &f : matrix33)
		file->writeFloat(f);
	file->writeUint32(sunk7);
}

void CKHkBasicEnemy::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHkSquadSeizableEnemy::deserialize(kenv, file, length);
	beClone1 = kenv->readObjRef<CKSceneNode>(file);
	beClone2 = kenv->readObjRef<CKSceneNode>(file);
	beClone3 = kenv->readObjRef<CKSceneNode>(file);
	beClone4 = kenv->readObjRef<CKSceneNode>(file);
	beParticleNode1 = kenv->readObjRef<CKSceneNode>(file);
	beParticleNode2 = kenv->readObjRef<CKSceneNode>(file);
	beParticleNode3 = kenv->readObjRef<CKSceneNode>(file);
	beParticleNode4 = kenv->readObjRef<CKSceneNode>(file);
	beAnimDict = kenv->readObjRef<CAnimationDictionary>(file);
	beSoundDict = kenv->readObjRef<CKObject>(file);
	beBoundNode = kenv->readObjRef<CKBoundingShape>(file);

	romanAnimatedClone2 = kenv->readObjRef<CAnimatedClone>(file);
	beUnk1 = file->readUint8();
	for (float &f : beUnk2)
		f = file->readFloat();
	romanAnimatedClone3 = kenv->readObjRef<CAnimatedClone>(file);
	beUnk3 = file->readUint8();
	for (float &f : beUnk4)
		f = file->readFloat();
	beUnk5 = file->readFloat();
	beUnk6 = file->readFloat();
}

void CKHkBasicEnemy::serialize(KEnvironment * kenv, File * file)
{
	CKHkSquadSeizableEnemy::serialize(kenv, file);
	kenv->writeObjRef(file, beClone1);
	kenv->writeObjRef(file, beClone2);
	kenv->writeObjRef(file, beClone3);
	kenv->writeObjRef(file, beClone4);
	kenv->writeObjRef(file, beParticleNode1);
	kenv->writeObjRef(file, beParticleNode2);
	kenv->writeObjRef(file, beParticleNode3);
	kenv->writeObjRef(file, beParticleNode4);
	kenv->writeObjRef(file, beAnimDict);
	kenv->writeObjRef(file, beSoundDict);
	kenv->writeObjRef(file, beBoundNode);

	kenv->writeObjRef(file, romanAnimatedClone2);
	file->writeUint8(beUnk1);
	for (float &f : beUnk2)
		file->writeFloat(f);
	kenv->writeObjRef(file, romanAnimatedClone2);
	file->writeUint8(beUnk3);
	for (float &f : beUnk4)
		file->writeFloat(f);
	file->writeFloat(beUnk5);
	file->writeFloat(beUnk6);
}

void CKHkRocketRoman::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHkBasicEnemy::deserialize(kenv, file, length);
	rrAnimDict = kenv->readObjRef<CKObject>(file);
	rrParticleNode = kenv->readObjRef<CKObject>(file);
	rrCylinderNode = kenv->readObjRef<CKObject>(file);
	rrSoundDictID = kenv->readObjRef<CKObject>(file);
}

void CKHkRocketRoman::serialize(KEnvironment * kenv, File * file)
{
	CKHkBasicEnemy::serialize(kenv, file);
	kenv->writeObjRef(file, rrAnimDict);
	kenv->writeObjRef(file, rrParticleNode);
	kenv->writeObjRef(file, rrCylinderNode);
	kenv->writeObjRef(file, rrSoundDictID);
}

void CKHkSkyLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHookLife::deserialize(kenv, file, length);
	skyColor = file->readUint32();
	cloudColor = file->readUint32();
}

void CKHkSkyLife::serialize(KEnvironment * kenv, File * file)
{
	CKHookLife::serialize(kenv, file);
	file->writeUint32(skyColor);
	file->writeUint32(cloudColor);
}

void CKHkBoatLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHookLife::deserialize(kenv, file, length);
	boatHook = kenv->readObjRef<CKHook>(file);
}

void CKHkBoatLife::serialize(KEnvironment * kenv, File * file)
{
	CKHookLife::serialize(kenv, file);
	kenv->writeObjRef(file, boatHook);
}
