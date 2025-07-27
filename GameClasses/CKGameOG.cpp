#include "CKGameOG.h"

#include "CoreClasses/CKNode.h"
#include "CoreClasses/CKDictionary.h"
#include "CoreClasses/CKLogic.h"

void GameOG::CKEnemySectorCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(numModels, "numModels");
	ckescSceneNodes.resize(numModels);
	ckescAnimDicts.resize(numModels);
	ckescBlenderControllers.resize(numModels);
	r.reflect(ckescSceneNodes, "ckescSceneNodes");
	r.reflect(ckescAnimDicts, "ckescAnimDicts");
	r.reflect(ckescBlenderControllers, "ckescBlenderControllers");
	r.reflectSize<uint8_t>(ckescUnk5, "ckescUnk5_size");
	r.reflect(ckescUnk5, "ckescUnk5");
	r.reflectSize<uint8_t>(ckescUnk13, "ckescUnk13_size");
	r.reflect(ckescUnk13, "ckescUnk13");
	r.reflectSize<uint8_t>(ckescUnused1, "ckescUnused1_size");
	r.reflect(ckescUnused1, "ckescUnused1");
	r.reflect(ckescSoundDict, "ckescSoundDict");
	r.reflect(ckescNumDunno, "ckescNumDunno");
	ckescUnused2.resize(ckescNumDunno);
	ckescUnused3.resize(ckescNumDunno);
	r.reflect(ckescUnused2, "ckescUnused2");
	r.reflect(ckescUnused3, "ckescUnused3");
}

void GameOG::CKHkA3Enemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKHook::reflectMembers2(r, kenv);
	r.reflectSize<uint8_t>(ckhaeEnemySectorCpnts, "ckhaeEnemySectorCpnts_size");
	r.reflect(ckhaeEnemySectorCpnts, "ckhaeEnemySectorCpnts");
	r.reflectSize<uint8_t>(ckhaeShortsList, "ckhaeShortsList_size");
	r.reflect(ckhaeShortsList, "ckhaeShortsList");
	r.reflectSize<uint8_t>(ckhaeUnk6, "ckhaeUnk6_size");
	r.reflect(ckhaeUnk6, "ckhaeUnk6");
	r.reflectSize<uint8_t>(ckhaeUnk7, "ckhaeUnk7_size");
	r.reflect(ckhaeUnk7, "ckhaeUnk7");
	r.reflectSize<uint8_t>(ckhaeBoundingShapes, "ckhaeBoundingShapes_size");
	r.reflect(ckhaeBoundingShapes, "ckhaeBoundingShapes");
	r.reflect(ckhaeUnk19, "ckhaeUnk19");
	ckhaeUnk20.resize(ckhaeUnk19);
	ckhaeUnk21.resize(ckhaeUnk19);
	ckhaeUnk22.resize(ckhaeBoundingShapes.size());
	r.reflect(ckhaeUnk20, "ckhaeUnk20");
	r.reflect(ckhaeUnk21, "ckhaeUnk21");
	r.reflect(ckhaeUnk22, "ckhaeUnk22");
	r.reflectSize<uint8_t>(ckhaeThings1, "ckhaeThings1_size");
	r.foreachElement(ckhaeThings1, "ckhaeThings1", [&](Thing1& t) {
		r.reflect(t.ckhaeUnk24, "ckhaeUnk24");
		r.reflect(t.ckhaeUnk25, "ckhaeUnk25");
		r.reflect(t.ckhaeUnk26, "ckhaeUnk26");
		r.reflect(t.ckhaeUnk27, "ckhaeUnk27");
		});
	r.foreachElement(ckhaeThings1, "ckhaeThings1_flt", [&](Thing1& t) {
		r.reflect(t.ckhaeUnk32, "ckhaeUnk32");
		});
	r.foreachElement(ckhaeThings1, "ckhaeThings1_vec", [&](Thing1& t) {
		r.reflect(t.ckhaeUnk33, "ckhaeUnk33");
		});
	r.reflectSize<uint8_t>(ckhaeParticleNodes, "ckhaeParticleNodes_size");
	r.reflect(ckhaeParticleNodes, "ckhaeParticleNodes");
	ckhaeUnk41.resize(ckhaeParticleNodes.size());
	r.reflect(ckhaeUnk41, "ckhaeUnk41");
	r.reflectSize<uint8_t>(ckhaeTrailNodes, "ckhaeTrailNodes_size");
	r.reflect(ckhaeTrailNodes, "ckhaeTrailNodes");
	r.reflect(ckhaeUnk46, "ckhaeUnk46");
	r.reflect(ckhaeUnk47, "ckhaeUnk47");
	r.reflect(ckhaeUnk48, "ckhaeUnk48");
	r.reflect(ckhaeUnk49, "ckhaeUnk49");
	r.reflect(ckhaeUnk50, "ckhaeUnk50");
	r.reflect(ckhaeUnk51, "ckhaeUnk51");
	r.reflect(ckhaeUnk52, "ckhaeUnk52");
	r.reflect(ckhaeUnk53, "ckhaeUnk53");
	r.reflect(ckhaeUnk54, "ckhaeUnk54");
	r.reflect(ckhaeUnk55, "ckhaeUnk55");
	r.reflect(ckhaeUnk56, "ckhaeUnk56");
	r.reflect(ckhaeUnk57, "ckhaeUnk57");
	r.reflect(ckhaeUnk58, "ckhaeUnk58");
	r.reflect(ckhaeUnk59, "ckhaeUnk59");
	r.reflect(ckhaeUnk60, "ckhaeUnk60");
	r.reflectSize<uint8_t>(ckhaeUnk62, "ckhaeUnk62_size");
	r.reflect(ckhaeUnk62, "ckhaeUnk62");
	r.reflect(ckhaeUnk63, "ckhaeUnk63");
	r.reflect(ckhaeUnk64, "ckhaeUnk64");
	r.reflect(ckhaeUnk65, "ckhaeUnk65");
	r.reflect(ckhaeUnk66, "ckhaeUnk66");
	r.reflect(ckhaeUnkString, "ckhaeUnkString");
	r.reflect(ckhaeUnk69, "ckhaeUnk69");
}

void GameOG::CKGrpA3Enemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	GameX2::IKGrpEnemy::reflectMembers2(r, kenv);
	r.reflect(quakeCpnt, "quakeCpnt");
	r.reflect(evt1, "evt1", this);
	r.reflect(evt2, "evt2", this);
	r.reflect(evt3, "evt3", this);
	r.reflect(evt4, "evt4", this);
	r.reflect(evt5, "evt5", this);
	r.reflect(evt6, "evt6", this);
	r.reflect(evt7, "evt7", this);
	r.reflect(evt8, "evt8", this);
}

void GameOG::CKA3EnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	GameX2::CKEnemyCpnt::reflectMembers2(r, kenv);
	r.reflect(ckaecUnk0, "ckaecUnk0");
	r.reflect(ckaecUnk1, "ckaecUnk1");
	r.reflect(ckaecUnk2, "ckaecUnk2");
	r.reflect(ckaecUnk3, "ckaecUnk3");
	r.reflect(ckaecUnk4, "ckaecUnk4");
	r.reflect(ckaecUnk5, "ckaecUnk5");
	r.reflect(ckaecUnk6, "ckaecUnk6");
	r.reflect(ckaecUnk7, "ckaecUnk7");
	r.reflect(ckaecUnk8, "ckaecUnk8");
	r.reflect(ckaecUnk9, "ckaecUnk9");
	r.reflect(ckaecUnk10, "ckaecUnk10");
	r.reflect(ckaecUnk11, "ckaecUnk11");
	r.reflect(ckaecUnk12, "ckaecUnk12");
	r.reflect(ckaecUnk13, "ckaecUnk13");
	r.reflect(ckaecUnk14, "ckaecUnk14");
	r.reflectSize<uint32_t>(seses, "seses_size");
	r.foreachElement(seses, "seses", [&](SES& s) {
		r.reflect(s.ckaecUnk28, "ckaecUnk28");
		r.reflect(s.ckaecUnk29, "ckaecUnk29");
		r.reflect(s.ckaecUnk30, "ckaecUnk30");
		r.reflect(s.ckaecUnk31, "ckaecUnk31");
		r.reflect(s.ckaecUnk32, "ckaecUnk32");
		r.reflect(s.ckaecUnk33, "ckaecUnk33");
		r.reflect(s.ckaecUnk34, "ckaecUnk34");
		r.reflect(s.ckaecUnk35, "ckaecUnk35");
		r.reflect(s.ckaecUnk36, "ckaecUnk36");
		r.reflect(s.ckaecUnk37, "ckaecUnk37");
		r.reflect(s.ckaecUnk38, "ckaecUnk38");
		r.reflect(s.ckaecUnk39, "ckaecUnk39");
		});
	r.reflect(ckaecUnk40, "ckaecUnk40");
	r.reflect(ckaecUnk41, "ckaecUnk41");
	r.reflect(ckaecUnk42, "ckaecUnk42");
	r.reflect(ckaecUnk43, "ckaecUnk43");
	r.reflect(ckaecUnk44, "ckaecUnk44");
	r.reflect(ckaecUnk45, "ckaecUnk45");
	r.reflect(ckaecUnk46, "ckaecUnk46");
	r.reflect(ckaecUnk47, "ckaecUnk47");
	r.reflect(ckaecUnk48, "ckaecUnk48");
	r.reflect(ckaecUnk49, "ckaecUnk49");
	r.reflect(ckaecUnk50, "ckaecUnk50");
	r.reflect(ckaecUnk51, "ckaecUnk51");
	r.reflect(ckaecUnk52, "ckaecUnk52");
	r.reflect(ckaecUnk53, "ckaecUnk53");
	r.reflect(ckaecUnk54, "ckaecUnk54");
	r.reflect(ckaecUnk55, "ckaecUnk55");
	r.reflect(ckaecUnk56, "ckaecUnk56");
	r.reflect(ckaecUnk57, "ckaecUnk57");
	r.reflect(ckaecUnk58, "ckaecUnk58");
	r.reflect(ckaecUnk59, "ckaecUnk59");
	r.reflect(ckaecUnk60, "ckaecUnk60");
	r.reflect(ckaecUnk61, "ckaecUnk61");
	r.reflect(ckaecUnk62, "ckaecUnk62");
	r.reflect(ckaecUnk63, "ckaecUnk63");
	r.reflect(ckaecUnk64, "ckaecUnk64");
	r.reflect(ckaecUnk65, "ckaecUnk65");
	r.reflect(ckaecUnk66, "ckaecUnk66");
	r.reflect(ckaecUnk67, "ckaecUnk67");
	r.reflect(ckaecUnk68, "ckaecUnk68");
	r.reflect(ckaecUnk69, "ckaecUnk69");
	r.reflect(ckaecUnk70, "ckaecUnk70");
	r.reflect(ckaecUnk71, "ckaecUnk71");
	r.reflect(ckaecUnk72, "ckaecUnk72");
	r.reflect(ckaecUnk73, "ckaecUnk73");
	r.reflect(ckaecUnk74, "ckaecUnk74");
	r.reflect(ckaecUnk75, "ckaecUnk75");
}
