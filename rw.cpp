#include "rw.h"
#include <cassert>

void RwFrame::deserialize(File * file)
{
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 3; c++)
			matrix[r][c] = file->readFloat();
	index = file->readUint32();
	flags = file->readUint32();
}

uint32_t rwCheckHeader(File *file, uint32_t type)
{
	uint32_t h_type = file->readUint32();
	uint32_t h_size = file->readUint32();
	uint32_t h_ver = file->readUint32();
	assert(h_type == type);
	return h_size;
}

void RwFrameList::deserialize(File * file)
{
	rwCheckHeader(file, 0xE);
	uint32_t numFrames = file->readUint32();
	frames.resize(numFrames);
	for (RwFrame &frame : frames) {
		frame.deserialize(file);
	}
}

void RwGeometry::deserialize(File * file)
{
	rwCheckHeader(file, 0xF);
	flags = file->readUint32();
	numTris = file->readUint32();
	numVerts = file->readUint32();
	numMorphs = file->readUint32();
	if (flags & 0x01000000) {
		if (flags & 8) {
			colors.reserve(numVerts);
			for (uint32_t i = 0; i < numTris; i++)
				colors.push_back(file->readUint32());
		}
		if (flags & 4) {
			texCoords.reserve(numVerts);
			for (uint32_t i = 0; i < numTris; i++)
				texCoords.emplace_back(file->readFloat(), file->readFloat());
		}
		tris.resize(numTris);
		for (Triangle &tri : tris) {
			tri.indices[0] = file->readUint16();
			tri.indices[1] = file->readUint16();
			tri.materialId = file->readUint16();
			tri.indices[2] = file->readUint16();
		}
	}
	assert(numMorphs == 1);
	spherePos.x = file->readFloat();
	spherePos.y = file->readFloat();
	spherePos.z = file->readFloat();
	sphereRadius = file->readFloat();
	hasVertices = file->readUint32();
	hasNormals = file->readUint32();
	if (hasVertices) {
		verts.resize(numVerts);
		for (Vector3 &v : verts) {
			v.x = file->readFloat();
			v.y = file->readFloat();
			v.z = file->readFloat();
		}
	}
	if (hasNormals) {
		for (Vector3 &n : norms) {
			n.x = file->readFloat();
			n.y = file->readFloat();
			n.z = file->readFloat();
		}
	}
}

void RwTexture::deserialize(File * file)
{
	rwCheckHeader(file, 6);
	filtering = file->readUint8();
	uint8_t addr = file->readUint8();
	uAddr = addr & 15;	// TODO: Check if uv or vu
	vAddr = addr >> 4;
	uint16_t rest = file->readUint16();
	usesMips = rest & 1;

	name = file->readString(rwCheckHeader(file, 2));
	alphaName = file->readString(rwCheckHeader(file, 2));
}
