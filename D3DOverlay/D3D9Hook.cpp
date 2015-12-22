#include "stdafx.h"
#include "D3D9Hook.h"

#include "d3d9.h"

#include "VMTHook.h"

VMTHook *D3D9DeviceVMTHook;

typedef HRESULT(__stdcall* D3D9EndScene_t)(IDirect3DDevice9 *device);
D3D9EndScene_t D3D9EndScene_o;
HRESULT __stdcall D3D9EndScene_hk(IDirect3DDevice9 *device) 
{
	__asm pushad
	Logger::getInstance() << "EndScene!!\n";
	__asm popad
	return D3D9EndScene_o(device);
}

void HookD3D9()
{
	DWORD_PTR DeviceAddress = 0x037FF3C0;
	D3D9DeviceVMTHook = new VMTHook(DeviceAddress);
	D3D9EndScene_o = (D3D9EndScene_t) D3D9DeviceVMTHook->exchangeFunction(42, (DWORD_PTR) &D3D9EndScene_hk);

}

void UnhookD3D9()
{
	D3D9DeviceVMTHook->unhookVMT();
	delete D3D9DeviceVMTHook;
}


