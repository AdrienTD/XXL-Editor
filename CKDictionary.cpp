#include "CKDictionary.h"
#include "rw.h"
#include "KEnvironment.h"
#include <array>

void CAnimationDictionary::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t numIndices = file->readUint32();
	animIndices.reserve(numIndices);
	for (uint32_t i = 0; i < numIndices; i++) {
		animIndices.push_back(file->readUint32());
	}
}

void CAnimationDictionary::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(animIndices.size());
	for (uint32_t n : animIndices)
		file->writeUint32(n);
}

void CTextureDictionary::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t numTex = file->readUint32();
	textures.resize(numTex);
	for (Texture &tex : textures) {
		file->read(tex.name, 32);
		tex.name[32] = 0;
		tex.unk1 = file->readUint32();
		tex.unk2 = file->readUint32();
		tex.unk3 = file->readUint32();
		rwCheckHeader(file, 0x18);
		tex.image.deserialize(file);
	}
}

void CTextureDictionary::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(textures.size());
	for (Texture &tex : textures) {
		file->write(tex.name, 32);
		file->writeUint32(tex.unk1);
		file->writeUint32(tex.unk2);
		file->writeUint32(tex.unk3);
		tex.image.serialize(file);
	}
}

void CKSoundDictionaryID::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	soundEntries.resize(file->readUint32());
	for (SoundEntry &ent : soundEntries) {
		ent.active = file->readUint8() == 1;
		if (ent.active) {
			ent.id = file->readUint32();
			ent.flags = file->readUint32();
			if (ent.flags & 0x10) {
				ent.obj = kenv->readObjRef<CKObject>(file);
			}
			else {
				for (uint32_t &u : ent.refalt)
					u = file->readUint32();
			}
			ent.unk1 = file->readFloat();
			ent.unk2 = file->readFloat();
			ent.unk3 = file->readFloat();
			ent.unk4 = file->readFloat();
			for (float &f : ent.flar)
				f = file->readFloat();
			ent.unk6 = file->readUint8();
		}
	}
}

void CKSoundDictionaryID::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(soundEntries.size());
	for (SoundEntry &ent : soundEntries) {
		file->writeUint8(ent.active ? 1 : 0);
		if (ent.active) {
			file->writeUint32(ent.id);
			file->writeUint32(ent.flags);
			if (ent.flags & 0x10) {
				kenv->writeObjRef(file, ent.obj);
			}
			else {
				for (uint32_t &u : ent.refalt)
					file->writeUint32(u);
			}
			file->writeFloat(ent.unk1);
			file->writeFloat(ent.unk2);
			file->writeFloat(ent.unk3);
			file->writeFloat(ent.unk4);
			for (float &f : ent.flar)
				file->writeFloat(f);
			file->writeUint8(ent.unk6);
		}
	}
}

void CKSoundDictionary::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	inactive = file->readUint8();
	uint32_t numSounds = file->readUint32();
	sounds.resize(numSounds);
	if (numSounds > 0) {
		for (Sound &snd : sounds) {
			snd.id1 = file->readUint32();
			snd.unk2 = file->readFloat();
			snd.unk3 = file->readFloat();
			snd.unk4 = file->readUint8();
			snd.unk5 = file->readFloat();
			snd.sampleRate = file->readUint16();
			snd.unk7 = file->readUint32();
			snd.unk8 = file->readUint8();
			snd.unk9 = file->readUint8();
			snd.unkA = file->readUint8();
			snd.id2 = file->readUint32();
		}
		rwCheckHeader(file, RwSoundDictionary::tagID);
		rwSoundDict.deserialize(file);
	}
}

void CKSoundDictionary::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint8(inactive);
	file->writeUint32(sounds.size());
	if (sounds.size() > 0) {
		for (Sound &snd : sounds) {
			file->writeUint32(snd.id1);
			file->writeFloat(snd.unk2);
			file->writeFloat(snd.unk3);
			file->writeUint8(snd.unk4);
			file->writeFloat(snd.unk5);
			file->writeUint16(snd.sampleRate);
			file->writeUint32(snd.unk7);
			file->writeUint8(snd.unk8);
			file->writeUint8(snd.unk9);
			file->writeUint8(snd.unkA);
			file->writeUint32(snd.id2);
		}
		rwSoundDict.serialize(file);
	}
}
