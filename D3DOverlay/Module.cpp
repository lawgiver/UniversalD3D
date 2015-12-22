#include "stdafx.h"
#include "Module.h"

Module::Module(std::wstring moduleName)
{
	this->moduleName = moduleName;

	this->address = -1;
	this->size = -1;
	this->entryPoint = -1;

	this->module = nullptr;
}

DWORD_PTR Module::getModuleAddress()
{
	if (this->address == -1)
		gatherModuleInfo();

	return this->address;
}

DWORD_PTR Module::getEntryPoint()
{
	if (this->entryPoint == -1)
		gatherModuleInfo();

	return this->entryPoint;
}

int Module::getModuleSize()
{
	if (this->size == -1)
		gatherModuleInfo();

	return this->size;
}

int Module::getModuleOffset(DWORD_PTR address)
{
	return address - getModuleAddress();
}

DWORD_PTR Module::getExport(std::string exportName)
{
	if (this->module == nullptr)
		gatherModuleInfo();

	FARPROC proc = GetProcAddress(this->module, exportName.c_str());

	if (proc == nullptr)
		throw std::runtime_error("getExport(): GetProcAddress returned nullptr!");

	assert(sizeof(FARPROC) == sizeof(DWORD_PTR));

	return (DWORD_PTR)proc;
}

void Module::waitForModule()
{
	while (!GetModuleHandle(this->moduleName.c_str()))
		Sleep(100);
}

void Module::gatherModuleInfo() 
{
	module = GetModuleHandle(this->moduleName.c_str());
	if (module == INVALID_HANDLE_VALUE)
		throw std::runtime_error("gatherModuleInfo(): GetModuleHandle failed!");

	MODULEINFO moduleInfo;
	if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(moduleInfo)))
		throw std::runtime_error("gatherModuleInfo(): GetModuleInformation failed!");

	this->address = (DWORD_PTR)moduleInfo.lpBaseOfDll;
	this->entryPoint = (DWORD_PTR)moduleInfo.EntryPoint;
	this->size = moduleInfo.SizeOfImage;
}