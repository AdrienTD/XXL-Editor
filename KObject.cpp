#include "KObject.h"
#include "File.h"
#include <cstdlib>

std::unordered_map<CKObject*, int> CKObject::refCounts;
std::unordered_map<CKObject*, int> CKObject::objIdMap;
std::set<std::pair<int, int>> CKUnknown::hits;

bool CKUnknown::isSubclassOfID(uint32_t fid)
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

std::span<const int> CKUnknown::getClassHierarchy()
{
	return std::span<const int>();
}

void CKUnknown::deserialize(KEnvironment* kenv, File * file, size_t length) {
	this->mem.resize(length);
	this->offset = (uint32_t)file->tell();
	if (length > 0) {
		file->read(this->mem.data(), this->mem.size());
	}
}

void CKUnknown::serialize(KEnvironment* kenv, File * file) {
	if (this->mem.size() > 0) {
		file->write(this->mem.data(), this->mem.size());
	}
}

void CKUnknown::deserializeLvlSpecific(KEnvironment * kenv, File * file, size_t length)
{
	this->lsMem.resize(length);
	this->lvlSpecificOffset = (uint32_t)file->tell();
	if (length > 0) {
		file->read(this->lsMem.data(), this->lsMem.size());
	}
}

void CKUnknown::serializeLvlSpecific(KEnvironment * kenv, File * file)
{
	if (this->lsMem.size() > 0) {
		file->write(this->lsMem.data(), this->lsMem.size());
	}
}

CKUnknown::CKUnknown(const CKUnknown& another) = default;

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
