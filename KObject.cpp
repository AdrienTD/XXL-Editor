#include "KObject.h"
#include "File.h"
#include <cstdlib>

bool CKUnknown::isSubclassOf(uint32_t fid)
{
	return false;
}

int CKUnknown::getClassCategory()
{
	return clCategory;
}

int CKUnknown::getClassID()
{
	return clId;
}

const char * CKUnknown::getClassName()
{
	return "?";
}

void CKUnknown::deserialize(KEnvironment* kenv, File * file, size_t length) {
	mem = malloc(length);
	file->read(mem, length);
	this->length = length;
}

void CKUnknown::serialize(KEnvironment* kenv, File * file) {
	file->write(mem, length);
}

CKUnknown::CKUnknown(const CKUnknown & another)
{
	clCategory = another.clCategory; clId = another.clId; length = another.length;
	mem = malloc(length);
	memcpy(mem, another.mem, length);
}

CKUnknown::~CKUnknown() { if (mem) free(mem); }

void CKObject::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	printf("Deserialization unimplemented, skipping\n");
	file->seek(length, SEEK_CUR);
}

void CKObject::serialize(KEnvironment* kenv, File * file)
{
	printf("Serialization unimplemented, writing random stuff\n");
	const char *stuff = "Unimplemented, please fix this! ;)";
	file->write(stuff, strlen(stuff));
}
