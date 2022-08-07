#pragma once

#include <cstdint>
#include <vector>
#include "KObject.h"

struct KEnvironment;
struct File;
struct CKObject;
struct CKComparedData;

struct EventNodeX1 {
	uint8_t bit = 0;
	int16_t seqIndex = -1;

	void write(KEnvironment *kenv, File *file) const;
	void read(KEnvironment *kenv, File *file, CKObject *user);
};

struct EventNodeX2 {
	std::vector<kobjref<CKComparedData>> datas;
	void write(KEnvironment* kenv, File* file) const;
	void read(KEnvironment* kenv, File* file, CKObject* user);
};

struct EventNode {
	EventNodeX1 enx1;
	EventNodeX2 enx2;
	void write(KEnvironment* kenv, File* file) const;
	void read(KEnvironment* kenv, File* file, CKObject* user);
};

struct MarkerIndex {
	int32_t index = -1;
	int32_t arSecondIndex = -1;
	void write(KEnvironment* kenv, File* file) const;
	void read(KEnvironment* kenv, File* file);
};