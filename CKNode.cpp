#include "CKNode.h"
#include "File.h"
#include "KEnvironment.h"
#include "rw.h"
#include "GuiUtils.h"
#include "CKLogic.h"

void CKSceneNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	for (int i = 0; i < 16; i++)
		this->transform.v[i] = file->readFloat();
	this->parent = kenv->readObjRef<CKSceneNode>(file);
	this->unk1 = (kenv->version >= kenv->KVERSION_XXL2) ? file->readUint32() : file->readUint16();
	this->unk2 = file->readUint8();
	this->next = kenv->readObjRef<CKSceneNode>(file);
}

void CKSceneNode::serialize(KEnvironment * kenv, File * file)
{
	for (int i = 0; i < 16; i++)
		file->writeFloat(this->transform.v[i]);
	kenv->writeObjRef(file, this->parent);
	if (kenv->version >= kenv->KVERSION_XXL2)
		file->writeUint32(this->unk1);
	else
		file->writeUint16((uint16_t)this->unk1);
	file->writeUint8(this->unk2);
	kenv->writeObjRef(file, this->next);
}

Matrix CKSceneNode::getGlobalMatrix() const
{
	Matrix glob = this->transform;
	glob._14 = glob._24 = glob._34 = 0.0f;
	glob._44 = 1.0f;
	for (const CKSceneNode *node = this->parent.get(); node; node = node->parent.get()) {
		Matrix loc = node->transform;
		loc._14 = loc._24 = loc._34 = 0.0f;
		loc._44 = 1.0f;
		glob = loc * glob;
	}
	return glob;
}

void CNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CSGBranch::deserialize(kenv, file, length);
	this->geometry = kenv->readObjRef<CKAnyGeometry>(file);
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		this->romSomeName = file->readString(file->readUint16());
		kenv->setObjectName(this, GuiUtils::latinToUtf8(romSomeName));
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR)
		this->ogUnkFloat = file->readFloat();
}

void CNode::serialize(KEnvironment * kenv, File * file)
{
	CSGBranch::serialize(kenv, file);
	kenv->writeObjRef(file, this->geometry);
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		file->writeUint16(this->romSomeName.size());
		file->writeString(this->romSomeName);
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR)
		file->writeFloat(this->ogUnkFloat);
}

void CSGSectorRoot::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	if (kenv->version >= kenv->KVERSION_XXL2)
		this->sectorNum = file->readUint32();
	this->texDictionary = kenv->readObjRef<CKObject>(file);
}

void CSGSectorRoot::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	if (kenv->version >= kenv->KVERSION_XXL2)
		file->writeUint32(this->sectorNum);
	kenv->writeObjRef(file, this->texDictionary);
}

void CSGBranch::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CSGLeaf::deserialize(kenv, file, length);
	this->child = kenv->readObjRef<CKSceneNode>(file);
}

void CSGBranch::serialize(KEnvironment * kenv, File * file)
{
	CSGLeaf::serialize(kenv, file);
	kenv->writeObjRef(file, this->child);
}

void CSGBranch::insertChild(CKSceneNode * newChild)
{
	newChild->next = this->child;
	this->child = newChild;
	newChild->parent = this;
}

void CSGBranch::removeChild(CKSceneNode * toremove)
{
	if (this->child.get() == toremove) {
		this->child = toremove->next;
		toremove->next = nullptr;
		toremove->parent = nullptr;
	}
	for (CKSceneNode *sub = this->child.get(); sub; sub = sub->next.get()) {
		if (sub->next.get() == toremove) {
			sub->next = toremove->next;
			toremove->next = nullptr;
			toremove->parent = nullptr;
			break;
		}
	}
}

void CClone::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	if (kenv->version == kenv->KVERSION_XXL2 && kenv->isRemaster)
		hdKifName = file->readString(file->readUint16());
	cloneInfo = file->readUint32();
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
		hdKifName = file->readString(file->readUint16());
}

void CClone::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	if (kenv->version == kenv->KVERSION_XXL2 && kenv->isRemaster) {
		file->writeUint16(hdKifName.size());
		file->writeString(hdKifName);
	}
	file->writeUint32(cloneInfo);
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		file->writeUint16(hdKifName.size());
		file->writeString(hdKifName);
	}
}

void CAnimatedNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	branchs = kenv->readObjRef<CKSceneNode>(file);
	if (kenv->version < kenv->KVERSION_XXL2)
		unkref = kenv->readObjRef<CKObject>(file);
	if (kenv->version >= kenv->KVERSION_XXL2)
		if (unk1 & 1)
			x2someNum = (int32_t)file->readUint32();

	numBones = file->readUint32();
	rwCheckHeader(file, 0xE);
	frameList = std::make_shared<RwFrameList>();
	frameList->deserialize(file);

	if (kenv->isRemaster) {
		hdBoneNames.resize(file->readUint32());
		for (auto &str : hdBoneNames)
			str = file->readString(file->readUint16());
	}

	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		file->read(ogBlendBytes.data(), ogBlendBytes.size());
		ogBlender = kenv->readObjRef<CKObject>(file);
		if ((this->unk1 & 0x600) == 0x400) {
			auto aBlender = kenv->readObjRef<CKObject>(file);
			assert(aBlender == ogBlender);
		}
		ogBlendFloat = file->readFloat();
	}
}

void CAnimatedNode::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	kenv->writeObjRef(file, branchs);
	if (kenv->version < kenv->KVERSION_XXL2)
		kenv->writeObjRef(file, unkref);
	if (kenv->version >= kenv->KVERSION_XXL2)
		if (unk1 & 1)
			file->writeUint32((uint32_t)x2someNum);

	file->writeUint32(numBones);
	frameList->serialize(file);

	if (kenv->isRemaster) {
		file->writeUint32(hdBoneNames.size());
		for (const auto &str : hdBoneNames) {
			file->writeUint16(str.size());
			file->writeString(str);
		}
	}

	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		file->write(ogBlendBytes.data(), ogBlendBytes.size());
		kenv->writeObjRef(file, ogBlender);
		if ((this->unk1 & 0x600) == 0x400) {
			kenv->writeObjRef(file, ogBlender);
		}
		file->writeFloat(ogBlendFloat);
	}
}

void CAnimatedClone::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);

	if (kenv->version >= kenv->KVERSION_XXL2)
		if (unk1 & 1)
			x2someNum = (int32_t)file->readUint32();

	if (kenv->version == kenv->KVERSION_XXL2 && kenv->isRemaster)
		hdKifName = file->readString(file->readUint16());

	branchs = kenv->readObjRef<CKSceneNode>(file);
	cloneInfo = file->readUint32();

	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
		hdKifName = file->readString(file->readUint16());

	if (kenv->isRemaster) {
		hdBoneNames.resize(file->readUint32());
		for (auto &str : hdBoneNames)
			str = file->readString(file->readUint16());
	}

	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		ogBlender = kenv->readObjRef<CKObject>(file);
		file->read(ogBlendBytes.data(), ogBlendBytes.size());
		if ((this->unk1 & 0x600) == 0x400) {
			auto aBlender = kenv->readObjRef<CKObject>(file);
			assert(aBlender == ogBlender);
		}
		ogBlendFloat = file->readFloat();
	}
}

void CAnimatedClone::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);

	if (kenv->version >= kenv->KVERSION_XXL2)
		if (unk1 & 1)
			file->writeUint32((uint32_t)x2someNum);

	if (kenv->version == kenv->KVERSION_XXL2 && kenv->isRemaster) {
		file->writeUint16(hdKifName.size());
		file->writeString(hdKifName);
	}

	kenv->writeObjRef(file, branchs);
	file->writeUint32(cloneInfo);

	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		file->writeUint16(hdKifName.size());
		file->writeString(hdKifName);
	}

	if (kenv->isRemaster) {
		file->writeUint32(hdBoneNames.size());
		for (const auto &str : hdBoneNames) {
			file->writeUint16(str.size());
			file->writeString(str);
		}
	}

	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		kenv->writeObjRef(file, ogBlender);
		file->write(ogBlendBytes.data(), ogBlendBytes.size());
		if ((this->unk1 & 0x600) == 0x400) {
			kenv->writeObjRef(file, ogBlender);
		}
		file->writeFloat(ogBlendFloat);
	}
}

void CKBoundingShape::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CSGLeaf::deserialize(kenv, file, length);
	unk1 = file->readUint16();
	unk2 = file->readUint16();
	radius = file->readFloat();
	object = kenv->readObjRef<CKObject>(file);
}

void CKBoundingShape::serialize(KEnvironment * kenv, File * file)
{
	CSGLeaf::serialize(kenv, file);
	file->writeUint16(unk1);
	file->writeUint16(unk2);
	file->writeFloat(radius);
	kenv->writeObjRef(file, object);
}

void CKDynBSphereProjectile::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CSGLeaf::deserialize(kenv, file, length);
	unk1 = file->readUint16();
	unk2 = file->readUint16();
	radius = file->readFloat();
	cameraService = kenv->readObjRef<CKObject>(file);
}

void CKDynBSphereProjectile::serialize(KEnvironment * kenv, File * file)
{
	CSGLeaf::serialize(kenv, file);
	file->writeUint16(unk1);
	file->writeUint16(unk2);
	file->writeFloat(radius);
	kenv->writeObjRef(file, cameraService);
}

void CKBoundingBox::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKBoundingShape::deserialize(kenv, file, length);
	for (float &f : boxSize)
		f = file->readFloat();
}

void CKBoundingBox::serialize(KEnvironment * kenv, File * file)
{
	CKBoundingShape::serialize(kenv, file);
	for (float &f : boxSize)
		file->writeFloat(f);
}

void CKAACylinder::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CKBoundingShape::deserialize(kenv, file, length);
	cylinderRadius = file->readFloat();
	cylinderHeight = file->readFloat();
}

void CKAACylinder::serialize(KEnvironment * kenv, File * file)
{
	CKBoundingShape::serialize(kenv, file);
	file->writeFloat(cylinderRadius);
	file->writeFloat(cylinderHeight);
}

void CTrailNodeFx::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNodeFx::deserialize(kenv, file, length);
	if (kenv->version < kenv->KVERSION_XXL2) {
		tnUnk1 = file->readUint32();
		tnUnk2 = file->readUint32();
		tnUnk3 = file->readUint32();
	}
	uint32_t numParts = file->readUint32();
	parts.resize(numParts);
	for (TrailPart &part : parts) {
		part.unk1 = file->readUint8();
		part.unk2 = file->readUint8();
		part.unk3 = file->readUint32();
		if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
			continue;
		}
		part.branch1 = kenv->readObjRef<CKSceneNode>(file);
		part.branch2 = kenv->readObjRef<CKSceneNode>(file);
		if (kenv->version >= kenv->KVERSION_XXL2) {
			part.tnUnk2 = file->readUint32();
			part.tnUnk3 = file->readUint32();
		}
	}

	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		romFxName = file->readString(file->readUint16());
		romTexName = file->readString(file->readUint16());
		romUnkByte = file->readUint8();
	}
}

void CTrailNodeFx::serialize(KEnvironment * kenv, File * file)
{
	CNodeFx::serialize(kenv, file);
	if (kenv->version < kenv->KVERSION_XXL2) {
		file->writeUint32(tnUnk1);
		file->writeUint32(tnUnk2);
		file->writeUint32(tnUnk3);
	}
	file->writeUint32(parts.size());
	for (TrailPart &part : parts) {
		file->writeUint8(part.unk1);
		file->writeUint8(part.unk2);
		file->writeUint32(part.unk3);
		if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
			continue;
		}
		kenv->writeObjRef(file, part.branch1);
		kenv->writeObjRef(file, part.branch2);
		if (kenv->version >= kenv->KVERSION_XXL2) {
			file->writeUint32(part.tnUnk2);
			file->writeUint32(part.tnUnk3);
		}
	}

	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		file->writeUint16(romFxName.size());
		file->writeString(romFxName);
		file->writeUint16(romTexName.size());
		file->writeString(romTexName);
		file->writeUint8(romUnkByte);
	}
}

void CNodeFx::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		fxUnkByte = file->readUint8();
	if (kenv->isUsingNewFilenames())
		fxUnkByte2 = file->readUint8();
	if (kenv->version >= KEnvironment::KVERSION_SPYRO)
		fxUnkByte3 = file->readUint8();
}

void CNodeFx::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		file->writeUint8(fxUnkByte);
	if (kenv->isUsingNewFilenames())
		file->writeUint8(fxUnkByte2);
	if (kenv->version >= KEnvironment::KVERSION_SPYRO)
		file->writeUint8(fxUnkByte3);
}

void CFogBoxNodeFx::reflectFog(MemberListener & r, KEnvironment * kenv)
{
#define RREFLECT(w) r.reflect(w, #w);
	RREFLECT(fogUnk01);
	r.reflectSize<uint32_t>(matrices, "cnt_matrices");
	RREFLECT(matrices);
	RREFLECT(fogUnk02);
	RREFLECT(fogType);
	RREFLECT(fogUnk04);
	RREFLECT(fogUnk05);
	RREFLECT(fogUnk06);
	RREFLECT(fogUnk07);
	RREFLECT(fogUnk08);
	RREFLECT(fogUnk09);
	auto &fogUnk8_9 = (kenv->version >= kenv->KVERSION_ARTHUR) ? fogUnk08 : fogUnk09; // olympic seems to use unk08 for the next vector sizes but XXL/XXL2 use unk09
	RREFLECT(fogUnk10);
	RREFLECT(fogUnk11);
	RREFLECT(fogUnk12);
	r.reflectSize<uint32_t>(coords, "cnt_coords");
	RREFLECT(coords);
	if (kenv->version >= kenv->KVERSION_OLYMPIC) { // TODO: arthur?
		x2coordStuff.resize(coords.size() / 2);
		RREFLECT(x2coordStuff);
	}
	RREFLECT(fogUnk13);
	RREFLECT(fogUnk14);
	RREFLECT(fogUnk15);
	r.reflectSize<uint32_t>(fogDings, "cnt_fogDings");
	RREFLECT(fogDings);
	if (fogType != 1) {
		fogVec3.resize(fogUnk8_9);
		RREFLECT(fogVec3);
	}
	RREFLECT(fogUnk16);
	RREFLECT(fogUnk17);
	if (kenv->version >= kenv->KVERSION_XXL2) {
		RREFLECT(fogX2Unk1);
		RREFLECT(fogX2Unk2);
		RREFLECT(fogX2Unk3);
		RREFLECT(fogX2Unk4);
	}
	if (fogType == 1) {
		fogVecA.resize(fogUnk8_9);
		fogVecB.resize(fogUnk8_9);
		RREFLECT(fogVecA);
		RREFLECT(fogVecB);
	}
	RREFLECT(fogUnk18);
	RREFLECT(fogUnk19);
	RREFLECT(fogUnk20);
	RREFLECT(fogUnk21);
	RREFLECT(fogUnk22);
	if (kenv->version >= kenv->KVERSION_ARTHUR) {	// TODO: arthur?
		RREFLECT(fogOgUnk1);
		RREFLECT(fogOgUnk2);
		RREFLECT(fogOgUnk3);
		RREFLECT(fogOgUnk4);
	}
	RREFLECT(fogUnk23);
	RREFLECT(fogUnk24);
	if (kenv->version == kenv->KVERSION_XXL1 && kenv->isRemaster) {
		RREFLECT(fogRomaName);
	}
	if (kenv->version >= kenv->KVERSION_ARTHUR) {	// TODO: arthur?
		RREFLECT(fogOgUnk5);
		RREFLECT(fogOgUnk6);
		RREFLECT(fogOgUnk7);
		if (kenv->version >= kenv->KVERSION_OLYMPIC) {
			fogOgUnk8.resize(fogUnk09);
			RREFLECT(fogOgUnk8);
		}
	}
}

void CFogBoxNodeFx::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNodeFx::deserialize(kenv, file, length);
	ReadingMemberListener rml(file, kenv);
	reflectFog(rml, kenv);
}

void CFogBoxNodeFx::serialize(KEnvironment * kenv, File * file)
{
	CNodeFx::serialize(kenv, file);
	WritingMemberListener wml(file, kenv);
	reflectFog(wml, kenv);
}

void CFogBoxNodeFx::Ding::reflectMembers(MemberListener & r)
{
	r.reflect(flt1, "flt1");
	r.reflect(flt2, "flt2");
	r.reflect(color1, "color1");
	r.reflect(color2, "color2");
}

void CSGLight::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CSGLeaf::deserialize(kenv, file, length);
	sglUnk0 = file->readInt32();
	lightComponents.resize(file->readInt32());
	for (auto& ref : lightComponents)
		ref = kenv->readObjRef<CKObject>(file);
	bbox.deserialize(file);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		lightSets.resize(file->readUint32());
	else
		lightSets.resize(16);
	for (auto& ref : lightSets)
		ref = kenv->readObjRef<CKObject>(file);
	sglUnkLast = file->readInt32();
}

void CSGLight::serialize(KEnvironment* kenv, File* file)
{
	CSGLeaf::serialize(kenv, file);
	file->writeInt32(sglUnk0);
	file->writeUint32((uint32_t)lightComponents.size());
	for (auto& ref : lightComponents)
		kenv->writeObjRef(file, ref);
	bbox.serialize(file);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		file->writeUint32((uint32_t)lightSets.size());
	for (auto& ref : lightSets)
		kenv->writeObjRef(file, ref);
	file->writeInt32(sglUnkLast);
}

void CCloudsNodeFx::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CNodeFx::deserialize(kenv, file, length);
	cloudUnk1 = file->readFloat();
	cloudUnk2 = file->readFloat();
	for (auto& flt : cloudUnk3)
		flt = file->readFloat();
	for (auto& val : cloudUnk4)
		val = file->readUint32();
	for (auto& val : cloudColors)
		val = file->readUint32();
	particleGeo = kenv->readObjRef<CKParticleGeometry>(file);
}

void CCloudsNodeFx::serialize(KEnvironment* kenv, File* file)
{
	CNodeFx::serialize(kenv, file);
	file->writeFloat(cloudUnk1);
	file->writeFloat(cloudUnk2);
	for (auto& flt : cloudUnk3)
		file->writeFloat(flt);
	for (auto& val : cloudUnk4)
		file->writeUint32(val);
	for (auto& val : cloudColors)
		file->writeUint32(val);
	kenv->writeObjRef(file, particleGeo);
}

void CZoneNode::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		ogByte = file->readUint8();
	if (kenv->version == KEnvironment::KVERSION_ARTHUR) {
		arNumTypeA = file->readUint32();
		arNumTypeB = file->readUint32();
		zoneGeos.resize(arNumTypeA + arNumTypeB);
	}
	else {
		zoneGeos.resize(file->readUint32());
	}
	for (auto& ref : zoneGeos)
		ref = kenv->readObjRef<CMultiGeometryBasic>(file);
	if (kenv->version == KEnvironment::KVERSION_ARTHUR)
		ogByte = file->readUint8();
}

void CZoneNode::serialize(KEnvironment* kenv, File* file)
{
	CNode::serialize(kenv, file);
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		file->writeUint8(ogByte);
	if (kenv->version == KEnvironment::KVERSION_ARTHUR) {
		file->writeUint32(arNumTypeA);
		file->writeUint32(arNumTypeB);
		assert((uint32_t)zoneGeos.size() == arNumTypeA + arNumTypeB);
	}
	else {
		file->writeUint32((uint32_t)zoneGeos.size());
	}
	for (auto& ref : zoneGeos)
		kenv->writeObjRef(file, ref);
	if (kenv->version == KEnvironment::KVERSION_ARTHUR)
		file->writeUint8(ogByte);
}

void CSpawnNode::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	spawnPool = kenv->readObjRef<CKObject>(file);
	sector = file->readInt32();
	for (float& c : spawnVec)
		c = file->readFloat();
	spawnUnk2 = file->readFloat();
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		for (float& c : ogSpawnVec2)
			c = file->readFloat();
		ogSpawnUnk2 = file->readFloat();
		ogBBox.deserialize(file);
	}
}

void CSpawnNode::serialize(KEnvironment* kenv, File* file)
{
	CNode::serialize(kenv, file);
	kenv->writeObjRef(file, spawnPool);
	file->writeInt32(sector);
	for (float& c : spawnVec)
		file->writeFloat(c);
	file->writeFloat(spawnUnk2);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		for (float& c : ogSpawnVec2)
			file->writeFloat(c);
		file->writeFloat(ogSpawnUnk2);
		ogBBox.serialize(file);
	}
}

void CSGAnchor::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	CSGLeaf::deserialize(kenv, file, length);
	cameraBeacon = kenv->readObjRef<CKObject>(file);
}

void CSGAnchor::serialize(KEnvironment* kenv, File* file)
{
	CSGLeaf::serialize(kenv, file);
	kenv->writeObjRef(file, cameraBeacon);
}
