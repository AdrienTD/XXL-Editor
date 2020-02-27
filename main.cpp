#include "KEnvironment.h"
#include "CKManager.h"

void main()
{
	KEnvironment kenv;
	kenv.factories[CKServiceManager::FULL_ID] = KFactory::of<CKServiceManager>();
	kenv.loadGame("C:\\Program Files (x86)\\Atari\\Asterix & Obelix XXL", 1);
	kenv.loadLevel(0);

	printf("lol\n");
	getchar();
}