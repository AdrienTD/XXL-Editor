#include "rwsound.h"

static constexpr uint32_t byteswap32(uint32_t val) { return ((val & 255) << 24) | (((val >> 8) & 255) << 16) | (((val >> 16) & 255) << 8) | ((val >> 24) & 255); }

void RwSoundData::deserialize(File * file, uint32_t size)
{
	data.resize(size);
	file->read(data.data(), data.size());
}

void RwSoundData::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	file->write(data.data(), data.size());
	hw.end(file);
}

void RwSoundInfo::deserialize(File * file, uint32_t size)
{
	auto readU32 = [&]() { uint32_t val = file->readUint32(); return isBigEndian ? byteswap32(val) : val; };
	uint32_t start = file->tell();
	unk1 = readU32();
	dings.resize(2);
	for (Ding &ding : dings) {
		ding.sampleRate = readU32();
		ding.dunk1 = readU32();
		ding.dataSize = readU32();
		file->read(ding.rest.data(), ding.rest.size());
	}
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
	for (Ding &ding : dings) {
		writeU32(ding.sampleRate);
		writeU32(ding.dunk1);
		writeU32(ding.dataSize);
		file->write(ding.rest.data(), ding.rest.size());
	}
	file->write(rest.data(), rest.size());
	file->write(name.data(), name.size());
	hw.end(file);
}

void RwSound::deserialize(File * file)
{
	uint32_t size = rwCheckHeader(file, RwSoundInfo::tagID);
	info.deserialize(file, size);
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
	// TODO :)
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
	file->writeUint32(str_k);
	file->writeUint32(str_l);
	file->writeUint32(str_m);
	file->writeUint32(str_n);
	file->writeUint32(str_o);
	file->write(streamName.data(), 16);

	for (const auto& segment : segments) {
		file->writeUint32(segment.segVal_1a);
		file->writeUint32(segment.segVal_1b);
		file->writeUint32(segment.segVal_1c);
		file->writeUint32(segment.segVal_1d);
		file->writeUint32(segment.segVal_1e);
		file->writeUint32(segment.segVal_1f);
		file->writeUint32(segment.dataAlignedSize);
		file->writeUint32(segment.dataOffset);
	}
	for (const auto& segment : segments) {
		file->writeUint32(segment.dataSize);
	}
	for (const auto& segment : segments) {
		file->writeUint32(segment.segVal_3a);
		file->writeUint32(segment.segVal_3b);
		file->writeUint32(segment.segVal_3c);
		file->writeUint32(segment.segVal_3d);
	}
	for (const auto& segment : segments) {
		file->write(segment.name.data(), 16);
	}

	file->writeUint32(fin_a);
	file->writeUint32(fin_b);
	file->writeUint32(fin_c);
	file->writeUint32(sub_d);
	file->writeUint32(subSectorSize);
	file->writeUint32(fin_f);
	file->writeUint32(sub_g);
	file->writeUint32(sub_h);
	file->writeUint32(subSectorUsedSize);
	file->writeUint32(fin_j);
	file->writeUint32(subSampleRate);
	file->writeUint32(fin_l);
	file->writeUint32(fin_m);
	file->writeUint8(fin_n1);
	file->writeUint8(subNumChannels);
	file->writeUint8(fin_n3);
	file->writeUint8(fin_n4);
	file->writeUint32(fin_o);
	file->writeUint32(fin_p);
	file->writeUint32(fin_q);
	file->writeUint32(fin_r);
	file->writeUint32(fin_s);
	file->writeUint32(fin_t);
	file->writeUint32(fin_u);
	file->writeUint32(fin_v);
	file->writeUint32(fin_w);
	file->writeUint32(fin_x);
	file->writeUint32(fin_y);
	file->writeUint32(fin_z);
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
