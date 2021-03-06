#pragma once

//#include "rw.h"
//#include "File.h"
#include <vector>
#include "vecmat.h"

struct File;
struct RwsHeader;

struct RwExtension {
	virtual ~RwExtension() {}
	virtual uint32_t getType() = 0;
	virtual void deserialize(File *file, const RwsHeader &header, void *parent) = 0;
	virtual void serialize(File *file) = 0;
	virtual RwExtension* clone() = 0;
};

struct RwExtUnknown : RwExtension {
	void *_ptr = nullptr;
	uint32_t _length = 0;
	uint32_t _type;

	~RwExtUnknown();
	uint32_t getType() override;
	void deserialize(File *file, const RwsHeader &header, void *parent) override;
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

	uint32_t getType() override;
	void deserialize(File *file, const RwsHeader &header, void *parent) override;
	void serialize(File *file) override;
	RwExtension* clone() override;
};

struct RwExtSkin : RwExtension {
	uint8_t numBones, numUsedBones, maxWeightPerVertex;
	std::vector<uint8_t> usedBones;
	std::vector<std::array<uint8_t, 4>> vertexIndices;
	std::vector<std::array<float, 4>> vertexWeights;
	std::vector<Matrix> matrices;
	bool isSplit;
	uint32_t boneLimit, numMeshes, numRLE;

	uint32_t getType() override;
	void deserialize(File *file, const RwsHeader &header, void *parent) override;
	void serialize(File *file) override;
	RwExtension* clone() override;

	void merge(const RwExtSkin &other);
};

RwExtension *RwExtCreate(uint32_t type);