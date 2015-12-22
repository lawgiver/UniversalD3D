#include "stdafx.h"

#include "PatternSearcher.h"
#include "Logger.h"



PatternSearcher::PatternSearcher(std::wstring signature, DWORD_PTR start, int size)
{
	this->start = start;
	this->size = size;

	this->prepareSignature(signature);
}

PatternSearcher::PatternSearcher(std::wstring signature, Module module)
{
	this->start = module.getModuleAddress();
	this->size = module.getModuleSize();

	this->prepareSignature(signature);
}

PatternSearcher::PatternSearcher(std::vector<uint8_t> signature, DWORD_PTR start, int size)
{
	this->start = start;
	this->size = size;

	this->signature = signature;
	for (unsigned int i = 0; i < this->signature.size(); i++)
		this->ignore.push_back(false);
}

PatternSearcher::PatternSearcher(std::vector<uint8_t> signature, Module module)
{
	this->start = module.getModuleAddress();
	this->size = module.getModuleSize();

	this->signature = signature;
	for (unsigned int i = 0; i < this->signature.size(); i++)
		this->ignore.push_back(false);
}

void PatternSearcher::prepareSignature(std::wstring signature)
{
	std::wstringstream signatureStream(signature);
	std::vector<std::wstring> byteStrings;

	std::copy(	std::istream_iterator<std::wstring, wchar_t>(signatureStream), // Start at the beginning
				std::istream_iterator<std::wstring, wchar_t>(), // to EOF
				std::back_inserter<std::vector<std::wstring>>(byteStrings) );
		

	for (std::wstring s : byteStrings)
	{
		if (s == L"??") // Ignore this BYTE in our later signature
		{
			this->signature.push_back(00);
			this->ignore.push_back(true);
		}
		else
		{
			int newByte = std::stoi(s.c_str(), nullptr, 16);

			// Check if we have a valid byte
			assert(newByte >= 0);
			assert(newByte <= 0xFF);

			this->signature.push_back(newByte);
			this->ignore.push_back(false);
		}
	}

	assert(this->signature.size() == this->ignore.size());

}

bool PatternSearcher::compareChunk(DWORD_PTR start)
{
	int size = signature.size();

	for (int i = 0; i < size; i++)
	{
		uint8_t *byte = reinterpret_cast<uint8_t*>(start + i);
		if (*byte != signature.at(i) && !ignore.at(i))
			return false;
	}

	return true;
}

DWORD_PTR PatternSearcher::getFirstResult()
{
	for (int i = 0; i < this->size; i++)
	{
		if (this->compareChunk(this->start + i))
			return(this->start + i);
	}
	return -1;
}

std::vector<DWORD_PTR> PatternSearcher::getResults()
{
	std::vector<DWORD_PTR> results;
	for (int i = 0; i < this->size; i++)
	{
		if (this->compareChunk(this->start + i))
			results.push_back(this->start + i);
	}
	return results;
}
