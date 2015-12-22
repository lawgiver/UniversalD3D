#pragma once

#include "stdafx.h"



class Module
{
private:
	std::wstring moduleName;

	HMODULE module;

	DWORD_PTR address;
	DWORD_PTR entryPoint;
	int size;

	void gatherModuleInfo();
public:
	Module(std::wstring moduleName);

	DWORD_PTR getModuleAddress();
	DWORD_PTR getEntryPoint();
	int getModuleSize();
	int getModuleOffset(DWORD_PTR address);
	DWORD_PTR getExport(std::string exportName);

	void waitForModule();
};