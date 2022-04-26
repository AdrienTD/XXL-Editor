#include "Events.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKService.h"

void EventNode::write(KEnvironment * kenv, File * file) const {
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

void EventNode::read(KEnvironment * kenv, File * file, CKObject *user) {
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
