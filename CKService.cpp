#include "CKService.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKLogic.h"
#include "CKNode.h"

void CKSrvEvent::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numA = file->readUint16();
	numB = file->readUint16();
	numC = file->readUint16();
	numObjs = file->readUint16();
	bees.resize(numA + numB + numC);
	int ev = 0;
	for (StructB &b : bees) {
		b._1 = file->readUint8();
		b._2 = file->readUint8();
		printf("%i %i %i\n", b._1, b._2, ev);
		ev += b._1;
	}
	objs.reserve(numObjs);
	for (size_t i = 0; i < numObjs; i++)
		objs.push_back(kenv->readObjRef<CKObject>(file));
	objInfos.reserve(numObjs);
	for (size_t i = 0; i < numObjs; i++)
		objInfos.push_back(file->readUint16());
}

void CKSrvEvent::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(numA);
	file->writeUint16(numB);
	file->writeUint16(numC);
	numObjs = objs.size();
	file->writeUint16(numObjs);
	for (StructB &b : bees) {
		file->writeUint8(b._1);
		file->writeUint8(b._2);
	}
	for (auto &obj : objs)
		kenv->writeObjRef(file, obj);
	for (uint16_t &arg : objInfos)
		file->writeUint16(arg);
}

void CKSrvBeacon::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	struct BitReader {
		File *file;
		uint8_t byte, bit;
		BitReader(File *file) : file(file), bit(8) {}
		bool readBit() {
			if (bit >= 8) {
				byte = file->readUint8();
				bit = 0;
			}
			bool v = byte & (1 << bit);
			bit++;
			return v;
		}
	};
	unk1 = file->readUint8();
	numHandlers = file->readUint32();
	handlers.resize(numHandlers);
	for (Handler &handler : handlers) {
		handler.unk2a = file->readUint8();
		handler.numBits = file->readUint8();
		handler.handlerIndex = file->readUint8();
		handler.handlerId = file->readUint8();
		handler.persistent = file->readUint8();
		handler.object = kenv->readObjRef<CKObject>(file);
	}
	numSectors = file->readUint32();
	beaconSectors.resize(numSectors);
	for (auto it = beaconSectors.rbegin(); it != beaconSectors.rend(); it++) {
		BeaconSector &bs = *it;
		bs.numUsedBings = file->readUint32();
		bs.numBings = file->readUint32();
		bs.beaconArraySize = file->readUint32();
		bs.numBits = file->readUint32();
		BitReader bread(file);
		for(uint32_t i = 0; i < bs.numBits; i++)
			bs.bits.push_back(bread.readBit());
		bs.numBeaconKlusters = file->readUint8();
		for (uint8_t i = 0; i < bs.numBeaconKlusters; i++)
			bs.bkids.push_back(file->readUint32());
		//	bs.beaconKlusters.push_back(kenv->readObjRef<CKBeaconKluster>(file));
	}
}

void CKSrvBeacon::serialize(KEnvironment * kenv, File * file)
{
	struct BitWriter {
		File *file;
		uint8_t byte, bit;
		BitWriter(File *file) : file(file), byte(0), bit(0) {}
		void writeBit(bool v) {
			if (bit >= 8) {
				file->writeUint8(byte);
				byte = 0;
				bit = 0;
			}
			if(v)
				byte |= (1 << bit);
			bit++;
		}
		void end() {
			if(bit)
				file->writeUint8(byte);
		}
	};
	file->writeUint8(unk1);
	file->writeUint32(handlers.size());
	for (Handler &handler : handlers) {
		file->writeUint8(handler.unk2a);
		file->writeUint8(handler.numBits);
		file->writeUint8(handler.handlerIndex);
		file->writeUint8(handler.handlerId);
		file->writeUint8(handler.persistent);
		kenv->writeObjRef(file, handler.object);
	}
	file->writeUint32(beaconSectors.size());
	for (auto it = beaconSectors.rbegin(); it != beaconSectors.rend(); it++) {
		BeaconSector &bs = *it;
		file->writeUint32(bs.numUsedBings);
		file->writeUint32(bs.numBings);
		file->writeUint32(bs.beaconArraySize);
		file->writeUint32(bs.bits.size());
		BitWriter bwrite(file);
		for (uint32_t i = 0; i < bs.bits.size(); i++)
			bwrite.writeBit(bs.bits[i]);
		bwrite.end();
		file->writeUint8(bs.beaconKlusters.size());
		for (auto &ref : bs.beaconKlusters)
			kenv->writeObjRef(file, ref);
	}
}

void CKSrvBeacon::onLevelLoaded(KEnvironment * kenv)
{
	int str = -1;
	for (BeaconSector &bs : beaconSectors) {
		for (uint32_t id : bs.bkids)
			bs.beaconKlusters.push_back(kenv->getObjRef<CKBeaconKluster>(id, str));
		str++;
	}
}

void CKSrvCollision::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numWhat = file->readUint16();
	huh = file->readUint8();
	for (auto &ref : dynBSphereProjectiles)
		ref = kenv->readObjRef<CKSceneNode>(file);
	objs.resize(numWhat);
	for (auto &vec : objs) {
		vec.resize(file->readUint8());
		for (auto &ref : vec)
			ref = kenv->readObjRef<CKObject>(file);
	}
	unk1 = file->readUint16();
	unk2 = file->readUint16();
	objs2.resize(unk1);
	for (auto &ref : objs2) {
		ref = kenv->readObjRef<CKObject>(file);
	}
	bings.resize(unk2);
	for (Bing &bing : bings) {
		bing.v1 = file->readUint16();
		bing.obj1 = kenv->readObjRef<CKObject>(file);
		bing.obj2 = kenv->readObjRef<CKObject>(file);
		bing.b1 = file->readUint16();
		bing.b2 = file->readUint16();
		bing.v2 = file->readUint8();
		for (uint8_t &u : bing.aa)
			u = file->readUint8();
	}
	lastnum = file->readUint32();
}

void CKSrvCollision::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(objs.size());
	file->writeUint8(huh);
	for (auto &ref : dynBSphereProjectiles)
		kenv->writeObjRef(file, ref);
	for (auto &vec : objs) {
		file->writeUint8(vec.size());
		for (auto &ref : vec)
			kenv->writeObjRef<CKObject>(file, ref);
	}
	file->writeUint16(unk1);
	file->writeUint16(unk2);
	for (auto &ref : objs2)
		kenv->writeObjRef(file, ref);
	for (Bing &bing : bings) {
		file->writeUint16(bing.v1);
		kenv->writeObjRef(file, bing.obj1);
		kenv->writeObjRef(file, bing.obj2);
		file->writeUint16(bing.b1);
		file->writeUint16(bing.b2);
		file->writeUint8(bing.v2);
		for (uint8_t &u : bing.aa)
			file->writeUint8(u);
	}
	file->writeUint32(lastnum);
}
