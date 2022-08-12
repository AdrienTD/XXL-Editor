#pragma once

struct KEnvironment;

namespace ClassRegister {
	void registerClassesForXXL1PC(KEnvironment& kenv);
	void registerClassesForXXL1Console(KEnvironment& kenv);
	void registerClassesForXXL2PlusPC(KEnvironment& kenv);
	void registerClassesForXXL2PlusConsole(KEnvironment& kenv);
	void registerClasses(KEnvironment& kenv, int gameVersion, int gamePlatform, bool isRemaster);
};