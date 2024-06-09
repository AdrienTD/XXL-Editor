#pragma once

#include <array>
#include <memory>
#include <vector>
#include "CKHook.h"
#include "CKGroup.h"
#include "CKComponent.h"
#include "CKUtils.h"
#include "CKLogic.h"

using CKParticlesEffectFxData = CKObject;
using CKBonusData = CKObject;
using CKQuakeCpnt = CKObject;
using CKQuakeCpntUpdater = CKObject;
using CKProjectileAccessor = CKObject;
using CKCameraBeacon = CKObject;
using CKLocTextAccessor = CKObject;
using CKSekensLauncherCpnt = CKObject;
using CKInputIconFxData = CKObject;
using CKRollingBarrelPool = CKObject;

struct CKOBB;
struct CGlowNodeFx;
struct CCloudsNodeFx;
struct CFogBoxNodeFx;
struct CSGLeaf;
struct CContainer2d;
struct CSGHotSpot;
struct CKVibrationData;
struct CKHDRFxData;
struct CKBomb;
struct CKSandal;
struct CKNumber;
struct CKMarkerBeacon;
struct CKExtendedMarkerBeacon;
struct CKProjectileTypeTargetLock;
struct CKGrpBonusPool;
struct CAnimationDictionary;
struct CKSceneNode;
struct CKBoundingShape;
struct CAnimatedClone;
struct CKHook;
struct CKSoundDictionaryID;
struct CKFlaggedPath;
struct CAnimatedNode;
struct CDynamicGround;
struct CSGBranch;
struct CNode;
struct CClone;
struct CKProjectileTypeScrap;
struct CParticlesNodeFx;
struct CKBoundingSphere;
struct CKSpline4L;
struct CKAABB;
struct CKShadowCpnt;
struct CKProjectileTypeBase;
struct CGround;
struct CKProjectileTypeBallisticPFX;
struct CKCrateCpnt;
struct CColorTextButton2d;
struct CText2d;
struct CBillboard2d;
struct CKAACylinder;
struct CKFlashUI;
struct CFlashMessageBox2d;
struct CKFlashMessageIn;

namespace GameX2 {
	struct CKGrpA2Hero;
	struct CKA2BossGrid;
	struct CKA2BossSequence;

	struct CKCorridorEnemyCpnt : CKMRSubclass<CKCorridorEnemyCpnt, CKReflectableComponent, 1> {
		kobjref<CParticlesNodeFx> ckcecUnk0;
		kobjref<CParticlesNodeFx> ckcecUnk1;
		kobjref<CParticlesNodeFx> ckcecUnk2;
		kobjref<CParticlesNodeFx> ckcecUnk3;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKTargetCpnt : CKMRSubclass<CKTargetCpnt, CKReflectableComponent, 2> {
		kobjref<CNode> cktcUnk0;
		float cktcUnk1;
		float cktcUnk2;
		float cktcUnk3;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKCrumblyZoneCpnt : CKMRSubclass<CKCrumblyZoneCpnt, CKReflectableComponent, 3> {
		//int32_t ckczcUnk0;
		std::vector<kobjref<CParticlesNodeFx>> particleNodes;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKCatapultCpnt : CKMRSubclass<CKCatapultCpnt, CKReflectableComponent, 4> {
		kobjref<CParticlesNodeFx> ckccUnk0;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKEnemyCpnt : CKMRSubclass<CKEnemyCpnt, CKReflectableComponent, 6> {
		int32_t ckaecUnk0;
		CKHkMoveCpnt moveCpnt;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKShadowCpnt : CKMRSubclass<CKShadowCpnt, CKReflectableComponent, 18> {
		float ckscUnk0;
		float ckscUnk1;
		float ckscUnk2;
		float ckscUnk3;
		float ckscUnk4;
		float ckscUnk5;
		float ckscUnk6;
		float ckscUnk7;
		float ckscUnk8;
		uint8_t ckscUnk9;
		uint8_t ckscUnk10;
		uint8_t ckscUnk11;
		uint8_t ckscUnk12;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKBonusCpnt : CKMRSubclass<CKBonusCpnt, CKReflectableComponent, 22> {
		float ckbcUnk0;
		kobjref<CKSoundDictionaryID> ckbcUnk1;
		float ckbcUnk2;
		float ckbcUnk3;
		float ckbcUnk4;
		float ckbcUnk5;
		float ckbcUnk6;
		float ckbcUnk7;
		float ckbcUnk8;
		float ckbcUnk9;
		float ckbcUnk10;
		float ckbcUnk11;
		float ckbcUnk12;
		float ckbcUnk13;
		float ckbcUnk14;
		int32_t ckbcUnk15;
		kobjref<CKProjectileAccessor> ogProjectile;
		float ogFlt1 = 0.5f, ogFlt2 = 0.5f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKPathFindingCpnt : CKMRSubclass<CKPathFindingCpnt, CKReflectableComponent, 35> {
		float pfcUnk1;
		int32_t pfcUnk2;
		float pfcUnk3;
		float pfcOgUnk1; // OG
		float pfcOgUnk2; // OG
		int32_t pfcUnk4;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2EnemyCpnt : CKMRSubclass<CKA2EnemyCpnt, CKEnemyCpnt, 36> {
		uint8_t drm;
		
		float ckaecUnk28;
		float ckaecUnk29;
		int32_t ckaecUnk30;
		float ckaecUnk31;
		float ckaecUnk32;
		uint8_t ckaecUnk33;
		uint8_t ckaecUnk34;
		float ckaecUnk35;
		uint8_t ckaecUnk36;
		uint8_t ckaecUnk37;
		uint8_t ckaecUnk38;
		uint8_t ckaecUnk39;
		uint8_t ckaecUnk40;
		uint8_t ckaecUnk41;
		uint8_t ckaecUnk42;
		float ckaecUnk43;
		float ckaecUnk44;
		uint8_t ckaecUnk45;
		uint8_t ckaecUnk46;
		float ckaecUnk47;
		float ckaecUnk48;
		float ckaecUnk49;

		struct SomeEnemyStruct {
			int32_t ckaecUnk50;
			uint8_t drmVal1;
			uint8_t drmVal2;
			int32_t ckaecUnk51;
			uint8_t ckaecUnk52;
			uint8_t ckaecUnk53;
			uint8_t ckaecUnk54;
			uint8_t ckaecUnk55;
			uint8_t ckaecUnk56;
			float ckaecUnk57;
			uint8_t ckaecUnk58;
			float ckaecUnk59;
			uint8_t ckaecUnk60;
		};
		std::array<SomeEnemyStruct, 3> enstructs;

		float ckaecUnk83;
		float ckaecUnk84;
		uint8_t ckaecUnk85;
		float ckaecUnk86;
		float ckaecUnk87;
		float ckaecUnk88;
		float ckaecUnk89;
		float ckaecUnk90;
		float ckaecUnk91;
		float ckaecUnk92;
		float ckaecUnk93;
		float ckaecUnk94;

		CKHedgeHopTrailFxData hedgeHopTrailFx;

		float ckaecUnk111;
		float ckaecUnk112;
		float ckaecUnk113;

		CKExplosionFxData explosionFx1;
		CKExplosionFxData explosionFx2;

		float ckaecUnk138;
		float ckaecUnk139;
		//int32_t ckaecUnk140;
		std::vector<std::array<float, 2>> ckaecUnk141;

		// OG
		struct A3EnemyCpnt {
			int32_t ckaecUnk0;
			int32_t ckaecUnk1;
			float ckaecUnk2;
			float ckaecUnk3;
			float ckaecUnk4;
			float ckaecUnk5;
			float ckaecUnk6;
			int32_t ckaecUnk7;
			float ckaecUnk8;
			float ckaecUnk9;
			float ckaecUnk10;
			int32_t ckaecUnk11;
			std::array<float, 3> ckaecUnk12;
			std::array<float, 3> ckaecUnk13;
			float ckaecUnk14;
			//int32_t ckaecUnk15;
			struct SES {
				uint8_t ckaecUnk28;
				float ckaecUnk29;
				float ckaecUnk30;
				int32_t ckaecUnk31;
				float ckaecUnk32;
				float ckaecUnk33;
				float ckaecUnk34;
				float ckaecUnk35;
				float ckaecUnk36;
				int32_t ckaecUnk37;
				kobjref<CKProjectileAccessor> ckaecUnk38;
				float ckaecUnk39;
			};
			std::vector<SES> seses;
			kobjref<CKExplosionFxData> ckaecUnk40;
			kobjref<CKExplosionFxData> ckaecUnk41;
			kobjref<CKExplosionFxData> ckaecUnk42;
			kobjref<CKExplosionFxData> ckaecUnk43;
			kobjref<CKExplosionFxData> ckaecUnk44;
			float ckaecUnk45;
			float ckaecUnk46;
			float ckaecUnk47;
			float ckaecUnk48;
			float ckaecUnk49;
			float ckaecUnk50;
			float ckaecUnk51;
			float ckaecUnk52;
			float ckaecUnk53;
			float ckaecUnk54;
			float ckaecUnk55;
			float ckaecUnk56;
			float ckaecUnk57;
			float ckaecUnk58;
			float ckaecUnk59;
			float ckaecUnk60;
			float ckaecUnk61;
			float ckaecUnk62;
			float ckaecUnk63;
			float ckaecUnk64;
			float ckaecUnk65;
			float ckaecUnk66;
			float ckaecUnk67;
			float ckaecUnk68;
			float ckaecUnk69;
			float ckaecUnk70;
			float ckaecUnk71;
			float ckaecUnk72;
			float ckaecUnk73;
			kobjref<CKObject> ckaecUnk74;
			kobjref<CKObject> ckaecUnk75;
		};
		std::shared_ptr<A3EnemyCpnt> ogVersion; // TODO: Replace this with some sort of deep copy pointer

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2JetPackEnemyCpnt : CKMRSubclass<CKA2JetPackEnemyCpnt, CKA2EnemyCpnt, 37> {
		float ckajpecUnk0;
		float ckajpecUnk1;
		float ckajpecUnk2;
		float ckajpecUnk3;
		float ckajpecUnk4;
		float ckajpecUnk5;
		float ckajpecUnk6;
		float ckajpecUnk7;
		float ckajpecUnk8;
		float ckajpecUnk9;
		float ckajpecUnk10;
		float ckajpecUnk11;
		float ckajpecUnk12;
		float ckajpecUnk13;
		uint8_t ckajpecUnk14;
		float ckajpecUnk15;
		uint8_t ckajpecUnk16;
		float ckajpecUnk17;
		float ckajpecUnk18;
		float ckajpecUnk19;
		uint8_t ckajpecUnk20;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKWeatherPreset : CKMRSubclass<CKWeatherPreset, CKReflectableComponent, 39> {
		std::array<float, 8> ckwpUnk0;
		std::array<int32_t, 3> ckwpUnk1;
		std::array<int32_t, 2> ckwpUnk2;
		int32_t ckwpUnk3;
		int32_t ckwpUnk4;
		uint8_t ckwpUnk5;
		std::array<float, 2> ckwpUnk6;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2PotionStoneCpnt : CKMRSubclass<CKA2PotionStoneCpnt, CKReflectableComponent, 41> {
		float ckapscUnk0;
		float ckapscUnk1;
		float ckapscUnk2;
		uint8_t ckapscUnk3;
		uint8_t ckapscUnk4;
		float ckapscUnk5;
		float ckapscUnk6;
		float ckapscUnk7;
		float ckapscUnk8;
		float ckapscUnk9;
		float ckapscUnk10;
		float ckapscUnk11;
		float ckapscUnk12;
		uint8_t ckapscUnk13;
		float ckapscUnk14;
		float ckapscUnk15;
		uint8_t ckapscUnk16;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKMecaCpnt : CKMRSubclass<CKMecaCpnt, CKReflectableComponent, 43> {
		kobjref<CParticlesNodeFx> ckmcUnk0;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKBonusSpitterCpnt : CKMRSubclass<CKBonusSpitterCpnt, CKReflectableComponent, 45> {
		std::array<float, 6> ckbscUnk0;
		std::array<float, 2> ckbscUnk1;
		uint8_t ckbscUnk2;
		//int32_t ckbscUnk3;
		std::vector<std::pair<uint16_t, uint16_t>> bonusesToSpit;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2InvincibleEnemyCpnt : CKMRSubclass<CKA2InvincibleEnemyCpnt, CKA2EnemyCpnt, 46> {
		uint8_t ckaiecUnk0;
		uint8_t ckaiecUnk1;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2ArcherEnemyCpnt : CKMRSubclass<CKA2ArcherEnemyCpnt, CKA2EnemyCpnt, 47> {
		float ckaaecUnk0;
		//int32_t ckaaecUnk1;
		std::vector<float> throwTimes;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKPushBombCpnt : CKMRSubclass<CKPushBombCpnt, CKReflectableComponent, 48> {
		float ckpbcUnk0;
		float ckpbcUnk1;
		float ckpbcUnk2;
		float ckpbcUnk3;
		float ckpbcUnk4;
		float ckpbcUnk5;
		float ckpbcUnk6;
		float ckpbcUnk7;
		float ckpbcUnk8;
		float ckpbcUnk9;
		float ckpbcUnk10;
		float ckpbcUnk11;
		float ckpbcUnk12;
		float ckpbcUnk13;
		float ckpbcUnk14;
		float ckpbcUnk15;
		float ckpbcUnk16;
		float ckpbcUnk17;
		float ckpbcUnk18;
		float ckpbcUnk19;
		float ckpbcUnk20;
		float ckpbcUnk21;
		float ckpbcUnk22;
		kobjref<CNode> ckpbcUnk23;
		float ckpbcUnk24;
		//int32_t ckpbcUnk25;
		std::vector<std::array<float, 2>> ckpbcUnk26;
		CKExplosionFxData explosionFx;
		kobjref<CParticlesNodeFx> ckpbcUnk39;
		kobjref<CParticlesNodeFx> ckpbcUnk40;
		kobjref<CParticlesNodeFx> ckpbcUnk41;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKPushCpnt : CKMRSubclass<CKPushCpnt, CKReflectableComponent, 49> {
		uint8_t numAnims = 0;
		std::vector<float> ckpcUnk1;
		std::vector<kobjref<CAnimationDictionary>> ckpcUnk2;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKParticlesSequencerCpnt : CKMRSubclass<CKParticlesSequencerCpnt, CKReflectableComponent, 50> {
		//uint16_t ckpscUnk0;
		//uint16_t ckpscUnk1;
		std::vector<KPostponedRef<CParticlesNodeFx>> particleNodes;
		struct Emitter {
			uint8_t ckpscUnk3;
			float ckpscUnk4;
			//uint16_t ckpscUnk5;
			//uint16_t ckpscUnk6;
			struct EmitFrame {
				float time;
				uint32_t colorPerhaps;
				float iDontKnowEmil;
			};
			struct EmitPart {
				KPostponedRef<CParticlesNodeFx> ckpscUnk8;
				std::vector<int32_t> ckpscUnk9; // same size as emitTimes
				Matrix ckpscUnk10;
			};
			std::vector<EmitFrame> emitFrames;
			std::vector<EmitPart> emitParts;
			Matrix ckpscUnk11;
		};
		std::vector<Emitter> emitters;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
		void onLevelLoaded(KEnvironment* kenv) override;
	};

	struct CKMovableBlocCpnt : CKMRSubclass<CKMovableBlocCpnt, CKReflectableComponent, 51> {
		kobjref<CKSoundDictionaryID> ckmbcUnk0;
		float ckmbcUnk1;
		float ckmbcUnk2;
		float ckmbcUnk3;
		float ckmbcUnk4;
		float ckmbcUnk5;
		float ckmbcUnk6;
		float ckmbcUnk7;
		float ckmbcUnk8;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKRollingBarrelCpnt : CKMRSubclass<CKRollingBarrelCpnt, CKReflectableComponent, 54> {
		//uint8_t ckrbcUnk0;
		float ckrbcUnk1;
		float ckrbcUnk2;
		struct Barrel {
			kobjref<CSGBranch> ckrbcUnk3;
			kobjref<CClone> ckrbcUnk4;
			kobjref<CKOBB> ckrbcUnk5;
			kobjref<CKSoundDictionaryID> ckrbcUnk6;
		};
		std::vector<Barrel> barrels;
		kobjref<CKShadowCpnt> ckrbcUnk23;
		float ckrbcUnk24;
		//int32_t ckrbcUnk25;
		std::vector<std::array<float, 2>> ckrbcUnk26;
		CKExplosionFxData explosionFx;
		kobjref<CParticlesNodeFx> ckrbcUnk39;
		float ckrbcUnk40;
		float ckrbcUnk41;
		float ckrbcUnk42;
		float ckrbcUnk43;
		float ckrbcUnk44;

		// OG
		std::vector<kobjref<CKRollingBarrelPool>> barrelPool;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKBumperCpnt : CKMRSubclass<CKBumperCpnt, CKReflectableComponent, 59> {
		kobjref<CParticlesNodeFx> ckbcUnk0;
		kobjref<CKSoundDictionaryID> ckbcUnk1;
		//int32_t ckbcUnk2;
		std::vector<std::array<float, 2>> ckbcUnk3;
		//int32_t ckbcUnk4;
		std::vector<std::array<float, 2>> ckbcUnk5;
		//int32_t ckbcUnk6;
		std::vector<std::array<float, 2>> ckbcUnk7;
		//int32_t ckbcUnk8;
		std::vector<std::array<float, 2>> ckbcUnk9;
		uint8_t ckbcUnk10;
		float ckbcUnk11;
		float ckbcUnk12;
		float ckbcUnk13;
		float ckbcUnk14;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2ComboTwisterData : CKMRSubclass<CKA2ComboTwisterData, CKReflectableComponent, 60> {
		kobjref<CNode> ckactdUnk0;
		kobjref<CNode> ckactdUnk1;
		int32_t ckactdUnk2;
		
		CKElectricArcFxData electricArcFx1;
		CKElectricArcFxData electricArcFx2;
		CKFlashFxData flashFx;

		kobjref<CParticlesNodeFx> ckactdUnk41;
		kobjref<CParticlesNodeFx> ckactdUnk42;
		kobjref<CParticlesNodeFx> ckactdUnk43;
		kobjref<CParticlesNodeFx> ckactdUnk44;
		std::array<float, 3> ckactdUnk45;
		std::array<float, 3> ckactdUnk46;
		float ckactdUnk47;
		float ckactdUnk48;
		float ckactdUnk49;
		float ckactdUnk50;
		float ckactdUnk51;
		float ckactdUnk52;
		float ckactdUnk53;
		float ckactdUnk54;
		float ckactdUnk55;
		float ckactdUnk56;
		float ckactdUnk57;
		float ckactdUnk58;
		float ckactdUnk59;
		std::array<float, 2> ckactdUnk60;
		std::array<float, 2> ckactdUnk61;

		CKCameraQuakeDatas quakeData;

		uint8_t ckactdUnk67;
		float ckactdUnk68;
		float ckactdUnk69;
		//int32_t ckactdUnk70;
		std::vector<float> ckactdUnk71;

		uint8_t ckactdUnk76;
		std::array<float, 2> ckactdUnk77;
		std::array<float, 2> ckactdUnk78;
		float ckactdUnk79;
		float ckactdUnk80;
		float ckactdUnk81;
		float ckactdUnk82;
		float ckactdUnk83;
		float ckactdUnk84;
		uint8_t ckactdUnk85;
		float ckactdUnk86;
		kobjref<CNode> ckactdUnk87;
		float ckactdUnk88;
		float ckactdUnk89;
		EventNode ckactdUnk90;
		EventNode ckactdUnk91;
		kobjref<CKSoundDictionaryID> ckactdUnk92;
		uint8_t ckactdUnk93;
		float ckactdUnk94;
		kobjref<CKVibrationData> ckactdUnk95;
		kobjref<CKHDRFxData> ckactdUnk96;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2ComboMenhirRainData : CKMRSubclass<CKA2ComboMenhirRainData, CKReflectableComponent, 61> {
		kobjref<CKProjectileTypeBallisticPFX> ckacmrdUnk0;
		kobjref<CKSoundDictionaryID> ckacmrdUnk1;
		kobjref<CParticlesNodeFx> ckacmrdUnk2;
		float ckacmrdUnk3;
		float ckacmrdUnk4;
		float ckacmrdUnk5;
		float ckacmrdUnk6;
		float ckacmrdUnk7;
		float ckacmrdUnk8;
		//int32_t ckacmrdUnk9;
		std::vector<float> ckacmrdUnk10;
		//int32_t ckacmrdUnk11;
		std::vector<float> ckacmrdUnk12;
		uint8_t ckacmrdUnk13;
		uint8_t ckacmrdUnk14;
		int32_t ckacmrdUnk15;
		float ckacmrdUnk16;
		kobjref<CKVibrationData> ckacmrdUnk17;
		kobjref<CKHDRFxData> ckacmrdUnk18;
		float ckacmrdUnk19;
		CKElectricArcFxData electricArcFx;
		CKFlashFxData flashFx;
		CKShockWaveFxData shockWaveFx;
		CKCameraQuakeDatas quakeData;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKSMCpnt : CKMRSubclass<CKSMCpnt, CKReflectableComponent, 62> {
		kobjref<CParticlesNodeFx> cksmcUnk0;
		kobjref<CParticlesNodeFx> cksmcUnk1;
		kobjref<CParticlesNodeFx> cksmcUnk2;
		float cksmcUnk3;
		float cksmcUnk4;
		float cksmcUnk5;
		float cksmcUnk6;
		int32_t cksmcUnk7;
		float cksmcUnk8;
		float cksmcUnk9;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2ComboLightningData : CKMRSubclass<CKA2ComboLightningData, CKReflectableComponent, 63> {
		kobjref<CParticlesNodeFx> ckacldUnk0;
		kobjref<CParticlesNodeFx> ckacldUnk1;
		kobjref<CParticlesNodeFx> ckacldUnk2;
		kobjref<CParticlesNodeFx> ckacldUnk3;
		kobjref<CTrailNodeFx> ckacldUnk4;
		kobjref<CParticlesNodeFx> ckacldUnk5;
		int32_t ckacldUnk6;

		CKElectricArcFxData electricFx;
		CKFlashFxData flashFx;
		CKPowerBallFxData powerBallFx;
		CKShockWaveFxData shockWaveFx;

		uint8_t ckacldUnk68;
		float ckacldUnk69;
		float ckacldUnk70;
		float ckacldUnk71;
		float ckacldUnk72;
		float ckacldUnk73;
		std::array<float, 3> ckacldUnk74;
		float ckacldUnk75;
		float ckacldUnk76;
		float ckacldUnk77;
		float ckacldUnk78;
		float ckacldUnk79;
		float ckacldUnk80;
		float ckacldUnk81;
		float ckacldUnk82;
		float ckacldUnk83;
		uint8_t ckacldUnk84;
		//int32_t ckacldUnk85;
		std::vector<float> ckacldUnk86;
		uint8_t ckacldUnk91;
		float ckacldUnk92;
		float ckacldUnk93;
		float ckacldUnk94;
		float ckacldUnk95;
		float ckacldUnk96;
		float ckacldUnk97;
		float ckacldUnk98;
		kobjref<CKSoundDictionaryID> ckacldUnk99;
		float ckacldUnk100;
		float ckacldUnk101;
		float ckacldUnk102;
		float ckacldUnk103;
		uint8_t ckacldUnk104;
		float ckacldUnk105;
		kobjref<CKVibrationData> ckacldUnk106;
		kobjref<CKHDRFxData> ckacldUnk107;
		kobjref<CKBoundingSphere> ckacldUnk108;
		float ckacldUnk109;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2MarioEnemyCpnt : CKMRSubclass<CKA2MarioEnemyCpnt, CKA2EnemyCpnt, 64> {
		float mecUnk0;
		float mecUnk1;
		CKShockWaveFxData shockWave;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKTelepherTowedCpnt : CKMRSubclass<CKTelepherTowedCpnt, CKReflectableComponent, 65> {
		kobjref<CParticlesNodeFx> ckttcUnk0;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKBonusHolderCpnt : CKMRSubclass<CKBonusHolderCpnt, CKReflectableComponent, 66> {
		float ckbhcUnk0;
		float ckbhcUnk1;
		float ckbhcUnk2;
		float ckbhcUnk3;
		int32_t ckbhcUnk4;
		float ckbhcUnk5;
		kobjref<CParticlesNodeFx> ckbhcUnk6;
		CKExplosionFxData explosionFx;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkShoppingArea : CKHookSubclass<CKHkShoppingArea, CKHook, 9> {
		float ckhsaUnk0;
		kobjref<CKObject> ckhsaUnk1;
		kobjref<CKObject> ckhsaUnk3;
		float ckhsaUnk5;
		float ckhsaUnk6;
		float ckhsaUnk7;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkBonusSpitter : CKHookSubclass<CKHkBonusSpitter, CKHook, 14> {
		kobjref<CKBonusSpitterCpnt> ckhbsUnk0;
		KPostponedRef<CKSceneNode> ckhbsUnk1;
		kobjref<CKSoundDictionaryID> ckhbsUnk2;
		float ckhbsUnk3 = 1.4f;
		float ckhbsUnk4 = 1.6f;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkPressionStone : CKHookSubclass<CKHkPressionStone, CKHook, 21> {
		KPostponedRef<CNode> ckhpsUnk6;
		KPostponedRef<CDynamicGround> ckhpsUnk7;
		KPostponedRef<CGround> ckhpsUnk8;
		KPostponedRef<CKSoundDictionaryID> ckhpsUnk9;
		EventNode ckhpsUnk10;
		EventNode ckhpsUnk12;
		EventNode ckhpsUnk13;
		float ckhpsUnk14;
		float ckhpsUnk15;
		float ckhpsUnk16;
		uint16_t ckhpsUnk17;

		// OG
		KPostponedRef<CAnimatedNode> ogAnimNode;
		KPostponedRef<CAnimationDictionary> ogAnimDict;
		KPostponedRef<CParticlesNodeFx> ogParticles;
		KPostponedRef<CKObject> ogThirdGround;
		EventNode ogEvent4;
		EventNode ogEvent5;
		EventNode ogEvent6;
		std::vector<kobjref<CSGHotSpot>> ogHotSpots;
		float ogNewFloat = 1.0f;
		uint8_t ogNewUnk1 = 255;
		float ogNewUnk2 = 0.25f;
		float ogNewUnk3 = 0.4f;

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

	struct CKHkA2EnemyBase : CKHookSubclass<CKHkA2EnemyBase, CKHook, 80> {
		kobjref<CAnimationDictionary> ckhaieUnk0;
		//uint8_t ckhaieUnk1;
		std::vector<kobjref<CKObject>> ckhaieUnk2;
		//uint8_t ckhaieUnk3;
		std::vector<kobjref<CDynamicGround>> ckhaieUnk4;
		//uint8_t ckhaieUnk5;
		std::vector<kobjref<CKSceneNode>> ckhaieUnk6;
		uint8_t ckhaieUnk12;
		float ckhaieUnk13;
		float ckhaieUnk14;
		uint8_t ckhaieUnk15;
		uint16_t ckhaieUnk16;
		uint8_t ckhaieUnk17;
		uint8_t ckhaieUnk18;
		uint8_t ckhaieUnk19;
		uint16_t ckhaieUnk20;
		uint8_t ckhaieUnk21;
		uint8_t ckhaieUnk22;
		uint8_t ckhaieUnk23;
		std::array<float, 2> ckhaieUnk24;
		std::array<float, 6> ckhaieUnk25;
		//uint8_t ckhaieUnk26;
		std::vector<kobjref<CSGHotSpot>> ckhaieUnk27;
		kobjref<CKSoundDictionaryID> ckhaieUnk34;
		//uint8_t ckhaieUnk35;
		std::vector<kobjref<CParticlesNodeFx>> ckhaieUnk36;
		//uint8_t ckhaieUnk47;
		std::vector<kobjref<CTrailNodeFx>> ckhaieUnk48;
		//uint8_t ckhaieUnk51;
		std::vector<kobjref<CFogBoxNodeFx>> ckhaieUnk52;
		uint8_t ckhaieUnk53;
		kobjref<CKObject> ckhaieUnk54;
		kobjref<CKObject> ckhaieUnk55;
		std::array<float, 3> ckhaieUnk56;
		CKPathFindingCpnt pathFindCpnt;
		float ckhaieUnk61;
		kobjref<CKObject> ckhaieUnk62;
		std::array<float, 9> ckhaieUnk63;
		int32_t ckhaieUnk64;
		kobjref<CKShadowCpnt> ckhaieUnk65;
		//kobjref<CKSandal> ckhaieUnk66;
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
		float ckhdUnk0_C;
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

		// Arthur+:
		uint16_t ogUnk01, ogUnk02, ogUnk03;
		float ogUnk04, ogUnk05, ogUnk06, ogUnk07;
		std::vector<kobjref<CSGHotSpot>> ogHotSpots;
		Vector3 ogUnk08, ogUnk09;
		// OG:
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

	struct CKHkTelepher : CKHookSubclass<CKHkTelepher, CKHook, 158> {
		kobjref<CKDetectorBase> ckhtUnk0;
		uint8_t ckhtUnk1;
		float ckhtUnk2;
		float ckhtUnk3;
		float ckhtUnk4;
		float ckhtUnk5;
		float ckhtUnk6;
		float ckhtUnk7;
		float ckhtUnk8;
		float ckhtUnk9;
		float ckhtUnk10;
		float ckhtUnk11;
		float ckhtUnk12;
		float ckhtUnk13;
		float ckhtUnk14;
		float ckhtUnk15;
		float ogUnk1, ogUnk2, ogUnk3;
		std::array<float, 3> ckhtUnk16;
		KPostponedRef<CDynamicGround> ckhtUnk17;
		KPostponedRef<CDynamicGround> ckhtUnk18;
		KPostponedRef<CGround> ckhtUnk19;
		KPostponedRef<CGround> ckhtUnk20;
		KPostponedRef<CGround> ckhtUnk21;
		kobjref<CKCameraSector> ckhtUnk22;
		kobjref<CKCameraSector> ckhtUnk23;
		kobjref<CKCameraSector> ckhtUnk24;
		std::vector<kobjref<CKCameraSector>> ogMoreCameraSectors;
		KPostponedRef<CNode> ckhtUnk25;
		KPostponedRef<CNode> ckhtUnk26;
		KPostponedRef<CNode> ckhtUnk27;
		KPostponedRef<CNode> ckhtUnk28;
		kobjref<CKFlaggedPath> ckhtUnk29;
		kobjref<CSGHotSpot> ckhtUnk30;
		kobjref<CSGHotSpot> ckhtUnk31;
		kobjref<CSGHotSpot> ckhtUnk32;
		kobjref<CSGHotSpot> ckhtUnk33;
		kobjref<CSGHotSpot> ckhtUnk34;
		kobjref<CSGHotSpot> ckhtUnk35;
		kobjref<CSGHotSpot> ckhtUnk36;
		kobjref<CSGHotSpot> ckhtUnk37;
		KPostponedRef<CAnimationDictionary> ckhtUnk38;
		kobjref<CKShadowCpnt> ckhtUnk39;
		EventNode ckhtUnk40;
		EventNode ckhtUnk41;
		EventNode ogUnkEvent;
		KPostponedRef<CKSoundDictionaryID> ckhtUnk42;
		kobjref<CKCameraBeacon> ogCameraBeacon;
		kobjref<CKBoundingSphere> ogBoundingSphere;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkTelepherTowed : CKHookSubclass<CKHkTelepherTowed, CKHkTelepher, 159> {
		uint8_t ckhttUnk0;
		float ckhttUnk1;
		float ckhttUnk2;
		float ckhttUnk3;
		float ckhttUnk4;
		float ckhttUnk5;
		float ckhttUnk6;
		float ckhttUnk7;
		float ckhttUnk8;
		float ckhttUnk9;
		float ckhttUnk10;
		float ckhttUnk11;
		float ckhttUnk12;
		float ckhttUnk13;
		float ckhttUnk14;

		int32_t ckhttUnk15;
		int32_t ckhttUnk16;
		// or
		kobjref<CKLocTextAccessor> ogText;
		float ogTowUnk1;
		uint8_t ogTowUnk2;
		uint8_t ogTowUnk3;
		float ogTowUnk4;
		float ogTowUnk5;

		KPostponedRef<CNode> ckhttUnk17;
		KPostponedRef<CNode> ckhttUnk18;
		kobjref<CSGHotSpot> ckhttUnk19;
		KPostponedRef<CAnimationDictionary> ckhttUnk20;
		kobjref<CKDetectorBase> ckhttUnk21;
		KPostponedRef<CKSoundDictionaryID> ckhttUnk22;
		kobjref<CKTelepherTowedCpnt> ckhttUnk23;
		EventNode ckhttUnk24;
		EventNode ckhttUnk25;

		kobjref<CKSekensLauncherCpnt> ogSekensLauncherCpnt;
		kobjref<CKMarkerBeacon> ogMarkerBeacon1;
		kobjref<CKMarkerBeacon> ogMarkerBeacon2;
		kobjref<CKInputIconFxData> ogInputIconFxData;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkTelepherAuto : CKHookSubclass<CKHkTelepherAuto, CKHkTelepher, 246> {
		int32_t ckhtaUnk0;
		float ckhtaUnk1;
		float ckhtaUnk2;
		float ckhtaUnk3;
		float ckhtaUnk4;
		float ckhtaUnk5;
		float ckhtaUnk6;
		float ckhtaUnk7;
		float ckhtaUnk8;
		float ckhtaUnk9;
		float ckhtaUnk10;
		float ckhtaUnk11;
		//int32_t ckhtaUnk12;
		std::vector<std::array<float, 2>> ckhtaUnk13;
		KPostponedRef<CNode> ckhtaUnk14;
		KPostponedRef<CNode> ckhtaUnk15;
		KPostponedRef<CNode> ckhtaUnk16;
		kobjref<CKSoundDictionaryID> ckhtaUnk17;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkMovableCharacter : CKHookSubclass<CKHkMovableCharacter, CKHook, 208> {
		int32_t ckhmcUnk0;
		std::array<float, 3> ckhmcUnk1;
		std::array<float, 3> ckhmcUnk2;
		std::array<float, 3> ckhmcUnk3;
		float ckhmcUnk4;
		float ckhmcUnk5;
		float ckhmcUnk6;
		float ckhmcUnk7;
		float ckhmcUnk8;
		float ckhmcUnk9;
		float ckhmcUnk10;
		float ckhmcUnk11;
		float ckhmcUnk12;
		float ckhmcUnk13;
		float ckhmcUnk14;
		float ckhmcUnk15;
		float ckhmcUnk16;
		float ckhmcUnk17;
		float ckhmcUnk18;
		float ckhmcUnk19;
		float ckhmcUnk20;
		float ckhmcUnk21;
		float ckhmcUnk22;
		float ckhmcUnk23;
		float ckhmcUnk24;
		KPostponedRef<CAnimationDictionary> ckhmcUnk25;
		KPostponedRef<CKSoundDictionaryID> ckhmcUnk26;
		kobjref<CKBoundingSphere> ckhmcUnk27;
		kobjref<CKShadowCpnt> ckhmcUnk28;
		kobjref<CKObject> ckhmcUnk29;
		uint16_t ckhmcUnk30;
		kobjref<CKObject> ckhmcUnk31;

		// OG
		int8_t ogUnk1;
		std::array<float, 35> ogFloats; float ogLastFloat;
		Vector3 ogVec1, ogVec2;
		std::vector<kobjref<CKSceneNode>> ogNodes;
		kobjref<CKObject> ogObj1;
		kobjref<CKObject> ogObj2;
		kobjref<CKObject> ogObj3;
		EventNode ogEvent1;
		EventNode ogEvent2;
		EventNode ogEvent3;
		CKPathFindingCpnt ogPfCpnt;
		kobjref<CKObject> ogObj4;
		kobjref<CKObject> ogObj5;
		kobjref<CKObject> ogObj6;

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

		CKPathFindingCpnt pathFindCpnt;

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

	struct CKHkLockMachineGun : CKHookSubclass<CKHkLockMachineGun, CKHook, 220> {
		uint8_t ckhlmgUnk0;
		float ckhlmgUnk1;
		float ckhlmgUnk2;
		float ckhlmgUnk3;
		float ckhlmgUnk4;
		float ckhlmgUnk5;
		float ckhlmgUnk6;
		float ckhlmgUnk7;
		float ckhlmgUnk8;
		uint8_t ckhlmgUnk9;
		float ckhlmgUnk10;
		float ckhlmgUnk11;
		float ckhlmgUnk12;
		float ckhlmgUnk13;
		uint8_t ckhlmgUnk14;
		float ckhlmgUnk15;
		float ckhlmgUnk16;
		int32_t ckhlmgUnk17;
		int32_t ckhlmgUnk18;
		float ckhlmgUnk19;
		float ckhlmgUnk20;
		float ckhlmgUnk21;
		float ckhlmgUnk22;
		float ckhlmgUnk23;
		float ckhlmgUnk24;
		uint8_t ckhlmgNumVectors;
		kobjref<CAnimationDictionary> ckhlmgUnk26;
		kobjref<CAnimationDictionary> ckhlmgUnk27;
		kobjref<CKSoundDictionaryID> ckhlmgUnk28;
		KPostponedRef<CAnimatedNode> ckhlmgUnk29;
		KPostponedRef<CDynamicGround> ckhlmgUnk30;
		KPostponedRef<CDynamicGround> ckhlmgUnk31;
		kobjref<CKObject> ckhlmgUnk32;
		kobjref<CContainer2d> ckhlmgUnk33;
		kobjref<CBillboard2d> ckhlmgUnk34;
		kobjref<CBillboard2d> ckhlmgUnk35;
		kobjref<CBillboard2d> ckhlmgUnk36;
		kobjref<CBillboard2d> ckhlmgUnk37;
		kobjref<CBillboard2d> ckhlmgUnk38;
		kobjref<CBillboard2d> ckhlmgUnk39;
		kobjref<CBillboard2d> ckhlmgUnk40;
		kobjref<CBillboard2d> ckhlmgUnk41;
		kobjref<CBillboard2d> ckhlmgUnk42;
		kobjref<CBillboard2d> ckhlmgUnk43;
		kobjref<CBillboard2d> ckhlmgUnk44;
		kobjref<CKCameraSector> ckhlmgUnk45;
		kobjref<CKCameraSector> ckhlmgUnk46;
		KPostponedRef<CSGLeaf> ckhlmgUnk47;
		kobjref<CKBoundingSphere> ckhlmgUnk48;
		
		CKCameraQuakeDatas quakeDatas;
		
		std::string ckhlmgUnk55;
		std::string ckhlmgUnk57;
		std::string ckhlmgUnk59;
		std::string ckhlmgUnk61;
		std::string ckhlmgUnk63;
		std::string ckhlmgUnk65;
		std::string ckhlmgUnk67;
		kobjref<CKProjectileTypeTargetLock> ckhlmgUnk68;
		kobjref<CKObject> ckhlmgUnk69;
		kobjref<CSGHotSpot> ckhlmgUnk70;
		kobjref<CSGHotSpot> ckhlmgUnk71;
		uint8_t ckhlmgNumHeroes;
		std::vector<kobjref<CKHkA2Hero>> ckhlmgHeroes;
		std::vector<kobjref<CAnimationDictionary>> ckhlmgHeroAnimDicts;
		std::vector<kobjref<CSGHotSpot>> ckhlmgHeroHotSpots;
		std::vector<Vector3> ckhlmgVectors;
		EventNode ckhlmgUnk85;
		EventNode ckhlmgUnk91;
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

	struct CKHkA2Enemy : CKHookSubclass<CKHkA2Enemy, CKHkA2EnemyBase, 228> {
		kobjref<CKSandal> sandal;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2JetPackEnemy : CKHookSubclass<CKHkA2JetPackEnemy, CKHkA2Enemy, 167> {
		kobjref<CKBomb> bomb;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkCrumblyZoneAnimated : CKHookSubclass<CKHkCrumblyZoneAnimated, CKHkCrumblyZone, 230> {
		KPostponedRef<CAnimationDictionary> ckhczaUnk0;
		float ckhczaUnk1;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkDynamicObject : CKHookSubclass<CKHkDynamicObject, CKHook, 231> {
		struct DOValues {
			std::vector<std::array<float, 2>> fltVec;
			float fltLone;
		};

		// XXL2
		int32_t ckhdoFlags = 0;
		std::array<float, 3> ckhdoUnk1;
		DOValues dov1;
		float ckhdoUnk3;
		float ckhdoUnk4;
		DOValues dov2;
		float ckhdoUnk7;
		std::array<float, 3> ckhdoUnk8;
		std::array<float, 3> ckhdoUnk9;
		DOValues dov3;
		float ckhdoUnk11;
		float ckhdoUnk12;
		Matrix ckhdoMatrix;
		KPostponedRef<CKObject> arUnkRef; // Arthur

		// OG
		float ogUnk0;
		float ogUnk1;
		float ogUnk2;
		int32_t ogUnk3;
		kobjref<CKObject> ogUnk4;
		std::array<float, 3> ogUnk5;
		float ogUnk6;
		float ogUnk7;
		float ogUnk8;
		kobjref<CKObject> ogUnk9;
		float ogUnk10;
		float ogUnk11;
		kobjref<CKObject> ogUnk12;
		std::array<float, 3> ogUnk13;
		std::array<float, 3> ogUnk14;
		float ogUnk15;
		float ogUnk16;
		kobjref<CKObject> ogUnk17;
		float ogUnk18;
		float ogUnk19;
		kobjref<CKObject> ogUnk20;
		float ogUnk21;
		//Matrix ogUnk22;
		Matrix ogSecondMatrix;
		KPostponedRef<CDynamicGround> ogObjRefLast;
		EventNode ogEvent1;
		EventNode ogEvent2;

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

	struct CKHkCounter;

	struct CKHkEnemyTarget : CKHookSubclass<CKHkEnemyTarget, CKHook, 235> {
		kobjref<CKDetectorBase> ckhetUnk0;
		uint8_t ckhetUnk1;
		uint8_t ckhetUnk2;
		int32_t ckhetUnk3;
		float ckhetUnk4;
		float ckhetUnk5;
		kobjref<CKBoundingSphere> ckhetUnk6;
		kobjref<CKSoundDictionaryID> ckhetUnk7;
		kobjref<CKHkCounter> ckhetUnk8;
		KPostponedRef<CSGLeaf> ckhetUnk9;
		EventNode ckhetUnk10;
		EventNode ckhetUnk11;
		kobjref<CKTargetCpnt> ckhetUnk13;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkEnemyTargetPit : CKHookSubclass<CKHkEnemyTargetPit, CKHook, 236> {
		kobjref<CKDetectorBase> ckhetpUnk0;
		kobjref<CKBoundingSphere> ckhetpUnk1;
		kobjref<CKSoundDictionaryID> ckhetpUnk2;
		kobjref<CKBonusSpitterCpnt> ckhetpUnk3;
		KPostponedRef<CNode> ckhetpUnk4;
		KPostponedRef<CNode> ckhetpUnk5;
		KPostponedRef<CNode> ckhetpUnk6;
		KPostponedRef<CGround> ckhetpUnk7;
		KPostponedRef<CGround> ckhetpUnk8;
		KPostponedRef<CDynamicGround> ckhetpUnk9;
		KPostponedRef<CDynamicGround> ckhetpUnk10;
		kobjref<CSGHotSpot> ckhetpUnk11;
		kobjref<CKHkCounter> ckhetpUnk12;
		std::array<float, 3> ckhetpUnk13;
		float ckhetpUnk14;
		float ckhetpUnk15;
		std::array<float, 3> ckhetpUnk16;
		std::array<int32_t, 3> ckhetpUnk17;
		uint8_t ckhetpUnk18;
		EventNode ckhetpUnk19;
		EventNode ckhetpUnk20;
		kobjref<CKTargetCpnt> ckhetpUnk22;
		uint8_t ckhetpUnk23;
		std::vector<std::array<float, 2>> ckhetpUnk24;
		std::vector<std::array<float, 2>> ckhetpUnk25;
		std::vector<std::array<float, 2>> ckhetpUnk26;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkWaterWork : CKHookSubclass<CKHkWaterWork, CKHook, 237> {
		float ckhwwUnk0;
		float ckhwwUnk1;
		float ckhwwUnk2;
		//uint8_t ckhwwNumGrounds;
		//uint8_t ckhwwNumNodes;
		kobjref<CKSoundDictionaryID> ckhwwUnk5;
		EventNode ckhwwUnk6;
		EventNode ckhwwUnk8;
		std::vector<KPostponedRef<CGround>> ckhwwGrounds;
		std::vector<KPostponedRef<CNode>> ckhwwNodes;
		std::array<float, 3> ckhwwUnk26;
		float ckhwwUnk27;
		float ckhwwUnk28;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkSwitch : CKHookSubclass<CKHkSwitch, CKHook, 238> {
		//uint8_t ckhsUnk0;
		//uint8_t ckhsUnk1;
		std::vector<uint8_t> spsBytes;
		std::vector<kobjref<CKHkPressionStone>> spsHooks;
		EventNode spsEvent;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkCounter : CKHookSubclass<CKHkCounter, CKHook, 241> {
		kobjref<CKDetectorBase> ckhcUnk0;
		int32_t ckhcUnk1;
		kobjref<CKSoundDictionaryID> ckhcUnk2;
		kobjref<CKNumber> ckhcUnk3;
		float ckhcUnk4;
		float ckhcUnk5;
		float ckhcUnk6;
		kobjref<CSGBranch> ckhcUnk7;
		//int32_t ckhcUnk8;
		struct CounterValue { // note: same as in CKComparedData??
			kobjref<CKObject> ckhcUnk9;
			int32_t ckhcUnk10;
			int32_t ckhcUnk11;
			int32_t ckhcUnk12;
		};
		std::vector<CounterValue> values;
		uint8_t ckhcUnk13;
		uint8_t ckhcUnk14;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2InvincibleEnemy : CKHookSubclass<CKHkA2InvincibleEnemy, CKHkA2Enemy, 244> {
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

	struct CKHkA2ArcherEnemy : CKHookSubclass<CKHkA2ArcherEnemy, CKHkA2Enemy, 247> {
		kobjref<CKProjectileTypeBallisticPFX> archerProjectiles;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkPushBomb : CKHookSubclass<CKHkPushBomb, CKHook, 249> {
		kobjref<CSGHotSpot> ckhpbUnk0;
		kobjref<CKMecaCpnt> ckhpbUnk1;
		kobjref<CKPushCpnt> ckhpbUnk2;
		kobjref<CKPushBombCpnt> ckhpbUnk3;
		kobjref<CKBoundingSphere> ckhpbUnk4;
		kobjref<CKSoundDictionaryID> ckhpbUnk5;
		EventNode ckhpbUnk6;
		EventNode ckhpbUnk8;
		EventNode ckhpbUnk9;
		EventNode ckhpbUnk10;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkMovableBloc : CKHookSubclass<CKHkMovableBloc, CKHook, 250> {
		kobjref<CKPushCpnt> ckhmbUnk0;
		kobjref<CKMovableBlocCpnt> ckhmbUnk1;
		KPostponedRef<CNode> ckhmbUnk2;
		KPostponedRef<CSGLeaf> ckhmbUnk3;
		KPostponedRef<CDynamicGround> ckhmbUnk4;
		KPostponedRef<CParticlesNodeFx> ckhmbUnk5;
		//uint8_t ckhmbUnk6;
		std::vector<kobjref<CSGHotSpot>> ckhmbUnk7;
		std::array<kobjref<CSGHotSpot>, 4> ckhmbUnk9;
		kobjref<CKCameraSector> ckhmbUnk13;
		kobjref<CKCameraSector> ckhmbUnk14;
		EventNode ckhmbUnk15;
		EventNode ckhmbUnk16;
		EventNode ckhmbUnk17;
		EventNode ckhmbUnk18;
		kobjref<CKFlaggedPath> ckhmbUnk20;
		float ckhmbUnk21;
		float ckhmbUnk22;
		float ckhmbUnk23;
		float ckhmbUnk24;
		float ckhmbUnk25;
		float ckhmbUnk26;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkParticlesSequencer : CKHookSubclass<CKHkParticlesSequencer, CKHook, 251> {
		kobjref<CKParticlesSequencerCpnt> ckhpsUnk0;
		EventNode ckhpsUnk1;
		int32_t ckhpsUnk2;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2TurtleEnemy : CKHookSubclass<CKHkA2TurtleEnemy, CKHkA2EnemyBase, 252> {};

	struct CKHkCatapult : CKHookSubclass<CKHkCatapult, CKHook, 253> {
		KPostponedRef<CAnimatedNode> ckhcUnk0;
		KPostponedRef<CNode> ckhcUnk1;
		kobjref<CKBoundingSphere> ckhcUnk2;
		kobjref<CKAACylinder> ckhcUnk3;
		int32_t ckhcUnk4 = 1;
		int32_t ckhcDemo = 2;
		kobjref<CAnimationDictionary> ckhcUnk5;
		kobjref<CAnimationDictionary> ckhcUnk6;
		kobjref<CAnimationDictionary> ckhcUnk7;
		kobjref<CKFlaggedPath> ckhcUnk8;
		kobjref<CKFlaggedPath> ckhcUnk9;
		float ckhcUnk10;
		kobjref<CSGHotSpot> ckhcUnk11;
		kobjref<CSGHotSpot> ckhcUnk12;
		kobjref<CKSoundDictionaryID> ckhcUnk13;
		kobjref<CKMecaCpnt> ckhcUnk14;
		float ckhcUnk15;
		kobjref<CKCameraSector> ckhcUnk16;
		EventNode ckhcUnk17;
		EventNode ckhcUnk18;
		float ckhcUnk19;
		KPostponedRef<CGround> ckhcUnk20;
		float ckhcUnk21;
		float ckhcUnk22;
		float ckhcUnk23;
		float ckhcUnk24;
		kobjref<CKCatapultCpnt> ckhcUnk25;
		float ckhcUnk26;
		int32_t ckhcUnk27;
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

	struct CKHkRollingBarrel : CKHookSubclass<CKHkRollingBarrel, CKHook, 256> {
		kobjref<CKRollingBarrelCpnt> ckhrbUnk0;
		kobjref<CKFlaggedPath> ckhrbUnk1;
		float ckhrbUnk2;
		//int32_t ckhrbUnk3;
		std::vector<float> ckhrbUnk4;
		float ckhrbUnk5;
		float ckhrbUnk6;
		float ckhrbUnk7;
		float ckhrbUnk8;

		// OG
		kobjref<CKRollingBarrelPool> ogBarrelPool;
		EventNode ogEvent;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkFoldawayBridge : CKHookSubclass<CKHkFoldawayBridge, CKHook, 257> {
		//uint8_t ckhfbUnk0;
		struct FBPart {
			KPostponedRef<CDynamicGround> ckhfbUnk1;
			KPostponedRef<CNode> ckhfbUnk2;
			KPostponedRef<CDynamicGround> ckhfbUnk3;
			KPostponedRef<CNode> ckhfbUnk4;
			KPostponedRef<CDynamicGround> ckhfbUnk5;
		};
		std::vector<FBPart> fbParts;
		float ckhfbUnk16;
		float ckhfbUnk17;
		float ckhfbUnk18;
		float ckhfbUnk19;
		float ckhfbUnk20;
		float ckhfbUnk21;
		kobjref<CKSoundDictionaryID> ckhfbUnk22;
		EventNode ckhfbUnk23;
		float ckhfbUnk25;
		float ckhfbUnk26;
		float ckhfbUnk27;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkBumper : CKHookSubclass<CKHkBumper, CKHook, 258> {
		KPostponedRef<CNode> ckhbUnk0;
		kobjref<CKBoundingSphere> ckhbUnk1;
		kobjref<CKBoundingSphere> ckhbUnk2;
		kobjref<CKFlaggedPath> ckhbUnk3;
		kobjref<CSGHotSpot> ckhbUnk4;
		kobjref<CKBumperCpnt> ckhbUnk5;
		float ckhbUnk6;
		float ckhbUnk7;
		float ckhbUnk8;
		uint8_t ckhbUnk9;
		kobjref<CKCameraSector> ckhbUnk10;
		float ckhbUnk11;
		float ckhbUnk12;
		float ckhbUnk13;
		float ckhbUnk14;
		EventNode ckhbUnk15;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkToll : CKHookSubclass<CKHkToll, CKHook, 259> {
		kobjref<CKSoundDictionaryID> ckhtUnk0;
		KPostponedRef<CAnimatedNode> ckhtUnk1;
		kobjref<CAnimationDictionary> ckhtUnk2;
		kobjref<CKBoundingSphere> ckhtUnk3;
		EventNode ckhtUnk4;
		int32_t ckhtPrice;
		int32_t ckhtUnk7;
		float ckhtUnk8;
		float ckhtUnk9;
		float ckhtUnk10;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkSlotMachine : CKHookSubclass<CKHkSlotMachine, CKHook, 260> {
		kobjref<CKSoundDictionaryID> ckhsmUnk0;
		KPostponedRef<CNode> ckhsmUnk1;
		KPostponedRef<CGround> ckhsmUnk2;
		kobjref<CKBonusSpitterCpnt> ckhsmUnk3;
		kobjref<CAnimationDictionary> ckhsmUnk4;
		kobjref<CSGHotSpot> ckhsmUnk5;
		uint32_t smPrice;
		float ckhsmUnk6;
		float ckhsmUnk7;
		float ckhsmUnk8;
		float ckhsmUnk9;
		std::array<int32_t, 3> smProbabilities;
		int32_t smNumShields;
		int32_t smNumHelmets;
		int32_t smNumBoars;
		int32_t smNumMultipliers;
		float ckhsmUnk15;
		float ckhsmUnk16;
		float ckhsmUnk17;
		kobjref<CKMecaCpnt> ckhsmUnk18;
		kobjref<CKSMCpnt> ckhsmUnk19;
		kobjref<CKTargetCpnt> ckhsmUnk20;
		int32_t ckhsmUnk21;
		int32_t ckhsmUnk22;
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

	struct CKHkCheckPoint : CKHookSubclass<CKHkCheckPoint, CKHook, 262> {
		KPostponedRef<CGround> ckhcpUnk0;
		float ckhcpUnk1;
		int32_t ckhcpUnk2;
		kobjref<CKObject> ckhcpUnk3;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKHkA2CrumblyZone : CKSubclass<CKHkCrumblyZone, 263> {};

	struct CKHkA2MarioEnemy : CKHookSubclass<CKHkA2MarioEnemy, CKHkA2Enemy, 265> {
		kobjref<CKProjectileTypeBallisticPFX> archerProjectiles;
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

	struct CKGrpA2FoodBasket;

	struct CKHkBonusHolder : CKHookSubclass<CKHkBonusHolder, CKHook, 268> {
		float ckhbhUnk0;
		kobjref<CKGrpA2FoodBasket> ckhbhUnk1;
		kobjref<CDynamicGround> ckhbhUnk2;
		kobjref<CKBoundingSphere> ckhbhUnk3;
		kobjref<CSGHotSpot> ckhbhUnk4;
		kobjref<CSGHotSpot> ckhbhUnk5;
		kobjref<CSGHotSpot> ckhbhUnk6;
		kobjref<CSGLeaf> ckhbhUnk7;
		kobjref<CAnimationDictionary> ckhbhUnk8;
		kobjref<CKSoundDictionaryID> ckhbhUnk9;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	//struct CKHkShoppingArea : CKPartlyUnknown<CKHook, 9> {};
	//struct CKHkBonusSpitter : CKPartlyUnknown<CKHook, 14> {};
	//struct CKHkPressionStone : CKPartlyUnknown<CKHook, 21> {};
	//struct CKHkActivator : CKPartlyUnknown<CKHook, 52> {};
	//struct CKHkDoor : CKPartlyUnknown<CKHook, 101> {};
	//struct CKHkCrumblyZone : CKPartlyUnknown<CKHook, 102> {};
	//struct CKHkCrate : CKPartlyUnknown<CKHook, 112> {};
	//struct CKHkBasicBonus : CKPartlyUnknown<CKHook, 114> {};
	//struct CKHkTelepher : CKPartlyUnknown<CKHook, 158> {};
	//struct CKHkTelepherTowed : CKPartlyUnknown<CKHook, 159> {};
	//struct CKHkA2JetPackEnemy : CKPartlyUnknown<CKHook, 167> {};
	//struct CKHkMovableCharacter : CKPartlyUnknown<CKHook, 208> {};
	//struct CKHkA2Hero : CKPartlyUnknown<CKHook, 218> {};
	//struct CKHkLockMachineGun : CKPartlyUnknown<CKHook, 220> {};
	//struct CKHkA2PotionStone : CKPartlyUnknown<CKHook, 227> {};
	//struct CKHkA2Enemy : CKPartlyUnknown<CKHook, 228> {};
	//struct CKHkCrumblyZoneAnimated : CKPartlyUnknown<CKHook, 230> {};
	//struct CKHkDynamicObject : CKPartlyUnknown<CKHook, 231> {};
	//struct CKHkPlatform : CKPartlyUnknown<CKHook, 233> {};
	//struct CKHkWeatherCenter : CKPartlyUnknown<CKHook, 234> {};
	//struct CKHkEnemyTarget : CKPartlyUnknown<CKHook, 235> {};
	//struct CKHkEnemyTargetPit : CKPartlyUnknown<CKHook, 236> {};
	//struct CKHkWaterWork : CKPartlyUnknown<CKHook, 237> {};
	//struct CKHkSwitch : CKPartlyUnknown<CKHook, 238> {};
	//struct CKHkCounter : CKPartlyUnknown<CKHook, 241> {};
	//struct CKHkA2InvincibleEnemy : CKPartlyUnknown<CKHook, 244> {};
	//struct CKHkCorridorEnemy : CKPartlyUnknown<CKHook, 245> {};
	//struct CKHkTelepherAuto : CKPartlyUnknown<CKHook, 246> {};
	//struct CKHkA2ArcherEnemy : CKPartlyUnknown<CKHook, 247> {};
	//struct CKHkPushBomb : CKPartlyUnknown<CKHook, 249> {};
	//struct CKHkMovableBloc : CKPartlyUnknown<CKHook, 250> {};
	//struct CKHkParticlesSequencer : CKPartlyUnknown<CKHook, 251> {};
	//struct CKHkA2TurtleEnemy : CKPartlyUnknown<CKHook, 252> {};
	//struct CKHkCatapult : CKPartlyUnknown<CKHook, 253> {};
	//struct CKHkA2Boss : CKPartlyUnknown<CKHook, 254> {};
	//struct CKHkRollingBarrel : CKPartlyUnknown<CKHook, 256> {};
	//struct CKHkFoldawayBridge : CKPartlyUnknown<CKHook, 257> {};
	//struct CKHkBumper : CKPartlyUnknown<CKHook, 258> {};
	//struct CKHkToll : CKPartlyUnknown<CKHook, 259> {};
	//struct CKHkSlotMachine : CKPartlyUnknown<CKHook, 260> {};
	//struct CKHkA2BossTrap : CKPartlyUnknown<CKHook, 261> {};
	//struct CKHkCheckPoint : CKPartlyUnknown<CKHook, 262> {};
	//struct CKHkA2CrumblyZone : CKSubclass<CKHkCrumblyZone, 263> {};
	//struct CKHkA2MarioEnemy : CKPartlyUnknown<CKHook, 265> {};
	//struct CKHkA2DeathFx : CKPartlyUnknown<CKHook, 266> {};
	//struct CKHkBonusHolder : CKPartlyUnknown<CKHook, 268> {};

	struct CKGrpMeca : CKReflectableGroupSubclass<CKGrpMeca, CKGroup, 11> {
		std::vector<kobjref<CKObject>> components;
		std::vector<int32_t> hookClasses;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKGrpCrate : CKReflectableGroupSubclass<CKGrpCrate, CKGroup, 60> {
		kobjref<CKCrateCpnt> crateCpnt;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKGrpA2Hero : CKReflectableGroupSubclass<CKGrpA2Hero, CKGroup, 86> {
		uint8_t ckgahUnk0;
		//uint8_t ckgahUnk1;
		std::vector<kobjref<CKHkA2Hero>> ckgahUnk2;
		kobjref<CSGLeaf> ckgahUnk5;
		float ckgahUnk6;
		float ckgahUnk7;
		float ckgahUnk8;
		float ckgahUnk9;
		float ckgahUnk10;
		uint16_t ckgahUnk11;
		uint16_t ckgahUnk12;
		uint8_t ckgahUnk13;
		kobjref<CKSoundDictionaryID> ckgahUnk14;
		kobjref<CSGLeaf> ckgahUnk15;
		float ckgahUnk16;
		float ckgahUnk17;
		float ckgahUnk18;
		float ckgahUnk19;
		kobjref<CKA2ComboTwisterData> ckgahUnk20;
		kobjref<CKA2ComboMenhirRainData> ckgahUnk21;
		kobjref<CKA2ComboLightningData> ckgahUnk22;
		//int32_t ckgahUnk23;
		std::vector<kobjref<CKExtendedMarkerBeacon>> ckgahUnk24;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKGrpA2LevelPotion : CKReflectableGroupSubclass<CKGrpA2LevelPotion, CKGroup, 88> {
		float ckgalpUnk0;
		float ckgalpUnk1;
		EventNode ckgalpUnk2;
		float ckgalpUnk3;
		kobjref<CParticlesNodeFx> ckgalpUnk4;
		kobjref<CParticlesNodeFx> ckgalpUnk5;

		CKCameraQuakeDatas quakeData;
		CKFlashFxData flashFx1;
		CKFlashFxData flashFx2;
		CKElectricArcFxData electricArcFx;

		float ckgalpUnk42;
		float ckgalpUnk43;
		float ckgalpUnk44;
		float ckgalpUnk45;
		float ckgalpUnk46;
		float ckgalpUnk47;
		float ckgalpUnk48;
		float ckgalpUnk49;
		float ckgalpUnk50;
		float ckgalpUnk51;
		kobjref<CParticlesNodeFx> ckgalpUnk52;
		std::array<float, 2> ckgalpUnk53;
		kobjref<CParticlesNodeFx> ckgalpUnk54;
		kobjref<CParticlesNodeFx> ckgalpUnk55;
		std::array<float, 2> ckgalpUnk56;
		kobjref<CKBoundingSphere> ckgalpUnk57;
		float ckgalpUnk58;
		kobjref<CKSoundDictionaryID> ckgalpUnk59;
		float ckgalpUnk60;
		float ckgalpUnk61;
		float ckgalpUnk62;
		float ckgalpUnk63;
		kobjref<CAnimationDictionary> ckgalpUnk64;

		CKDistortionFxData distortionFx;

		uint8_t ckgalpUnk69; // might this be a list?
		kobjref<CKObject> ckgalpUnk70;
		kobjref<CKObject> ckgalpUnk71;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKGrpFightZone;
	struct CKCommonBaseGroup;
	struct CKFightZoneSectorGrpRoot;
	struct CKGrpA2Enemy : CKReflectableGroupSubclass<CKGrpA2Enemy, CKGroup, 94> {
		struct GEStruct1 {
			float ges1_1, ges1_2, ges1_3; uint8_t ges1_4;
			float ogges1;
		};
		std::vector<GEStruct1> ges1vec;
		kobjref<CKCommonBaseGroup> poolGroup;
		std::vector<kobjref<CKFightZoneSectorGrpRoot>> fightZoneGroups;
		float unkFloat1 = 0.25f;
		
		struct GEStruct2 {
			float ges2_1, ges2_2; uint8_t ges2_3, ges2_4, ges2_5;
		};
		std::array<GEStruct2, 8> ges2vec;
		
		struct TipBonusValues {
			float tbv1, tbv2;
			std::array<uint8_t, 4> tbv3;
			std::array<uint8_t, 8> tbv4;
			uint8_t tbv5, tbv6;
			void reflectMembers(MemberListener& r);
		};
		TipBonusValues defaultTipBonusValues;
		std::vector<std::pair<kobjref<CKGrpFightZone>, TipBonusValues>> fightZoneTipBonusValues;
		kobjref<CParticlesNodeFx> particleNode;

		// OG
		std::vector<KPostponedRef<CSGBranch>> branches;
		kobjref<CKQuakeCpnt> quakeCpnt;
		EventNode evt1, evt2, evt3, evt4, evt5, evt6, evt7, evt8;
	
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
		void onLevelLoaded(KEnvironment* kenv) override;
	};

	struct CKGrpFightZone : CKReflectableGroupSubclass<CKGrpFightZone, CKGroup, 95> {
		Vector3 zonePos1, zoneSize1, zonePos2, zoneSize2;
		int32_t zoneSomething;

		// OG
		X2FightData fightData;
		//int32_t ckgfzUnk0;
		std::vector<kobjref<CKGrpSquad>> ogSquads;
		// <pools>
		int32_t ckgfzUnk7;
		int32_t ckgfzUnk8;

		// <slots>
		// <slots2>

		int32_t ckgfzUnk37 = 0xFFFFFFFF;
		//int ckgfzUnk38;
		struct GFZThing1 {
			uint8_t gfzUnk1, gfzUnk2, gfzUnk3;
			kobjref<CKObject> squad1, squad2;
		};
		std::vector<GFZThing1> ogUnkGFZThings; // only used by Arthur, unused by OG (but still exists)
		//int32_t ckgfzUnk39;
		std::vector<Vector3> ogUnkVectors;
		uint8_t ckgfzUnk40;
		EventNode ckgfzUnk41;
		EventNode ckgfzUnk42;
		EventNode ckgfzUnk44;
		EventNode ckgfzUnk45;
		EventNode ckgfzUnk46;
		kobjref<CKObject> ckgfzUnk47;

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKGrpA2FoodBasket : CKReflectableGroupSubclass<CKGrpA2FoodBasket, CKGroup, 103> {
		float ckgafbUnk0 = 0.0f;
		int32_t numHolders = 0;
		kobjref<CKHkBonusHolder> curBonusHolder;
		std::vector<kobjref<CKHkBonusHolder>> bonusHolders;
		kobjref<CKBonusHolderCpnt> holderCpnt;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	//struct CKGroupRoot : CKPartlyUnknown<CKGroup, 1> {};
	struct CKGrpA2Boss : CKSubclass<CKGroup, 4> {};
	//struct CKGrpMeca : CKPartlyUnknown<CKGroup, 11> {};
	//struct CKGrpSquad : CKPartlyUnknown<CKGroup, 24> {};
	//struct CKGrpPoolSquad : CKPartlyUnknown<CKGroup, 44> {};
	//struct CKGrpCrate : CKPartlyUnknown<CKGroup, 60> {};
	//struct CKGrpBonusPool : CKPartlyUnknown<CKGroup, 61> {};
	//struct CKGrpA2Hero : CKPartlyUnknown<CKGroup, 86> {};
	//struct CKGrpA2LevelPotion : CKPartlyUnknown<CKGroup, 88> {};
	struct CKGrpLevelManager : CKSubclass<CKGroup, 89> {};
	//struct CKGrpA2BonusPool : CKPartlyUnknown<CKGroup, 91> {};
	//struct CKGrpBonus : CKPartlyUnknown<CKGroup, 92> {};
	//struct CKGrpA2Enemy : CKPartlyUnknown<CKGroup, 94> {};
	//struct CKGrpFightZone : CKPartlyUnknown<CKGroup, 95> {};
	struct CKGrpMecaLast : CKSubclass<CKGroup, 98> {};
	struct CKCommonBaseGroup : CKSubclass<CKGroup, 99> {};
	struct CKFightZoneSectorGrpRoot : CKSubclass<CKGroup, 100> {};
	//struct CKGrpA2FoodBasket : CKPartlyUnknown<CKGroup, 103> {};

	struct CKA2FlashInterface : CKMRSubclass<CKA2FlashInterface, CKReflectableLogic, 219> {
		kobjref<CKFlashUI> flashUI;
		//int32_t ckafigbUnk1;
		struct FIStruct1 {
			int32_t ckafigbUnk2;
			kobjref<CKObject> ckafigbUnk3;
		};
		std::vector<FIStruct1> fiVec1;
		std::vector<std::pair<kobjref<CKObject>, kobjref<CKObject>>> fiVec2;
		//int32_t ckafigbUnk12;
		struct FIStruct3 {
			kobjref<CKObject> flashHotSpot;
			kobjref<CKObject> node;
			std::array<Vector3, 3> vectorValues;
			std::array<float, 5> floatValues;
		};
		std::vector<FIStruct3> fiVec3;
		//int32_t ckafigbUnk13;
		struct FIStruct4 {
			kobjref<CKObject> ckafigbUnk14;
			kobjref<CKObject> ckafigbUnk15;
			kobjref<CKObject> ckafigbUnk16;
			float ckafigbUnk17;
			kobjref<CKObject> ckafigbUnk18;
		};
		std::vector<FIStruct4> fiVec4;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
		virtual int getS2Count() { return 0; }
	};

	struct CKA2FlashInGameBoss : CKSubclass<CKA2FlashInterface, 39> {
		virtual int getS2Count() override { return 0; }
	};
	struct CKA2FlashMenuOutGame : CKMRSubclass<CKA2FlashMenuOutGame, CKA2FlashInterface, 224> {
		kobjref<CKObject> fmogRef;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
		virtual int getS2Count() override { return 44; }
	};
	struct CKA2FlashInGame : CKMRSubclass<CKA2FlashInGame, CKA2FlashInterface, 225> {
		std::vector<kobjref<CKTimeCounter>> ckafigUnk0;
		kobjref<CKSoundDictionaryID> ckafigUnk1;
		kobjref<CText2d> ckafigUnk2;
		kobjref<CKFlashMessageIn> ckafigUnk3;
		kobjref<CKFlashMessageIn> ckafigUnk4;
		kobjref<CKFlashMessageIn> ckafigUnk5;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
		virtual int getS2Count() override { return 31; }
	};
	struct CKA2FlashMenuInGame : CKSubclass<CKA2FlashInterface, 226> {
		virtual int getS2Count() override { return 9; }
	};

	struct CKA2BossCell;
	struct CKA2BossGrid : CKMRSubclass<CKA2BossGrid, CKReflectableLogic, 215> {
		uint8_t bossGridLenX = 0;
		uint8_t bossGridLenZ = 0;
		std::vector<kobjref<CKA2BossCell>> bossCells;
		std::array<float, 3> ckabgUnk51;
		//uint8_t ckabgUnk52;
		std::vector<int32_t> ckabgUnk53;
		std::vector<uint8_t> ckabgUnk58;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2BossCell : CKMRSubclass<CKA2BossCell, CKReflectableLogic, 216> {
		kobjref<CKA2BossGrid> ckabcUnk0;
		kobjref<CKHkA2BossTrap> ckabcUnk1;
		int32_t ckabcUnk2;
		uint8_t ckabcUnk3;
		uint8_t ckabcUnk4;
		uint8_t ckabcUnk5;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2BossSequence : CKMRSubclass<CKA2BossSequence, CKReflectableLogic, 218> {
		int32_t ckabsUnk0;
		float ckabsUnk1;
		float ckabsUnk2;
		float ckabsUnk3;
		float ckabsUnk4;
		float ckabsUnk5;
		float ckabsUnk6;
		float ckabsUnk7;
		float ckabsUnk8;
		float ckabsUnk9;
		float ckabsUnk10;
		int32_t ckabsUnk11;
		float ckabsUnk12;
		float ckabsUnk13;
		float ckabsUnk14;
		float ckabsUnk15;
		float ckabsUnk16;
		float ckabsUnk17;
		float ckabsUnk18;
		int32_t ckabsUnk19;
		float ckabsUnk20;
		float ckabsUnk21;
		float ckabsUnk22;
		float ckabsUnk23;
		std::array<uint16_t, 3> ckabsUnk24;
		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
	};

	struct CKA2GameManager : CKMRSubclass<CKA2GameManager, CKReflectableLogic, 167> {
		kobjref<CKGroup> ckagmGrpEnemy;
		kobjref<CKGroup> ckagmGrpHero;
		kobjref<CKGroup> ckagmGrpMeca;
		kobjref<CKGroup> ckagmGrpBonus;
		kobjref<CKGroup> ckagmGrpLevelManager;
		EventNode ckagmUnk5;
		// ^base
		int32_t ckagmUnk6; // not in HD
		kobjref<CKGrpA2LevelPotion> ckagmUnk7;
		kobjref<CKHkA2DeathFx> ckagmUnk8;
		kobjref<CFlashMessageBox2d> ckagmUnk9;
		EventNode ckagmUnk10;
		EventNode ckagmUnk11;
		EventNode ckagmUnk12;
		kobjref<CKObject> ckagmUnk13;
		kobjref<CKObject> ckagmUnk14;
		kobjref<CKObject> ckagmUnk15;
		float ckagmUnk16;
		float ckagmUnk17;
		float ckagmUnk18;
		float ckagmUnk19;
		int32_t ckagmUnk20;
		kobjref<CKObject> ckagmUnk21;
		int32_t ckagmUnk22;
		kobjref<CKObject> ckagmUnk23;
		std::vector<std::string> ckagmUnk25;
		std::array<int32_t, 9> ckagmUnk43;
		std::array<kobjref<CKObject>, 3> ckagmUnk44;
		struct MenuStuff {
			kobjref<CKObject> gmmnObj;
			std::string gmmnUnk1;
			std::string gmmnUnk2;
		};
		std::vector<MenuStuff> ckagmMenuStuff;
		kobjref<CKObject> ckagmUnk48;
		int32_t ckagmUnk49;
		int32_t ckagmUnk50;
		int32_t ckagmUnk51;
		std::string ckagmUnk52;
		std::string ckagmUnk53;
		std::array<int32_t, 24> ckagmUnk54;
		kobjref<CKObject> ckagmUnk55;
		EventNode ckagmUnk56;
		EventNode ckagmUnk58;
		kobjref<CKObject> ckagmUnk59;
		kobjref<CKObject> ckagmUnk60;
		std::vector<int32_t> ckagmUnk61; // count: 299 original, 249 HD
		kobjref<CKA2FlashMenuOutGame> ckagmUnk62;
		kobjref<CKA2FlashMenuInGame> ckagmUnk63;
		kobjref<CKA2FlashInGame> ckagmUnk64;
		kobjref<CKObject> ckagmUnk65;

		// GAME
		uint32_t ckagmGlob0;
		// ^base
		std::vector<uint32_t> ckagmGlob1;
		// ^base
		kobjref<CKObject> ckagmGlob2;
		kobjref<CKObject> ckagmGlob3;
		kobjref<CKObject> ckagmGlob4;
		std::array<kobjref<CKObject>, 9> ckagmGlob5;
		std::array<kobjref<CKObject>, 11> ckagmGlob6;
		std::array<kobjref<CKObject>, 11> ckagmGlob7;
		kobjref<CKObject> ckagmGlob8;
		std::array<int32_t, 18> ckagmGlob9;
		int32_t ckagmGlob10;
		kobjref<CKObject> ckagmGlob11;
		int32_t ckagmGlob12;
		int32_t ckagmGlob13;
		int32_t ckagmGlob14;
		int32_t ckagmGlob15;
		int32_t ckagmGlob16;
		int32_t ckagmGlob17;
		int32_t ckagmGlob18;
		std::array<int32_t, 7> ckagmGlob19;
		kobjref<CKObject> ckagmGlob20;
		std::array<std::array<int32_t, 3>, 3> ckagmGlob21;
		std::array<int32_t, 11> ckagmGlob22; // not in HD

		std::array<int32_t, 6> ckagmGlob30;
		std::array<int32_t, 14> ckagmGlob31;
		int32_t ckagmGlob32; // not in HD
		int32_t ckagmGlob33; // not in HD
		int32_t ckagmGlob34; // not in HD
		int32_t ckagmGlob35; // not in HD

		void reflectMembers2(MemberListener& r, KEnvironment* kenv);
		void reflectGlobal(MemberListener& r, KEnvironment* kenv);
		void deserializeGlobal(KEnvironment* kenv, File* file, size_t length);
		void serializeGlobal(KEnvironment* kenv, File* file);
	};

	struct CKA2DiamondHelmet : CKSubclass<CKReflectableGameDef, 232> {
		// game
		uint32_t dhUnk1; kobjref<CKObject> dhStage;
		// level
		uint32_t dhBeaconRef; EventNode dhEventNode;

		void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
		void reflectLevel(MemberListener& ml, KEnvironment* kenv) override;
		void resetLvlSpecific(KEnvironment* kenv) override;
	};

	struct CKA2GameModule : CKSubclass<CKGameModule, 228> {
		// game
		uint8_t gmA2Unk;
		void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
	};

	struct CKA2SpawnPoint : CKSubclass<CKGameSpawnPoint, 199> {
		// level
		uint32_t gspAsterixBeacon, gspObelixBeacon, gspIdefixBeacon;
		std::vector<std::pair<uint8_t, uint8_t>> gspA2Unk1;
		kobjref<CKObject> gspWeatherPreset;
		void reflectLevel(MemberListener& ml, KEnvironment* kenv) override;
		void resetLvlSpecific(KEnvironment* kenv) override;
	};

	struct CKA2GameStage : CKSubclass<CKGameStage, 223> {
		// game
		std::vector<kobjref<CKObject>> gsgA2ObjList1;
		std::vector<kobjref<CKObject>> gsgA2ObjList2;
		std::vector<kobjref<CKObject>> gsgA2ObjList3;
		uint32_t gsgA2UserLevelNumber = 0x1D;
		// level
		std::vector<kobjref<CKObject>> gsgA2ObjList4;
		std::vector<kobjref<CKObject>> gsgA2ObjList5;
		std::vector<kobjref<CKObject>> gsgA2ObjList6;
		void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
		void reflectLevel(MemberListener& ml, KEnvironment* kenv) override;
		void resetLvlSpecific(KEnvironment* kenv) override;
	};

	struct CKA2ShoppingArea : CKSubclass<CKReflectableGameDef, 38> {
		// game
		kobjref<CKObject> shopStage;
		void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
	};

	struct CKA2Video : CKSubclass<CKReflectableGameDef, 231> {
		// game
		kobjref<CKObject> gfxVideo;
		kobjref<CKObject> stage;
		int vidUnkInt;
		void reflectGame(MemberListener& ml, KEnvironment* kenv) override;
	};

	struct CKA2Player : CKSubclass<CKDefaultPlayer, 166> {};
	struct CKA2GameStructure : CKSubclass<CKGameStructure, 221> {};

}