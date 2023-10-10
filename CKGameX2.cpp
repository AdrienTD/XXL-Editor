#include "CKGameX2.h"
#include "CKComponent.h"
#include "CKNode.h"
#include "CKDictionary.h"
#include "CKLogic.h"
#include "CKCamera.h"
#include "CKGraphical.h"

namespace GameX2 {
	void CKHkBonusSpitter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhbsUnk0, "ckhbsUnk0");
		r.reflect(ckhbsUnk1, "ckhbsUnk1");
		r.reflect(ckhbsUnk2, "ckhbsUnk2");
		r.reflect(ckhbsUnk3, "ckhbsUnk3");
		r.reflect(ckhbsUnk4, "ckhbsUnk4");
	}

	void CKHkActivator::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhaUnk0, "ckhaUnk0");
		r.reflect(ckhaUnk1, "ckhaUnk1");
		r.reflect(ckhaUnk2, "ckhaUnk2");
		r.reflect(ckhaUnk3, "ckhaUnk3");
		r.reflect(ckhaUnk4, "ckhaUnk4");
		r.reflect(ckhaUnk5, "ckhaUnk5");
		r.reflect(ckhaUnk6, "ckhaUnk6");
		r.reflect(ckhaUnk7, "ckhaUnk7");
		r.reflect(ckhaUnk8, "ckhaUnk8");
		r.reflect(ckhaUnk9, "ckhaUnk9", this);
	}

	void CKHkDoor::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhdUnk0, "ckhdUnk0");
		r.message("DRM");
		r.reflect(ckhdNonFinalFloat, "ckhdNonFinalFloat");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ckhdUnk0_C, "ckhdUnk0_C");
		}
		r.reflect(ckhdUnk1, "ckhdUnk1");
		r.reflectSize<uint8_t>(ckhdDetectors, "ckhdDetectors_size");
		r.reflect(ckhdDetectors, "ckhdDetectors");
		r.reflect(ckhdUnk3, "ckhdUnk3");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhdUnk4, "ckhdUnk4");
			r.reflect(ckhdUnk5, "ckhdUnk5");
		} 
		else if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ckhdGround, "ckhdGround");
		}
		r.reflect(ckhdUnk6, "ckhdUnk6");
		r.reflect(ckhdUnk7, "ckhdUnk7");
		r.reflect(ckhdUnk8, "ckhdUnk8");
		r.reflect(ckhdUnk9, "ckhdUnk9", this);
		r.reflect(ckhdUnk10, "ckhdUnk10", this);
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(pfxData, "pfxData");
		}
	}

	void CKHkCrumblyZone::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhczUnk0, "ckhczUnk0");
		r.reflect(ckhczUnk1, "ckhczUnk1");
		r.reflect(ckhczUnk2, "ckhczUnk2");
		r.reflect(ckhczUnk3, "ckhczUnk3");
		if (kenv->version <= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ckhczUnk4, "ckhczUnk4");
		}
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhczUnk5, "ckhczUnk5");
		}
		else if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogProjectileAcc, "ogProjectileAcc");
		}
		r.reflect(ckhczUnk6, "ckhczUnk6");
		r.reflect(ckhczUnk7, "ckhczUnk7");
		r.reflect(ckhczUnk8, "ckhczUnk8");
		r.reflect(ckhczUnk9, "ckhczUnk9");
		r.reflect(ckhczUnk10, "ckhczUnk10");
		r.reflect(ckhczUnk11, "ckhczUnk11");
		r.reflect(ckhczUnk12, "ckhczUnk12", this);
		r.reflect(ckhczUnk13, "ckhczUnk13", this);
		r.reflect(ckhczUnk14, "ckhczUnk14");
		r.reflect(ckhczUnk15, "ckhczUnk15");
		r.reflect(ckhczUnk16, "ckhczUnk16");
		r.reflect(ckhczUnk17, "ckhczUnk17");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhczUnk18, "ckhczUnk18");
			r.reflect(ckhczUnk19, "ckhczUnk19");
		}
		else if (kenv->version == KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogMidUnk1, "ogMidUnk1");
			r.reflect(ckhczUnk18, "ckhczUnk18");
			r.reflect(ckhczUnk19, "ckhczUnk19");
		}
		else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogMidUnk1, "ogMidUnk1");
			r.reflect(ogMidUnk2, "ogMidUnk2");
			r.reflect(ogMidUnk3, "ogMidUnk3");
		}
		r.reflect(ckhczUnk20, "ckhczUnk20");
		r.reflect(ckhczUnk21, "ckhczUnk21");
		r.reflect(ckhczUnk22, "ckhczUnk22");
		r.reflect(ckhczUnk23, "ckhczUnk23");
		r.reflect(ckhczUnk24, "ckhczUnk24");
		r.reflect(ckhczUnk25, "ckhczUnk25");
		r.reflectSize<uint32_t>(ckhczGroundList1, "ckhczGroundList1_size");
		r.reflect(ckhczGroundList1, "ckhczGroundList1");
		r.reflectSize<uint32_t>(ckhczGroundList2, "ckhczGroundList2_size");
		r.reflect(ckhczGroundList2, "ckhczGroundList2");
		r.reflect(ckhczUnk28, "ckhczUnk28");
		r.reflect(ckhczUnk29, "ckhczUnk29");
		r.reflect(ckhczUnk30, "ckhczUnk30");
		r.reflect(ckhczUnk31, "ckhczUnk31");
		r.reflect(ckhczUnk32, "ckhczUnk32");
		r.reflect(ckhczUnk33, "ckhczUnk33");
		r.reflect(ckhczUnk34, "ckhczUnk34");
		r.reflect(ckhczUnk35, "ckhczUnk35");
		r.reflect(ckhczUnk36, "ckhczUnk36");
		r.reflect(ckhczUnk37, "ckhczUnk37");
		r.reflect(ckhczUnk38, "ckhczUnk38");
		r.reflect(ckhczUnk39, "ckhczUnk39");
		r.reflect(ckhczUnk40, "ckhczUnk40");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogUnk01, "ogUnk01");
			r.reflect(ogUnk02, "ogUnk02");
			r.reflect(ogUnk03, "ogUnk03");
			r.reflect(ogUnk04, "ogUnk04");
			r.reflect(ogUnk05, "ogUnk05");
			r.reflect(ogUnk06, "ogUnk06");
			r.reflect(ogUnk07, "ogUnk07");
			r.reflectSize<uint32_t>(ogHotSpots, "ogHotSpots_size");
			r.reflect(ogHotSpots, "ogHotSpots");
			r.reflect(ogUnk08, "ogUnk08");
			r.reflect(ogUnk09, "ogUnk09");
		}
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogBonusData, "ogBonusData");
			r.reflectSize<uint8_t>(ogNeighbours, "ogNeighbours_size");
			r.reflect(ogNeighbours, "ogNeighbours");
			r.reflect(ogUnk10, "ogUnk10");
			r.reflect(ogUnk11, "ogUnk11");
			r.reflectSize<uint32_t>(ogProjectileAccessors, "ogProjectileAccessors_size");
			r.reflect(ogProjectileAccessors, "ogProjectileAccessors");
			r.reflectSize<uint32_t>(ogHotSpotInfo, "ogHotSpotInfo_size");
			r.foreachElement(ogHotSpotInfo, "ogHotSpotInfo", [&](auto& s) {
				r.reflect(s.first, "first");
				r.reflect(s.second, "second");
				});
			r.reflectSize<uint32_t>(ogBoxes, "ogBoxes_size");
			r.foreachElement(ogBoxes, "ogBoxes", [&](auto& s) {
				r.reflect(s.first, "first");
				r.reflect(s.second, "second");
				});
			r.reflect(ogExplosionFxData, "ogExplosionFxData");
		}
	}

	void CKHkA2PotionStone::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhapsUnk0, "ckhapsUnk0");
		r.reflect(ckhapsUnk1, "ckhapsUnk1");
		r.reflect(ckhapsUnk2, "ckhapsUnk2");
		r.reflect(ckhapsUnk3, "ckhapsUnk3");
		r.reflect(ckhapsUnk4, "ckhapsUnk4");
		r.reflect(ckhapsUnk5, "ckhapsUnk5");
		r.reflect(ckhapsUnk6, "ckhapsUnk6", this);
		r.reflectSize<uint8_t>(aaas, "aaas_size");
		r.foreachElement(aaas, "aaas", [&](AAA& a) {
			r.reflect(a.ckhapsUnk8, "ckhapsUnk8");
			r.reflect(a.ckhapsUnk9, "ckhapsUnk9");
			r.reflect(a.ckhapsUnk10, "ckhapsUnk10");
			r.reflect(a.ckhapsUnk11, "ckhapsUnk11");
			});
		r.reflect(ckhapsUnk20, "ckhapsUnk20");
	}

	void CKHkPlatform::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			ckhpGrounds.resize(2);
			r.reflect(ckhpGrounds, "ckhpGrounds");
		}
		else if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflectSize<uint8_t>(ckhpGrounds, "ckhpGrounds_size");
			r.reflect(ckhpGrounds, "ckhpGrounds");
		}
		r.reflect(ckhpSpline, "ckhpSpline");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
			r.reflect(ogUnkFloat, "ogUnkFloat");
		r.reflect(ckhpUnk3, "ckhpUnk3");
		r.reflect(ckhpUnk4, "ckhpUnk4");
		r.reflect(ckhpUnk5, "ckhpUnk5");
		r.reflect(ckhpUnk6, "ckhpUnk6");
		r.reflect(ckhpUnk7, "ckhpUnk7", this);
		r.reflect(ckhpUnk8, "ckhpUnk8", this);
	}

	void CKHkCorridorEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhceUnk0, "ckhceUnk0");
		r.reflect(ckhceUnk1, "ckhceUnk1");
		r.reflect(ckhceUnk2, "ckhceUnk2");
		r.reflect(ckhceUnk3, "ckhceUnk3");
		r.reflect(ckhceUnk4, "ckhceUnk4");
		r.reflect(ckhceUnk5, "ckhceUnk5");
		r.reflect(ckhceUnk6, "ckhceUnk6");
		r.reflect(ckhceUnk7, "ckhceUnk7");
		r.reflect(ckhceUnk8, "ckhceUnk8");
		r.reflect(ckhceUnk9, "ckhceUnk9");
		r.reflect(ckhceUnk10, "ckhceUnk10");
		r.reflect(ckhceUnk11, "ckhceUnk11");
		r.reflect(ckhceUnk12, "ckhceUnk12");
		r.reflect(ckhceUnk13, "ckhceUnk13");
		r.reflect(ckhceUnk14, "ckhceUnk14");
		r.reflect(ckhceUnk15, "ckhceUnk15", this);
		r.reflectSize<uint32_t>(ckhceUnk18, "ckhceUnk18_size");
		r.reflect(ckhceUnk18, "ckhceUnk18");
		r.reflect(ckhceUnk38, "ckhceUnk38");
		r.reflect(ckhceUnk39, "ckhceUnk39");
		r.reflect(ckhceUnk40, "ckhceUnk40");
		r.reflect(ckhceUnk41, "ckhceUnk41");
		r.reflect(ckhceUnk42, "ckhceUnk42");
		r.reflect(ckhceUnk43, "ckhceUnk43");
		r.reflect(ckhceUnk44, "ckhceUnk44");
		r.reflect(ckhceUnk45, "ckhceUnk45");
		r.reflectSize<uint32_t>(moves, "moves_size");
		r.foreachElement(moves, "moves", [&](Move& m) {
			r.reflect(m.ckhceUnk47, "ckhceUnk47");
			r.reflect(m.ckhceUnk48, "ckhceUnk48");
			r.reflect(m.ckhceUnk49, "ckhceUnk49");
			});
		r.reflect(ckhceUnk92, "ckhceUnk92");
	}


	void CKHkParticlesSequencer::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhpsUnk0, "ckhpsUnk0");
		r.reflect(ckhpsUnk1, "ckhpsUnk1", this);
		r.reflect(ckhpsUnk2, "ckhpsUnk2");
	}

	void CKHkA2BossTrap::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhabtUnk0, "ckhabtUnk0");
		r.reflect(ckhabtUnk1, "ckhabtUnk1");
		r.reflect(ckhabtUnk2, "ckhabtUnk2");
		r.reflect(ckhabtUnk3, "ckhabtUnk3");
		explosionData.reflectMembers2(r, kenv);
		r.reflect(ckhabtUnk16, "ckhabtUnk16");
		r.reflect(ckhabtUnk17, "ckhabtUnk17");
		r.reflect(ckhabtUnk18, "ckhabtUnk18");
		r.reflect(ckhabtUnk19, "ckhabtUnk19");
		r.reflect(ckhabtUnk20, "ckhabtUnk20");
		r.reflect(ckhabtUnk21, "ckhabtUnk21");
		r.reflect(ckhabtUnk22, "ckhabtUnk22");
		r.reflect(ckhabtUnk23, "ckhabtUnk23");
		r.reflect(ckhabtUnk24, "ckhabtUnk24");
		r.reflect(ckhabtUnk25, "ckhabtUnk25");
		r.reflectSize<uint32_t>(ckhabtUnk27, "ckhabtUnk27_size");
		r.reflect(ckhabtUnk27, "ckhabtUnk27");
	}
	void CKHkA2DeathFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.enterStruct("flashFxData1");
		flashFxData1.reflectMembers2(r, kenv);
		r.leaveStruct();
		r.enterStruct("flashFxData2");
		flashFxData2.reflectMembers2(r, kenv);
		r.leaveStruct();
		r.enterStruct("electricArcFxData");
		electricArcFxData.reflectMembers2(r, kenv);
		r.leaveStruct();
		r.reflect(ckhadfUnk31, "ckhadfUnk31");
		r.reflect(ckhadfUnk32, "ckhadfUnk32");
		r.reflect(ckhadfUnk33, "ckhadfUnk33");
		r.reflect(ckhadfUnk34, "ckhadfUnk34");
		r.reflect(ckhadfUnk35, "ckhadfUnk35");
		r.reflect(ckhadfUnk36, "ckhadfUnk36");
		r.reflect(ckhadfUnk37, "ckhadfUnk37");
		r.reflect(ckhadfUnk38, "ckhadfUnk38");
		r.reflect(ckhadfUnk39, "ckhadfUnk39");
		r.reflect(ckhadfUnk40, "ckhadfUnk40");
		r.reflect(ckhadfUnk41, "ckhadfUnk41");
		r.reflect(ckhadfUnk42, "ckhadfUnk42");
		r.reflect(ckhadfUnk43, "ckhadfUnk43");
		r.reflect(ckhadfUnk44, "ckhadfUnk44");
		r.reflect(ckhadfUnk45, "ckhadfUnk45");
		r.reflect(ckhadfBlurFxData, "ckhadfBlurFxData");
	}
	void CKHkA2Boss::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhabUnk0, "ckhabUnk0");
		r.reflect(ckhabUnk1, "ckhabUnk1");
		r.reflect(ckhabUnk6, "ckhabUnk6");
		r.reflect(ckhabUnk10, "ckhabUnk10");
		r.reflect(ckhabUnk25, "ckhabUnk25");
		r.reflect(ckhabUnk47, "ckhabUnk47");
		r.reflect(ckhabUnk48, "ckhabUnk48");
		r.reflect(ckhabUnk49, "ckhabUnk49");
		r.reflect(ckhabUnk50, "ckhabUnk50");
		r.reflect(ckhabUnk51, "ckhabUnk51");
		r.reflect(ckhabUnk52, "ckhabUnk52");
		r.reflect(ckhabUnk53, "ckhabUnk53");
		r.reflect(ckhabUnk54, "ckhabUnk54");
		r.reflect(ckhabUnk55, "ckhabUnk55");
		r.reflect(ckhabUnk56, "ckhabUnk56");
		r.reflect(ckhabUnk57, "ckhabUnk57");
		r.reflect(ckhabUnk58, "ckhabUnk58");
		r.reflect(ckhabUnk59, "ckhabUnk59");
		r.reflect(ckhabUnk60, "ckhabUnk60");
		r.reflect(ckhabUnk61, "ckhabUnk61");
		r.reflect(ckhabUnk62, "ckhabUnk62");
		r.reflect(ckhabUnk63, "ckhabUnk63");
		r.reflect(ckhabUnk64, "ckhabUnk64");
		r.reflect(ckhabUnk65, "ckhabUnk65");
		r.reflect(ckhabUnk66, "ckhabUnk66");
		r.reflect(ckhabUnk67, "ckhabUnk67");
		r.reflect(ckhabUnk68, "ckhabUnk68");
		r.reflect(ckhabUnk69, "ckhabUnk69");
		r.reflect(ckhabUnk70, "ckhabUnk70");
		r.reflect(ckhabUnk71, "ckhabUnk71");
		r.reflect(ckhabUnk72, "ckhabUnk72");
		r.reflect(ckhabUnk73, "ckhabUnk73", this);
		r.reflect(ckhabUnk76, "ckhabUnk76", this);
		r.reflect(ckhabUnk80, "ckhabUnk80", this);
		r.reflect(ckhabUnk82, "ckhabUnk82", this);
		r.reflect(ckhabUnk83, "ckhabUnk83", this);
		r.reflect(ckhabUnk84, "ckhabUnk84", this);
		r.reflect(ckhabUnk86, "ckhabUnk86", this);
		r.reflect(ckhabUnk87, "ckhabUnk87", this);
		r.reflect(ckhabUnk89, "ckhabUnk89");
		r.message("DRM");
		r.reflect(ckhabUnk90RomasterDuplicate, "ckhabUnk90RomasterDuplicate");
		r.reflect(ckhabUnk90, "ckhabUnk90");
		r.reflect(ckhabUnk91, "ckhabUnk91");
		r.reflect(ckhabUnk92, "ckhabUnk92");
		r.reflect(ckhabUnk93, "ckhabUnk93");
		r.reflect(ckhabUnk94, "ckhabUnk94");
		r.reflect(ckhabUnk95, "ckhabUnk95");
		r.reflect(ckhabUnk96, "ckhabUnk96");
		r.reflect(ckhabUnk97, "ckhabUnk97");
		r.reflect(ckhabUnk98, "ckhabUnk98");
		r.reflect(ckhabUnk99, "ckhabUnk99");
		r.reflect(ckhabUnk100, "ckhabUnk100");
		r.reflect(ckhabUnk101, "ckhabUnk101");
		r.reflect(ckhabUnk102, "ckhabUnk102");
		r.reflect(ckhabUnk103, "ckhabUnk103");
		r.reflect(ckhabUnk104, "ckhabUnk104");
		r.reflect(ckhabUnk105, "ckhabUnk105");
		r.reflect(ckhabUnk106, "ckhabUnk106");
		r.reflect(ckhabUnk107, "ckhabUnk107");
		r.reflect(ckhabUnk108, "ckhabUnk108");

		r.foreachElement(shockWaveFxDatas, "shockWaveFxDatas", [&](auto& inst) {
			r.reflectComposed(inst, "inst", kenv);
			});

		r.reflect(ckhabUnk151, "ckhabUnk151");
		r.reflect(ckhabUnk152, "ckhabUnk152");

		r.foreachElement(cameraQuakeDatas, "cameraQuakeDatas", [&](auto& inst) {
			r.reflectComposed(inst, "inst", kenv);
			});
		r.reflectComposed(fireBallData, "fireBallData", kenv);
		r.foreachElement(explosionDatas, "explosionDatas", [&](auto& inst) {
			r.reflectComposed(inst, "inst", kenv);
			});
		r.reflectComposed(flashData, "flashData", kenv);
		r.foreachElement(electricArcDatas, "electricArcDatas", [&](auto& inst) {
			r.reflectComposed(inst, "inst", kenv);
			});

		r.reflect(ckhabUnk266, "ckhabUnk266");
		r.reflect(ckhabUnk267, "ckhabUnk267");

		r.reflectComposed(powerBallData, "powerBallData", kenv);

		r.reflect(ckhabUnk292, "ckhabUnk292");
		r.reflect(ckhabUnk293, "ckhabUnk293");
		r.reflect(ckhabUnk294, "ckhabUnk294");
		r.reflect(ckhabUnk295, "ckhabUnk295");
		r.reflect(ckhabUnk296, "ckhabUnk296");
		r.reflect(ckhabUnk297, "ckhabUnk297");
		r.reflect(ckhabUnk298, "ckhabUnk298");
		r.reflect(ckhabUnk299, "ckhabUnk299");
		r.reflect(ckhabUnk300, "ckhabUnk300");
		r.reflect(ckhabUnk301, "ckhabUnk301");
		r.reflect(ckhabUnk302, "ckhabUnk302");
		r.reflect(ckhabUnk303, "ckhabUnk303");
		r.reflect(ckhabUnk304, "ckhabUnk304");
		r.reflect(ckhabUnk305, "ckhabUnk305");
		r.reflect(ckhabUnk306, "ckhabUnk306");
		r.reflect(ckhabUnk307, "ckhabUnk307");
		r.reflect(ckhabUnk308, "ckhabUnk308");
		r.reflect(ckhabUnk309, "ckhabUnk309");
		r.reflect(ckhabUnk310, "ckhabUnk310");
		r.reflect(ckhabUnk311, "ckhabUnk311");
		r.reflect(ckhabUnk312, "ckhabUnk312");
		r.reflect(ckhabUnk313, "ckhabUnk313");
		r.reflect(ckhabUnk314, "ckhabUnk314");
		r.reflect(ckhabUnk315, "ckhabUnk315");
		r.reflect(ckhabUnk316, "ckhabUnk316");
		r.reflect(ckhabUnk317, "ckhabUnk317");
	}
	void CKHkA2Hero::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhahUnk0, "ckhahUnk0");
		r.reflect(ckhahHeroIndex, "ckhahHeroIndex");
		r.reflect(ckhahUnk2, "ckhahUnk2");
		r.reflect(ckhahUnk3, "ckhahUnk3");
		r.reflect(ckhahUnk4, "ckhahUnk4");
		r.reflect(ckhahUnk5, "ckhahUnk5");
		r.reflect(ckhahUnk6, "ckhahUnk6");
		r.reflect(ckhahUnk7, "ckhahUnk7");
		r.reflectSize<uint8_t>(ckhahUnk9, "ckhahUnk9_size");
		r.reflect(ckhahUnk9, "ckhahUnk9");
		r.reflectSize<uint8_t>(ckhahUnk18, "ckhahUnk18_size");
		r.reflect(ckhahUnk18, "ckhahUnk18");
		r.reflectSize<uint8_t>(ckhahUnk30, "ckhahUnk30_size");
		r.reflect(ckhahUnk30, "ckhahUnk30");
		r.reflect(ckhahUnk37, "ckhahUnk37");
		assert(ckhahUnk37 == 0);
		r.reflectSize<uint8_t>(ckhahUnk39, "ckhahUnk39_size");
		r.reflect(ckhahUnk39, "ckhahUnk39");
		r.reflect(ckhahUnk45, "ckhahUnk45");
		r.reflect(ckhahUnk46, "ckhahUnk46");
		r.reflect(ckhahUnk47, "ckhahUnk47");
		r.reflect(ckhahUnk48, "ckhahUnk48");
		r.reflect(ckhahUnk49, "ckhahUnk49");

		r.reflectComposed(waterWaveFxData, "waterWaveFxData", kenv);
		r.reflectComposed(waterSplashFxData, "waterSplashFxData", kenv);

		r.reflect(ckhahUnk67, "ckhahUnk67");
		r.reflect(ckhahUnk68, "ckhahUnk68");
		r.reflect(ckhahUnk69, "ckhahUnk69");
		r.reflect(ckhahUnk70, "ckhahUnk70");
		r.reflect(ckhahUnk71, "ckhahUnk71");
		r.reflect(ckhahUnk72, "ckhahUnk72");
		r.reflect(ckhahUnk73, "ckhahUnk73");
		r.reflect(ckhahUnk74, "ckhahUnk74");
		r.reflect(ckhahUnk75, "ckhahUnk75");
		r.reflect(ckhahUnk76, "ckhahUnk76");
		r.reflect(ckhahUnk77, "ckhahUnk77");
		r.reflect(ckhahUnk78, "ckhahUnk78");
		r.reflect(ckhahUnk79, "ckhahUnk79");
		r.reflect(ckhahUnk80, "ckhahUnk80");
		r.reflect(ckhahUnk81, "ckhahUnk81");
		r.reflect(ckhahUnk82, "ckhahUnk82");
		r.reflect(ckhahUnk83, "ckhahUnk83");
		r.reflect(ckhahUnk84, "ckhahUnk84");
		r.reflect(ckhahUnk85, "ckhahUnk85");
		r.reflect(ckhahUnk86, "ckhahUnk86");
		r.reflect(ckhahUnk87, "ckhahUnk87");

		unkStructs1.resize((ckhahHeroIndex == 2) ? 6 : 8);
		r.foreachElement(unkStructs1, "unkStructs1", [&](UnkStruct1& s) {
			r.reflect(s.ckhahUnk88, "ckhahUnk88");
			r.reflect(s.ckhahUnk89, "ckhahUnk89");
			r.reflect(s.ckhahUnk90, "ckhahUnk90");
			r.reflect(s.ckhahUnk91, "ckhahUnk91");
			r.reflect(s.ckhahUnk92, "ckhahUnk92");
			r.reflect(s.ckhahUnk93, "ckhahUnk93");
			r.reflect(s.ckhahUnk94, "ckhahUnk94");
			});
		unkStructs2.resize(2);
		r.foreachElement(unkStructs2, "unkStructs2", [&](UnkStruct2& s) {
			r.reflect(s.ckhahUnk144, "ckhahUnk144");
			r.reflect(s.ckhahUnk145, "ckhahUnk145");
			});
		ckhahUnk148.resize((ckhahHeroIndex == 2) ? 0 : 7);
		r.reflect(ckhahUnk148, "ckhahUnk148");
		ckhahUnk149.resize(1);
		r.reflect(ckhahUnk149, "ckhahUnk149");

		r.reflect(ckhahUnk152, "ckhahUnk152");
		r.reflect(ckhahUnk153, "ckhahUnk153");
		r.reflect(ckhahUnk154, "ckhahUnk154");
		r.message("DRM");
		r.reflect(ckhahUnkBefore155, "ckhahUnkBefore155");
		r.reflect(ckhahUnk155, "ckhahUnk155");
		r.reflect(ckhahUnk156, "ckhahUnk156");
		r.reflect(ckhahUnk157, "ckhahUnk157");
		r.reflect(ckhahUnk158, "ckhahUnk158");
		r.reflect(ckhahUnk159, "ckhahUnk159");
		r.reflect(ckhahUnk160, "ckhahUnk160");
		r.message("DRM");
		r.reflect(ckhahUnk162Duplicate, "ckhahUnk162Duplicate");
		r.reflect(ckhahUnk161, "ckhahUnk161");
		r.reflect(ckhahUnk162, "ckhahUnk162");
		r.reflect(ckhahUnk163, "ckhahUnk163");
		r.reflect(ckhahUnk164, "ckhahUnk164");
		r.reflect(ckhahUnk165, "ckhahUnk165");
		r.reflect(ckhahUnk166, "ckhahUnk166");
		r.reflect(ckhahUnk167, "ckhahUnk167");
		r.reflect(ckhahUnk168, "ckhahUnk168");
		r.reflect(ckhahUnk169, "ckhahUnk169");
		r.reflect(ckhahUnk170, "ckhahUnk170");
		r.reflect(ckhahUnk171, "ckhahUnk171");
		r.reflect(ckhahUnk172, "ckhahUnk172");
		r.reflect(ckhahUnk173, "ckhahUnk173");
		r.reflect(ckhahUnk174, "ckhahUnk174");
		r.reflect(ckhahUnk175, "ckhahUnk175");
		r.reflect(ckhahUnk176, "ckhahUnk176");
		r.reflect(ckhahUnk177, "ckhahUnk177");
		r.reflect(ckhahUnk178, "ckhahUnk178");
		r.reflect(ckhahUnk179, "ckhahUnk179");
		r.reflect(ckhahUnk180, "ckhahUnk180");
		r.reflect(ckhahUnk181, "ckhahUnk181");
		r.reflect(ckhahUnk182, "ckhahUnk182");
		r.reflect(ckhahUnk183, "ckhahUnk183");
		r.reflect(ckhahUnk184, "ckhahUnk184");
		r.reflect(ckhahUnk185, "ckhahUnk185");
		r.reflect(ckhahUnk186, "ckhahUnk186");
		r.reflect(ckhahUnk187, "ckhahUnk187");
		r.reflect(ckhahUnk188, "ckhahUnk188");
		r.reflect(ckhahUnk189, "ckhahUnk189");
		r.reflect(ckhahUnk190, "ckhahUnk190");
		r.reflect(ckhahUnk191, "ckhahUnk191");
		r.reflect(ckhahUnk192, "ckhahUnk192");
		r.reflect(ckhahUnk193, "ckhahUnk193");
		r.reflect(ckhahUnk194, "ckhahUnk194");
		r.reflect(ckhahUnk195, "ckhahUnk195");
		r.reflect(ckhahUnk196, "ckhahUnk196");
		r.reflect(ckhahUnk197, "ckhahUnk197");
		r.reflect(ckhahUnk198, "ckhahUnk198");
		r.reflect(ckhahUnk199, "ckhahUnk199");
		r.reflect(ckhahUnk200, "ckhahUnk200");
		r.reflect(ckhahUnk201, "ckhahUnk201");
		r.reflect(ckhahUnk202, "ckhahUnk202");
		r.reflect(ckhahUnk203, "ckhahUnk203");
		r.reflect(ckhahUnk204, "ckhahUnk204");
		r.reflect(ckhahUnk205, "ckhahUnk205");
		r.reflect(ckhahUnk206, "ckhahUnk206");
		r.reflect(ckhahUnk207, "ckhahUnk207");
		r.reflect(ckhahUnk208, "ckhahUnk208");
		r.reflect(ckhahUnk209, "ckhahUnk209");
		r.reflectComposed(pathFindCpnt, "pathFindCpnt", kenv);
		r.reflect(ckhahUnk214, "ckhahUnk214");
		r.reflect(ckhahUnk215, "ckhahUnk215");
		r.reflect(ckhahUnk216, "ckhahUnk216");
		r.reflect(ckhahUnk217, "ckhahUnk217");
		r.reflect(ckhahUnk218, "ckhahUnk218");
		r.reflect(ckhahUnk219, "ckhahUnk219");
		r.reflect(ckhahUnk220, "ckhahUnk220");
		r.reflect(ckhahUnk221, "ckhahUnk221");
		r.reflect(ckhahUnk222, "ckhahUnk222");
		r.reflect(ckhahUnk223, "ckhahUnk223");
		r.reflect(ckhahUnk224, "ckhahUnk224");

		r.reflect(unkStructs3, "unkStructs3");
		r.foreachElement(unkStructs4, "unkStructs4", [&](UnkStruct4& s) {
			r.reflect(s.ckhahUnk240, "ckhahUnk240");
			r.reflect(s.ckhahUnk241, "ckhahUnk241");
			r.reflect(s.ckhahUnk242, "ckhahUnk242");
			});
		r.reflect(unkStructs5, "unkStructs5");
		r.foreachElement(unkStructs6, "unkStructs6", [&](UnkStruct6& s) {
			r.reflect(s.ckhahUnk274, "ckhahUnk274");
			r.reflect(s.ckhahUnk275, "ckhahUnk275");
			r.reflect(s.ckhahUnk276, "ckhahUnk276");
			r.reflect(s.ckhahUnk277, "ckhahUnk277");
			});

		r.reflect(ckhahUnk306, "ckhahUnk306");
		r.reflect(ckhahUnk307, "ckhahUnk307");
		r.reflect(ckhahUnk308, "ckhahUnk308");
		r.reflect(ckhahUnk309, "ckhahUnk309");
		r.reflect(ckhahUnk310, "ckhahUnk310");
		r.reflect(ckhahUnk311, "ckhahUnk311");
		r.reflect(ckhahUnk312, "ckhahUnk312");
		r.reflect(ckhahUnk313, "ckhahUnk313");
		r.reflect(ckhahUnk314, "ckhahUnk314");
		r.reflect(ckhahUnk315, "ckhahUnk315");
		r.reflect(ckhahUnk316, "ckhahUnk316");
		r.reflect(ckhahUnk317, "ckhahUnk317");
		r.reflect(ckhahUnk318, "ckhahUnk318");
		r.reflect(ckhahUnk319, "ckhahUnk319");
		r.reflect(ckhahUnk320, "ckhahUnk320");
		r.reflect(ckhahUnk321, "ckhahUnk321");
		r.reflect(ckhahUnk322, "ckhahUnk322");
		r.reflect(ckhahUnk323, "ckhahUnk323");
		r.reflect(ckhahUnk324, "ckhahUnk324");
		r.reflect(ckhahUnk325, "ckhahUnk325");
		r.reflect(ckhahUnk326, "ckhahUnk326");
		r.reflect(ckhahUnk327, "ckhahUnk327");
		r.reflect(ckhahUnk328, "ckhahUnk328");
		r.reflect(ckhahUnk329, "ckhahUnk329");
		r.reflect(ckhahUnk330, "ckhahUnk330");
		r.reflect(ckhahUnk331, "ckhahUnk331");
		r.reflect(ckhahUnk332, "ckhahUnk332");
		r.reflect(ckhahUnk333, "ckhahUnk333");
		r.reflect(ckhahUnk334, "ckhahUnk334");
		r.reflect(ckhahUnk335, "ckhahUnk335");
		r.reflect(ckhahUnk336, "ckhahUnk336");
		r.reflect(ckhahUnk337, "ckhahUnk337");
		r.reflect(ckhahUnk338, "ckhahUnk338");
		r.reflect(ckhahUnk339, "ckhahUnk339");
		r.reflect(ckhahUnk340, "ckhahUnk340");
		r.reflect(ckhahUnk341, "ckhahUnk341");
		r.reflect(ckhahUnk342, "ckhahUnk342");
		r.reflect(ckhahUnk343, "ckhahUnk343");
		r.reflect(ckhahUnk344, "ckhahUnk344");
		r.reflect(ckhahUnk345, "ckhahUnk345");
		r.reflect(ckhahUnk346, "ckhahUnk346");
		r.reflect(ckhahUnk347, "ckhahUnk347");
		r.reflect(ckhahUnk348, "ckhahUnk348");
		r.reflect(ckhahUnk349, "ckhahUnk349");
		r.reflect(ckhahUnk350, "ckhahUnk350");
		r.reflect(ckhahUnk351, "ckhahUnk351");
		r.reflect(ckhahUnk352, "ckhahUnk352");
		r.reflect(ckhahUnk353, "ckhahUnk353");
		r.reflect(ckhahUnk354, "ckhahUnk354");
		r.reflect(ckhahUnk355, "ckhahUnk355");
		r.reflect(ckhahUnk356, "ckhahUnk356");
		r.reflect(ckhahUnk357, "ckhahUnk357");
		r.reflect(ckhahUnk358, "ckhahUnk358");
		r.reflect(ckhahUnk359, "ckhahUnk359");
		r.reflect(ckhahUnk360, "ckhahUnk360");
		r.reflect(ckhahUnk361, "ckhahUnk361");
		r.reflect(ckhahUnk362, "ckhahUnk362");
		r.reflect(ckhahUnk363, "ckhahUnk363");
		r.reflect(ckhahUnk364, "ckhahUnk364");
		r.reflect(ckhahUnk365, "ckhahUnk365");
		r.reflect(ckhahUnk366, "ckhahUnk366");
		r.reflect(ckhahUnk367, "ckhahUnk367");
		r.reflect(ckhahUnk368, "ckhahUnk368");
		r.reflect(ckhahUnk369, "ckhahUnk369");

		r.reflectComposed(fireBallData, "fireBallData", kenv);
		r.foreachElement(shockWaveDatas, "shockWaveDatas", [&](auto& inst) {
			r.reflectComposed(inst, "inst", kenv);
			});
		r.reflectComposed(cameraQuakeDatas1, "cameraQuakeDatas1", kenv);
		r.reflectComposed(cameraQuakeDatas2, "cameraQuakeDatas2", kenv);
		r.reflectComposed(explosionData1, "explosionData1", kenv);
		r.reflectComposed(explosionData2, "explosionData2", kenv);

		r.reflect(ckhahUnk483, "ckhahUnk483");
	}
	void CKHkWeatherCenter::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhwcUnk0, "ckhwcUnk0");
		r.reflect(ckhwcUnk1, "ckhwcUnk1");
		r.reflect(ckhwcUnk2, "ckhwcUnk2");
		r.reflect(ckhwcCloudsNodeFx, "ckhwcCloudsNodeFx");
		r.reflectSize<uint8_t>(ckhwcPresetList, "ckhwcPresetList_size");
		r.reflect(ckhwcPresetList, "ckhwcPresetList");
		r.reflect(ckhwcSpecialPreset, "ckhwcSpecialPreset");
		r.reflect(ckhwcUnk14, "ckhwcUnk14");
	}
	void CKPathFindingCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(pfcUnk1, "pfcUnk1");
		r.reflect(pfcUnk2, "pfcUnk2");
		r.reflect(pfcUnk3, "pfcUnk3");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(pfcOgUnk1, "pfcOgUnk1");
			r.reflect(pfcOgUnk2, "pfcOgUnk2");
		}
		r.reflect(pfcUnk4, "pfcUnk4");
	}
	void CKHkA2EnemyBase::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhaieUnk0, "ckhaieUnk0");
		r.reflectSize<uint8_t>(ckhaieUnk2, "ckhaieUnk2_size");
		r.reflect(ckhaieUnk2, "ckhaieUnk2");
		r.reflectSize<uint8_t>(ckhaieUnk4, "ckhaieUnk4_size");
		r.reflect(ckhaieUnk4, "ckhaieUnk4");
		r.reflectSize<uint8_t>(ckhaieUnk6, "ckhaieUnk6_size");
		r.reflect(ckhaieUnk6, "ckhaieUnk6");
		r.reflect(ckhaieUnk12, "ckhaieUnk12");
		r.reflect(ckhaieUnk13, "ckhaieUnk13");
		r.reflect(ckhaieUnk14, "ckhaieUnk14");
		r.reflect(ckhaieUnk15, "ckhaieUnk15");
		r.reflect(ckhaieUnk16, "ckhaieUnk16");
		r.reflect(ckhaieUnk17, "ckhaieUnk17");
		r.reflect(ckhaieUnk18, "ckhaieUnk18");
		r.reflect(ckhaieUnk19, "ckhaieUnk19");
		r.reflect(ckhaieUnk20, "ckhaieUnk20");
		r.reflect(ckhaieUnk21, "ckhaieUnk21");
		r.reflect(ckhaieUnk22, "ckhaieUnk22");
		r.reflect(ckhaieUnk23, "ckhaieUnk23");
		r.reflect(ckhaieUnk24, "ckhaieUnk24");
		r.reflect(ckhaieUnk25, "ckhaieUnk25");
		r.reflectSize<uint8_t>(ckhaieUnk27, "ckhaieUnk27_size");
		r.reflect(ckhaieUnk27, "ckhaieUnk27");
		r.reflect(ckhaieUnk34, "ckhaieUnk34");
		r.reflectSize<uint8_t>(ckhaieUnk36, "ckhaieUnk36_size");
		r.reflect(ckhaieUnk36, "ckhaieUnk36");
		r.reflectSize<uint8_t>(ckhaieUnk48, "ckhaieUnk48_size");
		r.reflect(ckhaieUnk48, "ckhaieUnk48");
		r.reflectSize<uint8_t>(ckhaieUnk52, "ckhaieUnk52_size");
		r.reflect(ckhaieUnk52, "ckhaieUnk52");
		r.reflect(ckhaieUnk53, "ckhaieUnk53");
		r.reflect(ckhaieUnk54, "ckhaieUnk54");
		r.reflect(ckhaieUnk55, "ckhaieUnk55");
		r.reflect(ckhaieUnk56, "ckhaieUnk56");
		r.reflectComposed(pathFindCpnt, "pathFindCpnt", kenv);
		r.reflect(ckhaieUnk61, "ckhaieUnk61");
		r.reflect(ckhaieUnk62, "ckhaieUnk62");
		r.reflect(ckhaieUnk63, "ckhaieUnk63");
		r.reflect(ckhaieUnk64, "ckhaieUnk64");
		r.reflect(ckhaieUnk65, "ckhaieUnk65");
	}
	void CKHkA2Enemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHkA2EnemyBase::reflectMembers2(r, kenv);
		r.reflect(sandal, "sandal");
	}
	void CKHkA2ArcherEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHkA2Enemy::reflectMembers2(r, kenv);
		r.reflect(archerProjectiles, "archerProjectiles");
	}
	void CKHkA2JetPackEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHkA2Enemy::reflectMembers2(r, kenv);
		r.reflect(bomb, "bomb");
	}
	void CKHkA2MarioEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHkA2Enemy::reflectMembers2(r, kenv);
		r.reflect(archerProjectiles, "archerProjectiles");
	}

	void CKHkBonusHolder::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhbhUnk0, "ckhbhUnk0");
		r.reflect(ckhbhUnk1, "ckhbhUnk1");
		r.reflect(ckhbhUnk2, "ckhbhUnk2");
		r.reflect(ckhbhUnk3, "ckhbhUnk3");
		r.reflect(ckhbhUnk4, "ckhbhUnk4");
		r.reflect(ckhbhUnk5, "ckhbhUnk5");
		r.reflect(ckhbhUnk6, "ckhbhUnk6");
		r.reflect(ckhbhUnk7, "ckhbhUnk7");
		r.reflect(ckhbhUnk8, "ckhbhUnk8");
		r.reflect(ckhbhUnk9, "ckhbhUnk9");
	}

	void CKHkBumper::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhbUnk0, "ckhbUnk0");
		r.reflect(ckhbUnk1, "ckhbUnk1");
		r.reflect(ckhbUnk2, "ckhbUnk2");
		r.reflect(ckhbUnk3, "ckhbUnk3");
		r.reflect(ckhbUnk4, "ckhbUnk4");
		r.reflect(ckhbUnk5, "ckhbUnk5");
		r.reflect(ckhbUnk6, "ckhbUnk6");
		r.reflect(ckhbUnk7, "ckhbUnk7");
		r.reflect(ckhbUnk8, "ckhbUnk8");
		r.reflect(ckhbUnk9, "ckhbUnk9");
		r.reflect(ckhbUnk10, "ckhbUnk10");
		r.reflect(ckhbUnk11, "ckhbUnk11");
		r.reflect(ckhbUnk12, "ckhbUnk12");
		r.reflect(ckhbUnk13, "ckhbUnk13");
		r.reflect(ckhbUnk14, "ckhbUnk14");
		r.reflect(ckhbUnk15, "ckhbUnk15", this);
	};

	void CKGrpA2FoodBasket::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckgafbUnk0, "ckgafbUnk0");
		r.reflect(numHolders, "numHolders");
		r.reflect(curBonusHolder, "curBonusHolder");
		bonusHolders.resize(numHolders);
		r.reflect(bonusHolders, "bonusHolders");
		r.reflect(holderCpnt, "holderCpnt");
	}

	void CKHkA2InvincibleEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHkA2Enemy::reflectMembers2(r, kenv);
	}

	void CKHkCrumblyZoneAnimated::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHkCrumblyZone::reflectMembers2(r, kenv);
		r.reflect(ckhczaUnk0, "ckhczaUnk0");
		r.reflect(ckhczaUnk1, "ckhczaUnk1");
	}

	void CKHkPressionStone::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhpsUnk6, "ckhpsUnk6");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogAnimNode, "ogAnimNode");
			r.reflect(ogAnimDict, "ogAnimDict");
			r.reflect(ogParticles, "ogParticles");
		}
		r.reflect(ckhpsUnk7, "ckhpsUnk7");
		r.reflect(ckhpsUnk8, "ckhpsUnk8");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogThirdGround, "ogThirdGround");
		}
		r.reflect(ckhpsUnk9, "ckhpsUnk9");
		r.reflect(ckhpsUnk10, "ckhpsUnk10", this);
		r.reflect(ckhpsUnk12, "ckhpsUnk12", this);
		r.reflect(ckhpsUnk13, "ckhpsUnk13", this);
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogEvent4, "ogEvent4", this);
			r.reflect(ogEvent5, "ogEvent5", this);
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
				r.reflect(ogEvent6, "ogEvent6", this);
				r.reflectSize<uint8_t>(ogHotSpots, "ogHotSpots_size");
				r.reflect(ogHotSpots, "ogHotSpots");
			}
		}
		r.reflect(ckhpsUnk14, "ckhpsUnk14");
		r.reflect(ckhpsUnk15, "ckhpsUnk15");
		r.reflect(ckhpsUnk16, "ckhpsUnk16");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogNewFloat, "ogNewFloat");
		}
		r.reflect(ckhpsUnk17, "ckhpsUnk17");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogNewUnk1, "ogNewUnk1");
			r.reflect(ogNewUnk2, "ogNewUnk2");
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
				r.reflect(ogNewUnk3, "ogNewUnk3");
		}
	}

	void CKHkSwitch::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflectSize<uint8_t>(spsBytes, "spsBytes_size");
		r.reflectSize<uint8_t>(spsHooks, "spsHooks_size");
		r.reflect(spsBytes, "spsBytes");
		r.reflect(spsHooks, "spsHooks");
		r.reflect(spsEvent, "spsEvent", this);
	}

	void CKHkShoppingArea::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhsaUnk0, "ckhsaUnk0");
		r.reflect(ckhsaUnk1, "ckhsaUnk1");
		r.reflect(ckhsaUnk3, "ckhsaUnk3");
		r.reflect(ckhsaUnk5, "ckhsaUnk5");
		r.reflect(ckhsaUnk6, "ckhsaUnk6");
		r.reflect(ckhsaUnk7, "ckhsaUnk7");
	};

	void CKHkMovableCharacter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogUnk1, "ogUnk1");
		}
		r.reflect(ckhmcUnk0, "ckhmcUnk0");
		r.reflect(ckhmcUnk1, "ckhmcUnk1");
		r.reflect(ckhmcUnk2, "ckhmcUnk2");
		r.reflect(ckhmcUnk3, "ckhmcUnk3");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhmcUnk4, "ckhmcUnk4");
			r.reflect(ckhmcUnk5, "ckhmcUnk5");
			r.reflect(ckhmcUnk6, "ckhmcUnk6");
			r.reflect(ckhmcUnk7, "ckhmcUnk7");
			r.reflect(ckhmcUnk8, "ckhmcUnk8");
			r.reflect(ckhmcUnk9, "ckhmcUnk9");
			r.reflect(ckhmcUnk10, "ckhmcUnk10");
			r.reflect(ckhmcUnk11, "ckhmcUnk11");
			r.reflect(ckhmcUnk12, "ckhmcUnk12");
			r.reflect(ckhmcUnk13, "ckhmcUnk13");
			r.reflect(ckhmcUnk14, "ckhmcUnk14");
			r.reflect(ckhmcUnk15, "ckhmcUnk15");
			r.reflect(ckhmcUnk16, "ckhmcUnk16");
			r.reflect(ckhmcUnk17, "ckhmcUnk17");
			r.reflect(ckhmcUnk18, "ckhmcUnk18");
			r.reflect(ckhmcUnk19, "ckhmcUnk19");
			r.reflect(ckhmcUnk20, "ckhmcUnk20");
			r.reflect(ckhmcUnk21, "ckhmcUnk21");
			r.reflect(ckhmcUnk22, "ckhmcUnk22");
			r.reflect(ckhmcUnk23, "ckhmcUnk23");
			r.reflect(ckhmcUnk24, "ckhmcUnk24");
		}
		else if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogFloats, "ogFloats");
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
				r.reflect(ogLastFloat, "ogLastFloat");
		}
		r.reflect(ckhmcUnk25, "ckhmcUnk25");
		r.reflect(ckhmcUnk26, "ckhmcUnk26");
		r.reflect(ckhmcUnk27, "ckhmcUnk27");
		r.reflect(ckhmcUnk28, "ckhmcUnk28");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhmcUnk29, "ckhmcUnk29");
			r.reflect(ckhmcUnk30, "ckhmcUnk30");
			r.reflect(ckhmcUnk31, "ckhmcUnk31");
		}
		else if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogVec1, "ogVec1");
			r.reflect(ogVec2, "ogVec2");
			r.reflectSize<uint16_t>(ogNodes, "ogNodes_size");
			r.reflect(ogNodes, "ogNodes");
			r.reflect(ogObj1, "ogObj1");
			r.reflect(ogObj2, "ogObj2");
			r.reflect(ogObj3, "ogObj3");
			r.reflect(ogEvent1, "ogEvent1", this);
			r.reflect(ogEvent2, "ogEvent2", this);
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
				r.reflect(ogEvent3, "ogEvent3", this);
			}
			r.reflectComposed(ogPfCpnt, "ogPfCpnt", kenv);
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
				r.reflect(ogObj4, "ogObj4");
				r.reflect(ogObj5, "ogObj5");
				r.reflect(ogObj6, "ogObj6");
			}
		}
	};

	void CKHkEnemyTarget::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhetUnk0, "ckhetUnk0");
		r.reflect(ckhetUnk1, "ckhetUnk1");
		r.reflect(ckhetUnk2, "ckhetUnk2");
		r.reflect(ckhetUnk3, "ckhetUnk3");
		r.reflect(ckhetUnk4, "ckhetUnk4");
		r.reflect(ckhetUnk5, "ckhetUnk5");
		r.reflect(ckhetUnk6, "ckhetUnk6");
		r.reflect(ckhetUnk7, "ckhetUnk7");
		r.reflect(ckhetUnk8, "ckhetUnk8");
		r.reflect(ckhetUnk9, "ckhetUnk9");
		r.reflect(ckhetUnk10, "ckhetUnk10", this);
		r.reflect(ckhetUnk11, "ckhetUnk11", this);
		r.reflect(ckhetUnk13, "ckhetUnk13");
	};

	void CKHkTelepher::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhtUnk0, "ckhtUnk0");
		r.reflect(ckhtUnk1, "ckhtUnk1");
		r.reflect(ckhtUnk2, "ckhtUnk2");
		r.reflect(ckhtUnk3, "ckhtUnk3");
		r.reflect(ckhtUnk4, "ckhtUnk4");
		r.reflect(ckhtUnk5, "ckhtUnk5");
		r.reflect(ckhtUnk6, "ckhtUnk6");
		r.reflect(ckhtUnk7, "ckhtUnk7");
		r.reflect(ckhtUnk8, "ckhtUnk8");
		r.reflect(ckhtUnk9, "ckhtUnk9");
		r.reflect(ckhtUnk10, "ckhtUnk10");
		r.reflect(ckhtUnk11, "ckhtUnk11");
		r.reflect(ckhtUnk12, "ckhtUnk12");
		r.reflect(ckhtUnk13, "ckhtUnk13");
		r.reflect(ckhtUnk14, "ckhtUnk14");
		r.reflect(ckhtUnk15, "ckhtUnk15");
		if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogUnk1, "ogUnk1");
			r.reflect(ogUnk2, "ogUnk2");
			r.reflect(ogUnk3, "ogUnk3");
		}
		r.reflect(ckhtUnk16, "ckhtUnk16");
		r.reflect(ckhtUnk17, "ckhtUnk17");
		r.reflect(ckhtUnk18, "ckhtUnk18");
		r.reflect(ckhtUnk19, "ckhtUnk19");
		r.reflect(ckhtUnk20, "ckhtUnk20");
		r.reflect(ckhtUnk21, "ckhtUnk21");
		r.reflect(ckhtUnk22, "ckhtUnk22");
		r.reflect(ckhtUnk23, "ckhtUnk23");
		r.reflect(ckhtUnk24, "ckhtUnk24");
		if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflectSize<uint32_t>(ogMoreCameraSectors, "ogMoreCameraSectors_size");
			r.reflect(ogMoreCameraSectors, "ogMoreCameraSectors");
		}
		r.reflect(ckhtUnk25, "ckhtUnk25");
		r.reflect(ckhtUnk26, "ckhtUnk26");
		r.reflect(ckhtUnk27, "ckhtUnk27");
		r.reflect(ckhtUnk28, "ckhtUnk28");
		r.reflect(ckhtUnk29, "ckhtUnk29");
		r.reflect(ckhtUnk30, "ckhtUnk30");
		r.reflect(ckhtUnk31, "ckhtUnk31");
		r.reflect(ckhtUnk32, "ckhtUnk32");
		r.reflect(ckhtUnk33, "ckhtUnk33");
		r.reflect(ckhtUnk34, "ckhtUnk34");
		r.reflect(ckhtUnk35, "ckhtUnk35");
		r.reflect(ckhtUnk36, "ckhtUnk36");
		r.reflect(ckhtUnk37, "ckhtUnk37");
		r.reflect(ckhtUnk38, "ckhtUnk38");
		r.reflect(ckhtUnk39, "ckhtUnk39");
		r.reflect(ckhtUnk40, "ckhtUnk40", this);
		r.reflect(ckhtUnk41, "ckhtUnk41", this);
		if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogUnkEvent, "ogUnkEvent", this);
		}
		r.reflect(ckhtUnk42, "ckhtUnk42");
		if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogCameraBeacon, "ogCameraBeacon");
			r.reflect(ogBoundingSphere, "ogBoundingSphere");
		}
	};

	void CKHkTelepherTowed::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHkTelepher::reflectMembers2(r, kenv);
		r.reflect(ckhttUnk0, "ckhttUnk0");
		r.reflect(ckhttUnk1, "ckhttUnk1");
		r.reflect(ckhttUnk2, "ckhttUnk2");
		r.reflect(ckhttUnk3, "ckhttUnk3");
		r.reflect(ckhttUnk4, "ckhttUnk4");
		r.reflect(ckhttUnk5, "ckhttUnk5");
		r.reflect(ckhttUnk6, "ckhttUnk6");
		r.reflect(ckhttUnk7, "ckhttUnk7");
		r.reflect(ckhttUnk8, "ckhttUnk8");
		r.reflect(ckhttUnk9, "ckhttUnk9");
		r.reflect(ckhttUnk10, "ckhttUnk10");
		r.reflect(ckhttUnk11, "ckhttUnk11");
		r.reflect(ckhttUnk12, "ckhttUnk12");
		r.reflect(ckhttUnk13, "ckhttUnk13");
		r.reflect(ckhttUnk14, "ckhttUnk14");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhttUnk15, "ckhttUnk15");
			r.reflect(ckhttUnk16, "ckhttUnk16");
		}
		else if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogText, "ogText");
			r.reflect(ogTowUnk1, "ogTowUnk1");
			r.reflect(ogTowUnk2, "ogTowUnk2");
			r.reflect(ogTowUnk3, "ogTowUnk3");
			r.reflect(ogTowUnk4, "ogTowUnk4");
			r.reflect(ogTowUnk5, "ogTowUnk5");
		}
		r.reflect(ckhttUnk17, "ckhttUnk17");
		r.reflect(ckhttUnk18, "ckhttUnk18");
		r.reflect(ckhttUnk19, "ckhttUnk19");
		r.reflect(ckhttUnk20, "ckhttUnk20");
		r.reflect(ckhttUnk21, "ckhttUnk21");
		r.reflect(ckhttUnk22, "ckhttUnk22");
		r.reflect(ckhttUnk23, "ckhttUnk23");
		r.reflect(ckhttUnk24, "ckhttUnk24", this);
		r.reflect(ckhttUnk25, "ckhttUnk25", this);
		if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogSekensLauncherCpnt, "ogSekensLauncherCpnt");
			r.reflect(ogMarkerBeacon1, "ogMarkerBeacon1");
			r.reflect(ogMarkerBeacon2, "ogMarkerBeacon2");
			r.reflect(ogInputIconFxData, "ogInputIconFxData");
		}
	};

	void CKHkTelepherAuto::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHkTelepher::reflectMembers2(r, kenv);
		r.reflect(ckhtaUnk0, "ckhtaUnk0");
		r.reflect(ckhtaUnk1, "ckhtaUnk1");
		r.reflect(ckhtaUnk2, "ckhtaUnk2");
		r.reflect(ckhtaUnk3, "ckhtaUnk3");
		r.reflect(ckhtaUnk4, "ckhtaUnk4");
		r.reflect(ckhtaUnk5, "ckhtaUnk5");
		r.reflect(ckhtaUnk6, "ckhtaUnk6");
		r.reflect(ckhtaUnk7, "ckhtaUnk7");
		r.reflect(ckhtaUnk8, "ckhtaUnk8");
		r.reflect(ckhtaUnk9, "ckhtaUnk9");
		r.reflect(ckhtaUnk10, "ckhtaUnk10");
		r.reflect(ckhtaUnk11, "ckhtaUnk11");
		r.reflectSize<uint32_t>(ckhtaUnk13, "ckhtaUnk13_size");
		r.reflect(ckhtaUnk13, "ckhtaUnk13");
		r.reflect(ckhtaUnk14, "ckhtaUnk14");
		r.reflect(ckhtaUnk15, "ckhtaUnk15");
		r.reflect(ckhtaUnk16, "ckhtaUnk16");
		r.reflect(ckhtaUnk17, "ckhtaUnk17");
	};

	void CKHkCounter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhcUnk0, "ckhcUnk0");
		r.reflect(ckhcUnk1, "ckhcUnk1");
		r.reflect(ckhcUnk2, "ckhcUnk2");
		r.reflect(ckhcUnk3, "ckhcUnk3");
		r.reflect(ckhcUnk4, "ckhcUnk4");
		r.reflect(ckhcUnk5, "ckhcUnk5");
		r.reflect(ckhcUnk6, "ckhcUnk6");
		r.reflect(ckhcUnk7, "ckhcUnk7");
		r.reflectSize<uint32_t>(values, "values_size");
		r.foreachElement(values, "values", [&](CounterValue& val) {
			r.reflect(val.ckhcUnk9, "ckhcUnk9");
			r.reflect(val.ckhcUnk10, "ckhcUnk10");
			r.reflect(val.ckhcUnk11, "ckhcUnk11");
			r.reflect(val.ckhcUnk12, "ckhcUnk12");
			});
		r.reflect(ckhcUnk13, "ckhcUnk13");
		r.reflect(ckhcUnk14, "ckhcUnk14");
	};

	void CKHkWaterWork::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhwwUnk0, "ckhwwUnk0");
		r.reflect(ckhwwUnk1, "ckhwwUnk1");
		r.reflect(ckhwwUnk2, "ckhwwUnk2");
		r.reflectSize<uint8_t>(ckhwwGrounds, "ckhwwNumGrounds");
		r.reflectSize<uint8_t>(ckhwwNodes, "ckhwwNumNodes");
		r.reflect(ckhwwUnk5, "ckhwwUnk5");
		r.reflect(ckhwwUnk6, "ckhwwUnk6", this);
		r.reflect(ckhwwUnk8, "ckhwwUnk8", this);
		r.reflect(ckhwwGrounds, "ckhwwGrounds");
		r.reflect(ckhwwNodes, "ckhwwNodes");
		r.reflect(ckhwwUnk26, "ckhwwUnk26");
		r.reflect(ckhwwUnk27, "ckhwwUnk27");
		r.reflect(ckhwwUnk28, "ckhwwUnk28");
	}

	void CKHkToll::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhtUnk0, "ckhtUnk0");
		r.reflect(ckhtUnk1, "ckhtUnk1");
		r.reflect(ckhtUnk2, "ckhtUnk2");
		r.reflect(ckhtUnk3, "ckhtUnk3");
		r.reflect(ckhtUnk4, "ckhtUnk4", this);
		r.reflect(ckhtPrice, "ckhtPrice");
		r.reflect(ckhtUnk7, "ckhtUnk7");
		r.reflect(ckhtUnk8, "ckhtUnk8");
		r.reflect(ckhtUnk9, "ckhtUnk9");
		r.reflect(ckhtUnk10, "ckhtUnk10");
	}

	void CKHkSlotMachine::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhsmUnk0, "ckhsmUnk0");
		r.reflect(ckhsmUnk1, "ckhsmUnk1");
		r.reflect(ckhsmUnk2, "ckhsmUnk2");
		r.reflect(ckhsmUnk3, "ckhsmUnk3");
		r.reflect(ckhsmUnk4, "ckhsmUnk4");
		r.reflect(ckhsmUnk5, "ckhsmUnk5");
		// DRM-protected (again) in final release, hence why only seen in demo (and remaster)
		r.message("DRM");
		r.reflect(smPrice, "smPrice");
		r.reflect(ckhsmUnk6, "ckhsmUnk6");
		r.reflect(ckhsmUnk7, "ckhsmUnk7");
		r.reflect(ckhsmUnk8, "ckhsmUnk8");
		r.reflect(ckhsmUnk9, "ckhsmUnk9");
		r.reflect(smProbabilities, "smProbabilities");
		r.reflect(smNumShields, "smNumShields");
		r.reflect(smNumHelmets, "smNumHelmets");
		r.reflect(smNumBoars, "smNumBoars");
		r.reflect(smNumMultipliers, "smNumMultipliers");
		r.reflect(ckhsmUnk15, "ckhsmUnk15");
		r.reflect(ckhsmUnk16, "ckhsmUnk16");
		r.reflect(ckhsmUnk17, "ckhsmUnk17");
		r.reflect(ckhsmUnk18, "ckhsmUnk18");
		r.reflect(ckhsmUnk19, "ckhsmUnk19");
		r.reflect(ckhsmUnk20, "ckhsmUnk20");
		r.reflect(ckhsmUnk21, "ckhsmUnk21");
		r.reflect(ckhsmUnk22, "ckhsmUnk22");
	};

	void CKHkCheckPoint::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhcpUnk0, "ckhcpUnk0");
		r.reflect(ckhcpUnk1, "ckhcpUnk1");
		r.reflect(ckhcpUnk2, "ckhcpUnk2");
		r.reflect(ckhcpUnk3, "ckhcpUnk3");
	}

	void CKHkFoldawayBridge::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflectSize<uint8_t>(fbParts, "fbParts_size");
		r.foreachElement(fbParts, "fbParts", [&](FBPart& p) {
			r.reflect(p.ckhfbUnk1, "ckhfbUnk1");
			r.reflect(p.ckhfbUnk2, "ckhfbUnk2");
			r.reflect(p.ckhfbUnk3, "ckhfbUnk3");
			r.reflect(p.ckhfbUnk4, "ckhfbUnk4");
			r.reflect(p.ckhfbUnk5, "ckhfbUnk5");
			});
		r.reflect(ckhfbUnk16, "ckhfbUnk16");
		r.reflect(ckhfbUnk17, "ckhfbUnk17");
		r.reflect(ckhfbUnk18, "ckhfbUnk18");
		r.reflect(ckhfbUnk19, "ckhfbUnk19");
		r.reflect(ckhfbUnk20, "ckhfbUnk20");
		r.reflect(ckhfbUnk21, "ckhfbUnk21");
		r.reflect(ckhfbUnk22, "ckhfbUnk22");
		r.reflect(ckhfbUnk23, "ckhfbUnk23", this);
		r.reflect(ckhfbUnk25, "ckhfbUnk25");
		r.reflect(ckhfbUnk26, "ckhfbUnk26");
		r.reflect(ckhfbUnk27, "ckhfbUnk27");
	}
	void CKHkMovableBloc::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhmbUnk0, "ckhmbUnk0");
		r.reflect(ckhmbUnk1, "ckhmbUnk1");
		r.reflect(ckhmbUnk2, "ckhmbUnk2");
		r.reflect(ckhmbUnk3, "ckhmbUnk3");
		r.reflect(ckhmbUnk4, "ckhmbUnk4");
		r.reflect(ckhmbUnk5, "ckhmbUnk5");
		r.reflectSize<uint8_t>(ckhmbUnk7, "ckhmbUnk7_size");
		r.reflect(ckhmbUnk7, "ckhmbUnk7");
		r.reflect(ckhmbUnk9, "ckhmbUnk9");
		r.reflect(ckhmbUnk13, "ckhmbUnk13");
		r.reflect(ckhmbUnk14, "ckhmbUnk14");
		r.reflect(ckhmbUnk15, "ckhmbUnk15", this);
		r.reflect(ckhmbUnk16, "ckhmbUnk16", this);
		r.reflect(ckhmbUnk17, "ckhmbUnk17", this);
		r.reflect(ckhmbUnk18, "ckhmbUnk18", this);
		r.reflect(ckhmbUnk20, "ckhmbUnk20");
		r.reflect(ckhmbUnk21, "ckhmbUnk21");
		r.reflect(ckhmbUnk22, "ckhmbUnk22");
		r.reflect(ckhmbUnk23, "ckhmbUnk23");
		r.reflect(ckhmbUnk24, "ckhmbUnk24");
		r.reflect(ckhmbUnk25, "ckhmbUnk25");
		r.reflect(ckhmbUnk26, "ckhmbUnk26");
	}
	void CKHkDynamicObject::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		auto rdov = [this,&r](DOValues& dov, const char* name, int32_t bit) {
			r.enterStruct(name);
			if (ckhdoFlags & bit) {
				r.reflectSize<uint32_t>(dov.fltVec, "fltVec_size");
				r.reflect(dov.fltVec, "fltVec");
			}
			else {
				r.reflect(dov.fltLone, "fltLone");
			}
			r.leaveStruct();
		};
		if (kenv->version <= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ckhdoFlags, "ckhdoFlags");
			r.reflect(ckhdoUnk1, "ckhdoUnk1");
			rdov(dov1, "dov1", 0x100);
			r.reflect(ckhdoUnk3, "ckhdoUnk3");
			r.reflect(ckhdoUnk4, "ckhdoUnk4");
			rdov(dov2, "dov2", 0x200);
			r.reflect(ckhdoUnk7, "ckhdoUnk7");
			r.reflect(ckhdoUnk8, "ckhdoUnk8");
			r.reflect(ckhdoUnk9, "ckhdoUnk9");
			rdov(dov3, "dov3", 0x400);
			r.reflect(ckhdoUnk11, "ckhdoUnk11");
			r.reflect(ckhdoUnk12, "ckhdoUnk12");
			r.reflect(ckhdoMatrix, "ckhdoMatrix");
			if (kenv->version == KEnvironment::KVERSION_ARTHUR)
				r.reflect(arUnkRef, "arUnkRef");
		}
		else if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogUnk0, "ogUnk0");
			r.reflect(ogUnk1, "ogUnk1");
			r.reflect(ogUnk2, "ogUnk2");
			r.reflect(ogUnk3, "ogUnk3");
			r.reflect(ogUnk4, "ogUnk4");
			r.reflect(ogUnk5, "ogUnk5");
			r.reflect(ogUnk6, "ogUnk6");
			r.reflect(ogUnk7, "ogUnk7");
			r.reflect(ogUnk8, "ogUnk8");
			r.reflect(ogUnk9, "ogUnk9");
			r.reflect(ogUnk10, "ogUnk10");
			r.reflect(ogUnk11, "ogUnk11");
			r.reflect(ogUnk12, "ogUnk12");
			r.reflect(ogUnk13, "ogUnk13");
			r.reflect(ogUnk14, "ogUnk14");
			r.reflect(ogUnk15, "ogUnk15");
			r.reflect(ogUnk16, "ogUnk16");
			r.reflect(ogUnk17, "ogUnk17");
			r.reflect(ogUnk18, "ogUnk18");
			r.reflect(ogUnk19, "ogUnk19");
			r.reflect(ogUnk20, "ogUnk20");
			r.reflect(ogUnk21, "ogUnk21");
			r.reflect(ckhdoMatrix, "ckhdoMatrix");
			r.reflect(ogSecondMatrix, "ogSecondMatrix");
			r.reflect(ogObjRefLast, "ogObjRefLast");
			r.reflect(ogEvent1, "ogEvent1", this);
			r.reflect(ogEvent2, "ogEvent2", this);
		}
	}
	void CKHkRollingBarrel::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhrbUnk0, "ckhrbUnk0");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogBarrelPool, "ogBarrelPool");
		}
		r.reflect(ckhrbUnk1, "ckhrbUnk1");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogEvent, "ogEvent", this);
		}
		r.reflect(ckhrbUnk2, "ckhrbUnk2");
		r.reflectSize<uint32_t>(ckhrbUnk4, "ckhrbUnk4_size");
		r.reflect(ckhrbUnk4, "ckhrbUnk4");
		r.reflect(ckhrbUnk5, "ckhrbUnk5");
		r.reflect(ckhrbUnk6, "ckhrbUnk6");
		r.reflect(ckhrbUnk7, "ckhrbUnk7");
		r.reflect(ckhrbUnk8, "ckhrbUnk8");
	}
	void CKHkPushBomb::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhpbUnk0, "ckhpbUnk0");
		r.reflect(ckhpbUnk1, "ckhpbUnk1");
		r.reflect(ckhpbUnk2, "ckhpbUnk2");
		r.reflect(ckhpbUnk3, "ckhpbUnk3");
		r.reflect(ckhpbUnk4, "ckhpbUnk4");
		r.reflect(ckhpbUnk5, "ckhpbUnk5");
		r.reflect(ckhpbUnk6, "ckhpbUnk6", this);
		r.reflect(ckhpbUnk8, "ckhpbUnk8", this);
		r.reflect(ckhpbUnk9, "ckhpbUnk9", this);
		r.reflect(ckhpbUnk10, "ckhpbUnk10", this);
	}
	void CKHkEnemyTargetPit::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhetpUnk0, "ckhetpUnk0");
		r.reflect(ckhetpUnk1, "ckhetpUnk1");
		r.reflect(ckhetpUnk2, "ckhetpUnk2");
		r.reflect(ckhetpUnk3, "ckhetpUnk3");
		r.reflect(ckhetpUnk4, "ckhetpUnk4");
		r.reflect(ckhetpUnk5, "ckhetpUnk5");
		r.reflect(ckhetpUnk6, "ckhetpUnk6");
		r.reflect(ckhetpUnk7, "ckhetpUnk7");
		r.reflect(ckhetpUnk8, "ckhetpUnk8");
		r.reflect(ckhetpUnk9, "ckhetpUnk9");
		r.reflect(ckhetpUnk10, "ckhetpUnk10");
		r.reflect(ckhetpUnk11, "ckhetpUnk11");
		r.reflect(ckhetpUnk12, "ckhetpUnk12");
		r.reflect(ckhetpUnk13, "ckhetpUnk13");
		r.reflect(ckhetpUnk14, "ckhetpUnk14");
		r.reflect(ckhetpUnk15, "ckhetpUnk15");
		r.reflect(ckhetpUnk16, "ckhetpUnk16");
		r.reflect(ckhetpUnk17, "ckhetpUnk17");
		r.reflect(ckhetpUnk18, "ckhetpUnk18");
		r.reflect(ckhetpUnk19, "ckhetpUnk19", this);
		r.reflect(ckhetpUnk20, "ckhetpUnk20", this);
		r.reflect(ckhetpUnk22, "ckhetpUnk22");
		r.reflect(ckhetpUnk23, "ckhetpUnk23");
		ckhetpUnk24.resize(ckhetpUnk17[0]);
		ckhetpUnk25.resize(ckhetpUnk17[1]);
		ckhetpUnk26.resize(ckhetpUnk17[2]);
		r.reflect(ckhetpUnk24, "ckhetpUnk24");
		r.reflect(ckhetpUnk25, "ckhetpUnk25");
		r.reflect(ckhetpUnk26, "ckhetpUnk26");
	}
	void CKHkLockMachineGun::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhlmgUnk0, "ckhlmgUnk0");
		r.reflect(ckhlmgUnk1, "ckhlmgUnk1");
		r.reflect(ckhlmgUnk2, "ckhlmgUnk2");
		r.reflect(ckhlmgUnk3, "ckhlmgUnk3");
		r.reflect(ckhlmgUnk4, "ckhlmgUnk4");
		r.reflect(ckhlmgUnk5, "ckhlmgUnk5");
		r.reflect(ckhlmgUnk6, "ckhlmgUnk6");
		r.reflect(ckhlmgUnk7, "ckhlmgUnk7");
		r.reflect(ckhlmgUnk8, "ckhlmgUnk8");
		r.reflect(ckhlmgUnk9, "ckhlmgUnk9");
		r.reflect(ckhlmgUnk10, "ckhlmgUnk10");
		r.reflect(ckhlmgUnk11, "ckhlmgUnk11");
		r.reflect(ckhlmgUnk12, "ckhlmgUnk12");
		r.reflect(ckhlmgUnk13, "ckhlmgUnk13");
		r.reflect(ckhlmgUnk14, "ckhlmgUnk14");
		r.reflect(ckhlmgUnk15, "ckhlmgUnk15");
		r.reflect(ckhlmgUnk16, "ckhlmgUnk16");
		r.reflect(ckhlmgUnk17, "ckhlmgUnk17");
		r.reflect(ckhlmgUnk18, "ckhlmgUnk18");
		r.reflect(ckhlmgUnk19, "ckhlmgUnk19");
		r.reflect(ckhlmgUnk20, "ckhlmgUnk20");
		r.reflect(ckhlmgUnk21, "ckhlmgUnk21");
		r.reflect(ckhlmgUnk22, "ckhlmgUnk22");
		r.reflect(ckhlmgUnk23, "ckhlmgUnk23");
		r.reflect(ckhlmgUnk24, "ckhlmgUnk24");
		r.reflect(ckhlmgNumVectors, "ckhlmgNumVectors");
		r.reflect(ckhlmgUnk26, "ckhlmgUnk26");
		r.reflect(ckhlmgUnk27, "ckhlmgUnk27");
		r.reflect(ckhlmgUnk28, "ckhlmgUnk28");
		r.reflect(ckhlmgUnk29, "ckhlmgUnk29");
		r.reflect(ckhlmgUnk30, "ckhlmgUnk30");
		r.reflect(ckhlmgUnk31, "ckhlmgUnk31");
		r.reflect(ckhlmgUnk32, "ckhlmgUnk32");
		r.reflect(ckhlmgUnk33, "ckhlmgUnk33");
		r.reflect(ckhlmgUnk34, "ckhlmgUnk34");
		r.reflect(ckhlmgUnk35, "ckhlmgUnk35");
		r.reflect(ckhlmgUnk36, "ckhlmgUnk36");
		r.reflect(ckhlmgUnk37, "ckhlmgUnk37");
		r.reflect(ckhlmgUnk38, "ckhlmgUnk38");
		r.reflect(ckhlmgUnk39, "ckhlmgUnk39");
		r.reflect(ckhlmgUnk40, "ckhlmgUnk40");
		r.reflect(ckhlmgUnk41, "ckhlmgUnk41");
		r.reflect(ckhlmgUnk42, "ckhlmgUnk42");
		r.reflect(ckhlmgUnk43, "ckhlmgUnk43");
		r.reflect(ckhlmgUnk44, "ckhlmgUnk44");
		r.reflect(ckhlmgUnk45, "ckhlmgUnk45");
		r.reflect(ckhlmgUnk46, "ckhlmgUnk46");
		r.reflect(ckhlmgUnk47, "ckhlmgUnk47");
		r.reflect(ckhlmgUnk48, "ckhlmgUnk48");
		r.reflectComposed(quakeDatas, "quakeDatas", kenv);
		r.reflect(ckhlmgUnk55, "ckhlmgUnk55");
		r.reflect(ckhlmgUnk57, "ckhlmgUnk57");
		r.reflect(ckhlmgUnk59, "ckhlmgUnk59");
		r.reflect(ckhlmgUnk61, "ckhlmgUnk61");
		r.reflect(ckhlmgUnk63, "ckhlmgUnk63");
		r.reflect(ckhlmgUnk65, "ckhlmgUnk65");
		r.reflect(ckhlmgUnk67, "ckhlmgUnk67");
		r.reflect(ckhlmgUnk68, "ckhlmgUnk68");
		r.reflect(ckhlmgUnk69, "ckhlmgUnk69");
		r.reflect(ckhlmgUnk70, "ckhlmgUnk70");
		r.reflect(ckhlmgUnk71, "ckhlmgUnk71");
		r.reflect(ckhlmgNumHeroes, "ckhlmgNumHeroes");
		ckhlmgHeroes.resize(ckhlmgNumHeroes);
		ckhlmgHeroAnimDicts.resize(ckhlmgNumHeroes);
		ckhlmgHeroHotSpots.resize(ckhlmgNumHeroes);
		r.reflect(ckhlmgHeroes, "ckhlmgHeroes");
		r.reflect(ckhlmgHeroAnimDicts, "ckhlmgHeroAnimDicts");
		r.reflect(ckhlmgHeroHotSpots, "ckhlmgHeroHotSpots");
		ckhlmgVectors.resize(ckhlmgNumVectors);
		r.reflect(ckhlmgVectors, "ckhlmgVectors");
		r.reflect(ckhlmgUnk85, "ckhlmgUnk85", this);
		r.reflect(ckhlmgUnk91, "ckhlmgUnk91", this);
	}

	void CKHkCatapult::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKHook::reflectMembers2(r, kenv);
		r.reflect(ckhcUnk0, "ckhcUnk0");
		r.reflect(ckhcUnk1, "ckhcUnk1");
		r.reflect(ckhcUnk2, "ckhcUnk2");
		r.reflect(ckhcUnk3, "ckhcUnk3");
		r.reflect(ckhcUnk4, "ckhcUnk4");
		r.message("DRM");
		r.reflect(ckhcDemo, "ckhcDemo");
		r.reflect(ckhcUnk5, "ckhcUnk5");
		r.reflect(ckhcUnk6, "ckhcUnk6");
		r.reflect(ckhcUnk7, "ckhcUnk7");
		r.reflect(ckhcUnk8, "ckhcUnk8");
		r.reflect(ckhcUnk9, "ckhcUnk9");
		r.reflect(ckhcUnk10, "ckhcUnk10");
		r.reflect(ckhcUnk11, "ckhcUnk11");
		r.reflect(ckhcUnk12, "ckhcUnk12");
		r.reflect(ckhcUnk13, "ckhcUnk13");
		r.reflect(ckhcUnk14, "ckhcUnk14");
		r.reflect(ckhcUnk15, "ckhcUnk15");
		r.reflect(ckhcUnk16, "ckhcUnk16");
		r.reflect(ckhcUnk17, "ckhcUnk17", this);
		r.reflect(ckhcUnk18, "ckhcUnk18", this);
		r.reflect(ckhcUnk19, "ckhcUnk19");
		r.reflect(ckhcUnk20, "ckhcUnk20");
		r.reflect(ckhcUnk21, "ckhcUnk21");
		r.reflect(ckhcUnk22, "ckhcUnk22");
		r.reflect(ckhcUnk23, "ckhcUnk23");
		r.reflect(ckhcUnk24, "ckhcUnk24");
		r.reflect(ckhcUnk25, "ckhcUnk25");
		r.reflect(ckhcUnk26, "ckhcUnk26");
		r.reflect(ckhcUnk27, "ckhcUnk27");
	}
	void CKEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckaecUnk0, "ckaecUnk0");
		r.reflectComposed(moveCpnt, "moveCpnt", kenv);
	}
	void CKA2EnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKEnemyCpnt::reflectMembers2(r, kenv);
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.message("DRM");
			r.reflect(drm, "drm");
			r.reflect(ckaecUnk28, "ckaecUnk28");
			r.reflect(ckaecUnk29, "ckaecUnk29");
			r.reflect(ckaecUnk30, "ckaecUnk30");
			r.reflect(ckaecUnk31, "ckaecUnk31");
			r.reflect(ckaecUnk32, "ckaecUnk32");
			r.reflect(ckaecUnk33, "ckaecUnk33");
			r.reflect(ckaecUnk34, "ckaecUnk34");
			r.reflect(ckaecUnk35, "ckaecUnk35");
			r.reflect(ckaecUnk36, "ckaecUnk36");
			r.reflect(ckaecUnk37, "ckaecUnk37");
			r.reflect(ckaecUnk38, "ckaecUnk38");
			r.reflect(ckaecUnk39, "ckaecUnk39");
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
			r.foreachElement(enstructs, "enstructs", [&](SomeEnemyStruct& s) {
				r.reflect(s.ckaecUnk50, "ckaecUnk50");
				r.message("DRM");
				r.reflect(s.drmVal1, "drmVal1");
				r.message("DRM");
				r.reflect(s.drmVal2, "drmVal2");
				r.reflect(s.ckaecUnk51, "ckaecUnk51");
				r.reflect(s.ckaecUnk52, "ckaecUnk52");
				r.reflect(s.ckaecUnk53, "ckaecUnk53");
				r.reflect(s.ckaecUnk54, "ckaecUnk54");
				r.reflect(s.ckaecUnk55, "ckaecUnk55");
				r.reflect(s.ckaecUnk56, "ckaecUnk56");
				r.reflect(s.ckaecUnk57, "ckaecUnk57");
				r.reflect(s.ckaecUnk58, "ckaecUnk58");
				r.reflect(s.ckaecUnk59, "ckaecUnk59");
				r.reflect(s.ckaecUnk60, "ckaecUnk60");
				});
			r.reflect(ckaecUnk83, "ckaecUnk83");
			r.reflect(ckaecUnk84, "ckaecUnk84");
			r.reflect(ckaecUnk85, "ckaecUnk85");
			r.reflect(ckaecUnk86, "ckaecUnk86");
			r.reflect(ckaecUnk87, "ckaecUnk87");
			r.reflect(ckaecUnk88, "ckaecUnk88");
			r.reflect(ckaecUnk89, "ckaecUnk89");
			r.reflect(ckaecUnk90, "ckaecUnk90");
			r.reflect(ckaecUnk91, "ckaecUnk91");
			r.reflect(ckaecUnk92, "ckaecUnk92");
			r.reflect(ckaecUnk93, "ckaecUnk93");
			r.reflect(ckaecUnk94, "ckaecUnk94");
			r.reflectComposed(hedgeHopTrailFx, "hedgeHopTrailFx", kenv);
			r.reflect(ckaecUnk111, "ckaecUnk111");
			r.reflect(ckaecUnk112, "ckaecUnk112");
			r.reflect(ckaecUnk113, "ckaecUnk113");
			r.reflectComposed(explosionFx1, "explosionFx1", kenv);
			r.reflectComposed(explosionFx2, "explosionFx2", kenv);
			r.reflect(ckaecUnk138, "ckaecUnk138");
			r.reflect(ckaecUnk139, "ckaecUnk139");
			r.reflectSize<uint32_t>(ckaecUnk141, "ckaecUnk141_size");
			r.reflect(ckaecUnk141, "ckaecUnk141");
		}
		else if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			if (!ogVersion)
				ogVersion = std::make_unique<A3EnemyCpnt>();
			auto& v = *ogVersion;
			r.reflect(v.ckaecUnk0, "ckaecUnk0");
			r.reflect(v.ckaecUnk1, "ckaecUnk1");
			r.reflect(v.ckaecUnk2, "ckaecUnk2");
			r.reflect(v.ckaecUnk3, "ckaecUnk3");
			r.reflect(v.ckaecUnk4, "ckaecUnk4");
			r.reflect(v.ckaecUnk5, "ckaecUnk5");
			r.reflect(v.ckaecUnk6, "ckaecUnk6");
			r.reflect(v.ckaecUnk7, "ckaecUnk7");
			r.reflect(v.ckaecUnk8, "ckaecUnk8");
			r.reflect(v.ckaecUnk9, "ckaecUnk9");
			r.reflect(v.ckaecUnk10, "ckaecUnk10");
			r.reflect(v.ckaecUnk11, "ckaecUnk11");
			r.reflect(v.ckaecUnk12, "ckaecUnk12");
			r.reflect(v.ckaecUnk13, "ckaecUnk13");
			r.reflect(v.ckaecUnk14, "ckaecUnk14");
			r.reflectSize<uint32_t>(v.seses, "seses_size");
			r.foreachElement(v.seses, "seses", [&](A3EnemyCpnt::SES& s) {
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
			r.reflect(v.ckaecUnk40, "ckaecUnk40");
			r.reflect(v.ckaecUnk41, "ckaecUnk41");
			r.reflect(v.ckaecUnk42, "ckaecUnk42");
			r.reflect(v.ckaecUnk43, "ckaecUnk43");
			r.reflect(v.ckaecUnk44, "ckaecUnk44");
			r.reflect(v.ckaecUnk45, "ckaecUnk45");
			r.reflect(v.ckaecUnk46, "ckaecUnk46");
			r.reflect(v.ckaecUnk47, "ckaecUnk47");
			r.reflect(v.ckaecUnk48, "ckaecUnk48");
			r.reflect(v.ckaecUnk49, "ckaecUnk49");
			r.reflect(v.ckaecUnk50, "ckaecUnk50");
			r.reflect(v.ckaecUnk51, "ckaecUnk51");
			r.reflect(v.ckaecUnk52, "ckaecUnk52");
			r.reflect(v.ckaecUnk53, "ckaecUnk53");
			r.reflect(v.ckaecUnk54, "ckaecUnk54");
			r.reflect(v.ckaecUnk55, "ckaecUnk55");
			r.reflect(v.ckaecUnk56, "ckaecUnk56");
			r.reflect(v.ckaecUnk57, "ckaecUnk57");
			r.reflect(v.ckaecUnk58, "ckaecUnk58");
			r.reflect(v.ckaecUnk59, "ckaecUnk59");
			r.reflect(v.ckaecUnk60, "ckaecUnk60");
			r.reflect(v.ckaecUnk61, "ckaecUnk61");
			r.reflect(v.ckaecUnk62, "ckaecUnk62");
			r.reflect(v.ckaecUnk63, "ckaecUnk63");
			r.reflect(v.ckaecUnk64, "ckaecUnk64");
			r.reflect(v.ckaecUnk65, "ckaecUnk65");
			r.reflect(v.ckaecUnk66, "ckaecUnk66");
			r.reflect(v.ckaecUnk67, "ckaecUnk67");
			r.reflect(v.ckaecUnk68, "ckaecUnk68");
			r.reflect(v.ckaecUnk69, "ckaecUnk69");
			r.reflect(v.ckaecUnk70, "ckaecUnk70");
			r.reflect(v.ckaecUnk71, "ckaecUnk71");
			r.reflect(v.ckaecUnk72, "ckaecUnk72");
			r.reflect(v.ckaecUnk73, "ckaecUnk73");
			r.reflect(v.ckaecUnk74, "ckaecUnk74");
			r.reflect(v.ckaecUnk75, "ckaecUnk75");
		}
	}
	void CKA2JetPackEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKA2EnemyCpnt::reflectMembers2(r, kenv);
		r.reflect(ckajpecUnk0, "ckajpecUnk0");
		r.reflect(ckajpecUnk1, "ckajpecUnk1");
		r.reflect(ckajpecUnk2, "ckajpecUnk2");
		r.reflect(ckajpecUnk3, "ckajpecUnk3");
		r.reflect(ckajpecUnk4, "ckajpecUnk4");
		r.reflect(ckajpecUnk5, "ckajpecUnk5");
		r.reflect(ckajpecUnk6, "ckajpecUnk6");
		r.reflect(ckajpecUnk7, "ckajpecUnk7");
		r.reflect(ckajpecUnk8, "ckajpecUnk8");
		r.reflect(ckajpecUnk9, "ckajpecUnk9");
		r.reflect(ckajpecUnk10, "ckajpecUnk10");
		r.reflect(ckajpecUnk11, "ckajpecUnk11");
		r.reflect(ckajpecUnk12, "ckajpecUnk12");
		r.reflect(ckajpecUnk13, "ckajpecUnk13");
		r.reflect(ckajpecUnk14, "ckajpecUnk14");
		r.reflect(ckajpecUnk15, "ckajpecUnk15");
		r.reflect(ckajpecUnk16, "ckajpecUnk16");
		r.reflect(ckajpecUnk17, "ckajpecUnk17");
		r.reflect(ckajpecUnk18, "ckajpecUnk18");
		r.reflect(ckajpecUnk19, "ckajpecUnk19");
		r.reflect(ckajpecUnk20, "ckajpecUnk20");
	}
	void CKA2InvincibleEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKA2EnemyCpnt::reflectMembers2(r, kenv);
		r.reflect(ckaiecUnk0, "ckaiecUnk0");
		r.reflect(ckaiecUnk1, "ckaiecUnk1");
	}
	void CKA2ArcherEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKA2EnemyCpnt::reflectMembers2(r, kenv);
		r.reflect(ckaaecUnk0, "ckaaecUnk0");
		r.reflectSize<uint32_t>(throwTimes, "throwTimes_size");
		r.reflect(throwTimes, "throwTimes");
	}
	void CKA2MarioEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKA2EnemyCpnt::reflectMembers2(r, kenv);
		r.reflect(mecUnk0, "mecUnk0");
		r.reflect(mecUnk1, "mecUnk1");
		r.reflectComposed(shockWave, "shockWave", kenv);
	}
	void CKGrpA2Hero::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckgahUnk0, "ckgahUnk0");
		r.reflectSize<uint8_t>(ckgahUnk2, "ckgahUnk2");
		r.reflect(ckgahUnk2, "ckgahUnk2");
		r.reflect(ckgahUnk5, "ckgahUnk5");
		r.reflect(ckgahUnk6, "ckgahUnk6");
		r.reflect(ckgahUnk7, "ckgahUnk7");
		r.reflect(ckgahUnk8, "ckgahUnk8");
		r.reflect(ckgahUnk9, "ckgahUnk9");
		r.reflect(ckgahUnk10, "ckgahUnk10");
		r.reflect(ckgahUnk11, "ckgahUnk11");
		r.reflect(ckgahUnk12, "ckgahUnk12");
		r.reflect(ckgahUnk13, "ckgahUnk13");
		r.reflect(ckgahUnk14, "ckgahUnk14");
		r.reflect(ckgahUnk15, "ckgahUnk15");
		r.reflect(ckgahUnk16, "ckgahUnk16");
		r.reflect(ckgahUnk17, "ckgahUnk17");
		r.reflect(ckgahUnk18, "ckgahUnk18");
		r.reflect(ckgahUnk19, "ckgahUnk19");
		r.reflect(ckgahUnk20, "ckgahUnk20");
		r.reflect(ckgahUnk21, "ckgahUnk21");
		r.reflect(ckgahUnk22, "ckgahUnk22");
		r.reflectSize<uint32_t>(ckgahUnk24, "ckgahUnk24_size");
		r.reflect(ckgahUnk24, "ckgahUnk24");
	}
	void CKGrpMeca::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflectSize<uint16_t>(components, "components_size");
		r.reflect(components, "components");
		r.reflectSize<uint16_t>(hookClasses, "hookClasses_size");
		r.reflect(hookClasses, "hookClasses");
	}
	void CKGrpCrate::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(crateCpnt, "crateCpnt");
	}
	void CKGrpA2LevelPotion::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckgalpUnk0, "ckgalpUnk0");
		r.reflect(ckgalpUnk1, "ckgalpUnk1");
		r.reflect(ckgalpUnk2, "ckgalpUnk2", this);
		r.reflect(ckgalpUnk3, "ckgalpUnk3");
		r.reflect(ckgalpUnk4, "ckgalpUnk4");
		r.reflect(ckgalpUnk5, "ckgalpUnk5");

		r.reflectComposed(quakeData, "quakeData", kenv);
		r.reflectComposed(flashFx1, "flashFx1", kenv);
		r.reflectComposed(flashFx2, "flashFx2", kenv);
		r.reflectComposed(electricArcFx, "electricArcFx", kenv);

		r.reflect(ckgalpUnk42, "ckgalpUnk42");
		r.reflect(ckgalpUnk43, "ckgalpUnk43");
		r.reflect(ckgalpUnk44, "ckgalpUnk44");
		r.reflect(ckgalpUnk45, "ckgalpUnk45");
		r.reflect(ckgalpUnk46, "ckgalpUnk46");
		r.reflect(ckgalpUnk47, "ckgalpUnk47");
		r.reflect(ckgalpUnk48, "ckgalpUnk48");
		r.reflect(ckgalpUnk49, "ckgalpUnk49");
		r.reflect(ckgalpUnk50, "ckgalpUnk50");
		r.reflect(ckgalpUnk51, "ckgalpUnk51");
		r.reflect(ckgalpUnk52, "ckgalpUnk52");
		r.reflect(ckgalpUnk53, "ckgalpUnk53");
		r.reflect(ckgalpUnk54, "ckgalpUnk54");
		r.reflect(ckgalpUnk55, "ckgalpUnk55");
		r.reflect(ckgalpUnk56, "ckgalpUnk56");
		r.reflect(ckgalpUnk57, "ckgalpUnk57");
		r.reflect(ckgalpUnk58, "ckgalpUnk58");
		r.reflect(ckgalpUnk59, "ckgalpUnk59");
		r.reflect(ckgalpUnk60, "ckgalpUnk60");
		r.reflect(ckgalpUnk61, "ckgalpUnk61");
		r.reflect(ckgalpUnk62, "ckgalpUnk62");
		r.reflect(ckgalpUnk63, "ckgalpUnk63");
		r.reflect(ckgalpUnk64, "ckgalpUnk64");
		
		r.reflectComposed(distortionFx, "distortionFx", kenv);

		r.reflect(ckgalpUnk69, "ckgalpUnk69");
		r.reflect(ckgalpUnk70, "ckgalpUnk70");
		r.reflect(ckgalpUnk71, "ckgalpUnk71");
	}
	void CKGrpA2Enemy::GEStruct3::reflectMembers(MemberListener& r)
	{
		r.reflect(ges1, "ges1");
		r.reflect(ges2, "ges2");
		r.reflect(ges3, "ges3");
		r.reflect(ges4, "ges4");
		r.reflect(ges5, "ges5");
		r.reflect(ges6, "ges6");
	}
	void CKGrpA2Enemy::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflectSize<uint32_t>(ges1vec, "ges1vec_size");
		r.foreachElement(ges1vec, "ges1vec", [&](GEStruct1& s) {
			r.reflect(s.ges1, "ges1");
			r.reflect(s.ges2, "ges2");
			r.reflect(s.ges3, "ges3");
			r.reflect(s.ges4, "ges4");
			if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
				r.reflect(s.ogges1, "ogges1");
			});
		r.reflect(poolGroup, "poolGroup");
		r.reflectSize<uint32_t>(fightZoneGroups, "fightZoneGroups_size");
		r.reflect(fightZoneGroups, "fightZoneGroups");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(unkFloat1, "unkFloat1");
			r.foreachElement(ges2vec, "ges2vec", [&](GEStruct2& s) {
				r.reflect(s.ges1, "ges1");
				r.reflect(s.ges2, "ges2");
				r.reflect(s.ges3, "ges3");
				r.reflect(s.ges4, "ges4");
				r.reflect(s.ges5, "ges5");
				});
			r.reflect(ges3vec, "ges3vec");
			r.reflectSize<uint32_t>(fightZoneInfo, "fightZoneInfo_size");
			r.reflect(fightZoneInfo, "fightZoneInfo");
			r.reflect(particleNode, "particleNode");
		}
		else if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflectSize<uint32_t>(branches, "branches_size");
			r.reflect(branches, "branches");
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
	}
	void CKGrpA2Enemy::onLevelLoaded(KEnvironment* kenv)
	{
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			for (int i = 0; i < (int)branches.size(); ++i)
				branches[i].bind(kenv, i - 1);
		}
	}
	void CKGrpFightZone::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(zonePos1, "zonePos1");
		r.reflect(zoneSize1, "zoneSize1");
		r.reflect(zonePos2, "zonePos2");
		r.reflect(zoneSize2, "zoneSize2");
		r.reflect(zoneSomething, "zoneSomething");
		if (kenv->version == KEnvironment::KVERSION_OLYMPIC) {
			r.reflectSize<uint32_t>(ogSquads, "ogSquads_size");
			r.reflect(ogSquads, "ogSquads");
			r.reflectSize<uint32_t>(ogPools, "ogPools_size");
			r.foreachElement(ogPools, "ogPools", [&](Pool& p) {
				r.reflect(p.poolGroup, "poolGroup");
				r.reflect(p.poolVal1, "poolVal1");
				r.reflect(p.poolVal2, "poolVal2");
				});
			r.reflect(ckgfzUnk7, "ckgfzUnk7");
			r.reflect(ckgfzUnk8, "ckgfzUnk8");
			r.reflectSize<uint32_t>(fzs1Vec, "fzs1Vec_size");
			r.foreachElement(fzs1Vec, "fzs1Vec", [&](FZStruct1& s) {
				r.reflect(s.ckgfzUnk10, "ckgfzUnk10");
				r.reflect(s.ckgfzUnk11, "ckgfzUnk11");
				r.reflect(s.ckgfzUnk12, "ckgfzUnk12");
				});
			r.reflectSize<uint32_t>(fzs2Vec, "fzs2Vec_size");
			r.foreachElement(fzs2Vec, "fzs2Vec", [&](FZStruct2& s) {
				r.reflect(s.ckgfzUnk23, "ckgfzUnk23");
				r.reflect(s.ckgfzUnk24, "ckgfzUnk24");
				r.reflect(s.ckgfzUnk25, "ckgfzUnk25");
				r.reflect(s.ckgfzUnk26, "ckgfzUnk26");
				r.reflect(s.ckgfzUnk27, "ckgfzUnk27");
				r.reflect(s.ckgfzUnk28, "ckgfzUnk28");
				r.reflect(s.ckgfzUnk29, "ckgfzUnk29");
				});
			r.reflect(ckgfzUnk37, "ckgfzUnk37");
			r.reflect(ckgfzUnk38, "ckgfzUnk38");
			r.reflectSize<uint32_t>(ogUnkVectors, "ogUnkVectors_size");
			r.reflect(ogUnkVectors, "ogUnkVectors");
			r.reflect(ckgfzUnk40, "ckgfzUnk40");
			r.reflect(ckgfzUnk41, "ckgfzUnk41", this);
			r.reflect(ckgfzUnk42, "ckgfzUnk42", this);
			r.reflect(ckgfzUnk44, "ckgfzUnk44", this);
			r.reflect(ckgfzUnk45, "ckgfzUnk45", this);
			r.reflect(ckgfzUnk46, "ckgfzUnk46", this);
			r.reflect(ckgfzUnk47, "ckgfzUnk47");
		}
	}
	void CKTargetCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(cktcUnk0, "cktcUnk0");
		r.reflect(cktcUnk1, "cktcUnk1");
		r.reflect(cktcUnk2, "cktcUnk2");
		r.reflect(cktcUnk3, "cktcUnk3");
	}
	void CKCrumblyZoneCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflectSize<uint32_t>(particleNodes, "particleNodes_size");
		r.reflect(particleNodes, "particleNodes");
	}
	void CKShadowCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckscUnk0, "ckscUnk0");
		r.reflect(ckscUnk1, "ckscUnk1");
		r.reflect(ckscUnk2, "ckscUnk2");
		r.reflect(ckscUnk3, "ckscUnk3");
		r.reflect(ckscUnk4, "ckscUnk4");
		r.reflect(ckscUnk5, "ckscUnk5");
		r.reflect(ckscUnk6, "ckscUnk6");
		r.reflect(ckscUnk7, "ckscUnk7");
		r.reflect(ckscUnk8, "ckscUnk8");
		r.reflect(ckscUnk9, "ckscUnk9");
		r.reflect(ckscUnk10, "ckscUnk10");
		r.reflect(ckscUnk11, "ckscUnk11");
		r.reflect(ckscUnk12, "ckscUnk12");
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
		r.reflect(ckbcUnk15, "ckbcUnk15");
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflect(ogProjectile, "ogProjectile");
			r.reflect(ogFlt1, "ogFlt1");
			r.reflect(ogFlt2, "ogFlt2");
		}
	}
	void CKWeatherPreset::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckwpUnk0, "ckwpUnk0");
		r.reflect(ckwpUnk1, "ckwpUnk1");
		r.reflect(ckwpUnk2, "ckwpUnk2");
		r.reflect(ckwpUnk3, "ckwpUnk3");
		r.reflect(ckwpUnk4, "ckwpUnk4");
		r.reflect(ckwpUnk5, "ckwpUnk5");
		r.reflect(ckwpUnk6, "ckwpUnk6");
	}
	void CKA2PotionStoneCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckapscUnk0, "ckapscUnk0");
		r.reflect(ckapscUnk1, "ckapscUnk1");
		r.reflect(ckapscUnk2, "ckapscUnk2");
		r.reflect(ckapscUnk3, "ckapscUnk3");
		r.reflect(ckapscUnk4, "ckapscUnk4");
		r.reflect(ckapscUnk5, "ckapscUnk5");
		r.reflect(ckapscUnk6, "ckapscUnk6");
		r.reflect(ckapscUnk7, "ckapscUnk7");
		r.reflect(ckapscUnk8, "ckapscUnk8");
		r.reflect(ckapscUnk9, "ckapscUnk9");
		r.reflect(ckapscUnk10, "ckapscUnk10");
		r.reflect(ckapscUnk11, "ckapscUnk11");
		r.reflect(ckapscUnk12, "ckapscUnk12");
		r.reflect(ckapscUnk13, "ckapscUnk13");
		r.reflect(ckapscUnk14, "ckapscUnk14");
		r.reflect(ckapscUnk15, "ckapscUnk15");
		r.reflect(ckapscUnk16, "ckapscUnk16");
	}
	void CKMecaCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckmcUnk0, "ckmcUnk0");
	}
	void CKBonusSpitterCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckbscUnk0, "ckbscUnk0");
		r.reflect(ckbscUnk1, "ckbscUnk1");
		r.reflect(ckbscUnk2, "ckbscUnk2");
		r.reflectSize<uint32_t>(bonusesToSpit, "bonusesToSpit_size");
		r.reflect(bonusesToSpit, "bonusesToSpit");
	}

	void CKBumperCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckbcUnk0, "ckbcUnk0");
		r.reflect(ckbcUnk1, "ckbcUnk1");
		r.reflectSize<uint32_t>(ckbcUnk3, "ckbcUnk2");
		r.reflect(ckbcUnk3, "ckbcUnk3");
		r.reflectSize<uint32_t>(ckbcUnk5, "ckbcUnk4");
		r.reflect(ckbcUnk5, "ckbcUnk5");
		r.reflectSize<uint32_t>(ckbcUnk7, "ckbcUnk6");
		r.reflect(ckbcUnk7, "ckbcUnk7");
		r.reflectSize<uint32_t>(ckbcUnk9, "ckbcUnk8");
		r.reflect(ckbcUnk9, "ckbcUnk9");
		r.reflect(ckbcUnk10, "ckbcUnk10");
		r.reflect(ckbcUnk11, "ckbcUnk11");
		r.reflect(ckbcUnk12, "ckbcUnk12");
		r.reflect(ckbcUnk13, "ckbcUnk13");
		r.reflect(ckbcUnk14, "ckbcUnk14");
	}

	void CKBonusHolderCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckbhcUnk0, "ckbhcUnk0");
		r.reflect(ckbhcUnk1, "ckbhcUnk1");
		r.reflect(ckbhcUnk2, "ckbhcUnk2");
		r.reflect(ckbhcUnk3, "ckbhcUnk3");
		r.reflect(ckbhcUnk4, "ckbhcUnk4");
		r.reflect(ckbhcUnk5, "ckbhcUnk5");
		r.reflect(ckbhcUnk6, "ckbhcUnk6");
		r.reflectComposed(explosionFx, "explosionFx", kenv);
	}

	void CKSMCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(cksmcUnk0, "cksmcUnk0");
		r.reflect(cksmcUnk1, "cksmcUnk1");
		r.reflect(cksmcUnk2, "cksmcUnk2");
		r.reflect(cksmcUnk3, "cksmcUnk3");
		r.reflect(cksmcUnk4, "cksmcUnk4");
		r.reflect(cksmcUnk5, "cksmcUnk5");
		r.reflect(cksmcUnk6, "cksmcUnk6");
		r.reflect(cksmcUnk7, "cksmcUnk7");
		r.reflect(cksmcUnk8, "cksmcUnk8");
		r.reflect(cksmcUnk9, "cksmcUnk9");
	}

	void CKRollingBarrelCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			r.reflectSize<uint32_t>(barrelPool, "barrelPool");
			r.reflect(barrelPool, "barrelPool");
			return;
		}
		r.reflectSize<uint8_t>(barrels, "barrels_size");
		r.reflect(ckrbcUnk1, "ckrbcUnk1");
		r.reflect(ckrbcUnk2, "ckrbcUnk2");
		r.foreachElement(barrels, "barrels", [&](Barrel& b) {
			r.reflect(b.ckrbcUnk3, "ckrbcUnk3");
			r.reflect(b.ckrbcUnk4, "ckrbcUnk4");
			r.reflect(b.ckrbcUnk5, "ckrbcUnk5");
			r.reflect(b.ckrbcUnk6, "ckrbcUnk6");
			});
		r.reflect(ckrbcUnk23, "ckrbcUnk23");
		r.reflect(ckrbcUnk24, "ckrbcUnk24");
		r.reflectSize<uint32_t>(ckrbcUnk26, "ckrbcUnk25");
		r.reflect(ckrbcUnk26, "ckrbcUnk26");
		r.reflectComposed(explosionFx, "explosionFx", kenv);
		r.reflect(ckrbcUnk39, "ckrbcUnk39");
		r.reflect(ckrbcUnk40, "ckrbcUnk40");
		r.reflect(ckrbcUnk41, "ckrbcUnk41");
		r.reflect(ckrbcUnk42, "ckrbcUnk42");
		r.reflect(ckrbcUnk43, "ckrbcUnk43");
		r.reflect(ckrbcUnk44, "ckrbcUnk44");
	}

	void CKParticlesSequencerCpnt::onLevelLoaded(KEnvironment* kenv)
	{
		if (kenv->version < KEnvironment::KVERSION_OLYMPIC) {
			for (auto& ref : particleNodes) {
				ref.bind(kenv, -1);
			}
			for (auto& em : emitters) {
				for (auto& part : em.emitParts) {
					part.ckpscUnk8.bind(kenv, -1);
				}
			}
		}
	}

	void CKParticlesSequencerCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflectSize<uint16_t>(particleNodes, "particleNodes_size");
		r.reflectSize<uint16_t>(emitters, "emitters_size");
		r.reflect(particleNodes, "particleNodes");
		r.foreachElement(emitters, "emitters", [&](Emitter& s) {
			r.reflect(s.ckpscUnk3, "ckpscUnk3");
			r.reflect(s.ckpscUnk4, "ckpscUnk4");
			r.reflectSize<uint16_t>(s.emitFrames, "emitFrames_size");
			r.reflectSize<uint16_t>(s.emitParts, "emitParts_size");
			r.foreachElement(s.emitFrames, "emitFrames", [&](Emitter::EmitFrame& f) {
				r.reflect(f.time, "time");
				if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
					r.reflect(f.colorPerhaps, "colorPerhaps");
					r.reflect(f.iDontKnowEmil, "iDontKnowEmil");
				}
				});
			r.foreachElement(s.emitParts, "emitParts", [&](Emitter::EmitPart& p) {
				r.reflect(p.ckpscUnk8, "ckpscUnk8");
				p.ckpscUnk9.resize(s.emitFrames.size());
				r.reflect(p.ckpscUnk9, "ckpscUnk9");
				r.reflect(p.ckpscUnk10, "ckpscUnk10");
				});
			r.reflect(s.ckpscUnk11, "ckpscUnk11");
			});
	}
	
	void CKCatapultCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckccUnk0, "ckccUnk0");
	}

	void CKPushBombCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckpbcUnk0, "ckpbcUnk0");
		r.reflect(ckpbcUnk1, "ckpbcUnk1");
		r.reflect(ckpbcUnk2, "ckpbcUnk2");
		r.reflect(ckpbcUnk3, "ckpbcUnk3");
		r.reflect(ckpbcUnk4, "ckpbcUnk4");
		r.reflect(ckpbcUnk5, "ckpbcUnk5");
		r.reflect(ckpbcUnk6, "ckpbcUnk6");
		r.reflect(ckpbcUnk7, "ckpbcUnk7");
		r.reflect(ckpbcUnk8, "ckpbcUnk8");
		r.reflect(ckpbcUnk9, "ckpbcUnk9");
		r.reflect(ckpbcUnk10, "ckpbcUnk10");
		r.reflect(ckpbcUnk11, "ckpbcUnk11");
		r.reflect(ckpbcUnk12, "ckpbcUnk12");
		r.reflect(ckpbcUnk13, "ckpbcUnk13");
		r.reflect(ckpbcUnk14, "ckpbcUnk14");
		r.reflect(ckpbcUnk15, "ckpbcUnk15");
		r.reflect(ckpbcUnk16, "ckpbcUnk16");
		r.reflect(ckpbcUnk17, "ckpbcUnk17");
		r.reflect(ckpbcUnk18, "ckpbcUnk18");
		r.reflect(ckpbcUnk19, "ckpbcUnk19");
		r.reflect(ckpbcUnk20, "ckpbcUnk20");
		r.reflect(ckpbcUnk21, "ckpbcUnk21");
		r.reflect(ckpbcUnk22, "ckpbcUnk22");
		r.reflect(ckpbcUnk23, "ckpbcUnk23");
		r.reflect(ckpbcUnk24, "ckpbcUnk24");
		r.reflectSize<uint32_t>(ckpbcUnk26, "ckpbcUnk26_size");
		r.reflect(ckpbcUnk26, "ckpbcUnk26");
		r.reflectComposed(explosionFx, "explosionFx", kenv);
		r.reflect(ckpbcUnk39, "ckpbcUnk39");
		r.reflect(ckpbcUnk40, "ckpbcUnk40");
		r.reflect(ckpbcUnk41, "ckpbcUnk41");
	}

	void CKPushCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(numAnims, "numAnims");
		ckpcUnk1.resize(numAnims);
		ckpcUnk2.resize(numAnims);
		r.reflect(ckpcUnk1, "ckpcUnk1");
		r.reflect(ckpcUnk2, "ckpcUnk2");
	}

	void CKCorridorEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckcecUnk0, "ckcecUnk0");
		r.reflect(ckcecUnk1, "ckcecUnk1");
		r.reflect(ckcecUnk2, "ckcecUnk2");
		r.reflect(ckcecUnk3, "ckcecUnk3");
	}

	void CKMovableBlocCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckmbcUnk0, "ckmbcUnk0");
		r.reflect(ckmbcUnk1, "ckmbcUnk1");
		r.reflect(ckmbcUnk2, "ckmbcUnk2");
		r.reflect(ckmbcUnk3, "ckmbcUnk3");
		r.reflect(ckmbcUnk4, "ckmbcUnk4");
		r.reflect(ckmbcUnk5, "ckmbcUnk5");
		r.reflect(ckmbcUnk6, "ckmbcUnk6");
		r.reflect(ckmbcUnk7, "ckmbcUnk7");
		r.reflect(ckmbcUnk8, "ckmbcUnk8");
	}

	void CKTelepherTowedCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckttcUnk0, "ckttcUnk0");
	}

	void CKA2ComboMenhirRainData::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckacmrdUnk0, "ckacmrdUnk0");
		r.reflect(ckacmrdUnk1, "ckacmrdUnk1");
		r.reflect(ckacmrdUnk2, "ckacmrdUnk2");
		r.reflect(ckacmrdUnk3, "ckacmrdUnk3");
		r.reflect(ckacmrdUnk4, "ckacmrdUnk4");
		r.reflect(ckacmrdUnk5, "ckacmrdUnk5");
		r.reflect(ckacmrdUnk6, "ckacmrdUnk6");
		r.reflect(ckacmrdUnk7, "ckacmrdUnk7");
		r.reflect(ckacmrdUnk8, "ckacmrdUnk8");
		r.reflectSize<uint32_t>(ckacmrdUnk10, "ckacmrdUnk10_size");
		r.reflect(ckacmrdUnk10, "ckacmrdUnk10");
		r.reflectSize<uint32_t>(ckacmrdUnk12, "ckacmrdUnk12_size");
		r.reflect(ckacmrdUnk12, "ckacmrdUnk12");
		r.reflect(ckacmrdUnk13, "ckacmrdUnk13");
		r.reflect(ckacmrdUnk14, "ckacmrdUnk14");
		r.reflect(ckacmrdUnk15, "ckacmrdUnk15");
		r.reflect(ckacmrdUnk16, "ckacmrdUnk16");
		r.reflect(ckacmrdUnk17, "ckacmrdUnk17");
		r.reflect(ckacmrdUnk18, "ckacmrdUnk18");
		r.reflect(ckacmrdUnk19, "ckacmrdUnk19");
		r.reflectComposed(electricArcFx, "electricArcFx", kenv);
		r.reflectComposed(flashFx, "flashFx", kenv);
		r.reflectComposed(shockWaveFx, "shockWaveFx", kenv);
		r.reflectComposed(quakeData, "quakeData", kenv);
	}

	void CKA2ComboLightningData::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckacldUnk0, "ckacldUnk0");
		r.reflect(ckacldUnk1, "ckacldUnk1");
		r.reflect(ckacldUnk2, "ckacldUnk2");
		r.reflect(ckacldUnk3, "ckacldUnk3");
		r.reflect(ckacldUnk4, "ckacldUnk4");
		r.reflect(ckacldUnk5, "ckacldUnk5");
		r.reflect(ckacldUnk6, "ckacldUnk6");
		r.reflectComposed(electricFx, "electricFx", kenv);
		r.reflectComposed(flashFx, "flashFx", kenv);
		r.reflectComposed(powerBallFx, "powerBallFx", kenv);
		r.reflectComposed(shockWaveFx, "shockWaveFx", kenv);
		r.reflect(ckacldUnk68, "ckacldUnk68");
		r.reflect(ckacldUnk69, "ckacldUnk69");
		r.reflect(ckacldUnk70, "ckacldUnk70");
		r.reflect(ckacldUnk71, "ckacldUnk71");
		r.reflect(ckacldUnk72, "ckacldUnk72");
		r.reflect(ckacldUnk73, "ckacldUnk73");
		r.reflect(ckacldUnk74, "ckacldUnk74");
		r.reflect(ckacldUnk75, "ckacldUnk75");
		r.reflect(ckacldUnk76, "ckacldUnk76");
		r.reflect(ckacldUnk77, "ckacldUnk77");
		r.reflect(ckacldUnk78, "ckacldUnk78");
		r.reflect(ckacldUnk79, "ckacldUnk79");
		r.reflect(ckacldUnk80, "ckacldUnk80");
		r.reflect(ckacldUnk81, "ckacldUnk81");
		r.reflect(ckacldUnk82, "ckacldUnk82");
		r.reflect(ckacldUnk83, "ckacldUnk83");
		r.reflect(ckacldUnk84, "ckacldUnk84");
		r.reflectSize<uint32_t>(ckacldUnk86, "ckacldUnk86_size");
		r.reflect(ckacldUnk86, "ckacldUnk86");
		r.reflect(ckacldUnk91, "ckacldUnk91");
		r.reflect(ckacldUnk92, "ckacldUnk92");
		r.reflect(ckacldUnk93, "ckacldUnk93");
		r.reflect(ckacldUnk94, "ckacldUnk94");
		r.reflect(ckacldUnk95, "ckacldUnk95");
		r.reflect(ckacldUnk96, "ckacldUnk96");
		r.reflect(ckacldUnk97, "ckacldUnk97");
		r.reflect(ckacldUnk98, "ckacldUnk98");
		r.reflect(ckacldUnk99, "ckacldUnk99");
		r.reflect(ckacldUnk100, "ckacldUnk100");
		r.reflect(ckacldUnk101, "ckacldUnk101");
		r.reflect(ckacldUnk102, "ckacldUnk102");
		r.reflect(ckacldUnk103, "ckacldUnk103");
		r.reflect(ckacldUnk104, "ckacldUnk104");
		r.reflect(ckacldUnk105, "ckacldUnk105");
		r.reflect(ckacldUnk106, "ckacldUnk106");
		r.reflect(ckacldUnk107, "ckacldUnk107");
		r.reflect(ckacldUnk108, "ckacldUnk108");
		r.reflect(ckacldUnk109, "ckacldUnk109");
	};

	void CKA2ComboTwisterData::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckactdUnk0, "ckactdUnk0");
		r.reflect(ckactdUnk1, "ckactdUnk1");
		r.reflect(ckactdUnk2, "ckactdUnk2");
		r.reflectComposed(electricArcFx1, "electricArcFx1", kenv);
		r.reflectComposed(electricArcFx2, "electricArcFx2", kenv);
		r.reflectComposed(flashFx, "flashFx", kenv);
		r.reflect(ckactdUnk41, "ckactdUnk41");
		r.reflect(ckactdUnk42, "ckactdUnk42");
		r.reflect(ckactdUnk43, "ckactdUnk43");
		r.reflect(ckactdUnk44, "ckactdUnk44");
		r.reflect(ckactdUnk45, "ckactdUnk45");
		r.reflect(ckactdUnk46, "ckactdUnk46");
		r.reflect(ckactdUnk47, "ckactdUnk47");
		r.reflect(ckactdUnk48, "ckactdUnk48");
		r.reflect(ckactdUnk49, "ckactdUnk49");
		r.reflect(ckactdUnk50, "ckactdUnk50");
		r.reflect(ckactdUnk51, "ckactdUnk51");
		r.reflect(ckactdUnk52, "ckactdUnk52");
		r.reflect(ckactdUnk53, "ckactdUnk53");
		r.reflect(ckactdUnk54, "ckactdUnk54");
		r.reflect(ckactdUnk55, "ckactdUnk55");
		r.reflect(ckactdUnk56, "ckactdUnk56");
		r.reflect(ckactdUnk57, "ckactdUnk57");
		r.reflect(ckactdUnk58, "ckactdUnk58");
		r.reflect(ckactdUnk59, "ckactdUnk59");
		r.reflect(ckactdUnk60, "ckactdUnk60");
		r.reflect(ckactdUnk61, "ckactdUnk61");
		r.reflectComposed(quakeData, "quakeData", kenv);
		r.reflect(ckactdUnk67, "ckactdUnk67");
		r.reflect(ckactdUnk68, "ckactdUnk68");
		r.reflect(ckactdUnk69, "ckactdUnk69");
		r.reflectSize<uint32_t>(ckactdUnk71, "ckactdUnk71_size");
		r.reflect(ckactdUnk71, "ckactdUnk71");

		r.reflect(ckactdUnk76, "ckactdUnk76");
		r.reflect(ckactdUnk77, "ckactdUnk77");
		r.reflect(ckactdUnk78, "ckactdUnk78");
		r.reflect(ckactdUnk79, "ckactdUnk79");
		r.reflect(ckactdUnk80, "ckactdUnk80");
		r.reflect(ckactdUnk81, "ckactdUnk81");
		r.reflect(ckactdUnk82, "ckactdUnk82");
		r.reflect(ckactdUnk83, "ckactdUnk83");
		r.reflect(ckactdUnk84, "ckactdUnk84");
		r.reflect(ckactdUnk85, "ckactdUnk85");
		r.reflect(ckactdUnk86, "ckactdUnk86");
		r.reflect(ckactdUnk87, "ckactdUnk87");
		r.reflect(ckactdUnk88, "ckactdUnk88");
		r.reflect(ckactdUnk89, "ckactdUnk89");
		r.reflect(ckactdUnk90, "ckactdUnk90", this);
		r.reflect(ckactdUnk91, "ckactdUnk91", this);
		r.reflect(ckactdUnk92, "ckactdUnk92");
		r.reflect(ckactdUnk93, "ckactdUnk93");
		r.reflect(ckactdUnk94, "ckactdUnk94");
		r.reflect(ckactdUnk95, "ckactdUnk95");
		r.reflect(ckactdUnk96, "ckactdUnk96");
	}
	void CKA2FlashInterface::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(flashUI, "flashUI");
		r.reflectSize<uint32_t>(fiVec1, "fiVec1_size");
		r.foreachElement(fiVec1, "fiVec1", [&](FIStruct1& s) {
			r.reflect(s.ckafigbUnk2, "ckafigbUnk2");
			r.reflect(s.ckafigbUnk3, "ckafigbUnk3");
			});
		fiVec2.resize(this->getS2Count());
		r.reflect(fiVec2, "fiVec2");
		r.reflectSize<uint32_t>(fiVec3, "fiVec3_size");
		r.foreachElement(fiVec3, "fiVec3", [&](FIStruct3& s) {
			r.reflect(s.flashHotSpot, "flashHotSpot");
			r.reflect(s.node, "node");
			r.reflect(s.vectorValues, "vectorValues");
			r.reflect(s.floatValues, "floatValues");
			});
		r.reflectSize<uint32_t>(fiVec4, "fiVec4_size");
		r.foreachElement(fiVec4, "fiVec4", [&](FIStruct4& s) {
			r.reflect(s.ckafigbUnk14, "ckafigbUnk14");
			r.reflect(s.ckafigbUnk15, "ckafigbUnk15");
			r.reflect(s.ckafigbUnk16, "ckafigbUnk16");
			r.reflect(s.ckafigbUnk17, "ckafigbUnk17");
			r.reflect(s.ckafigbUnk18, "ckafigbUnk18");
			});
	}
	void CKA2FlashInGame::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		CKA2FlashInterface::reflectMembers2(r, kenv);
		r.reflectSize<uint32_t>(ckafigUnk0, "ckafigUnk0_size");
		r.reflect(ckafigUnk0, "ckafigUnk0");
		r.reflect(ckafigUnk1, "ckafigUnk1");
		r.reflect(ckafigUnk2, "ckafigUnk2");
		r.reflect(ckafigUnk3, "ckafigUnk3");
		r.reflect(ckafigUnk4, "ckafigUnk4");
		r.reflect(ckafigUnk5, "ckafigUnk5");
	};

	void CKA2FlashMenuOutGame::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKA2FlashInterface::reflectMembers2(r, kenv);
		r.reflect(fmogRef, "fmogRef");
	}

	void CKA2BossGrid::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(bossGridLenX, "bossGridLenX");
		r.reflect(bossGridLenZ, "bossGridLenZ");
		bossCells.resize(bossGridLenX * bossGridLenZ);
		r.reflect(bossCells, "bossCells");
		r.reflect(ckabgUnk51, "ckabgUnk51");
		r.reflectSize<uint8_t>(ckabgUnk53, "ckabgUnk53_size");
		r.reflect(ckabgUnk53, "ckabgUnk53");
		ckabgUnk58.resize(bossGridLenX * bossGridLenZ);
		r.reflect(ckabgUnk58, "ckabgUnk58");
	}

	void CKA2BossCell::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflect(ckabcUnk0, "ckabcUnk0");
		r.reflect(ckabcUnk1, "ckabcUnk1");
		r.reflect(ckabcUnk2, "ckabcUnk2");
		r.reflect(ckabcUnk3, "ckabcUnk3");
		r.reflect(ckabcUnk4, "ckabcUnk4");
		r.reflect(ckabcUnk5, "ckabcUnk5");
	}

	void CKA2BossSequence::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckabsUnk0, "ckabsUnk0");
		r.reflect(ckabsUnk1, "ckabsUnk1");
		r.reflect(ckabsUnk2, "ckabsUnk2");
		r.reflect(ckabsUnk3, "ckabsUnk3");
		r.reflect(ckabsUnk4, "ckabsUnk4");
		r.reflect(ckabsUnk5, "ckabsUnk5");
		r.reflect(ckabsUnk6, "ckabsUnk6");
		r.reflect(ckabsUnk7, "ckabsUnk7");
		r.reflect(ckabsUnk8, "ckabsUnk8");
		r.reflect(ckabsUnk9, "ckabsUnk9");
		r.reflect(ckabsUnk10, "ckabsUnk10");
		r.reflect(ckabsUnk11, "ckabsUnk11");
		r.reflect(ckabsUnk12, "ckabsUnk12");
		r.reflect(ckabsUnk13, "ckabsUnk13");
		r.reflect(ckabsUnk14, "ckabsUnk14");
		r.reflect(ckabsUnk15, "ckabsUnk15");
		r.reflect(ckabsUnk16, "ckabsUnk16");
		r.reflect(ckabsUnk17, "ckabsUnk17");
		r.reflect(ckabsUnk18, "ckabsUnk18");
		r.reflect(ckabsUnk19, "ckabsUnk19");
		r.reflect(ckabsUnk20, "ckabsUnk20");
		r.reflect(ckabsUnk21, "ckabsUnk21");
		r.reflect(ckabsUnk22, "ckabsUnk22");
		r.reflect(ckabsUnk23, "ckabsUnk23");
		r.reflect(ckabsUnk24, "ckabsUnk24");
	}
}