#include "renderer.h"
#include "window.h"
#include <d3d9.h>
#include <cassert>
#include "rw.h"

#define BGRA_TO_RGBA(x) ( (((x)&0xFF)<<16) | (((x)&0xFF0000)>>16) | ((x)&0xFF00FF00) )

D3DVERTEXELEMENT9 batchvdecl[] = {
 {0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
 {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
 {0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
 D3DDECL_END()
};

struct RVertexBufferD3D9 : public RVertexBuffer
{
	IDirect3DVertexBuffer9 *vbuf = nullptr;
	RVertex *locked;
	size_t size;
	RVertexBufferD3D9(int cnt, IDirect3DDevice9 *ddev) {
		size = cnt;
		if(cnt)
			ddev->CreateVertexBuffer(cnt * sizeof(RVertex), 0, 0, D3DPOOL_MANAGED, &vbuf, NULL);
	}
	~RVertexBufferD3D9() { if(vbuf) vbuf->Release(); }
	RVertex *lock() override {
		if (!vbuf)
			return nullptr;
		void *dat;
		vbuf->Lock(0, 0, &dat, 0);
		locked = (RVertex*)dat;
		return (RVertex*)dat;
	}
	void unlock() override {
		for (size_t i = 0; i < size; i++)
			locked[i].color = BGRA_TO_RGBA(locked[i].color);
		if(vbuf)
			vbuf->Unlock();
	}
};

struct RIndexBufferD3D9 : public RIndexBuffer
{
	IDirect3DIndexBuffer9 *ibuf = nullptr;
	RIndexBufferD3D9(int cnt, IDirect3DDevice9 *ddev) {
		if(cnt)
			ddev->CreateIndexBuffer(cnt * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ibuf, NULL);
	}
	~RIndexBufferD3D9() { if(ibuf) ibuf->Release(); }
	uint16_t *lock() override {
		if (!ibuf)
			return nullptr;
		void *dat;
		ibuf->Lock(0, 0, &dat, 0);
		return (uint16_t*)dat;
	}
	void unlock() override {
		if(ibuf)
			ibuf->Unlock();
	}
};

struct RendererD3D9 : public Renderer {
	Window *_window;
	IDirect3D9 *d3d9;
	IDirect3DDevice9 *ddev;
	IDirect3DVertexDeclaration9 *dVertDecl;
	unsigned int curWidth, curHeight;
	D3DPRESENT_PARAMETERS dpp;
	RendererD3D9(Window *window) : _window(window)
	{
		HWND hWnd = (HWND)window->getNativeWindow();
		curWidth = window->getWidth();
		curHeight = window->getHeight();
		dpp = { curWidth, curHeight, D3DFMT_UNKNOWN, 0, D3DMULTISAMPLE_NONE, 0,
			D3DSWAPEFFECT_DISCARD, 0, TRUE, TRUE, D3DFMT_D24X8, D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL,
			0, 0 };
		dpp.hDeviceWindow = hWnd;
		dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
		dpp.BackBufferCount = 2;
		dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
		HRESULT hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &dpp, &ddev);
		if(hr != D3D_OK)
			hr = d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &dpp, &ddev);
		assert(hr == D3D_OK);
		ddev->CreateVertexDeclaration(batchvdecl, &dVertDecl);
	}

	void beginFrame() override {
		static const D3DMATRIX idmx = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
		ddev->BeginScene();
		ddev->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&idmx);
		ddev->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&idmx);
		ddev->SetVertexDeclaration(dVertDecl);
	}

	void endFrame() override {
		ddev->EndScene();
		HRESULT r = ddev->Present(NULL, NULL, NULL, NULL);
		if (r == D3DERR_DEVICELOST) {
			//printf("Device lost\n");
			if (ddev->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
				ddev->Reset(&dpp);
			else
				Sleep(50);
		}
	}
	void setSize(int width, int height) override {
		if (width != curWidth || height != curHeight) {
			curWidth = width;
			curHeight = height;
			dpp.BackBufferWidth = width;
			dpp.BackBufferHeight = height;
			HRESULT r = ddev->Reset(&dpp);
		}
	}
	void clearFrame(bool clearColors, bool clearDepth, uint32_t color) override {
		DWORD flags = 0;
		if (clearColors) flags |= D3DCLEAR_TARGET;
		if (clearDepth) flags |= D3DCLEAR_ZBUFFER;
		ddev->Clear(NULL, NULL, flags, BGRA_TO_RGBA(color), 1.0f, 0);
	}

	void setTransformMatrix(const Matrix &matrix) override
	{
		ddev->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)matrix.v);
	}
	void bindTexture(int stage, texture_t texture) override
	{
		ddev->SetTexture(stage, (IDirect3DTexture9*)texture);
	}
	void unbindTexture(int stage) override
	{
		ddev->SetTexture(stage, nullptr);
	}
	void enableScissor() override
	{
		ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
	}
	void disableScissor() override
	{
		ddev->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	}
	void setScissorRect(int x, int y, int w, int h) override
	{
		RECT r = { x, y, x + w, y + h };
		ddev->SetScissorRect(&r);
	}
	texture_t createTexture(const RwImage &image) override
	{
		RwImage cimg = image.convertToRGBA32();
		for(uint32_t y = 0; y < cimg.height; y++)
			for (uint32_t x = 0; x < cimg.width; x++) {
				uint32_t *p = (uint32_t*)(cimg.pixels.data() + y * cimg.pitch + 4 * x);
				*p = BGRA_TO_RGBA(*p);
			}
		IDirect3DTexture9 *dtex;
		D3DLOCKED_RECT lore;
		ddev->CreateTexture(cimg.width, cimg.height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &dtex, NULL);
		dtex->LockRect(0, &lore, NULL, 0);
		for (uint32_t y = 0; y < cimg.height; y++)
			memcpy((char*)lore.pBits + y * lore.Pitch, cimg.pixels.data() + y * cimg.width * 4, cimg.width * 4);
		dtex->UnlockRect(0);
		return (texture_t)dtex;
	}
	void deleteTexture(texture_t texture) override
	{
		((IDirect3DTexture9*)texture)->Release();
	}
	RVertexBuffer *createVertexBuffer(int numVerts) override
	{
		return new RVertexBufferD3D9(numVerts, ddev);
	}
	RIndexBuffer *createIndexBuffer(int numIndices) override
	{
		return new RIndexBufferD3D9(numIndices, ddev);
	}
	size_t curVertBufSize = 0;
	void setVertexBuffer(RVertexBuffer *buffer) override
	{
		curVertBufSize = ((RVertexBufferD3D9*)buffer)->size;
		ddev->SetStreamSource(0, ((RVertexBufferD3D9*)buffer)->vbuf, 0, sizeof(RVertex));
	}
	void setIndexBuffer(RIndexBuffer *buffer) override
	{
		ddev->SetIndices(((RIndexBufferD3D9*)buffer)->ibuf);
	}
	void drawBuffer(int first, int count, int baseVertex) override
	{
		ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, baseVertex, 0, curVertBufSize, first, count / 3);
	}
	void initFormDrawing() override
	{
		ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
		ddev->SetRenderState(D3DRS_ZENABLE, FALSE);
		ddev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		ddev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		ddev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		Matrix m = Matrix::getZeroMatrix();
		m._11 = 2.0f / _window->getWidth();
		m._22 = -2.0f / _window->getHeight();
		m._41 = -1 - 1.f / _window->getWidth();
		m._42 = 1 + 1.f / _window->getHeight();
		m._44 = 1;
		setTransformMatrix(m);
	}
	void drawRect(float x, float y, float w, float h, uint32_t c) override
	{
		RVertex verts[5];
		verts[0].x = x;		verts[0].y = y;
		verts[1].x = x + w;	verts[1].y = y;
		verts[2].x = x + w;	verts[2].y = y + h;
		verts[3].x = x;		verts[3].y = y + h;
		verts[4].x = x;		verts[4].y = y;
		for (int i = 0; i < 5; i++) { verts[i].color = c; verts[i].x -= 0.5f; verts[i].y -= 0.5f; verts[i].z = verts[i].u = verts[i].v = 0.0f; }
		ddev->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, verts, sizeof(RVertex));
	}
	void fillRect(float x, float y, float w, float h, uint32_t c, float u, float v, float o, float p) override
	{
		RVertex verts[4];
		verts[0].x = x;		verts[0].y = y;		verts[0].u = u;		verts[0].v = v;
		verts[1].x = x + w;	verts[1].y = y;		verts[1].u = u + o;	verts[1].v = v;
		verts[2].x = x;		verts[2].y = y + h;	verts[2].u = u;		verts[2].v = v + p;
		verts[3].x = x + w;	verts[3].y = y + h;	verts[3].u = u + o;	verts[3].v = v + p;
		for (int i = 0; i < 4; i++) { verts[i].color = c; verts[i].x -= 0.5f; verts[i].y -= 0.5f; verts[i].z = 0.0f; }
		ddev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(RVertex));
	}
	void drawLine3D(const Vector3 &start, const Vector3 &end, uint32_t c) override
	{
		RVertex verts[2];
		verts[0].x = start.x; verts[0].y = start.y; verts[0].z = start.z; verts[0].color = c; verts[0].u = verts[0].v = 0.0f;
		verts[1].x = end.x; verts[1].y = end.y; verts[1].z = end.z; verts[1].color = c; verts[1].u = verts[1].v = 0.0f;
		ddev->DrawPrimitiveUP(D3DPT_LINELIST, 1, verts, sizeof(RVertex));
	}
	void initModelDrawing() override
	{
		ddev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
		ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
		ddev->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		ddev->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		ddev->SetRenderState(D3DRS_ALPHAREF, 240);
		//ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
		ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		ddev->SetRenderState(D3DRS_BLENDFACTOR, 0xFFFFFFFF);
		ddev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		ddev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		ddev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
	}
	void setBlendColor(uint32_t color) override {
		ddev->SetRenderState(D3DRS_BLENDFACTOR, BGRA_TO_RGBA(color));
	}
};

Renderer * CreateRendererD3D9(Window *window)
{
	return new RendererD3D9(window);
}
