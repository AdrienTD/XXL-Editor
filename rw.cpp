#include "rw.h"
#include "rwext.h"
#include <cassert>
#include <stb_image.h>
#include <map>

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

void RwsExtHolder::read(File * file, void *parent) {
	RwsHeader ctrhead = rwReadHeader(file);
	assert(ctrhead.type == 3);
	uint32_t bytesRead = 0;
	while (bytesRead < ctrhead.length) {
		RwsHeader exthead = rwReadHeader(file);
		uint32_t startoff = file->tell();
		RwExtension *ext = RwExtCreate(exthead.type);
		ext->deserialize(file, exthead, parent);
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

RwExtension * RwsExtHolder::find(uint32_t type)
{
	for (RwExtension *ext : exts)
		if (ext->getType() == type)
			return ext;
	return nullptr;
}

const RwExtension * RwsExtHolder::find(uint32_t type) const
{
	for (RwExtension *ext : exts)
		if (ext->getType() == type)
			return ext;
	return nullptr;
}

RwsExtHolder::RwsExtHolder(const RwsExtHolder & orig)
{
	exts.reserve(orig.exts.size());
	for (RwExtension* ext : orig.exts)
		exts.push_back(ext->clone());
}

void RwsExtHolder::operator=(const RwsExtHolder & orig)
{
	exts.clear();
	exts.reserve(orig.exts.size());
	for (RwExtension* ext : orig.exts)
		exts.push_back(ext->clone());
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
		reh.read(file, this);
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

	extensions.read(file, this);
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

void RwGeometry::merge(const RwGeometry & other)
{
	assert(numMorphs == other.numMorphs == 1);
	printf("norms: %i %i", norms.size(), other.norms.size());
	assert((flags|0x11) == (other.flags|0x11));
	assert(hasVertices == other.hasVertices);
	//assert(hasNormals == other.hasNormals);
	assert(texSets.size() == other.texSets.size());

	uint16_t newmatstart = materialList.slots.size();
	uint16_t newtristart = verts.size();
	for (const Triangle &tri : other.tris) {
		Triangle newtri = tri;
		for (uint16_t &ix : newtri.indices)
			ix += newtristart;
		newtri.materialId += newmatstart;
		tris.push_back(std::move(newtri));
	}

	numTris += other.numTris;
	numVerts += other.numVerts;
	if ((flags & 0x10) ^ (other.flags & 0x10)) {
		if ((flags & 0x10) == 0) {
			for (int i = 0; i < verts.size(); i++)
				norms.push_back(Vector3(0, 0, 0));
		}
		if ((other.flags & 0x10) == 0) {
			assert(other.norms.size() == 0);
			for (int i = 0; i < other.verts.size(); i++)
				norms.push_back(Vector3(0, 0, 0));
		}
	}
	for (auto &vert : other.verts)
		verts.push_back(vert);
	for (auto &norm : other.norms)
		norms.push_back(norm);
	for (auto &color : other.colors)
		colors.push_back(color);
	for (size_t i = 0; i < texSets.size(); i++)
		for (auto &coord : other.texSets[i])
			texSets[i].push_back(coord);

	if (spherePos == other.spherePos)
		sphereRadius = std::max(sphereRadius, other.sphereRadius);
	else {
		Vector3 n_t2o = (other.spherePos - spherePos).normal();
		Vector3 ext_t = spherePos - n_t2o * sphereRadius;
		Vector3 ext_o = other.spherePos + n_t2o * other.sphereRadius;
		if ((ext_o - spherePos).len3() <= sphereRadius);
		else if ((ext_t - other.spherePos).len3() <= other.sphereRadius) {
			spherePos = other.spherePos;
			sphereRadius = other.sphereRadius;
		}
		else {
			spherePos = (ext_t + ext_o) * 0.5f;
			sphereRadius = 0.5f * (ext_o - ext_t).len3();
		}
	}

	for (uint32_t slot : other.materialList.slots)
		materialList.slots.push_back((slot == 0xFFFFFFFF) ? slot : (slot + newmatstart));
	for (auto &mat : other.materialList.materials)
		materialList.materials.push_back(mat);

	RwsExtHolder newholder;
	if (RwExtension *ext = extensions.find(0x116)) {
		RwExtSkin *skin = (RwExtSkin*)ext->clone();
		skin->merge(*(RwExtSkin*)other.extensions.find(0x116));
		newholder.exts.push_back(skin);
	}
	extensions = std::move(newholder);
}

std::vector<std::unique_ptr<RwGeometry>> RwGeometry::splitByMaterial()
{
	std::vector<std::unique_ptr<RwGeometry>> geolist;
	std::vector<std::map<uint16_t, uint16_t>> ixmaps;
	
	size_t numMats = materialList.materials.size();
	RwExtSkin *oskin = (RwExtSkin*)extensions.find(0x116);

	for (size_t i = 0; i < numMats; i++) {
		std::unique_ptr<RwGeometry> sgeo(new RwGeometry);
		sgeo->flags = flags & ~RWGEOFLAG_TRISTRIP;
		sgeo->numVerts = 0;
		sgeo->numTris = 0;
		sgeo->numMorphs = numMorphs;
		sgeo->texSets.resize(texSets.size());
		sgeo->spherePos = spherePos;
		sgeo->sphereRadius = sphereRadius;
		sgeo->hasVertices = hasVertices;
		sgeo->hasNormals = hasNormals;
		sgeo->materialList.slots = { 0xFFFFFFFF };
		sgeo->materialList.materials.push_back(materialList.materials[i]);
		if (oskin) {
			RwExtSkin *nskin = new RwExtSkin;
			nskin->numBones = oskin->numBones;
			nskin->numUsedBones = oskin->numBones;
			nskin->usedBones = oskin->usedBones;
			nskin->maxWeightPerVertex = oskin->maxWeightPerVertex;
			nskin->matrices = oskin->matrices;
			nskin->isSplit = oskin->isSplit;
			nskin->boneLimit = nskin->numMeshes = nskin->numRLE = 0;
			sgeo->extensions.exts.push_back(nskin);
		}
		geolist.push_back(std::move(sgeo));
	}
	ixmaps.resize(numMats);
	for (Triangle &tri : tris) {
		uint16_t matid = tri.materialId;
		RwGeometry *sgeo = geolist[matid].get();
		std::map<uint16_t, uint16_t> &ixmap = ixmaps[matid];
		Triangle stri;
		stri.materialId = 0;
		for (int i = 0; i < 3; i++) {
			uint16_t oi = tri.indices[i];
			auto it = ixmap.find(oi);
			if (it != ixmap.end())
				stri.indices[i] = it->second;
			else {
				uint16_t ni = sgeo->numVerts++;
				stri.indices[i] = ni;
				ixmap[oi] = ni;
				if(flags & RWGEOFLAG_POSITIONS)
					sgeo->verts.push_back(verts[oi]);
				if(flags & RWGEOFLAG_NORMALS)
					sgeo->norms.push_back(norms[oi]);
				if(flags & RWGEOFLAG_PRELIT)
					sgeo->colors.push_back(colors[oi]);
				for (int ts = 0; ts < sgeo->texSets.size(); ts++)
					sgeo->texSets[ts].push_back(texSets[ts][oi]);
				if (oskin) {
					RwExtSkin *nskin = (RwExtSkin*)sgeo->extensions.exts[0];
					nskin->vertexIndices.push_back(oskin->vertexIndices[oi]);
					nskin->vertexWeights.push_back(oskin->vertexWeights[oi]);
				}
			}
		}
		sgeo->tris.push_back(stri);
		sgeo->numTris++;
	}
	return geolist;
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

	extensions.read(file, this);
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
	extensions.read(file, this);
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
			RwMaterial mat;
			rwCheckHeader(file, 7);
			mat.deserialize(file);
			this->materials.push_back(std::move(mat));
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
			matptr.serialize(file);
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
		rwCheckHeader(file, 6);
		texture.deserialize(file);
	}
	extensions.read(file, this);
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
			texture.serialize(file);
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
	extensions.read(file, this);
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

void RwTeamDictionary::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	_numDings = file->readUint32();
	_unk1 = file->readUint32();
	_dings.reserve(_numDings);
	for (uint32_t i = 0; i < _numDings; i++) {
		_dings.push_back(file->readUint32());
	}
	//uint32_t sum = 0, lol = 0;
	_bings.resize(_numDings);
	for (Bing &bing : _bings) {
		bing._someNum = file->readUint32();

		//printf("%i %i %i %i\n", i, bing._someNum, sum, lol);
		//lol += bing._someNum;
		//sum += bing._someNum;

		bing._clump = std::make_unique<RwMiniClump>();
		bing._clump->deserialize(file);
		if (bing._someNum == 1) {
			uint32_t ad = file->readUint32();
			assert(ad == 0xFFFFFFFF);
		}
	}
}

void RwTeamDictionary::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x22);
	head2.begin(file, 1);
	file->writeUint32(_numDings);
	file->writeUint32(_unk1);
	for (uint32_t u : _dings)
		file->writeUint32(u);
	for (Bing &bing : _bings) {
		file->writeUint32(bing._someNum);
		bing._clump->serialize(file);
		if (bing._someNum == 1)
			file->writeUint32(0xFFFFFFFF);
	}
	head2.end(file);
	head1.end(file);
}

void RwTeam::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	numBongs = file->readUint32();
	numDongs = file->readUint32();
	head2.deserialize(file);
	dongs.resize(numDongs);
	for (Dong &dong : dongs) {
		dong.head3.deserialize(file);
		dong.head4.deserialize(file);
		for (uint32_t i = 0; i < numBongs; i++) {
			dong.bongs.push_back(file->readUint32());
		}
		rwCheckHeader(file, 0x10);
		dong.clump.deserialize(file);
	}
	end.deserialize(file);
}

void RwTeam::serialize(File * file)
{
	HeaderWriter headw1, headw2;
	headw1.begin(file, 0x1C);
	headw2.begin(file, 1);
	file->writeUint32(numBongs);
	file->writeUint32(numDongs);
	head2.serialize(file);
	for (Dong &dong : dongs) {
		dong.head3.serialize(file);
		dong.head4.serialize(file);
		for (uint32_t bong : dong.bongs)
			file->writeUint32(bong);
		dong.clump.serialize(file);
	}
	end.serialize(file);
	headw2.end(file);
	headw1.end(file);
}

void RwImage::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	width = file->readUint32();
	height = file->readUint32();
	bpp = file->readUint32();
	pitch = file->readUint32();

	size_t totalSize = pitch * height;
	pixels.resize(totalSize);
	file->read(pixels.data(), totalSize);
	//file->seek(totalSize, SEEK_CUR);

	if (bpp <= 8) {
		size_t numPalEntries = 1 << bpp;
		palette.resize(numPalEntries);
		file->read(palette.data(), 4 * numPalEntries);
	}
}

void RwImage::serialize(File * file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x18);
	head2.begin(file, 1);
	file->writeUint32(width);
	file->writeUint32(height);
	file->writeUint32(bpp);
	file->writeUint32(pitch);
	head2.end(file);
	file->write(pixels.data(), pixels.size());
	if (bpp <= 8) {
		assert(palette.size() == (1 << bpp));
		file->write(palette.data(), 4 * palette.size());
	}
	head1.end(file);
}

RwImage RwImage::convertToRGBA32() const
{
	if (bpp == 32)
		return RwImage(*this);
	RwImage img;
	img.width = width;
	img.height = height;
	img.bpp = 32;
	img.pitch = width * 4;
	img.pixels.resize(img.pitch * img.height);
	assert(bpp <= 8);
	assert(width == pitch);
	uint8_t *oldpix = (uint8_t*)pixels.data();
	uint32_t *newpix = (uint32_t*)img.pixels.data();
	size_t oldsize = pitch * height;
	for (size_t i = 0; i < oldsize; i++)
		newpix[i] = palette[oldpix[i]];
	return img;
}

RwImage RwImage::loadFromFile(const char * filename)
{
	RwImage img;
	int sizx, sizy, origBpp;
	void *pix = stbi_load(filename, &sizx, &sizy, &origBpp, 4);
	assert(pix && "Failed to load image file\n");
	img.width = sizx;
	img.height = sizy;
	img.bpp = 32;
	img.pitch = img.width * 4;
	img.pixels.resize(img.pitch * img.height);
	memcpy(img.pixels.data(), pix, img.pixels.size());
	stbi_image_free(pix);
	return img;
}
