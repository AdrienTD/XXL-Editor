#include "CKGeometry.h"
#include "KEnvironment.h"
#include "File.h"
#include "rw.h"
#include <cassert>

void CKAnyGeometry::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	this->nextGeo = kenv->readObjRef<CKAnyGeometry>(file);
	this->flags = file->readUint32();
	if (!(flags & 0x80)) {
		uint32_t numCostumes = 1;
		if (flags & 0x2000)
			numCostumes = file->readUint32();
		costumes.reserve(numCostumes);
		for (uint32_t i = 0; i < numCostumes; i++) {
			this->clump = new RwMiniClump;
			this->clump->deserialize(file);
			costumes.push_back(this->clump);
		}
		clump = costumes[0];
	}
	else {
		this->clump = nullptr;
	}

	objref<CKAnyGeometry> d_sameGeo = kenv->readObjRef<CKAnyGeometry>(file);
	assert(d_sameGeo.get() == this);
	this->flags2 = file->readUint32();
	if (flags & 0x80)
		for (uint32_t &v : this->unkarea)
			v = file->readUint32();
}

void CKAnyGeometry::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, nextGeo);
	file->writeUint32(flags);
	if (!(flags & 0x80)) {
		if (flags & 0x2000) {
			file->writeUint32(this->costumes.size());
			for (RwMiniClump *cl : this->costumes)
				cl->serialize(file);
		}
		else
			this->clump->serialize(file);
	}
	//kenv->writeObjRef(file, sameGeo);
	kenv->writeObjID(file, this);
	file->writeUint32(flags2);
	if (flags & 0x80)
		for (uint32_t &v : unkarea)
			file->writeUint32(v);
}

void CKParticleGeometry::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKAnyGeometry::deserialize(kenv, file, length);
	if (flags & 0x80) {
		extraSize = length - 16 - 7*4;	// TODO: Correct length
		extra = malloc(extraSize);
		file->read(extra, extraSize);
	}
}

void CKParticleGeometry::serialize(KEnvironment * kenv, File * file)
{
	CKAnyGeometry::serialize(kenv, file);
	if (flags & 0x80)
		file->write(extra, extraSize);
}
