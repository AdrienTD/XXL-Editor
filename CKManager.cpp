#include "CKManager.h"
#include "File.h"
#include "KEnvironment.h"

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
	CKReflectableManager::reflectMembers(r);
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
	CKReflectableManager::reflectMembers(r);
	r.reflectSize<uint32_t>(ksndmgrSndDicts, "sizeFor_ksndmgrSndDicts");
	r.reflect(ksndmgrSndDicts, "ksndmgrSndDicts");
	r.reflect(ksndmgrSndDictID, "ksndmgrSndDictID");
	r.reflect(ksndmgrUnk3, "ksndmgrUnk3");
	r.reflect(ksndmgrUnk4, "ksndmgrUnk4");
	r.reflect(ksndmgrUnk5, "ksndmgrUnk5");
	r.reflect(ksndmgrUnk6, "ksndmgrUnk6");
	r.reflect(ksndmgrUnk7, "ksndmgrUnk7");
	r.reflect(ksndmgrUnk8, "ksndmgrUnk8");
	r.reflectSize<uint32_t>(ksndmgrDings, "sizeFor_ksndmgrDings");
	//r.reflect(ksndmgrDings, "ksndmgrDings"); // <--- TODO
	for (size_t i = 0; i < ksndmgrDings.size(); i++) {
		std::string en = "ksndmgrDings[" + std::to_string(i) + "].";
		if(kenv->isRemaster)
			r.reflect(ksndmgrDings[i].remasterPath, (en + "remasterPath").c_str());
		r.reflect(ksndmgrDings[i].value, (en + "value").c_str());
	}
	r.reflect(ksndmgrUnk11, "ksndmgrUnk11");
	r.reflect(ksndmgrUnk12, "ksndmgrUnk12");
	r.reflect(ksndmgrUnk13, "ksndmgrUnk13");
	r.reflect(ksndmgrUnk14, "ksndmgrUnk14");
	r.reflect(ksndmgrSampleRate, "ksndmgrSampleRate");
}