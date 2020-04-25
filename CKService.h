#pragma once

#include "KObject.h"
#include <vector>
#include <array>

struct CKBeaconKluster;

struct CKService : CKCategory<1> {};

struct CKSrvEvent : CKSubclass<CKService, 5>
{
	struct StructB {
		uint8_t _1, _2;
	};
	uint16_t numA, numB, numC, numObjs;
	std::vector<StructB> bees;
	std::vector<kobjref<CKObject>> objs;
	std::vector<uint16_t> objInfos;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
};

struct CKSrvBeacon : CKSubclass<CKService, 11> {
	uint8_t unk1;
	uint32_t numHandlers;
	struct Handler {
		uint8_t unk2a, numBits, handlerIndex, handlerId, persistent;
		kobjref<CKObject> object;
	};
	std::vector<Handler> handlers;
	uint32_t numSectors;
	struct BeaconSector {
		uint32_t numUsedBings, numBings, beaconArraySize, numBits;
		std::vector<bool> bits;
		uint8_t numBeaconKlusters;
		std::vector<uint32_t> bkids;
		std::vector<kobjref<CKBeaconKluster>> beaconKlusters;
	};
	std::vector<BeaconSector> beaconSectors;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	void onLevelLoaded(KEnvironment *kenv) override;
};