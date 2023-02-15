#include "rwext.h"
#include "rw.h"
#include "File.h"
#include <cassert>
#include <set>

RwExtUnknown::~RwExtUnknown()
{
	if (_ptr)
		free(_ptr);
}

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
	file->writeUint32(HeaderWriter::rwver);
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

RwExtension * RwExtCreate(uint32_t type, uint32_t parentType)
{
	RwExtension *ext;
	switch (type) {
	case 0x116:
		ext = new RwExtSkin(); break;
	case 0x11E:
		ext = new RwExtHAnim(); break;
	case 0x120:
		if (parentType == 0x14)
			ext = new RwExtMaterialEffectsPLG_Atomic();
		else
			ext = new RwExtMaterialEffectsPLG_Material();
		break;
	case 0x50E:
		ext = new RwExtBinMesh(); break;
	case 0x510:
		ext = new RwExtNativeData(); break;
	default:
		printf(" UUU %i UUU\n", type);
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
	RwGeometry* geo = (RwGeometry*)parent;
	if (geo->flags & RwGeometry::RWGEOFLAG_NATIVE) {
		RwsHeader rws = rwReadHeader(file);
		if (rws.type == 1 && rws.length == header.length - 12) {
			nativeData.resize(rws.length);
			file->read(nativeData.data(), nativeData.size());
			return;
		}
		else
			file->seek(-12, SEEK_CUR);
	}
	numBones = file->readUint8();
	numUsedBones = file->readUint8();
	maxWeightPerVertex = file->readUint8();
	file->readUint8();
	usedBones.reserve(numUsedBones);
	for (int i = 0; i < numUsedBones; i++)
		usedBones.push_back(file->readUint8());
	vertexIndices.resize(geo->numVerts);
	vertexWeights.resize(geo->numVerts);
	for (auto &viarr : vertexIndices)
		for (auto &ix : viarr)
			ix = file->readUint8();
	for (auto &warr : vertexWeights)
		for (auto &f : warr)
			f = file->readFloat();
	matrices.resize(numBones);
	for (Matrix &mat : matrices) {
		if (maxWeightPerVertex == 0)
			file->readUint32();
		for (int i = 0; i < 16; i++)
			mat.v[i] = file->readFloat();
	}
	isSplit = numUsedBones != 0; // TODO: Find a better way to detect the presence of split data
	if (isSplit) {
		boneLimit = file->readUint32();
		boneGroups.resize(file->readUint32());
		boneGroupIndices.resize(file->readUint32());
		if (!boneGroups.empty()) {
			boneIndexRemap.resize(numBones);
			file->read(boneIndexRemap.data(), boneIndexRemap.size());
			for (auto& grp : boneGroups) {
				grp.first = file->readUint8();
				grp.second = file->readUint8();
			}
			for (auto& gi : boneGroupIndices) {
				gi.first = file->readUint8();
				gi.second = file->readUint8();
			}
		}
	}
}

void RwExtSkin::serialize(File* file)
{
	HeaderWriter head1;
	head1.begin(file, 0x116);

	//RwGeometry* geo = (RwGeometry*)parent;
	//if (geo->flags & RwGeometry::RWGEOFLAG_NATIVE) {
	if (!nativeData.empty()) {
		HeaderWriter head2;
		head2.begin(file, 1);
		file->write(nativeData.data(), nativeData.size());
		head2.end(file);
		head1.end(file);
		return;
	}

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
	for (Matrix &mat : matrices) {
		if (maxWeightPerVertex == 0)
			file->writeUint32(0);
		for (int i = 0; i < 16; i++)
			file->writeFloat(mat.v[i]);
	}
	if (isSplit) {
		file->writeUint32(boneLimit);
		file->writeUint32(boneGroups.size());
		file->writeUint32(boneGroupIndices.size());
		if (!boneGroups.empty()) {
			assert(boneIndexRemap.size() == numBones);
			file->write(boneIndexRemap.data(), boneIndexRemap.size());
			for (auto& grp : boneGroups) {
				file->writeUint8(grp.first);
				file->writeUint8(grp.second);
			}
			for (auto& gi : boneGroupIndices) {
				file->writeUint8(gi.first);
				file->writeUint8(gi.second);
			}
		}
	}
	head1.end(file);
}

RwExtension * RwExtSkin::clone()
{
	return new RwExtSkin(*this);
}

void RwExtSkin::merge(const RwExtSkin & other)
{
	assert(numBones == other.numBones);
	assert(isSplit == other.isSplit);
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

uint32_t RwExtBinMesh::getType()
{
	return 0x50E;
}

void RwExtBinMesh::deserialize(File* file, const RwsHeader& header, void* parent)
{
	RwGeometry* geo = (RwGeometry*)parent;
	isNative = geo->flags & geo->RWGEOFLAG_NATIVE;

	flags = file->readUint32();
	meshes.resize(file->readUint32());
	totalIndices = file->readUint32();
	for (auto& mesh : meshes) {
		mesh.indices.resize(file->readUint32());
		mesh.material = file->readUint32();
		if (!isNative)
			for (uint32_t& ind : mesh.indices)
				ind = file->readUint32();
	}
}

void RwExtBinMesh::serialize(File* file)
{
	HeaderWriter head1;
	head1.begin(file, 0x50E);
	file->writeUint32(flags);
	file->writeUint32(meshes.size());
	file->writeUint32(totalIndices);
	for (auto& mesh : meshes) {
		file->writeUint32(mesh.indices.size());
		file->writeUint32(mesh.material);
		if (!isNative)
			for (uint32_t ind : mesh.indices)
				file->writeUint32(ind);
	}
	head1.end(file);
}

RwExtension* RwExtBinMesh::clone()
{
	return new RwExtBinMesh(*this);
}

uint32_t RwExtNativeData::getType()
{
	return 0x510;
}

void RwExtNativeData::deserialize(File* file, const RwsHeader& header, void* parent)
{
	uint32_t len = rwCheckHeader(file, 1);
	data.resize(len);
	file->read(data.data(), data.size());
}

void RwExtNativeData::serialize(File* file)
{
	HeaderWriter h1;
	h1.begin(file, 0x510);
	HeaderWriter h2;
	h2.begin(file, 1);
	file->write(data.data(), data.size());
	h2.end(file);
	h1.end(file);
}

RwExtension* RwExtNativeData::clone()
{
	return new RwExtNativeData(*this);
}

uint32_t RwExtMaterialEffectsPLG_Material::getType()
{
	return 0x120;
}

void RwExtMaterialEffectsPLG_Material::deserialize(File* file, const RwsHeader& header, void* parent)
{
	static const auto constructVariantAux = [](File* file, auto& var, uint32_t fxType, auto _index, auto& rec) -> bool {
		using TVAR = std::remove_reference_t<decltype(var)>;
		static constexpr size_t INDEX = decltype(_index)::value;
		if constexpr (INDEX < std::variant_size_v<TVAR>) {
			if (std::variant_alternative_t<INDEX, TVAR>::ID == fxType) {
				var.emplace<INDEX>().read(file);
				return true;
			}
			else {
				return rec(file, var, fxType, std::integral_constant<size_t, INDEX + 1>(), rec);
			}
		}
		return false;
	};

	static const auto constructVariant = [](File* file, auto& var) {
		uint32_t fxType = file->readUint32();
		printf("t%i ", fxType);
		bool valid = constructVariantAux(file, var, fxType, std::integral_constant<size_t, 0>(), constructVariantAux);
		assert(valid);
	};

	type = file->readUint32();
	printf(">>> ");
	constructVariant(file, firstEffect);
	constructVariant(file, secondEffect);
	printf(" <<<\n");
}

void RwExtMaterialEffectsPLG_Material::serialize(File* file)
{
	HeaderWriter h1;
	h1.begin(file, 0x120);

	file->writeUint32(type);
	auto writeVar = [file](auto& fx) {
		file->writeInt32(fx.ID);
		fx.write(file);
	};
	std::visit(writeVar, firstEffect);
	std::visit(writeVar, secondEffect);

	h1.end(file);
}

RwExtension* RwExtMaterialEffectsPLG_Material::clone()
{
	return new RwExtMaterialEffectsPLG_Material(*this);
}

void RwExtMaterialEffectsPLG_Material::BumpMapEffect::read(File* file)
{
	intensity = file->readFloat();
	hasBumpMap = file->readUint32();
	if (hasBumpMap) {
		rwCheckHeader(file, 6);
		bumpMap.deserialize(file);
	}
	hasHeightMap = file->readUint32();
	if (hasHeightMap) {
		rwCheckHeader(file, 6);
		heightMap.deserialize(file);
	}
}

void RwExtMaterialEffectsPLG_Material::BumpMapEffect::write(File* file)
{
	file->writeFloat(intensity);
	file->writeUint32(hasBumpMap);
	if (hasBumpMap)
		bumpMap.serialize(file);
	file->writeUint32(hasHeightMap);
	if (hasHeightMap)
		heightMap.serialize(file);
}

void RwExtMaterialEffectsPLG_Material::EnvironmentEffect::read(File* file)
{
	reflectionCoeff = file->readFloat();
	fbac = file->readUint32();
	hasEnvMap = file->readUint32();
	if (hasEnvMap) {
		rwCheckHeader(file, 6);
		envMap.deserialize(file);
	}
}

void RwExtMaterialEffectsPLG_Material::EnvironmentEffect::write(File* file)
{
	file->writeFloat(reflectionCoeff);
	file->writeUint32(fbac);
	file->writeUint32(hasEnvMap);
	if (hasEnvMap)
		envMap.serialize(file);
}

void RwExtMaterialEffectsPLG_Material::DualTextureEffect::read(File* file)
{
	srcBlendMode = file->readUint32();
	destBlendMode = file->readUint32();
	hasTexture = file->readUint32();
	if (hasTexture) {
		rwCheckHeader(file, 6);
		texture.deserialize(file);
	}
}

void RwExtMaterialEffectsPLG_Material::DualTextureEffect::write(File* file)
{
	file->writeUint32(srcBlendMode);
	file->writeUint32(destBlendMode);
	file->writeUint32(hasTexture);
	if (hasTexture)
		texture.serialize(file);
}

uint32_t RwExtMaterialEffectsPLG_Atomic::getType()
{
	return 0x120;
}

void RwExtMaterialEffectsPLG_Atomic::deserialize(File* file, const RwsHeader& header, void* parent)
{
	matFxEnabled = file->readUint32();
}

void RwExtMaterialEffectsPLG_Atomic::serialize(File* file)
{
	HeaderWriter h1;
	h1.begin(file, 0x120);
	file->writeUint32(matFxEnabled);
	h1.end(file);
}

RwExtension* RwExtMaterialEffectsPLG_Atomic::clone()
{
	return new RwExtMaterialEffectsPLG_Atomic(*this);
}
