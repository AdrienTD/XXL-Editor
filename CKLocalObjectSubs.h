#pragma once

#include <vector>
#include "rw.h"
#include "KLocalObject.h"
#include "DynArray.h"

struct Loc_CKGraphic : KLocalObjectSub<0, 2> {
	struct CKGTexture {
		std::string name;
		RwImage img;
	};
	std::vector<CKGTexture> textures;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	KLocalObject *clone() override { return new Loc_CKGraphic(*this); }
};

struct Loc_CLocManager : KLocalObjectSub<12, 59> {
	std::vector<std::pair<uint32_t, std::wstring>> trcStrings;
	std::vector<std::wstring> stdStrings;
	uint16_t numLanguages;
	std::vector<uint32_t> langStrIndices, langIDs, langArIndices;
	uint32_t spUnk0 = 0;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	KLocalObject *clone() override { return new Loc_CLocManager(*this); }
};

struct Loc_CManager2d : KLocalObjectSub<13, 16> {
	struct Font {
		uint32_t fontId;
		RwFont2D rwFont;
		std::string x2Name;
	};

	RwPITexDict piTexDict;
	std::vector<Font> fonts;

	bool empty = true;

	void deserialize(KEnvironment* kenv, File *file, size_t length) override;
	void serialize(KEnvironment* kenv, File *file) override;
	KLocalObject *clone() override { return new Loc_CManager2d(*this); }
};

struct Loc_CKSrvSekensor : KLocalObjectSub<1, 10> {
	struct LocalizedLine {
		float duration = 0.0f;
		// Arthur+:
		float oneFloat = 1.0f;
		uint8_t someByte = 0;
	};
	struct LocalizedSekens {
		float totalTime = 0.0f;
		//uint8_t numLines = 0;
		uint8_t numVoiceLines = 0;
		std::vector<LocalizedLine> locLines;
		// Arthur+:
		uint32_t arSekensIndex = 0;
		uint32_t arUnkValue = 0; // removed in OG
	};
	std::vector<LocalizedSekens> locSekens;

	void deserialize(KEnvironment* kenv, File* file, size_t length) override;
	void serialize(KEnvironment* kenv, File* file) override;
	KLocalObject* clone() override { return new Loc_CKSrvSekensor(*this); }
};