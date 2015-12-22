#include "stdafx.h"
#include "Logger.h"

Logger *Logger::instance = nullptr;
std::mutex Logger::creationMutex;

void Logger::createInstance(std::wstring fileName)
{
	std::lock_guard<std::mutex> lock(creationMutex);

	if (instance == nullptr)
		instance = new Logger(fileName);
}

Logger &Logger::getInstance()
{
	return *instance;
}

Logger::Logger(std::wstring fileName) : logStream(fileName, std::ios_base::out | std::ios_base::app )
{
	if (!logStream.good())
		throw new std::runtime_error("Logger(): logStream could not be opened!");
}

Logger::~Logger()
{
	this->logStream.flush();
}
