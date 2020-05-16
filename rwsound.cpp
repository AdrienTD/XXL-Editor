#include "rwsound.h"

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
	uint32_t start = file->tell();
	unk1 = file->readUint32();
	dings.resize(2);
	for (Ding &ding : dings) {
		ding.sampleRate = file->readUint32();
		ding.dunk1 = file->readUint32();
		ding.dataSize = file->readUint32();
		file->read(ding.rest.data(), ding.rest.size());
	}
	file->read(rest.data(), rest.size());
	name.resize(size - file->tell() + start);
	file->read(name.data(), name.size());
}

void RwSoundInfo::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	file->writeUint32(unk1);
	for (Ding &ding : dings) {
		file->writeUint32(ding.sampleRate);
		file->writeUint32(ding.dunk1);
		file->writeUint32(ding.dataSize);
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
	sounds.resize(numSounds);
	for (RwSound &snd : sounds) {
		rwCheckHeader(file, RwSound::tagID);
		snd.deserialize(file);
	}
}

void RwSoundList::serialize(File * file)
{
	HeaderWriter hw;
	hw.begin(file, tagID);
	file->writeUint32(sounds.size());
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
