#include "stdafx.h"
#include "D3D9Hook.h"

#include "d3d9.h"

#include "VMTHook.h"
#include "Module.h"
#include "DrawBuffer.h"
#include "Draw.h"

struct Size
{
	int width;
	int height;
};

// TODO: Make this dynamic

Size getScreenSize()
{
	return Size{640, 500};
}

void set2DRenderStates(IDirect3DDevice9 *device)
{
	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // 0x16
	device->SetRenderState(D3DRS_WRAP0, FALSE); // 0x80

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	device->SetRenderState(D3DRS_COLORVERTEX, FALSE);

	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	device->SetRenderState(D3DRS_LIGHTING, FALSE);
}

bool fillTexture(IDirect3DTexture9 *texture, int width, int height)
{
	D3DLOCKED_RECT lockedRect;
	if (texture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD) != D3D_OK)
	{
		return false;
	}

	unsigned char *bytes = (unsigned char*)lockedRect.pBits;
	
	DrawBuffer drawBuffer(bytes, width, height);
	draw(drawBuffer);

	if (texture->UnlockRect(0) != D3D_OK)
	{
		return false;
	}

	return true;
}
IDirect3DTexture9 *createTexture(IDirect3DDevice9 *device, int width, int height)
{
	Logger::getInstance() << "Creating D3D9 Texture\n";

	IDirect3DTexture9 *result;
	HRESULT callResult = device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &result, nullptr);

	if (callResult != D3D_OK)
	{
		return nullptr;
	}

	return result;
}

struct D3DTLVERTEX 
{
	float    x, y, z, rhw; // Position
	float    tu, tv;  // Texture coordinates
};
static const DWORD D3DFVF_TLVERTEX = D3DFVF_XYZRHW | D3DFVF_TEX1;

VMTHook *D3D9DeviceVMTHook;

typedef HRESULT(__stdcall* D3D9Present_t)(IDirect3DDevice9Ex *, CONST RECT *, CONST RECT *, HWND, CONST RGNDATA *);
D3D9Present_t D3D9Present_o;
HRESULT __stdcall D3D9Present_hk(IDirect3DDevice9Ex *device, CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	__asm pushad


	static bool once = false;
	static IDirect3DTexture9 *texture = nullptr;
	static Size screenSize = getScreenSize();

	if (!once)
	{
		texture = createTexture(device, screenSize.width, screenSize.height);
		once = true;
	}

	D3DTLVERTEX vertices[4]=
	{
		{ 0.0f,		0.0f,	1.0f, 1.0f, 0.0f, 0.0f },
		{ 640.0f,	0.0f,	1.0f, 1.0f, 1.0f, 0.0f },
		{ 640.0f,	480.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		{ 0.0f,		480.0f, 1.0f, 1.0f, 0.0f, 1.0f }
	};


	device->BeginScene();

	// Save all States
	IDirect3DStateBlock9 *stateBlock;
	//device->CreateStateBlock(D3DSBT_ALL, &stateBlock);

	device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);

	fillTexture(texture, screenSize.width, screenSize.height);
	device->SetTexture(0, texture);

	device->SetFVF(D3DFVF_TLVERTEX);
	device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(D3DTLVERTEX));

	// Restore all States
	//stateBlock->Apply();

	device->EndScene();

	__asm popad;
	return D3D9Present_o(device, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

// Temporary Test Code
struct D3DState
{
	uint32_t pad1;
	IDirect3DDevice9 *device;
};

DWORD_PTR getSimpleSampleD3DDevice()
{
	DWORD_PTR moduleBase = Module(L"SimpleSample.exe").getModuleAddress();
	DWORD_PTR offset = 0x06816C;

	DWORD_PTR ptrToState = moduleBase + offset;
	D3DState *state = *(D3DState**)ptrToState;

	return (DWORD_PTR) state->device;
}

void HookD3D9()
{
	DWORD_PTR DeviceAddress = getSimpleSampleD3DDevice();
	Logger::getInstance() << "HookD3D9(): DeviceAddress = 0x" << DeviceAddress << "\n";
	

	D3D9DeviceVMTHook = new VMTHook(DeviceAddress);
	D3D9Present_o = (D3D9Present_t) D3D9DeviceVMTHook->exchangeFunction(17, (DWORD_PTR) &D3D9Present_hk);
	D3D9DeviceVMTHook->hookVMT();
}

void UnhookD3D9()
{
	D3D9DeviceVMTHook->unhookVMT();
	delete D3D9DeviceVMTHook;
}


