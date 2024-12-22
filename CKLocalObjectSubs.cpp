#include "CKLocalObjectSubs.h"
#include "KEnvironment.h"
#include <numeric>
#include "CoreClasses/CKLogic.h"
#include "CoreClasses/CKGraphical.h"

void Loc_CManager2d::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	if (length == 0) {
		assert(kenv->version >= kenv->KVERSION_ARTHUR && kenv->version <= kenv->KVERSION_OLYMPIC);
		empty = true;
		return;
	}
	empty = false;

	if (kenv->version < kenv->KVERSION_XXL2) {
		rwCheckHeader(file, 0x23);
		piTexDict.deserialize(file);
	}
	else {
		piTexDict.textures.resize(file->readUint32());
		for (auto& tex : piTexDict.textures) {
			tex.texture.name = file->readSizedString<uint16_t>();
			if (kenv->version >= kenv->KVERSION_ARTHUR) {
				uint16_t zero = file->readUint16();
				assert(zero == 0);
			}
			tex.images.resize(1);
			rwCheckHeader(file, 0x18);
			tex.images.front().deserialize(file);
		}
	}

	size_t numFonts;
	if (kenv->version < kenv->KVERSION_XXL2) {
		// Number of fonts from CManager2d in GAME.KWN
		auto it = std::find_if(kenv->globalObjects.begin(), kenv->globalObjects.end(), [](CKObject* obj) {return obj->isSubclassOf<CManager2d>(); });
		assert(it != kenv->globalObjects.end());
		CManager2d* gmgr2d = (*it)->cast<CManager2d>();
		numFonts = gmgr2d->numFonts;
	}
	else {
		// XXL2+ put it back to the *GLOC.KWN (though still present in GAME.KWN)
		numFonts = file->readUint32();
	}

	fonts.resize(numFonts);
	for (auto &font : fonts) {
		font.fontId = file->readUint32();
		rwCheckHeader(file, 0x199);
		font.rwFont.deserialize(file);
		if (kenv->version >= kenv->KVERSION_XXL2)
			font.x2Name = file->readSizedString<uint16_t>();
	}
}

void Loc_CManager2d::serialize(KEnvironment * kenv, File * file)
{
	if (empty)
		return;

	if (kenv->version < kenv->KVERSION_XXL2) {
		piTexDict.serialize(file);
	}
	else {
		file->writeUint32(piTexDict.textures.size());
		for (auto& tex : piTexDict.textures) {
			file->writeSizedString<uint16_t>(tex.texture.name);
			if (kenv->version >= kenv->KVERSION_ARTHUR)
				file->writeUint16(0);
			tex.images.front().serialize(file);
		}
	}
	if (kenv->version >= kenv->KVERSION_XXL2) {
		file->writeUint32(fonts.size());
	}
	for (auto &font : fonts) {
		file->writeUint32(font.fontId);
		font.rwFont.serialize(file);
		if (kenv->version >= kenv->KVERSION_XXL2)
			file->writeSizedString<uint16_t>(font.x2Name);
	}
}

void Loc_CLocManager::deserialize(KEnvironment * kenv, File * file, size_t length)
{
	numLanguages = file->readUint16();
	if (kenv->version < KEnvironment::KVERSION_OLYMPIC || kenv->version >= KEnvironment::KVERSION_ALICE) {
		for (int i = 0; i < numLanguages; i++)
			langStrIndices.push_back(file->readUint32());
	}
	for (int i = 0; i < numLanguages; i++)
		langIDs.push_back(file->readUint32());
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		for (int i = 0; i < numLanguages; i++)
			langArIndices.push_back(file->readUint32());
	}
	if (kenv->version >= kenv->KVERSION_SPYRO || (kenv->version == kenv->KVERSION_OLYMPIC && kenv->platform == kenv->PLATFORM_X360))
		spUnk0 = file->readUint32();

	// Get number of strings from global CLocManager (in GAME.K*)
	auto it = std::find_if(kenv->globalObjects.begin(), kenv->globalObjects.end(), [](CKObject* obj) {return obj->isSubclassOf<CLocManager>(); });
	assert(it != kenv->globalObjects.end());
	CLocManager* glmgr = (*it)->cast<CLocManager>();
	int numStdStrings = (int)glmgr->numStdStrings;
	int numTrcStrings = (int)glmgr->numTrcStrings;

	// TRC (Strings with IDs)
	auto readTRC = [&]() {
		uint32_t totalChars = file->readUint32();
		uint32_t accChars = 0;
		for (int s = 0; s < numTrcStrings; s++) {
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
	};

	// Standard strings (without IDs, but still indexed of course)
	auto readStd = [&]() {
		uint32_t totalChars = file->readUint32();
		uint32_t accChars = 0;
		for (int s = 0; s < numStdStrings; s++) {
			uint32_t len = file->readUint32();
			std::wstring str;
			for (uint32_t i = 0; i < len; i++)
				str.push_back(file->readUint16());
			stdStrings.push_back(std::move(str));
			accChars += len;
		}
		if (accChars != totalChars)
			printf("Incorrect char total!\n");
	};

	if (kenv->version < kenv->KVERSION_OLYMPIC) {
		readTRC();
		readStd();
	}
	else {
		readStd();
		readTRC();
	}
}

void Loc_CLocManager::serialize(KEnvironment * kenv, File * file)
{
	file->writeUint16(numLanguages);
	if (kenv->version < KEnvironment::KVERSION_OLYMPIC || kenv->version >= KEnvironment::KVERSION_ALICE) {
		for (int i = 0; i < numLanguages; i++)
			file->writeUint32(langStrIndices[i]);
	}
	for (int i = 0; i < numLanguages; i++)
		file->writeUint32(langIDs[i]);
	if (kenv->version >= kenv->KVERSION_ARTHUR) {
		for (int i = 0; i < numLanguages; i++)
			file->writeUint32(langArIndices[i]);
	}
	if (kenv->version >= kenv->KVERSION_SPYRO || (kenv->version == kenv->KVERSION_OLYMPIC && kenv->platform == kenv->PLATFORM_X360))
		file->writeUint32(spUnk0);

	auto writeTRC = [&]() {
		file->writeUint32(std::accumulate(trcStrings.begin(), trcStrings.end(), (uint32_t)0, [](uint32_t prev, const auto& trc) { return prev + trc.second.size(); }));
		for (auto& trc : trcStrings) {
			file->writeUint32(trc.first);
			file->writeUint32(trc.second.size());
			file->write(trc.second.data(), 2 * trc.second.size());
		}
	};

	auto writeStd = [&]() {
		file->writeUint32(std::accumulate(stdStrings.begin(), stdStrings.end(), (uint32_t)0, [](uint32_t prev, const std::wstring& std) { return prev + std.size(); }));
		for (auto& std : stdStrings) {
			file->writeUint32(std.size());
			file->write(std.data(), 2 * std.size());
		}
	};
	
	if (kenv->version < kenv->KVERSION_OLYMPIC) {
		writeTRC();
		writeStd();
	}
	else {
		writeStd();
		writeTRC();
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

void Loc_CKSrvSekensor::deserialize(KEnvironment* kenv, File* file, size_t length)
{
	auto startOffset = file->tell();
	while (file->tell() - startOffset < length) {
		LocalizedSekens& sek = locSekens.emplace_back();
		if (kenv->version < KEnvironment::KVERSION_ARTHUR)
			sek.totalTime = file->readFloat();
		uint8_t numLines = file->readUint8();
		sek.numVoiceLines = file->readUint8();
		sek.locLines.resize(numLines);
		for (LocalizedLine& line : sek.locLines)
			line.duration = file->readFloat();
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			for (LocalizedLine& line : sek.locLines)
				line.oneFloat = file->readFloat();
			for (LocalizedLine& line : sek.locLines)
				line.someByte = file->readUint8();
			sek.arSekensIndex = file->readUint32();
			if (kenv->version == KEnvironment::KVERSION_ARTHUR)
				sek.arUnkValue = file->readUint32();
		}
	}
}

void Loc_CKSrvSekensor::serialize(KEnvironment* kenv, File* file)
{
	for(LocalizedSekens& sek : locSekens) {
		if (kenv->version < KEnvironment::KVERSION_ARTHUR)
			file->writeFloat(sek.totalTime);
		file->writeUint8(sek.locLines.size());
		file->writeUint8(sek.numVoiceLines);
		for (LocalizedLine& line : sek.locLines)
			file->writeFloat(line.duration);
		if (kenv->version >= KEnvironment::KVERSION_ARTHUR) {
			for (LocalizedLine& line : sek.locLines)
				file->writeFloat(line.oneFloat);
			for (LocalizedLine& line : sek.locLines)
				file->writeUint8(line.someByte);
			file->writeUint32(sek.arSekensIndex);
			if (kenv->version == KEnvironment::KVERSION_ARTHUR)
				file->writeUint32(sek.arUnkValue);
		}
	}
}
