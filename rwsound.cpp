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
