#include "Events.h"
#include "File.h"
#include "KEnvironment.h"
#include "CKService.h"

void EventNode::write(KEnvironment * kenv, File * file) const {
	uint16_t enc = (seqIndex << 3) | bit;
	file->writeUint16(enc);
}

void EventNode::read(KEnvironment * kenv, File * file, CKObject *user) {
	int16_t enc = (int16_t)file->readUint16();
	bit = enc & 7;
	seqIndex = enc >> 3;
	if (seqIndex != -1 && kenv->hasClass<CKSrvEvent>()) {
		if (CKSrvEvent *srvEvent = kenv->levelObjects.getFirst<CKSrvEvent>()) {
			srvEvent->bees[seqIndex].users.push_back(user);
			srvEvent->bees[seqIndex].userFound = true;
		}
	}
}
