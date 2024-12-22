#include "Events.h"
#include "File.h"
#include "KEnvironment.h"
#include "CoreClasses/CKService.h"
#include "CoreClasses/CKLogic.h"

void EventNodeX1::write(KEnvironment * kenv, File * file) const {
	CKSrvEvent* srvEvent = kenv->levelObjects.getFirst<CKSrvEvent>();
	assert(srvEvent);

	int16_t actualSeqIndex; uint8_t actualBit;
	auto& ids = srvEvent->evtSeqIDs;
	auto it = std::find(ids.begin(), ids.end(), seqIndex);
	if (it != ids.end()) {
		actualSeqIndex = (int16_t)(it - ids.begin());
		actualBit = bit;
	}
	else {
		actualSeqIndex = -1;
		actualBit = 0;
	}

	uint16_t enc = (actualSeqIndex << 3) | actualBit;
	file->writeUint16(enc);
}

void EventNodeX1::read(KEnvironment * kenv, File * file, CKObject *user) {
	int16_t enc = (int16_t)file->readUint16();
	bit = enc & 7;
	seqIndex = enc >> 3;
	if (seqIndex != -1 && kenv->hasClass<CKSrvEvent>()) {
		if (CKSrvEvent *srvEvent = kenv->levelObjects.getFirst<CKSrvEvent>()) {
			srvEvent->sequences[seqIndex].users.push_back(user);
			srvEvent->sequences[seqIndex].userFound = true;
		}
	}
}

void MarkerIndex::write(KEnvironment* kenv, File* file) const
{
	file->writeInt32(index);
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		file->writeInt32(arSecondIndex);
}

void MarkerIndex::read(KEnvironment* kenv, File* file)
{
	index = file->readInt32();
	if (kenv->version >= KEnvironment::KVERSION_ARTHUR)
		arSecondIndex = file->readInt32();
}

void EventNodeX2::write(KEnvironment* kenv, File* file)
{
	file->writeUint32((uint32_t)datas.size());
	for (auto& ref : datas)
		kenv->writeObjID(file, ref.get());
}

void EventNodeX2::read(KEnvironment* kenv, File* file, CKObject* user)
{
	datas.resize(file->readUint32());
	for (auto& ref : datas)
		ref = KWeakRef<CKComparedData>(kenv->readObjPnt(file)->cast<CKComparedData>());
}

void EventNodeX2::clean()
{
	auto it = std::remove_if(datas.begin(), datas.end(), [](KWeakRef<CKComparedData>& ref) {return !ref; });
	datas.erase(it, datas.end());
}

void EventNode::write(KEnvironment* kenv, File* file)
{
	if (kenv->version < KEnvironment::KVERSION_XXL2)
		enx1.write(kenv, file);
	else
		enx2.write(kenv, file);
}

void EventNode::read(KEnvironment* kenv, File* file, CKObject* user)
{
	if (kenv->version < KEnvironment::KVERSION_XXL2)
		enx1.read(kenv, file, user);
	else
		enx2.read(kenv, file, user);
}
