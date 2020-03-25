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
