#pragma once

#include "CKHook.h"
#include "CKGroup.h"
#include "CKUtils.h"
#include "CKLogic.h"

using CKBonusSpitterCpnt = CKObject;
using CKMecaCpnt = CKObject;
using CKCrumblyZoneCpnt = CKObject;
using CKA2PotionStoneCpnt = CKObject;
using CKCorridorEnemyCpnt = CKObject;
using CKParticlesSequencerCpnt = CKObject;
using CKParticlesEffectFxData = CKObject;
using CSGHotSpot = CKObject;
using CKBonusData = CKObject;
using CKQuakeCpnt = CKObject;
using CKQuakeCpntUpdater = CKObject;
using CKProjectileAccessor = CKObject;
using CKA2BossGrid = CKObject;
using CKA2BossSequence = CKObject;
using CKWeatherPreset = CKObject;

struct CKOBB;
struct CGlowNodeFx;
struct CCloudsNodeFx;

namespace GameX2 {
	struct CKGrpA2Hero;

	struct CKHkBonusSpitter : CKHookSubclass<CKHkBonusSpitter, CKHook, 14> {
		kobjref<CKBonusSpitterCpnt> ckhbsUnk0;
		KPostponedRef<CKSceneNode> ckhbsUnk1;
		kobjref<CKSoundDictionaryID> ckhbsUnk2;
		float ckhbsUnk3 = 1.4f;
		float ckhbsUnk4 = 1.6f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkActivator : CKHookSubclass<CKHkActivator, CKHook, 52> {
		float ckhaUnk0 = 0.0f;
		kobjref<CAnimationDictionary> ckhaUnk1;
		kobjref<CKSoundDictionaryID> ckhaUnk2;
		kobjref<CKBoundingSphere> ckhaUnk3;
		kobjref<CKBoundingSphere> ckhaUnk4;
		KPostponedRef<CGround> ckhaUnk5;
		KPostponedRef<CGround> ckhaUnk6;
		KPostponedRef<CKSceneNode> ckhaUnk7;
		kobjref<CKMecaCpnt> ckhaUnk8;
		EventNode ckhaUnk9;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkDoor : CKHookSubclass<CKHkDoor, CKHook, 101> {
		float ckhdUnk0; float ckhdNonFinalFloat;
		uint16_t ckhdUnk1;
		//uint8_t ckhdUnk2;
		std::vector<std::array<kobjref<CKDetectorBase>, 2>> ckhdDetectors;
		KPostponedRef<CKSoundDictionaryID> ckhdUnk3;
		KPostponedRef<CDynamicGround> ckhdUnk4;
		KPostponedRef<CDynamicGround> ckhdUnk5;
		KPostponedRef<CKSceneNode> ckhdUnk6;
		kobjref<CKPFGraphTransition> ckhdUnk7;
		kobjref<CKPFGraphTransition> ckhdUnk8;
		EventNode ckhdUnk9;
		EventNode ckhdUnk10;

		// OG
		float ckhdUnk0_B, ckhdUnk0_C;
		KPostponedRef<CGround> ckhdGround;
		kobjref<CKParticlesEffectFxData> pfxData;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkCrumblyZone : CKHookSubclass<CKHkCrumblyZone, CKHook, 102> {
		KPostponedRef<CKSoundDictionaryID> ckhczUnk0;
		KPostponedRef<CGround> ckhczUnk1;
		KPostponedRef<CNode> ckhczUnk2;
		KPostponedRef<CNode> ckhczUnk3;
		KPostponedRef<CKOBB> ckhczUnk4;					//
		kobjref<CKProjectileTypeScrap> ckhczUnk5;
		KPostponedRef<CParticlesNodeFx> ckhczUnk6;
		KPostponedRef<CKObject> ckhczUnk7;
		float ckhczUnk8;
		std::array<float, 3> ckhczUnk9;
		std::array<float, 3> ckhczUnk10;
		uint16_t ckhczUnk11;
		EventNode ckhczUnk12;
		EventNode ckhczUnk13;
		float ckhczUnk14;
		float ckhczUnk15;
		float ckhczUnk16;
		int32_t ckhczUnk17;

		uint8_t ckhczUnk18;
		uint8_t ckhczUnk19;

		KPostponedRef<CGround> ckhczUnk20;
		float ckhczUnk21;
		float ckhczUnk22;
		float ckhczUnk23;
		float ckhczUnk24;
		uint8_t ckhczUnk25;
		std::vector<KPostponedRef<CGround>> ckhczGroundList1;
		std::vector<KPostponedRef<CGround>> ckhczGroundList2;
		kobjref<CKCrumblyZoneCpnt> ckhczUnk28;
		int32_t ckhczUnk29;
		int32_t ckhczUnk30;
		uint8_t ckhczUnk31;
		uint8_t ckhczUnk32;
		float ckhczUnk33;
		uint8_t ckhczUnk34;
		uint8_t ckhczUnk35;
		float ckhczUnk36;
		KPostponedRef<CKObject> ckhczUnk37; // animdict
		float ckhczUnk38;
		float ckhczUnk39;
		float ckhczUnk40;

		// OG:
		uint16_t ogUnk01, ogUnk02, ogUnk03;
		float ogUnk04, ogUnk05, ogUnk06, ogUnk07;
		std::vector<kobjref<CSGHotSpot>> ogHotSpots;
		Vector3 ogUnk08, ogUnk09;
		kobjref<CKBonusData> ogBonusData;
		std::vector<kobjref<CKHkCrumblyZone>> ogNeighbours;
		kobjref<CKQuakeCpnt> ogUnk10;
		kobjref<CKQuakeCpntUpdater> ogUnk11;
		std::vector<kobjref<CKProjectileAccessor>> ogProjectileAccessors;
		std::vector<std::pair<int32_t, kobjref<CSGHotSpot>>> ogHotSpotInfo;
		std::vector<std::pair<kobjref<CKOBB>, Vector3>> ogBoxes;
		kobjref<CKExplosionFxData> ogExplosionFxData;

		int32_t ogMidUnk1, ogMidUnk2, ogMidUnk3;

		kobjref<CKProjectileAccessor> ogProjectileAcc;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2Hero : CKHookSubclass<CKHkA2Hero, CKHook, 218> {
		// based on XXL2 Asterix Level 10

		kobjref<CKGrpA2Hero> ckhahUnk0;
		uint8_t ckhahHeroIndex;
		uint8_t ckhahUnk2;
		uint8_t ckhahUnk3;
		std::array<float, 3> ckhahUnk4;
		std::array<float, 3> ckhahUnk5;
		uint8_t ckhahUnk6;
		uint8_t ckhahUnk7;
		//uint8_t ckhahUnk8;
		std::vector<kobjref<CSGHotSpot>> ckhahUnk9;
		//uint8_t ckhahUnk17;
		std::vector<kobjref<CParticlesNodeFx>> ckhahUnk18;
		//uint8_t ckhahUnk29;
		std::vector<kobjref<CKSceneNode>> ckhahUnk30;
		uint8_t ckhahUnk37 = 0;
		//uint8_t ckhahUnk38;
		std::vector<kobjref<CTrailNodeFx>> ckhahUnk39;
		kobjref<CKShadowCpnt> ckhahUnk45;
		kobjref<CAnimationDictionary> ckhahUnk46;
		kobjref<CKSoundDictionaryID> ckhahUnk47;
		uint8_t ckhahUnk48;
		uint8_t ckhahUnk49;

		CKWaterWaveFxData waterWaveFxData;
		CKWaterSplashFxData waterSplashFxData;
		
		uint8_t ckhahUnk67;
		uint8_t ckhahUnk68;
		uint8_t ckhahUnk69;
		uint8_t ckhahUnk70;
		uint8_t ckhahUnk71;
		uint8_t ckhahUnk72;
		uint8_t ckhahUnk73;
		uint8_t ckhahUnk74;
		uint8_t ckhahUnk75;
		uint8_t ckhahUnk76;
		uint8_t ckhahUnk77;
		uint8_t ckhahUnk78;
		uint8_t ckhahUnk79;
		uint8_t ckhahUnk80;
		uint8_t ckhahUnk81;
		uint8_t ckhahUnk82;
		uint8_t ckhahUnk83;
		uint8_t ckhahUnk84;
		uint8_t ckhahUnk85;
		uint8_t ckhahUnk86;
		uint8_t ckhahUnk87;

		struct UnkStruct1 {
			std::array<float, 3> ckhahUnk88;
			std::array<float, 3> ckhahUnk89;
			float ckhahUnk90;
			float ckhahUnk91;
			uint8_t ckhahUnk92;
			uint8_t ckhahUnk93;
			std::array<uint8_t, 3> ckhahUnk94;
		};
		std::vector<UnkStruct1> unkStructs1; // size given by CKA2GrpHero?
		
		struct UnkStruct2 {
			float ckhahUnk144;
			uint8_t ckhahUnk145;
		};
		std::vector<UnkStruct2> unkStructs2; // size given by CKA2GrpHero?

		std::vector<float> ckhahUnk148; // size given by CKA2GrpHero? always 7
		std::vector<Vector3> ckhahUnk149; // size given by CKA2GrpHero?
		float ckhahUnk152;
		float ckhahUnk153;
		std::array<float, 2> ckhahUnk154;
		float ckhahUnk155;
		float ckhahUnk156;
		float ckhahUnk157;
		float ckhahUnk158;
		float ckhahUnk159;
		float ckhahUnk160;
		float ckhahUnk161;
		float ckhahUnk162;
		float ckhahUnk163;
		float ckhahUnk164;
		float ckhahUnk165;
		float ckhahUnk166;
		float ckhahUnk167;
		std::array<float, 2> ckhahUnk168;
		float ckhahUnk169;
		float ckhahUnk170;
		float ckhahUnk171;
		float ckhahUnk172;
		float ckhahUnk173;
		float ckhahUnk174;
		float ckhahUnk175;
		float ckhahUnk176;
		float ckhahUnk177;
		float ckhahUnk178;
		float ckhahUnk179;
		float ckhahUnk180;
		float ckhahUnk181;
		float ckhahUnk182;
		float ckhahUnk183;
		float ckhahUnk184;
		float ckhahUnk185;
		float ckhahUnk186;
		float ckhahUnk187;
		float ckhahUnk188;
		float ckhahUnk189;
		float ckhahUnk190;
		float ckhahUnk191;
		float ckhahUnk192;
		int32_t ckhahUnk193;
		uint16_t ckhahUnk194;
		uint16_t ckhahUnk195;
		uint16_t ckhahUnk196;
		uint16_t ckhahUnk197;
		uint16_t ckhahUnk198;
		uint16_t ckhahUnk199;
		uint8_t ckhahUnk200;
		uint8_t ckhahUnk201;
		uint8_t ckhahUnk202;
		uint8_t ckhahUnk203;
		uint8_t ckhahUnk204;
		uint8_t ckhahUnk205;
		uint8_t ckhahUnk206;
		uint8_t ckhahUnk207;
		uint8_t ckhahUnk208;
		uint8_t ckhahUnk209;

		float ckhahUnk210;
		int32_t ckhahUnk211;
		float ckhahUnk212;
		float ckhahUnk213;

		uint8_t ckhahUnk214;

		////////////////////

		uint8_t ckhahUnk215;
		uint8_t ckhahUnk216;
		uint8_t ckhahUnk217;
		uint8_t ckhahUnk218;
		uint8_t ckhahUnk219;
		uint8_t ckhahUnk220;
		uint8_t ckhahUnk221;
		uint8_t ckhahUnk222;
		uint8_t ckhahUnk223;
		uint8_t ckhahUnk224;
		std::array<std::array<float, 3>, 5> unkStructs3;

		struct UnkStruct4 {
			float ckhahUnk240;
			float ckhahUnk241;
			uint8_t ckhahUnk242;
		};
		std::array<UnkStruct4, 8> unkStructs4;

		std::array<std::array<float, 2>, 5> unkStructs5;

		struct UnkStruct6 {
			uint8_t ckhahUnk274;
			uint8_t ckhahUnk275;
			uint8_t ckhahUnk276;
			float ckhahUnk277;
		};
		std::array<UnkStruct6, 8> unkStructs6;

		float ckhahUnk306;
		float ckhahUnk307;
		float ckhahUnk308;
		float ckhahUnk309;
		float ckhahUnk310;
		float ckhahUnk311;
		float ckhahUnk312;
		float ckhahUnk313;
		float ckhahUnk314;
		float ckhahUnk315;
		float ckhahUnk316;
		float ckhahUnk317;
		float ckhahUnk318;
		float ckhahUnk319;
		float ckhahUnk320;
		float ckhahUnk321;
		float ckhahUnk322;
		float ckhahUnk323;
		float ckhahUnk324;
		float ckhahUnk325;
		float ckhahUnk326;
		float ckhahUnk327;
		float ckhahUnk328;
		float ckhahUnk329;
		float ckhahUnk330;
		float ckhahUnk331;
		float ckhahUnk332;
		float ckhahUnk333;
		float ckhahUnk334;
		float ckhahUnk335;
		float ckhahUnk336;
		float ckhahUnk337;
		float ckhahUnk338;
		float ckhahUnk339;
		float ckhahUnk340;
		float ckhahUnk341;
		float ckhahUnk342;
		float ckhahUnk343;
		float ckhahUnk344;
		float ckhahUnk345;
		float ckhahUnk346;
		float ckhahUnk347;
		float ckhahUnk348;
		float ckhahUnk349;
		float ckhahUnk350;
		float ckhahUnk351;
		float ckhahUnk352;
		float ckhahUnk353;
		float ckhahUnk354;
		float ckhahUnk355;
		uint8_t ckhahUnk356;
		uint8_t ckhahUnk357;
		uint8_t ckhahUnk358;
		uint8_t ckhahUnk359;
		uint8_t ckhahUnk360;
		uint8_t ckhahUnk361;
		uint8_t ckhahUnk362;
		uint8_t ckhahUnk363;
		uint8_t ckhahUnk364;
		uint8_t ckhahUnk365;
		uint8_t ckhahUnk366;
		uint8_t ckhahUnk367;
		uint8_t ckhahUnk368;
		uint8_t ckhahUnk369;

		CKFireBallFxData fireBallData;
		std::array<CKShockWaveFxData, 5> shockWaveDatas;
		CKCameraQuakeDatas cameraQuakeDatas1, cameraQuakeDatas2;
		CKExplosionFxData explosionData1, explosionData2;

		kobjref<CKSoundDictionaryID> ckhahUnk483;

		// XXL2 Demo
		float ckhahUnkBefore155;
		float ckhahUnk162Duplicate;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2PotionStone : CKHookSubclass<CKHkA2PotionStone, CKHook, 227> {
		kobjref<CAnimationDictionary> ckhapsUnk0;
		kobjref<CKSoundDictionaryID> ckhapsUnk1;
		KPostponedRef<CGround> ckhapsUnk2;
		KPostponedRef<CAnimatedNode> ckhapsUnk3;
		kobjref<CKBoundingSphere> ckhapsUnk4;
		kobjref<CKA2PotionStoneCpnt> ckhapsUnk5;
		EventNode ckhapsUnk6;

		//uint8_t ckhapsUnk7;
		struct AAA {
			std::array<float, 3> ckhapsUnk8;
			float ckhapsUnk9;
			float ckhapsUnk10;
			std::array<float, 3> ckhapsUnk11;
		};
		std::vector<AAA> aaas;

		uint8_t ckhapsUnk20;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkPlatform : CKHookSubclass<CKHkPlatform, CKHook, 233> {
		std::vector<KPostponedRef<CDynamicGround>> ckhpGrounds; // XXL2: Max 2, OG: Any size
		kobjref<CKObject> ckhpSpline;
		float ogUnkFloat = 30.0f;
		std::array<float, 18> ckhpUnk3;
		std::array<float, 9> ckhpUnk4;
		std::array<float, 4> ckhpUnk5;
		KPostponedRef<CKSoundDictionaryID> ckhpUnk6;
		EventNode ckhpUnk7;
		EventNode ckhpUnk8;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkWeatherCenter : CKHookSubclass<CKHkWeatherCenter, CKHook, 234> {
		kobjref<CNode> ckhwcUnk0;
		kobjref<CNode> ckhwcUnk1;
		kobjref<CNode> ckhwcUnk2;
		kobjref<CCloudsNodeFx> ckhwcCloudsNodeFx;
		//uint8_t ckhwcUnk4;
		std::vector<kobjref<CKWeatherPreset>> ckhwcPresetList;
		kobjref<CKWeatherPreset> ckhwcSpecialPreset;
		uint8_t ckhwcUnk14;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkCorridorEnemy : CKHookSubclass<CKHkCorridorEnemy, CKHook, 245> {
		kobjref<CKObject> ckhceUnk0;
		kobjref<CKObject> ckhceUnk1;
		kobjref<CKObject> ckhceUnk2;
		kobjref<CKObject> ckhceUnk3;
		kobjref<CKObject> ckhceUnk4;
		kobjref<CKObject> ckhceUnk5;
		kobjref<CKObject> ckhceUnk6;
		kobjref<CKObject> ckhceUnk7;
		kobjref<CKObject> ckhceUnk8;
		kobjref<CKObject> ckhceUnk9;
		kobjref<CKObject> ckhceUnk10;
		kobjref<CKCorridorEnemyCpnt> ckhceUnk11;
		std::array<float, 33> ckhceUnk12;
		float ckhceUnk13;
		std::array<uint8_t, 9> ckhceUnk14;
		EventNode ckhceUnk15;

		//int32_t ckhceUnk17;
		std::vector<kobjref<CAnimatedClone>> ckhceUnk18;

		KPostponedRef<CAnimatedNode> ckhceUnk38;
		KPostponedRef<CNode> ckhceUnk39;
		kobjref<CKOBB> ckhceUnk40;
		KPostponedRef<CDynamicGround> ckhceUnk41;
		KPostponedRef<CAnimatedNode> ckhceUnk42;
		KPostponedRef<CNode> ckhceUnk43;
		kobjref<CKOBB> ckhceUnk44;
		KPostponedRef<CDynamicGround> ckhceUnk45;

		//int32_t ckhceUnk46;
		struct Move {
			uint8_t ckhceUnk47;
			uint8_t ckhceUnk48;
			float ckhceUnk49;
		};
		std::vector<Move> moves;

		std::array<float, 3> ckhceUnk92;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkParticlesSequencer : CKHookSubclass<CKHkParticlesSequencer, CKHook, 251> {
		kobjref<CKParticlesSequencerCpnt> ckhpsUnk0;
		EventNode ckhpsUnk1;
		int32_t ckhpsUnk2;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2Boss : CKHookSubclass<CKHkA2Boss, CKHook, 254> {
		kobjref<CKSoundDictionaryID> ckhabUnk0;
		std::array<kobjref<CAnimationDictionary>, 5> ckhabUnk1;
		std::array<kobjref<CKBoundingSphere>, 4> ckhabUnk6;
		std::array<kobjref<CKOBB>, 15> ckhabUnk10;
		std::array<kobjref<CSGHotSpot>, 22> ckhabUnk25;
		kobjref<CDynamicGround> ckhabUnk47;
		kobjref<CDynamicGround> ckhabUnk48;
		kobjref<CAnimatedNode> ckhabUnk49;
		kobjref<CAnimatedNode> ckhabUnk50;
		kobjref<CNode> ckhabUnk51;
		kobjref<CAnimatedNode> ckhabUnk52;
		kobjref<CNode> ckhabUnk53;
		kobjref<CNode> ckhabUnk54;
		kobjref<CNode> ckhabUnk55;
		kobjref<CKA2BossGrid> ckhabUnk56;
		kobjref<CKProjectileTypeBallisticPFX> ckhabUnk57;
		kobjref<CKProjectileTypeBallisticPFX> ckhabUnk58;
		kobjref<CKShadowCpnt> ckhabUnk59;
		kobjref<CParticlesNodeFx> ckhabUnk60;
		kobjref<CParticlesNodeFx> ckhabUnk61;
		kobjref<CParticlesNodeFx> ckhabUnk62;
		kobjref<CParticlesNodeFx> ckhabUnk63;
		kobjref<CParticlesNodeFx> ckhabUnk64;
		kobjref<CParticlesNodeFx> ckhabUnk65;
		kobjref<CParticlesNodeFx> ckhabUnk66;
		kobjref<CGlowNodeFx> ckhabUnk67;
		kobjref<CGlowNodeFx> ckhabUnk68;
		kobjref<CKCameraSector> ckhabUnk69;
		kobjref<CKCameraSector> ckhabUnk70;
		float ckhabUnk71;
		float ckhabUnk72;
		EventNode ckhabUnk73;
		EventNode ckhabUnk76;
		EventNode ckhabUnk80;
		EventNode ckhabUnk82;
		EventNode ckhabUnk83;
		EventNode ckhabUnk84;
		EventNode ckhabUnk86;
		EventNode ckhabUnk87;
		std::array<float, 3> ckhabUnk89;
		float ckhabUnk90, ckhabUnk90RomasterDuplicate;
		float ckhabUnk91;
		float ckhabUnk92;
		float ckhabUnk93;
		float ckhabUnk94;
		float ckhabUnk95;
		std::array<float, 18> ckhabUnk96;
		std::array<float, 6> ckhabUnk97;
		std::array<uint16_t, 3> ckhabUnk98;
		float ckhabUnk99;
		std::array<float, 3> ckhabUnk100;
		float ckhabUnk101;
		float ckhabUnk102;
		uint8_t ckhabUnk103;
		uint8_t ckhabUnk104;
		uint8_t ckhabUnk105;
		kobjref<CKA2BossSequence> ckhabUnk106;
		kobjref<CKA2BossSequence> ckhabUnk107;
		kobjref<CKA2BossSequence> ckhabUnk108;

		std::array<CKShockWaveFxData, 3> shockWaveFxDatas;
		
		std::array<float, 3> ckhabUnk151;
		float ckhabUnk152;

		std::array<CKCameraQuakeDatas, 6> cameraQuakeDatas;
		CKFireBallFxData fireBallData;
		std::array<CKExplosionFxData, 3> explosionDatas;
		CKFlashFxData flashData;
		std::array<CKElectricArcFxData, 2> electricArcDatas;

		std::array<float, 2> ckhabUnk266;
		float ckhabUnk267;

		CKPowerBallFxData powerBallData;

		std::array<float, 6> ckhabUnk292;
		float ckhabUnk293;
		float ckhabUnk294;
		float ckhabUnk295;
		float ckhabUnk296;
		float ckhabUnk297;
		float ckhabUnk298;
		uint8_t ckhabUnk299;
		float ckhabUnk300;
		float ckhabUnk301;
		float ckhabUnk302;
		uint8_t ckhabUnk303;
		float ckhabUnk304;
		float ckhabUnk305;
		float ckhabUnk306;
		uint8_t ckhabUnk307;
		float ckhabUnk308;
		float ckhabUnk309;
		float ckhabUnk310;
		float ckhabUnk311;
		float ckhabUnk312;
		float ckhabUnk313;
		uint8_t ckhabUnk314;
		float ckhabUnk315;
		float ckhabUnk316;
		int32_t ckhabUnk317;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2BossTrap : CKHookSubclass<CKHkA2BossTrap, CKHook, 261> {
		kobjref<CKSoundDictionaryID> ckhabtUnk0;
		kobjref<CNode> ckhabtUnk1;
		kobjref<CKBoundingSphere> ckhabtUnk2;
		kobjref<CSGHotSpot> ckhabtUnk3;
		CKExplosionFxData explosionData;
		std::array<float, 3> ckhabtUnk16;
		float ckhabtUnk17;
		float ckhabtUnk18;
		float ckhabtUnk19;
		float ckhabtUnk20;
		float ckhabtUnk21;
		float ckhabtUnk22;
		float ckhabtUnk23;
		float ckhabtUnk24;
		float ckhabtUnk25;
		//int32_t ckhabtUnk26;
		std::vector<std::array<float, 2>> ckhabtUnk27;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2DeathFx : CKHookSubclass<CKHkA2DeathFx, CKHook, 266> {
		CKFlashFxData flashFxData1;
		CKFlashFxData flashFxData2;
		CKElectricArcFxData electricArcFxData;
		float ckhadfUnk31;
		float ckhadfUnk32;
		float ckhadfUnk33;
		float ckhadfUnk34;
		float ckhadfUnk35;
		uint8_t ckhadfUnk36;
		kobjref<CKCamera> ckhadfUnk37;
		float ckhadfUnk38;
		uint8_t ckhadfUnk39;
		std::array<float, 3> ckhadfUnk40;
		std::array<float, 3> ckhadfUnk41;
		float ckhadfUnk42;
		uint8_t ckhadfUnk43;
		std::array<float, 3> ckhadfUnk44;
		std::array<float, 3> ckhadfUnk45;
		kobjref<CKObject> ckhadfBlurFxData;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkShoppingArea : CKPartlyUnknown<CKHook, 9> {};
	//struct CKHkBonusSpitter : CKPartlyUnknown<CKHook, 14> {};
	struct CKHkPressionStone : CKPartlyUnknown<CKHook, 21> {};
	//struct CKHkActivator : CKPartlyUnknown<CKHook, 52> {};
	//struct CKHkDoor : CKPartlyUnknown<CKHook, 101> {};
	//struct CKHkCrumblyZone : CKPartlyUnknown<CKHook, 102> {};
	struct CKHkCrate : CKPartlyUnknown<CKHook, 112> {};
	//struct CKHkBasicBonus : CKPartlyUnknown<CKHook, 114> {};
	struct CKHkTelepher : CKPartlyUnknown<CKHook, 158> {};
	struct CKHkTelepherTowed : CKPartlyUnknown<CKHook, 159> {};
	struct CKHkA2JetPackEnemy : CKPartlyUnknown<CKHook, 167> {};
	struct CKHkMovableCharacter : CKPartlyUnknown<CKHook, 208> {};
	//struct CKHkA2Hero : CKPartlyUnknown<CKHook, 218> {};
	struct CKHkLockMachineGun : CKPartlyUnknown<CKHook, 220> {};
	//struct CKHkA2PotionStone : CKPartlyUnknown<CKHook, 227> {};
	struct CKHkA2Enemy : CKPartlyUnknown<CKHook, 228> {};
	struct CKHkCrumblyZoneAnimated : CKPartlyUnknown<CKHook, 230> {};
	struct CKHkDynamicObject : CKPartlyUnknown<CKHook, 231> {};
	//struct CKHkPlatform : CKPartlyUnknown<CKHook, 233> {};
	//struct CKHkWeatherCenter : CKPartlyUnknown<CKHook, 234> {};
	struct CKHkEnemyTarget : CKPartlyUnknown<CKHook, 235> {};
	struct CKHkEnemyTargetPit : CKPartlyUnknown<CKHook, 236> {};
	struct CKHkWaterWork : CKPartlyUnknown<CKHook, 237> {};
	struct CKHkSwitch : CKPartlyUnknown<CKHook, 238> {};
	struct CKHkCounter : CKPartlyUnknown<CKHook, 241> {};
	struct CKHkA2InvincibleEnemy : CKPartlyUnknown<CKHook, 244> {};
	//struct CKHkCorridorEnemy : CKPartlyUnknown<CKHook, 245> {};
	struct CKHkTelepherAuto : CKPartlyUnknown<CKHook, 246> {};
	struct CKHkA2ArcherEnemy : CKPartlyUnknown<CKHook, 247> {};
	struct CKHkPushBomb : CKPartlyUnknown<CKHook, 249> {};
	struct CKHkMovableBloc : CKPartlyUnknown<CKHook, 250> {};
	//struct CKHkParticlesSequencer : CKPartlyUnknown<CKHook, 251> {};
	struct CKHkA2TurtleEnemy : CKPartlyUnknown<CKHook, 252> {};
	struct CKHkCatapult : CKPartlyUnknown<CKHook, 253> {};
	//struct CKHkA2Boss : CKPartlyUnknown<CKHook, 254> {};
	struct CKHkRollingBarrel : CKPartlyUnknown<CKHook, 256> {};
	struct CKHkFoldawayBridge : CKPartlyUnknown<CKHook, 257> {};
	struct CKHkBumper : CKPartlyUnknown<CKHook, 258> {};
	struct CKHkToll : CKPartlyUnknown<CKHook, 259> {};
	struct CKHkSlotMachine : CKPartlyUnknown<CKHook, 260> {};
	//struct CKHkA2BossTrap : CKPartlyUnknown<CKHook, 261> {};
	struct CKHkCheckPoint : CKPartlyUnknown<CKHook, 262> {};
	struct CKHkA2CrumblyZone : CKSubclass<CKHkCrumblyZone, 263> {};
	struct CKHkA2MarioEnemy : CKPartlyUnknown<CKHook, 265> {};
	//struct CKHkA2DeathFx : CKPartlyUnknown<CKHook, 266> {};
	struct CKHkBonusHolder : CKPartlyUnknown<CKHook, 268> {};

	//struct CKGroupRoot : CKPartlyUnknown<CKGroup, 1> {};
	struct CKGrpA2Boss : CKSubclass<CKGroup, 4> {};
	struct CKGrpMeca : CKPartlyUnknown<CKGroup, 11> {};
	//struct CKGrpSquad : CKPartlyUnknown<CKGroup, 24> {};
	//struct CKGrpPoolSquad : CKPartlyUnknown<CKGroup, 44> {};
	struct CKGrpCrate : CKPartlyUnknown<CKGroup, 60> {};
	//struct CKGrpBonusPool : CKPartlyUnknown<CKGroup, 61> {};
	struct CKGrpA2Hero : CKPartlyUnknown<CKGroup, 86> {};
	struct CKGrpA2LevelPotion : CKPartlyUnknown<CKGroup, 88> {};
	struct CKGrpLevelManager : CKSubclass<CKGroup, 89> {};
	//struct CKGrpA2BonusPool : CKPartlyUnknown<CKGroup, 91> {};
	//struct CKGrpBonus : CKPartlyUnknown<CKGroup, 92> {};
	struct CKGrpA2Enemy : CKPartlyUnknown<CKGroup, 94> {};
	struct CKGrpFightZone : CKPartlyUnknown<CKGroup, 95> {};
	struct CKGrpMecaLast : CKSubclass<CKGroup, 98> {};
	struct CKCommonBaseGroup : CKSubclass<CKGroup, 99> {};
	struct CKFightZoneSectorGrpRoot : CKSubclass<CKGroup, 100> {};
	struct CKGrpA2FoodBasket : CKPartlyUnknown<CKGroup, 103> {};
}