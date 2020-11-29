#pragma once

#include "vecmat.h"
#include <cstdint>

struct Window;
struct RwImage;

typedef void* texture_t;

struct RVertex {
	union {
		struct { float x, y, z; };
		float pos[3];
	};
	uint32_t color;
	union {
		struct { float u, v; };
		float texcoord[2];
	};
	RVertex() {}
	RVertex(float _x, float _y, float _z, uint32_t _color, float _u, float _v) : x(_x), y(_y), z(_z), color(_color), u(_u), v(_v) {}
};

struct RVertexBuffer
{
	virtual ~RVertexBuffer() {}
	virtual RVertex *lock() = 0;
	virtual void unlock() = 0;
};

struct RIndexBuffer
{
	virtual ~RIndexBuffer() {}
	virtual uint16_t *lock() = 0;
	virtual void unlock() = 0;
};

struct Renderer {
	// Frame start/end
	virtual void beginFrame() = 0;
	virtual void endFrame() = 0;
	virtual void setSize(int width, int height) = 0;

	// State changes
	virtual void setTransformMatrix(const Matrix &matrix) = 0;
	virtual void bindTexture(int stage, texture_t texture) = 0;
	virtual void unbindTexture(int stage) = 0;
	virtual void enableScissor() = 0;
	virtual void disableScissor() = 0;
	virtual void setScissorRect(int x, int y, int w, int h) = 0;
	virtual void setBlendColor(uint32_t color) = 0;
	virtual void setBackgroundColor(uint32_t color) = 0;

	// Texture management
	virtual texture_t createTexture(const RwImage &image) = 0;
	virtual void deleteTexture(texture_t texture) = 0;

	// Buffer drawing
	virtual RVertexBuffer *createVertexBuffer(int nv) = 0;
	virtual RIndexBuffer *createIndexBuffer(int ni) = 0;
	virtual void setVertexBuffer(RVertexBuffer *_rv) = 0;
	virtual void setIndexBuffer(RIndexBuffer *_ri) = 0;
	virtual void drawBuffer(int first, int count) = 0;

	// Form Drawing
	virtual void initFormDrawing() = 0;
	virtual void drawRect(float x, float y, float w, float h, uint32_t c = 0xFFFFFFFF) = 0;
	virtual void fillRect(float x, float y, float w, float h, uint32_t c = 0xFFFFFFFF, float u = 0.0f, float v = 0.0f, float o = 1.0f, float p = 1.0f) = 0;
	virtual void drawLine3D(const Vector3 &start, const Vector3 &end, uint32_t color = 0xFFFFFFFF) = 0;

	// Model drawing
	virtual void initModelDrawing() = 0;
};

Renderer *CreateRendererOGL1(Window *window);
Renderer *CreateRendererD3D9(Window *window);