#include "CKService.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKLogic.h"
#include "CKNode.h"
#include "CKHook.h"
#include "CKGroup.h"

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
	objs.resize(numObjs);
	for (auto &obj : objs)
		obj.read(file);
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
		obj.write(kenv, file);
	for (uint16_t &arg : objInfos)
		file->writeUint16(arg);
}

void CKSrvEvent::onLevelLoaded2(KEnvironment * kenv)
{
	int eventindex = 0;
	for (StructB &b : bees) {
		for (int i = 0; i < b._1; i++) {
			if (b.userFound) {
				int str = -1;
				if (!b.users.empty()) {
					CKObject *user = b.users[0];
					if (CKFlaggedPath *path = user->dyncast<CKFlaggedPath>()) {
						str = path->usingSector;
					}
					else if (CKSector *kstr = user->dyncast<CKSector>()) {
						str = kstr->strId - 1;
					}
					else if (CKHkLight *hkLight = /*nullptr*/ user->dyncast<CKHkLight>()) {
						CKGrpLight *grpLight = hkLight->lightGrpLight->cast<CKGrpLight>();
						std::vector<Vector3> &points = grpLight->node->cast<CNode>()->geometry->cast<CKParticleGeometry>()->pgPoints;
						int index = points.size() - 1;
						CKHook *hook;
						for (hook = grpLight->childHook.get(); hook; hook = hook->next.get()) {
							if (hook == hkLight)
								break;
							index--;
						}
						assert(hook);
						const Vector3 &pnt = points[index];
						int bestSector = -1; float bestDistance = HUGE_VALF;
						for (int sector = 0; sector < kenv->numSectors; sector++) {
							CKMeshKluster *kluster = kenv->sectorObjects[sector].getFirst<CKMeshKluster>();
							for (auto &gnd : kluster->grounds) {
								if (gnd->isSubclassOf<CDynamicGround>())
									continue;
								if (pnt.x >= gnd->aabb.lowCorner.x && pnt.x <= gnd->aabb.highCorner.x
									&& pnt.z >= gnd->aabb.lowCorner.z && pnt.z <= gnd->aabb.highCorner.z)
								{
									float dist = 0.0f;
									if (pnt.y < gnd->aabb.lowCorner.y)
										dist = std::abs(pnt.y - gnd->aabb.lowCorner.y);
									else if (pnt.y > gnd->aabb.highCorner.y)
										dist = std::abs(pnt.y - gnd->aabb.highCorner.y);
									if (dist < bestDistance) {
										bestSector = sector;
										bestDistance = dist;
									}
								}
							}
						}
						str = bestSector;
					}
					else if (CKHook *hook = user->dyncast<CKHook>())
						if (hook->life)
							str = (hook->life->unk1 >> 2) - 1;
				}
				objs[eventindex + i].bind(kenv, str);
			}
			else objs[eventindex + i].bind(kenv, -1);
		}
		eventindex += b._1;
	}
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

void CKSrvPathFinding::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	nodes.resize(file->readUint32());
	for (auto &ref : nodes)
		ref = kenv->readObjRef<CKPFGraphNode>(file, -1);
}

void CKSrvPathFinding::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(nodes.size());
	for (auto &ref : nodes)
		kenv->writeObjRef(file, ref);
}

void CKSrvMarker::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	lists.resize(file->readUint32());
	for (std::vector<Marker> &list : lists) {
		list.resize(file->readUint32());
		for (Marker &marker : list) {
			for (float &f : marker.position)
				f = file->readFloat();
			marker.orientation1 = file->readUint8();
			marker.orientation2 = file->readUint8();
			marker.val3 = file->readUint16();
		}
	}
}

void CKSrvMarker::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(lists.size());
	for (std::vector<Marker> &list : lists) {
		file->writeUint32(list.size());
		for (Marker &marker : list) {
			for (float &f : marker.position)
				file->writeFloat(f);
			file->writeUint8(marker.orientation1);
			file->writeUint8(marker.orientation2);
			file->writeUint16(marker.val3);
		}
	}
}

void CKSrvDetector::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numA = file->readUint16();
	numB = file->readUint16();
	numC = file->readUint16();
	numD = file->readUint16();
	numE = file->readUint16();
	numAABB = file->readUint16();
	numSpheres = file->readUint16();
	numRectangles = file->readUint16();
	numRefs = file->readUint16();
	numJ = file->readUint16();

	aaBoundingBoxes.resize(numAABB);
	for (auto &aabb : aaBoundingBoxes)
		aabb.deserialize(file);

	spheres.resize(numSpheres);
	for (auto &sph : spheres)
		sph.deserialize(file, true);

	rectangles.resize(numRectangles);
	for (auto &h : rectangles) {
		for (float &f : h.center)
			f = file->readFloat();
		h.length1 = file->readFloat();
		h.length2 = file->readFloat();
		h.direction = file->readUint8();
	}

	for (auto detvec : { std::make_pair(&aDetectors, &numA), std::make_pair(&bDetectors, &numB), std::make_pair(&cDetectors, &numC),
						 std::make_pair(&dDetectors, &numD), std::make_pair(&eDetectors, &numE) })
	{
		detvec.first->resize(*detvec.second);
		for (auto &det : *detvec.first) {
			det.shapeIndex = file->readUint16();
			det.nodeIndex = file->readUint16();
			det.flags = file->readUint16();
			det.eventSeqIndex.read(kenv, file, this);
		}
	}

	nodes.resize(numRefs);
	for (auto &ref : nodes)
		ref = kenv->readObjRef<CKSceneNode>(file, -1);
}

void CKSrvDetector::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(numA);
	file->writeUint16(numB);
	file->writeUint16(numC);
	file->writeUint16(numD);
	file->writeUint16(numE);
	file->writeUint16(numAABB);
	file->writeUint16(numSpheres);
	file->writeUint16(numRectangles);
	file->writeUint16(numRefs);
	file->writeUint16(numJ);

	for (auto &aabb : aaBoundingBoxes)
		aabb.serialize(file);

	for (auto &sph : spheres)
		sph.serialize(file, true);

	for (auto &h : rectangles) {
		for (float &f : h.center)
			file->writeFloat(f);
		file->writeFloat(h.length1);
		file->writeFloat(h.length2);
		file->writeUint8(h.direction);
	}

	for (auto detvec : { std::make_pair(&aDetectors, &numA), std::make_pair(&bDetectors, &numB), std::make_pair(&cDetectors, &numC),
						 std::make_pair(&dDetectors, &numD), std::make_pair(&eDetectors, &numE) })
	{
		for (const auto &det : *detvec.first) {
			file->writeUint16(det.shapeIndex);
			file->writeUint16(det.nodeIndex);
			file->writeUint16(det.flags);
			det.eventSeqIndex.write(kenv, file);
		}
	}

	for (auto &ref : nodes)
		kenv->writeObjRef(file, ref);

}
