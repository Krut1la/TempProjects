#ifndef HARDUTILS_H
#define HARDUTILS_H

#include "Types.h"

#include <intrin.h>

namespace General
{

#ifdef _M_X64
static inline RWORD Popcount(K_UINT64 Value)
{
   return __popcnt64(Value);
}
#else

static inline RWORD Popcount(K_UINT64 Value)
{
  UINT32 u, v;
  u = (Value & 0xffffffff);
  v = Value >> 32;
  u -= ((u >> 1) & 0x55555555);
  v -= ((v >> 1) & 0x55555555);
  u = (((u >> 2) & 0x33333333) + (u & 0x33333333));
  v = (((v >> 2) & 0x33333333) + (v & 0x33333333));
  u = (((u >> 4) + u) & 0x0f0f0f0f);
  v = (((v >> 4) + v) & 0x0f0f0f0f);
  return ((u * 0x01010101) >> 24) + ((v * 0x01010101) >> 24);
}
#endif
                    

}
#endif // HARDUTILS_H