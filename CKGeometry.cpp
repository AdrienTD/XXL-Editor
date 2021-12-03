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

		if (kenv->isRemaster) {
			hdKifPath = file->readString(file->readUint16());
			hdMatName = file->readString(file->readUint16());
			hdUnk1 = file->readUint32();
		}

		if (!kenv->isRemaster) { // TODO: Look for presence in PS2 version
			kobjref<CKAnyGeometry> d_sameGeo = kenv->readObjRef<CKAnyGeometry>(file);
			assert(d_sameGeo.get() == this);
			this->flags2 = file->readUint32();
			// cases 7 and 8 seem to be never used...
			switch ((flags2 >> 3) & 15) {
			case 7:
				unkloner = file->readUint32();
				break;
			case 8:
				unkarea[0] = file->readUint32();
				unkarea[1] = file->readUint32();
				unkstring = file->readSizedString<uint16_t>();
				break;
			}
		}
	}
	else {
		if (kenv->version >= kenv->KVERSION_SPYRO)
			this->spUnk1 = file->readUint32();
		else
			this->unkobj1 = kenv->readObjRef<CKObject>(file);
		this->lightSet = kenv->readObjRef<CKObject>(file);
		this->flags = file->readUint32();
		this->nextGeo = kenv->readObjRef<CKAnyGeometry>(file);
		this->material = kenv->readObjRef<CKObject>(file);
		if(kenv->version >= kenv->KVERSION_ARTHUR)
			this->ogUnkObj = kenv->readObjRef<CKObject>(file);
		this->color = file->readUint32();
		if (kenv->isRemaster) {
			hdKifPath = file->readString(file->readUint16());
			hdMatName = file->readString(file->readUint16());
			hdUnk1 = file->readUint32();
			std::string cdcd = file->readString(64);
			//assert(cdcd == std::string(64, '\xCD'));
		}
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
		if (kenv->version >= kenv->KVERSION_SPYRO) {
			spLastByte2 = file->readUint8();
			spLastByte3 = file->readUint8();
			spLastByte4 = file->readUint8();
		}
		else if (kenv->version == kenv->KVERSION_OLYMPIC && kenv->platform == kenv->PLATFORM_X360)
			spLastByte2 = file->readUint8();
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

		if (kenv->isRemaster) {
			file->writeUint16(hdKifPath.size());
			file->writeString(hdKifPath);
			file->writeUint16(hdMatName.size());
			file->writeString(hdMatName);
			file->writeUint32(hdUnk1);
		}

		if (!kenv->isRemaster) {
			//kenv->writeObjRef(file, sameGeo);
			kenv->writeObjID(file, this);
			file->writeUint32(flags2);
			switch ((flags2 >> 3) & 15) {
			case 7:
				file->writeUint32(unkloner);
				break;
			case 8:
				file->writeUint32(unkarea[0]);
				file->writeUint32(unkarea[1]);
				file->writeSizedString<uint16_t>(unkstring);
				break;
			}
		}
	}
	else {
		if (kenv->version >= kenv->KVERSION_SPYRO)
			file->writeUint32(this->spUnk1);
		else
			kenv->writeObjRef(file, this->unkobj1);
		kenv->writeObjRef(file, this->lightSet);
		file->writeUint32(this->flags);
		kenv->writeObjRef(file, this->nextGeo);
		kenv->writeObjRef(file, this->material);
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			kenv->writeObjRef(file, this->ogUnkObj);
		file->writeUint32(this->color);
		if (kenv->isRemaster) {
			file->writeUint16(hdKifPath.size());
			file->writeString(hdKifPath);
			file->writeUint16(hdMatName.size());
			file->writeString(hdMatName);
			file->writeUint32(hdUnk1);
			file->writeUint32(0xCDCDCDCD); file->writeUint32(0xCDCDCDCD); file->writeUint32(0xCDCDCDCD); file->writeUint32(0xCDCDCDCC);
			for(int i = 0; i < 12; i++)
				file->writeUint32(0xCDCDCDCD);
		}
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
		if (kenv->version >= kenv->KVERSION_SPYRO) {
			file->writeUint8(spLastByte2);
			file->writeUint8(spLastByte3);
			file->writeUint8(spLastByte4);
		}
		else if (kenv->version == kenv->KVERSION_OLYMPIC && kenv->platform == kenv->PLATFORM_X360)
			file->writeUint8(spLastByte2);
	}
}

void CKParticleGeometry::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version == kenv->KVERSION_XXL2 && kenv->isRemaster)
		return;
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
	if (kenv->version == kenv->KVERSION_XXL2 && kenv->isRemaster)
		return;
	CKAnyGeometry::serialize(kenv, file);
	if (kenv->version < kenv->KVERSION_XXL2) {
		if (flags & 0x80) {
			file->writeUint32(pgHead1);
			file->writeUint32(pgHead2);
			file->writeUint32(pgHead3);
			for (float &f : pgSphere)
				file->writeFloat(f);
			if (flags & 0x1000) {
				//assert(pgHead2 == pgHead3);
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
