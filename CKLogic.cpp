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
	aabb.deserialize(file);
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
	assert(this->numa == (((6 * numTris + 12 * numVerts + 4 * numInfWalls + 12 * numFinWalls)+3)&~3));
}

void CGround::serialize(KEnvironment * kenv, File * file)
{
	//file->writeUint32(this->numa);
	file->writeUint32(((6 * triangles.size() + 12 * vertices.size() + 4 * infiniteWalls.size() + 12 * finiteWalls.size())+3)&~3);
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
	aabb.serialize(file);
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
	aabb.deserialize(file);
	uint16_t numGrounds = file->readUint16();
	uint16_t numWalls = file->readUint16();
	uint16_t unk2 = file->readUint16();
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
	aabb.serialize(file);
	file->writeUint16(grounds.size());
	file->writeUint16(walls.size());
	file->writeUint16(0);
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
	boundaries.deserialize(file);
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
	boundaries.serialize(file);
	file->writeUint32(unk2);
}

void CKBeaconKluster::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	nextKluster = kenv->readObjRef<CKBeaconKluster>(file);
	bounds.deserialize(file, true);
	uint16_t numBings = file->readUint16();
	numUsedBings = file->readUint16();
	bings.resize(numBings);
	for (Bing &bing : bings) {
		bing.active = file->readUint8() != 0;
		if (bing.active) {
			bing.numBeacons = file->readUint32();
			bing.unk2a = file->readUint8();
			bing.numBits = file->readUint8();
			bing.handlerId = file->readUint8();
			bing.sectorIndex = file->readUint8();
			bing.klusterIndex = file->readUint8();
			bing.handlerIndex = file->readUint8();
			bing.bitIndex = file->readUint16();
			if (bing.numBeacons != 0) {
				bing.handler = kenv->readObjRef<CKObject>(file);
				bing.unk6 = file->readUint32();
				bing.beacons.resize(bing.numBeacons);
				for (Beacon &beacon : bing.beacons) {
					beacon.posx = file->readUint16();
					beacon.posy = file->readUint16();
					beacon.posz = file->readUint16();
					beacon.params = file->readUint16();
				}
			}
		}
	}
}

void CKBeaconKluster::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, nextKluster);
	bounds.serialize(file, true);
	file->writeUint16(bings.size());
	file->writeUint16(numUsedBings);
	for (Bing &bing : bings) {
		file->writeUint8(bing.active ? 1 : 0);
		if (bing.active) {
			file->writeUint32(bing.beacons.size());
			file->writeUint8(bing.unk2a);
			file->writeUint8(bing.numBits);
			file->writeUint8(bing.handlerId);
			file->writeUint8(bing.sectorIndex);
			file->writeUint8(bing.klusterIndex);
			file->writeUint8(bing.handlerIndex);
			file->writeUint16(bing.bitIndex);
			if (bing.beacons.size() != 0) {
				kenv->writeObjRef(file, bing.handler);
				file->writeUint32(bing.unk6);
				for (Beacon &beacon : bing.beacons) {
					file->writeUint16(beacon.posx);
					file->writeUint16(beacon.posy);
					file->writeUint16(beacon.posz);
					file->writeUint16(beacon.params);
				}
			}
		}
	}
}

void CKSas::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	sector[0] = file->readUint32();
	sector[1] = file->readUint32();
	for (auto &box : boxes) {
		box.deserialize(file);
	}
}

void CKSas::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(sector[0]);
	file->writeUint32(sector[1]);
	for (auto &box : boxes) {
		box.serialize(file);
	}
}

void CDynamicGround::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CGround::deserialize(kenv, file, length);
	for (float &c : mpos)
		c = file->readFloat();
	for (float &c : mrot)
		c = file->readFloat();
	//node = kenv->readObjRef<CKSceneNode>(file);
	nodeId = file->readUint32();
	for (int i = 0; i < 16; i++)
		transform.v[i] = file->readFloat();
}

void CDynamicGround::serialize(KEnvironment * kenv, File * file)
{
	CGround::serialize(kenv, file);
	for (float &c : mpos)
		file->writeFloat(c);
	for (float &c : mrot)
		file->writeFloat(c);
	kenv->writeObjRef(file, node);
	for (int i = 0; i < 16; i++)
		file->writeFloat(transform.v[i]);
}

void CDynamicGround::onLevelLoaded(KEnvironment * kenv)
{
	int strnum = 0;
	int numfnd = 0;
	for (auto &str : kenv->sectorObjects) {
		if (CKMeshKluster *mk = str.getFirst<CKMeshKluster>()) {
			auto it = std::find_if(mk->grounds.begin(), mk->grounds.end(), [this](kobjref<CGround> &gnd) {return gnd.get() == this; });
			if (it != mk->grounds.end()) {
				auto rnode = kenv->getObjRef<CKSceneNode>(nodeId, strnum);
				if (node.get())
					assert(node == rnode);
				node = rnode;
				numfnd++;
				continue;
			}
		}
		strnum++;
	}
	if (!node.get())
		node = kenv->getObjRef<CKSceneNode>(nodeId);
}

void CKLine::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numSegments = file->readUint8();
	somenum = file->readFloat();
	points.resize(numSegments + 1);
	segmentWeights.resize(numSegments);
	for (Vector3 &p : points)
		for (float &c : p)
			c = file->readFloat();
	for (float &f : segmentWeights)
		f = file->readFloat();
}

void CKLine::serialize(KEnvironment * kenv, File * file)
{
	assert(points.size() == numSegments + 1);
	assert(segmentWeights.size() == numSegments);
	file->writeUint8(numSegments);
	file->writeFloat(somenum);
	for (Vector3 &p : points)
		for (float &c : p)
			file->writeFloat(c);
	for (float &f : segmentWeights)
		file->writeFloat(f);
}

void CKSpline4L::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	unkchar1 = file->readUint8();
	unkfloat1 = file->readFloat();
	unkfloat2 = file->readFloat();
	unkchar2 = file->readUint8();
	numBings = file->readUint32();
	bings.resize(numBings);
	for (Vector3 &v : bings)
		for (float &c : v)
			c = file->readFloat();
	numDings = file->readUint32();
	dings.resize(numDings);
	for (Vector3 &v : dings)
		for (float &c : v)
			c = file->readFloat();
}

void CKSpline4L::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint8(unkchar1);
	file->writeFloat(unkfloat1);
	file->writeFloat(unkfloat2);
	file->writeUint8(unkchar2);
	file->writeUint32(bings.size());
	for (Vector3 &v : bings)
		for (float &c : v)
			file->writeFloat(c);
	file->writeUint32(dings.size());
	for (Vector3 &v : dings)
		for (float &c : v)
			file->writeFloat(c);
}

void CKChoreography::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	unkfloat = file->readFloat();
	unk2 = file->readUint8();
	numKeys = file->readUint8();
}

void CKChoreography::serialize(KEnvironment * kenv, File * file)
{
	file->writeFloat(unkfloat);
	file->writeUint8(unk2);
	file->writeUint8(numKeys);
}

void CKChoreoKey::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t numSlots = file->readUint32();
	slots.resize(numSlots);
	for (auto &slot : slots) {
		for (float &f : slot.position)
			f = file->readFloat();
		for (float &f : slot.direction)
			f = file->readFloat();
		slot.enemyGroup = file->readUint8();
	}
	unk1 = file->readFloat();
	unk2 = file->readFloat();
	unk3 = file->readFloat();
	flags = file->readUint16();
}

void CKChoreoKey::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(slots.size());
	for (auto &slot : slots) {
		for (float &f : slot.position)
			file->writeFloat(f);
		for (float &f : slot.direction)
			file->writeFloat(f);
		file->writeUint8(slot.enemyGroup);
	}
	file->writeFloat(unk1);
	file->writeFloat(unk2);
	file->writeFloat(unk3);
	file->writeUint16(flags);
}

void CKPFGraphNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	for (float &f : lowBBCorner)
		f = file->readFloat();
	for (float &f : highBBCorner)
		f = file->readFloat();
	numCellsX = file->readUint8();
	numCellsZ = file->readUint8();
	int numCells = numCellsX * numCellsZ;
	bool firstHalf = true;
	uint8_t byteRead;
	for (int i = 0; i < numCells; i++) {
		if (firstHalf) {
			byteRead = file->readUint8();
			cells.push_back(byteRead >> 4);
		}
		else {
			cells.push_back(byteRead & 15);
		}
		firstHalf = !firstHalf;
	}
	transitions.resize(file->readUint32());
	for (auto &trans : transitions)
		trans = kenv->readObjRef<CKPFGraphTransition>(file, -1);
	another = kenv->readObjRef<CKPFGraphNode>(file, -1);
}

void CKPFGraphNode::serialize(KEnvironment * kenv, File * file)
{
	for (float &f : lowBBCorner)
		file->writeFloat(f);
	for (float &f : highBBCorner)
		file->writeFloat(f);
	file->writeUint8(numCellsX);
	file->writeUint8(numCellsZ);
	int numCells = numCellsX * numCellsZ;
	bool firstHalf = true;
	uint8_t toWrite;
	for (int i = 0; i < numCells; i++) {
		uint8_t val = cells[i];
		if (firstHalf)
			toWrite = val & 15;
		else {
			toWrite = (toWrite << 4) | (val & 15);
			file->writeUint8(toWrite);
		}
		firstHalf = !firstHalf;
	}
	if (!firstHalf)
		file->writeUint8((toWrite << 4) | 7);
	file->writeUint32(transitions.size());
	for (auto &trans : transitions)
		kenv->writeObjRef(file, trans);
	kenv->writeObjRef(file, another);
}

void CKPFGraphTransition::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	unk1 = file->readUint8();
	node = kenv->readObjRef<CKPFGraphNode>(file, -1);
	unk2 = file->readUint32();
	uint32_t numThings = file->readUint32();
	things.resize(numThings);
	for (auto &t : things) {
		for (float &f : t.matrix)
			f = file->readFloat();
		t.unk = file->readUint32();
	}
}

void CKPFGraphTransition::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint8(unk1);
	kenv->writeObjRef(file, node);
	file->writeUint32(unk2);
	file->writeUint32(things.size());
	for (auto &t : things) {
		for (float &f : t.matrix)
			file->writeFloat(f);
		file->writeUint32(t.unk);
	}
}
