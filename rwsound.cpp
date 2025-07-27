#include "rwsound.h"
#include "File.h"

static constexpr uint32_t byteswap32(uint32_t val) { return ((val & 255) << 24) | (((val >> 8) & 255) << 16) | (((val >> 16) & 255) << 8) | ((val >> 24) & 255); }

static void byteswapRwUuid(std::array<uint8_t, 16>& uuid)
{
	std::swap(uuid[0], uuid[3]);
	std::swap(uuid[1], uuid[2]);
	std::swap(uuid[4], uuid[5]);
	std::swap(uuid[6], uuid[7]);
}

static std::array<uint8_t, 16> readRwUuid(File* file, bool isBigEndian)
{
	std::array<uint8_t, 16> uuid;
	file->read(uuid.data(), 16);
	if (isBigEndian)
		byteswapRwUuid(uuid);
	return uuid;
}

static void writeRwUuid(const std::array<uint8_t, 16>& uuid, File* file, bool isBigEndian)
{
	if (isBigEndian) {
		std::array<uint8_t, 16> byteswappedUuid = uuid;
		byteswapRwUuid(byteswappedUuid);
		file->write(byteswappedUuid.data(), 16);
	}
	else {
		file->write(uuid.data(), 16);
	}
}

void RwaWaveFormat::deserialize(File* file, bool isBigEndian)
{
	auto readU32 = [&]() { uint32_t val = file->readUint32(); return isBigEndian ? byteswap32(val) : val; };
	this->sampleRate = readU32();
	this->ptrDataTypeUuid = readU32();
	this->dataSize = readU32();
	this->bitsPerSample = file->readUint8();
	this->numChannels = file->readUint8();
	this->padding1 = file->readUint8();
	this->padding2 = file->readUint8();
	this->ptrMiscData = readU32();
	this->miscDataSize = readU32();
	this->flags = file->readUint8();
	this->reserved = file->readUint8();
	this->padding3 = file->readUint8();
	this->padding4 = file->readUint8();
	this->uuid = readRwUuid(file, isBigEndian);
}

void RwaWaveFormat::serialize(File* file, bool isBigEndian) const
{
	auto writeU32 = [&](uint32_t val) { file->writeUint32(isBigEndian ? byteswap32(val) : val); };
	writeU32(this->sampleRate);
	writeU32(this->ptrDataTypeUuid);
	writeU32(this->dataSize);
	file->writeUint8(this->bitsPerSample);
	file->writeUint8(this->numChannels);
	file->writeUint8(this->padding1);
	file->writeUint8(this->padding2);
	writeU32(this->ptrMiscData);
	writeU32(this->miscDataSize);
	file->writeUint8(this->flags);
	file->writeUint8(this->reserved);
	file->writeUint8(this->padding3);
	file->writeUint8(this->padding4);
	writeRwUuid(this->uuid, file, isBigEndian);
}

void RwSoundData::deserialize(File * file, uint32_t size)
{
	file->read(additionalEncodingInfo.data(), additionalEncodingInfo.size());
	data.resize(size);
	file->read(data.data(), data.size());
}

void RwSoundData::serialize(File * file)
{
	rwWriteHeader(file, tagID, data.size());
	file->write(additionalEncodingInfo.data(), additionalEncodingInfo.size());
	file->write(data.data(), data.size());
}

void RwSoundInfo::deserialize(File * file, uint32_t size)
{
	auto readU32 = [&]() { uint32_t val = file->readUint32(); return isBigEndian ? byteswap32(val) : val; };
	uint32_t start = file->tell();
	unk1 = readU32();
	format.deserialize(file, isBigEndian);
	targetFormat.deserialize(file, isBigEndian);
	file->read(rest.data(), rest.size());
	name.resize(size - file->tell() + start);
	file->read(name.data(), name.size());
}

void RwSoundInfo::serialize(File * file)
{
	auto writeU32 = [&](uint32_t val) { file->writeUint32(isBigEndian ? byteswap32(val) : val); };
	HeaderWriter hw;
	hw.begin(file, tagID);
	writeU32(unk1);
	format.serialize(file, isBigEndian);
	targetFormat.serialize(file, isBigEndian);
	file->write(rest.data(), rest.size());
	file->write(name.data(), name.size());
	hw.end(file);
}

void RwSound::deserialize(File * file)
{
	uint32_t size = rwCheckHeader(file, RwSoundInfo::tagID);
	info.deserialize(file, size);
	if (info.format.uuid == RWA_FORMAT_ID_GCN_ADPCM) {
		data.additionalEncodingInfo.resize(0x64);
	}
	uint32_t siz = rwCheckHeader(file, RwSoundData::tagID);
	data.deserialize(file, siz);
}

void RwSound::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	info.serialize(file);
	data.serialize(file);
	hw.end(file);
}

void RwSoundList::deserialize(File * file)
{
	uint32_t numSounds = file->readUint32();
	isBigEndian = (numSounds != 0 && (numSounds >> 16) != 0); // check endianness, detection fails when there's more than 65535 sounds
	if (isBigEndian)
		numSounds = byteswap32(numSounds);
	sounds.resize(numSounds);
	for (RwSound &snd : sounds) {
		rwCheckHeader(file, RwSound::tagID);
		snd.info.isBigEndian = isBigEndian;
		snd.deserialize(file);
	}
}

void RwSoundList::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	file->writeUint32(isBigEndian ? byteswap32(sounds.size()) : sounds.size());
	for (RwSound &snd : sounds)
		snd.serialize(file);
	hw.end(file);
}

void RwSoundDictionaryInfo::deserialize(File * file, uint32_t size)
{
	bytes.resize(size);
	file->read(bytes.data(), size);
}

void RwSoundDictionaryInfo::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	file->write(bytes.data(), bytes.size());
	hw.end(file);
}

void RwSoundDictionary::deserialize(File * file)
{
	uint32_t siz = rwCheckHeader(file, RwSoundDictionaryInfo::tagID);
	info.deserialize(file, siz);
	rwCheckHeader(file, RwSoundList::tagID);
	list.deserialize(file);
}

void RwSoundDictionary::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	info.serialize(file);
	list.serialize(file);
	hw.end(file);
}

void RwStreamInfo::deserialize(File* file)
{
	const auto zero = file->readUint32();
	//assert(zero == 0);
	head_a = file->readUint32();
	head_b = file->readUint32();
	head_c = file->readUint32();
	head_d = file->readUint32();

	str_a = file->readUint32();
	str_b = file->readUint32();
	str_c = file->readUint32();
	numSegments = file->readUint32();
	str_e = file->readUint32();
	numSubstreams = file->readUint32();
	str_g = file->readUint32();
	basicSectorSize = file->readUint32();
	streamSectorSize = file->readUint32();
	basicSectorSize2 = file->readUint32();
	streamDataOffset = file->readUint32();
	file->read(streamUuid.data(), 16);
	file->read(streamName.data(), 16);

	segments.resize(numSegments);
	for (auto& segment : segments) {
		segment.segVal_1a = file->readUint32();
		segment.segVal_1b = file->readUint32();
		segment.segVal_1c = file->readUint32();
		segment.segVal_1d = file->readUint32();
		segment.numMarkers = file->readUint32();
		segment.ptrMarkers = file->readUint32();
		segment.dataAlignedSize = file->readUint32();
		segment.dataOffset = file->readUint32();
	}
	for (auto& segment : segments) {
		segment.dataSize = file->readUint32();
	}
	for (auto& segment : segments) {
		file->read(segment.uuid.data(), 16);
	}
	for (auto& segment : segments) {
		file->read(segment.name.data(), 16);
	}

	fin_a = file->readUint32();
	fin_b = file->readUint32();
	fin_c = file->readUint32();
	samplesPerFrame = file->readUint32();
	subSectorSize = file->readUint32();
	fin_f = file->readUint32();
	channelInterleaveSize = file->readUint16();
	audioFrameSize = file->readUint16();
	repeatChannels = file->readUint16();
	padding1 = file->readUint8();
	padding2 = file->readUint8();
	subSectorUsedSize = file->readUint32();
	subSectorOffset = file->readUint32();
	waveFormat.deserialize(file, false);
	fin_v = file->readUint32();
	file->read(subUuid.data(), 16);
	file->read(subName.data(), 16);
}

void RwStreamInfo::serialize(File* file)
{
	uint32_t startOffset = file->tell();
	HeaderWriter hw;
	hw.begin(file, tagID);

	auto infoSizeOffset = file->tell();
	file->writeUint32(0);
	file->writeUint32(head_a);
	file->writeUint32(head_b);
	file->writeUint32(head_c);
	file->writeUint32(head_d);

	file->writeUint32(str_a);
	file->writeUint32(str_b);
	file->writeUint32(str_c);
	file->writeUint32(numSegments);
	file->writeUint32(str_e);
	file->writeUint32(numSubstreams);
	file->writeUint32(str_g);
	file->writeUint32(basicSectorSize);
	file->writeUint32(streamSectorSize);
	file->writeUint32(basicSectorSize2);
	file->writeUint32(streamDataOffset);
	file->write(streamUuid.data(), 16);
	file->write(streamName.data(), 16);

	for (const auto& segment : segments) {
		file->writeUint32(segment.segVal_1a);
		file->writeUint32(segment.segVal_1b);
		file->writeUint32(segment.segVal_1c);
		file->writeUint32(segment.segVal_1d);
		file->writeUint32(segment.numMarkers);
		file->writeUint32(segment.ptrMarkers);
		file->writeUint32(segment.dataAlignedSize);
		file->writeUint32(segment.dataOffset);
	}
	for (const auto& segment : segments) {
		file->writeUint32(segment.dataSize);
	}
	for (const auto& segment : segments) {
		file->write(segment.uuid.data(), 16);
	}
	for (const auto& segment : segments) {
		file->write(segment.name.data(), 16);
	}

	file->writeUint32(fin_a);
	file->writeUint32(fin_b);
	file->writeUint32(fin_c);
	file->writeUint32(samplesPerFrame);
	file->writeUint32(subSectorSize);
	file->writeUint32(fin_f);
	file->writeUint16(channelInterleaveSize);
	file->writeUint16(audioFrameSize);
	file->writeUint16(repeatChannels);
	file->writeUint8(padding1);
	file->writeUint8(padding2);
	file->writeUint32(subSectorUsedSize);
	file->writeUint32(subSectorOffset);
	waveFormat.serialize(file, false);
	file->writeUint32(fin_v);
	file->write(subUuid.data(), 16);
	file->write(subName.data(), 16);

	auto posBackup = file->tell();
	file->seek(infoSizeOffset, SEEK_SET);
	file->writeUint32(posBackup - (infoSizeOffset + 4 * 5));
	file->seek(posBackup, SEEK_SET);

	const int remainingBytes = startOffset + 0x800 - 12 * 2 - file->tell();
	for (int i = 0; i < remainingBytes; ++i)
		file->writeUint8(0);
	hw.end(file);
}

void RwStream::deserialize(File* file)
{
	const auto infoSize = rwCheckHeader(file, RwStreamInfo::tagID);
	const auto infoOffset = file->tell();
	info.deserialize(file);
	file->seek(infoOffset + infoSize, SEEK_SET);
	const auto dataSize = rwCheckHeader(file, 0x80F);
	data.resize(dataSize);
	file->read(data.data(), data.size());
}

void RwStream::serialize(File* file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	info.serialize(file);
	HeaderWriter hw2;
	hw2.begin(file, 0x80F);
	file->write(data.data(), data.size());
	hw2.end(file);
	hw.end(file);
}
