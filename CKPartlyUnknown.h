#pragma once

#include "KObject.h"
#include "File.h"

template<class T, int T_ID> struct CKPartlyUnknown : CKSubclass<T, T_ID> {
	void *unkPart = nullptr; size_t unkPartSize = 0;
	void deserialize(KEnvironment* kenv, File *file, size_t length) override {
		size_t start = file->tell();
		T::deserialize(kenv, file, length);
		unkPartSize = length - (file->tell() - start);
		unkPart = malloc(unkPartSize);
		file->read(unkPart, unkPartSize);
	}
	void serialize(KEnvironment *kenv, File *file) override {
		T::serialize(kenv, file);
		file->write(unkPart, unkPartSize);
	}
	~CKPartlyUnknown()
	{
		if (unkPart)
			free(unkPart);
	}
};
