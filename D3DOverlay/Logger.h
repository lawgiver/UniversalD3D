#pragma once
#include "stdafx.h"


class Logger
{
private:
	static Logger *instance;
	static std::mutex creationMutex;
	
	Logger(std::wstring fileName);
	~Logger();

	std::wfstream logStream;
	std::mutex streamMutex;
protected:
public:
	static void createInstance(std::wstring fileName);
	static Logger& getInstance();

	template<class T>
		void log(const T& t);

	template<class T>
		Logger &operator<<(const T& t);

		inline void setHex()
		{
			std::lock_guard<std::mutex> lock(streamMutex);
			logStream << std::hex;
		}
		inline void setDec()
		{
			std::lock_guard<std::mutex> lock(streamMutex);
			logStream << std::dec;
		}
};

template<class T>
inline void Logger::log(const T& t)
{
#ifdef _DEBUG
	std::lock_guard<std::mutex> lock(streamMutex);
	logStream << t;
	logStream.flush();
#endif
}

template<class T>
inline Logger &Logger::operator<<(const T& t)
{
	log(t);
	return *this;
}
