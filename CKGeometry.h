#pragma once

#include "KObject.h"

struct CKGeometry : CKCategory<10> {
	objref<CKGeometry> nextGeo;
	uint32_t flags;
};