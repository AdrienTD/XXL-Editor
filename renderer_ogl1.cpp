#include "renderer.h"
#include "window.h"
#include "rw.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "DynArray.h"

#define BGRA_TO_RGBA(x) ( (((x)&0xFF)<<16) | (((x)&0xFF0000)>>16) | ((x)&0xFF00FF00) )

struct RVertexBufferOGL1 : public RVertexBuffer
{
	DynArray<RVertex> verts;
	RVertexBufferOGL1(int cnt) : verts(cnt) {}
	RVertex *lock() override { return verts.data(); }
	void unlock() override
	{
		//for (RVertex &v : verts)
		//	v.color = BGRA_TO_RGBA(v.color);
	}
};

struct RIndexBufferOGL1 : public RIndexBuffer
{
	DynArray<uint16_t> indices;
	RIndexBufferOGL1(int cnt) : indices(cnt) {}
	uint16_t *lock() override { return indices.data(); }
	void unlock() override {}
};

struct RendererOGL1 : Renderer {
	Window *_window;
	SDL_GLContext _context;
	RVertexBufferOGL1 *_currentVertexBuffer = nullptr;
	RIndexBufferOGL1 *_currentIndexBuffer = nullptr;

	RendererOGL1(Window *window) : _window(window)
	{
		_context = SDL_GL_CreateContext(_window->getSDLWindow());

		SDL_GL_SetSwapInterval(1);

		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		//glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_2D);
		glAlphaFunc(GL_GEQUAL, 240.0f / 255.0f);
		glCullFace(GL_FRONT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	void beginFrame() override
	{
		glViewport(0, 0, _window->getWidth(), _window->getHeight());
	}
	void endFrame() override
	{
		SDL_GL_SwapWindow(_window->getSDLWindow());
	}
	void setSize(int width, int height) override {}
	void clearFrame(bool clearColors, bool clearDepth, uint32_t color) override {
		GLbitfield flags = 0;
		if (clearColors) flags |= GL_COLOR_BUFFER_BIT;
		if (clearDepth) flags |= GL_DEPTH_BUFFER_BIT;
		glClearColor(0.25f, 0.25f, 1.0f, 1.0f);
		glClear(flags);
	}
	void setTransformMatrix(const Matrix &matrix) override
	{
		glLoadMatrixf(matrix.v);
	}
	void bindTexture(int stage, texture_t texture) override
	{
		glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
	}
	void unbindTexture(int stage) override
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	void enableScissor() override
	{
		glEnable(GL_SCISSOR_TEST);
	}
	void disableScissor() override
	{
		glDisable(GL_SCISSOR_TEST);
	}
	void setScissorRect(int x, int y, int w, int h) override
	{
		glScissor(x, _window->getHeight() - 1 - (y + h), w, h);
	}
	texture_t createTexture(const RwImage &image) override
	{
		RwImage cimg = image.convertToRGBA32();
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, cimg.width, cimg.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, cimg.pixels.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		return (texture_t)tex;
	}
	void deleteTexture(texture_t texture) override
	{
		GLuint gltex = (GLuint)texture;
		glDeleteTextures(1, &gltex);
	}
	RVertexBuffer *createVertexBuffer(int numVerts) override
	{
		return new RVertexBufferOGL1(numVerts);
	}
	RIndexBuffer *createIndexBuffer(int numInidces) override
	{
		return new RIndexBufferOGL1(numInidces);
	}
	void setVertexBuffer(RVertexBuffer *buffer) override
	{
		_currentVertexBuffer = (RVertexBufferOGL1*)buffer;
	}
	void setIndexBuffer(RIndexBuffer *buffer) override
	{
		_currentIndexBuffer = (RIndexBufferOGL1*)buffer;
	}
	void drawBuffer(int first, int count) override
	{
		RVertex *vert = _currentVertexBuffer->verts.data();

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(RVertex), &vert->x);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(RVertex), &vert->color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(RVertex), &vert->u);
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, _currentIndexBuffer->indices.data() + first);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
	void initFormDrawing() override
	{
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		Matrix m = Matrix::getZeroMatrix();
		m._11 = 2.0f / _window->getWidth();
		m._22 = -2.0f / _window->getHeight();
		m._41 = -1;
		m._42 = 1;
		m._44 = 1;
		glLoadMatrixf(m.v);
	}
	void drawRect(float x, float y, float w, float h, uint32_t c) override
	{
		glColor4ub((c >> 16) & 255, (c >> 8) & 255, c & 255, (c >> 24) & 255);
		glBegin(GL_LINE_STRIP);
		glVertex3f(x, y, 0.5f);
		glVertex3f(x + w, y, 0.5f);
		glVertex3f(x + w, y + h, 0.5f);
		glVertex3f(x, y + h, 0.5f);
		glVertex3f(x, y, 0.5f);
		glEnd();
	}
	void fillRect(float x, float y, float w, float h, uint32_t c, float u, float v, float o, float p) override
	{
		glColor4ub((c >> 16) & 255, (c >> 8) & 255, c & 255, (c >> 24) & 255);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(u, v); glVertex3f(x, y, 0.5f);
		glTexCoord2f(u + o, v); glVertex3f(x + w, y, 0.5f);
		glTexCoord2f(u, v + p); glVertex3f(x, y + h, 0.5f);
		glTexCoord2f(u + o, v + p); glVertex3f(x + w, y + h, 0.5f);
		glEnd();
	}
	void drawLine3D(const Vector3 &start, const Vector3 &end, uint32_t c) override
	{
		glColor4ub((c >> 16) & 255, (c >> 8) & 255, c & 255, (c >> 24) & 255);
		glBegin(GL_LINES);
		glVertex3f(start.x, start.y, start.z);
		glVertex3f(end.x, end.y, end.z);
		glEnd();
	}
	void initModelDrawing() override
	{
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void setBlendColor(uint32_t color) override {
		// TODO
	}
};

Renderer * CreateRendererOGL1(Window *window)
{
	return new RendererOGL1(window);
}
