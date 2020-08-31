#include "KLocalObject.h"
#include "File.h"
#include <cassert>

void KUnknownLocalObject::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	memsize = length;
	if (length != 0) {
		memptr = malloc(length);
		file->read(memptr, length);
	}
}

void KUnknownLocalObject::serialize(KEnvironment * kenv, File * file)
{
	if (memsize)
		file->write(memptr, memsize);
}

KLocalObject * KUnknownLocalObject::clone()
{
	KUnknownLocalObject *clone = new KUnknownLocalObject(this->cls_fid);
	clone->memptr = malloc(this->memsize);
	clone->memsize = this->memsize;
	memcpy(clone->memptr, this->memptr, this->memsize);
	return clone;
}

void KLocalPack::deserialize(KEnvironment * kenv, File * file)
{
	uint32_t numObjects = file->readUint32();
	objects.reserve(numObjects);
	for (uint32_t i = 0; i < numObjects; i++) {
		uint32_t clcat = file->readUint32();
		uint32_t clid = file->readUint32();
		uint32_t nextoff = file->readUint32();
		int cls_fid = clcat | (clid << 6);
		KLocalObject *locObj;
		auto factory = factories.find(cls_fid);
		if (factory == factories.end()) {
			locObj = new KUnknownLocalObject(cls_fid);
		} else {
			locObj = factory->second();
		}
		locObj->deserialize(kenv, file, nextoff - file->tell());
		assert(file->tell() == nextoff);
		objects.emplace_back(locObj);
	}
}

void KLocalPack::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(objects.size());
	for (const auto &locObj : objects) {
		int fid = locObj->getFullID();
		file->writeUint32(fid & 63);
		file->writeUint32(fid >> 6);
		file->writeUint32(0);
		size_t spos = file->tell();
		locObj->serialize(kenv, file);
		size_t npos = file->tell();
		file->seek(spos - 4, SEEK_SET);
		file->writeUint32(npos);
		file->seek(npos, SEEK_SET);
	}
}
