#ifndef DATETIMEUTILS_H
#define DATETIMEUTILS_H

#include <string>
#include <Types.h>

#ifdef _WINDOWS
#include <windows.h>
#endif

namespace General
{
UINT32 GetTicks( void );
}



#endif // DATETIMEUTILS_H