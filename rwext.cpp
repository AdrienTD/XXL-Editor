#include "rwext.h"
#include "rw.h"
#include "File.h"
#include <cassert>
#include <set>

uint32_t RwExtUnknown::getType()
{
	return _type;
}

void RwExtUnknown::deserialize(File * file, const RwsHeader &header, void *parent)
{
	_type = header.type;
	_length = header.length;
	if (_length) {
		_ptr = malloc(_length);
		file->read(_ptr, _length);
	}
}

void RwExtUnknown::serialize(File * file)
{
	file->writeUint32(_type);
	file->writeUint32(_length);
	file->writeUint32(0x1803FFFF);
	if(_length)
		file->write(_ptr, _length);
}

RwExtension * RwExtUnknown::clone()
{
	RwExtUnknown *d = new RwExtUnknown();
	d->_type = _type;
	d->_length = _length;
	if (_length) {
		d->_ptr = malloc(_length);
		memcpy(d->_ptr, _ptr, _length);
	}
	return d;
}

uint32_t RwExtHAnim::getType()
{
	return 0x11E;
}

void RwExtHAnim::deserialize(File * file, const RwsHeader &header, void *parent)
{
	version = file->readUint32();
	nodeId = file->readUint32();
	uint32_t numNodes = file->readUint32();
	if (numNodes) {
		flags = file->readUint32();
		keyFrameSize = file->readUint32();
		bones.resize(numNodes);
		for (Bone &bone : bones) {
			bone.nodeId = file->readUint32();
			bone.nodeIndex = file->readUint32();
			bone.flags = file->readUint32();
		}
	}
	else
		bones.clear();
}

void RwExtHAnim::serialize(File * file)
{
	HeaderWriter head1;
	head1.begin(file, 0x11E);
	file->writeUint32(version);
	file->writeUint32(nodeId);
	file->writeUint32(bones.size());
	if (!bones.empty()) {
		file->writeUint32(flags);
		file->writeUint32(keyFrameSize);
		for (Bone &bone : bones) {
			file->writeUint32(bone.nodeId);
			file->writeUint32(bone.nodeIndex);
			file->writeUint32(bone.flags);
		}
	}
	head1.end(file);
}

RwExtension * RwExtHAnim::clone()
{
	RwExtHAnim *d = new RwExtHAnim;
	d->version = version;
	d->nodeId = nodeId;
	d->flags = flags;
	d->keyFrameSize = keyFrameSize;
	d->bones = bones;
	return d;
}

RwExtension * RwExtCreate(uint32_t type)
{
	RwExtension *ext;
	switch (type) {
	case 0x116:
		ext = new RwExtSkin(); break;
	case 0x11E:
		ext = new RwExtHAnim(); break;
	default:
		ext = new RwExtUnknown(); break;
	}
	return ext;
}

uint32_t RwExtSkin::getType()
{
	return 0x116;
}

void RwExtSkin::deserialize(File * file, const RwsHeader & header, void *parent)
{
	numBones = file->readUint8();
	numUsedBones = file->readUint8();
	maxWeightPerVertex = file->readUint8();
	file->readUint8();
	usedBones.reserve(numUsedBones);
	for (int i = 0; i < numUsedBones; i++)
		usedBones.push_back(file->readUint8());
	RwGeometry *geo = (RwGeometry*)parent;
	vertexIndices.resize(geo->numVerts);
	vertexWeights.resize(geo->numVerts);
	for (auto &viarr : vertexIndices)
		for (auto &ix : viarr)
			ix = file->readUint8();
	for (auto &warr : vertexWeights)
		for (auto &f : warr)
			f = file->readFloat();
	matrices.resize(numBones);
	for (Matrix &mat : matrices)
		for (int i = 0; i < 16; i++)
			mat.v[i] = file->readFloat();
	boneLimit = file->readUint32();
	numMeshes = file->readUint32();
	numRLE = file->readUint32();
	assert(boneLimit == numMeshes == numRLE == 0);
}

void RwExtSkin::serialize(File * file)
{
	HeaderWriter head1;
	head1.begin(file, 0x116);
	file->writeUint8(numBones);
	file->writeUint8(usedBones.size());
	file->writeUint8(maxWeightPerVertex);
	file->writeUint8(0);
	for (uint8_t ub : usedBones)
		file->writeUint8(ub);
	for (auto &viarr : vertexIndices)
		for (auto &ix : viarr)
			file->writeUint8(ix);
	for (auto &warr : vertexWeights)
		for (auto &f : warr)
			file->writeFloat(f);
	for (Matrix &mat : matrices)
		for (int i = 0; i < 16; i++)
			file->writeFloat(mat.v[i]);
	file->writeUint32(boneLimit);
	file->writeUint32(numMeshes);
	file->writeUint32(numRLE);
	head1.end(file);
}

RwExtension * RwExtSkin::clone()
{
	return new RwExtSkin(*this);
}

void RwExtSkin::merge(const RwExtSkin & other)
{
	assert(numBones == other.numBones);
	maxWeightPerVertex = std::max(maxWeightPerVertex, other.maxWeightPerVertex);

	std::set<uint8_t> ubset;
	for (uint8_t ub : usedBones)
		ubset.insert(ub);
	for (uint8_t ub : other.usedBones)
		ubset.insert(ub);
	usedBones = std::vector<uint8_t>(ubset.begin(), ubset.end());
	numUsedBones = usedBones.size();

	for (auto &vi : other.vertexIndices)
		vertexIndices.push_back(vi);
	for (auto &vw : other.vertexWeights)
		vertexWeights.push_back(vw);

	for (int i = 0; i < numBones; i++)
		if (std::find(other.usedBones.begin(), other.usedBones.end(), i) != other.usedBones.end())
			matrices[i] = other.matrices[i];
}
