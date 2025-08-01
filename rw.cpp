#include "rw.h"
#include "rwext.h"
#include "File.h"
#include <cassert>
#include <stb_image.h>
#include <map>
#include <squish.h>
#include <algorithm>

struct DDS_PIXELFORMAT {
	uint32_t dwSize, dwFlags, dwFourCC, dwRGBBitCount, dwRBitMask, dwGBitMask, dwBBitMask, dwABitMask;
};
struct DDS_HEADER {
	uint32_t dwSize, dwFlags, dwHeight, dwWidth, dwLinearSize, dwDepth, dwMipMapCount;
	uint32_t dwReserved1[11];
	DDS_PIXELFORMAT ddpf;
	uint32_t dwCaps, dwCaps2, dwCaps3, dwCaps4;
	uint32_t dwReserved2;
};

static constexpr uint16_t byteswap16(uint16_t val) { return ((val & 255) << 8) | ((val >> 8) & 255); }
static constexpr uint32_t byteswap32(uint32_t val) { return ((val & 255) << 24) | (((val >> 8) & 255) << 16) | (((val >> 16) & 255) << 8) | ((val >> 24) & 255); }
static constexpr float byteswapFlt(float val) { auto b = byteswap32(*(uint32_t*)&val); return *(float*)&b; }

uint32_t HeaderWriter::rwver = 0x1803FFFF;

void HeaderWriter::begin(File* file, int type) {
	headpos = (uint32_t)file->tell();
	file->writeUint32(type);
	file->writeUint32(0);
	file->writeUint32(rwver);
}

void HeaderWriter::end(File* file) {
	uint32_t endpos = (uint32_t)file->tell();
	uint32_t length = endpos - headpos - 12;
	file->seek(headpos + 4, SEEK_SET);
	file->writeUint32(length);
	file->seek(endpos, SEEK_SET);
}

RwsHeader rwReadHeader(File * file)
{
	uint32_t h_type = file->readUint32();
	uint32_t h_size = file->readUint32();
	uint32_t h_ver = file->readUint32();
	HeaderWriter::rwver = h_ver; // very hacky :/
	return RwsHeader(h_type, h_size, h_ver);
}

void rwWriteHeader(File* file, uint32_t type, uint32_t length)
{
	file->writeUint32(type);
	file->writeUint32(length);
	file->writeUint32(HeaderWriter::rwver);
}

RwsHeader rwFindHeader(File * file, uint32_t type)
{
	RwsHeader head;
	while(true) {
		head = rwReadHeader(file);
		assert(head.rwVersion == HeaderWriter::rwver);
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
	HeaderWriter::rwver = h_ver; // very hacky :/
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
	file->writeUint32(HeaderWriter::rwver);
	if (str.empty()) {
		file->writeUint32(0xDDDDDD00);	// we need this for binary matching with original
	}
	else {
		file->write(str.c_str(), len);
		for (uint32_t i = 0; i < pad - len; i++)
			file->writeUint8(0);
	}
}

void RwsExtHolder::read(File * file, void *parent, uint32_t parentType) {
	RwsHeader ctrhead = rwReadHeader(file);
	assert(ctrhead.type == 3);
	uint32_t bytesRead = 0;
	while (bytesRead < ctrhead.length) {
		RwsHeader exthead = rwReadHeader(file);
		uint32_t startoff = file->tell();
		auto ext = RwExtCreate(exthead.type, parentType);
		ext->deserialize(file, exthead, parent);
		assert(file->tell() == startoff + exthead.length);
		this->exts.push_back(std::move(ext));
		bytesRead += 12 + exthead.length;
	}
}

void RwsExtHolder::write(File * file)
{
	HeaderWriter ctrhead;
	ctrhead.begin(file, 3);
	for (auto& ext : exts)
		ext->serialize(file);
	ctrhead.end(file);
}

RwExtension * RwsExtHolder::find(uint32_t type)
{
	for (auto& ext : exts)
		if (ext->getType() == type)
			return ext.get();
	return nullptr;
}

const RwExtension * RwsExtHolder::find(uint32_t type) const
{
	for (auto& ext : exts)
		if (ext->getType() == type)
			return ext.get();
	return nullptr;
}

RwsExtHolder::RwsExtHolder() = default;
RwsExtHolder::~RwsExtHolder() = default;

RwsExtHolder::RwsExtHolder(const RwsExtHolder & orig)
{
	exts.reserve(orig.exts.size());
	for (auto& ext : orig.exts)
		exts.push_back(ext->clone());
}

RwsExtHolder::RwsExtHolder(RwsExtHolder&& old) noexcept = default;

RwsExtHolder& RwsExtHolder::operator=(const RwsExtHolder & orig)
{
	exts.clear();
	exts.reserve(orig.exts.size());
	for (auto& ext : orig.exts)
		exts.push_back(ext->clone());
	return *this;
}

RwsExtHolder& RwsExtHolder::operator=(RwsExtHolder&& old) noexcept = default;

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
	if ((flags & RWGEOFLAG_NATIVE) == 0) {
		if (flags & RWGEOFLAG_PRELIT) {
			colors.reserve(numVerts);
			for (uint32_t i = 0; i < numVerts; i++)
				colors.push_back(file->readUint32());
		}

		uint32_t numSets = (flags >> 16) & 255;
		if (numSets == 0) {
			if (flags & RWGEOFLAG_TEXTURED2) numSets = 2;
			else if (flags & RWGEOFLAG_TEXTURED) numSets = 1;
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

	if (flags & RWGEOFLAG_NATIVE) {
		auto old = std::make_shared<RwGeometry>(std::move(*this));
		*this = old->convertToPI();
		this->nativeVersion = std::move(old);
	}
}

void RwGeometry::serialize(File * file)
{
	if (nativeVersion) {
		nativeVersion->serialize(file);
		return;
	}
	HeaderWriter head1, head2;
	head1.begin(file, 0xF);
	{
		head2.begin(file, 1);
		{
			if (texSets.size() > 2)
				printf("Writing a geometry with %zu texcoord sets!!!\n", texSets.size());
			uint32_t flagsToWrite;
			if (flags & RWGEOFLAG_NATIVE) {
				flagsToWrite = flags;
			}
			else {
				flagsToWrite = flags & ~uint32_t(0x00FF0000 | RWGEOFLAG_TEXTURED | RWGEOFLAG_TEXTURED2);
				flagsToWrite |= (uint32_t)texSets.size() << 16;
				if (texSets.size() == 1) flagsToWrite |= RWGEOFLAG_TEXTURED;
				if (texSets.size() == 2) flagsToWrite |= RWGEOFLAG_TEXTURED2;
			}

			file->writeUint32(flagsToWrite);
			file->writeUint32(numTris);
			file->writeUint32(numVerts);
			file->writeUint32(numMorphs);

			if ((flags & RWGEOFLAG_NATIVE) == 0) {
				if (flags & RWGEOFLAG_PRELIT) {
					for (uint32_t color : colors)
						file->writeUint32(color);
				}

				for (auto& texCoords : texSets)
					for (auto& tc : texCoords)
						for (float& c : tc)
							file->writeFloat(c);

				for (Triangle& tri : tris) {
					file->writeUint16(tri.indices[0]);
					file->writeUint16(tri.indices[1]);
					file->writeUint16(tri.materialId);
					file->writeUint16(tri.indices[2]);
				}
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
	//printf("norms: %i %i", norms.size(), other.norms.size());
	const uint32_t toleratedFlags = 0x00FF0000 | RWGEOFLAG_TRISTRIP | RWGEOFLAG_NORMALS | RWGEOFLAG_LIGHT | RWGEOFLAG_MODULATECOLOR
		| RWGEOFLAG_TEXTURED | RWGEOFLAG_TEXTURED2;
	assert((flags | toleratedFlags) == (other.flags | toleratedFlags));
	assert(hasVertices == other.hasVertices);

	uint16_t newmatstart = materialList.slots.size();
	uint16_t newtristart = verts.size();
	for (const Triangle &tri : other.tris) {
		Triangle newtri = tri;
		for (uint16_t &ix : newtri.indices)
			ix += newtristart;
		newtri.materialId += newmatstart;
		tris.push_back(std::move(newtri));
	}

	// update number of texture coordinate sets
	if (other.texSets.size() > texSets.size()) {
		size_t oldTexSetsCount = texSets.size();
		texSets.resize(other.texSets.size());
		for (size_t i = oldTexSetsCount; i < texSets.size(); ++i) {
			texSets[i].assign(numVerts, { 0.0f, 0.0f });
		}
	}

	numTris += other.numTris;
	numVerts += other.numVerts;
	if ((hasNormals != 0) ^ (other.hasNormals != 0)) {
		if (hasNormals == 0) {
			for (int i = 0; i < verts.size(); i++)
				norms.push_back(Vector3(0, 0, 0));
			hasNormals = 1;
			flags |= RWGEOFLAG_NORMALS;
		}
		if (other.hasNormals == 0) {
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
	for (size_t i = 0; i < texSets.size(); i++) {
		if (i < other.texSets.size()) {
			for (auto& coord : other.texSets[i])
				texSets[i].push_back(coord);
		}
		else {
			texSets[i].insert(texSets[i].end(), other.numVerts, {0.0f, 0.0f});
		}
	}

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
		auto skinUP = ext->clone();
		RwExtSkin* skin = (RwExtSkin*)skinUP.get();
		skin->merge(*(RwExtSkin*)other.extensions.find(0x116));
		newholder.exts.push_back(std::move(skinUP));
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
		std::unique_ptr<RwGeometry> sgeo = std::make_unique<RwGeometry>();
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
			auto nskin = std::make_unique<RwExtSkin>();
			nskin->numBones = oskin->numBones;
			nskin->numUsedBones = oskin->numBones;
			nskin->usedBones = oskin->usedBones;
			nskin->maxWeightPerVertex = oskin->maxWeightPerVertex;
			nskin->matrices = oskin->matrices;
			nskin->isSplit = oskin->isSplit;
			nskin->boneLimit = 0;
			sgeo->extensions.exts.push_back(std::move(nskin));
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
				if(hasNormals != 0)
					sgeo->norms.push_back(norms[oi]);
				if(flags & RWGEOFLAG_PRELIT)
					sgeo->colors.push_back(colors[oi]);
				for (int ts = 0; ts < sgeo->texSets.size(); ts++)
					sgeo->texSets[ts].push_back(texSets[ts][oi]);
				if (oskin) {
					RwExtSkin *nskin = (RwExtSkin*)sgeo->extensions.exts[0].get();
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

RwGeometry RwGeometry::convertToPI()
{
	assert(flags & RWGEOFLAG_NATIVE);
	RwExtNativeData* nat = (RwExtNativeData*)extensions.find(0x510);
	assert(nat);
	assert(nat->data.size() >= 4);
	uint32_t platform = *(uint32_t*)nat->data.data();
	switch (platform) {
	case 6:
		return convertToPI_GCN();
	case byteswap32(9):
		return convertToPI_X360();
	default: {
		// give an empty geometry for unsupported platforms
		RwGeometry empty;
		empty.flags = 0;
		empty.numTris = 0;
		empty.numVerts = 0;
		empty.numMorphs = 0;
		return empty;
	}
	}
	assert(false && "unknown platform for native RwGeometry");
	return {};
}

RwGeometry RwGeometry::convertToPI_GCN()
{
	RwGeometry* geo = this;

	RwGeometry cvt;
	cvt.flags = 0;
	cvt.numTris = 0;
	cvt.numVerts = 0;
	cvt.numMorphs = 1;
	cvt.texSets.resize(1);
	cvt.spherePos = geo->spherePos;
	cvt.sphereRadius = geo->sphereRadius;
	cvt.hasVertices = 0;
	cvt.hasNormals = 0;
	cvt.materialList = geo->materialList;
	//cvt.extensions = geo->extensions;

	RwExtNativeData* nat = (RwExtNativeData*)geo->extensions.find(0x510);
	assert(nat);

	bool hasSkinIndices = false;
	RwExtSkin* skin = (RwExtSkin*)geo->extensions.find(0x116);
	RwExtSkin* cvtSkin = nullptr;
	std::vector<uint8_t> cmpIndices;
	std::vector<uint8_t> cmpWeights;
	if (skin && *(uint32_t*)skin->nativeData.data() == 6) {
		hasSkinIndices = true;
		auto cvtUniquePtr = std::make_unique<RwExtSkin>();
		cvtSkin = cvtUniquePtr.get();
		cvt.extensions.exts.push_back(std::move(cvtUniquePtr));

		// On GC, skin native data looks almost like non-native one, except:
		//  - if numOfWeights == 1, vertex weights+indices are missing, 
		//    instead, vertex indices are stored in the geometry's native data (and weight is always 1.0)
		//  - else, only numOfWeights vertex indices are stored per vertex (no useless 0 when numOfWeights < 4)
		//      and numOfWeights vertex weights are stored per vertex, each being 8 bits, from 0x00 = 0.0 to 0x80 = 1.0
		//      sum of weights should be 0x80 = 1.0
		//  - matrices are byte-swapped
		MemFile data{ skin->nativeData.data() };
		uint32_t skinPlatform = data.readUint32();
		assert(skinPlatform == 6);
		cvtSkin->numBones = data.readUint8();
		cvtSkin->numUsedBones = data.readUint8();
		cvtSkin->maxWeightPerVertex = data.readUint8();
		data.readUint8();
		cvtSkin->usedBones.reserve(cvtSkin->numUsedBones);
		for (int i = 0; i < cvtSkin->numUsedBones; i++)
			cvtSkin->usedBones.push_back(data.readUint8());

		if (cvtSkin->maxWeightPerVertex >= 2) {
			cmpIndices.resize(geo->numVerts * cvtSkin->maxWeightPerVertex);
			cmpWeights.resize(geo->numVerts * cvtSkin->maxWeightPerVertex);
			data.read(cmpIndices.data(), cmpIndices.size());
			data.read(cmpWeights.data(), cmpWeights.size());
		}

		cvtSkin->matrices.resize(cvtSkin->numBones);
		for (Matrix& mat : cvtSkin->matrices) {
			if (cvtSkin->maxWeightPerVertex == 0)
				data.readUint32();
			for (int i = 0; i < 16; i++)
				mat.v[i] = byteswapFlt(data.readFloat());
		}
		cvtSkin->isSplit = cvtSkin->numUsedBones != 0; // TODO: Find a better way to detect the presence of split data
		if (cvtSkin->isSplit) {
			cvtSkin->boneLimit = data.readUint32();
			cvtSkin->boneGroups.resize(data.readUint32());
			cvtSkin->boneGroupIndices.resize(data.readUint32());
			assert(cvtSkin->maxWeightPerVertex < 2 || cvtSkin->boneLimit == 0);
			if (!cvtSkin->boneGroups.empty()) {
				cvtSkin->boneIndexRemap.resize(cvtSkin->numBones);
				data.read(cvtSkin->boneIndexRemap.data(), cvtSkin->boneIndexRemap.size());
				for (auto& grp : cvtSkin->boneGroups) {
					grp.first = data.readUint8();
					grp.second = data.readUint8();
				}
				for (auto& gi : cvtSkin->boneGroupIndices) {
					gi.first = data.readUint8();
					gi.second = data.readUint8();
				}
			}
		}
	}

	MemFile mf(nat->data.data());
	auto platform = mf.readUint32();
	auto headSize = mf.readUint32();
	auto gfxDataSize = mf.readUint32();
	auto headOffset = mf.tell();
	auto gfxDataOffset = headOffset + headSize;
	mf.readUint32();
	mf.readUint32();
	auto numAttribs = byteswap32(mf.readUint32());
	struct Attribute {
		uint32_t offset;
		uint8_t type;
		uint8_t stride;
		uint8_t indexType, unk2;
	};
	std::vector<Attribute> attribs(numAttribs);
	for (auto& atb : attribs) {
		atb.offset = byteswap32(mf.readUint32());
		atb.type = mf.readUint8();
		atb.stride = mf.readUint8();
		atb.indexType = mf.readUint8();
		atb.unk2 = mf.readUint8();
		switch (atb.type) {
		case 9: // POS
			cvt.flags |= RWGEOFLAG_POSITIONS;
			cvt.hasVertices = 1;
			break;
		case 10: // NRM
			cvt.flags |= RWGEOFLAG_NORMALS;
			cvt.hasNormals = 1;
			break;
		case 11: // CLR0
			cvt.flags |= RWGEOFLAG_PRELIT;
			break;
		case 13: // TEX0
			cvt.flags |= RWGEOFLAG_TEXTURED;
			break;
		}
	}
	int numMeshes = (headSize - (12 + 8 * numAttribs)) / 8;
	std::vector<std::pair<uint32_t, uint32_t>> meshes(numMeshes);
	for (auto& mesh : meshes) {
		mesh.first = byteswap32(mf.readUint32());
		mesh.second = byteswap32(mf.readUint32());
	}
	uint16_t vtxind = 0;
	int meshIndex = 0;
	assert(!hasSkinIndices || cvtSkin->boneLimit == 0 || cvtSkin->boneGroups.size() == (size_t)numMeshes);
	for (auto& mesh : meshes) {
		auto cmdOffset = mesh.first;
		auto cmdSize = mesh.second;
		uint8_t* cmdStart = nat->data.data() + gfxDataOffset + cmdOffset;
		uint8_t* cmd = cmdStart;
		while (cmd - cmdStart < cmdSize) {
			uint8_t prim = *cmd++;
			if (prim == 0)
				break;
			assert(prim == 0x90 || prim == 0x98);
			uint8_t unk = *cmd++;
			uint16_t count = *cmd++ | (unk << 8);

			cvt.numVerts += count;
			cvt.colors.insert(cvt.colors.end(), count, 0xFFFFFFFF);
			cvt.texSets[0].insert(cvt.texSets[0].end(), count, { 0.0f,0.0f });
			cvt.verts.insert(cvt.verts.end(), count, Vector3(0.0f, 0.0f, 0.0f));
			if (cvt.hasNormals)
				cvt.norms.insert(cvt.norms.end(), count, Vector3(0.0f, 0.0f, 0.0f));
			if (hasSkinIndices) {
				cvtSkin->vertexIndices.insert(cvtSkin->vertexIndices.end(), count, { 0, 0, 0, 0 });
				cvtSkin->vertexWeights.insert(cvtSkin->vertexWeights.end(), count, { 0.0f, 0.0f, 0.0f, 0.0f });
			}

			for (int i = 0; i < count; i++) {
				uint8_t skinByte = 0;
				if (hasSkinIndices && cvtSkin->maxWeightPerVertex == 1)
					skinByte = *cmd++;
				for (int a = 0; a < attribs.size(); a++) {
					auto& atb = attribs[a];
					uint16_t index = *cmd++;
					if (atb.indexType == 3)
						index = (index << 8) | *cmd++;
					uint8_t* gdptr = nat->data.data() + gfxDataOffset + atb.offset + atb.stride * index;
					if (atb.type == 9) { // POS
						float x = byteswapFlt(*(float*)(gdptr + 0));
						float y = byteswapFlt(*(float*)(gdptr + 4));
						float z = byteswapFlt(*(float*)(gdptr + 8));
						cvt.verts[vtxind] = Vector3(x, y, z);
						if (hasSkinIndices) {
							// vertex weight + indices
							std::array<uint8_t, 4> nativeBoneIndices = { 0,0,0,0 };
							std::array<float, 4> nativeBoneWeights = { 0.0f, 0.0f, 0.0f, 0.0f };
							if (cvtSkin->maxWeightPerVertex == 1) {
								assert(skinByte % 3 == 0);
								nativeBoneIndices[0] = skinByte / 3;
								nativeBoneWeights[0] = 1.0f;
							}
							else if (cvtSkin->maxWeightPerVertex >= 2) {
								for (int w = 0; w < cvtSkin->maxWeightPerVertex; ++w) {
									nativeBoneIndices[w] = cmpIndices[index * cvtSkin->maxWeightPerVertex + w];
									nativeBoneWeights[w] = (float)cmpWeights[index * cvtSkin->maxWeightPerVertex + w] / 128.0f;
								}
							}
							// apply bone index remapping
							if (!cvtSkin->boneGroups.empty()) {
								auto [remStart, remCount] = cvtSkin->boneGroups[meshIndex];
								for (int w = 0; w < cvtSkin->maxWeightPerVertex; ++w) {
									uint8_t& boneIndex = nativeBoneIndices[w];
									for (uint8_t rem = remStart; rem < remStart + remCount; ++rem) {
										auto [riStart, riCount] = cvtSkin->boneGroupIndices[rem];
										auto itStart = cvtSkin->boneIndexRemap.begin() + riStart;
										auto itEnd = cvtSkin->boneIndexRemap.begin() + riStart + riCount;
										auto it = std::find(itStart, itEnd, boneIndex);
										if (it != itEnd) {
											boneIndex = (uint8_t)(it - itStart) + riStart;
											break;
										}
									}
								}
							}
							cvtSkin->vertexIndices[vtxind] = nativeBoneIndices;
							cvtSkin->vertexWeights[vtxind] = nativeBoneWeights;
						}
					}
					else if (atb.type == 10) { // NRM
						float x = byteswapFlt(*(float*)(gdptr + 0));
						float y = byteswapFlt(*(float*)(gdptr + 4));
						float z = byteswapFlt(*(float*)(gdptr + 8));
						cvt.norms[vtxind] = Vector3(x, y, z);
					}
					else if (atb.type == 11) { // CLR0
						cvt.colors[vtxind] = *(uint32_t*)gdptr;
					}
					else if (atb.type == 13) { // TEX0
						float x = byteswapFlt(*(float*)(gdptr + 0));
						float y = byteswapFlt(*(float*)(gdptr + 4));
						cvt.texSets[0][vtxind] = { x,y };
					}

				}
				if (prim == 0x90) { // TRIANGLES
					if ((i % 3) == 2) {
						cvt.tris.push_back({ {vtxind, (uint16_t)(vtxind - 1), (uint16_t)(vtxind - 2)}, (uint16_t)0 });
					}
				}
				else if (prim == 0x98) { // TRIANGLESTRIP
					if (i >= 2) {
						std::array<uint16_t, 3> arr = { vtxind, (uint16_t)(vtxind - 1), (uint16_t)(vtxind - 2) };
						if (i & 1) std::swap(arr[0], arr[1]);
						cvt.tris.push_back({ arr, (uint16_t)0 });
					}
				}
				vtxind++;
			}

		}
		meshIndex++;
	}
	cvt.numTris = cvt.tris.size();
	if (hasSkinIndices) {
		// we just applied the remapping, now we can clear it
		cvtSkin->boneLimit = 0;
		auto clear_and_shrink = [](auto& vec) {vec.clear(); vec.shrink_to_fit(); };
		clear_and_shrink(cvtSkin->boneIndexRemap);
		clear_and_shrink(cvtSkin->boneGroupIndices);
		clear_and_shrink(cvtSkin->boneGroups);
	}
	return cvt;
}

RwGeometry RwGeometry::convertToPI_X360()
{
	RwGeometry* geo = this;

	RwGeometry cvt;
	cvt.flags = 0;
	cvt.numTris = 0;
	cvt.numVerts = 0;
	cvt.numMorphs = 1;
	cvt.texSets.resize(1);
	cvt.spherePos = geo->spherePos;
	cvt.sphereRadius = geo->sphereRadius;
	cvt.hasVertices = 0;
	cvt.hasNormals = 0;
	cvt.materialList = geo->materialList;
	cvt.extensions = geo->extensions;

	RwExtNativeData* nat = (RwExtNativeData*)geo->extensions.find(0x510);
	assert(nat);

	MemFile mf(nat->data.data());
	auto platform = byteswap32(mf.readUint32()); assert(platform == 9);
	auto headSize = byteswap32(mf.readUint32()); assert(headSize == 0x68);
	auto unk01 = byteswap32(mf.readUint32());
	auto unk02 = byteswap32(mf.readUint32()); assert(unk02 == 1);
	auto unk03 = byteswap32(mf.readUint32());
	auto primitiveType = byteswap32(mf.readUint32()); assert(primitiveType == 6 || primitiveType == 4);
	auto unk05 = byteswap32(mf.readUint32());
	auto unk06 = byteswap32(mf.readUint32()); assert(unk06 == 0);
	auto stride = byteswap32(mf.readUint32()); //assert(unk07 == 32);
	auto unk08 = byteswap32(mf.readUint32());
	auto unk09 = byteswap32(mf.readUint32()); assert(unk09 == 0);
	auto unk10 = byteswap32(mf.readUint32()); assert(unk10 == 0);
	auto unk11 = byteswap32(mf.readUint32()); assert(unk11 == 0);
	auto unk12 = byteswap32(mf.readUint32()); assert(unk12 == 0);
	auto unk13 = byteswap32(mf.readUint32()); assert(unk13 == 0);
	auto unk14 = byteswap32(mf.readUint32());
	auto numIndices = byteswap32(mf.readUint32());
	auto numVertices = byteswap32(mf.readUint32());
	auto numIndices2 = byteswap32(mf.readUint32()); assert(numIndices2 == numIndices);
	auto unk18 = byteswap32(mf.readUint32()); assert(unk18 == 0);
	auto unk19 = byteswap32(mf.readUint32()); assert(unk19 == 0);
	auto unk20 = byteswap32(mf.readUint32()); //assert(unk20 == 0);
	auto unk21 = byteswap32(mf.readUint32()); assert(unk21 == 0);
	auto unk22 = byteswap32(mf.readUint32()); assert(unk22 == 0);
	auto unk23 = byteswap32(mf.readUint32()); assert(unk23 == 0);
	auto numVertices2 = byteswap32(mf.readUint32()); //assert(numVertices2 == numVertices);
	auto unk25 = byteswap32(mf.readUint32()); assert(unk25 == 0);
	auto numElements = byteswap32(mf.readUint32());
	uint32_t expectedElements = (primitiveType == 6) ? (numIndices - 2) : (numIndices / 3);
	assert(numElements == expectedElements);
	cvt.numVerts = numVertices;

	auto numAttribs = byteswap32(mf.readUint32());
	struct Attribute {
		uint16_t stream;
		uint32_t offset;
		uint32_t format;
		uint16_t usage, aunk3;
	};
	std::vector<Attribute> attribs(numAttribs);
	for (auto& atb : attribs) {
		atb.stream = byteswap16(mf.readUint16());
		atb.offset = byteswap16(mf.readUint16());
		atb.format = byteswap32(mf.readUint32());
		atb.usage = byteswap16(mf.readUint16());
		atb.aunk3 = byteswap16(mf.readUint16());
		if (atb.stream != 255) {
			switch (atb.usage) {
			case 0: // D3DDECLUSAGE_POSITION
				cvt.flags |= RWGEOFLAG_POSITIONS;
				cvt.hasVertices = 1;
				cvt.verts.resize(numVertices);
				break;
			case 3: // D3DDECLUSAGE_NORMAL
				cvt.flags |= RWGEOFLAG_NORMALS;
				cvt.hasNormals = 1;
				cvt.norms.resize(numVertices);
				break;
			case 5: // D3DDECLUSAGE_TEXCOORD
				cvt.flags |= RWGEOFLAG_TEXTURED;
				cvt.texSets[0].resize(numVertices);
				break;
			case 10: // D3DDECLUSAGE_COLOR
				cvt.flags |= RWGEOFLAG_PRELIT;
				cvt.colors.assign(numVertices, 0xFFFFFFFF);
				break;
			}
		}
	}

	uint16_t* indexBuffer = (uint16_t*)mf._curptr;
	for (uint16_t t = 0; t < numElements; t++) {
		Triangle tri;
		if (primitiveType == 4) { // triangle list
			for (uint16_t c = 0; c < 3; c++)
				tri.indices[c] = byteswap16(indexBuffer[3*t + c]);
			std::swap(tri.indices[1], tri.indices[2]);
		}
		else if (primitiveType == 6) { // triangle strip
			for (uint16_t c = 0; c < 3; c++)
				tri.indices[c] = byteswap16(indexBuffer[t + c]);
			if ((t & 1) == 0) std::swap(tri.indices[1], tri.indices[2]);
		}
		tri.materialId = 0;
		cvt.tris.push_back(tri);
		cvt.numTris++;
	}

	mf.seek(numIndices * 2, SEEK_CUR);
	auto vUnk1 = byteswap32(mf.readUint32());
	auto vUnk2 = byteswap32(mf.readUint32()); assert(vUnk2 == 0);
	auto vStride = byteswap32(mf.readUint32()); assert(vStride == stride);
	auto vUnk4 = byteswap32(mf.readUint32());

	uint8_t* vertexBuffer = (uint8_t*)mf._curptr;

	auto fetchAttrib = [](uint32_t format, uint8_t* ptr)->std::array<float, 4> {
		float vec[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		switch (format & 63) {
		case 6:
			for (int c = 0; c < 4; c++)
				vec[c] = (float)((uint8_t*)ptr)[c] / 255.0f;
			break;
		case 25:
			if (format & 256)
				for (int c = 0; c < 2; c++)
					vec[c] = std::max((float)byteswap16(((int16_t*)ptr)[c]) / 32767.0f, -1.0f);
			else
				for (int c = 0; c < 2; c++)
					vec[c] = (float)byteswap16(((uint16_t*)ptr)[c]) / 65535.0f;
			break;
		case 37:
			for (int c = 0; c < 2; c++)
				vec[c] = byteswapFlt(((float*)ptr)[c]);
			break;
		case 57:
			for (int c = 0; c < 3; c++)
				vec[c] = byteswapFlt(((float*)ptr)[c]);
			break;
		default:
			assert("unknown X360 attribute format");
		}
		return { vec[0], vec[1], vec[2], vec[3] };
	};
	
	for (auto& attrib : attribs) {
		if (attrib.stream == 255)
			break;
		for (size_t i = 0; i < numVertices; i++) {
			uint8_t* apnt = vertexBuffer + i * vStride + attrib.offset;
			int ftype = attrib.format & 63;
			auto vec = fetchAttrib(attrib.format, apnt);
			auto vecTuple = std::tie(vec[0], vec[1], vec[2], vec[3]);
			if (attrib.usage == 0) { // D3DDECLUSAGE_POSITION
				std::tie(cvt.verts[i].x, cvt.verts[i].y, cvt.verts[i].z, std::ignore) = vecTuple;
			}
			else if (attrib.usage == 3) { // D3DDECLUSAGE_NORMAL
				std::tie(cvt.norms[i].x, cvt.norms[i].y, cvt.norms[i].z, std::ignore) = vecTuple;
			}
			else if (attrib.usage == 5) { // D3DDECLUSAGE_TEXCOORD
				std::tie(cvt.texSets[0][i][0], cvt.texSets[0][i][1], std::ignore, std::ignore) = vecTuple;
			}
			else if (attrib.usage == 10) { // D3DDECLUSAGE_COLOR
				uint32_t clr = 0;
				for (int c = 0; c < 4; c++)
					clr |= (int)(std::clamp(vec[c], 0.0f, 1.0f) * 255.0f) << (c*8);
				auto swapRB = [](auto x) {return ((x & 0xFF) << 16) | ((x & 0xFF0000) >> 16) | (x & 0xFF00FF00); };
				cvt.colors[i] = swapRB(byteswap32(clr));
			}
		}
	}

	/*
	printf("------\n");
	for (size_t i = 0; i < attribs.size()-1; i++) {
		auto& attrib = attribs[i];
		printf("%08X %08X (%02i) %04X %04X : ", attrib.offset, attrib.format, attrib.format & 63, attrib.usage, attrib.aunk3);
		size_t len = std::min(attribs[i + 1].offset - attribs[i].offset, vStride);
		uint8_t* apnt = vertexBuffer + attrib.offset;
		for (size_t b = 0; b < len; b++) {
			printf(" %02X", apnt[b]);
		}
		printf("\n");
	}
	*/

	return cvt;
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
		auto geo = std::make_shared<RwGeometry>();
		geo->deserialize(file);
		this->geometry = std::move(geo);
	}
	extensions.read(file, this, 0x14);
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
		auto geo = std::make_shared<RwGeometry>();
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
		for (auto& geo : this->geometries)
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
		RwAtomic& atom = atomics.emplace_back();
		atom.deserialize(file, false);
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
	for (RwAtomic& atom : atomics)
		atom.serialize(file);
	extensions.write(file);
	head1.end(file);
}

void RwTeamDictionary::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	uint32_t _numDings = file->readUint32();
	_unk1 = file->readUint32();
	_bings.resize(_numDings);
	for (Bing& bing : _bings) {
		bing._ding = file->readUint32();
	}
	//uint32_t sum = 0, lol = 0;
	for (Bing& bing : _bings) {
		bing._someNum = file->readUint32();

		//printf("%i %i %i %i\n", i, bing._someNum, sum, lol);
		//lol += bing._someNum;
		//sum += bing._someNum;

		bing._clump.deserialize(file);
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
	file->writeUint32((uint32_t)_bings.size());
	file->writeUint32(_unk1);
	for (Bing& bing : _bings)
		file->writeUint32(bing._ding);
	for (Bing& bing : _bings) {
		file->writeUint32(bing._someNum);
		bing._clump.serialize(file);
		if (bing._someNum == 1)
			file->writeUint32(0xFFFFFFFF);
	}
	head2.end(file);
	head1.end(file);
}

void RwTeam::deserialize(File * file)
{
	rwCheckHeader(file, 1);
	const uint32_t numBongs = file->readUint32();
	const uint32_t numDongs = file->readUint32();
	file->read(head2.data(), head2.size());
	dongs.resize(numDongs);
	for (Dong &dong : dongs) {
		file->read(dong.head3.data(), dong.head3.size());
		file->read(dong.head4.data(), dong.head4.size());
		for (uint32_t i = 0; i < numBongs; i++) {
			const uint32_t bingIndex = file->readUint32();
			if (bingIndex != 0xFFFFFFFF)
				dong.bongs.push_back(bingIndex);
		}
		rwCheckHeader(file, 0x10);
		dong.clump.deserialize(file);
	}
	file->read(end.data(), end.size());
}

void RwTeam::serialize(File * file)
{
	uint32_t numBongs = 0;
	for (Dong& dong : dongs) {
		numBongs = std::max(numBongs, (uint32_t)dong.bongs.size());
	}

	HeaderWriter headw1, headw2;
	headw1.begin(file, 0x1C);
	headw2.begin(file, 1);
	file->writeUint32(numBongs);
	file->writeUint32((uint32_t)dongs.size());
	file->write(head2.data(), head2.size());
	for (Dong &dong : dongs) {
		file->write(dong.head3.data(), dong.head3.size());
		file->write(dong.head4.data(), dong.head4.size());
		for (uint32_t bong : dong.bongs)
			file->writeUint32(bong);
		for (size_t i = dong.bongs.size(); i < numBongs; ++i)
			file->writeUint32(0xFFFFFFFF);
		dong.clump.serialize(file);
	}
	file->write(end.data(), end.size());
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
	if (bpp <= 8) {
		assert(width == pitch);
		uint8_t* oldpix = (uint8_t*)pixels.data();
		uint32_t* newpix = (uint32_t*)img.pixels.data();
		size_t oldsize = pitch * height;
		for (size_t i = 0; i < oldsize; i++)
			newpix[i] = palette[oldpix[i]];
	}
	else if (bpp == Format::ImageFormat_DXT1) {
		squish::DecompressImage(img.pixels.data(), width, height, pixels.data(), squish::kDxt1);
	}
	else if (bpp == Format::ImageFormat_DXT2) {
		squish::DecompressImage(img.pixels.data(), width, height, pixels.data(), squish::kDxt3);
	}
	else if (bpp == Format::ImageFormat_DXT4) {
		squish::DecompressImage(img.pixels.data(), width, height, pixels.data(), squish::kDxt5);
	}
	else {
		assert(false && "unsupported format for RGBA32 conversion");
	}
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

RwImage RwImage::loadFromFile(const wchar_t* filename)
{
	FILE* file;
	_wfopen_s(&file, filename, L"rb");
	RwImage img = loadFromFile(file);
	fclose(file);
	return img;
}

RwImage RwImage::loadFromFile(FILE* file)
{
	RwImage img;
	int sizx, sizy, origBpp;
	void* pix = stbi_load_from_file(file, &sizx, &sizy, &origBpp, 4);
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

RwImage RwImage::loadFromMemory(void* ptr, size_t len) {
	RwImage img;
	int sizx, sizy, origBpp;
	void* pix = stbi_load_from_memory((uint8_t*)ptr, (int)len, &sizx, &sizy, &origBpp, 4);
	assert(pix && "Failed to load image from memory\n");
	img.width = sizx;
	img.height = sizy;
	img.bpp = 32;
	img.pitch = img.width * 4;
	img.pixels.resize(img.pitch * img.height);
	memcpy(img.pixels.data(), pix, img.pixels.size());
	stbi_image_free(pix);
	return img;
}

std::vector<RwImage> RwImage::loadDDS(const void* ddsData)
{
	uint8_t* mmptr = (uint8_t*)ddsData;
	assert(*(uint32_t*)mmptr == byteswap32('DDS '));
	DDS_HEADER* dds = (DDS_HEADER*)(mmptr + 4);
	mmptr += 4 + dds->dwSize;

	std::vector<RwImage> images;
	images.resize(dds->dwMipMapCount ? dds->dwMipMapCount : 1);
	int width = dds->dwWidth, height = dds->dwHeight;
	for (auto& img : images) {
		img.width = width;
		img.height = height;
		img.bpp = 32;
		img.pitch = width * 4;
		if (dds->ddpf.dwFourCC == byteswap32('DXT1')) {
			img.bpp = RwImage::Format::ImageFormat_DXT1;
			int dxtSize = squish::GetStorageRequirements(width, height, squish::kDxt1);
			img.pixels.resize(dxtSize);
			memcpy(img.pixels.data(), mmptr, dxtSize);
			mmptr += dxtSize;
		}
		else if (dds->ddpf.dwFourCC == byteswap32('DXT4') || dds->ddpf.dwFourCC == byteswap32('DXT5')) {
			img.bpp = RwImage::Format::ImageFormat_DXT4;
			int dxtSize = squish::GetStorageRequirements(width, height, squish::kDxt5);
			img.pixels.resize(dxtSize);
			memcpy(img.pixels.data(), mmptr, dxtSize);
			mmptr += dxtSize;
		}
		else {
			assert(false && "unknown DDS texture format");
		}
		width /= 2;  if (width == 0) width = 1;
		height /= 2; if (height == 0) height = 1;
	}

	return images;
}

int RwImage::computeDDSSize(const void* ddsData)
{
	assert(*(uint32_t*)ddsData == byteswap32('DDS '));
	DDS_HEADER* dds = (DDS_HEADER*)((uint8_t*)ddsData + 4);

	int mmptr = 4 + dds->dwSize;
	assert(mmptr == 128);
	int numMipMaps = dds->dwMipMapCount ? dds->dwMipMapCount : 1;
	int width = dds->dwWidth, height = dds->dwHeight;
	for (int mm = 0; mm < numMipMaps; ++mm) {
		if (dds->ddpf.dwFourCC == byteswap32('DXT1')) {
			int dxtSize = squish::GetStorageRequirements(width, height, squish::kDxt1);
			mmptr += dxtSize;
		}
		else if (dds->ddpf.dwFourCC == byteswap32('DXT4') || dds->ddpf.dwFourCC == byteswap32('DXT5')) {
			int dxtSize = squish::GetStorageRequirements(width, height, squish::kDxt5);
			mmptr += dxtSize;
		}
		else {
			assert(false && "unknown DDS texture format");
		}
		width /= 2;  if (width == 0) width = 1;
		height /= 2; if (height == 0) height = 1;
	}
	return mmptr;
}

void RwPITexDict::deserialize(File * file)
{
	uint16_t numTex = file->readUint16();
	flags = file->readUint16();
	textures.resize(numTex);
	for (PITexture &pit : textures) {
		pit.images.resize(file->readUint32());
		for (auto& img : pit.images) {
			rwCheckHeader(file, 0x18);
			img.deserialize(file);
		}
		rwCheckHeader(file, 6);
		pit.texture.deserialize(file);
	}
}

void RwPITexDict::serialize(File * file)
{
	HeaderWriter head;
	head.begin(file, 0x23);
	file->writeUint16(textures.size());
	file->writeUint16(flags);
	for (PITexture &pit : textures) {
		file->writeUint32(pit.images.size());
		for (auto& img : pit.images)
			img.serialize(file);
		pit.texture.serialize(file);
	}
	head.end(file);
}

size_t RwPITexDict::findTexture(const std::string& name) const
{
	for (size_t i = 0; i < textures.size(); ++i) {
		if (textures[i].texture.name == name)
			return i;
	}
	return -1;
}

void RwFont2D::deserialize(File * file)
{
	flags = file->readUint32();
	fntUnk1 = file->readUint32();
	glyphHeight = file->readFloat();
	fntUnk3 = file->readFloat();
	fntUnk4 = file->readUint32();
	fntUnk5 = file->readUint32();
	firstWideChar = file->readUint32();
	uint32_t numGlyphs = file->readUint32();
	uint32_t numWideChars = file->readUint32();

	wideGlyphTable.reserve(numWideChars);
	for (uint32_t i = 0; i < numWideChars; i++)
		wideGlyphTable.push_back(file->readUint16());
	for (int i = 0; i < charGlyphTable.size(); i++)
		charGlyphTable[i] = file->readUint16();

	glyphs.resize(numGlyphs);
	for (Glyph &gl : glyphs) {
		for (float &f : gl.coords)
			f = file->readFloat();
		gl.glUnk1 = file->readFloat();
		gl.texIndex = file->readUint8();
	}

	texNames.resize(file->readUint32());
	for (auto &tn : texNames)
		tn = file->readString(32).c_str();
}

void RwFont2D::serialize(File * file)
{
	HeaderWriter head;
	head.begin(file, 0x199);

	file->writeUint32(flags);
	file->writeUint32(fntUnk1);
	file->writeFloat(glyphHeight);
	file->writeFloat(fntUnk3);
	file->writeUint32(fntUnk4);
	file->writeUint32(fntUnk5);
	file->writeUint32(firstWideChar);
	file->writeUint32(glyphs.size());
	file->writeUint32(wideGlyphTable.size());

	for (uint16_t &i : wideGlyphTable)
		file->writeUint16(i);
	for (uint16_t &i : charGlyphTable)
		file->writeUint16(i);

	for (Glyph &gl : glyphs) {
		for (float &f : gl.coords)
			file->writeFloat(f);
		file->writeFloat(gl.glUnk1);
		file->writeUint8(gl.texIndex);
	}

	file->writeUint32(texNames.size());
	for (auto &tn : texNames) {
		char buf[32];
		memset(buf, 0, 32);
		for (int i = 0; i < 32; i++) {
			if (tn[i] == 0) break;
			buf[i] = tn[i];
		}
		file->write(buf, 32);
	}

	head.end(file);
}

uint16_t* RwFont2D::createGlyphSlot(uint16_t charId)
{
	assert(charId != 0xFFFF);
	if (charId >= 0 && charId < 128)
		return &charGlyphTable[charId];
	else if (charId >= 128) {
		// first char added
		if (wideGlyphTable.empty()) {
			wideGlyphTable = { 0xFFFF };
			firstWideChar = charId;
		}
		// char before wide char range
		else if (charId < firstWideChar) {
			wideGlyphTable.insert(wideGlyphTable.begin(), firstWideChar - charId, 0xFFFF);
			firstWideChar = charId;
		}
		// char after wide char range
		else if (charId >= firstWideChar + wideGlyphTable.size()) {
			wideGlyphTable.insert(wideGlyphTable.end(), charId + 1 - (firstWideChar + wideGlyphTable.size()), 0xFFFF);
		}
		return &wideGlyphTable[charId - firstWideChar];
	}
}

void RwBrush2D::deserialize(File * file)
{
	rbUnk1 = file->readUint32();
	for (float &f : rbFloats)
		f = file->readFloat();
	rbUnk2 = file->readUint32();
	rbUnk3 = file->readFloat();
	rbUnk4 = file->readUint32();
}

void RwBrush2D::serialize(File * file)
{
	HeaderWriter head;
	head.begin(file, 0x1A2);

	file->writeUint32(rbUnk1);
	for (float &f : rbFloats)
		file->writeFloat(f);
	file->writeUint32(rbUnk2);
	file->writeFloat(rbUnk3);
	file->writeUint32(rbUnk4);

	head.end(file);
}

void RwAnimAnimation::deserialize(File* file)
{
	version = file->readUint32();
	schemeId = file->readUint32();
	auto numFrames = file->readUint32();
	flags = file->readUint32();
	if (version >= 0x101)
		extra = file->readUint32();
	duration = file->readFloat();
	if (schemeId == 1) {
		auto& hframes = frames.emplace<std::vector<HAnimKeyFrame>>();
		hframes.resize(numFrames);
		for (auto& frame : hframes) {
			frame.time = file->readFloat();
			for (float& f : frame.quaternion)
				f = file->readFloat();
			for (float& f : frame.translation)
				f = file->readFloat();
			frame.prevFrame = file->readUint32();
		}
	}
	else if (schemeId == 2) {
		auto& cframes = frames.emplace<std::vector<CompressedKeyFrame>>();
		cframes.resize(numFrames);
		for (auto& frame : cframes) {
			frame.time = file->readFloat();
			for (int16_t& f : frame.quaternion)
				f = file->readUint16();
			for (int16_t& f : frame.translation)
				f = file->readUint16();
			frame.prevFrame = file->readUint32();
		}
		for (float& f : compressedTranslationOffset)
			f = file->readFloat();
		for (float& f : compressedTranslationScale)
			f = file->readFloat();
	}
	else
		assert(0 && "unknown scheme id");
}

void RwAnimAnimation::serialize(File* file)
{
	HeaderWriter head;
	head.begin(file, 0x1B);
	file->writeUint32(version);
	file->writeUint32(schemeId);
	file->writeUint32(numFrames());
	file->writeUint32(flags);
	if (version >= 0x101)
		file->writeUint32(extra);
	file->writeFloat(duration);
	if (schemeId == 1) {
		for (const auto& frame : std::get<std::vector<HAnimKeyFrame>>(frames)) {
			file->writeFloat(frame.time);
			for (const float& f : frame.quaternion)
				file->writeFloat(f);
			for (const float& f : frame.translation)
				file->writeFloat(f);
			file->writeUint32(frame.prevFrame);
		}
	}
	else if (schemeId == 2) {
		for (const auto& frame : std::get<std::vector<CompressedKeyFrame>>(frames)) {
			file->writeFloat(frame.time);
			for (const int16_t& f : frame.quaternion)
				file->writeUint16(f);
			for (const int16_t& f : frame.translation)
				file->writeUint16(f);
			file->writeUint32(frame.prevFrame);
		}
		for (const float& f : compressedTranslationOffset)
			file->writeFloat(f);
		for (const float& f : compressedTranslationScale)
			file->writeFloat(f);
	}
	head.end(file);
}

size_t RwAnimAnimation::numFrames() const
{
	return std::visit([](const auto& vec) {return vec.size(); }, frames);
}

RwAnimAnimation::HAnimKeyFrame RwAnimAnimation::decompressFrame(int frameIndex) const
{
	if (schemeId == 2) {
		const auto& compressed = std::get<std::vector<CompressedKeyFrame>>(frames).at(frameIndex);
		HAnimKeyFrame decompressed;
		decompressed.time = compressed.time;
		for (int i = 0; i < 4; ++i)
			decompressed.quaternion[i] = decompressFloat(compressed.quaternion[i]);
		for (int i = 0; i < 3; ++i)
			decompressed.translation.coord[i] = decompressFloat(compressed.translation[i]);
		decompressed.translation = decompressed.translation * compressedTranslationScale + compressedTranslationOffset;
		decompressed.prevFrame = compressed.prevFrame;
		return decompressed;
	}
	return std::get<std::vector<HAnimKeyFrame>>(frames).at(frameIndex);
}

float RwAnimAnimation::frameTime(int frameIndex) const
{
	return std::visit([frameIndex](const auto& vec) -> float {return vec[frameIndex].time; }, frames);
}

std::span<Matrix> RwAnimAnimation::interpolateNodeTransforms(int numNodes, float time) const
{
	// https://github.com/electronicarts/RenderWare3Docs/blob/master/userguide/UserGuideVol2.pdf
	// 15.3 Creating Animation Data, Keyframe Ordering, page 40

	time = std::clamp(time, 0.0f, duration);

	static std::vector<Matrix> buffer;
	buffer.clear();
	buffer.resize(numNodes);

	struct FrameIndex { int frame; float time; };
	static std::vector<FrameIndex> nodeCurrentFrame;
	nodeCurrentFrame.clear();
	nodeCurrentFrame.resize(numNodes);

	int framePtr = numNodes;
	for (int nodeIndex = 0; nodeIndex < numNodes; ++nodeIndex) {
		nodeCurrentFrame[nodeIndex] = { .frame = framePtr, .time = frameTime(framePtr) };
		framePtr += 1;
	}

	const int frameCount = numFrames();
	for (; framePtr < frameCount; ++framePtr) {
		auto it = std::ranges::min_element(nodeCurrentFrame, {}, &FrameIndex::time);
		if (it->time >= time)
			break;

		*it = { .frame = framePtr, .time = frameTime(framePtr) };
	}

	const int keyFrameSize = (schemeId == 2) ? 24 : 36;

	for (int nodeIndex = 0; nodeIndex < numNodes; ++nodeIndex) {
		const auto& frameB = decompressFrame(nodeCurrentFrame[nodeIndex].frame);
		const auto& frameA = decompressFrame(frameB.prevFrame / keyFrameSize);
		float delta = (time - frameA.time) / (frameB.time - frameA.time);
		const Vector3 translation = frameA.translation + (frameB.translation - frameA.translation) * delta;
		float q[4];
		float qNorm = 0.0f;
		for (int i = 0; i < 4; ++i) {
			q[i] = frameA.quaternion[i] + (frameB.quaternion[i] - frameA.quaternion[i]) * delta;
			qNorm += q[i] * q[i];
		}
		qNorm = std::sqrt(qNorm);
		if (qNorm > 0.0f) {
			for (int i = 0; i < 4; ++i) {
				q[i] /= qNorm;
			}
		}

		// https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#From_a_quaternion_to_an_orthogonal_matrix
		const auto& a = q[3];
		const auto& b = q[0];
		const auto& c = q[1];
		const auto& d = q[2];
		Matrix& matrix = buffer[nodeIndex];
		matrix = Matrix::getTranslationMatrix(translation);
		matrix._11 = a * a + b * b - c * c - d * d;
		matrix._21 = 2 * b * c - 2 * a * d;
		matrix._31 = 2 * b * d + 2 * a * c;
		matrix._12 = 2 * b * c + 2 * a * d;
		matrix._22 = a * a - b * b + c * c - d * d;
		matrix._32 = 2 * c * d - 2 * a * b;
		matrix._13 = 2 * b * d - 2 * a * c;
		matrix._23 = 2 * c * d + 2 * a * b;
		matrix._33 = a * a - b * b - c * c + d * d;
	}

	return buffer;
}

int RwAnimAnimation::guessNumNodes() const
{
	return std::visit([](const auto& vec) -> int
		{
			// To get the number nodes/bones in the anim,
			// we just count the number of frames with time == 0.
			size_t i = 0;
			size_t maxCount = vec.size() / 2 + 1;
			for (; i < maxCount; ++i) {
				if (vec[i].time > 0.0f)
					return i;
			}
			return 0;
		}, frames);
}

constexpr float RwAnimAnimation::decompressFloat(uint16_t compressedValue)
{
	const float sign = (compressedValue & 0x8000) ? -1.0f : 1.0f;
	if ((compressedValue & 0x7FFF) == 0)
		return sign * 0.0f;
	const int exponent = (int)((compressedValue >> 11) & 15) - 15;
	const float mantissa = (float)(compressedValue & 0x07FF) / (float)0x800 + 1.0f;
	const float power = 1.0f / float(1 << -exponent);
	return sign * mantissa * power;
}
static_assert(RwAnimAnimation::decompressFloat(0) == 0.0f);
static_assert(RwAnimAnimation::decompressFloat(0x7800) == 1.0f);

void RwRaster::deserialize(File* file, uint32_t rasterLen)
{
	uint32_t len = rwCheckHeader(file, 1);
	data.resize(len);
	file->read(data.data(), data.size());
	//extensions.read(file, this);
	furtherSegs.resize(rasterLen - len - 12);
	file->read(furtherSegs.data(), furtherSegs.size());
}

void RwRaster::serialize(File* file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x15);
	head2.begin(file, 1);
	file->write(data.data(), data.size());
	head2.end(file);
	//extensions.write(file);
	file->write(furtherSegs.data(), furtherSegs.size());
	head1.end(file);
}

RwPITexDict::PITexture RwRaster::convertToPI() const
{
	assert(data.size() >= 4);
	uint32_t platform = *(uint32_t*)data.data();
	switch (platform) {
	case byteswap32(6):
		return convertToPI_GCN();
	case byteswap32(9):
		return convertToPI_X360();
	default: {
		// 1x1 white pixel texture for unsupported platforms
		RwPITexDict::PITexture pit;
		RwImage& img = pit.images.emplace_back();
		img.width = 1;
		img.height = 1;
		img.bpp = 32;
		img.pitch = 4;
		img.pixels.resize(4);
		*(uint32_t*)img.pixels.data() = 0xFFFFFFFF;
		pit.nativeVersion = std::make_shared<RwRaster>(*this);
		return pit;
	}
	}
	assert(false && "unknown platform for RwRaster");
	return RwPITexDict::PITexture();
}

RwPITexDict::PITexture RwRaster::convertToPI_GCN() const
{
	RwPITexDict::PITexture pit;
	MemFile mf(const_cast<uint8_t*>(data.data()));
	auto platform = byteswap32(mf.readUint32());
	assert(platform == 6); // only GCN/Wii supported for now
	auto mode = byteswap32(mf.readUint32());
	auto unk3 = byteswap32(mf.readUint32());
	auto unk4 = byteswap32(mf.readUint32());
	auto unk5 = byteswap32(mf.readUint32());
	auto unk6 = byteswap32(mf.readUint32());
	auto name = mf.readString(32);
	auto name2 = mf.readString(32);
	auto unk7 = byteswap32(mf.readUint32());
	auto width = byteswap16(mf.readUint16());
	auto height = byteswap16(mf.readUint16());
	auto bpp = mf.readUint8();
	auto numMipmaps = mf.readUint8();
	auto texFormat = mf.readUint8();
	auto unk9d = mf.readUint8();
	auto unkA = byteswap32(mf.readUint32());
	auto len = byteswap32(mf.readUint32());

	const uint8_t* mmptr = data.data() + 108;
	const uint8_t* endptr = mmptr + len;

	pit.images.resize(numMipmaps);
	for (auto& img : pit.images) {
		img.width = width;
		img.height = height;
		img.bpp = 32;
		img.pitch = width * 4;
		assert(mmptr < endptr);

		// https://github.com/devkitPro/libogc/blob/master/gc/ogc/gx.h for texture format enums
		if (texFormat == 3) { // IA8
			int fwidth = (width + 3) & ~3;
			int fheight = (height + 3) & ~3;
			int mcols = fwidth / 4, mrows = fheight / 4;
			const uint8_t* inp = mmptr;
			DynArray<uint8_t> out(fwidth * fheight * 4);
			for (size_t r = 0; r < mrows; r++) {
				for (size_t c = 0; c < mcols; c++) {
					for (int y = 0; y < 4; y++) {
						for (int x = 0; x < 4; x++) {
							int p = (r * 4 + y) * fwidth + (c * 4 + x);
							uint8_t tAlpha = *inp++;
							uint8_t tIntensity = *inp++;
							out[p * 4 + 0] = tIntensity;
							out[p * 4 + 1] = tIntensity;
							out[p * 4 + 2] = tIntensity;
							out[p * 4 + 3] = tAlpha;
						}
					}
				}
			}
			if (width == fwidth)
				img.pixels = std::move(out);
			else {
				img.pixels.resize(width * height * 4);
				for (int y = 0; y < height; y++)
					memcpy(img.pixels.data() + y * width * 4, out.data() + y * fwidth * 4, width * 4);
			}
			mmptr += 2 * fwidth * fheight;
		}
		else if (texFormat == 6) { // RGBA8
			int fwidth = (width + 3) & ~3;
			int fheight = (height + 3) & ~3;
			int mcols = fwidth / 4, mrows = fheight / 4;
			const uint8_t* inp = mmptr;
			DynArray<uint8_t> out(fwidth * fheight * 4);
			for (size_t r = 0; r < mrows; r++) {
				for (size_t c = 0; c < mcols; c++) {
					for (int y = 0; y < 4; y++) {
						for (int x = 0; x < 4; x++) {
							int p = (r * 4 + y) * fwidth + (c * 4 + x);
							out[p * 4 + 3] = *inp++;
							out[p * 4 + 0] = *inp++;
						}
					}
					for (int y = 0; y < 4; y++) {
						for (int x = 0; x < 4; x++) {
							int p = (r * 4 + y) * fwidth + (c * 4 + x);
							out[p * 4 + 1] = *inp++;
							out[p * 4 + 2] = *inp++;
						}
					}
				}
			}
			if (width == fwidth)
				img.pixels = std::move(out);
			else {
				img.pixels.resize(width * height * 4);
				for (int y = 0; y < height; y++)
					memcpy(img.pixels.data() + y * width * 4, out.data() + y * fwidth * 4, width * 4);
			}
			mmptr += 4 * fwidth * fheight;
		}
		else if (texFormat == 0x0E) { // CMPR
			int fwidth = (width + 7) & ~7;
			int fheight = (height + 7) & ~7;
			size_t mcols = fwidth / 8, mrows = fheight / 8;
			uint8_t* block = (uint8_t*)mmptr;
			DynArray<uint8_t> fout(fwidth * fheight * 4);
			uint32_t* out = (uint32_t*)fout.data();
			for (size_t r = 0; r < mrows; r++) {
				for (size_t c = 0; c < mcols; c++) {
					for (int y : {0, 1}) {
						for (int x : {0, 1}) {
							// byte swap
							uint8_t enc[8];
							memcpy(enc, block, 8);
							std::swap(enc[0], enc[1]);
							std::swap(enc[2], enc[3]);

							uint32_t dec[16];
							squish::Decompress((uint8_t*)dec, enc, squish::kDxt1);
							for (int p = 0; p < 16; p++)
								out[(r * 8 + y * 4 + (p / 4)) * fwidth + c * 8 + x * 4 + ((p & 3) ^ 3)] = dec[p];
							block += 8;
						}
					}
				}
			}
			if (width == fwidth)
				img.pixels = std::move(fout);
			else {
				img.pixels.resize(width * height * 4);
				for (int y = 0; y < height; y++)
					memcpy(img.pixels.data() + y * width * 4, fout.data() + y * fwidth * 4, width * 4);
			}
			mmptr = block;
		}
		else {
			assert(nullptr && "unknown gc/wii native texture format");
		}

		width /= 2;  if (width == 0) width = 1;
		height /= 2; if (height == 0) height = 1;
	}

	assert(mmptr == endptr);

	pit.texture.name = std::move(name);
	pit.texture.alphaName = std::move(name2);

	// TODO: Filter, wrap flags, ...
	pit.texture.filtering = mode & 255;
	pit.texture.uAddr = (mode >> 8) & 15;
	pit.texture.vAddr = (mode >> 12) & 15;

	pit.nativeVersion = std::make_shared<RwRaster>(*this);

	return pit;
}

RwPITexDict::PITexture RwRaster::convertToPI_X360() const
{
	RwPITexDict::PITexture pit;
	MemFile mf(const_cast<uint8_t*>(data.data()));
	auto platform = byteswap32(mf.readUint32());
	assert(platform == 9); // only GCN/Wii supported for now
	auto mode = byteswap32(mf.readUint32());
	//auto unk3 = byteswap32(mf.readUint32());
	//auto unk4 = byteswap32(mf.readUint32());
	//auto unk5 = byteswap32(mf.readUint32());
	//auto unk6 = byteswap32(mf.readUint32());
	auto name = mf.readString(32);
	auto name2 = mf.readString(32);
	auto unk1 = byteswap32(mf.readUint32());
	auto unk2 = byteswap32(mf.readUint32());
	auto width = byteswap16(mf.readUint16());
	auto height = byteswap16(mf.readUint16());
	auto bpp = mf.readUint8();
	auto numMipmaps = mf.readUint8();
	auto texFormat = mf.readUint8();
	auto unk9d = mf.readUint8();
	//auto unkA = byteswap32(mf.readUint32());
	auto len = byteswap32(mf.readUint32());

	const uint8_t* mmptr = mf._curptr;
	const uint8_t* endptr = mmptr + len;

	assert(mmptr + RwImage::computeDDSSize(mmptr) == endptr);
	pit.images = RwImage::loadDDS(mmptr);

	pit.texture.name = std::move(name);
	pit.texture.alphaName = std::move(name2);

	// TODO: Filter, wrap flags, ...
	pit.texture.filtering = mode & 255;
	pit.texture.uAddr = (mode >> 8) & 15;
	pit.texture.vAddr = (mode >> 12) & 15;

	pit.nativeVersion = std::make_shared<RwRaster>(*this);

	return pit;
}

RwRaster RwRaster::createFromPI(const RwPITexDict::PITexture& pit)
{
	if (pit.nativeVersion) {
		return *pit.nativeVersion;
	}

	const RwImage& img0 = pit.images.front();
	assert(img0.bpp == 32);

	// check for alpha
	bool hasAlpha = false;
	uint32_t* opix = (uint32_t*)img0.pixels.data();
	for (int p = 0; p < img0.width * img0.height; p++) {
		auto alpha = (opix[p] >> 24) & 255;
		if (alpha != 0 && alpha != 255) {
			hasAlpha = true;
			break;
		}
	}

	uint32_t datasize = 0;
	for (auto& img : pit.images) {
		int align = (hasAlpha ? 4 : 8) - 1;
		int fw = (img.width + align) & ~align;
		int fh = (img.height + align) & ~align;
		datasize += hasAlpha ? (4 * fw * fh) : (fw * fh / 2);
	}
	const uint32_t headsize = 108;
	uint32_t totsize = headsize + datasize;
	std::vector<uint8_t> bin(totsize);


	MemFile mf(bin.data());
	auto writeStr32 = [&mf](const std::string& str) {
		std::array<char, 32> buf;
		if (!strlen(str.c_str())) {
			buf.fill(0xDD);
			buf[0] = 0;
		}
		else {
			buf.fill(0);
			size_t ms = std::min((size_t)32, strlen(str.c_str()));
			for (size_t i = 0; i < ms; i++) buf[i] = str[i];
		}
		mf.write(buf.data(), 32);
	};

	mf.writeUint32(byteswap32(6));
	uint32_t mode = pit.texture.filtering | (pit.texture.uAddr << 8) | (pit.texture.vAddr << 12);
	mf.writeUint32(byteswap32(mode));
	mf.writeUint32(byteswap32(0));
	mf.writeUint32(byteswap32(1));
	mf.writeUint32(byteswap32(1));
	mf.writeUint32(byteswap32(0));
	writeStr32(pit.texture.name);
	writeStr32(pit.texture.alphaName);
	mf.writeUint32(byteswap32(0)); //
	mf.writeUint16(byteswap16((uint16_t)img0.width));
	mf.writeUint16(byteswap16((uint16_t)img0.height));
	mf.writeUint8(hasAlpha ? 32 : 4);
	mf.writeUint8((uint8_t)pit.images.size()); // num mipmaps, need to save all mipmaps too!
	mf.writeUint8(hasAlpha ? 6 : 0xE);
	mf.writeUint8(255);
	mf.writeUint32(byteswap32(0)); // 0 or 1
	mf.writeUint32(byteswap32(datasize));
	assert(mf.tell() == headsize);

	uint8_t* mmptr = bin.data() + headsize;

	for (auto& img : pit.images) {
		int width = img.width;
		int height = img.height;
		int align = (hasAlpha ? 4 : 8) - 1;
		int fwidth = (width + align) & ~align;
		int fheight = (height + align) & ~align;
		DynArray<uint8_t> temp(fwidth * fheight * 4);
		for (int y = 0; y < height; y++)
			memcpy(temp.data() + y * fwidth * 4, img.pixels.data() + y * width * 4, width * 4);
		if (hasAlpha) {
			uint8_t* dec = temp.data();
			uint8_t* enc = mmptr;
			int mcols = fwidth / 4, mrows = fheight / 4;
			for (size_t r = 0; r < mrows; r++) {
				for (size_t c = 0; c < mcols; c++) {
					for (int y = 0; y < 4; y++) {
						for (int x = 0; x < 4; x++) {
							int p = (r * 4 + y) * fwidth + (c * 4 + x);
							*enc++ = dec[p * 4 + 3];
							*enc++ = dec[p * 4 + 0];
						}
					}
					for (int y = 0; y < 4; y++) {
						for (int x = 0; x < 4; x++) {
							int p = (r * 4 + y) * fwidth + (c * 4 + x);
							*enc++ = dec[p * 4 + 1];
							*enc++ = dec[p * 4 + 2];
						}
					}
				}
			}
			mmptr = enc;
		}
		else {
			int mcols = fwidth / 8, mrows = fheight / 8;
			uint8_t* block = (uint8_t*)mmptr;
			uint32_t* dec = (uint32_t*)temp.data();
			for (int r = 0; r < mrows; r++) {
				for (int c = 0; c < mcols; c++) {
					for (int y : {0, 1}) {
						for (int x : {0, 1}) {
							uint32_t tile[16];
							for (int p = 0; p < 16; p++)
								tile[p] = dec[(r * 8 + y * 4 + (p / 4)) * fwidth + c * 8 + x * 4 + ((p & 3) ^ 3)];
							uint8_t enc[8];
							squish::Compress((uint8_t*)tile, enc, squish::kDxt1 | squish::kColourRangeFit);
							// byte swap
							std::swap(enc[0], enc[1]);
							std::swap(enc[2], enc[3]);
							memcpy(block, enc, 8);
							block += 8;
						}
					}
				}
			}
			mmptr = block;
		}
	}
	assert(mmptr == bin.data() + totsize);

	RwRaster raster;
	raster.data = std::move(bin);

	raster.furtherSegs.resize(12);
	*(uint32_t*)(raster.furtherSegs.data()) = 3;
	*(uint32_t*)(raster.furtherSegs.data() + 4) = 0;
	*(uint32_t*)(raster.furtherSegs.data() + 8) = HeaderWriter::rwver;

	return raster;
}

void RwNTTexDict::deserialize(File* file)
{
	uint32_t headlen = rwCheckHeader(file, 1);
	assert(headlen == 4);
	auto numTextures = file->readUint16();
	platform = file->readUint16();
	textures.resize(numTextures);
	for (auto& tex : textures) {
		uint32_t len = rwCheckHeader(file, 0x15);
		tex.deserialize(file, len);
	}
	extensions.read(file, this);
}

void RwNTTexDict::serialize(File* file)
{
	HeaderWriter head1, head2;
	head1.begin(file, 0x16);
	head2.begin(file, 1);
	file->writeUint16(textures.size());
	file->writeUint16(platform);
	head2.end(file);
	for (auto& tex : textures) {
		tex.serialize(file);
	}
	extensions.write(file);
	head1.end(file);
}

RwPITexDict RwNTTexDict::convertToPI()
{
	RwPITexDict pitd;
	pitd.flags = 1;
	pitd.textures.reserve(textures.size());
	for (size_t i = 0; i < textures.size(); i++) {
		pitd.textures.push_back(textures[i].convertToPI());
	}
	pitd.nativeVersionPlatform = platform;
	return pitd;
}

RwNTTexDict RwNTTexDict::createFromPI(const RwPITexDict& pi)
{
	RwNTTexDict ntdict;
	ntdict.platform = pi.nativeVersionPlatform;
	ntdict.textures.reserve(pi.textures.size());
	for (auto& pit : pi.textures)
		ntdict.textures.push_back(RwRaster::createFromPI(pit));
	return ntdict;
}
