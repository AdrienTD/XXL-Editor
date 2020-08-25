#include "CKGeometry.h"
#include "KEnvironment.h"
#include "File.h"
#include "rw.h"
#include <cassert>

CKAnyGeometry::~CKAnyGeometry()
{
	if (clump)
		delete clump;
	for (RwMiniClump* cost : costumes)
		if(cost != clump)
			delete cost;
}

void CKAnyGeometry::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
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

		kobjref<CKAnyGeometry> d_sameGeo = kenv->readObjRef<CKAnyGeometry>(file);
		assert(d_sameGeo.get() == this);
		this->flags2 = file->readUint32();
		// cases 7 and 8 seem to be never used...
		switch ((flags2 >> 3) & 15) {
		case 7:
			unkloner = file->readUint32();
			break;
		case 8:
			for (uint32_t &v : this->unkarea)
				v = file->readUint32();
			break;
		}
	}
	else {
		this->unkobj1 = kenv->readObjRef<CKObject>(file);
		this->lightSet = kenv->readObjRef<CKObject>(file);
		this->flags = file->readUint32();
		this->nextGeo = kenv->readObjRef<CKAnyGeometry>(file);
		this->material = kenv->readObjRef<CKObject>(file);
		if(kenv->version >= kenv->KVERSION_ARTHUR)
			this->ogUnkObj = kenv->readObjRef<CKObject>(file);
		this->color = file->readUint32();
		uint32_t hasGeoFlag = (kenv->version == kenv->KVERSION_XXL2) ? 0x2000 : 0x4000; // check arthur
		if (!(flags & hasGeoFlag)) {
			uint8_t isUniqueGeo = file->readUint8();
			if (!isUniqueGeo) {
				this->duplicateGeo = kenv->readObjRef<CKAnyGeometry>(file);
			}
			else {
				clump = new RwMiniClump;
				clump->deserialize(file);
			}
		}
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			ogLastByte = file->readUint8();
	}
}

void CKAnyGeometry::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version < kenv->KVERSION_XXL2) {
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
		switch ((flags2 >> 3) & 15) {
		case 7:
			file->writeUint32(unkloner);
			break;
		case 8:
			for (uint32_t &v : unkarea)
				file->writeUint32(v);
			break;
		}
	}
	else {
		kenv->writeObjRef(file, this->unkobj1);
		kenv->writeObjRef(file, this->lightSet);
		file->writeUint32(this->flags);
		kenv->writeObjRef(file, this->nextGeo);
		kenv->writeObjRef(file, this->material);
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			kenv->writeObjRef(file, this->ogUnkObj);
		file->writeUint32(this->color);
		uint32_t hasGeoFlag = (kenv->version == kenv->KVERSION_XXL2) ? 0x2000 : 0x4000; // check arthur
		if (!(flags & hasGeoFlag)) {
			if (this->duplicateGeo) {
				file->writeUint8(0);
				kenv->writeObjRef(file, this->duplicateGeo);
			}
			else {
				file->writeUint8(1);
				clump->serialize(file);
			}
		}
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			file->writeUint8(ogLastByte);
	}
}

void CKParticleGeometry::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	size_t startoff = file->tell();
	CKAnyGeometry::deserialize(kenv, file, length);
	if (kenv->version < kenv->KVERSION_XXL2) {
		if (flags & 0x80) {
			pgHead1 = file->readUint32();
			pgHead2 = file->readUint32();
			pgHead3 = file->readUint32();
			for (float &f : pgSphere)
				f = file->readFloat();
			if (flags & 0x1000) {
				//assert(pgHead2 == pgHead3);
				pgPoints.resize(pgHead2);
				for (Vector3 &vec : pgPoints)
					for (float &f : vec)
						f = file->readFloat();
			}
			extraSize = length - (file->tell() - startoff);
			extra = malloc(extraSize);
			file->read(extra, extraSize);
		}
	} else {
		extraSize = length - (file->tell() - startoff);
		extra = malloc(extraSize);
		file->read(extra, extraSize);
	}
}

void CKParticleGeometry::serialize(KEnvironment * kenv, File * file)
{
	CKAnyGeometry::serialize(kenv, file);
	if (kenv->version < kenv->KVERSION_XXL2) {
		if (flags & 0x80) {
			file->writeUint32(pgHead1);
			file->writeUint32(pgHead2);
			file->writeUint32(pgHead3);
			for (float &f : pgSphere)
				file->writeFloat(f);
			if (flags & 0x1000) {
				assert(pgHead2 == pgHead3);
				for (Vector3 &vec : pgPoints)
					for (float &f : vec)
						file->writeFloat(f);
			}
			file->write(extra, extraSize);
		}
	} else {
		file->write(extra, extraSize);
	}
}
