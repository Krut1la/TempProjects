#include "DateTimeUtils.h"

namespace General
{
UINT32 GetTicks( void )
{
#ifdef _WINDOWS
   return timeGetTime();
#endif
}
}