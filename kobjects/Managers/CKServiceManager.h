#include "../../KObject.h"

struct CKServiceManager : CKSubclass<CKManager, 1> {
	std::vector<objref<CKService*>> services;
};