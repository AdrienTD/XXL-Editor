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
		if(kenv->version >= kenv->KVERSION_XXL2)
			handler.x2respawn = file->readUint8();
		handler.object = kenv->readObjRef<CKObject>(file);
	}
	numSectors = file->readUint32();
	beaconSectors.resize(numSectors);
	for (auto it = beaconSectors.rbegin(); it != beaconSectors.rend(); it++) {
		BeaconSector &bs = *it;
		bs.numUsedBings = file->readUint32();
		bs.numBings = file->readUint32();
		bs.beaconArraySize = file->readUint32();
		uint32_t numBits = file->readUint32();
		BitReader bread(file);
		for (uint32_t i = 0; i < numBits; i++)
			bs.bits.push_back(bread.readBit());
		uint32_t numBeaconKlusters;
		if (kenv->version >= kenv->KVERSION_XXL2 || kenv->isRemaster) // xxl1 Romaster extended it to 32-bit
			numBeaconKlusters = file->readUint32();
		else
			numBeaconKlusters = file->readUint8();
		for (uint32_t i = 0; i < numBeaconKlusters; i++)
			bs._bkids.push_back(file->readUint32());
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
		if (kenv->version >= kenv->KVERSION_XXL2)
			file->writeUint8(handler.x2respawn);
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
		if (kenv->version >= kenv->KVERSION_XXL2 || kenv->isRemaster) // xxl1 Romaster extended it to 32-bit
			file->writeUint32(bs.beaconKlusters.size());
		else
			file->writeUint8(bs.beaconKlusters.size());
		for (auto &ref : bs.beaconKlusters)
			kenv->writeObjRef(file, ref);
	}
}

void CKSrvBeacon::onLevelLoaded(KEnvironment * kenv)
{
	int str = -1;
	for (BeaconSector &bs : beaconSectors) {
		for (uint32_t id : bs._bkids)
			bs.beaconKlusters.push_back(kenv->getObjRef<CKBeaconKluster>(id, str));
		str++;
	}
}

void CKSrvBeacon::removeBeacon(int sectorIndex, int klusterIndex, int bingIndex, int beaconIndex)
{
	BeaconSector& bsec = beaconSectors[sectorIndex];
	CKBeaconKluster* bkluster = bsec.beaconKlusters[klusterIndex].get();
	CKBeaconKluster::Bing& bbing = bkluster->bings[bingIndex];

	// remove bits
	int beaNumBits = handlers[bbing.handlerIndex].numBits;
	int beax = bbing.bitIndex + beaNumBits * beaconIndex;
	auto it = bsec.bits.begin() + beax;
	bsec.bits.erase(it, it + beaNumBits);

	// remove beacon
	bbing.beacons.erase(bbing.beacons.begin() + beaconIndex);
	bsec.beaconArraySize -= 8;

	// relocate bitIndex of next bings
	bingIndex++;
	while (bkluster) {
		for (; bingIndex < bkluster->bings.size(); bingIndex++) {
			CKBeaconKluster::Bing& bing = bkluster->bings[bingIndex];
			if (bing.active)
				bing.bitIndex -= beaNumBits;
		}
		bingIndex = 0;
		bkluster = bkluster->nextKluster.get();
	}
}

void CKSrvBeacon::addHandler(CKObject* handler, uint8_t numBits, uint8_t handlerId, uint8_t persistent, uint8_t respawn)
{
	size_t numHands = handlers.size();
	CKSrvBeacon::Handler h = { 8, numBits, numHands, handlerId, persistent, respawn, handler };
	handlers.push_back(h);

	// add new bing at every kluster
	for (auto& bsec : beaconSectors) {
		bsec.numBings += bsec.beaconKlusters.size();
		for (auto& bk : bsec.beaconKlusters) {
			bk->bings.emplace_back();
		}
	}
}

void CKSrvBeacon::enableBing(int sectorIndex, int klusterIndex, int bingIndex)
{
	auto& bsec = beaconSectors[sectorIndex];
	auto& bkluster = bsec.beaconKlusters[klusterIndex];
	auto& bing = bkluster->bings[bingIndex];
	auto& hs = handlers[bingIndex];

	if (bing.active)
		return;

	// find bit position
	int bx = 0;
	for (auto& k : bsec.beaconKlusters) {
		for (auto& g : k->bings) {
			if (&g == &bing)
				goto fnd;
			bx += g.numBits * g.beacons.size();
		}
	}
fnd:
	bing.active = true;
	bing.handler = hs.object.get();
	bing.unk2a = hs.unk2a;
	bing.numBits = hs.numBits;
	bing.handlerId = hs.handlerId;
	bing.sectorIndex = sectorIndex;
	bing.klusterIndex = klusterIndex;
	bing.handlerIndex = hs.handlerIndex;
	bing.bitIndex = bx;
	bing.unk6 = 0x128c;	// some class id?

	bkluster->numUsedBings++;
	bsec.numUsedBings++;
}

void CKSrvBeacon::addBeacon(int sectorIndex, int klusterIndex, int handlerIndex, const void * _beacon)
{
	const auto& beacon = *(const CKBeaconKluster::Beacon*)_beacon;
	enableBing(sectorIndex, klusterIndex, handlerIndex);
	auto& bsec = beaconSectors[sectorIndex];
	auto* bkluster = bsec.beaconKlusters[klusterIndex].get();
	auto& bing = bkluster->bings[handlerIndex];
	auto& hs = handlers[handlerIndex];

	int beaconIndex = (int)bing.beacons.size();
	bing.beacons.push_back(beacon);
	bsec.beaconArraySize += 8;

	// add bits
	int beaNumBits = hs.numBits;
	int beax = bing.bitIndex + beaNumBits * beaconIndex;
	bsec.bits.insert(bsec.bits.begin() + beax, beaNumBits, true);
	for (int i = 0; i < beaNumBits; i++)
		bsec.bits[beax + i] = (beacon.params >> i) & 1;

	// relocate bitIndex of next bings
	handlerIndex++;
	while (bkluster) {
		for (; handlerIndex < bkluster->bings.size(); handlerIndex++) {
			CKBeaconKluster::Bing& bing = bkluster->bings[handlerIndex];
			if (bing.active)
				bing.bitIndex += beaNumBits;
		}
		handlerIndex = 0;
		bkluster = bkluster->nextKluster.get();
	}
}

int CKSrvBeacon::addKluster(KEnvironment& kenv, int sectorIndex)
{
	auto& strObjList = (sectorIndex == 0) ? kenv.levelObjects : kenv.sectorObjects[sectorIndex-1];
	BeaconSector& bsec = beaconSectors[sectorIndex];

	CKBeaconKluster* kluster = kenv.createObject<CKBeaconKluster>(sectorIndex-1);
	int klusterIndex = (int)bsec.beaconKlusters.size();
	bsec.beaconKlusters.push_back(kluster);
	bsec.numBings += handlers.size();

	auto numBkObjs = strObjList.getClassType<CKBeaconKluster>().objects.size();
	if (numBkObjs >= 2) {
		strObjList.getObject<CKBeaconKluster>(numBkObjs - 2)->nextKluster = kluster;
	}

	kluster->numUsedBings = 0;
	kluster->bings.resize(handlers.size());
	return klusterIndex;
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
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		for (auto &ref : arQuadTreeBranches)
			ref = kenv->readObjRef<CKObject>(file);
		for (auto &val : arQTBInts)
			val = file->readUint32();
	}
	if(kenv->version >= kenv->KVERSION_XXL2)
		x2flt = file->readFloat();
}

void CKSrvPathFinding::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(nodes.size());
	for (auto &ref : nodes)
		kenv->writeObjRef(file, ref);
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		for (auto &ref : arQuadTreeBranches)
			kenv->writeObjRef(file, ref);
		for (auto &val : arQTBInts)
			file->writeUint32(val);
	}
	if (kenv->version >= kenv->KVERSION_XXL2)
		file->writeFloat(x2flt);
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

void CKSrvCinematic::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	cineScenes.resize(file->readUint32());
	for (auto &scene : cineScenes)
		scene = kenv->readObjRef<CKCinematicScene>(file);
	cineBillboard1 = kenv->readObjRef<CKObject>(file);
	cineBillboard2 = kenv->readObjRef<CKObject>(file);
	cineBillboard3 = kenv->readObjRef<CKObject>(file);
}

void CKSrvCinematic::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(cineScenes.size());
	for (auto &scene : cineScenes)
		kenv->writeObjRef(file, scene);
	kenv->writeObjRef(file, cineBillboard1);
	kenv->writeObjRef(file, cineBillboard2);
	kenv->writeObjRef(file, cineBillboard3);
}

void CKSrvTrigger::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	rootDomain = kenv->readObjRef<CKTriggerDomain>(file);
	stUnk1 = file->readUint32();
	stUnk2 = file->readUint32();
}

void CKSrvTrigger::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, rootDomain);
	file->writeUint32(stUnk1);
	file->writeUint32(stUnk2);
}

void CKSrvCamera::reflectMembers2(MemberListener & r, KEnvironment *kenv)
{
	CKReflectableService::reflectMembers(r);
	r.reflect(scamUnk0, "scamUnk0");
	r.reflect(scamCam, "scamCam");
	r.reflect(scamCamstr, "scamCamstr");
	if(kenv->isRemaster)
		r.reflect(scamCamfixtrack, "scamCamfixtrack");
	else
		r.reflect(scamCamfixtrack[0], "scamCamfixtrack");
	scamCameraInst.reflectMembers2(r, kenv);
	if(kenv->isRemaster)
		r.reflect(scamAnimNode, "scamAnimNode");
	else
		r.reflect(scamAnimNode[0], "scamAnimNode");
	r.reflect(scamUnk15, "scamUnk15");
	r.reflect(scamUnk16, "scamUnk16");
	r.reflect(scamUnk17, "scamUnk17");
	r.reflect(scamUnk18, "scamUnk18");
	r.reflect(scamUnk19, "scamUnk19");
	r.reflect(scamUnk20, "scamUnk20");
	r.reflect(scamUnk21, "scamUnk21");
	r.reflect(scamUnk22, "scamUnk22");
	r.reflect(scamSphere1, "scamSphere1");
	r.reflect(scamSphere2, "scamSphere2");
	r.reflect(scamUnk25, "scamUnk25");
	r.reflect(scamUnk26, "scamUnk26");
	r.reflect(scamUnk27, "scamUnk27");
	r.reflect(scamUnk28, "scamUnk28");
	r.reflect(scamUnk29, "scamUnk29");
	r.reflect(scamUnk30, "scamUnk30");
	r.reflect(scamUnk31, "scamUnk31");
	r.reflect(scamUnk32, "scamUnk32");
	r.reflect(scamUnk33, "scamUnk33");
	r.reflect(scamUnk34, "scamUnk34");
	r.reflect(scamUnk35, "scamUnk35");
	r.reflect(scamUnk36, "scamUnk36");
	if (kenv->isRemaster) {
		r.reflect(scamRoma1, "scamRoma1");
		r.reflect(scamRoma2, "scamRoma2");
	}
}

void CKServiceLife::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	firstBundle = kenv->readObjRef<CKBundle>(file);
}

void CKServiceLife::serialize(KEnvironment * kenv, File * file)
{
	kenv->writeObjRef(file, firstBundle);
}

void CKSrvSekensor::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	sekens.resize(file->readUint32());
	for (auto &ref : sekens)
		ref = kenv->readObjRef<CKSekens>(file);
}

void CKSrvSekensor::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(sekens.size());
	for (auto &ref : sekens)
		kenv->writeObjRef(file, ref);
}

void CKSrvAvoidance::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	avoidValue = file->readFloat();
}

void CKSrvAvoidance::serialize(KEnvironment * kenv, File * file)
{
	file->writeFloat(avoidValue);
}

void CKSrvShadow::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	shadUnk1 = file->readUint32();
	shadUnk2 = file->readUint32();
	shadNode = kenv->readObjRef<CNode>(file);
	shadGeometry = kenv->readObjRef<CKParticleGeometry>(file);
	shadTexture = file->readSizedString<uint16_t>();
	shadValues.resize(file->readUint8());
	for (auto& a : shadValues)
		for (float& f : a)
			f = file->readFloat();
	shadUnk3 = file->readUint8();
	shadUnk4 = file->readUint8();
	shadUnk5 = file->readFloat();
}

void CKSrvShadow::serialize(KEnvironment* kenv, File* file)
{
	file->writeUint32(shadUnk1);
	file->writeUint32(shadUnk2);
	kenv->writeObjRef(file, shadNode);
	kenv->writeObjRef(file, shadGeometry);
	file->writeSizedString<uint16_t>(shadTexture);
	file->writeUint8(shadValues.size()); // truncation
	for (auto& a : shadValues)
		for (float& f : a)
			file->writeFloat(f);
	file->writeUint8(shadUnk3);
	file->writeUint8(shadUnk4);
	file->writeFloat(shadUnk5);
}

void CKSrvFx::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	fxTypes.resize(file->readUint8());
	for (auto& fxt : fxTypes) {
		fxt.clsFullId = file->readUint32();
		fxt.numInstances = file->readUint8();
		fxt.startIndex = file->readUint8();
	}
	fxInstances.resize(file->readUint8());
	for (auto& fxi : fxInstances)
		fxi = kenv->readObjRef<CKObject>(file);
}

void CKSrvFx::serialize(KEnvironment* kenv, File* file)
{
	file->writeUint8(fxTypes.size());
	for (auto& fxt : fxTypes) {
		file->writeUint32(fxt.clsFullId);
		file->writeUint8(fxt.numInstances);
		file->writeUint8(fxt.startIndex);
	}
	file->writeUint8(fxInstances.size());
	for (auto& fxi : fxInstances)
		kenv->writeObjRef<CKObject>(file, fxi);
}
