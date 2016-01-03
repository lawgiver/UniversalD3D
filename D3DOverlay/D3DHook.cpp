#include "stdafx.h"
#include "D3DHook.h"

#include "D3D9Hook.h"
#include "D3D10Hook.h"
#include "D3D11Hook.h"


void HookD3D()
{
	// TODO: Decide which version to hook.

	HookD3D11();
}

void UnhookD3D()
{
	UnhookD3D11();
}
