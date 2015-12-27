// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "D3DHook.h"

DWORD WINAPI initThread(_In_ LPVOID lpParameter)
{
	HMODULE myModule = *static_cast<HMODULE*>(lpParameter);

	wchar_t dllPathBuffer[512];
	GetModuleFileName(myModule, dllPathBuffer, 512);
	std::wstring dllPath(dllPathBuffer);
	dllPath.erase(dllPath.length() - 3); // erase dll

	Logger::createInstance(dllPath + L"log");
	Logger::getInstance().setHex(); // Lets make this default
	Logger::getInstance() << L"Logger class initialized and initThread() called \n";

	HookD3D();

	return TRUE;
}


DWORD WINAPI unloadThread(_In_ LPVOID lpParameter)
{
	HMODULE myModule = *static_cast<HMODULE*>(lpParameter);

	while (!GetKeyState(VK_END))
		Sleep(100);

	Logger::getInstance() << "unloadThread(): Cleaning up and calling FreeLibraryAndExitThread() \n";

	UnhookD3D();

	FreeLibraryAndExitThread(myModule, 0);

	return TRUE;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HMODULE *module = new HMODULE;
		*module = hModule;
		CreateThread(0, 0, &initThread, module, 0, 0);
		CreateThread(0, 0, &unloadThread, module, 0, 0);
		break;
	}	
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

