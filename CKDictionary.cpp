#include "CKDictionary.h"
#include "rw.h"

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
