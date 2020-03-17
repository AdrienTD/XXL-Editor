#pragma once

//#include "rw.h"
//#include "File.h"
#include <vector>

struct File;
struct RwsHeader;

struct RwExtension {
	virtual void deserialize(File *file, const RwsHeader &header) = 0;
	virtual void serialize(File *file) = 0;
	virtual RwExtension* clone() = 0;
};

struct RwExtUnknown : RwExtension {
	void *_ptr = nullptr;
	uint32_t _length = 0;
	uint32_t _type;
	void deserialize(File *file, const RwsHeader &header) override;
	void serialize(File *file) override;
	RwExtension* clone() override;
};

struct RwExtHAnim : RwExtension {
	struct Bone {
		uint32_t nodeId, nodeIndex, flags;
	};
	uint32_t version, nodeId; // , numNodes;
	uint32_t flags, keyFrameSize;
	std::vector<Bone> bones;
	void deserialize(File *file, const RwsHeader &header) override;
	void serialize(File *file) override;
	RwExtension* clone() override;
};

RwExtension *RwExtCreate(uint32_t type);