// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
// C RunTime Header Files

#ifdef INSTALL_DLL_EXPORTS
#define INSTALL_DLL_API __declspec(dllexport)
#else
#define INSTALL_DLL_API __declspec(dllimport)
#endif

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <Winbase.h>
#include <shlwapi.h>
#include <tchar.h>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>

#include <krutila.h>
#include <expat.h>

// TODO: reference additional headers your program requires here
