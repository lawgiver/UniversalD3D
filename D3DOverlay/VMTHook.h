#pragma once

#include "stdafx.h"

class VMTHook
{
private:
	DWORD_PTR classPtr;
	DWORD_PTR *myVMT, *orgVMT;
	unsigned int VMTSize;

public:
	VMTHook( DWORD_PTR classPtr );
	~VMTHook();

	unsigned int getVMTSize();

	DWORD_PTR exchangeFunction( unsigned int index, DWORD_PTR newFunction );

	bool hookVMT();
	bool unhookVMT();
};