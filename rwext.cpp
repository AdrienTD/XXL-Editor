#include "rwext.h"
#include "rw.h"
#include "File.h"

void RwExtUnknown::deserialize(File * file, const RwsHeader &header)
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

void RwExtHAnim::deserialize(File * file, const RwsHeader &header)
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
	return nullptr;
}

RwExtension * RwExtCreate(uint32_t type)
{
	RwExtension *ext;
	switch (type) {
	case 0x11E:
		ext = new RwExtHAnim(); break;
	default:
		ext = new RwExtUnknown(); break;
	}
	return ext;
}
