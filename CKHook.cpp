#include "CKHook.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"
#include "CKGroup.h"
#include "CKDictionary.h"
#include "CKService.h"
#include "CKLogic.h"

void CKHook::reflectMembers(MemberListener & r)
{
	r.reflect(next, "next");
	r.reflect(unk1, "unk1");
	r.reflect(life, "life");
	r.reflect(node, "node");
}

void CKHook::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
		next = kenv->readObjRef<CKHook>(file);
		unk1 = file->readUint32();
		life = kenv->readObjRef<CKHookLife>(file);
		node.read(file);
	}
	else {
		x2UnkA = file->readUint32();
		x2UnkB = file->readUint32();
		auto x2next = kenv->readObjRef<CKHook>(file);
		next = kenv->readObjRef<CKHook>(file);
		assert(x2next == next);
		life = kenv->readObjRef<CKHookLife>(file);
		node.read(file);
	}
}

void CKHook::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
		kenv->writeObjRef(file, next);
		file->writeUint32(unk1);
		kenv->writeObjRef(file, life);
		node.write(kenv, file);
	}
	else {
		file->writeUint32(x2UnkA);
		file->writeUint32(x2UnkB);
		kenv->writeObjRef(file, next);
		kenv->writeObjRef(file, next);
		kenv->writeObjRef(file, life);
		node.write(kenv, file);
	}
}

void CKHook::onLevelLoaded(KEnvironment * kenv)
{
	int str = -1;
	if (CKHkAnimatedCharacter *hkanim = this->dyncast<CKHkAnimatedCharacter>()) {
		//printf("ac %u\n", hkanim->sector);
		str = (int)hkanim->sector - 1;
	} else if(this->life) {
		//printf("life %u\n", this->life->unk1);
		str = (this->life->unk1 >> 2) - 1;
	}
	printf("bind %s's node to sector %i\n", this->getClassName(), str);
	node.bind(kenv, str);
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

void CKHkAnimatedCharacter::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKHook::deserialize(kenv, file, length);
	animDict = kenv->readObjRef<CAnimationDictionary>(file);
	shadowCpnt = kenv->readObjRef<CKObject>(file);
	unkRef1 = kenv->readObjRef<CKObject>(file);
	for (float &f : matrix.v)
		f = file->readFloat();
	for (float &f : position)
		f = file->readFloat();
	for (float &f : orientation)
		f = file->readFloat();
	for (float &f : unkFloatArray)
		f = file->readFloat();
	unkFloat1 = file->readFloat();
	unkFloat2 = file->readFloat();
	unkFloat3 = file->readFloat();
	unkFloat4 = file->readFloat();
	sector = file->readUint8();
}

void CKHkAnimatedCharacter::serialize(KEnvironment * kenv, File * file)
{
	CKHook::serialize(kenv, file);
	kenv->writeObjRef(file, animDict);
	kenv->writeObjRef(file, shadowCpnt);
	kenv->writeObjRef(file, unkRef1);
	for (float &f : matrix.v)
		file->writeFloat(f);
	for (float &f : position)
		file->writeFloat(f);
	for (float &f : orientation)
		file->writeFloat(f);
	for (float &f : unkFloatArray)
		file->writeFloat(f);
	file->writeFloat(unkFloat1);
	file->writeFloat(unkFloat2);
	file->writeFloat(unkFloat3);
	file->writeFloat(unkFloat4);
	file->writeUint8(sector);

}

void CKHkActivator::reflectMembers(MemberListener & r)
{
	CKHook::reflectMembers(r);
	r.reflect(actAnimDict, "actAnimDict");
	r.reflect(actSndDict, "actSndDict");
	r.reflect(actSphere1, "actSphere1");
	r.reflect(actSphere2, "actSphere2");
	r.reflect(actUnk4, "actUnk4");
	r.reflect(actEvtSeq1, "actEvtSeq1", this);
	r.reflect(actEvtSeq2, "actEvtSeq2", this);
}

//void CKHkActivator::deserialize(KEnvironment * kenv, File * file, size_t length)
//{
//	CKHook::deserialize(kenv, file, length);
//	actAnimDict = kenv->readObjRef<CAnimationDictionary>(file);
//	actSndDict = kenv->readObjRef<CKSoundDictionaryID>(file);
//	actSphere1 = kenv->readObjRef<CKBoundingShape>(file);
//	actSphere2 = kenv->readObjRef<CKBoundingShape>(file);
//	actUnk4 = file->readFloat();
//	actEvtSeq1.read(kenv, file, this);
//	actEvtSeq2.read(kenv, file, this);
//}
//
//void CKHkActivator::serialize(KEnvironment * kenv, File * file)
//{
//	CKHook::serialize(kenv, file);
//	kenv->writeObjRef(file, actAnimDict);
//	kenv->writeObjRef(file, actSndDict);
//	kenv->writeObjRef(file, actSphere1);
//	kenv->writeObjRef(file, actSphere2);
//	file->writeFloat(actUnk4);
//	actEvtSeq1.write(kenv, file);
//	actEvtSeq2.write(kenv, file);
//}

void CKHkTorch::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(torchSndDict, "torchSndDict");
	r.reflect(torchBranch, "torchBranch");
	r.reflect(torchUnk2, "torchUnk2");
	r.reflect(torchUnk3, "torchUnk3");
	r.reflect(torchUnk4, "torchUnk4");
	r.reflect(torchUnk5, "torchUnk5");
	r.reflect(torchUnk6, "torchUnk6");
	r.reflect(torchUnk7, "torchUnk7");
	r.reflect(torchUnk8, "torchUnk8");
	r.reflect(torchEvtSeq1, "torchEvtSeq1", this);
	r.reflect(torchEvtSeq2, "torchEvtSeq2", this);
	r.reflect(torchEvtSeq3, "torchEvtSeq3", this);
}
void CKHkHearth::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(hearthSndDict, "hearthSndDict");
	r.reflect(hearthDynGround, "hearthDynGround");
	r.reflect(hearthEvtSeq1, "hearthEvtSeq1", this);
	r.reflect(hearthEvtSeq2, "hearthEvtSeq2", this);
	r.reflect(hearthEvtSeq3, "hearthEvtSeq3", this);
	r.reflect(hearthEvtSeq4, "hearthEvtSeq4", this);
	r.reflect(hearthEvtSeq5, "hearthEvtSeq5", this);
	r.reflect(hearthUnk7, "hearthUnk7");
	r.reflect(hearthUnk8, "hearthUnk8");
	r.reflect(hearthUnk9, "hearthUnk9");
	r.reflect(hearthUnk10, "hearthUnk10");
	r.reflect(hearthUnk11, "hearthUnk11");
	r.reflect(hearthUnk12, "hearthUnk12");
	r.reflect(hearthUnk13, "hearthUnk13");
	r.reflect(hearthUnk14, "hearthUnk14");
}
void CKHkLight::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(lightGrpLight, "lightGrpLight");
	r.reflect(lightEvtSeq1, "lightEvtSeq1", this);
	r.reflect(lightEvtSeq2, "lightEvtSeq2", this);
	r.reflect(lightEvtSeq3, "lightEvtSeq3", this);
	r.reflect(lightEvtSeq4, "lightEvtSeq4", this);
}

void CKHkPressionStone::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(psSquad, "psSquad");
	r.reflect(psDynGround, "psDynGround");
	r.reflect(psSndDict, "psSndDict");
	r.reflect(psEvtSeq1, "psEvtSeq1", this);
	r.reflect(psEvtSeq2, "psEvtSeq2", this);
	r.reflect(psUnk5, "psUnk5");
	r.reflect(psUnk6, "psUnk6");
	r.reflect(psUnk7, "psUnk7");
	r.reflect(psUnk8, "psUnk8");
}
void CKHkHero::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(heroGrpTrio, "heroGrpTrio");
	r.reflect(heroUnk1, "heroUnk1");
	r.reflect(heroUnk2, "heroUnk2");
	r.reflect(heroUnk3, "heroUnk3");
	r.reflect(heroUnk4, "heroUnk4");
	r.reflect(heroUnk5, "heroUnk5");
	r.reflect(heroUnk6, "heroUnk6");
	r.reflect(heroUnk7, "heroUnk7");
	r.reflect(heroUnk8, "heroUnk8");
	r.reflect(heroUnk9, "heroUnk9");
	r.reflect(heroUnk10, "heroUnk10");
	r.reflect(heroUnk11, "heroUnk11");
	r.reflect(heroBranch1, "heroBranch1");
	r.reflect(heroUnk13, "heroUnk13");
	r.reflect(heroUnk14, "heroUnk14");
	r.reflect(heroUnk15, "heroUnk15");
	r.reflect(heroUnk16, "heroUnk16");
	r.reflect(heroUnk17, "heroUnk17");
	r.reflect(heroUnk18, "heroUnk18");
	r.reflect(heroUnk19, "heroUnk19");
	r.reflect(heroUnk20, "heroUnk20");
	r.reflect(heroUnk21, "heroUnk21");
	r.reflect(heroUnk22, "heroUnk22");
	r.reflect(heroUnk23, "heroUnk23");
	r.reflect(heroAnimDict, "heroAnimDict");
	r.reflect(heroUnk25, "heroUnk25");
	r.reflect(heroUnk26, "heroUnk26");
	r.reflect(heroUnk27, "heroUnk27");
	r.reflect(heroUnk28, "heroUnk28");
	r.reflect(heroUnk29, "heroUnk29");
	r.reflect(heroBranch2, "heroBranch2");
	r.reflect(heroUnk31, "heroUnk31");
	r.reflect(heroUnk32, "heroUnk32");
	r.reflect(heroUnk33, "heroUnk33");
	r.reflect(heroUnk34, "heroUnk34");
	r.reflect(heroUnk35, "heroUnk35");
	r.reflect(heroUnk36, "heroUnk36");
	r.reflect(heroUnk37, "heroUnk37");
	r.reflect(heroUnk38, "heroUnk38");
	r.reflect(heroUnk39, "heroUnk39");
	r.reflect(heroSphere, "heroSphere");
	r.reflect(heroUnk41, "heroUnk41");
	r.reflect(heroUnk42, "heroUnk42");
	r.reflect(heroDynSphere, "heroDynSphere");
	r.reflect(heroCylinder, "heroCylinder");
	r.reflect(heroUnk45, "heroUnk45");
	r.reflect(heroUnk46, "heroUnk46");
	r.reflect(heroParticleNode, "heroParticleNode");
	r.reflect(heroTrailNode, "heroTrailNode");
	r.reflect(heroSndDict, "heroSndDict");
	r.reflect(heroUnk50, "heroUnk50");
	r.reflect(heroUnk51, "heroUnk51");
	r.reflect(heroUnk52, "heroUnk52");
	r.reflect(heroUnk53, "heroUnk53");
	r.reflect(heroUnk54, "heroUnk54");
	r.reflect(heroUnk55, "heroUnk55");
	r.reflect(heroUnk56, "heroUnk56");
	r.reflect(heroUnk57, "heroUnk57");
	r.reflect(heroUnk58, "heroUnk58");
	r.reflect(heroUnk59, "heroUnk59");
	r.reflect(heroUnk60, "heroUnk60");
	r.reflect(heroUnk61, "heroUnk61");
	r.reflect(heroUnk62, "heroUnk62");
	r.reflect(heroUnk63, "heroUnk63");
	r.reflect(heroUnk64, "heroUnk64");
	r.reflect(heroUnk65, "heroUnk65");
	r.reflect(heroUnk66, "heroUnk66");
	r.reflect(heroUnk67, "heroUnk67");
	r.reflect(heroUnk68, "heroUnk68");
	r.reflect(heroUnk69, "heroUnk69");
	r.reflect(heroUnk70, "heroUnk70");
	r.reflect(heroUnk71, "heroUnk71");
	r.reflect(heroUnk72, "heroUnk72");
	r.reflect(heroUnk73, "heroUnk73");
	r.reflect(heroUnk74, "heroUnk74");
	r.reflect(heroUnk75, "heroUnk75");
	r.reflect(heroUnk76, "heroUnk76");
	r.reflect(heroUnk77, "heroUnk77");
	r.reflect(heroUnk78, "heroUnk78");
	r.reflect(heroUnk79, "heroUnk79");
	r.reflect(heroUnk80, "heroUnk80");
	r.reflect(heroUnk81, "heroUnk81");
	r.reflect(heroUnk82, "heroUnk82");
	r.reflect(heroUnk83, "heroUnk83");
	r.reflect(heroBranch3, "heroBranch3");
	r.reflect(heroUnk85, "heroUnk85");
	r.reflect(heroUnk86, "heroUnk86");
	r.reflect(heroUnk87, "heroUnk87");
	r.reflect(heroUnk88, "heroUnk88");
	r.reflect(heroUnk89, "heroUnk89");
	r.reflect(heroUnk90, "heroUnk90");
	r.reflect(heroUnk91, "heroUnk91");
	r.reflect(heroUnk92, "heroUnk92");
	r.reflect(heroUnk93, "heroUnk93");
	r.reflect(heroUnk94, "heroUnk94");
	r.reflect(heroUnk95, "heroUnk95");
	r.reflect(heroUnk96, "heroUnk96");
	r.reflect(heroUnk97, "heroUnk97");
	r.reflect(heroUnk98, "heroUnk98");
	r.reflect(heroUnk99, "heroUnk99");
	r.reflect(heroUnk100, "heroUnk100");
	r.reflect(heroUnk101, "heroUnk101");
	r.reflect(heroUnk102, "heroUnk102");
	r.reflect(heroUnk103, "heroUnk103");
	r.reflect(heroUnk104, "heroUnk104");
	r.reflect(heroUnk105, "heroUnk105");
	r.reflect(heroUnk106, "heroUnk106");
	r.reflect(heroUnk107, "heroUnk107");
	r.reflect(heroUnk108, "heroUnk108");
	r.reflect(heroUnk109, "heroUnk109");
	r.reflect(heroUnk110, "heroUnk110");
	r.reflect(heroUnk111, "heroUnk111");
	r.reflect(heroUnk112, "heroUnk112");
	r.reflect(heroUnk113, "heroUnk113");
	r.reflect(heroUnk114, "heroUnk114");
	r.reflect(heroUnk115, "heroUnk115");
	r.reflect(heroUnk116, "heroUnk116");
	r.reflect(heroUnk117, "heroUnk117");
	r.reflect(heroUnk118, "heroUnk118");
	r.reflect(heroUnk119, "heroUnk119");
	r.reflect(heroUnk120, "heroUnk120");
	r.reflect(heroUnk121, "heroUnk121");
	r.reflect(heroUnk122, "heroUnk122");
	r.reflect(heroUnk123, "heroUnk123");
	r.reflect(heroUnk124, "heroUnk124");
	r.reflect(heroUnk125, "heroUnk125");
	r.reflect(heroUnk126, "heroUnk126");
	r.reflect(heroUnk127, "heroUnk127");
	r.reflect(heroUnk128, "heroUnk128");
	r.reflect(heroUnk129, "heroUnk129");
	r.reflect(heroUnk130, "heroUnk130");
	r.reflect(heroUnk131, "heroUnk131");
	r.reflect(heroUnk132, "heroUnk132");
	r.reflect(heroUnk133, "heroUnk133");
	r.reflect(heroUnk134, "heroUnk134");
	r.reflect(heroUnk135, "heroUnk135");
	r.reflect(heroUnk136, "heroUnk136");
	r.reflect(heroUnk137, "heroUnk137");
	r.reflect(heroUnk138, "heroUnk138");
	r.reflect(heroUnk139, "heroUnk139");
	r.reflect(heroUnk140, "heroUnk140");
	r.reflect(heroUnk141, "heroUnk141");
	r.reflect(heroUnk142, "heroUnk142");
	r.reflect(heroUnk143, "heroUnk143");
	r.reflect(heroUnk144, "heroUnk144");
	r.reflect(heroUnk145, "heroUnk145");
	r.reflect(heroUnk146, "heroUnk146");
	r.reflect(heroUnk147, "heroUnk147");
	r.reflect(heroUnk148, "heroUnk148");
	r.reflect(heroUnk149, "heroUnk149");
	r.reflect(heroUnk150, "heroUnk150");
	r.reflect(heroUnk151, "heroUnk151");
	r.reflect(heroUnk152, "heroUnk152");
	r.reflect(heroUnk153, "heroUnk153");
	r.reflect(heroUnk154, "heroUnk154");
	r.reflect(heroUnk155, "heroUnk155");
	r.reflect(heroUnk156, "heroUnk156");
	r.reflect(heroUnk157, "heroUnk157");
	r.reflect(heroUnk158, "heroUnk158");
	r.reflect(heroUnk159, "heroUnk159");
	r.reflect(heroUnk160, "heroUnk160");
	r.reflect(heroUnk161, "heroUnk161");
	r.reflect(heroUnk162, "heroUnk162");
	r.reflect(heroUnk163, "heroUnk163");
	r.reflect(heroUnk164, "heroUnk164");
	r.reflect(heroUnk165, "heroUnk165");
	r.reflect(heroUnk166, "heroUnk166");
	r.reflect(heroUnk167, "heroUnk167");
	r.reflect(heroUnk168, "heroUnk168");
	r.reflect(heroUnk169, "heroUnk169");
	r.reflect(heroUnk170, "heroUnk170");
	r.reflect(heroUnk171, "heroUnk171");
	r.reflect(heroUnk172, "heroUnk172");
	r.reflect(heroUnk173, "heroUnk173");
	r.reflect(heroUnk174, "heroUnk174");
	r.reflect(heroUnk175, "heroUnk175");
	r.reflect(heroShadowCpnt, "heroShadowCpnt");
	r.reflect(heroWaterFxHook, "heroWaterFxHook");
	r.reflect(heroUnk178, "heroUnk178");
}
void CKHkAsterix::reflectMembers(MemberListener &r) {
	CKHkHero::reflectMembers(r);
	r.reflect(asteUnk0, "asteUnk0");
	r.reflect(asteUnk1, "asteUnk1");
	r.reflect(asteUnk2, "asteUnk2");
	r.reflect(asteUnk3, "asteUnk3");
	r.reflect(asteUnk4, "asteUnk4");
	r.reflect(asteUnk5, "asteUnk5");
	r.reflect(asteUnk6, "asteUnk6");
	r.reflect(asteUnk7, "asteUnk7");
	r.reflect(asteUnk8, "asteUnk8");
	r.reflect(asteUnk9, "asteUnk9");
	r.reflect(asteUnk10, "asteUnk10");
	r.reflect(asteUnk11, "asteUnk11");
	r.reflect(asteUnk12, "asteUnk12");
	r.reflect(asteUnk13, "asteUnk13");
	r.reflect(asteUnk14, "asteUnk14");
	r.reflect(asteUnk15, "asteUnk15");
	r.reflect(asteUnk16, "asteUnk16");
	r.reflect(asteUnk17, "asteUnk17");
	r.reflect(asteUnk18, "asteUnk18");
	r.reflect(asteUnk19, "asteUnk19");
	r.reflect(asteUnk20, "asteUnk20");
	r.reflect(asteUnk21, "asteUnk21");
	r.reflect(asteUnk22, "asteUnk22");
	r.reflect(asteUnk23, "asteUnk23");
	r.reflect(asteUnk24, "asteUnk24");
	r.reflect(asteDynSphere1, "asteDynSphere1");
	r.reflect(asteDynSphere2, "asteDynSphere2");
	r.reflect(asteBranch1, "asteBranch1");
	r.reflect(asteBranch2, "asteBranch2");
	r.reflect(asteUnk29, "asteUnk29");
	r.reflect(asteUnk30, "asteUnk30");
	r.reflect(asteSphere1, "asteSphere1");
	r.reflect(asteSphere2, "asteSphere2");
	r.reflect(asteBranch3, "asteBranch3");
	r.reflect(asteBranch4, "asteBranch4");
	r.reflect(asteParticleNode0, "asteParticleNode0");
	r.reflect(asteParticleNode1, "asteParticleNode1");
	r.reflect(asteParticleNode2, "asteParticleNode2");
	r.reflect(asteParticleNode3, "asteParticleNode3");
	r.reflect(asteParticleNode4, "asteParticleNode4");
	r.reflect(asteParticleNode5, "asteParticleNode5");
	r.reflect(asteParticleNode6, "asteParticleNode6");
	r.reflect(asteParticleNode7, "asteParticleNode7");
	r.reflect(asteParticleNode8, "asteParticleNode8");
	r.reflect(asteParticleNode9, "asteParticleNode9");
	r.reflect(asteParticleNode10, "asteParticleNode10");
	r.reflect(asteParticleNode11, "asteParticleNode11");
	r.reflect(asteParticleNode12, "asteParticleNode12");
	r.reflect(asteParticleNode13, "asteParticleNode13");
	r.reflect(asteParticleNode14, "asteParticleNode14");
	r.reflect(asteParticleNode15, "asteParticleNode15");
	r.reflect(asteParticleNode16, "asteParticleNode16");
	r.reflect(asteBranch5, "asteBranch5");
	r.reflect(asteBranch6, "asteBranch6");
}
void CKHkObelix::reflectMembers(MemberListener &r) {
	CKHkHero::reflectMembers(r);
	r.reflect(obeUnk0, "obeUnk0");
	r.reflect(obeDynSphere1, "obeDynSphere1");
	r.reflect(obeDynSphere2, "obeDynSphere2");
	r.reflect(obeBranchA1, "obeBranchA1");
	r.reflect(obeBranchA2, "obeBranchA2");
	r.reflect(obeUnk5, "obeUnk5");
	r.reflect(obeUnk6, "obeUnk6");
	r.reflect(obeUnk7, "obeUnk7");
	r.reflect(obeSphere1, "obeSphere1");
	r.reflect(obeSphere2, "obeSphere2");
	r.reflect(obeBranchB1, "obeBranchB1");
	r.reflect(obeBranchB2, "obeBranchB2");
	r.reflect(obeParticleNode0, "obeParticleNode0");
	r.reflect(obeParticleNode1, "obeParticleNode1");
	r.reflect(obeParticleNode2, "obeParticleNode2");
	r.reflect(obeParticleNode3, "obeParticleNode3");
	r.reflect(obeParticleNode4, "obeParticleNode4");
	r.reflect(obeParticleNode5, "obeParticleNode5");
	r.reflect(obeParticleNode6, "obeParticleNode6");
	r.reflect(obeParticleNode7, "obeParticleNode7");
	r.reflect(obeParticleNode8, "obeParticleNode8");
	r.reflect(obeParticleNode9, "obeParticleNode9");
	r.reflect(obeParticleNode10, "obeParticleNode10");
	r.reflect(obeParticleNode11, "obeParticleNode11");
	r.reflect(obeParticleNode12, "obeParticleNode12");
	r.reflect(obeParticleNode13, "obeParticleNode13");
	r.reflect(obeParticleNode14, "obeParticleNode14");
	r.reflect(obeParticleNode15, "obeParticleNode15");
	r.reflect(obeParticleNode16, "obeParticleNode16");
	r.reflect(obeParticleNode17, "obeParticleNode17");
	r.reflect(obeBranchE1, "obeBranchE1");
	r.reflect(obeBranchE2, "obeBranchE2");
	r.reflect(obeBranchE3, "obeBranchE3");
	r.reflect(obeBranchE4, "obeBranchE4");
	r.reflect(obeBranchE5, "obeBranchE5");
	r.reflect(obeBranchE6, "obeBranchE6");
}
void CKHkIdefix::reflectMembers(MemberListener &r) {
	CKHkHero::reflectMembers(r);
	r.reflect(ideUnk0, "ideUnk0");
	r.reflect(ideSphere, "ideSphere");
	r.reflect(ideBranch, "ideBranch");
	r.reflect(ideUnk3, "ideUnk3");
	r.reflect(ideUnk4, "ideUnk4");
	r.reflect(ideUnk5, "ideUnk5");
	r.reflect(ideUnk6, "ideUnk6");
	r.reflect(ideUnk7, "ideUnk7");
	r.reflect(ideParticleNode1, "ideParticleNode1");
	r.reflect(ideParticleNode2, "ideParticleNode2");
}
void CKHkMachinegun::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(mgunUnk0, "mgunUnk0");
	r.reflect(mgunUnk1, "mgunUnk1");
	r.reflect(mgunSpline, "mgunSpline");
	r.reflect(mgunUnk3, "mgunUnk3");
	r.reflect(mgunUnk4, "mgunUnk4");
	r.reflect(mgunUnk5, "mgunUnk5");
	r.reflect(mgunUnk6, "mgunUnk6");
	r.reflect(mgunUnk7, "mgunUnk7");
	r.reflect(mgunUnk8, "mgunUnk8");
	r.reflect(mgunUnk9, "mgunUnk9");
	r.reflect(mgunUnk10, "mgunUnk10");
	r.reflect(mgunUnk11, "mgunUnk11");
	r.reflect(mgunUnk12, "mgunUnk12");
	r.reflect(mgunUnk13, "mgunUnk13");
	r.reflect(mgunUnk14, "mgunUnk14");
	r.reflect(mgunUnk15, "mgunUnk15");
	r.reflect(mgunUnk16, "mgunUnk16");
	r.reflect(mgunUnk17, "mgunUnk17");
	r.reflect(mgunUnk18, "mgunUnk18");
	r.reflect(mgunUnk19, "mgunUnk19");
	r.reflect(mgunUnk20, "mgunUnk20");
	r.reflect(mgunUnk21, "mgunUnk21");
	r.reflect(mgunAnimDict, "mgunAnimDict");
	r.reflect(mgunSndDict, "mgunSndDict");
	r.reflect(mgunAnimNode, "mgunAnimNode");
	r.reflect(mgunNode, "mgunNode");
	r.reflect(mgunUnk26, "mgunUnk26");
	r.reflect(mgunParticles1, "mgunParticles1");
	r.reflect(mgunParticles2, "mgunParticles2");
	r.reflect(mgunParticles3, "mgunParticles3");
	r.reflect(mgunUnk30, "mgunUnk30");
	r.reflect(mgunUnk31, "mgunUnk31");
	r.reflect(mgunUnk32, "mgunUnk32");
	r.reflect(mgunUnk33, "mgunUnk33");
	r.reflect(mgunClones, "mgunClones");
	r.reflect(mgunUnk35, "mgunUnk35");
	r.reflect(mgunUnk36, "mgunUnk36");
	r.reflect(mgunUnk37, "mgunUnk37");
	r.reflect(mgunUnk38, "mgunUnk38");
	r.reflect(mgunUnk39, "mgunUnk39");
	r.reflect(mgunUnk40, "mgunUnk40");
	r.reflect(mgunBranch1, "mgunBranch1");
	r.reflect(mgunBranch2, "mgunBranch2");
	r.reflect(mgunBranch3, "mgunBranch3");
	r.reflect(mgunBranch4, "mgunBranch4");
	r.reflect(mgunUnk45, "mgunUnk45");
	r.reflect(mgunUnk46, "mgunUnk46");
	r.reflect(mgunUnk47, "mgunUnk47");
	r.reflect(mgunDynGround1, "mgunDynGround1");
	r.reflect(mgunDynGround2, "mgunDynGround2");
	r.reflect(mgunProjectile, "mgunProjectile");
	r.reflect(mgunUnk51, "mgunUnk51");
	r.reflect(mgunUnk52, "mgunUnk52");
	r.reflect(mgunShadowCpnt, "mgunShadowCpnt");
	r.reflect(mgunUnk54, "mgunUnk54");
	r.reflect(mgunUnk55, "mgunUnk55");
}
void CKHkDrawbridge::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(dbStaticGround, "dbStaticGround");
	r.reflect(dbDynGround, "dbDynGround");
	r.reflect(dbSndDict, "dbSndDict");
	r.reflect(dbEvtSeq, "dbEvtSeq", this);
	r.reflect(dbUnk4, "dbUnk4");
	r.reflect(dbUnk5, "dbUnk5");
	r.reflect(dbUnk6, "dbUnk6");
	r.reflect(dbUnk7, "dbUnk7");
	r.reflect(dbUnk8, "dbUnk8");
}
void CKHkMegaAshtray::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(maAnimDict, "maAnimDict");
	r.reflect(maSndDict, "maSndDict");
	r.reflect(maDynGround1, "maDynGround1");
	r.reflect(maDynGround2, "maDynGround2");
	r.reflect(maUnk4, "maUnk4");
	r.reflect(maUnk5, "maUnk5");
}
void CKHkCorkscrew::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(cswDynGround, "cswDynGround");
	r.reflect(cswSndDict, "cswSndDict");
	r.reflect(cswUnk2, "cswUnk2");
	r.reflect(cswUnk3, "cswUnk3", this);
	r.reflect(cswUnk4, "cswUnk4", this);
	r.reflect(cswUnk5, "cswUnk5");
	r.reflect(cswUnk6, "cswUnk6");
	r.reflect(cswUnk7, "cswUnk7");
	r.reflect(cswUnk8, "cswUnk8");
	r.reflect(cswUnk9, "cswUnk9");
	r.reflect(cswUnk10, "cswUnk10");
	r.reflect(cswUnk11, "cswUnk11");
}
void CKHkTurnstile::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(tsDynGround, "tsDynGround");
	r.reflect(tsSndDict, "tsSndDict");
	r.reflect(tsCylinder, "tsCylinder");
	r.reflect(tsUnk3, "tsUnk3", this);
	r.reflect(tsUnk4, "tsUnk4", this);
	r.reflect(tsUnk5, "tsUnk5", this);
	r.reflect(tsUnk6, "tsUnk6");
	r.reflect(tsUnk7, "tsUnk7");
	r.reflect(tsUnk8, "tsUnk8");
}
void CKHkLifter::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(liftSquad, "liftSquad");
	r.reflect(liftDynGround, "liftDynGround");
	r.reflect(liftStaticGround, "liftStaticGround");
	r.reflect(liftNode1, "liftNode1");
	r.reflect(liftNode2, "liftNode2");
	r.reflect(liftSndDict, "liftSndDict");
	r.reflect(liftUnk6, "liftUnk6");
	r.reflect(liftUnk7, "liftUnk7");
	r.reflect(liftUnk8, "liftUnk8");
	r.reflect(liftUnk9, "liftUnk9");
	r.reflect(liftUnk10, "liftUnk10");
	r.reflect(liftUnk11, "liftUnk11");
	r.reflect(liftUnk12, "liftUnk12", this);
	r.reflect(liftUnk13, "liftUnk13", this);
	r.reflect(liftUnk14, "liftUnk14", this);
	r.reflect(liftUnk15, "liftUnk15", this);
	r.reflect(liftUnk16, "liftUnk16", this);
	r.reflect(liftUnk17, "liftUnk17", this);
}
void CKHkRotaryBeam::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(rbAnimDict, "rbAnimDict");
	r.reflect(rbSndDict, "rbSndDict");
	r.reflect(rbUnk2, "rbUnk2");
	r.reflect(rbUnk3, "rbUnk3");
	r.reflect(rbUnk4, "rbUnk4");
	r.reflect(rbUnk5, "rbUnk5");
	r.reflect(rbUnk6, "rbUnk6");
	r.reflect(rbUnk7, "rbUnk7");
	r.reflect(rbUnk8, "rbUnk8");
	r.reflect(rbUnk9, "rbUnk9");
	r.reflect(rbUnk10, "rbUnk10");
	r.reflect(rbUnk11, "rbUnk11");
	r.reflect(rbUnk12, "rbUnk12");
}
void CKHkLightPillar::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
}
void CKHkWind::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(windSndDict, "windSndDict");
	r.reflect(windFogBoxes, "windFogBoxes");
	r.reflect(windUnk2, "windUnk2");
	r.reflect(windUnk3, "windUnk3");
	r.reflect(windUnk4, "windUnk4");
	r.reflect(windUnk5, "windUnk5");
	r.reflect(windUnk6, "windUnk6");
	r.reflect(windUnk7, "windUnk7", this);
	r.reflect(windUnk8, "windUnk8", this);
}
void CKHkPowderKeg::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(pkGround, "pkGround");
	r.reflect(pkCylinder, "pkCylinder");
	r.reflect(pkSndDict, "pkSndDict");
	r.reflect(pkUnk3, "pkUnk3");
	r.reflect(pkUnk4, "pkUnk4", this);
	r.reflect(pkUnk5, "pkUnk5", this);
	r.reflect(pkUnk6, "pkUnk6");
	r.reflect(pkUnk7, "pkUnk7");
	r.reflect(pkUnk8, "pkUnk8");
	r.reflect(pkUnk9, "pkUnk9");
}
void CKHkSwingDoor::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(swdSndDict, "swdSndDict");
	r.reflect(swdEvtSeq1, "swdEvtSeq1", this);
	r.reflect(swdEvtSeq2, "swdEvtSeq2", this);
	r.reflect(swdUnk3, "swdUnk3");
	r.reflect(swdUnk4, "swdUnk4");
	r.reflect(swdUnk5, "swdUnk5");
	r.reflect(swdUnk6, "swdUnk6");
	r.reflect(swdUnk7, "swdUnk7");
	r.reflect(swdNode1, "swdNode1");
	r.reflect(swdNode2, "swdNode2");
}
void CKHkSlideDoor::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(sldSndDict, "sldSndDict");
	r.reflect(sldEvtSeq1, "sldEvtSeq1", this);
	r.reflect(sldEvtSeq2, "sldEvtSeq2", this);
	r.reflect(sldUnk3, "sldUnk3");
	r.reflect(sldUnk4, "sldUnk4");
	r.reflect(sldUnk5, "sldUnk5");
	r.reflect(sldUnk6, "sldUnk6");
	r.reflect(sldUnk7, "sldUnk7");
	r.reflect(sldNode, "sldNode");
	r.reflect(sldDynGround, "sldDynGround");
	r.reflect(sldUnk10, "sldUnk10");
	r.reflect(sldUnk11, "sldUnk11");
}
void CKHkCrumblyZone::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(czSndDict, "czSndDict");
	r.reflect(czGround, "czGround");
	r.reflect(czNode1, "czNode1");
	r.reflect(czNode2, "czNode2");
	r.reflect(czObb, "czObb");
	r.reflect(czProjectileScrap, "czProjectileScrap");
	r.reflect(czParticleNode, "czParticleNode");
	r.reflect(czUnk7, "czUnk7");
	r.reflect(czUnk8, "czUnk8");
	r.reflect(czEvtSeqMaybe, "czEvtSeqMaybe");
	r.reflect(czEvtSeq2, "czEvtSeq2", this);
}
void CKHkHelmetCage::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(hcNode1, "hcNode1");
	r.reflect(hcClone1, "hcClone1");
	r.reflect(hcClone2, "hcClone2");
	r.reflect(hcDynGround1, "hcDynGround1");
	r.reflect(hcDynGround2, "hcDynGround2");
	r.reflect(hcSndDict, "hcSndDict");
	r.reflect(hcUnk6, "hcUnk6");
	r.reflect(hcUnk7, "hcUnk7");
	r.reflect(hcUnk8, "hcUnk8");
	r.reflect(hcUnk9, "hcUnk9");
	r.reflect(hcUnk10, "hcUnk10");
	r.reflect(hcUnk11, "hcUnk11");
	r.reflect(hcUnk12, "hcUnk12");
	r.reflect(hcUnk13, "hcUnk13");
	r.reflect(hcUnk14, "hcUnk14");
	r.reflect(hcUnk15, "hcUnk15");
	r.reflect(hcObb1, "hcObb1");
	r.reflect(hcObb2, "hcObb2");
}
void CKHkRollingStone::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(rlstPath, "rlstPath");
	r.reflect(rlstSphere, "rlstSphere");
	r.reflect(rlstProjScrap, "rlstProjScrap");
	r.reflect(rlstSndDict, "rlstSndDict");
	r.reflect(rlstClone, "rlstClone");
	r.reflect(rlstUnk5, "rlstUnk5");
	r.reflect(rlstUnk6, "rlstUnk6");
	r.reflect(rlstUnk7, "rlstUnk7");
	r.reflect(rlstUnk8, "rlstUnk8");
	r.reflect(rlstUnk9, "rlstUnk9");
	r.reflect(rlstUnk10, "rlstUnk10");
	r.reflect(rlstUnk11, "rlstUnk11");
	r.reflect(rlstUnk12, "rlstUnk12");
	r.reflect(rlstUnk13, "rlstUnk13");
	r.reflect(rlstUnk14, "rlstUnk14", this);
}
void CKHkRollingStone::onLevelLoaded(KEnvironment * kenv)
{
	if(rlstPath)
		rlstPath->usingSector = (this->life->unk1 >> 2) - 1;
}
void CKHkPushPullAsterix::Special::reflectMembers(MemberListener &r) {
	r.reflect(mUnk0, "mUnk0");
	r.reflect(mUnk1, "mUnk1");
	r.reflect(mUnk2, "mUnk2");
	r.reflect(mUnk3, "mUnk3");
	r.reflect(mUnk4, "mUnk4");
	r.reflect(mUnk5, "mUnk5");
	r.reflect(mUnk6, "mUnk6");
	r.reflect(mSndDict, "mSndDict");
}
void CKHkPushPullAsterix::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(ppaUnk0, "ppaUnk0");
	r.reflect(ppaUnk1, "ppaUnk1");
	r.reflect(ppaUnk2, "ppaUnk2");
	r.reflect(ppaUnk3, "ppaUnk3");
	r.reflect(ppaUnk4, "ppaUnk4");
	r.reflect(ppaUnk5, "ppaUnk5");
	r.reflect(ppaUnk6, "ppaUnk6");
	r.reflect(ppaUnk7, "ppaUnk7");
	r.reflect(ppaUnk8, "ppaUnk8");
	r.reflect(ppaUnk9, "ppaUnk9");
	r.reflect(ppaUnk10, "ppaUnk10");
	r.reflect(ppaUnk11, "ppaUnk11");
	r.reflect(ppaUnk12, "ppaUnk12");
	r.reflect(ppaFlaggedPath, "ppaFlaggedPath");
	r.reflect(ppaLine, "ppaLine");
	r.reflect(ppaUnk15, "ppaUnk15");
	r.reflect(ppaUnk16, "ppaUnk16");
	r.reflect(ppaUnk17, "ppaUnk17");
	r.reflect(ppaUnk18, "ppaUnk18");
	r.reflect(ppaUnk19, "ppaUnk19");
	r.reflect(ppaUnk20, "ppaUnk20");
	r.reflect(ppaUnk21, "ppaUnk21");
	r.reflect(ppaObb, "ppaObb");
	r.reflect(ppaUnk23, "ppaUnk23");
	r.reflect(ppaUnk24, "ppaUnk24");
	r.reflect(ppaUnk25, "ppaUnk25");
	r.reflect(ppaUnk26, "ppaUnk26");
	r.reflect(ppaParticleNode1, "ppaParticleNode1");
	r.reflect(ppaParticleNode2, "ppaParticleNode2");
	r.reflect(ppaUnk29, "ppaUnk29");
	r.reflect(ppaSndDict, "ppaSndDict");
	r.reflect(ppaDynGround, "ppaDynGround");
	r.reflect(ppaUnk32, "ppaUnk32", this);
	r.reflect(ppaUnk33, "ppaUnk33", this);
	r.reflect(ppaUnk34, "ppaUnk34", this);
	r.reflect(ppaUnk35, "ppaUnk35", this);
	r.reflect(ppaUnk36, "ppaUnk36");
	r.reflect(ppaUnk37, "ppaUnk37");
	r.reflect(ppaUnk38, "ppaUnk38");
	r.reflect(ppaBranch1, "ppaBranch1");
	r.reflect(ppaBranch2, "ppaBranch2");
	r.reflect(ppaBranch3, "ppaBranch3");
	r.reflect(ppaBranch4, "ppaBranch4");
	r.reflect(ppaUnk43, "ppaUnk43");
	r.reflect(ppaUnk44, "ppaUnk44");
}
void CKHkPushPullAsterix::onLevelLoaded(KEnvironment * kenv)
{
	if(ppaFlaggedPath)
		ppaFlaggedPath->cast<CKFlaggedPath>()->usingSector = (this->life->unk1 >> 2) - 1;
}
void CKHkBumper::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(bmpBranch, "bmpBranch");
	r.reflect(bmpAnimnode, "bmpAnimnode");
	r.reflect(bmpAnimDict, "bmpAnimDict");
	r.reflect(bmpSndDict, "bmpSndDict");
	r.reflect(bmpObb, "bmpObb");
	r.reflect(bmpGround, "bmpGround");
	r.reflect(bmpUnk6, "bmpUnk6");
	r.reflect(bmpUnk7, "bmpUnk7");
	r.reflect(bmpUnk8, "bmpUnk8");
	r.reflect(bmpUnk9, "bmpUnk9");
	r.reflect(bmpUnk10, "bmpUnk10");
}
void CKHkClueMan::Ing::reflectMembers(MemberListener &r) {
	r.reflect(mUnk0, "mUnk0");
	r.reflect(mCamera, "mCamera");
	r.reflect(mUnk2, "mUnk2");
	r.reflect(mUnk3, "mUnk3");
}
void CKHkClueMan::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(cmUnk0, "cmUnk0");
	r.reflect(cmUnk1, "cmUnk1");
	r.reflect(cmUnk2, "cmUnk2");
	r.reflect(cmUnk3, "cmUnk3");
	r.reflect(cmUnk4, "cmUnk4");
	r.reflect(cmUnk5, "cmUnk5");
	r.reflect(cmUnk6, "cmUnk6");
	r.reflect(cmUnk7, "cmUnk7");
	r.reflectSize<uint16_t>(cmBings, "sizeFor_cmBings");
	r.reflect(cmBings, "cmBings");
	r.reflectSize<uint16_t>(cmDings, "sizeFor_cmDings");
	r.reflect(cmDings, "cmDings");
	r.reflectSize<uint16_t>(cmFings, "sizeFor_cmFings");
	r.reflect(cmFings, "cmFings");
	r.reflect(cmUnk14, "cmUnk14");
	r.reflect(cmUnk15, "cmUnk15");
	r.reflect(cmUnk16, "cmUnk16");
	r.reflect(cmUnk17, "cmUnk17");
	r.reflect(cmUnk18, "cmUnk18");
	r.reflect(cmUnk19, "cmUnk19", this);
	r.reflect(cmUnk20, "cmUnk20", this);
	r.reflect(cmUnk21, "cmUnk21", this);
	r.reflect(cmUnk22, "cmUnk22", this);
	r.reflect(cmUnk23, "cmUnk23", this);
	r.reflect(cmUnk24, "cmUnk24");
	r.reflect(cmUnk25, "cmUnk25");
	r.reflect(cmUnk26, "cmUnk26");
	r.reflect(cmUnk27, "cmUnk27");
	r.reflect(cmUnk28, "cmUnk28");
	r.reflect(cmUnk29, "cmUnk29");
	r.reflect(cmUnk30, "cmUnk30");
	r.reflect(cmUnk31, "cmUnk31");
	r.reflect(cmUnk32, "cmUnk32");
	r.reflect(cmUnk33, "cmUnk33");
	r.reflect(cmUnk34, "cmUnk34");
	r.reflect(cmUnk35, "cmUnk35");
	r.reflect(cmUnk36, "cmUnk36");
	r.reflect(cmUnk37, "cmUnk37");
	r.reflect(cmBillboard1, "cmBillboard1");
	r.reflect(cmBillboard2, "cmBillboard2");
	r.reflect(cmUnk40, "cmUnk40");
	r.reflect(cmUnk41, "cmUnk41");
	r.reflect(cmUnk42, "cmUnk42");
	r.reflect(cmUnk43, "cmUnk43");
	r.reflect(cmUnk44, "cmUnk44");
	r.reflect(cmUnk45, "cmUnk45");
	r.reflect(cmUnk46, "cmUnk46");
	r.reflect(cmUnk47, "cmUnk47");
	r.reflect(cmUnk48, "cmUnk48");
	r.reflect(cmUnk49, "cmUnk49");
	r.reflect(cmUnk50, "cmUnk50");
	r.reflect(cmUnk51, "cmUnk51");
	r.reflect(cmUnk52, "cmUnk52");
	r.reflect(cmUnk53, "cmUnk53");
	r.reflect(cmUnk54, "cmUnk54");
	r.reflect(cmUnk55, "cmUnk55");
	r.reflect(cmUnk56, "cmUnk56");
	r.reflect(cmUnk57, "cmUnk57");
	r.reflect(cmUnk58, "cmUnk58");
	r.reflect(cmUnk59, "cmUnk59");
	r.reflect(cmUnk60, "cmUnk60");
}
void CKHkSky::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
}
void CKHkAsterixShop::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(shopAnimNode2, "shopAnimNode2");
	r.reflect(shopAnimDict1, "shopAnimDict1");
	r.reflect(shopAnimDict2, "shopAnimDict2");
	r.reflect(shopSndDict, "shopSndDict");
	r.reflect(shopBillboardList, "shopBillboardList");
	r.reflect(shopText1, "shopText1");
	r.reflect(shopText2, "shopText2");
	r.reflect(shopText3, "shopText3");
	r.reflect(shopText4, "shopText4");
	r.reflect(shopBillboard1, "shopBillboard1");
	r.reflect(shopBillboard2, "shopBillboard2");
	r.reflect(shopUnk11, "shopUnk11");
	r.reflect(shopUnk12, "shopUnk12");
	r.reflect(shopUnk13, "shopUnk13");
	r.reflect(shopUnk14, "shopUnk14");
	r.reflect(shopUnk15, "shopUnk15");
	r.reflect(shopUnk16, "shopUnk16");
	r.reflect(shopUnk17, "shopUnk17");
	r.reflect(shopUnk18, "shopUnk18");
	r.reflect(shopCameraTrack, "shopCameraTrack");
	r.reflect(shopUnk20, "shopUnk20");
	r.reflect(shopUnk21, "shopUnk21");
	r.reflect(shopUnk22, "shopUnk22");
	r.reflect(shopUnk23, "shopUnk23");
	r.reflect(shopUnk24, "shopUnk24");
	r.reflect(shopDynGround, "shopDynGround");
	r.reflect(shopUnk26, "shopUnk26");
	r.reflect(shopUnk27, "shopUnk27");
	r.reflect(shopUnk28, "shopUnk28");
	r.reflect(shopUnk29, "shopUnk29");
	r.reflect(shopUnk30, "shopUnk30");
	r.reflect(shopUnk31, "shopUnk31");
}
void CKHkWaterFall::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(wfallBranch2, "wfallBranch2");
	r.reflect(wfallUnk1, "wfallUnk1");
	r.reflect(wfallUnk2, "wfallUnk2");
}
void CKHkAsterixCheckpoint::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(acpNode, "acpNode");
	r.reflect(acpAnimDict, "acpAnimDict");
	r.reflect(acpSndDict, "acpSndDict");
	r.reflect(acpSphere1, "acpSphere1");
	r.reflect(acpSphere2, "acpSphere2");
	r.reflect(acpSphere3, "acpSphere3");
	r.reflect(acpParticleNode1, "acpParticleNode1");
	r.reflect(acpParticleNode2, "acpParticleNode2");
	r.reflect(acpGrpCheckpoint, "acpGrpCheckpoint");
	r.reflect(acpUnk9, "acpUnk9");
}
void CKHkBonusSpitter::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(bsDynGround, "bsDynGround");
	r.reflect(bsNode, "bsNode");
	r.reflect(bsUnk2, "bsUnk2");
	r.reflect(bsUnk3, "bsUnk3");
	r.reflect(bsUnk4, "bsUnk4");
	r.reflect(bsBonusType, "bsBonusType");
}
void CKHkTeleBridge::Part::reflectMembers(MemberListener &r) {
	r.reflect(mClone1, "mClone1");
	r.reflect(mDynGround, "mDynGround");
	r.reflect(mClone2, "mClone2");
	r.reflect(mClone3, "mClone3");
}
void CKHkTeleBridge::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(tbStaticGround, "tbStaticGround");
	r.reflect(tbSndDict, "tbSndDict");
	r.reflect(tbUnk2, "tbUnk2");
	r.reflect(tbUnk3, "tbUnk3");
	r.reflect(tbUnk4, "tbUnk4");
	r.reflect(tbUnk5, "tbUnk5");
	r.reflect(tbUnk6, "tbUnk6");
	r.reflect(tbUnk7, "tbUnk7");
	r.reflect(tbUnk8, "tbUnk8");
	r.reflect(tbUnk9, "tbUnk9");
	r.reflect(tbUnk10, "tbUnk10", this);
	r.reflect(tbUnk11, "tbUnk11", this);
	r.reflect(tbParts, "tbParts");
}
void CKHkTelepher::reflectMembers(MemberListener &r) {
	CKHook::reflectMembers(r);
	r.reflect(telUnk0, "telUnk0");
	r.reflect(telUnk1, "telUnk1");
	r.reflect(telUnk2, "telUnk2");
	r.reflect(telUnk3, "telUnk3");
	r.reflect(telFlaggedPath, "telFlaggedPath");
	r.reflect(telDynGround, "telDynGround");
	r.reflect(telUnk6, "telUnk6");
	r.reflect(telUnk7, "telUnk7");
	r.reflect(telUnk8, "telUnk8");
	r.reflect(telUnk9, "telUnk9");
	r.reflect(telUnk10, "telUnk10");
	r.reflect(telUnk11, "telUnk11");
	r.reflect(telUnk12, "telUnk12");
	r.reflect(telUnk13, "telUnk13");
	r.reflect(telUnk14, "telUnk14");
	r.reflect(telUnk15, "telUnk15");
	r.reflect(telUnk16, "telUnk16");
	r.reflect(telUnk17, "telUnk17");
	r.reflect(telUnk18, "telUnk18");
	r.reflect(telUnk19, "telUnk19");
	r.reflect(telUnk20, "telUnk20");
	r.reflect(telUnk21, "telUnk21", this);
	r.reflect(telSndDict, "telSndDict");
}
void CKHkTowedTelepher::reflectMembers(MemberListener &r) {
	CKHkTelepher::reflectMembers(r);
	r.reflect(towtelTowNode1, "towtelTowNode1");
	r.reflect(towtelTowNode2, "towtelTowNode2");
	r.reflect(towtelParticleNode, "towtelParticleNode");
	r.reflect(towtelUnk3, "towtelUnk3");
	r.reflect(towtelUnk4, "towtelUnk4");
	r.reflect(towtelUnk5, "towtelUnk5");
	r.reflect(towtelUnk6, "towtelUnk6");
	r.reflect(towtelUnk7, "towtelUnk7");
	r.reflect(towtelUnk8, "towtelUnk8");
	r.reflect(towtelUnk9, "towtelUnk9");
	r.reflect(towtelUnk10, "towtelUnk10");
	r.reflect(towtelUnk11, "towtelUnk11");
	r.reflect(towtelSphere, "towtelSphere");
	r.reflect(towtelUnk13, "towtelUnk13");
	r.reflect(towtelUnk14, "towtelUnk14", this);
	r.reflect(towtelUnk15, "towtelUnk15", this);
}
