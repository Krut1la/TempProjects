#ifndef TYPES_H
#define TYPES_H

#include <crtdefs.h>

namespace General
{
   typedef unsigned short     UINT16;
   typedef unsigned int       UINT32;
   typedef unsigned __int64   K_UINT64;

   typedef signed short       INT16;
   typedef signed int         INT32;
   typedef signed __int64     K_INT64;

   typedef void*              VOID_PTR;
   typedef unsigned char      BYTE;
   typedef size_t             RWORD;

   typedef UINT32             COLOR;
     
   typedef float              RADIAN;
}

#endif // TYPES_H