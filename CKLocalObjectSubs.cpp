#include "CKLocalObjectSubs.h"
#include "KEnvironment.h"
#include <numeric>

void Loc_CManager2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	size_t startoff = file->tell();
	rwCheckHeader(file, 0x23);
	piTexDict.deserialize(file);

	fonts.resize(2); // TODO: number of fonts in CManager2d in GAME.KWN, let's assume 2 for now...
	for (auto &font : fonts) {
		font.first = file->readUint32();
		rwCheckHeader(file, 0x199);
		font.second.deserialize(file);
	}
}

void Loc_CManager2d::serialize(KEnvironment * kenv, File * file)
{
	piTexDict.serialize(file);
	for (auto &font : fonts) {
		file->writeUint32(font.first);
		font.second.serialize(file);
	}
}

void Loc_CLocManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numLanguages = file->readUint16();
	for (int i = 0; i < numLanguages; i++)
		langStrIndices.push_back(file->readUint32());
	for (int i = 0; i < numLanguages; i++)
		langIDs.push_back(file->readUint32());

	// TRC (Strings with IDs)
	uint32_t totalChars = file->readUint32();
	uint32_t accChars = 0;
	//while (accChars < totalChars) {
	for (int s = 0; s < 0x2f; s++) { // TODO: Read amount from GAME.KWN!
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
	//while (accChars < totalChars) {
	for (int s = 0; s < 0x42a; s++) {
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
	file->writeUint16(numLanguages);
	for (int i = 0; i < numLanguages; i++)
		file->writeUint32(langStrIndices[i]);
	for (int i = 0; i < numLanguages; i++)
		file->writeUint32(langIDs[i]);

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

void Loc_CKGraphic::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	textures.resize(file->readUint32());
	for (CKGTexture &tex : textures) {
		uint16_t nameLength = file->readUint16();
		tex.name = file->readString(nameLength);
		rwCheckHeader(file, 0x18);
		tex.img.deserialize(file);
	}
}

void Loc_CKGraphic::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint32(textures.size());
	for (CKGTexture &tex : textures) {
		file->writeUint16((uint16_t)tex.name.size());
		file->write(tex.name.data(), (uint16_t)tex.name.size());
		tex.img.serialize(file);
	}
}
