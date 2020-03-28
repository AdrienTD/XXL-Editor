#include "CKLogic.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKNode.h"
#include <cassert>

void CGround::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	this->numa = file->readUint32();
	uint16_t numTris = file->readUint16();
	uint16_t numVerts = file->readUint16();
	this->triangles.resize(numTris);
	this->vertices.resize(numVerts);
	for (Triangle &tri : this->triangles) {
		for (auto &coord : tri.indices)
			coord = file->readUint16();
	}
	for (Vector3 &vert : this->vertices) {
		vert.x = file->readFloat();
		vert.y = file->readFloat();
		vert.z = file->readFloat();
	}
	for (float &c : this->aabb)
		c = file->readFloat();
	param1 = file->readUint16();
	param2 = file->readUint16();
	uint16_t numInfWalls = file->readUint16();
	infiniteWalls.resize(numInfWalls);
	for (InfiniteWall &infwall : infiniteWalls) {
		for (auto &ix : infwall.baseIndices)
			ix = file->readUint16();
	}
	uint16_t numFinWalls = file->readUint16();
	finiteWalls.resize(numFinWalls);
	for (FiniteWall &wall : finiteWalls) {
		for (auto &ix : wall.baseIndices)
			ix = file->readUint16();
		for (auto &h : wall.heights)
			h = file->readFloat();
	}
	this->param3 = file->readFloat();
	this->param4 = file->readFloat();
}

void CGround::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(this->numa);
	file->writeUint16(this->triangles.size());
	file->writeUint16(this->vertices.size());
	for (Triangle &tri : this->triangles) {
		for (auto &coord : tri.indices)
			file->writeUint16(coord);
	}
	for (Vector3 &vert : this->vertices) {
		file->writeFloat(vert.x);
		file->writeFloat(vert.y);
		file->writeFloat(vert.z);
	}
	for (float &c : this->aabb)
		file->writeFloat(c);
	file->writeUint16(param1);
	file->writeUint16(param2);
	file->writeUint16(infiniteWalls.size());
	for (InfiniteWall &infwall : infiniteWalls) {
		for (auto &ix : infwall.baseIndices)
			file->writeUint16(ix);
	}
	file->writeUint16(finiteWalls.size());
	for (FiniteWall &wall : finiteWalls) {
		for (auto &ix : wall.baseIndices)
			file->writeUint16(ix);
		for (auto &h : wall.heights)
			file->writeFloat(h);
	}
	file->writeFloat(this->param3);
	file->writeFloat(this->param4);
}

void CKMeshKluster::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	for (float &f : aabb)
		f = file->readFloat();
	uint16_t numGrounds = file->readUint16();
	uint16_t numWalls = file->readUint16();
	unk2 = file->readUint16();
	assert(unk2 == 0);
	grounds.reserve(numGrounds);
	for (uint16_t i = 0; i < numGrounds; i++) {
		grounds.push_back(kenv->readObjRef<CGround>(file));
	}
	walls.reserve(numWalls);
	for (uint16_t i = 0; i < numWalls; i++) {
		walls.push_back(kenv->readObjRef<CKObject>(file));
	}
	unkRef = kenv->readObjRef<CKObject>(file);
}

void CKMeshKluster::serialize(KEnvironment * kenv, File * file)
{
	for (float &f : aabb)
		file->writeFloat(f);
	file->writeUint16(grounds.size());
	file->writeUint16(walls.size());
	file->writeUint16(unk2);
	for (auto &ref : grounds) {
		kenv->writeObjRef(file, ref);
	}
	for (auto &ref : walls) {
		kenv->writeObjRef(file, ref);
	}
	kenv->writeObjRef(file, unkRef);
}

void CKSector::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	//sgRoot = kenv->readObjRef<CKObject>(file);
	file->seek(4, SEEK_CUR);
	strId = file->readUint16();
	unk1 = file->readUint16();
	uint32_t numSases = file->readUint32();
	for (uint32_t i = 0; i < numSases; i++)
		sases.push_back(kenv->readObjRef<CKObject>(file));
	//soundDictionary = kenv->readObjRef<CKObject>(file);
	//beaconKluster = kenv->readObjRef<CKObject>(file);
	//meshKluster = kenv->readObjRef<CKObject>(file);
	file->seek(12, SEEK_CUR);
	for (float &f : boundaries)
		f = file->readFloat();
	unk2 = file->readUint32();
}

void CKSector::serialize(KEnvironment * kenv, File * file)
{
	KObjectList &objlist = (strId == 0) ? kenv->levelObjects : kenv->sectorObjects[strId-1];
	CKObject *fndSGRoot, *fndSoundDictionary, *fndBeaconKluster = nullptr, *fndMeshKluster;
	fndSGRoot = objlist.getClassType<CSGSectorRoot>().objects[0];
	fndSoundDictionary = objlist.getClassType(9, 3).objects[0];
	if(!objlist.getClassType(12, 73).objects.empty())
		fndBeaconKluster = objlist.getClassType(12, 73).objects[0];
	fndMeshKluster = objlist.getClassType<CKMeshKluster>().objects[0];

	kenv->writeObjID(file, fndSGRoot);
	file->writeUint16(strId);
	file->writeUint16(unk1);
	file->writeUint32(sases.size());
	for (auto &sas : sases)
		kenv->writeObjRef(file, sas);
	kenv->writeObjID(file, fndSoundDictionary);
	kenv->writeObjID(file, fndBeaconKluster);
	kenv->writeObjID(file, fndMeshKluster);
	for (float &f : boundaries)
		file->writeFloat(f);
	file->writeUint32(unk2);
}
