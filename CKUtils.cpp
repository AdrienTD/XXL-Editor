#include "CKUtils.h"
#include "File.h"
#include "vecmat.h"
#include "KEnvironment.h"

void ReadingMemberListener::reflect(uint8_t & ref, const char * name) { ref = file->readUint8(); }

void ReadingMemberListener::reflect(uint16_t & ref, const char * name) { ref = file->readUint16(); }

void ReadingMemberListener::reflect(uint32_t & ref, const char * name) { ref = file->readUint32(); }

void ReadingMemberListener::reflect(float & ref, const char * name) { ref = file->readFloat(); }

void ReadingMemberListener::reflectAnyRef(kanyobjref & ref, int clfid, const char * name) {
	CKObject *obj = kenv->readObjPnt(file);
	if (clfid != -1) assert(obj->isSubclassOfID(clfid));
	ref.anyreset(obj);
}

void ReadingMemberListener::reflect(Vector3 & ref, const char * name) { for (float &f : ref) f = file->readFloat(); }

void WritingMemberListener::reflect(uint8_t & ref, const char * name) { file->writeUint8(ref); }

void WritingMemberListener::reflect(uint16_t & ref, const char * name) { file->writeUint16(ref); }

void WritingMemberListener::reflect(uint32_t & ref, const char * name) { file->writeUint32(ref); }

void WritingMemberListener::reflect(float & ref, const char * name) { file->writeFloat(ref); }

void WritingMemberListener::reflectAnyRef(kanyobjref & ref, int clfid, const char * name) {
	if (clfid != -1) assert(ref._pointer->isSubclassOfID(clfid));
	kenv->writeObjID(file, ref._pointer);
}

void WritingMemberListener::reflect(Vector3 & ref, const char * name) { for (float &f : ref) file->writeFloat(f); }
