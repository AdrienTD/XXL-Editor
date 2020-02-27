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

void CKUnknown::deserialize(KEnvironment* kenv, File * file, size_t length) {
	mem = malloc(length);
	file->read(mem, length);
	this->length = length;
}

void CKUnknown::serialize(File * file) {
	file->write(mem, length);
}

CKUnknown::~CKUnknown() { if (mem) free(mem); }
