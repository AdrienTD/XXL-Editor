#include "CKManager.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKLogic.h"
#include "CKDictionary.h"

void CKServiceManager::deserialize(KEnvironment* kenv, File * file, size_t length)
{
	uint32_t count = file->readUint32();
	//file->seek(4 * count, SEEK_CUR);
	services.reserve(count);
	for (uint32_t i = 0; i < count; i++) {
		services.push_back(kenv->readObjRef<CKService>(file, 0));
		//printf("Service %i %i\n", services[i]->getClassCategory(), services[i]->getClassID());
	}
	//printf("Level has %u services.\n", count);
}

void CKServiceManager::serialize(KEnvironment* kenv, File * file)
{
	file->writeUint32(this->services.size());
	for (kobjref<CKService> &srv : this->services)
		kenv->writeObjRef(file, srv);
}

void CKGraphic::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKReflectableManager::reflectMembers2(r, kenv);
	r.reflect(kgfcSgRootNode, "kgfcSgRootNode");
	r.reflect(kgfcUnk1, "kgfcUnk1");
	r.reflect(kgfcUnk2, "kgfcUnk2");
	r.reflect(kgfcUnk3, "kgfcUnk3");
	r.reflect(kgfcUnk4, "kgfcUnk4");
	r.reflect(kgfcUnk5, "kgfcUnk5");
	if (kenv->isRemaster)
		r.reflect(kgfcRomasterValue, "kgfcRomasterValue");
}

void CKSoundManager::reflectMembers2(MemberListener &r, KEnvironment *kenv) {
	CKReflectableManager::reflectMembers2(r, kenv);
	if (kenv->version <= KEnvironment::KVERSION_XXL2) {
		r.reflectSize<uint32_t>(ksndmgrSndDicts, "sizeFor_ksndmgrSndDicts");
		r.reflect(ksndmgrSndDicts, "ksndmgrSndDicts");
		r.reflect(ksndmgrSndDictID, "ksndmgrSndDictID");
		r.reflect(ksndmgrUnk3, "ksndmgrUnk3");
		r.reflect(ksndmgrUnk4, "ksndmgrUnk4");
		if (kenv->version >= KEnvironment::KVERSION_XXL2) {
			r.reflect(ksndmgrX2UnkByte, "ksndmgrX2UnkByte");
			r.reflect(ksndmgrX2UnkVector, "ksndmgrX2UnkVector");
		}
		r.reflect(ksndmgrUnk5, "ksndmgrUnk5");
		r.reflect(ksndmgrUnk6, "ksndmgrUnk6");
		r.reflect(ksndmgrUnk7, "ksndmgrUnk7");
		r.reflect(ksndmgrUnk8, "ksndmgrUnk8");
		r.reflectSize<uint32_t>(ksndmgrDings, "sizeFor_ksndmgrDings");
		r.enterArray("ksndmgrDings");
		for (size_t i = 0; i < ksndmgrDings.size(); i++) {
			r.enterStruct("ksndmgrDings");
			if (kenv->isRemaster)
				r.reflect(ksndmgrDings[i].remasterPath, "remasterPath");
			r.reflect(ksndmgrDings[i].duration, "duration");
			r.leaveArray();
			r.incrementIndex();
		}
		r.leaveArray();
		r.reflect(ksndmgrUnk11, "ksndmgrUnk11");
		r.reflect(ksndmgrUnk12, "ksndmgrUnk12");
		r.reflect(ksndmgrUnk13, "ksndmgrUnk13");
		r.reflect(ksndmgrUnk14, "ksndmgrUnk14");
		if (kenv->version == KEnvironment::KVERSION_XXL1) {
			r.reflect(ksndmgrSampleRate, "ksndmgrSampleRate");
		}
		else {
			r.reflectSize<uint32_t>(ksndmgrSoundObjects, "ksndmgrSoundObjects_size");
			r.reflect(ksndmgrSoundObjects, "ksndmgrSoundObjects");
		}
	}
	else if (kenv->version == KEnvironment::KVERSION_ARTHUR) {
		r.reflectSize<uint32_t>(ksndmgrSndDicts, "sizeFor_ksndmgrSndDicts");
		r.reflect(ksndmgrSndDicts, "ksndmgrSndDicts");
		r.reflect(ksndmgrSndDictID, "ksndmgrSndDictID");
		r.reflect(ksndmgrX2UnkVector, "ksndmgrX2UnkVector"); // ??
		r.reflect(ksndmgrUnk8, "ksndmgrUnk8"); // ??
		r.reflectSize<uint32_t>(ksndmgrDings, "sizeFor_ksndmgrDings");
		r.enterArray("ksndmgrDings");
		for (size_t i = 0; i < ksndmgrDings.size(); i++) {
			r.enterStruct("ksndmgrDings");
			r.reflect(ksndmgrDings[i].duration, "duration");
			r.reflect(ksndmgrDings[i].arValue1, "arValue1");
			r.reflect(ksndmgrDings[i].arValue2, "arValue2");
			r.leaveArray();
			r.incrementIndex();
		}
		r.leaveArray();
		r.reflect(ksndmgrUnk14, "ksndmgrUnk14"); // ??
	}
	else if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
		r.reflectSize<uint32_t>(ogSoundOutputEffects, "ogSoundOutputEffects_size");
		r.reflect(ogSoundOutputEffects, "ogSoundOutputEffects");
		r.reflectSize<uint32_t>(ogSoundOutputEffects2, "ogSoundOutputEffects2_size");
		r.reflect(ogSoundOutputEffects2, "ogSoundOutputEffects2");
		r.reflect(ogFloatValues, "ogFloatValues");
		r.reflect(ksndmgrSndDictID, "ksndmgrSndDictID");
		r.reflectSize<uint32_t>(ksndmgrSndDicts, "sizeFor_ksndmgrSndDicts");
		r.reflect(ksndmgrSndDicts, "ksndmgrSndDicts");
	}
}

void CKSoundManager::deserializeGlobal(KEnvironment* kenv, File* file, size_t length)
{
	for (float& f : arGlobFloatValues1)
		f = file->readFloat();
	if (kenv->version == KEnvironment::KVERSION_ARTHUR) {
		for (float& f : arGlobFloatValues2)
			f = file->readFloat();
	}
	else {
		ogGlobStreamWaves.resize(file->readUint32());
		for (auto& sw : ogGlobStreamWaves)
			sw = kenv->readObjRef<CKObject>(file);
		ogGlobStreamTypes.resize(file->readUint32());
		for (auto& st : ogGlobStreamTypes) {
			st.strtName = file->readSizedString<uint16_t>();
			for (float& f : st.strtValues)
				f = file->readFloat();
		}
	}
}

void CKGraphicX2::reflectMembers2(MemberListener& r, KEnvironment* kenv)
{
	r.reflect(kgfcSgRootNode, "kgfcSgRootNode");
	if (kenv->version >= KEnvironment::KVERSION_OLYMPIC)
		r.reflect(kgfcSgRootNode2, "kgfcSgRootNode2");
	r.reflect(ckgUnk1, "ckgUnk1");
	r.reflect(ckgUnk2, "ckgUnk2");
	r.reflect(ckgUnk3, "ckgUnk3");
	r.reflectSize<uint32_t>(ckgGfxManagers, "ckgGfxManagers_size");
	r.reflect(ckgGfxManagers, "ckgGfxManagers");
	r.reflectSize<uint32_t>(ckgVideoReplacements, "ckgVideoReplacements_size");
	r.foreachElement(ckgVideoReplacements, "ckgVideoReplacements", [&](VideoReplacement& vr) {
		r.reflect(vr.texName, "texName");
		r.reflect(vr.ckgUnk17, "ckgUnk17");
		r.reflect(vr.ckgUnk18, "ckgUnk18");
		r.reflect(vr.ckgUnk19, "ckgUnk19");
		r.reflect(vr.ckgUnk20, "ckgUnk20");
		r.reflect(vr.ckgUnk21, "ckgUnk21");
		if (kenv->version >= KEnvironment::KVERSION_OLYMPIC) {
			r.reflect(vr.ogvrUnk1, "ogvrUnk1");
			r.reflect(vr.ogvrUnk2, "ogvrUnk2");
		}
		});
	r.reflect(ckgTexDict, "ckgTexDict");
}

void CKInput::deserializeGlobal(KEnvironment* kenv, File* file, size_t length)
{
	data.resize(length);
	file->read(data.data(), data.size());
}

void CKInput::serializeGlobal(KEnvironment* kenv, File* file)
{
	file->write(data.data(), data.size());
}
