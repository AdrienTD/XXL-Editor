#pragma once

#include "KObject.h"
#include "File.h"
#include <vector>

template<class T, int T_ID> struct CKPartlyUnknown : CKSubclass<T, T_ID> {
	std::vector<uint8_t> unkPart;
	void deserialize(KEnvironment* kenv, File *file, size_t length) override {
		size_t start = file->tell();
		T::deserialize(kenv, file, length);
		size_t unkPartSize = length - (file->tell() - start);
		unkPart.resize(unkPartSize);
		file->read(unkPart.data(), unkPartSize);
	}
	void serialize(KEnvironment *kenv, File *file) override {
		T::serialize(kenv, file);
		file->write(unkPart.data(), unkPart.size());
	}
};
