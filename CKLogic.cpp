#include <cassert>
#include "CKLogic.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKHook.h"
#include "CKDictionary.h"
#include "CKNode.h"
#include "CKCinematicNode.h"
#include "CKGroup.h"
#include "CKService.h"
#include "CKManager.h"
#include "CKGraphical.h"
#include "CKComponent.h"
#include "CKCamera.h"

// Default-construct the variant's holding value with specified type index
// if holding value's type is different, else keep the value unchanged.
template<typename T, size_t N = 0> void changeVariantType(T& var, size_t index) {
	if constexpr (N < std::variant_size_v<T>) {
		if (index == N) {
			if (var.index() != N)
				var.emplace<N>();
		}
		else {
			changeVariantType<T, N + 1>(var, index);
		}
	}
}

void CGround::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	[[maybe_unused]] uint32_t numa = file->readUint32();
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

	if (kenv->version >= kenv->KVERSION_XXL2) {
		x2neoByte = file->readUint8();
		if (kenv->version >= kenv->KVERSION_OLYMPIC)
			x4unkRef = kenv->readObjRef<CKObject>(file);
		x2sectorObj = kenv->readObjRef<CKSector>(file);
	}

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
	assert(numa == computeSize());
}

void CGround::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(computeSize());
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

	if (kenv->version >= kenv->KVERSION_XXL2) {
		file->writeUint8(x2neoByte);
		if (kenv->version >= kenv->KVERSION_OLYMPIC)
			kenv->writeObjRef(file, x4unkRef);
		kenv->writeObjRef(file, x2sectorObj);
	}

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
	if (kenv->version >= kenv->KVERSION_XXL2) {
		for (auto* vec : { &x2compdatas1, &x2compdatas2 }) {
			vec->resize(file->readUint32());
			for (auto& elem : *vec)
				elem = kenv->readObjRef<CKObject>(file);
		}
	}
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
	if (kenv->version <= kenv->KVERSION_XXL1) {
		evt1.read(kenv, file, this);
		evt2.read(kenv, file, this);
	} else {
		x2sectorDetector = kenv->readObjRef<CKObject>(file);
	}
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

	if (kenv->version >= kenv->KVERSION_XXL2) {
		for (auto* vec : { &x2compdatas1, &x2compdatas2 }) {
			file->writeUint32(vec->size());
			for (auto& elem : *vec)
				kenv->writeObjRef(file, elem);
		}
	}
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
	if (kenv->version <= kenv->KVERSION_XXL1) {
		evt1.write(kenv, file);
		evt2.write(kenv, file);
	} else {
		kenv->writeObjRef(file, x2sectorDetector);
	}
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
			uint32_t numBeacons = file->readUint32();
			bing.unk2a = file->readUint8();
			bing.numBits = file->readUint8();
			bing.handlerId = file->readUint8();
			if (kenv->version < kenv->KVERSION_ARTHUR) {
				bing.sectorIndex = file->readUint8();
				bing.klusterIndex = (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) ? file->readUint32() : file->readUint8();
				bing.handlerIndex = file->readUint8();
			} else {
				bing.sectorIndex = file->readUint16();
				bing.klusterIndex = file->readUint16();
				bing.handlerIndex = file->readUint16();
			}
			bing.bitIndex = file->readUint16();
			if (numBeacons != 0) {
				bing.handler = kenv->readObjRef<CKObject>(file);
				bing.unk6 = file->readUint32();
				bing.beacons.resize(numBeacons);
				for (Beacon& beacon : bing.beacons) {
					beacon.posx = file->readUint16();
					beacon.posy = file->readUint16();
					beacon.posz = file->readUint16();
					beacon.params = file->readUint16();
				}
			}
			else
				bing.beacons.clear();
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
			if (kenv->version < kenv->KVERSION_ARTHUR) {
				file->writeUint8(bing.sectorIndex); // how to deal with truncation? show error? just ignore it? still not sure, so let's keep the warnings appearing 8)
				if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
					file->writeUint32(bing.klusterIndex);
				else
					file->writeUint8(bing.klusterIndex);
				file->writeUint8(bing.handlerIndex);
			} else {
				file->writeUint16(bing.sectorIndex);
				file->writeUint16(bing.klusterIndex);
				file->writeUint16(bing.handlerIndex);
			}
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
	if (kenv->version >= kenv->KVERSION_XXL2) {
		for (int i = 0; i < numKeys; i++)
			keys.push_back(kenv->readObjRef<CKChoreoKey>(file));
	}
}

void CKChoreography::serialize(KEnvironment * kenv, File * file)
{
	file->writeFloat(unkfloat);
	file->writeUint8(unk2);
	file->writeUint8(numKeys);
	if (kenv->version >= kenv->KVERSION_XXL2) {
		for (int i = 0; i < numKeys; i++)
			kenv->writeObjRef<CKChoreoKey>(file, keys[i]);
	}
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
		slot.enemyGroup = (kenv->version < kenv->KVERSION_XXL2) ? (int8_t)file->readUint8() : (int16_t)file->readUint16();
	}
	if (kenv->version < kenv->KVERSION_XXL2) {
		unk1 = file->readFloat();
		unk2 = file->readFloat();
		unk3 = file->readFloat();
	}
	else
		x2unk1 = file->readFloat();
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
		if (kenv->version < kenv->KVERSION_XXL2)
			file->writeUint8((uint8_t)slot.enemyGroup);
		else
			file->writeUint16((uint16_t)slot.enemyGroup);
	}
	if (kenv->version < kenv->KVERSION_XXL2) {
		file->writeFloat(unk1);
		file->writeFloat(unk2);
		file->writeFloat(unk3);
	}
	else
		file->writeFloat(x2unk1);
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
	if (kenv->version < kenv->KVERSION_XXL2) {
		others.clear();
		if (auto ref = kenv->readObjRef<CKPFGraphNode>(file, -1))
			others.push_back(std::move(ref));
	}
	else {
		others.resize(file->readUint32());
		for (auto &ref : others)
			ref = kenv->readObjRef<CKPFGraphNode>(file, -1);
	}
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
	if (kenv->version < kenv->KVERSION_XXL2) {
		assert(others.size() <= 1);
		if (others.empty())
			kenv->writeObjID(file, nullptr);
		else
			kenv->writeObjRef(file, others[0]);
	}
	else {
		file->writeUint32(others.size());
		for (auto &ref : others)
			kenv->writeObjRef(file, ref);
	}
}

void CKPFGraphTransition::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	unk1 = (kenv->version >= kenv->KVERSION_XXL2) ? file->readUint32() : file->readUint8();
	node = kenv->readObjRef<CKPFGraphNode>(file, -1);
	unk2 = file->readUint32();
	if (kenv->version >= kenv->KVERSION_XXL2) {
		x2UnkA = file->readFloat();
		if (kenv->version >= kenv->KVERSION_OLYMPIC) {
			ogUnkB = file->readFloat();
			ogUnkC = file->readFloat();
		}
	}

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
	if (kenv->version >= kenv->KVERSION_XXL2)
		file->writeUint32(unk1);
	else
		file->writeUint8((uint8_t)unk1);
	kenv->writeObjRef(file, node);
	file->writeUint32(unk2);
	if (kenv->version >= kenv->KVERSION_XXL2) {
		file->writeFloat(x2UnkA);
		if (kenv->version >= kenv->KVERSION_OLYMPIC) {
			file->writeFloat(ogUnkB);
			file->writeFloat(ogUnkC);
		}
	}

	file->writeUint32(things.size());
	for (auto &t : things) {
		for (float &f : t.matrix)
			file->writeFloat(f);
		file->writeUint32(t.unk);
	}
}

void CKFlaggedPath::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	line = kenv->readObjRef<CKObject>(file);
	numPoints = file->readUint32();
	fpSomeFloat = file->readFloat();
	pntValues.resize(numPoints);
	for (float &f : pntValues)
		f = file->readFloat();
	pntEvents.resize(numPoints);
	for (EventNode &evt : pntEvents)
		evt.read(kenv, file, this);
}

void CKFlaggedPath::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, line);
	file->writeUint32(numPoints);
	file->writeFloat(fpSomeFloat);
	for (float &f : pntValues)
		file->writeFloat(f);
	for (EventNode &evt : pntEvents)
		evt.write(kenv, file);
}

void CKCinematicSceneData::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	hook = kenv->readObjRef<CKHook>(file);
	animDict = kenv->readObjRef<CAnimationDictionary>(file);
	csdUnkA = file->readUint8();
	csdUnkB = file->readUint32();
}

void CKCinematicSceneData::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, hook);
	kenv->writeObjRef(file, animDict);
	file->writeUint8(csdUnkA);
	file->writeUint32(csdUnkB);
}

void CKCinematicScene::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	csUnk1 = file->readUint16();
	cineDatas.resize(file->readUint32());
	for (auto &data : cineDatas)
		data = kenv->readObjRef<CKCinematicSceneData>(file);
	cineNodes.resize(file->readUint32());
	for (auto &node : cineNodes)
		node = kenv->readObjRef<CKCinematicNode>(file);
	startDoor = kenv->readObjRef<CKStartDoor>(file);
	csUnk2 = file->readUint8();
	csUnk3 = file->readUint32();
	csUnk4 = file->readFloat();
	csUnk5 = file->readFloat();
	csUnk6 = file->readFloat();
	csUnk7 = file->readFloat();
	csUnk8 = file->readFloat();
	csUnk9 = file->readFloat();
	csUnkA = file->readFloat();
	onSomething.read(kenv, file, this);
	groups.resize(file->readUint32());
	for (auto &grp : groups)
		grp = kenv->readObjRef<CKObject>(file);
	sndDict = kenv->readObjRef<CKSoundDictionaryID>(file);
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->platform != kenv->PLATFORM_PS2)
		csUnkF = file->readUint8();
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
		for (auto &u : otherUnkF)
			u = file->readUint8();
}

void CKCinematicScene::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(csUnk1);
	file->writeUint32(cineDatas.size());
	for (auto &data : cineDatas)
		kenv->writeObjRef(file, data);
	file->writeUint32(cineNodes.size());
	for (auto &node : cineNodes)
		kenv->writeObjRef(file, node);
	kenv->writeObjRef(file, startDoor);
	file->writeUint8(csUnk2);
	file->writeUint32(csUnk3);
	file->writeFloat(csUnk4);
	file->writeFloat(csUnk5);
	file->writeFloat(csUnk6);
	file->writeFloat(csUnk7);
	file->writeFloat(csUnk8);
	file->writeFloat(csUnk9);
	file->writeFloat(csUnkA);
	onSomething.write(kenv, file);
	file->writeUint32(groups.size());
	for (auto &grp : groups)
		kenv->writeObjRef(file, grp);
	kenv->writeObjRef(file, sndDict);
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->platform != kenv->PLATFORM_PS2)
		file->writeUint8(csUnkF);
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
		for (const auto &u : otherUnkF)
			file->writeUint8(u);
}

void CKGameState::readSVV8(KEnvironment * kenv, File * file, std::vector<StateValue<uint8_t>>& gameValues, bool hasByte)
{
	gameValues.resize(file->readUint32());
	for (auto &val : gameValues) {
		val.object = kenv->readObjRef<CKObject>(file);
		if(hasByte)
			val.data = file->readUint8();
	}
}

void CKGameState::writeSVV8(KEnvironment * kenv, File * file, std::vector<StateValue<uint8_t>>& gameValues, bool hasByte)
{
	file->writeUint32(gameValues.size());
	for (auto &val : gameValues) {
		kenv->writeObjRef(file, val.object);
		if(hasByte)
			file->writeUint8(val.data);
	}
}

void CKGameState::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	gsName.resize(file->readUint16());
	file->read((void*)gsName.data(), gsName.size());
	gsUnk1 = file->readUint32();
	gsStructureRef = file->readUint32();
	gsSpawnPoint = kenv->readObjRef<CKObject>(file);

	readSVV8(kenv, file, gsStages, true);
	readSVV8(kenv, file, gsModules, true);
}

void CKGameState::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(gsName.size());
	file->write(gsName.data(), gsName.size());
	file->writeUint32(gsUnk1);
	file->writeUint32(gsStructureRef);
	kenv->writeObjRef(file, gsSpawnPoint);

	writeSVV8(kenv, file, gsStages, true);
	writeSVV8(kenv, file, gsModules, true);
}

void CKGameState::deserializeLvlSpecific(KEnvironment * kenv, File * file, size_t length)
{
	for (auto &lvlValues : lvlValuesArray) {
		lvlValues.resize(file->readUint32());
		for (auto &val : lvlValues) {
			val.object = kenv->readObjRef<CKObject>(file);
			size_t nextoff = file->readUint32();
			size_t len = nextoff - file->tell();
			val.data.resize(len);
			file->read(val.data.data(), len);
		}
	}
}

void CKGameState::serializeLvlSpecific(KEnvironment * kenv, File * file)
{
	for (auto &lvlValues : lvlValuesArray) {
		file->writeUint32(lvlValues.size());
		for (auto &val : lvlValues) {
			kenv->writeObjRef(file, val.object);
			file->writeUint32(file->tell() + 4 + val.data.size());
			file->write(val.data.data(), val.data.size());
		}
	}
}

void CKGameState::resetLvlSpecific(KEnvironment * kenv)
{
	for (auto &lvlValues : lvlValuesArray)
		lvlValues.clear();
	for (auto &gameValues : { &gsStages, &gsModules })
		gameValues->clear();
}

void CKA2GameState::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGameState::deserialize(kenv, file, length);

	readSVV8(kenv, file, gsDiamondHelmets, true);
	readSVV8(kenv, file, gsVideos, true);
	readSVV8(kenv, file, gsShoppingAreas, true);

	gsUnk2 = file->readUint32();
	file->read(gsRemainder.data(), gsRemainder.size());

	//printf("gsUnk1 = %08X\n", gsUnk1);
	if ((gsUnk1 & 1) == 0)
		deserializeLvlSpecific(kenv, file, length);
}

void CKA2GameState::serialize(KEnvironment * kenv, File * file)
{
	CKGameState::serialize(kenv, file);

	writeSVV8(kenv, file, gsDiamondHelmets, true);
	writeSVV8(kenv, file, gsVideos, true);
	writeSVV8(kenv, file, gsShoppingAreas, true);

	file->writeUint32(gsUnk2);
	file->write(gsRemainder.data(), gsRemainder.size());

	if ((gsUnk1 & 1) == 0)
		serializeLvlSpecific(kenv, file);
}

void CKA2GameState::resetLvlSpecific(KEnvironment * kenv)
{
	CKGameState::resetLvlSpecific(kenv);
	for (auto &gameValues : { &gsDiamondHelmets, &gsVideos, &gsShoppingAreas })
		gameValues->clear();
}

void CKA3GameState::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKGameState::deserialize(kenv, file, length);

	readSVV8(kenv, file, gsVideos, true);
	readSVV8(kenv, file, gsUnkObjects, true); // hasByte or not?
	gsStdText = kenv->readObjRef<CKObject>(file);
	readSVV8(kenv, file, gsBirdZones, false);
	readSVV8(kenv, file, gsBirdlimes, false);
	readSVV8(kenv, file, gsShields, false);
	readSVV8(kenv, file, gsTalcs, true);

	file->read(gsRemainder.data(), gsRemainder.size());

	//printf("gsUnk1 = %08X\n", gsUnk1);
	if ((gsUnk1 & 1) == 0)
		deserializeLvlSpecific(kenv, file, length);

}

void CKA3GameState::serialize(KEnvironment * kenv, File * file)
{
	CKGameState::serialize(kenv, file);

	writeSVV8(kenv, file, gsVideos, true);
	writeSVV8(kenv, file, gsUnkObjects, true); // hasByte or not?
	kenv->writeObjRef(file, gsStdText);
	writeSVV8(kenv, file, gsBirdZones, false);
	writeSVV8(kenv, file, gsBirdlimes, false);
	writeSVV8(kenv, file, gsShields, false);
	writeSVV8(kenv, file, gsTalcs, true);

	file->write(gsRemainder.data(), gsRemainder.size());

	if ((gsUnk1 & 1) == 0)
		serializeLvlSpecific(kenv, file);
}

void CKA3GameState::resetLvlSpecific(KEnvironment * kenv)
{
	CKGameState::resetLvlSpecific(kenv);
	for (auto &gameValues : { &gsVideos, &gsUnkObjects, &gsBirdZones, &gsBirdlimes, &gsShields, &gsTalcs })
		gameValues->clear();
	gsStdText = nullptr;
}

void CKMsgAction::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	mas1.resize(file->readUint32());
	for (MAStruct1 &a : mas1) {
		a.mas2.resize(file->readUint8());
	}
	for (MAStruct1 &a : mas1) {
		for (MAStruct2 &b : a.mas2) {
			b.event = file->readUint32();
			b.mas3.resize(file->readUint8());
		}
	}
	for (MAStruct1 &a : mas1) {
		for (MAStruct2 &b : a.mas2) {
			for (MAStruct3 &c : b.mas3) {
				c.num = file->readUint8();
				c.mas4.resize(file->readUint8());
			}
		}
	}
	for (MAStruct1 &a : mas1) {
		for (MAStruct2 &b : a.mas2) {
			for (MAStruct3 &c : b.mas3) {
				for (MAStruct4 &d : c.mas4) {
					d.type = file->readUint32();
					switch (d.type) {
					case 2:
						d.valFloat = file->readFloat(); break;
					case 3:
						d.ref = kenv->readObjRef<CKObject>(file); break;
					default:
						d.valU32 = file->readUint32(); break;
					}
				}
			}
		}
	}
}

void CKMsgAction::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(mas1.size());
	for (MAStruct1 &a : mas1) {
		file->writeUint8(a.mas2.size());
	}
	for (MAStruct1 &a : mas1) {
		for (MAStruct2 &b : a.mas2) {
			file->writeUint32(b.event);
			file->writeUint8(b.mas3.size());
		}
	}
	for (MAStruct1 &a : mas1) {
		for (MAStruct2 &b : a.mas2) {
			for (MAStruct3 &c : b.mas3) {
				file->writeUint8(c.num);
				file->writeUint8(c.mas4.size());
			}
		}
	}
	for (MAStruct1 &a : mas1) {
		for (MAStruct2 &b : a.mas2) {
			for (MAStruct3 &c : b.mas3) {
				for (MAStruct4 &d : c.mas4) {
					file->writeUint32(d.type);
					switch (d.type) {
					case 2:
						file->writeFloat(d.valFloat); break;
					case 3:
						kenv->writeObjRef<CKObject>(file, d.ref); break;
					default:
						file->writeUint32(d.valU32); break;
					}
				}
			}
		}
	}

}

void CKBundle::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	next = kenv->readObjRef<CKBundle>(file);
	flags = file->readUint8();
	grpLife = kenv->readObjRef<CKGroupLife>(file);
	firstHookLife = kenv->readObjRef<CKHookLife>(file);
	otherHookLife = kenv->readObjRef<CKHookLife>(file);
}

void CKBundle::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, next);
	file->writeUint8(flags);
	kenv->writeObjRef(file, grpLife);
	kenv->writeObjRef(file, firstHookLife);
	kenv->writeObjRef(file, otherHookLife);
}

void CKTriggerDomain::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version < kenv->KVERSION_OLYMPIC) {
		unkRef = kenv->readObjRef<CKObject>(file);
		activeSector = file->readUint32();
	}
	flags = file->readUint32();
	subdomains.resize(file->readUint32());
	triggers.resize(file->readUint32());
	for (auto &ref : subdomains)
		ref = kenv->readObjRef<CKTriggerDomain>(file);
	for (auto &ref : triggers)
		ref = kenv->readObjRef<CKTrigger>(file);
	if (kenv->version >= kenv->KVERSION_OLYMPIC)
		triggerSynchro = kenv->readObjRef<CKTriggerSynchro>(file);
}

void CKTriggerDomain::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version < kenv->KVERSION_OLYMPIC) {
		kenv->writeObjRef(file, unkRef);
		file->writeUint32(activeSector);
	}
	file->writeUint32(flags);
	file->writeUint32(subdomains.size());
	file->writeUint32(triggers.size());
	for (auto &ref : subdomains)
		kenv->writeObjRef(file, ref);
	for (auto &ref : triggers)
		kenv->writeObjRef(file, ref);
	if (kenv->version >= kenv->KVERSION_OLYMPIC)
		kenv->writeObjRef(file, triggerSynchro);
}

void CKTriggerDomain::onLevelLoaded(KEnvironment* kenv)
{
	// Bind the trigger action target postrefs using activeSector for XXL2
	// or the CKTriggerSynchro for Arthur/OG+
	auto fixtrigger = [kenv](CKTrigger *trig, int str) {
		for (auto& act : trig->actions) {
			act.target.bind(kenv, str);
			if (act.valType == 3)
				act.valRef.bind(kenv, str);
		}
	};
	if (kenv->version <= kenv->KVERSION_ARTHUR) {
		int str = (int)this->activeSector - 1;
		for (auto& trig : triggers)
			fixtrigger(trig.get(), str);
	}
	else if (kenv->version >= kenv->KVERSION_OLYMPIC) {
		const auto& lvldoms = kenv->levelObjects.getFirst<CKSrvTrigger>()->rootDomain->subdomains;
		auto it = std::find_if(lvldoms.begin(), lvldoms.end(), [this](const auto& ref) {return ref.get() == this; });
		if (it != lvldoms.end() && triggerSynchro) {
			auto she = std::find_if(triggerSynchro->elements.begin(), triggerSynchro->elements.end(), [this](const CKTriggerSynchro::SynchroElement& se) {return se.domains[0].get() == this; });
			if (she != triggerSynchro->elements.end()) {
				int str = (int)she->syeunk - 1;
				auto walk = [str,&fixtrigger](CKTriggerDomain* dom, const auto& rec) -> void {
					for (auto& trig : dom->triggers)
						fixtrigger(trig.get(), str);
					for (auto& subdom : dom->subdomains)
						rec(subdom.get(), rec);
				};
				walk(this, walk);
			}
		}
	}
}

void CKTrigger::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (kenv->version >= kenv->KVERSION_OLYMPIC) {
		ogDatas.resize(file->readUint32());
		for (auto &ref : ogDatas)
			ref = kenv->readObjRef<CKObject>(file);
	}
	condition = kenv->readObjRef<CKConditionNode>(file);
	actions.resize(file->readUint32());
	for (Action &act : actions) {
		act.target.read(file);
		act.event = file->readUint16();
		act.valType = file->readUint32();
		switch (act.valType) {
		case 0: act.valU8 = file->readUint8(); break;
		case 1: act.valU32 = file->readUint32(); break;
		case 2: act.valFloat = file->readFloat(); break;
		case 3: act.valRef.read(file); break;
		default: assert(nullptr && "unknown trigger value type");
		}
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		uint8_t ogUnk2 = file->readUint8();
		assert(ogUnk2 == 1);
	}
}

void CKTrigger::serialize(KEnvironment * kenv, File * file)
{
	if (kenv->version >= kenv->KVERSION_OLYMPIC) {
		file->writeUint32(ogDatas.size());
		for (auto &ref : ogDatas)
			kenv->writeObjRef(file, ref);
	}
	kenv->writeObjRef(file, condition);
	file->writeUint32(actions.size());
	for (const Action &act : actions) {
		act.target.write(kenv, file);
		file->writeUint16(act.event);
		file->writeUint32(act.valType);
		switch (act.valType) {
		case 0: file->writeUint8(act.valU8); break;
		case 1: file->writeUint32(act.valU32); break;
		case 2: file->writeFloat(act.valFloat); break;
		case 3: act.valRef.write(kenv, file); break;
		default: assert(nullptr && "unknown trigger value type");
		}
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR)
		file->writeUint8(1);
}

void CKLevel::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	lvlNumber = file->readUint32();
	sectors.resize(file->readUint32());
	for (auto &str : sectors)
		str = kenv->readObjRef<CKSector>(file);
	objs.resize(file->readUint32());
	for (auto &obj : objs)
		obj = kenv->readObjRef<CKObject>(file);
	if (!kenv->isRemaster) {
		initialSector[0] = file->readUint32();
	}
	else {
		for (uint32_t &e : initialSector)
			e = file->readUint32();
		for (std::string &s : lvlRemasterCheatSpawnNames)
			s = file->readSizedString<uint16_t>();
	}
	lvlUnk2 = file->readUint8();
	lvlUnk3 = file->readUint32();
}

void CKLevel::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(lvlNumber);
	file->writeUint32(sectors.size());
	for (auto &str : sectors)
		kenv->writeObjRef(file, str);
	file->writeUint32(objs.size());
	for (auto &obj : objs)
		kenv->writeObjRef(file, obj);
	if (!kenv->isRemaster) {
		file->writeUint32(initialSector[0]);
	}
	else {
		for (uint32_t &e : initialSector)
			file->writeUint32(e);
		for (std::string &s : lvlRemasterCheatSpawnNames)
			file->writeSizedString<uint16_t>(s);
	}
	file->writeUint8(lvlUnk2);
	file->writeUint32(lvlUnk3);
}


void CKDefaultGameManager::reflectMembers(MemberListener &r) {
	CKReflectableLogic::reflectMembers(r);
	r.reflect(dgmGrpEnemy, "dgmGrpEnemy");
	r.reflect(dgmGrpTrio, "dgmGrpTrio");
	r.reflect(dgmGrpMeca, "dgmGrpMeca");
	r.reflect(dgmGrpBonus, "dgmGrpBonus");
}
void CKAsterixGameManager::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKDefaultGameManager::reflectMembers(r);
	r.reflect(dgmBillboard, "dgmBillboard");
	r.reflect(dgmUnk1, "dgmUnk1", this);
	r.reflect(dgmUnk2, "dgmUnk2", this);
	r.reflect(dgmUnk3, "dgmUnk3", this);
	r.reflect(dgmUnk4, "dgmUnk4", this);
	r.reflect(dgmUnk5, "dgmUnk5", this);
	r.reflect(dgmUnk6, "dgmUnk6", this);
	r.reflect(dgmUnk7, "dgmUnk7", this);
	r.reflect(dgmUnk8, "dgmUnk8", this);
	r.reflect(dgmUnk9, "dgmUnk9", this);
	r.reflect(dgmUnk10, "dgmUnk10", this);
	r.reflect(dgmUnk11, "dgmUnk11", this);
	r.reflect(dgmUnk12, "dgmUnk12", this);
	r.reflect(dgmUnk13, "dgmUnk13", this);
	r.reflect(dgmUnk14, "dgmUnk14", this);
	r.reflect(dgmUnk15, "dgmUnk15", this);
	r.reflect(dgmUnk16, "dgmUnk16", this);
	r.reflect(dgmUnk17, "dgmUnk17", this);
	r.reflect(dgmUnk18, "dgmUnk18", this);
	r.reflect(dgmUnk19, "dgmUnk19", this);
	r.reflect(dgmUnk20, "dgmUnk20", this);
	r.reflect(dgmUnk21, "dgmUnk21", this);
	r.reflect(dgmUnk22, "dgmUnk22", this);
	r.reflect(dgmUnk23, "dgmUnk23", this);
	r.reflect(dgmUnk24, "dgmUnk24", this);
	r.reflect(dgmUnk25, "dgmUnk25", this);
	r.reflect(dgmUnk26, "dgmUnk26", this);
	r.reflect(dgmUnk27, "dgmUnk27", this);
	r.reflect(dgmUnk28, "dgmUnk28", this);
	r.reflect(dgmUnk29, "dgmUnk29", this);
	r.reflect(dgmUnk30, "dgmUnk30", this);
	r.reflect(dgmUnk31, "dgmUnk31", this);
	r.reflect(dgmUnk32, "dgmUnk32", this);
	r.reflect(dgmUnk33, "dgmUnk33", this);
	r.reflect(dgmUnk34, "dgmUnk34", this);
	r.reflect(dgmUnk35, "dgmUnk35", this);
	r.reflect(dgmUnk36, "dgmUnk36", this);
	r.reflect(dgmUnk37, "dgmUnk37", this);
	r.reflect(dgmUnk38, "dgmUnk38", this);
	r.reflect(dgmUnk39, "dgmUnk39", this);
	r.reflect(dgmUnk40, "dgmUnk40", this);
	r.reflect(dgmUnk41, "dgmUnk41", this);
	r.reflect(dgmUnk42, "dgmUnk42", this);
	r.reflect(dgmUnk43, "dgmUnk43", this);
	r.reflect(dgmUnk44, "dgmUnk44", this);
	r.reflect(dgmUnk45, "dgmUnk45", this);
	r.reflect(dgmUnk46, "dgmUnk46", this);
	r.reflect(dgmUnk47, "dgmUnk47", this);
	r.reflect(dgmUnk48, "dgmUnk48", this);
	r.reflect(dgmUnk49, "dgmUnk49", this);
	r.reflect(dgmUnk50, "dgmUnk50", this);
	r.reflect(dgmUnk51, "dgmUnk51", this);
	r.reflect(dgmUnk52, "dgmUnk52", this);
	r.reflect(dgmUnk53, "dgmUnk53", this);
	r.reflect(dgmUnk54, "dgmUnk54", this);
	r.reflect(dgmUnk55, "dgmUnk55", this);
	r.reflect(dgmUnk56, "dgmUnk56", this);
	r.reflect(dgmUnk57, "dgmUnk57", this);
	r.reflect(dgmUnk58, "dgmUnk58", this);
	r.reflect(dgmUnk59, "dgmUnk59", this);
	r.reflect(dgmUnk60, "dgmUnk60", this);
	r.reflect(dgmUnk61, "dgmUnk61", this);
	r.reflect(dgmUnk62, "dgmUnk62", this);
	r.reflect(dgmUnk63, "dgmUnk63", this);
	r.reflect(dgmUnk64, "dgmUnk64", this);
	r.reflect(dgmUnk65, "dgmUnk65", this);
	r.reflect(dgmUnk66, "dgmUnk66", this);
	r.reflect(dgmUnk67, "dgmUnk67", this);
	r.reflect(dgmUnk68, "dgmUnk68", this);
	r.reflect(dgmUnk69, "dgmUnk69", this);
	r.reflect(dgmUnk70, "dgmUnk70", this);
	r.reflect(dgmUnk71, "dgmUnk71", this);
	r.reflect(dgmUnk72, "dgmUnk72", this);
	r.reflect(dgmUnk73, "dgmUnk73", this);
	r.reflect(dgmUnk74, "dgmUnk74", this);
	r.reflect(dgmUnk75, "dgmUnk75", this);
	r.reflect(dgmUnk76, "dgmUnk76", this);
	r.reflect(dgmUnk77, "dgmUnk77", this);
	r.reflect(dgmUnk78, "dgmUnk78", this);
	r.reflect(dgmUnk79, "dgmUnk79", this);
	r.reflect(dgmUnk80, "dgmUnk80", this);
	r.reflect(dgmUnk81, "dgmUnk81", this);
	r.reflect(dgmUnk82, "dgmUnk82", this);
	r.reflect(dgmUnk83, "dgmUnk83", this);
	r.reflect(dgmUnk84, "dgmUnk84", this);
	r.reflect(dgmUnk85, "dgmUnk85", this);
	r.reflect(dgmUnk86, "dgmUnk86", this);
	r.reflect(dgmUnk87, "dgmUnk87", this);
	r.reflect(dgmUnk88, "dgmUnk88", this);
	r.reflect(dgmUnk89, "dgmUnk89", this);
	r.reflect(dgmUnk90, "dgmUnk90", this);
	r.reflect(dgmUnk91, "dgmUnk91", this);
	r.reflect(dgmUnk92, "dgmUnk92", this);
	r.reflect(dgmUnk93, "dgmUnk93", this);
	r.reflect(dgmUnk94, "dgmUnk94", this);
	r.reflect(dgmUnk95, "dgmUnk95", this);
	r.reflect(dgmUnk96, "dgmUnk96", this);
	r.reflect(dgmUnk97, "dgmUnk97", this);
	r.reflect(dgmUnk98, "dgmUnk98", this);
	r.reflect(dgmUnk99, "dgmUnk99", this);
	r.reflect(dgmUnk100, "dgmUnk100", this);
	r.reflect(dgmUnk101, "dgmUnk101", this);
	r.reflect(dgmUnk102, "dgmUnk102", this);
	r.reflect(dgmUnk103, "dgmUnk103", this);
	r.reflect(dgmUnk104, "dgmUnk104");
	r.reflect(dgmCamera, "dgmCamera");
	r.reflect(dgmUnk106, "dgmUnk106", this);
	r.reflect(dgmUnk107, "dgmUnk107");
	r.reflect(dgmUnk108, "dgmUnk108");
	r.reflect(dgmUnk109, "dgmUnk109");
	r.reflect(dgmUnk110, "dgmUnk110");
	r.reflect(dgmUnk111, "dgmUnk111");
	r.reflect(dgmUnk112, "dgmUnk112", this);
	r.reflect(dgmUnk113, "dgmUnk113");
	r.reflect(dgmUnk114, "dgmUnk114");
	r.reflect(dgmUnk115, "dgmUnk115", this);
	r.reflect(dgmGrpCheckpoint, "dgmGrpCheckpoint");
	r.reflect(dgmUnk117, "dgmUnk117", this);
	r.reflect(dgmUnk118, "dgmUnk118", this);
	if (!kenv->isRemaster) {
		r.reflect(dgmDrmValue, "dgmDrmValue");
		r.reflect(dgmUnk120, "dgmUnk120");
		r.reflect(dgmUnk121, "dgmUnk121");
		r.reflect(dgmUnk122, "dgmUnk122");
	}
	r.reflect(dgmUnk123, "dgmUnk123");
	r.reflect(dgmUnk124, "dgmUnk124");
	r.reflect(dgmUnk125, "dgmUnk125");
	r.reflect(dgmUnk126, "dgmUnk126");
	r.reflect(dgmUnk127, "dgmUnk127");
	r.reflect(dgmUnk128, "dgmUnk128");
	r.reflect(dgmUnk129, "dgmUnk129");
	r.reflect(dgmUnk130, "dgmUnk130");
	r.reflect(dgmUnk131, "dgmUnk131");
	r.reflect(dgmUnk132, "dgmUnk132");
	r.reflect(dgmUnk133, "dgmUnk133");
	r.reflect(dgmUnk134, "dgmUnk134");
	r.reflect(dgmUnk135, "dgmUnk135");
	r.reflect(dgmUnk136, "dgmUnk136");
	r.reflect(dgmUnk137, "dgmUnk137");
	r.reflect(dgmUnk138, "dgmUnk138");
	r.reflect(dgmUnk139, "dgmUnk139");
	r.reflect(dgmSkyLife, "dgmSkyLife");
	r.reflect(dgmUnk141, "dgmUnk141");
	r.reflect(dgmUnk142, "dgmUnk142");
	r.reflect(dgmUnk143, "dgmUnk143");
	r.reflect(dgmUnk144, "dgmUnk144");
	r.reflect(dgmUnk145, "dgmUnk145");
	r.reflect(dgmUnk146, "dgmUnk146");
	r.reflect(dgmUnk147, "dgmUnk147");
	r.reflect(dgmUnk148, "dgmUnk148");
	r.reflect(dgmUnk149, "dgmUnk149");
	r.reflect(dgmUnk150, "dgmUnk150");
	r.reflect(dgmUnk151, "dgmUnk151");
	r.reflect(dgmUnk152, "dgmUnk152");
	r.reflect(dgmUnk153, "dgmUnk153");
	r.reflect(dgmCam1, "dgmCam1");
	r.reflect(dgmCam2, "dgmCam2");
	r.reflect(dgmUnk156, "dgmUnk156");
}
void CKAsterixGameManager::deserializeGlobal(KEnvironment* kenv, File* file, size_t length)
{
	file->read(dgmGlobalBytes.data(), dgmGlobalBytes.size());
}
void CKSekens::SLine::reflectMembers(MemberListener &r) {
	r.reflect(mUnk0, "mUnk0");
	r.reflect(mUnk1, "mUnk1");
	r.reflect(mUnk2, "mUnk2");
}
void CKSekens::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKReflectableLogic::reflectMembers(r);
	r.reflect(sekManager2d, "sekManager2d");
	r.reflect(sekSndManager, "sekSndManager");
	r.reflectSize<uint32_t>(sekLines, "sizeFor_sekLines");
	r.reflect(sekLines, "sekLines");
	if (kenv->isRemaster) {
		r.reflect(sekRomaSndDictID, "sekRomaSndDictID");
		sekRomaLineNames.resize(sekLines.size());
		r.reflect(sekRomaLineNames, "sekRomaLineNames");
	}
	r.reflect(sekUnk4, "sekUnk4");
	r.reflect(sekUnk5, "sekUnk5");
}

void CKCoreManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	groupRoot = kenv->readObjRef<CKGroupRoot>(file);
	srvLife = kenv->readObjRef<CKServiceLife>(file);
}

void CKCoreManager::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, groupRoot);
	kenv->writeObjRef(file, srvLife);
}

void CKCoreManager::init(KEnvironment * kenv)
{
	groupRoot = kenv->createAndInitObject<CKGroupRoot>();
	//srvLife = kenv->createAndInitObject<CKServiceLife>();
	srvLife = kenv->levelObjects.getFirst<CKServiceManager>()->addService<CKServiceLife>(kenv);
}

void CKTriggerSynchro::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	elements.resize(file->readUint32());
	for (auto& el : elements) {
		el.domains.resize(file->readUint32());
		for (auto& dom : el.domains)
			dom = kenv->readObjRef<CKTriggerDomain>(file);
		el.syeunk = file->readUint32();
	}
	syncModule = kenv->readObjRef<CKObject>(file);
}

void CKTriggerSynchro::serialize(KEnvironment* kenv, File* file)
{
	file->writeUint32(elements.size());
	for (const auto& el : elements) {
		file->writeUint32(el.domains.size());
		for (const auto& dom : el.domains)
			kenv->writeObjRef(file, dom);
		file->writeUint32(el.syeunk);
	}
	kenv->writeObjRef(file, syncModule);
}

void CKFlashNode2dFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(billboard, "billboard");
}

void CKElectricArcNodeFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(node, "node");
}

void CKQuadNodeFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(node, "node");
}

void CKLightningObjectNodeFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(node, "node");
	r.reflect(animDict, "animDict");
}

void CKFilterNode2dFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(billboard, "billboard");
}

void CKExplosionNodeFx::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(node, "node");
	r.reflect(node2, "node2");
	r.reflect(partNode, "partNode");
}

void CLocManager::deserializeGlobal(KEnvironment* kenv, File* file, size_t length)
{
	if (kenv->version < kenv->KVERSION_OLYMPIC) {
		lmUnk0 = file->readUint32();
		numTrcStrings = file->readUint32();
		lmNumDings = file->readUint32();
		numStdStrings = file->readUint32();

		if (kenv->version == kenv->KVERSION_XXL1 && (kenv->platform == kenv->PLATFORM_PS2 || kenv->isRemaster)) {
			numLanguages = file->readUint16();
			for (int i = 0; i < numLanguages; i++)
				langStrIndices.push_back(file->readUint32());
			for (int i = 0; i < numLanguages; i++)
				langIDs.push_back(file->readUint32());
		}

		lmDings.resize(lmNumDings);
		for (auto& d : lmDings) {
			d.lmdUnk1 = file->readUint32();
			d.lmdUnk2 = file->readUint32();
		}

		if (kenv->version == kenv->KVERSION_ARTHUR) {
			lmArStdStrInfo.resize(numStdStrings);
			for (uint32_t& elem : lmArStdStrInfo)
				elem = file->readUint32();
		}
	}
	else {
		lmNumDings = file->readUint32();
		lmDings.resize(lmNumDings);
		for (auto& d : lmDings) {
			d.lmdUnk1 = file->readUint32();
			d.lmdUnk2 = file->readUint32();
			if (kenv->version >= kenv->KVERSION_SPYRO)
				d.lmdUnk3 = file->readUint32();
		}
		numStdStrings = file->readUint32();
		stdStringRefs.resize(numStdStrings);
		for (auto& ref : stdStringRefs)
			ref = kenv->readObjRef<CKObject>(file);
		numTrcStrings = file->readUint32();
		trcStringRefs.resize(numTrcStrings);
		for (auto& trc : trcStringRefs) {
			trc.first = file->readUint32();
			trc.second = kenv->readObjRef<CKObject>(file);
		}
	}
}

void CKDetectorBase::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(dbFlags, "dbFlags");
	r.reflect(dbGeometry, "dbGeometry");
	r.reflect(dbMovable, "dbMovable");
	r.reflect(dbSectorIndex, "dbSectorIndex");
}

void CKSectorDetector::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflectSize<uint32_t>(sdDetectors, "sizeFor_sdDetectors");
	r.reflect(sdDetectors, "sdDetectors");
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		r.reflectSize<uint32_t>(sdGeometries, "sizeFor_sdGeometries");
		r.reflect(sdGeometries, "sdGeometries");
	}
}

void CMultiGeometry::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(mgShapeType, "mgShapeType");
	if (mgShapeType == 0) {
		r.reflect(mgAABB.highCorner, "mgAABB.highCorner");
		r.reflect(mgAABB.lowCorner, "mgAABB.lowCorner");
	}
	else if (mgShapeType == 1) {
		r.reflect(mgSphere.center, "mgSphere.center");
		r.reflect(mgSphere.radius, "mgSphere.radius");
		float sqrad = mgSphere.radius * mgSphere.radius;
		r.reflect(sqrad, "mgSphere.radius^2");
	}
	else if (mgShapeType == 2) {
		r.reflect(mgAACylinder.center, "mgAACylinder.center");
		r.reflect(mgAACylinder.radius, "mgAACylinder.radius");
		r.reflect(mgAACylinder.height, "mgAACylinder.height");
		r.reflect(mgAACyInfo, "mgAACyInfo");
	}
}

void CKDetectorEvent::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKDetectorBase::reflectMembers2(r, kenv);
	r.reflectSize<uint32_t>(deCmpDatas1, "sizeFor_deCmpDatas1");
	r.reflect(deCmpDatas1, "deCmpDatas1");
	r.reflectSize<uint32_t>(deCmpDatas2, "sizeFor_deCmpDatas2");
	r.reflect(deCmpDatas2, "deCmpDatas2");
	r.reflectSize<uint32_t>(deCmpDatas3, "sizeFor_deCmpDatas3");
	r.reflect(deCmpDatas3, "deCmpDatas3");
}

void CKDetectedMovable::Movable::reflectMembers(MemberListener& r)
{
	r.reflect(dtmovSceneNode, "dtmovSceneNode");
	r.reflect(dtmovSectorIndex, "dtmovSectorIndex");
	r.reflect(dtmovUnkFlag, "dtmovUnkFlag");
}

void CKDetectedMovable::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		dtmovMovables.resize(1);
		r.reflect(dtmovMovables.at(0).dtmovSceneNode, "dtmovSceneNode");
		r.reflect(dtmovMovables.at(0).dtmovSectorIndex, "dtmovSectorIndex");
	}
	else {
		r.reflectSize<uint32_t>(dtmovMovables, "sizeFor_dtmovMovables");
		r.reflect(dtmovMovables, "dtmovMovables");
	}
}

void CKDetectedMovable::onLevelLoaded(KEnvironment* kenv)
{
	for (auto& mov : dtmovMovables) {
		mov.dtmovSceneNode.bind(kenv, mov.dtmovSectorIndex - 1);
	}
}

void CKDetectorMusic::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKDetectorBase::reflectMembers2(r, kenv);
	r.reflect(dtmusUnk1, "dtmusUnk1");
	r.reflect(dtmusUnk2, "dtmusUnk2");
	r.reflect(dtmusUnk3, "dtmusUnk3");
}

void CKComparedData::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(cmpdatType, "cmpdatType");
	int type = (cmpdatType >> 2) & 3;
	changeVariantType(cmpdatValue, type);
	if (CmpDataType0* val = std::get_if<CmpDataType0>(&cmpdatValue)) {
		r.reflect(val->cmpdatT0Ref, "cmpdatT0Ref");
		r.reflect(val->cmpdatT0Unk1, "cmpdatT0Unk1");
		r.reflect(val->cmpdatT0Unk2, "cmpdatT0Unk2");
		r.reflect(val->cmpdatT0Unk3, "cmpdatT0Unk3");
	}
	else if (CmpDataType1* val = std::get_if<CmpDataType1>(&cmpdatValue)) {
		r.reflect(val->cmpdatT1Subtype, "cmpdatT1Subtype");
		if (val->cmpdatT1Subtype == 0) r.reflect(val->cmpdatT1Byte, "cmpdatT1Byte");
		if (val->cmpdatT1Subtype == 1) r.reflect(val->cmpdatT1Int, "cmpdatT1Int");
		if (val->cmpdatT1Subtype == 2) r.reflect(val->cmpdatT1Float, "cmpdatT1Float");
		if (val->cmpdatT1Subtype == 3) r.reflect(val->cmpdatT1Ref, "cmpdatT1Ref");
	}
	else if (CmpDataType2* val = std::get_if<CmpDataType2>(&cmpdatValue)) {
		r.reflect(val->cmpdatT2Unk1, "cmpdatT2Unk1");
		if (kenv->version >= kenv->KVERSION_ARTHUR)
			r.reflect(val->cmpdatT2Trigger, "cmpdatT2Trigger");
	}
}

void CKConditionNode::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	if (kenv->version < kenv->KVERSION_ARTHUR)
		r.reflect(nextCondNode, "nextCondNode");
	r.reflect(condNodeType, "condNodeType");
}

void CKCombiner::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKConditionNode::reflectMembers2(r, kenv);
	if (kenv->version < kenv->KVERSION_ARTHUR)
		r.reflect(childCondNode, "childCondNode");
	else {
		r.reflectSize<uint32_t>(condNodeChildren, "sizeFor_condNodeChildren");
		r.reflect(condNodeChildren, "condNodeChildren");
	}
}

void CKCombiner::onLevelLoaded(KEnvironment* kenv)
{
	if (kenv->version < kenv->KVERSION_ARTHUR) {
		for (CKConditionNode* sub = childCondNode.get(); sub; sub = sub->nextCondNode.get()) {
			condNodeChildren.emplace_back(sub);
		}
	}
}

void CKComparator::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	CKConditionNode::reflectMembers2(r, kenv);
	r.reflect(leftCmpData, "leftCmpData");
	r.reflect(rightCmpData, "rightCmpData");
}

void CMaterial::Extension::reflectMembers(MemberListener& r)
{
	r.reflect(extUnkCom_1, "extUnkCom_1");
	r.reflect(extUnkCom_2, "extUnkCom_2");
	switch (extensionType) {
	case 0:
	case 1:
		r.reflect(extUnk0_1, "extUnk0_1");
		r.reflect(extUnk0_2, "extUnk0_2");
		r.reflect(extUnk0_3, "extUnk0_3");
		r.reflect(extUnk0_4, "extUnk0_4");
		break;
	case 2:
		r.reflect(extUnk2_1, "extUnk2_1");
		r.reflect(extUnk2_2, "extUnk2_2");
		r.reflect(extUnk2_3, "extUnk2_3");
		break;
	case 3:
		break;
	case 4:
		r.reflect(extUnk4_1, "extUnk4_1");
		r.reflect(extUnk4_2, "extUnk4_2");
		r.reflect(extUnk4_3, "extUnk4_3");
		break;
	default:
		assert(false && "unknown material extension type");
	}
}

void CMaterial::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(geometry, "geometry");
	r.reflect(flags, "flags");
	int type = (flags >> 3) & 15;
	switch (type) {
	case 5:
		break;
	case 6:
		break;
	case 7:
	case 8:
		assert(false && "CMaterial flags problem");
	default:
		;
	}
	if ((flags & 0x100) && (type == 5 || type == 6)) {
		int cnt = type - 4; // 5->1, 6->2
		int b = (kenv->version >= kenv->KVERSION_OLYMPIC) ? (cnt * 7 + 12) : (cnt * 4 + 9);
		extensions.resize(cnt);
		for (auto& ext : extensions) {
			b -= (kenv->version >= kenv->KVERSION_OLYMPIC) ? 7 : 4;
			int exttype = ((flags >> b) & ((kenv->version >= kenv->KVERSION_OLYMPIC) ? 7 : 3));
			if (kenv->version < kenv->KVERSION_OLYMPIC)
				ext.extensionType = (exttype==2) ? 2 : 3;
			else
				ext.extensionType = exttype;
			r.reflect(ext, ("ext" + std::to_string(b)).c_str());
		}
	}
	if (kenv->version >= kenv->KVERSION_OLYMPIC) {
		if ((flags & 0x78) == 0x48) {
			r.reflect(ogUnkA1, "ogUnkA1");
			r.reflect(ogUnkA2, "ogUnkA2");
		}
		r.reflect(ogUnkFlt, "ogUnkFlt");
	}
}

void CKProjectileTypeBase::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(ckptbpfxUnk0, "ckptbpfxUnk0");
	r.reflect(ckptbpfxUnk1, "ckptbpfxUnk1");
}

void CKProjectileTypeScrap::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKProjectileTypeBase::reflectMembers2(r, kenv);
	r.reflect(ckptsUnk2, "ckptsUnk2");
	ckptsUnk3.resize(ckptbpfxUnk0);
	r.reflect(ckptsUnk3, "ckptsUnk3");
};

void CKProjectileTypeAsterixBomb::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKProjectileTypeBase::reflectMembers2(r, kenv);
	r.reflect(ckptabUnk2, "ckptabUnk2");
	r.reflect(ckptabUnk3, "ckptabUnk3");
	r.reflect(ckptabUnk4, "ckptabUnk4");
	r.reflect(ckptabUnk5, "ckptabUnk5");
	r.reflect(ckptabUnk6, "ckptabUnk6");
	r.reflect(ckptabUnk7, "ckptabUnk7");
	r.reflect(ckptabUnk8, "ckptabUnk8");
	r.reflect(ckptabUnk9, "ckptabUnk9");
	r.reflect(ckptabUnk10, "ckptabUnk10");
	r.reflect(ckptabUnk11, "ckptabUnk11");
	r.reflect(ckptabUnk12, "ckptabUnk12");
	ckptabUnk13.resize(ckptbpfxUnk0);
	r.reflect(ckptabUnk13, "ckptabUnk13");
};

void CKProjectileTypeBallisticPFX::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	CKProjectileTypeBase::reflectMembers2(r, kenv);
	r.reflect(ckptbpfxUnk2, "ckptbpfxUnk2");
	r.reflect(ckptbpfxUnk3, "ckptbpfxUnk3");
	r.reflect(ckptbpfxUnk4, "ckptbpfxUnk4");
	r.reflect(ckptbpfxUnk5, "ckptbpfxUnk5");
	r.reflect(ckptbpfxUnk6, "ckptbpfxUnk6");
	r.reflect(ckptbpfxUnk7, "ckptbpfxUnk7");
	ckptbpfxUnk8.resize(ckptbpfxUnk0);
	r.reflect(ckptbpfxUnk8, "ckptbpfxUnk8");
};

void CKSpline4::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(cksUnk0, "cksUnk0");
	r.reflect(cksUnk1, "cksUnk1");
	r.reflect(cksUnk2, "cksUnk2");
	r.reflect(cksUnk3, "cksUnk3");
	r.reflectSize<uint32_t>(cksUnk5, "size_cksUnk5");
	r.reflect(cksUnk5, "cksUnk5");
	r.reflectSize<uint32_t>(cksUnk7, "size_cksUnk7");
	r.reflect(cksUnk7, "cksUnk7");
	r.reflectSize<uint32_t>(cksUnk9, "size_cksUnk9");
	r.reflect(cksUnk9, "cksUnk9");
	r.reflectSize<uint32_t>(cksUnk11, "size_cksUnk11");
	r.reflect(cksUnk11, "cksUnk11");
	r.reflectSize<uint32_t>(cksUnk13, "size_cksUnk13");
	r.reflect(cksUnk13, "cksUnk13");
};

void CKCameraSector::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(ckcsUnk0, "ckcsUnk0");
	r.reflect(ckcsUnk1, "ckcsUnk1");
	r.reflect(ckcsUnk2, "ckcsUnk2");
	r.reflect(ckcsUnk3, "ckcsUnk3");
	r.reflect(ckcsUnk4, "ckcsUnk4");
	r.reflect(ckcsUnk5, "ckcsUnk5");
};

void CKAsterixSlideFP::reflectMembers2(MemberListener& r, KEnvironment* kenv) {
	r.reflect(asfpSpline, "asfpSpline");
	r.reflectSize<uint32_t>(slideParts, "num_slideParts");
	r.reflect(asfpLength, "asfpLength");
	r.reflect(asfpGrpTrio, "asfpGrpTrio");
	r.enterArray("slideParts");
	for (auto& part : slideParts) {
		r.enterStruct("slideParts");
		r.reflect(part.spValue, "value");
		r.reflect(part.spEvent, "event", this);
		r.leaveStruct();
		r.incrementIndex();
	}
	r.leaveArray();
};

void CWall::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	this->numa = file->readUint32();
	uint16_t numTris = file->readUint16();
	uint16_t numVerts = file->readUint16();
	this->triangles.resize(numTris);
	this->vertices.resize(numVerts);
	for (auto& tri : this->triangles) {
		for (auto& coord : tri.indices)
			coord = file->readUint16();
	}
	for (Vector3& vert : this->vertices) {
		vert.x = file->readFloat();
		vert.y = file->readFloat();
		vert.z = file->readFloat();
	}
	aabb.deserialize(file);
	param1 = file->readUint16();
	param2 = file->readUint16();

	for (float& f : wallMat1.v)
		f = file->readFloat();
	for (float& f : wallMat2.v)
		f = file->readFloat();
}

void CWall::serialize(KEnvironment* kenv, File* file)
{
	file->writeUint32(((6 * triangles.size() + 12 * vertices.size()) + 3) & ~3);
	file->writeUint16(this->triangles.size());
	file->writeUint16(this->vertices.size());
	for (auto& tri : this->triangles) {
		for (auto& coord : tri.indices)
			file->writeUint16(coord);
	}
	for (Vector3& vert : this->vertices) {
		file->writeFloat(vert.x);
		file->writeFloat(vert.y);
		file->writeFloat(vert.z);
	}
	aabb.serialize(file);
	file->writeUint16(param1);
	file->writeUint16(param2);

	for (float& f : wallMat1.v)
		file->writeFloat(f);
	for (float& f : wallMat2.v)
		file->writeFloat(f);
}
