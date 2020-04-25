#include "CKGraphical.h"
#include "File.h"
#include "KEnvironment.h"
#include "rw.h"
#include "CKNode.h"

void CCloneManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	uint32_t start = file->tell();
	_numClones = file->readUint32();
	if (_numClones == 0)
		return;
	_unk1 = file->readUint32();
	_unk2 = file->readUint32();
	_unk3 = file->readUint32();
	_unk4 = file->readUint32();

	_clones.reserve(_numClones);
	for (uint32_t i = 0; i < _numClones; i++)
		_clones.push_back(kenv->readObjRef<CSGBranch>(file));

	rwCheckHeader(file, 0x22);
	_teamDict.deserialize(file);

	printf("Team at %X\n", file->tell() - start);
	rwCheckHeader(file, 0x1C);
	_team.deserialize(file);

	flinfos.reserve(_numClones);
	for (uint32_t i = 0; i < _numClones; i++) {
		std::array<float, 4> arr;
		for (float &f : arr)
			f = file->readFloat();
		flinfos.push_back(std::move(arr));
	}
}

void CCloneManager::serialize(KEnvironment * kenv, File * file)
{
	printf("CCloneManager at %08X\n", file->tell());
	file->writeUint32(_numClones);
	if (_numClones == 0)
		return;
	file->writeUint32(_unk1);
	file->writeUint32(_unk2);
	file->writeUint32(_unk3);
	file->writeUint32(_unk4);
	for (auto &clone : _clones)
		kenv->writeObjRef(file, clone);
	_teamDict.serialize(file);
	_team.serialize(file);
	for (auto &fli : flinfos) {
		for (float &f : fli)
			file->writeFloat(f);
	}
}
