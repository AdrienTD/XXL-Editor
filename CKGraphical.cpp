#include "CKGraphical.h"
#include "File.h"
#include "KEnvironment.h"

void CCloneManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	_numClones = file->readUint32();
	_unk1 = file->readUint32();
	_unk2 = file->readUint32();
	_unk3 = file->readUint32();
	_unk4 = file->readUint32();

	_clones.reserve(_numClones);
	for (uint32_t i = 0; i < _numClones; i++)
		_clones.push_back(kenv->readObjRef<CClone>(file));
}

void CCloneManager::serialize(KEnvironment * kenv, File * file)
{
}
