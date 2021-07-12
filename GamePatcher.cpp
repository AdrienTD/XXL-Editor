#include "GamePatcher.h"
#include "KEnvironment.h"
#include "File.h"
#include "GamePatcherKClasses.h"
#include <stack>
#include <cstdarg>

namespace GamePatcher {

	struct GamePatcherKEnvironment : KEnvironment {
		void loadGame(const char* path, const char* outPath, int version, int platform) {
			this->gamePath = path;
			this->version = version;
			this->platform = platform;
			this->outGamePath = outPath;
		}
		void loadLevelHeader(File& lvlFile)
		{
			this->numSectors = lvlFile.readUint8();
			this->lvlUnk2 = lvlFile.readUint32();

			this->sectorObjects.resize(this->numSectors);

			for (int clcat = 0; clcat < 15; clcat++) {
				uint16_t numClasses = lvlFile.readUint16();
				this->levelObjects.categories[clcat].type.resize(numClasses);
				for (uint16_t clid = 0; clid < numClasses; clid++) {
					uint32_t fid = clcat | (clid << 6);
					uint16_t numTotalObjects = lvlFile.readUint16();
					uint16_t numLevelObjects = lvlFile.readUint16();
					uint8_t info = lvlFile.readUint8();

					auto& lvltype = this->levelObjects.categories[clcat].type[clid];
					lvltype.info = info;
					lvltype.totalCount = numTotalObjects;
					lvltype.startId = 0;
					lvltype.objects.reserve(numLevelObjects);
					for (uint16_t i = 0; i < numLevelObjects; i++) {
						lvltype.objects.push_back(constructObject(fid));
						//printf("Constructed %s\n", lvltype.objects.back()->getClassName());
					}
				}
			}

			for (int clcat = 0; clcat < 15; clcat++) {
				auto& type = this->levelObjects.categories[clcat].type;
				for (size_t clid = 0; clid < type.size(); clid++) {
					//printf("Class (%i,%i) : %i %i %i\n", clcat, clid, type[clid].totalCount,  type[clid].objects.size(), type[clid].info);
				}
			}
		}

		void loadLevel(int lvlNumber, File* headerFile, File* drmValues1)
		{
			if (levelLoaded)
				unloadLevel();

			this->loadingSector = -1;
			char lvlfn[500];
			sprintf_s(lvlfn, "%s/LVL%03u/LVL%02u.%s", gamePath.c_str(), lvlNumber, lvlNumber, platformExt[platform]);

			IOFile lvlFile(std::filesystem::u8path(lvlfn).c_str(), "rb");
			//if (platform == PLATFORM_PC) {
			//	std::string asthead = lvlFile.readString(8);
			//	assert(asthead == "Asterix ");
			//}
			this->lvlUnk1 = lvlFile.readUint32();
			uint32_t drmSize = lvlFile.readUint32(); // DRM
			if (headerFile) {
				loadLevelHeader(*headerFile);
				lvlFile.seek(drmSize, SEEK_CUR);
			}
			else {
				loadLevelHeader(lvlFile);
				lvlFile.seek(4, SEEK_CUR);
			}

			lvlFile.seek(8 /*+ 4 (drm)*/, SEEK_CUR);

			g_drmValues = drmValues1;

			for (int clcat : clcatReorder) {
				uint16_t numClasses = lvlFile.readUint16();
				uint32_t nextCat = lvlFile.readUint32();
				printf("Cat %i at %08X, next at %08X, numClasses = %i\n", clcat, lvlFile.tell(), nextCat, numClasses);
				for (size_t clid = 0; clid < this->levelObjects.categories[clcat].type.size(); clid++) {
					if (this->levelObjects.categories[clcat].type[clid].objects.empty())
						continue;
					uint32_t nextClass = lvlFile.readUint32();
					//printf("Class %i %i at %08X, next at %08X\n", clcat, clid, lvlFile.tell(), nextClass);
					if (this->levelObjects.categories[clcat].type[clid].info) {
						uint16_t startid = lvlFile.readUint16();
						assert(startid == 0);
					}
					//printf("* %08X\n", lvlFile.tell());
					for (CKObject* obj : this->levelObjects.categories[clcat].type[clid].objects) {
						uint32_t nextObjOffset = lvlFile.readUint32();
						obj->deserialize(this, &lvlFile, nextObjOffset - lvlFile.tell());
						assert(lvlFile.tell() == nextObjOffset);
					}
					assert(lvlFile.tell() == nextClass);
					numClasses--;
				}
				assert(numClasses == 0);
				assert(lvlFile.tell() == nextCat);
			}

			//for(int i = 0; i < this->numSectors; i++)
			//	loadSector(i, lvlNumber);

			this->loadingSector = -1;
			for (auto& cat : levelObjects.categories)
				for (auto& cl : cat.type)
					for (CKObject* obj : cl.objects)
						obj->onLevelLoaded(this);
			for (auto& str : sectorObjects)
				for (auto& cat : str.categories)
					for (auto& cl : cat.type)
						for (CKObject* obj : cl.objects)
							obj->onLevelLoaded(this);

			levelLoaded = true;
		}

		void prepareSavingMapGP()
		{
			saveMap.clear();
			for (int clcat = 0; clcat < 15; clcat++) {
				auto& lvlcat = levelObjects.categories[clcat];
				for (int clid = 0; clid < lvlcat.type.size(); clid++) {
					auto& lvltype = lvlcat.type[clid];
					for (int i = 0; i < lvltype.objects.size(); i++)
						saveMap[lvltype.objects[i]] = clcat | (clid << 6) | (i << 17);
				}
			}
		}

		void saveLevel(int lvlNumber)
		{
			struct OffsetStack {
				std::stack<uint32_t> offsets;
				File* file;
				void push()
				{
					offsets.push(file->tell());
					file->writeUint32(0);
				}
				void pop()
				{
					uint32_t endpos = file->tell();
					uint32_t prevpos = offsets.top();
					file->seek(prevpos, SEEK_SET);
					file->writeUint32(endpos);
					offsets.pop();
					file->seek(endpos, SEEK_SET);
				}
				OffsetStack(File* file) : file(file) {}
			};

			//lvlNumber = 69;
			char lvlfn[300];
			sprintf_s(lvlfn, "%s/LVL%03u/LVL%02u.%s", outGamePath.c_str(), lvlNumber, lvlNumber, platformExt[platform]);

			prepareSavingMapGP();

			IOFile lvlFile(std::filesystem::u8path(lvlfn).c_str(), "wb");
			OffsetStack offsetStack(&lvlFile);
			lvlFile.write("Asterix ", 8);
			lvlFile.writeUint32(this->lvlUnk1);
			lvlFile.writeUint8(this->numSectors);
			lvlFile.writeUint32(this->lvlUnk2);

			for (auto& cat : this->levelObjects.categories) {
				lvlFile.writeUint16(cat.type.size());
				for (auto& kcl : cat.type) {
					lvlFile.writeUint16(kcl.totalCount);
					lvlFile.writeUint16(kcl.objects.size());
					lvlFile.writeUint8(kcl.info);
				}
			}

			lvlFile.writeUint32(0);
			lvlFile.writeUint32(0);

			for (int nclcat : clcatReorder) {
				auto& cat = this->levelObjects.categories[nclcat];

				uint32_t clcnt = 0;
				for (auto& kcl : cat.type)
					if (!kcl.objects.empty())
						clcnt++;

				lvlFile.writeUint16(clcnt);
				offsetStack.push();

				for (auto& kcl : cat.type) {
					if (!kcl.objects.empty()) {
						offsetStack.push();
						if (kcl.info) {
							lvlFile.writeUint16(0); // startid
						}
						for (CKObject* obj : kcl.objects) {
							offsetStack.push();
							obj->serialize(this, &lvlFile);
							offsetStack.pop();
						}
						offsetStack.pop();
					}
				}
				offsetStack.pop();
			}

			//for (int i = 0; i < this->numSectors; i++)
			//	saveSector(i, lvlNumber);
		}
	};

	size_t findmem(const void* mem, size_t start, size_t memSize, const void* sign, size_t signSize)
	{
		size_t maxAddress = memSize - signSize;
		for (size_t p = start; p <= maxAddress; p++)
			if (memcmp((char*)mem + p, sign, signSize) == 0)
				return p;
		return -1;
	}

	struct MemBuffer {
		void* mem = nullptr;
		size_t size = 0;
		MemBuffer() {}
		MemBuffer(size_t size) : size(size) { mem = malloc(size); }
		MemBuffer(FILE* file) {
			fseek(file, 0, SEEK_END);
			size = ftell(file);
			fseek(file, 0, SEEK_SET);
			mem = malloc(size);
			fread(mem, size, 1, file);
		}
		~MemBuffer() { if (mem) free(mem); }
	};

	void GamePatcherThreadX1::start()
	{
		namespace fs = std::filesystem;
		progress = 0;

		// Read GameModule
		FILE* elbFile;
		_wfopen_s(&elbFile, elbPath.c_str(), L"rb");
		if (!elbFile) { throw GamePatcherException("Could not open the GameModule"); }
		MemBuffer elbData(elbFile);
		fclose(elbFile);

		static const unsigned char headerSignature[29] = {
			0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
			0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00,
			0x01, 0x00, 0x00, 0x0F, 0x00
		};

		static const unsigned char drmValSignature[32] = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x41,
			0x0C, 0x00, 0x01, 0x00, 0x44, 0x00, 0x01, 0x00
		};

		static const uint32_t drmValExeOffsets[9] = { 0, 0x2e934c, 0x2e937c, 0x2e9460, 0x2e9538, 0x2e9660, 0x2e975c,
			0x2e9344, 0x2e933c }; // two last ones might have been swapped, but they point to identical data anyway

		size_t drmValStart = findmem(elbData.mem, 0, elbData.size, drmValSignature, sizeof(drmValSignature));
		if (drmValStart == -1) { throw GamePatcherException("Could not find DRM-protected values in GameModule"); }

		size_t elbFindStart = 0;
		char ffn[512];

		GamePatcherKEnvironment kenv;
		kenv.addFactory<CKHkTorch>();
		kenv.addFactory<CKGrpSquadEnemy>();
		kenv.addFactory<CKGrpSquadJetPack>();
		kenv.addFactory<CKEnemyCpnt>();
		kenv.addFactory<CKSquadEnemyCpnt>();
		kenv.addFactory<CKSeizableEnemyCpnt>();
		kenv.addFactory<CKSquadSeizableEnemyCpnt>();
		kenv.addFactory<CKBasicEnemyCpnt>();
		kenv.addFactory<CKBasicEnemyLeaderCpnt>();
		kenv.addFactory<CKJumpingRomanCpnt>();
		kenv.addFactory<CKRomanArcherCpnt>();
		kenv.addFactory<CKRocketRomanCpnt>();
		kenv.addFactory<CKJetPackRomanCpnt>();
		kenv.addFactory<CKMobileTowerCpnt>();
		kenv.addFactory<CKTriangularTurtleCpnt>();
		kenv.addFactory<CKSquareTurtleCpnt>();
		kenv.addFactory<CKDonutTurtleCpnt>();
		kenv.addFactory<CKPyramidalTurtleCpnt>();
		kenv.addFactory<CKAsterixGameManager>();
		kenv.loadGame(inputPath.u8string().c_str(), outputPath.u8string().c_str(), 1, KEnvironment::PLATFORM_PC);

		for (int lvlnum = 0; lvlnum <= 8; lvlnum++) {
			setStatusFmt("Patching Level %i", lvlnum);
			size_t decHeadStart;
			MemFile* headerFile = nullptr, * drmValues1 = nullptr;
			if (lvlnum != 0) {
				// Find unencrypted header in GameModule
				size_t findres = findmem(elbData.mem, elbFindStart, elbData.size, headerSignature, sizeof(headerSignature));
				if (findres == -1) { throw GamePatcherException("Unencrypted header not found in GameModule"); }
				decHeadStart = findres - 5;
				elbFindStart = findres + 1;
				headerFile = new MemFile((uint8_t*)elbData.mem + decHeadStart);
				drmValues1 = new MemFile((uint8_t*)elbData.mem + (drmValExeOffsets[lvlnum] - 0x2e933c + drmValStart));
				drmValues1->readUint32();
			}

			kenv.loadLevel(lvlnum, headerFile, drmValues1);
			if (headerFile) delete headerFile;
			if (drmValues1) delete drmValues1;

			char lvlPath[16];
			sprintf_s(lvlPath, "LVL%03u", lvlnum);
			fs::create_directory(outputPath / lvlPath);

			kenv.saveLevel(lvlnum);

			// Copy sector and lloc files
			std::error_code ec;
			for (unsigned int strnum = 0; strnum < 30; strnum++) {
				sprintf_s(ffn, "LVL%03u/STR%02u_%02u.KWN", lvlnum, lvlnum, strnum);
				fs::copy_file(inputPath / ffn, outputPath / ffn, fs::copy_options::overwrite_existing, ec);
			}
			for (unsigned int locnum = 0; locnum < 10; locnum++) {
				sprintf_s(ffn, "LVL%03u/%02uLLOC%02u.KWN", lvlnum, locnum, lvlnum);
				fs::copy_file(inputPath / ffn, outputPath / ffn, fs::copy_options::overwrite_existing, ec);
			}

			progress++;
		}

		// Copy game and gloc files
		setStatus("Copying remaining files");
		std::error_code ec;
		fs::copy_file(inputPath / "GAME.KWN", outputPath / "GAME.KWN", fs::copy_options::overwrite_existing, ec);
		for (unsigned int locnum = 0; locnum < 10; locnum++) {
			sprintf_s(ffn, "%02uGLOC.KWN", locnum);
			fs::copy_file(inputPath / ffn, outputPath / ffn, fs::copy_options::overwrite_existing, ec);
		}
		progress++;

		// Patch GameModule...
		setStatus("Patching GameModule");

		static const unsigned char code1[93] = {
			0x8B, 0x55, 0x00, 0x68, 0x47, 0x32, 0x51, 0xAD, 0x68, 0x47, 0x32, 0x51,
			0xAD, 0x6A, 0x08, 0x8D, 0x4C, 0x24, 0x04, 0x51, 0x89, 0xE9, 0xFF, 0x52,
			0x14, 0x58, 0x3D, 0x41, 0x73, 0x74, 0x65, 0x75, 0x08, 0x58, 0x3D, 0x72,
			0x69, 0x78, 0x20, 0x74, 0x1C, 0x6A, 0x10, 0x6A, 0x00, 0x68, 0x60, 0x9C,
			0x62, 0x00, 0x6A, 0x00, 0xFF, 0x15, 0x18, 0x22, 0x62, 0x00, 0x68, 0xF5,
			0x00, 0x00, 0x00, 0xFF, 0x15, 0xCC, 0x21, 0x62, 0x00, 0x8B, 0x55, 0x00,
			0x8D, 0x43, 0x0C, 0x6A, 0x04, 0x50, 0x89, 0xE9, 0xFF, 0x52, 0x14, 0xB8,
			0x7F, 0xCE, 0x40, 0x00, 0xFF, 0xE0, 0x90, 0x90, 0x90
		};

		static const unsigned char code2[33] = {
			0x55, 0x8B, 0xCB, 0x90, 0x90, 0xE8, 0xC7, 0x01, 0x00, 0x00, 0xA1, 0xF4,
			0x21, 0x66, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
			0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
		};

		memcpy((char*)elbData.mem + 0xC1EB, code1, sizeof(code1));
		memcpy((char*)elbData.mem + 0xCE7F, code2, sizeof(code2));
		memset((char*)elbData.mem + 0x1018C9, 0x90, 0x1F);
		static const char wrongLvlFormatMsg[] = "The LVL hasn't been patched for modding!\nUse the XXL Editor by AdrienTD to fix it.\0";
		memcpy((char*)elbData.mem + 0x229C60, wrongLvlFormatMsg, sizeof(wrongLvlFormatMsg));
		memset((char*)elbData.mem + 0x510d, 0x90, 5); // don't change the drm level number variable

		_wfopen_s(&elbFile, (outputPath / "GameModule_MP.exe").c_str(), L"wb");
		if (!elbFile) { throw GamePatcherException("Could not write patched GameModule"); }
		fwrite(elbData.mem, elbData.size, 1, elbFile);
		fclose(elbFile);

		memset((char*)elbData.mem + 0x7A5C3, 0x90, 5);
		_wfopen_s(&elbFile, (outputPath / "GameModule_MP_windowed.exe").c_str(), L"wb");
		if (!elbFile) { throw GamePatcherException("Could not write patched windowed GameModule"); }
		fwrite(elbData.mem, elbData.size, 1, elbFile);
		fclose(elbFile);

		progress++;
	}

	void GamePatcherThreadX2::start()
	{
		namespace fs = std::filesystem;
		this->progress = 0;

		// Read GameModule
		FILE* elbFile;
		_wfopen_s(&elbFile, elbPath.c_str(), L"rb");
		if (!elbFile) { throw GamePatcherException("Could not open the GameModule"); }
		MemBuffer elbData(elbFile);
		fclose(elbFile);

		static const unsigned char headerSignature[44] = {
			0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
			0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00
		};

		size_t elbFindStart = 0;
		char ffn[512];

		for (int lvlnum : {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11}) {
			setStatusFmt("Patching Level %i", lvlnum);
			size_t decHeadStart;
			MemFile* headerFile = nullptr, * drmValues1 = nullptr;
			if (lvlnum != 0) {
				// Find unencrypted header in GameModule
				size_t findres = findmem(elbData.mem, elbFindStart, elbData.size, headerSignature, sizeof(headerSignature));
				if (findres == -1) { throw GamePatcherException("Unencrypted header not found in GameModule"); }
				decHeadStart = findres - 0x15;
				elbFindStart = findres + 1;
			}

			char lvlPath[16];
			sprintf_s(lvlPath, "LVL%03u", lvlnum);
			fs::create_directory(outputPath / lvlPath);

			char kwnFilePath[32];
			sprintf_s(kwnFilePath, "LVL%03u/LVL%02u.KWN", lvlnum, lvlnum);

			// load lvl
			FILE* lvlFile;
			_wfopen_s(&lvlFile, (inputPath / kwnFilePath).c_str(), L"rb");
			MemBuffer lvlmb(lvlFile);
			fclose(lvlFile);

			// modify header
			if (lvlnum == 0) {
				uint32_t headsize = *(uint32_t*)lvlmb.mem;
				memmove((char*)lvlmb.mem + 12, (char*)lvlmb.mem + 4, headsize - 8);
			}
			else {
				uint32_t headsize = *(uint32_t*)((uint8_t*)elbData.mem + decHeadStart - 4);
				memcpy((char*)lvlmb.mem + 12, (uint8_t*)elbData.mem + decHeadStart, headsize - 8);
			}
			memcpy(lvlmb.mem, "Asterix-XXL2", 12);

			// save lvl
			_wfopen_s(&lvlFile, (outputPath / kwnFilePath).c_str(), L"wb");
			fwrite(lvlmb.mem, lvlmb.size, 1, lvlFile);
			fclose(lvlFile);

			// Copy sector and lloc files
			std::error_code ec;
			for (unsigned int strnum = 0; strnum < 30; strnum++) {
				sprintf_s(ffn, "LVL%03u/STR%02u_%02u.KWN", lvlnum, lvlnum, strnum);
				fs::copy_file(inputPath / ffn, outputPath / ffn, fs::copy_options::overwrite_existing, ec);
			}
			for (unsigned int locnum = 0; locnum < 10; locnum++) {
				sprintf_s(ffn, "LVL%03u/%02uLLOC%02u.KWN", lvlnum, locnum, lvlnum);
				fs::copy_file(inputPath / ffn, outputPath / ffn, fs::copy_options::overwrite_existing, ec);
			}

			progress++;
		}

		// Copy game and gloc files
		setStatus("Copying remaining files");
		std::error_code ec;
		fs::copy_file(inputPath / "GAME.KWN", outputPath / "GAME.KWN", fs::copy_options::overwrite_existing, ec);
		for (unsigned int locnum = 0; locnum < 10; locnum++) {
			sprintf_s(ffn, "%02uGLOC.KWN", locnum);
			fs::copy_file(inputPath / ffn, outputPath / ffn, fs::copy_options::overwrite_existing, ec);
		}
		fs::copy_file(inputPath / "binkw32.dll", outputPath / "binkw32.dll", fs::copy_options::overwrite_existing, ec);
		progress++;

		// Patch GameModule
		setStatus("Patching GameModule");

		static const unsigned char code1[95] = {
			0x83, 0xEC, 0x0C, 0x89, 0xE0, 0x6A, 0x0C, 0x50, 0x89, 0xE9, 0x8B, 0x45,
			0x00, 0xFF, 0x50, 0x10, 0x83, 0xC4, 0x0C, 0x55, 0x89, 0xD9, 0xB8, 0xC0,
			0xE9, 0x40, 0x00, 0xFF, 0xD0, 0x83, 0xEC, 0x08, 0x8B, 0x55, 0x00, 0x8D,
			0x04, 0x24, 0x6A, 0x04, 0x50, 0x89, 0xE9, 0xFF, 0x52, 0x10, 0x8B, 0x55,
			0x00, 0x8D, 0x44, 0x24, 0x04, 0x6A, 0x04, 0x50, 0x89, 0xE9, 0xFF, 0x52,
			0x10, 0x8B, 0x0C, 0x24, 0x8B, 0x54, 0x24, 0x04, 0x51, 0xB9, 0x94, 0x29,
			0x66, 0x00, 0x52, 0xB8, 0x50, 0x15, 0x40, 0x00, 0xFF, 0xD0, 0x83, 0xC4,
			0x08, 0xB8, 0x47, 0xE8, 0x40, 0x00, 0xFF, 0xE0, 0x90, 0x90, 0x90
		};
		memset((char*)elbData.mem + 0x1591, 0x90, 5); // NOP DRM dealloc code
		memcpy((char*)elbData.mem + 0xDB3D, code1, sizeof(code1));

		*((char*)elbData.mem + 0x9E8DA) = 1;
		_wfopen_s(&elbFile, (outputPath / "GameModule_MP.exe").c_str(), L"wb");
		if (!elbFile) { throw GamePatcherException("Could not write patched GameModule"); }
		fwrite(elbData.mem, elbData.size, 1, elbFile);
		fclose(elbFile);

		*((char*)elbData.mem + 0x9E8DA) = 0;
		_wfopen_s(&elbFile, (outputPath / "GameModule_MP_windowed.exe").c_str(), L"wb");
		if (!elbFile) { throw GamePatcherException("Could not write patched windowed GameModule"); }
		fwrite(elbData.mem, elbData.size, 1, elbFile);
		fclose(elbFile);

		progress++;
	}

	void GamePatcherThreadCopy::start()
	{
		namespace fs = std::filesystem;
		const char* ext = KEnvironment::platformExt[platform];
		auto copy = [this](std::string_view sv) {
			std::error_code ec;
			fs::copy_file(inputPath / sv, outputPath / sv, fs::copy_options::overwrite_existing, ec);
		};
		char fname[32];

		for (int i = 0; i < 20; i++) {
			setStatusFmt("Copying Level %i", i);
			char f_lvl[8];
			sprintf_s(f_lvl, "LVL%03u", i);
			if (!fs::is_directory(inputPath / f_lvl))
				continue; // progress++ skipped!

			fs::create_directory(outputPath / f_lvl);
			sprintf_s(fname, "LVL%03u/LVL%02u.%s", i, i, ext);
			copy(fname);
			for (int j = 0; j < 20; j++) {
				sprintf_s(fname, "LVL%03u/%02uLLOC%02u.%s", i, j, i, ext);
				copy(fname);
			}
			for (int j = 0; j < 20; j++) {
				sprintf_s(fname, "LVL%03u/STR%02u_%02u.%s", i, i, j, ext);
				copy(fname);
			}
			progress++;
		}

		setStatus("Copying remaining files");
		sprintf_s(fname, "GAME.%s", ext);
		copy(fname);
		for (int j = 0; j < 20; j++) {
			sprintf_s(fname, "%02uGLOC.%s", j, ext);
			copy(fname);
		}
		progress++;
	}

	std::string GamePatcherThread::getStatus()
	{
		std::lock_guard lock(statusMutex);
		return status;
	}

	void GamePatcherThread::setStatus(const std::string& text) {
		statusMutex.lock();
		status = text;
		statusMutex.unlock();
	}
	void GamePatcherThread::setStatusFmt(const char* format, ...) {
		char buf[256];
		va_list args;
		va_start(args, format);
		vsprintf_s(buf, format, args);
		va_end(args);
		setStatus(buf);
	}
}
