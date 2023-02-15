#pragma once

#include "rw.h"
//#include "File.h"
#include <variant>
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
	uint32_t boneLimit; // , numMeshes, numRLE;
	std::vector<uint8_t> boneIndexRemap;
	std::vector<std::pair<uint8_t, uint8_t>> boneGroups;
	std::vector<std::pair<uint8_t, uint8_t>> boneGroupIndices;

	std::vector<uint8_t> nativeData; // for consoles

	uint32_t getType() override;
	void deserialize(File *file, const RwsHeader &header, void *parent) override;
	void serialize(File *file) override;
	RwExtension* clone() override;

	void merge(const RwExtSkin &other);
};

struct RwExtBinMesh : RwExtension {
	uint32_t flags = 0, /*numMeshes = 0,*/ totalIndices = 0;
	struct Mesh {
		uint32_t material;
		std::vector<uint32_t> indices;
	};
	std::vector<Mesh> meshes;
	bool isNative = false;

	uint32_t getType() override;
	void deserialize(File* file, const RwsHeader& header, void* parent) override;
	void serialize(File* file) override;
	RwExtension* clone() override;
};

struct RwExtNativeData : RwExtension {
	std::vector<uint8_t> data;
	uint32_t getType() override;
	void deserialize(File* file, const RwsHeader& header, void* parent) override;
	void serialize(File* file) override;
	RwExtension* clone() override;
};

struct RwExtMaterialEffectsPLG_Material : RwExtension {
	// https://gtamods.com/wiki/Material_Effects_PLG_(RW_Section)

	struct NullEffect {
		static constexpr int32_t ID = 0;
		void read(File* file) {}
		void write(File* file) {}
	};
	struct BumpMapEffect {
		static constexpr int32_t ID = 1;
		float intensity = 0.0f;
		uint32_t hasBumpMap = 0;
		RwTexture bumpMap;
		uint32_t hasHeightMap = 0;
		RwTexture heightMap;
		void read(File* file);
		void write(File* file);
	};
	struct EnvironmentEffect {
		static constexpr int32_t ID = 2;
		float reflectionCoeff = 1.0f;
		uint32_t fbac = 1;
		uint32_t hasEnvMap = 0;
		RwTexture envMap;
		void read(File* file);
		void write(File* file);
	};
	struct DualTextureEffect {
		static constexpr int32_t ID = 4;
		uint32_t srcBlendMode = 0;
		uint32_t destBlendMode = 0;
		uint32_t hasTexture = 0;
		RwTexture texture;
		void read(File* file);
		void write(File* file);
	};
	struct UVTransformationEffect {
		static constexpr int32_t ID = 5;
		void read(File* file) {}
		void write(File* file) {}
	};

	uint32_t type = 0;
	std::variant<NullEffect, BumpMapEffect, EnvironmentEffect, DualTextureEffect, UVTransformationEffect> firstEffect;
	std::variant<NullEffect, BumpMapEffect, EnvironmentEffect, DualTextureEffect, UVTransformationEffect> secondEffect;

	uint32_t getType() override;
	void deserialize(File* file, const RwsHeader& header, void* parent) override;
	void serialize(File* file) override;
	RwExtension* clone() override;
};

struct RwExtMaterialEffectsPLG_Atomic : RwExtension {
	uint32_t matFxEnabled = 1;
	uint32_t getType() override;
	void deserialize(File* file, const RwsHeader& header, void* parent) override;
	void serialize(File* file) override;
	RwExtension* clone() override;
};

RwExtension *RwExtCreate(uint32_t type, uint32_t parentType);