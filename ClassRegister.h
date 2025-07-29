#pragma once

struct KEnvironment;

namespace ClassRegister {
	void registerClassesForXXL1PC(KEnvironment& kenv);
	void registerClassesForXXL1PS2(KEnvironment& kenv);
	void registerClassesForXXL2PlusPC(KEnvironment& kenv);
	void registerClassesForXXL2(KEnvironment& kenv);
	void registerClassesForOlympic(KEnvironment& kenv);
	void registerClassesForSpyro(KEnvironment& kenv);
	void registerClasses(KEnvironment& kenv, int gameVersion, int gamePlatform, bool isRemaster);
};