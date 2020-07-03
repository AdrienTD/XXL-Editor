#pragma once

#include "KObject.h"
#include <array>
#include "vecmat.h"
#include "CKUtils.h"

struct CKSceneNode;

struct CKComponent : CKCategory<6> {

};

struct CKCrateCpnt : CKSubclass<CKComponent, 5> {
	kobjref<CKObject> group;
	kobjref<CKSceneNode> particleNode;
	kobjref<CKObject> soundIds;
	kobjref<CKObject> projectiles;
	kobjref<CKSceneNode> crateNode;
	std::array<float, 6> unk1;
	uint8_t unk7;
	std::array<float, 56 * 2> pack1;
	std::array<float, 56> pack2;
	std::array<int, 4> bonuses;
	uint16_t unk8;
	uint8_t unk9;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKEnemyCpnt : CKMRSubclass<CKEnemyCpnt, CKMemberReflectable<CKComponent>, 6> {
	// Some names proposed by Spork
	uint32_t flags;
	uint8_t health, damage;
	float unk1;
	float walkAnimSpeed;
	float coverAnimSpeed;
	float speed1;
	float runSpeed1;
	float speed2;
	float speed3;
	float runSpeed2;
	float speed4;
	float speed5;
	float unkf1;
	float unkf2;
	float unkf3;
	float coverRange;
	float unkf5;
	float attackCooldown;
	float unkf7;
	float unkf8;
	float unkf9;
	float targeting;
	float unkLast;

	void reflectMembers(MemberListener &r);
};

struct CKSquadEnemyCpnt : CKMRSubclass<CKSquadEnemyCpnt, CKEnemyCpnt, 7> {

};

struct CKSeizableEnemyCpnt : CKMRSubclass<CKSeizableEnemyCpnt, CKEnemyCpnt, 8> {
	float seUnk1;
	uint8_t seUnk2;
	Vector3 seUnk3;
	float seUnk4;
	uint8_t seUnk5;
	Vector3 seUnk6;
	float seUnk7;
	uint8_t seUnk8;
	Vector3 seUnk9;
	float seUnk10;
	float seUnk11;
	uint8_t seUnk12;
	Vector3 seUnk13;
	float seUnk14;
	float seUnk15;
	float seUnk16;
	float seUnk17;
	float seUnk18;
	float seUnk19;
	float seUnk20;
	float seUnk21;
	float seUnk22;
	float seUnk23;
	float seUnk24;
	float seUnk25;
	float seUnk26;
	uint8_t seUnk27;
	uint8_t seUnk28;
	float seUnk29;
	float seUnk30;
	float seUnk31;

	void reflectMembers(MemberListener &r);
};

struct CKSquadSeizableEnemyCpnt : CKMRSubclass<CKSquadSeizableEnemyCpnt, CKSeizableEnemyCpnt, 9> {
	float sqseUnk1;

	void reflectMembers(MemberListener &r);
};

struct CKBasicEnemyCpnt : CKMRSubclass<CKBasicEnemyCpnt, CKSquadSeizableEnemyCpnt, 10> {
	Vector3 beUnk1;
	uint8_t beUnk2;
	Vector3 beUnk3;
	float beUnk4;
	float beUnk5;
	float beUnk6;
	float beUnk7;
	float beUnk8;
	float beUnk9;
	float beUnk10;
	float beUnk11;
	float beUnk12;
	float beUnk13;
	void reflectMembers(MemberListener &r);
};

struct CKBasicEnemyLeaderCpnt : CKMRSubclass<CKBasicEnemyLeaderCpnt, CKBasicEnemyCpnt, 11> {

};

struct CKJumpingRomanCpnt : CKMRSubclass<CKJumpingRomanCpnt, CKSquadSeizableEnemyCpnt, 12> {
	float jrUnk1;
	float jrUnk2;
	Vector3 jrUnk3;
	float jrUnk4;
	float jrUnk5;
	float jrUnk6;
	float jrUnk7;
	float jrUnk8;
	float jrUnk9;
	float jrUnk10;
	float jrUnk11;
	float jrUnk12;
	float jrUnk13;
	float jrUnk14;
	float jrUnk15;
	float jrUnk16;
	float jrUnk17;
	float jrUnk18;
	float jrUnk19;
	void reflectMembers(MemberListener &r);
};

struct CKRomanArcherCpnt : CKMRSubclass<CKRomanArcherCpnt, CKSquadSeizableEnemyCpnt, 17> {
	float raUnk1;
	float raUnk2;
	float raUnk3;
	float raUnk4;
	float raUnk5;
	float raUnk6;
	float raUnk7;
	uint8_t raNumArrows;
	float raUnk9;
	void reflectMembers(MemberListener &r);
};

struct CKRocketRomanCpnt : CKMRSubclass<CKRocketRomanCpnt, CKBasicEnemyCpnt, 19> {
	float rrCylinderRadius = 0, rrCylinderHeight = 0;
	Vector3 rrUnk3 = Vector3(0,0,0);
	uint8_t rrUnk4 = 0;
	float rrFireDistance = 0;
	uint8_t rrUnk6 = 0;
	float rrFlySpeed = 0, rrRomanAimFactor = 0;
	kobjref<CKObject> rrUnk9;

	void reflectMembers(MemberListener &r);
};

struct CKJetPackRomanCpnt : CKMRSubclass<CKJetPackRomanCpnt, CKSquadEnemyCpnt, 23> {
	float jpUnk1;
	float jpUnk2;
	float jpUnk3;
	float jpUnk4;
	float jpUnk5;
	float jpUnk6;
	float jpUnk7;
	float jpUnk8;
	float jpUnk9;
	float jpUnk10;
	float jpUnk11;
	uint8_t jpUnk12;
	uint8_t jpUnk13;
	float jpUnk14;
	uint8_t jpUnk15;
	float jpUnk16;
	float jpUnk17;
	float jpUnk18;
	float jpUnk19;
	float jpUnk20;
	float jpUnk21;
	float jpUnk22;
	float jpUnk23;
	void reflectMembers(MemberListener &r);
};

struct CKMobileTowerCpnt : CKMRSubclass<CKMobileTowerCpnt, CKSquadEnemyCpnt, 26> {
	uint8_t mtUnk1;
	float mtUnk2;
	float mtUnk3;
	float mtUnk4;
	float mtUnk5;
	float mtUnk6;
	float mtUnk7;
	float mtUnk8;
	float mtUnk9;
	float mtUnk10;
	float mtUnk11;
	float mtUnk12;
	float mtUnk13;
	float mtUnk14;
	float mtUnk15;
	uint8_t mtUnk16;
	float mtUnk17;
	uint8_t mtUnk18;
	uint8_t mtUnk19;
	uint8_t mtUnk20;
	uint8_t mtUnk21;
	uint8_t mtUnk22;
	float mtUnk23;
	uint8_t mtUnk24;
	uint8_t mtUnk25;
	uint8_t mtUnk26;
	uint8_t mtUnk27;
	void reflectMembers(MemberListener &r);
};

struct CKTurtleCpnt : CKMRSubclass<CKTurtleCpnt, CKSquadEnemyCpnt, 27> {
	uint8_t ttUnk1;
	float ttUnk2;
	uint8_t ttUnk3;
	float ttUnk4;
	uint8_t ttUnk5;
	uint8_t ttUnk6;
	float ttUnk7;
	float ttUnk8;
	//uint16_t ttUnk9;

	struct SpearState {
		float delay;
		uint16_t sides;
		void reflectMembers(MemberListener &r) {
			r.reflect(delay, "delay");
			r.reflect(sides, "sides");
		}
	};
	std::vector<SpearState> ttSpearStates;

	uint16_t ttUnk11;
	float ttUnk12;
	float ttUnk13;
	uint8_t ttUnk14;

	void reflectMembers(MemberListener &r);
};

struct CKTriangularTurtleCpnt : CKMRSubclass<CKTriangularTurtleCpnt, CKTurtleCpnt, 28> {};
struct CKSquareTurtleCpnt : CKMRSubclass<CKSquareTurtleCpnt, CKTurtleCpnt, 29> {};
struct CKDonutTurtleCpnt : CKMRSubclass<CKDonutTurtleCpnt, CKTurtleCpnt, 30> {};
struct CKPyramidalTurtleCpnt : CKMRSubclass<CKPyramidalTurtleCpnt, CKTurtleCpnt, 31> {};