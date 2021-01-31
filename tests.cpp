#ifndef XEC_RELEASE

#include "tests.h"
#include <string>
#include <INIReader.h>
#include "KEnvironment.h"
#include "DynArray.h"
#include "Events.h"
#include "ClassRegister.h"
#include "CKComponent.h"
#include "CKLogic.h"

void Test_Cpnt2CSV() {
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	kenv.addFactory<CKBasicEnemyCpnt>();
	kenv.loadGame(gamePath.c_str(), KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC);

	struct NameListener : MemberListener {
		FILE* csv;
		NameListener(FILE* csv) : csv(csv) {}
		void write(const char* name) { fprintf(csv, "%s\t", name); }
		void reflect(uint8_t& ref, const char* name) override { write(name); }
		void reflect(uint16_t& ref, const char* name) override { write(name); }
		void reflect(uint32_t& ref, const char* name) override { write(name); }
		void reflect(float& ref, const char* name) override { write(name); }
		void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override { write(name); }
		void reflect(Vector3& ref, const char* name) override { fprintf(csv, "%s X\t%s Y\t%s Z\t", name, name, name); }
		void reflect(EventNode& ref, const char* name, CKObject* user) override { write(name); };
		void reflect(std::string& ref, const char* name) override { abort(); } // TODO
	};
	struct ValueListener : MemberListener {
		FILE* csv;
		ValueListener(FILE* csv) : csv(csv) {}
		void reflect(uint8_t& ref, const char* name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint16_t& ref, const char* name) override { fprintf(csv, "%u\t", ref); }
		void reflect(uint32_t& ref, const char* name) override { fprintf(csv, "%u\t", ref); }
		void reflect(float& ref, const char* name) override { fprintf(csv, "%f\t", ref); }
		void reflectAnyRef(kanyobjref& ref, int clfid, const char* name) override { fprintf(csv, "%s\t", ref._pointer->getClassName()); }
		void reflect(Vector3& ref, const char* name) override { fprintf(csv, "%f\t%f\t%f\t", ref.x, ref.y, ref.z); }
		void reflect(EventNode& ref, const char* name, CKObject* user) override { fprintf(csv, "(%i,%i)\t", ref.seqIndex, ref.bit); };
		void reflect(std::string& ref, const char* name) override { abort(); } // TODO
	};

	FILE* csv;
	fopen_s(&csv, "EnemyCpnts.txt", "w");
	NameListener nl(csv);
	ValueListener vl(csv);

	for (int lvl = 1; lvl <= 6; lvl++) {
		kenv.loadLevel(lvl);
		if (lvl == 1) {
			CKBasicEnemyCpnt* firstcpnt = kenv.levelObjects.getFirst<CKBasicEnemyCpnt>();
			fprintf(csv, "Level\tIndex\t");
			firstcpnt->reflectMembers2(nl, &kenv);
		}
		int index = 0;
		for (CKObject* obj : kenv.levelObjects.getClassType<CKBasicEnemyCpnt>().objects) {
			fprintf(csv, "\n%i\t%i\t", lvl, index);
			obj->cast<CKBasicEnemyCpnt>()->reflectMembers2(vl, &kenv);
			index++;
		}
	}

	fclose(csv);
}

void Test_LevelSizeCheck()
{
	static constexpr std::array<std::tuple<const char*, int, int>, 4> games = { {
		{"C:\\Users\\Adrien\\Downloads\\virtualboxshare\\aoxxl2demo\\Astérix & Obélix XXL2 DEMO", KEnvironment::KVERSION_XXL2, KEnvironment::PLATFORM_PC},
		{"C:\\Apps\\Asterix at the Olympic Games", KEnvironment::KVERSION_OLYMPIC, KEnvironment::PLATFORM_PC},
		{"D:\\PSP_GAME\\USRDIR", KEnvironment::KVERSION_ARTHUR, KEnvironment::PLATFORM_PSP},
		{"C:\\Users\\Adrien\\Desktop\\kthings\\xxl1_mp_orig", KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC},
	} };
	static const std::string outputPath = "C:\\Users\\Adrien\\Desktop\\kthings\\x2savetest";
	for (const auto& game : games) {
		printf("****** %s ******\n", std::get<0>(game));

		KEnvironment kenv;
		kenv.loadGame(std::get<0>(game), std::get<1>(game), std::get<2>(game));
		kenv.outGamePath = outputPath;
		kenv.loadLevel(1);
		kenv.saveLevel(1);

		std::string lvlpath = std::string("\\LVL001\\LVL01.") + KEnvironment::platformExt[kenv.platform];
		std::string inlvlpath = std::get<0>(game) + lvlpath;
		std::string outlvlpath = outputPath + lvlpath;
		DynArray<uint8_t> inCnt, outCnt;

		for (std::pair<const char*, DynArray<uint8_t>*> p : { std::make_pair(inlvlpath.c_str(), &inCnt), std::make_pair(outlvlpath.c_str(), &outCnt) }) {
			FILE* file;
			fopen_s(&file, p.first, "rb");
			fseek(file, 0, SEEK_END);
			size_t len = ftell(file);
			fseek(file, 0, SEEK_SET);
			p.second->resize(len);
			fread(p.second->data(), len, 1, file);
			fclose(file);
		}

		int numDiffBytes = 0;
		size_t minSize = (inCnt.size() < outCnt.size()) ? inCnt.size() : outCnt.size();

		for (size_t i = 0; i < minSize; i++)
			if (inCnt[i] != outCnt[i])
				numDiffBytes++;

		printf("************\n");
		printf(" Input file size: %10u bytes\n", inCnt.size());
		printf("Output file size: %10u bytes\n", outCnt.size());
		printf(" Different bytes: %10u bytes\n", numDiffBytes);
		printf("************\n");
	}
	getchar();
}

void Test_ChoreoFreqValues() {
	INIReader config("xec-settings.ini");
	std::string gamePath = config.Get("XXL-Editor", "gamepath", ".");

	KEnvironment kenv;
	//ClassRegister::registerClassesForXXL1PC(kenv);
	kenv.addFactory<CKChoreoKey>();
	kenv.loadGame(gamePath.c_str(), KEnvironment::KVERSION_XXL1, KEnvironment::PLATFORM_PC, true);

	std::map<std::array<float, 3>, int> triplets;
	for (int i = 1; i <= 6; i++) {
		kenv.loadLevel(i);

		for (CKObject* okey : kenv.levelObjects.getClassType<CKChoreoKey>().objects) {
			CKChoreoKey* key = okey->cast<CKChoreoKey>();
			//printf("key %f %f %f\n", key->unk1, key->unk2, key->unk3);
			triplets[{ key->unk1, key->unk2, key->unk3 }]++;
		}
	}
	for (auto& a : triplets) {
		printf("%f %f %f %i\n", a.first[0], a.first[1], a.first[2], a.second);
	}
	getchar();
}

void Tests::TestPrompt()
{
	static const std::pair<void(*)(), const char*> tests[] = {
		{Test_Cpnt2CSV, "Component values to CSV"},
		{Test_LevelSizeCheck, "Level size check"},
		{Test_ChoreoFreqValues, "Chereokey value frequency"},
	};
	const size_t numTests = std::size(tests);
	printf("Available tests:\n");
	for (size_t t = 0; t < numTests; t++) {
		printf("%3i. %s\n", t, tests[t].second);
	}
	printf("Run which test number? ");
	char input[32];
	gets_s(input);
	size_t t = atoi(input);
	if (t < numTests)
		tests[t].first();
}

#endif