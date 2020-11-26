#pragma once

#include <cstdint>

struct KEnvironment;
struct File;
struct CKObject;

struct EventNode {
	uint8_t bit = 0;
	int16_t seqIndex = -1;

	void write(KEnvironment *kenv, File *file) const;
	void read(KEnvironment *kenv, File *file, CKObject *user);
};
