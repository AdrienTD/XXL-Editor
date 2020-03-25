#include "CKService.h"
#include "File.h"
#include "KEnvironment.h"

void CKSrvEvent::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numA = file->readUint16();
	numB = file->readUint16();
	numC = file->readUint16();
	numObjs = file->readUint16();
	bees.resize(numA + numB + numC);
	for (StructB &b : bees) {
		b._1 = file->readUint8();
		b._2 = file->readUint8();
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
