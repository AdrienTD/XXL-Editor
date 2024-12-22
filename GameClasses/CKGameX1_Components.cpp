#include "CKGameX1_Components.h"
#include "CoreClasses/CKDictionary.h"

// =================== Component classes ===================

namespace GameX1 {

#define RREFLECT(r, var) r.reflect(var, #var)

	void CKSeizableEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKSquadSeizableEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
		CKSeizableEnemyCpnt::reflectMembers2(r, kenv);
		RREFLECT(r, sqseUnk1);
	}

	void CKBasicEnemyCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKRocketRomanCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKRomanArcherCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKTurtleCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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
		RREFLECT(r, ttNumSpearSides);
		RREFLECT(r, ttUnk12);
		RREFLECT(r, ttUnk13);
		RREFLECT(r, ttUnk14);
	}

	void CKJumpingRomanCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKMobileTowerCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKJetPackRomanCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv)
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

	void CKGrpMecaCpntAsterix::reflectMembers2(MemberListener& r, KEnvironment* kenv)
	{
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

	void CKWildBoarCpnt::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
		r.reflect(ckwbcUnk0, "ckwbcUnk0");
		r.reflect(ckwbcUnk1, "ckwbcUnk1");
		r.reflect(ckwbcUnk2, "ckwbcUnk2");
		r.reflect(ckwbcUnk3, "ckwbcUnk3");
		r.reflect(ckwbcUnk4, "ckwbcUnk4");
		r.reflect(ckwbcUnk5, "ckwbcUnk5");
	};

}