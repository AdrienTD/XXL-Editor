#include "CKNode.h"
#include "File.h"
#include "KEnvironment.h"
#include "rw.h"

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
	if (kenv->version >= kenv->KVERSION_ARTHUR)
		this->ogUnkFloat = file->readFloat();
}

void CNode::serialize(KEnvironment * kenv, File * file)
{
	CSGBranch::serialize(kenv, file);
	kenv->writeObjRef(file, this->geometry);
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
	cloneInfo = file->readUint32();
}

void CClone::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	file->writeUint32(cloneInfo);
}

CAnimatedNode::~CAnimatedNode() { delete frameList; }

void CAnimatedNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	branchs = kenv->readObjRef<CSGBranch>(file);
	if (kenv->version < kenv->KVERSION_XXL2)
		unkref = kenv->readObjRef<CKObject>(file);
	if (kenv->version >= kenv->KVERSION_XXL2)
		if (unk1 & 1)
			x2someNum = (int32_t)file->readUint32();

	numBones = file->readUint32();
	rwCheckHeader(file, 0xE);
	frameList = new RwFrameList;
	frameList->deserialize(file);

	if (kenv->version >= kenv->KVERSION_OLYMPIC) { // arthur?
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

	if (kenv->version >= kenv->KVERSION_OLYMPIC) { // arthur?
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

	branchs = kenv->readObjRef<CSGBranch>(file);
	cloneInfo = file->readUint32();

	if (kenv->version >= kenv->KVERSION_OLYMPIC) { // arthur?
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

	kenv->writeObjRef(file, branchs);
	file->writeUint32(cloneInfo);

	if (kenv->version >= kenv->KVERSION_OLYMPIC) { // arthur?
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
		part.branch1 = kenv->readObjRef<CKSceneNode>(file);
		part.branch2 = kenv->readObjRef<CKSceneNode>(file);
		if (kenv->version >= kenv->KVERSION_XXL2) {
			part.tnUnk2 = file->readUint32();
			part.tnUnk3 = file->readUint32();
		}
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
		kenv->writeObjRef(file, part.branch1);
		kenv->writeObjRef(file, part.branch2);
		if (kenv->version >= kenv->KVERSION_XXL2) {
			file->writeUint32(part.tnUnk2);
			file->writeUint32(part.tnUnk3);
		}
	}
}

void CNodeFx::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	if (kenv->version >= kenv->KVERSION_OLYMPIC) // arthur?
		fxUnkByte = file->readUint8();
}

void CNodeFx::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	if (kenv->version >= kenv->KVERSION_OLYMPIC) // arthur?
		file->writeUint8(fxUnkByte);
}
