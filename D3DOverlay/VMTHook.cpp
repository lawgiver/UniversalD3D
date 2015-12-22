#include "stdafx.h"

#include "VMTHook.h"

VMTHook::VMTHook( DWORD_PTR classPtr )
{
	this->classPtr = classPtr;
	this->orgVMT = *( DWORD_PTR** ) this->classPtr;
	this->VMTSize = getVMTSize();
	this->myVMT = new DWORD_PTR[this->VMTSize];
	memcpy( this->myVMT, this->orgVMT, this->VMTSize * sizeof( DWORD_PTR ) );
}

VMTHook::~VMTHook()
{
	delete[] this->myVMT;
}

unsigned int VMTHook::getVMTSize()
{
	unsigned int VMTSize = 0;

	while(IsBadCodePtr((FARPROC)this->orgVMT[VMTSize]) == false)
		VMTSize++;

	return VMTSize;
}

DWORD_PTR VMTHook::exchangeFunction(unsigned int index, DWORD_PTR newFunction)
{
	this->myVMT[index] = newFunction;

	return this->orgVMT[index];
}

bool VMTHook::hookVMT()
{
	*(DWORD_PTR*)this->classPtr = (DWORD_PTR)this->myVMT;
	return true;
}

bool VMTHook::unhookVMT()
{
	*(DWORD_PTR*)this->classPtr = (DWORD_PTR)this->orgVMT;
	return true;
}
