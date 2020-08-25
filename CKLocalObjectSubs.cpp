#include "CKLocalObjectSubs.h"
#include <numeric>

void Loc_CManager2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	size_t startoff = file->tell();
	rwCheckHeader(file, 0x23);
	piTexDict.deserialize(file);

	rest.resize(length - (file->tell() - startoff));
	file->read(rest.data(), rest.size());
}

void Loc_CManager2d::serialize(KEnvironment * kenv, File * file)
{
	piTexDict.serialize(file);
	file->write(rest.data(), rest.size());
}

void Loc_CLocManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numThings = file->readUint16();
	for (int i = 0; i < numThings; i++)
		thingTable1.push_back(file->readUint32());
	for (int i = 0; i < numThings; i++)
		thingTable2.push_back(file->readUint32());

	// TRC (Strings with IDs)
	uint32_t totalChars = file->readUint32();
	uint32_t accChars = 0;
	while (accChars < totalChars) {
		uint32_t id = file->readUint32();
		uint32_t len = file->readUint32();
		std::wstring str;
		for (uint32_t i = 0; i < len; i++)
			str.push_back(file->readUint16());
		trcStrings.emplace_back(id, std::move(str));
		accChars += len;
	}
	if (accChars != totalChars)
		printf("Incorrect char total!\n");

	// Standard strings (without IDs, but still indexed of course)
	totalChars = file->readUint32();
	accChars = 0;
	while (accChars < totalChars) {
		uint32_t len = file->readUint32();
		std::wstring str;
		for (uint32_t i = 0; i < len; i++)
			str.push_back(file->readUint16());
		stdStrings.push_back(std::move(str));
		accChars += len;
	}
	if (accChars != totalChars)
		printf("Incorrect char total!\n");
}

void Loc_CLocManager::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(numThings);
	for (int i = 0; i < numThings; i++)
		file->writeUint32(thingTable1[i]);
	for (int i = 0; i < numThings; i++)
		file->writeUint32(thingTable2[i]);

	file->writeUint32(std::accumulate(trcStrings.begin(), trcStrings.end(), (uint32_t)0, [](uint32_t prev, const auto &trc) { return prev + trc.second.size(); }));
	for (auto &trc : trcStrings) {
		file->writeUint32(trc.first);
		file->writeUint32(trc.second.size());
		file->write(trc.second.data(), 2 * trc.second.size());
	}

	file->writeUint32(std::accumulate(stdStrings.begin(), stdStrings.end(), (uint32_t)0, [](uint32_t prev, const std::wstring &std) { return prev + std.size(); }));
	for (auto &std : stdStrings) {
		file->writeUint32(std.size());
		file->write(std.data(), 2 * std.size());
	}

}
