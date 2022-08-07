#include "CKUtils.h"
#include "File.h"
#include "vecmat.h"
#include "KEnvironment.h"
#include "Events.h"

void ReadingMemberListener::reflect(uint8_t & ref, const char * name) { ref = file->readUint8(); }

void ReadingMemberListener::reflect(uint16_t & ref, const char * name) { ref = file->readUint16(); }

void ReadingMemberListener::reflect(uint32_t & ref, const char * name) { ref = file->readUint32(); }

void ReadingMemberListener::reflect(float & ref, const char * name) { ref = file->readFloat(); }

void ReadingMemberListener::reflectAnyRef(kanyobjref & ref, int clfid, const char * name) {
	CKObject *obj = kenv->readObjPnt(file);
	//if (obj && clfid != -1) assert(obj->isSubclassOfID(clfid));
	ref.anyreset(obj);
}

void ReadingMemberListener::reflect(Vector3 & ref, const char * name) { for (float &f : ref) f = file->readFloat(); }

void ReadingMemberListener::reflect(EventNode & ref, const char * name, CKObject * user) { ref.read(kenv, file, user); }

void ReadingMemberListener::reflect(MarkerIndex& ref, const char* name) { ref.read(kenv, file); }

void ReadingMemberListener::reflectAnyPostRef(KAnyPostponedRef& postref, int clfid, const char* name) {	postref.read(file); }

void ReadingMemberListener::reflect(std::string & ref, const char * name) { ref = file->readString(file->readUint16()); }



void WritingMemberListener::reflect(uint8_t & ref, const char * name) { file->writeUint8(ref); }

void WritingMemberListener::reflect(uint16_t & ref, const char * name) { file->writeUint16(ref); }

void WritingMemberListener::reflect(uint32_t & ref, const char * name) { file->writeUint32(ref); }

void WritingMemberListener::reflect(float & ref, const char * name) { file->writeFloat(ref); }

void WritingMemberListener::reflectAnyRef(kanyobjref & ref, int clfid, const char * name) {
	//if (ref._pointer && clfid != -1) assert(ref._pointer->isSubclassOfID(clfid));
	kenv->writeObjID(file, ref._pointer);
}

void WritingMemberListener::reflect(Vector3 & ref, const char * name) { for (float &f : ref) file->writeFloat(f); }

void WritingMemberListener::reflect(EventNode & ref, const char * name, CKObject * user) { ref.write(kenv, file); }

void WritingMemberListener::reflect(MarkerIndex& ref, const char* name) { ref.write(kenv, file); }

void WritingMemberListener::reflectAnyPostRef(KAnyPostponedRef& postref, int clfid, const char* name) { postref.write(kenv, file); }

void WritingMemberListener::reflect(std::string & ref, const char * name) { file->writeUint16(ref.size()); file->write(ref.data(), ref.size()); }



void MemberListener::reflect(Matrix & ref, const char * name) {
	char txt[32];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			sprintf_s(txt, "%s[%u][%u]", name, i, j);
			reflect(ref.m[i][j], txt);
		}
	}
}
