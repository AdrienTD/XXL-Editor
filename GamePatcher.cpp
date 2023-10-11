#include "GamePatcher.h"
#include "KEnvironment.h"
#include "File.h"
#include "GamePatcherKClasses.h"
#include <stack>
#include <cstdarg>
#include "CKUtils.h"
#include "GameClasses/CKGameX2.h"

namespace GamePatcher {

	struct GPX2ReadingMemberListener : ReadingMemberListener {
		File* drmFile;
		bool drmMode = false;
		GPX2ReadingMemberListener(File* file, KEnvironment* kenv, File* drmFile) : ReadingMemberListener(file, kenv), drmFile(drmFile) {}
		void reflect(uint8_t& ref, const char* name) override {
			if (drmMode) {
				drmMode = false;
				ref = drmFile->readUint8();
			}
			else {
				ReadingMemberListener::reflect(ref, name);
			}
		}
		void reflect(uint32_t& ref, const char* name) override {
			if (drmMode) {
				drmMode = false;
				ref = drmFile->readUint32();
			}
			else {
				ReadingMemberListener::reflect(ref, name);
			}
		}
		void reflect(int8_t& ref, const char* name) override {
			reflect((uint8_t&)ref, name);
		}
		void reflect(int32_t& ref, const char* name) override {
			reflect((uint32_t&)ref, name);
		}
		void reflect(float& ref, const char* name) override {
			reflect((uint32_t&)ref, name);
		}
		void reflect(EventNode& ref, const char* name, CKObject* self) override {
			ref.enx2.datas.resize(file->readUint32());
			for (auto& ref : ref.enx2.datas)
				ref = KWeakRef<CKComparedData>((CKComparedData*)kenv->readObjPnt(file));
		}
		void message(const char* msg) override {
			static const std::string pass = "DRM";
			if (pass == msg) {
				drmMode = true;
			}
		}
	};

	template <class D, class KENV> struct X2PatchedKClass : D {
		void deserialize(KEnvironment* kenv, File* file, size_t length) override {
			if constexpr (std::is_base_of_v<::CKHook, D>)
				::CKHook::deserialize(kenv, file, length);
			File* drmFile = std::is_same_v<GameX2::CKHkCatapult, D> ? KENV::x2DrmFile2 : KENV::x2DrmFile1;
			GPX2ReadingMemberListener r(file, kenv, drmFile);
			((D*)this)->reflectMembers2(r, kenv);
		}
	};

	struct GamePatcherKEnvironment : KEnvironment {
		inline static File* x2DrmFile1 = nullptr;
		inline static File* x2DrmFile2 = nullptr;
		DynArray<uint8_t> xxl2Rest;

		static auto ConcatGamePath(const std::string& gameDir, const std::string_view& gameFile) {
			return std::filesystem::u8path(gameDir).append(gameFile);
		}

		void loadGame(const char* path, const char* outPath, int version, int platform) {
			this->gamePath = path;
			this->version = version;
			this->platform = platform;
			this->outGamePath = outPath;

			if (version < KVERSION_XXL2)
				clcatReorder = { 0,9,1,2,3,4,5,6,7,8,10,11,12,13,14 };
			else
				clcatReorder = { 9,0,1,2,3,4,5,6,7,8,10,11,12,13,14 };

			auto gamefn = ConcatGamePath(gamePath, std::string("GAME.") + platformExt[platform]);

			IOFile gameFile(gamefn.c_str(), "rb");
			if (version == KVERSION_XXL2) {
				uint32_t numGameObjects = gameFile.readUint32();
				gameFile.read(this->gameManagerUuid.data(), 16);
				uint32_t gameManagerId = gameFile.readUint32();
				this->globalObjects.reserve(numGameObjects);
				uint32_t i = 0;
				while (i < numGameObjects) {
					uint32_t clfid = gameFile.readUint32();
					uint32_t count = gameFile.readUint32();
					uint8_t hasUuid = gameFile.readUint8();
					for (uint32_t j = 0; j < count; j++) {
						CKObject* obj = constructObject(clfid);
						this->globalObjects.push_back(obj);
						if (hasUuid) {
							kuuid uid;
							gameFile.read(uid.data(), 16);
							this->globalUuidMap[uid] = obj;
						}
					}
					i += count;
				}
				for (CKObject* obj : this->globalObjects) {
					uint32_t nextoff = gameFile.readUint32();
					obj->deserializeGlobal(this, &gameFile, nextoff - gameFile.tell());
					assert(nextoff == gameFile.tell());
				}
			}
		}
		void loadLevelHeader(File& lvlFile)
		{
			this->numSectors = lvlFile.readUint8();
			if (version >= KVERSION_XXL2)
				lvlFile.read(this->gameManagerUuid.data(), 16);
			this->lvlUnk2 = lvlFile.readUint32();

			this->sectorObjects.resize(this->numSectors);

			for (int clcat = 0; clcat < 15; clcat++) {
				uint16_t numClasses = lvlFile.readUint16();
				this->levelObjects.categories[clcat].type.resize(numClasses);
				for (uint16_t clid = 0; clid < numClasses; clid++) {
					auto& lvltype = this->levelObjects.categories[clcat].type[clid];
					
					uint32_t fid = clcat | (clid << 6);
					uint16_t numTotalObjects = lvlFile.readUint16();
					uint16_t numLevelObjects = lvlFile.readUint16();
					uint16_t numGlobs;
					if (version >= KVERSION_XXL2) {
						numGlobs = lvlFile.readUint16();
						lvltype.globByte = lvlFile.readUint8();
					}
					uint8_t info = lvlFile.readUint8();
					if (version >= KVERSION_XXL2) {
						lvltype.globUuids.resize(numGlobs);
						for (kuuid& id : lvltype.globUuids) {
							lvlFile.read(id.data(), 16);
						}
					}

					lvltype.info = info;
					lvltype.totalCount = numTotalObjects;
					lvltype.startId = 0;
					lvltype.objects.reserve(numLevelObjects);
					for (uint16_t i = 0; i < numLevelObjects; i++) {
						CKObject* obj = constructObject(fid);
						lvltype.objects.push_back(obj);
						if (lvltype.globByte) {
							kuuid id;
							lvlFile.read(id.data(), 16);
							levelUuidMap[id] = obj;
						}
						//printf("Constructed %s\n", lvltype.objects.back()->getClassName());
					}
				}
			}
		}

		void loadLevel(int lvlNumber, File* headerFile, File* drmValues1, File* drmValues2 = nullptr)
		{
			if (levelLoaded)
				unloadLevel();

			this->loadingSector = -1;
			char lvlfn[500];
			sprintf_s(lvlfn, "%s/LVL%03u/LVL%02u.%s", gamePath.c_str(), lvlNumber, lvlNumber, platformExt[platform]);

			IOFile lvlFile(std::filesystem::u8path(lvlfn).c_str(), "rb");
			std::string asthead = lvlFile.readString(7);
			if (asthead == "Asterix")
				throw GamePatcherException("The game is already patched for modding!");
			lvlFile.seek(0, SEEK_SET);
			if (version == KVERSION_XXL1) {
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
			}
			else if (version == KVERSION_XXL2) {
				// header size not present in PC demo
				uint32_t drmSize = lvlFile.readUint32();
				if (headerFile) {
					loadLevelHeader(*headerFile);
					lvlFile.seek(drmSize, SEEK_CUR);
				}
				else {
					loadLevelHeader(lvlFile);
					lvlFile.seek(8, SEEK_CUR);
				}
			}

			lvlFile.seek(8 /*+ 4 (drm)*/, SEEK_CUR);

			g_drmValues = drmValues1;
			x2DrmFile1 = drmValues1;
			x2DrmFile2 = drmValues2;

			for (int clcat : clcatReorder) {
				uint16_t numClasses = lvlFile.readUint16();
				uint32_t nextCat = lvlFile.readUint32();
				printf("Cat %i at %08X, next at %08X, numClasses = %i\n", clcat, lvlFile.tell(), nextCat, numClasses);
				for (size_t clid = 0; clid < this->levelObjects.categories[clcat].type.size(); clid++) {
					auto& cltype = this->levelObjects.categories[clcat].type[clid];
					if (cltype.objects.empty() && cltype.globUuids.empty())
						continue;
					uint32_t nextClass = lvlFile.readUint32();
					//printf("Class %i %i at %08X, next at %08X\n", clcat, clid, lvlFile.tell(), nextClass);
					if (cltype.info) {
						if (version >= KVERSION_XXL2) {
							uint16_t numGlobals = lvlFile.readUint16();
							assert(numGlobals == cltype.globUuids.size());
							for (int gb = 0; gb < numGlobals; gb++) {
								uint32_t nextGlob = lvlFile.readUint32();
								globalUuidMap.at(cltype.globUuids[gb])->deserializeLvlSpecific(this, &lvlFile, nextGlob - lvlFile.tell());
								assert(lvlFile.tell() == nextGlob);
							}
						}
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

			//this->loadingSector = -1;
			//for (auto& cat : levelObjects.categories)
			//	for (auto& cl : cat.type)
			//		for (CKObject* obj : cl.objects)
			//			obj->onLevelLoaded(this);
			//for (auto& str : sectorObjects)
			//	for (auto& cat : str.categories)
			//		for (auto& cl : cat.type)
			//			for (CKObject* obj : cl.objects)
			//				obj->onLevelLoaded(this);

			if (version >= KVERSION_XXL2) {
				uint32_t weirdOffset = lvlFile.readUint32();
				uint32_t currentOffset = (uint32_t)lvlFile.tell();
				assert(currentOffset == weirdOffset);
				lvlFile.seek(0, SEEK_END);
				uint32_t endOffset = (uint32_t)lvlFile.tell();
				lvlFile.seek(currentOffset, SEEK_SET);
				xxl2Rest.resize(endOffset - currentOffset);
				lvlFile.read(xxl2Rest.data(), xxl2Rest.size());
				assert(lvlFile.tell() == endOffset);
			}

			levelLoaded = true;
		}

		void prepareSavingMapGP()
		{
			saveMap.clear();
			for (int clcat = 0; clcat < 15; clcat++) {
				auto& lvlcat = levelObjects.categories[clcat];
				for (int clid = 0; clid < (int)lvlcat.type.size(); clid++) {
					auto& lvltype = lvlcat.type[clid];
					for (int i = 0; i < (int)lvltype.objects.size(); i++)
						saveMap[lvltype.objects[i]] = clcat | (clid << 6) | (i << 17);
				}
			}

			saveUuidMap.clear();
			for (auto& elem : globalUuidMap)
				saveUuidMap[elem.second] = elem.first;
			for (auto& elem : levelUuidMap)
				saveUuidMap[elem.second] = elem.first;

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
			if (version == KVERSION_XXL1) {
				lvlFile.write("Asterix ", 8);
				lvlFile.writeUint32(this->lvlUnk1);
				lvlFile.writeUint8(this->numSectors);
				lvlFile.writeUint32(this->lvlUnk2);
			}
			else {
				lvlFile.write("Asterix-XXL2", 12);
				lvlFile.writeUint8(this->numSectors);
				lvlFile.write(this->gameManagerUuid.data(), 16);
				lvlFile.writeUint32(this->lvlUnk2);
			}

			for (auto& cat : this->levelObjects.categories) {
				lvlFile.writeUint16((uint16_t)cat.type.size());
				for (auto& kcl : cat.type) {
					lvlFile.writeUint16(kcl.totalCount);
					lvlFile.writeUint16((uint16_t)kcl.objects.size());
					if (version >= KVERSION_XXL2) {
						lvlFile.writeUint16(kcl.globUuids.size());
						lvlFile.writeUint8(kcl.globByte);
					}
					lvlFile.writeUint8(kcl.info);
					if (version >= KVERSION_XXL2) {
						for (const kuuid& id : kcl.globUuids)
							lvlFile.write(id.data(), 16);
						if (kcl.globByte) {
							for (CKObject* obj : kcl.objects) {
								lvlFile.write(saveUuidMap.at(obj).data(), 16);
							}
						}
					}
				}
			}

			lvlFile.writeUint32(0);
			lvlFile.writeUint32(0);

			for (int nclcat : clcatReorder) {
				auto& cat = this->levelObjects.categories[nclcat];

				uint32_t clcnt = 0;
				for (auto& kcl : cat.type)
					if (!kcl.objects.empty() || !kcl.globUuids.empty())
						clcnt++;

				lvlFile.writeUint16(clcnt);
				offsetStack.push();

				for (auto& kcl : cat.type) {
					if (!kcl.objects.empty() || !kcl.globUuids.empty()) {
						offsetStack.push();
						if (kcl.info) {
							if (version >= KVERSION_XXL2) {
								lvlFile.writeUint16(kcl.globUuids.size());
								for (kuuid& id : kcl.globUuids) {
									offsetStack.push();
									globalUuidMap.at(id)->serializeLvlSpecific(this, &lvlFile);
									offsetStack.pop();
								}
							}
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

			if (version >= KVERSION_XXL2) {
				lvlFile.writeUint32(lvlFile.tell() + 4);
				lvlFile.write(xxl2Rest.data(), xxl2Rest.size());
			}
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

		uint32_t drmValExeOffsets[9] = { 0, 0x2e934c, 0x2e937c, 0x2e9460, 0x2e9538, 0x2e9660, 0x2e975c,
			0x2e9344, 0x2e933c }; // two last ones might have been swapped, but they point to identical data anyway (of size 0!)

		size_t drmValStart = findmem(elbData.mem, 0, elbData.size, drmValSignature, sizeof(drmValSignature));
		if (drmValStart != -1) {
			// Ipatix patch
			for (auto& off : drmValExeOffsets)
				off = off - 0x2e933c + drmValStart + 4;
		}
		else {
			// Polish patch
			uint32_t* filesptr = (uint32_t*)((char*)elbData.mem + 0x285000);
			if (filesptr[1] != 0x2B || filesptr[3] != 0xE0)
				throw GamePatcherException("Could not find DRM-protected values in GameModule");
			for (int i = 1; i <= 8; i++) {
				drmValExeOffsets[i] = *filesptr - 0x410000;
				filesptr += 2;
			}
		}

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
				drmValues1 = new MemFile((uint8_t*)elbData.mem + drmValExeOffsets[lvlnum]);
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

		//static const uint32_t headerOffsets[12] = { 0, 0x6E198C, 0x6E4E4C, 0x6E828C, 0x6EB6DC, 0, 0x6EEB3C, 0x6F1F3C, 0x6F537C, 0x6F874C, 0x6FBB1C, 0x6FEEEC };
		std::array<uint32_t, 12> drmValOffsets = { 0 };
		std::array<uint32_t, 12> catapuOffsets = { 0 };
		if (elbData.size >= 0x2A0010) {
			struct DRMFile { uint32_t dvOffset, dvSize, cataOffset, cataSize; };
			DRMFile* f = (DRMFile*)((char*)elbData.mem + 0x2A0000);
			if (f->dvSize == 0xDE && f->cataSize == 8) {
				// CD Projekt Patch
				for (int i = 1; i <= 11; ++i) {
					drmValOffsets[i] = f->dvOffset - 0x71F000 + 0x2A0000;
					catapuOffsets[i] = f->cataOffset - 0x71F000 + 0x2A0000;
					++f;
				}
			}
			else {
				// Ipatix patch
				drmValOffsets = { 0, 0x6E1390, 0x6E1474, 0x6E1538, 0x6E15FC, 0, 0x6E1748, 0x6E1800, 0x6E18E8, 0x6E190C, 0x6E1930, 0x6E196C };
				catapuOffsets = { 0, 0x7022AC, 0x7022B8, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				for (uint32_t& p : drmValOffsets)
					if (p)
						p = p - 0x400000 + 4;
				for (uint32_t& p : catapuOffsets)
					if (p)
						p = p - 0x400000 + 4;
			}
		}

		GamePatcherKEnvironment kenv;
		kenv.addFactory<X2PatchedKClass<GameX2::CKHkDoor, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKHkA2Boss, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKHkA2Hero, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKHkSlotMachine, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKHkCatapult, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKA2EnemyCpnt, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKA2JetPackEnemyCpnt, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKA2InvincibleEnemyCpnt, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKA2ArcherEnemyCpnt, GamePatcherKEnvironment>>();
		kenv.addFactory<X2PatchedKClass<GameX2::CKA2MarioEnemyCpnt, GamePatcherKEnvironment>>();
		kenv.addFactory<CKA2GameState>();
		kenv.loadGame(inputPath.u8string().c_str(), outputPath.u8string().c_str(), 2, KEnvironment::PLATFORM_PC);

		for (int lvlnum : {0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11}) {
			setStatusFmt("Patching Level %i", lvlnum);
			size_t decHeadStart;
			std::unique_ptr<MemFile> headerFile, drmValues1, drmCatapultValues;
			if (lvlnum != 0) {
				// Find unencrypted header in GameModule
				size_t findres = findmem(elbData.mem, elbFindStart, elbData.size, headerSignature, sizeof(headerSignature));
				if (findres == -1) { throw GamePatcherException("Unencrypted header not found in GameModule"); }
				decHeadStart = findres - 0x15;
				elbFindStart = findres + 1;
				headerFile = std::make_unique<MemFile>((uint8_t*)elbData.mem + decHeadStart);
				if (drmValOffsets[lvlnum])
					drmValues1 = std::make_unique<MemFile>((uint8_t*)elbData.mem + drmValOffsets[lvlnum]);
				if (catapuOffsets[lvlnum])
					drmCatapultValues = std::make_unique<MemFile>((uint8_t*)elbData.mem + catapuOffsets[lvlnum]);
			}

			kenv.loadLevel(lvlnum, headerFile.get(), drmValues1.get(), drmCatapultValues.get());

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
		memset((char*)elbData.mem + 0x5F38, 0x90, 5); // don't change the drm level number variable
		memset((char*)elbData.mem + 0xDC53, 0x90, 6); // don't change the drm level number variable


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
