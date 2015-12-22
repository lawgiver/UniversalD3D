// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>


#include <string>
#include <vector>
#include <exception>
#include <sstream>
#include <iterator>
#include <thread>
#include <mutex>
#include <fstream>
#include <cassert>
#include <memory>
#include <regex>
#include <map>

#include "Logger.h"