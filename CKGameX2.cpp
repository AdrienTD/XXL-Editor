#include "CKGameX2.h"
#include "CKComponent.h"
#include "CKNode.h"
#include "CKDictionary.h"
#include "CKLogic.h"
#include "CKCamera.h"
#include "CKGraphical.h"

namespace GameX2 {
	void CKHkBonusSpitter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckhbsUnk0, "ckhbsUnk0");
		r.reflect(ckhbsUnk1, "ckhbsUnk1");
		r.reflect(ckhbsUnk2, "ckhbsUnk2");
		r.reflect(ckhbsUnk3, "ckhbsUnk3");
		r.reflect(ckhbsUnk4, "ckhbsUnk4");
	}

	void CKHkActivator::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
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
		r.reflect(ckhdUnk0, "ckhdUnk0");
		r.message("DRM");
		r.reflect(ckhdNonFinalFloat, "ckhdNonFinalFloat");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ckhdUnk0_B, "ckhdUnk0_B");
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
		else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ckhdGround, "ckhdGround");
		}
		r.reflect(ckhdUnk6, "ckhdUnk6");
		r.reflect(ckhdUnk7, "ckhdUnk7");
		r.reflect(ckhdUnk8, "ckhdUnk8");
		r.reflect(ckhdUnk9, "ckhdUnk9", this);
		r.reflect(ckhdUnk10, "ckhdUnk10", this);
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(pfxData, "pfxData");
		}
	}

	void CKHkCrumblyZone::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckhczUnk0, "ckhczUnk0");
		r.reflect(ckhczUnk1, "ckhczUnk1");
		r.reflect(ckhczUnk2, "ckhczUnk2");
		r.reflect(ckhczUnk3, "ckhczUnk3");
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			r.reflect(ckhczUnk4, "ckhczUnk4");
			r.reflect(ckhczUnk5, "ckhczUnk5");
		}
		else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
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
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
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
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
			ckhpGrounds.resize(2);
			r.reflect(ckhpGrounds, "ckhpGrounds");
		}
		else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflectSize<uint8_t>(ckhpGrounds, "ckhpGrounds_size");
			r.reflect(ckhpGrounds, "ckhpGrounds");
		}
		r.reflect(ckhpSpline, "ckhpSpline");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
			r.reflect(ogUnkFloat, "ogUnkFloat");
		r.reflect(ckhpUnk3, "ckhpUnk3");
		r.reflect(ckhpUnk4, "ckhpUnk4");
		r.reflect(ckhpUnk5, "ckhpUnk5");
		r.reflect(ckhpUnk6, "ckhpUnk6");
		r.reflect(ckhpUnk7, "ckhpUnk7", this);
		r.reflect(ckhpUnk8, "ckhpUnk8", this);
	}

	void CKHkCorridorEnemy::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
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
		r.reflect(ckhpsUnk0, "ckhpsUnk0");
		r.reflect(ckhpsUnk1, "ckhpsUnk1", this);
		r.reflect(ckhpsUnk2, "ckhpsUnk2");
	}

	void CKHkA2BossTrap::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
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
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogEvent4, "ogEvent4", this);
			r.reflect(ogEvent5, "ogEvent5", this);
			r.reflect(ogEvent6, "ogEvent6", this);
			r.reflectSize<uint8_t>(ogHotSpots, "ogHotSpots_size");
			r.reflect(ogHotSpots, "ogHotSpots");
		}
		r.reflect(ckhpsUnk14, "ckhpsUnk14");
		r.reflect(ckhpsUnk15, "ckhpsUnk15");
		r.reflect(ckhpsUnk16, "ckhpsUnk16");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogNewFloat, "ogNewFloat");
		}
		r.reflect(ckhpsUnk17, "ckhpsUnk17");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogNewUnk1, "ogNewUnk1");
			r.reflect(ogNewUnk2, "ogNewUnk2");
			r.reflect(ogNewUnk3, "ogNewUnk3");
		}
	}

	void CKHkSwitch::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		r.reflectSize<uint8_t>(spsBytes, "spsBytes_size");
		r.reflectSize<uint8_t>(spsHooks, "spsHooks_size");
		r.reflect(spsBytes, "spsBytes");
		r.reflect(spsHooks, "spsHooks");
		r.reflect(spsEvent, "spsEvent", this);
	}

	void CKHkShoppingArea::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckhsaUnk0, "ckhsaUnk0");
		r.reflect(ckhsaUnk1, "ckhsaUnk1");
		r.reflect(ckhsaUnk3, "ckhsaUnk3");
		r.reflect(ckhsaUnk5, "ckhsaUnk5");
		r.reflect(ckhsaUnk6, "ckhsaUnk6");
		r.reflect(ckhsaUnk7, "ckhsaUnk7");
	};

	void CKHkMovableCharacter::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
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
		else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogFloats, "ogFloats");
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
		else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogVec1, "ogVec1");
			r.reflect(ogVec2, "ogVec2");
			r.reflectSize<uint16_t>(ogNodes, "ogNodes_size");
			r.reflect(ogNodes, "ogNodes");
			r.reflect(ogObj1, "ogObj1");
			r.reflect(ogObj2, "ogObj2");
			r.reflect(ogObj3, "ogObj3");
			r.reflect(ogEvent1, "ogEvent1", this);
			r.reflect(ogEvent2, "ogEvent2", this);
			r.reflect(ogEvent3, "ogEvent3", this);
			r.reflectComposed(ogPfCpnt, "ogPfCpnt", kenv);
			r.reflect(ogObj4, "ogObj4");
			r.reflect(ogObj5, "ogObj5");
			r.reflect(ogObj6, "ogObj6");
		}
	};

	void CKHkEnemyTarget::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
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
		r.reflect(ckhsmUnk10, "ckhsmUnk10");
		r.reflect(ckhsmUnk11, "ckhsmUnk11");
		r.reflect(ckhsmUnk12, "ckhsmUnk12");
		r.reflect(ckhsmUnk13, "ckhsmUnk13");
		r.reflect(ckhsmUnk14, "ckhsmUnk14");
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
		r.reflect(ckhcpUnk0, "ckhcpUnk0");
		r.reflect(ckhcpUnk1, "ckhcpUnk1");
		r.reflect(ckhcpUnk2, "ckhcpUnk2");
		r.reflect(ckhcpUnk3, "ckhcpUnk3");
	}

	void CKHkFoldawayBridge::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
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
		if (kenv->version == KEnvironment::KVERSION_XXL2) {
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
		r.reflect(ckhrbUnk0, "ckhrbUnk0");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ogBarrelPool, "ogBarrelPool");
		}
		r.reflect(ckhrbUnk1, "ckhrbUnk1");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
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
}