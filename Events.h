#pragma once

#include <cstdint>

struct KEnvironment;
struct File;
struct CKObject;

struct EventNode {
	uint8_t bit;
	int16_t seqIndex;

	void write(KEnvironment *kenv, File *file) const;
	void read(KEnvironment *kenv, File *file, CKObject *user);
};
