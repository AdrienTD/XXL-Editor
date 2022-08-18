#include "CKGameX2.h"
#include "CKComponent.h"
#include "CKNode.h"
#include "CKDictionary.h"
#include "CKLogic.h"
#include "CKCamera.h"

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
		if (kenv->version == KEnvironment::KVERSION_XXL2 && (kenv->isXXL2Demo || kenv->isRemaster)) {
			r.reflect(ckhdNonFinalFloat, "ckhdNonFinalFloat");
		}
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(ckhdUnk0_B, "ckhdUnk0_B");
			r.reflect(ckhdUnk0_C, "ckhdUnk0_C");
		}
		r.reflect(ckhdUnk1, "ckhdUnk1");
		r.reflectSize<uint8_t>(ckhdDetectors, "ckhdDetectors");
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
		r.reflect(ckhabUnk90, "ckhabUnk90");
		if (kenv->isRemaster)
			r.reflect(ckhabUnk90RomasterDuplicate, "ckhabUnk90RomasterDuplicate");
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
		if (kenv->isXXL2Demo)
			r.reflect(ckhahUnkBefore155, "ckhahUnkBefore155");
		r.reflect(ckhahUnk155, "ckhahUnk155");
		r.reflect(ckhahUnk156, "ckhahUnk156");
		r.reflect(ckhahUnk157, "ckhahUnk157");
		r.reflect(ckhahUnk158, "ckhahUnk158");
		r.reflect(ckhahUnk159, "ckhahUnk159");
		r.reflect(ckhahUnk160, "ckhahUnk160");
		if (kenv->isXXL2Demo)
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
		r.reflect(ckhahUnk210, "ckhahUnk210");
		r.reflect(ckhahUnk211, "ckhahUnk211");
		r.reflect(ckhahUnk212, "ckhahUnk212");
		r.reflect(ckhahUnk213, "ckhahUnk213");
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
}