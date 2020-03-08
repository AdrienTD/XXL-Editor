#include "rw.h"
#include "rwext.h"
#include <cassert>

RwsHeader rwReadHeader(File * file)
{
	uint32_t h_type = file->readUint32();
	uint32_t h_size = file->readUint32();
	uint32_t h_ver = file->readUint32();
	return RwsHeader(h_type, h_size, h_ver);
}

RwsHeader rwFindHeader(File * file, uint32_t type)
{
	RwsHeader head;
	while(true) {
		head = rwReadHeader(file);
		assert(head.rwVersion == 0x1803FFFF);
		if (head.type == type)
			break;
		file->seek(head.length, SEEK_CUR);
	}
	return RwsHeader();
}

uint32_t rwCheckHeader(File *file, uint32_t type)
{
	uint32_t h_type = file->readUint32();
	uint32_t h_size = file->readUint32();
	uint32_t h_ver = file->readUint32();
	assert(h_type == type);
	return h_size;
}

void rwCheckAndSkipHeader(File *file, uint32_t type)
{
	file->seek(rwCheckHeader(file, type), SEEK_CUR);
}

void rwWriteString(File * file, const std::string & str)
{
	uint32_t len = str.size() + 1;
	uint32_t pad = (len + 3) & ~3;
	file->writeUint32(2);
	file->writeUint32(pad);
	file->writeUint32(0x1803FFFF);
	if (str.empty()) {
		file->writeUint32(0xDDDDDD00);	// we need this for binary matching with original
	}
	else {
		file->write(str.c_str(), len);
		for (uint32_t i = 0; i < pad - len; i++)
			file->writeUint8(0);
	}
}

void RwsExtHolder::read(File * file) {
	RwsHeader ctrhead = rwReadHeader(file);
	assert(ctrhead.type == 3);
	uint32_t bytesRead = 0;
	while (bytesRead < ctrhead.length) {
		RwsHeader exthead = rwReadHeader(file);
		uint32_t startoff = file->tell();
		RwExtension *ext = RwExtCreate(exthead.type);
		ext->deserialize(file, exthead);
		assert(file->tell() == startoff + exthead.length);
		this->exts.push_back(ext);
		bytesRead += 12 + exthead.length;
	}
}

void RwsExtHolder::write(File * file)
{
	HeaderWriter ctrhead;
	ctrhead.begin(file, 3);
	for (RwExtension *ext : exts)
		ext->serialize(file);
	ctrhead.end(file);
}

void RwFrame::deserialize(File * file)
{
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 3; c++)
			matrix[r][c] = file->readFloat();
	index = file->readUint32();
	flags = file->readUint32();
}

void RwFrame::serialize(File * file)
{
	for (int r = 0; r < 4; r++)
		for (int c = 0; c < 3; c++)
			file->writeFloat(matrix[r][c]);
	file->writeUint32(index);
	file->writeUint32(flags);
}

void RwFrameList::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	uint32_t numFrames = file->readUint32();
	frames.resize(numFrames);
	for (RwFrame &frame : frames) {
		frame.deserialize(file);
	}
	extensions.resize(numFrames);
	for (RwsExtHolder &reh : extensions)
		reh.read(file);
}

void RwFrameList::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0xE);
	{
		head2.begin(file, 1);
		file->writeUint32(this->frames.size());
		for (RwFrame &frame : frames) {
			frame.serialize(file);
		}
		head2.end(file);
		for (RwsExtHolder &reh : extensions)
			reh.write(file);
	}
	head1.end(file);
}

void RwGeometry::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	flags = file->readUint32();
	numTris = file->readUint32();
	numVerts = file->readUint32();
	numMorphs = file->readUint32();
	//if (flags & 0x01000000) {
		if (flags & 8) {
			colors.reserve(numVerts);
			for (uint32_t i = 0; i < numVerts; i++)
				colors.push_back(file->readUint32());
		}

		uint32_t numSets = (flags >> 16) & 255;
		if (numSets == 0) {
			if (flags & 0x80) numSets = 2;
			else if (flags & 4) numSets = 1;
		}
		texSets.resize(numSets);
		for (auto &texCoords : texSets) {
			texCoords.reserve(numVerts);
			for (uint32_t i = 0; i < numVerts; i++) {
				std::array<float, 2> tc;
				for (float &c : tc)
					c = file->readFloat();
				texCoords.push_back(std::move(tc));
			}
		}

		tris.resize(numTris);
		for (Triangle &tri : tris) {
			tri.indices[0] = file->readUint16();
			tri.indices[1] = file->readUint16();
			tri.materialId = file->readUint16();
			tri.indices[2] = file->readUint16();
		}
	//}
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
		norms.resize(numVerts);
		for (Vector3 &n : norms) {
			n.x = file->readFloat();
			n.y = file->readFloat();
			n.z = file->readFloat();
		}
	}
	rwCheckHeader(file, 8);
	this->materialList.deserialize(file);

	extensions.read(file);
}

void RwGeometry::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0xF);
	{
		head2.begin(file, 1);
		{
			file->writeUint32(flags);
			file->writeUint32(numTris);
			file->writeUint32(numVerts);
			file->writeUint32(numMorphs);

			if (flags & 8) {
				for (uint32_t color : colors)
					file->writeUint32(color);
			}

			// TODO: check num of texture sets

			for (auto &texCoords : texSets)
				for (auto &tc : texCoords)
					for (float &c : tc)
						file->writeFloat(c);

			for (Triangle &tri : tris) {
				file->writeUint16(tri.indices[0]);
				file->writeUint16(tri.indices[1]);
				file->writeUint16(tri.materialId);
				file->writeUint16(tri.indices[2]);
			}

			file->writeFloat(spherePos.x);
			file->writeFloat(spherePos.y);
			file->writeFloat(spherePos.z);
			file->writeFloat(sphereRadius);
			file->writeUint32(hasVertices);
			file->writeUint32(hasNormals);

			if (hasVertices) {
				for (Vector3 &v : verts) {
					file->writeFloat(v.x);
					file->writeFloat(v.y);
					file->writeFloat(v.z);
				}
			}

			if (hasNormals) {
				for (Vector3 &n : norms) {
					file->writeFloat(n.x);
					file->writeFloat(n.y);
					file->writeFloat(n.z);
				}
			}
		}
		head2.end(file);
		materialList.serialize(file);
		extensions.write(file);
	}
	head1.end(file);
}

void RwTexture::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	filtering = file->readUint8();
	uint8_t addr = file->readUint8();
	uAddr = addr & 15;	// TODO: Check if uv or vu
	vAddr = addr >> 4;
	uint16_t rest = file->readUint16();
	usesMips = rest & 1;

	name = file->readString(rwCheckHeader(file, 2)).c_str();
	alphaName = file->readString(rwCheckHeader(file, 2)).c_str();

	extensions.read(file);
}

void RwTexture::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 6);
	{
		head2.begin(file, 1);
		{
			file->writeUint8(filtering);
			file->writeUint8(uAddr | (vAddr << 4));
			file->writeUint16(usesMips ? 1 : 0);
		}
		head2.end(file);
		rwWriteString(file, name);
		rwWriteString(file, alphaName);
		extensions.write(file);
	}
	head1.end(file);

}

void RwAtomic::deserialize(File * file, bool hasGeo)
{
	rwCheckHeader(file, 1);
	this->frameIndex = file->readUint32();
	this->geoIndex = file->readUint32();
	this->flags = file->readUint32();
	this->unused = file->readUint32();
	if (hasGeo) {
		rwCheckHeader(file, 0xF);
		RwGeometry *geo = new RwGeometry;
		geo->deserialize(file);
		this->geometry = std::unique_ptr<RwGeometry>(geo);
	}
	extensions.read(file);
}

void RwAtomic::serialize(File * file)
{
	HeaderWriter head;
	head.begin(file, 0x14);
	{
		HeaderWriter strhead;
		strhead.begin(file, 1);
		{
			file->writeUint32(this->frameIndex);
			file->writeUint32(this->geoIndex);
			file->writeUint32(this->flags);
			file->writeUint32(this->unused);
		}
		strhead.end(file);
		if (geometry) {
			geometry->serialize(file);
		}
		extensions.write(file);
	}
	head.end(file);
}

void RwMaterialList::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	uint32_t numSlots = file->readUint32();
	this->slots.reserve(numSlots);
	for (uint32_t i = 0; i < numSlots; i++)
		this->slots.push_back(file->readUint32());
	for (uint32_t x : this->slots) {
		if (x == -1) {
			RwMaterial *mat = new RwMaterial;
			rwCheckHeader(file, 7);
			mat->deserialize(file);
			this->materials.emplace_back(mat);
		}
	}
}

void RwMaterialList::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 8);
	{
		head2.begin(file, 1);
		{
			file->writeUint32(slots.size());
			for (uint32_t &slot : slots)
				file->writeUint32(slot);
		}
		head2.end(file);
		for (auto &matptr : materials)
			matptr->serialize(file);
	}
	head1.end(file);
}

void RwMaterial::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	this->flags = file->readUint32();
	this->color = file->readUint32();
	this->unused = file->readUint32();
	this->isTextured = file->readUint32();
	this->ambient = file->readFloat();
	this->specular = file->readFloat();
	this->diffuse = file->readFloat();
	if (this->isTextured) {
		texture = std::make_unique<RwTexture>();
		rwCheckHeader(file, 6);
		texture->deserialize(file);
	}
	extensions.read(file);
}

void RwMaterial::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 7);
	{
		head2.begin(file, 1);
		{
			file->writeUint32(flags);
			file->writeUint32(color);
			file->writeUint32(unused);
			file->writeUint32(isTextured);
			file->writeFloat(ambient);
			file->writeFloat(specular);
			file->writeFloat(diffuse);
		}
		head2.end(file);
		if (isTextured) {
			texture->serialize(file);
		}
		extensions.write(file);
	}
	head1.end(file);
}

void RwMiniClump::deserialize(File * file)
{
	RwsHeader rwhead = rwReadHeader(file);
	if (rwhead.type == 0xE) {
		this->frameList.deserialize(file);
		rwhead = rwReadHeader(file);
	}
	assert(rwhead.type == 0x14);
	this->atomic.deserialize(file);
}

void RwMiniClump::serialize(File * file)
{
	if (!this->frameList.frames.empty()) {
		this->frameList.serialize(file);
	}
	this->atomic.serialize(file);
}

void RwGeometryList::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	uint32_t numGeos = file->readUint32();
	this->geometries.reserve(numGeos);
	for (uint32_t i = 0; i < numGeos; i++) {
		rwCheckHeader(file, 0xF);
		RwGeometry *geo = new RwGeometry;
		geo->deserialize(file);
		this->geometries.push_back(geo);
	}
}

void RwGeometryList::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x1A);
	{
		head2.begin(file, 1);
		file->writeUint32(this->geometries.size());
		head2.end(file);
		for (RwGeometry *geo : this->geometries)
			geo->serialize(file);
	}
	head1.end(file);
}

void RwClump::deserialize(File * file)
{
	//rwCheckHeader(file, 0x10);
	rwCheckHeader(file, 1);
	uint32_t numAtomics = file->readUint32();
	file->readUint32();
	file->readUint32();
	rwCheckHeader(file, 0xE);
	frameList.deserialize(file);
	rwCheckHeader(file, 0x1A);
	geoList.deserialize(file);
	atomics.reserve(numAtomics);
	for (uint32_t a = 0; a < numAtomics; a++) {
		rwCheckHeader(file, 0x14);
		RwAtomic *atom = new RwAtomic;
		atom->deserialize(file, false);
		atomics.push_back(atom);
	}
	extensions.read(file);
}

void RwClump::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x10);
	head2.begin(file, 1);
	file->writeUint32(atomics.size());
	file->writeUint32(0);
	file->writeUint32(0);
	head2.end(file);
	frameList.serialize(file);
	geoList.serialize(file);
	for (RwAtomic *atom : atomics)
		atom->serialize(file);
	extensions.write(file);
	head1.end(file);
}

void RwTeam::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	_numDings = file->readUint32();
	_unk1 = file->readUint32();
	_dings.reserve(_numDings);
	for (uint32_t i = 0; i < _numDings; i++) {
		_dings.push_back(file->readUint32());
	}
	for (uint32_t i = 0; i < 16; i++) {
		Bing bing;
		bing._someNum = file->readUint32();
		if (bing._someNum != 0xFFFFFFFF) {
			bing._clump = std::make_unique<RwMiniClump>();
			bing._clump->deserialize(file);
		}
	}
}

void RwTeam::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x22);
	head2.begin(file, 1);

	head2.end(file);
	head1.end(file);
}
