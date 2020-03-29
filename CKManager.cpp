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
	for (objref<CKService> &srv : this->services)
		kenv->writeObjRef(file, srv);
}
