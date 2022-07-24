#include "GamePatcherKClasses.h"
#include "File.h"
#include "KEnvironment.h"

namespace GamePatcher {
	File* g_drmValues = nullptr;

	void CKGroup::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		nextGroup = kenv->readObjRef<CKGroup>(file);
		parentGroup = kenv->readObjRef<CKGroup>(file);
		life = kenv->readObjRef<CKObject>(file);
		bundle = kenv->readObjRef<CKObject>(file);
		unk2 = file->readUint32();
		childGroup = kenv->readObjRef<CKGroup>(file);
		childHook = kenv->readObjRef<CKHook>(file);
	}

	void CKGroup::serialize(KEnvironment* kenv, File* file)
	{
		kenv->writeObjRef(file, nextGroup);
		kenv->writeObjRef(file, parentGroup);
		kenv->writeObjRef(file, life);
		kenv->writeObjRef(file, bundle);
		file->writeUint32(unk2);
		kenv->writeObjRef(file, childGroup);
		kenv->writeObjRef(file, childHook);
	}

	void CKAsterixGameManager::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		file->read(part1.data(), part1.size());
		// 4 byte value from DRM
		if (!g_drmValues)
			file->readUint32();
		file->read(part2.data(), part2.size());
		uint16_t strsize = file->readUint16();
		texture = file->readString(strsize);
		file->read(part3.data(), part3.size());
	}

	void CKAsterixGameManager::serialize(KEnvironment* kenv, File* file)
	{
		file->write(part1.data(), part1.size());
		file->writeUint32(0x24011980);
		file->write(part2.data(), part2.size());
		file->writeUint16((uint16_t)texture.size());
		file->write(texture.data(), texture.size());
		file->write(part3.data(), part3.size());

	}

	void CKGrpBaseSquad::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		CKGroup::deserialize(kenv, file, length);
		bsUnk1 = file->readUint32();
		msgAction = kenv->readObjRef<CKObject>(file);
	}

	void CKGrpBaseSquad::serialize(KEnvironment* kenv, File* file)
	{
		CKGroup::serialize(kenv, file);
		file->writeUint32(bsUnk1);
		kenv->writeObjRef(file, msgAction);
	}

	void CKGrpSquad::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		CKGrpBaseSquad::deserialize(kenv, file, length);
		for (auto mat : { &mat1, &mat2 }) {
			for (auto& f : *mat)
				f = file->readFloat();
		}
		sqUnk1 = file->readFloat();
		for (float& c : sqUnk2)
			c = file->readFloat();
		for (auto& ref : refs)
			ref = kenv->readObjRef<CKObject>(file);
		for (auto arr : { &sqUnk3, &sqUnk4 })
			for (float& f : *arr)
				f = file->readFloat();
		sqUnk5 = file->readUint32();
		uint32_t numChoreographies = file->readUint32();
		choreographies.resize(numChoreographies);
		for (auto& ref : choreographies)
			ref = kenv->readObjRef<CKObject>(file);
		uint32_t numChoreoKeys = file->readUint32();
		choreoKeys.resize(numChoreoKeys);
		for (auto& ref : choreoKeys)
			ref = kenv->readObjRef<CKObject>(file);
		for (auto arr : { &bings, &dings }) {
			arr->resize(file->readUint32());
			for (auto& bing : *arr) {
				bing.a = file->readUint32();
				bing.b = file->readUint8();
			}
		}
		fings.resize(file->readUint32());
		for (auto& i : fings)
			i = file->readUint32();
		for (auto& f : sqUnk6)
			f = file->readFloat();
		sqUnk7 = file->readUint16();
		sqUnk8 = file->readUint8();
		pools.resize(file->readUint32());
		for (PoolEntry& pe : pools) {
			pe.pool = kenv->readObjRef<CKObject>(file);
			pe.cpnt = kenv->readObjRef<CKObject>(file);
			pe.u1 = file->readUint8();
			pe.numEnemies = g_drmValues->readUint16();
			pe.u2 = file->readUint8();
			pe.u3 = kenv->readObjRef<CKObject>(file);
		}
		sqUnkA = file->readUint16();
		sqUnkB = file->readFloat();
		sqUnkC = file->readUint16();
	}

	void CKGrpSquad::serialize(KEnvironment* kenv, File* file)
	{
		CKGrpBaseSquad::serialize(kenv, file);
		for (auto mat : { &mat1, &mat2 }) {
			for (auto& f : *mat)
				file->writeFloat(f);
		}
		file->writeFloat(sqUnk1);
		for (float& c : sqUnk2)
			file->writeFloat(c);
		for (auto& ref : refs)
			kenv->writeObjRef<CKObject>(file, ref);
		for (auto arr : { &sqUnk3, &sqUnk4 })
			for (float& f : *arr)
				file->writeFloat(f);
		file->writeUint32(sqUnk5);
		file->writeUint32(choreographies.size());
		for (auto& ref : choreographies)
			kenv->writeObjRef<CKObject>(file, ref);
		file->writeUint32(choreoKeys.size());
		for (auto& ref : choreoKeys)
			kenv->writeObjRef<CKObject>(file, ref);
		for (auto arr : { &bings, &dings }) {
			file->writeUint32(arr->size());
			for (auto& bing : *arr) {
				file->writeUint32(bing.a);
				file->writeUint8(bing.b);
			}
		}
		file->writeUint32(fings.size());
		for (auto& i : fings)
			file->writeUint32(i);
		for (auto& f : sqUnk6)
			file->writeFloat(f);
		file->writeUint16(sqUnk7);
		file->writeUint8(sqUnk8);
		file->writeUint32(pools.size());
		for (PoolEntry& pe : pools) {
			kenv->writeObjRef(file, pe.pool);
			kenv->writeObjRef<CKObject>(file, pe.cpnt);
			file->writeUint8(pe.u1);
			file->writeUint16(pe.numEnemies);
			file->writeUint8(pe.u2);
			kenv->writeObjRef<CKObject>(file, pe.u3);
		}
		file->writeUint16(sqUnkA);
		file->writeFloat(sqUnkB);
		file->writeUint16(sqUnkC);
	}

	void CKGrpSquadEnemy::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		CKGrpSquad::deserialize(kenv, file, length);
		seUnk1 = file->readFloat();
		seUnk2 = file->readFloat();
	}

	void CKGrpSquadEnemy::serialize(KEnvironment* kenv, File* file)
	{
		CKGrpSquad::serialize(kenv, file);
		file->writeFloat(seUnk1);
		file->writeFloat(seUnk2);
	}

	void CKEnemyCoreCpnt::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		firstVal = file->readUint32();
		health = g_drmValues->readUint8();
	}

	void CKEnemyCoreCpnt::serialize(KEnvironment* kenv, File* file)
	{
		file->writeUint32(firstVal);
		file->writeUint8(health);
	}

	void CKHkTorchCore::deserialize(KEnvironment* kenv, File* file, size_t length)
	{
		file->read(part1.data(), part1.size());
		timer = g_drmValues->readFloat();
	}

	void CKHkTorchCore::serialize(KEnvironment* kenv, File* file)
	{
		file->write(part1.data(), part1.size());
		file->writeFloat(timer);
	}
}