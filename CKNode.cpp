#include "CKNode.h"
#include "File.h"
#include "KEnvironment.h"
#include "rw.h"

void CKSceneNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	for (int i = 0; i < 16; i++)
		this->transform.v[i] = file->readFloat();
	this->parent = kenv->readObjRef<CKSceneNode>(file, 0);
	this->unk1 = file->readUint16();
	this->unk2 = file->readUint8();
	this->next = kenv->readObjRef<CKSceneNode>(file, 0);
}

void CKSceneNode::serialize(KEnvironment * kenv, File * file)
{
	for (int i = 0; i < 16; i++)
		file->writeFloat(this->transform.v[i]);
	kenv->writeObjRef(file, this->parent);
	file->writeUint16(this->unk1);
	file->writeUint8(this->unk2);
	kenv->writeObjRef(file, this->next);
}

void CNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CSGBranch::deserialize(kenv, file, length);
	this->geometry = kenv->readObjRef<CKGeometry>(file);
}

void CNode::serialize(KEnvironment * kenv, File * file)
{
	CSGBranch::serialize(kenv, file);
	kenv->writeObjRef(file, this->geometry);
}

void CSGSectorRoot::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	this->texDictionary = kenv->readObjRef<CKObject>(file);
}

void CSGSectorRoot::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	kenv->writeObjRef(file, this->texDictionary);
}

void CSGBranch::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CSGLeaf::deserialize(kenv, file, length);
	this->child = kenv->readObjRef<CKSceneNode>(file, 0);
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

void CAnimatedNode::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	branchs = kenv->readObjRef<CSGBranch>(file);
	unkref = kenv->readObjRef<CKObject>(file);
	numBones = file->readUint32();
	rwCheckHeader(file, 0xE);
	frameList = new RwFrameList;
	frameList->deserialize(file);
}

void CAnimatedNode::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	kenv->writeObjRef(file, branchs);
	kenv->writeObjRef(file, unkref);
	file->writeUint32(numBones);
	frameList->serialize(file);
}

void CAnimatedClone::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	CNode::deserialize(kenv, file, length);
	branchs = kenv->readObjRef<CSGBranch>(file);
	cloneInfo = file->readUint32();
}

void CAnimatedClone::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	kenv->writeObjRef(file, branchs);
	file->writeUint32(cloneInfo);
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
	CNode::deserialize(kenv, file, length);
	unk1 = file->readUint32();
	unk2 = file->readUint32();
	unk3 = file->readUint32();
	uint32_t numParts = file->readUint32();
	parts.resize(numParts);
	for (TrailPart &part : parts) {
		part.unk1 = file->readUint8();
		part.unk2 = file->readUint8();
		part.unk3 = file->readUint32();
		part.branch1 = kenv->readObjRef<CKSceneNode>(file);
		part.branch2 = kenv->readObjRef<CKSceneNode>(file);
	}
}

void CTrailNodeFx::serialize(KEnvironment * kenv, File * file)
{
	CNode::serialize(kenv, file);
	file->writeUint32(unk1);
	file->writeUint32(unk2);
	file->writeUint32(unk3);
	file->writeUint32(parts.size());
	for (TrailPart &part : parts) {
		file->writeUint8(part.unk1);
		file->writeUint8(part.unk2);
		file->writeUint32(part.unk3);
		kenv->writeObjRef(file, part.branch1);
		kenv->writeObjRef(file, part.branch2);
	}
}
