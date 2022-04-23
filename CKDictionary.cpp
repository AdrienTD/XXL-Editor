#include "CKDictionary.h"
#include "rw.h"
#include "KEnvironment.h"
#include <array>

void CAnimationDictionary::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t numIndices = file->readUint32();
	animIndices.reserve(numIndices);
	if(kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
		secondAnimIndices.reserve(numIndices);
	for (uint32_t i = 0; i < numIndices; i++) {
		animIndices.push_back(file->readUint32());
		if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
			secondAnimIndices.push_back(file->readUint32());
	}
}

void CAnimationDictionary::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(animIndices.size());
	for (size_t i = 0; i < animIndices.size(); i++) {
		file->writeUint32(animIndices[i]);
		if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster)
			file->writeUint32(secondAnimIndices[i]);
	}
}

void CTextureDictionary::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	// Console
	if (kenv->platform != kenv->PLATFORM_PC) {
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			uint8_t isRwDict = file->readUint8();
			assert(isRwDict == 1);
		}
		RwNTTexDict nativeDict;
		rwCheckHeader(file, 0x16);
		nativeDict.deserialize(file);
		piDict = nativeDict.convertToPI();
		return;
	}

	// XXL Remaster
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->isRemaster) {
		RwPITexDict pitd;
		rwCheckHeader(file, 0x23);
		piDict.deserialize(file);
		return;
	}

	// PC
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		uint8_t isRwDict = file->readUint8();
		assert(isRwDict == 0);
	}
	uint32_t numTex = file->readUint32();
	piDict.textures.resize(numTex);
	for (auto &pit : piDict.textures) {
		pit.texture.name = file->readString(32).c_str();
		pit.texture.filtering = (uint8_t)file->readUint32();
		pit.texture.uAddr = (uint8_t)file->readUint32();
		pit.texture.vAddr = (uint8_t)file->readUint32();
		pit.texture.usesMips = true;

		pit.images.emplace_back();
		rwCheckHeader(file, 0x18);
		pit.images[0].deserialize(file);
	}
}

void CTextureDictionary::serialize(KEnvironment * kenv, File * file)
{
	// Console
	if (kenv->platform != kenv->PLATFORM_PC) {
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			file->writeUint8(1);
		}
		RwNTTexDict nativeDict = RwNTTexDict::createFromPI(piDict);
		nativeDict.serialize(file);
		return;
	}

	// XXL1 Remaster
	if (kenv->version <= kenv->KVERSION_XXL1 && kenv->platform == kenv->PLATFORM_PC && kenv->isRemaster) { // for Romaster
		piDict.serialize(file);
		return;
	}

	// PC
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
		file->writeUint8(0);
	}
	file->writeUint32(piDict.textures.size());
	for (auto &pit : piDict.textures) {
		char buf[32] = { 0 };
		std::copy_n(pit.texture.name.data(), std::min((size_t)32, pit.texture.name.size()), std::begin(buf));
		file->write(buf, 32);
		file->writeUint32(pit.texture.filtering);
		file->writeUint32(pit.texture.uAddr);
		file->writeUint32(pit.texture.vAddr);
		pit.images[0].serialize(file);
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
			if (kenv->version <= kenv->KVERSION_XXL1) {
				snd.id1 = file->readUint32();
				snd.unk2 = file->readFloat();
				snd.unk3 = file->readFloat();
				snd.unk4 = file->readUint8();
				if (kenv->isRemaster)
					snd.hdPath = file->readSizedString<uint16_t>();
				snd.unk5 = file->readFloat();
				snd.sampleRate = file->readUint16();
				snd.unk7 = file->readUint32();
				snd.unk8 = file->readUint8();
				snd.unk9 = file->readUint8();
				snd.unkA = file->readUint8();
				snd.id2 = file->readUint32();
			}
			else if (kenv->version == kenv->KVERSION_XXL2) {
				snd.unk5 = file->readFloat(); // could have been unk2 or unk3 ?
				snd.sampleRate = file->readUint16();
				if (kenv->isRemaster)
					snd.hdPath = file->readSizedString<uint16_t>();
			}
			else if (kenv->version >= kenv->KVERSION_ARTHUR) {
				snd.waveObj = kenv->readObjRef<CKObject>(file);
			}
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
			if (kenv->version <= kenv->KVERSION_XXL1) {
				file->writeUint32(snd.id1);
				file->writeFloat(snd.unk2);
				file->writeFloat(snd.unk3);
				file->writeUint8(snd.unk4);
				if (kenv->isRemaster)
					file->writeSizedString<uint16_t>(snd.hdPath);
				file->writeFloat(snd.unk5);
				file->writeUint16(snd.sampleRate);
				file->writeUint32(snd.unk7);
				file->writeUint8(snd.unk8);
				file->writeUint8(snd.unk9);
				file->writeUint8(snd.unkA);
				file->writeUint32(snd.id2);
			}
			else if (kenv->version == kenv->KVERSION_XXL2) {
				file->writeFloat(snd.unk5); // could have been unk2 or unk3 ?
				file->writeUint16(snd.sampleRate);
				if (kenv->isRemaster)
					file->writeSizedString<uint16_t>(snd.hdPath);
			}
			else if (kenv->version >= kenv->KVERSION_ARTHUR) {
				kenv->writeObjRef(file, snd.waveObj);
			}
		}
		rwSoundDict.serialize(file);
	}
}
