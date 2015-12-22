#pragma once
#include "stdafx.h"
#include "Module.h"

class PatternSearcher
{
private:

	std::vector<uint8_t> signature;
	std::vector<bool> ignore;
	DWORD_PTR start;
	int size;

	void prepareSignature(std::wstring signature);
	bool compareChunk(DWORD_PTR start);

public:

	PatternSearcher(std::wstring signature, DWORD_PTR start, int size);
	PatternSearcher(std::wstring signature, Module module);
	PatternSearcher(std::vector<uint8_t> signature, DWORD_PTR start, int size); // search for a address
	PatternSearcher(std::vector<uint8_t> signature, Module module);

	DWORD_PTR getFirstResult();
	std::vector<DWORD_PTR> getResults();
};