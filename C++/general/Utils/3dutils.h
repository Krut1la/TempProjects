#ifndef _3DUTILS_H
#define _3DUTILS_H

#include "Types.h"
#include "3DTypes.h"

namespace General
{
const COLOR clBlack    = 0xFF000000;
const COLOR clRed      = 0xFFFF0000;
const COLOR clGreen    = 0xFF00FF00;
const COLOR clBlue     = 0xFF0000FF;

COLOR RGBA( BYTE R, BYTE G, BYTE B, BYTE A );
COLOR ABGR( BYTE A, BYTE B, BYTE G, BYTE R );
COLOR ARGB( BYTE A, BYTE R, BYTE G, BYTE B );

VECTOR3F operator+( const VECTOR3F& Vector3f_1, const VECTOR3F& Vector3f_2 );
VECTOR3F operator*( const float Scalar, const VECTOR3F& Vector3f );

float VectorLength( const VECTOR3F& Vector3f );
VECTOR3F NormalizeVector( const VECTOR3F& Vector3f );

// Shapes
DETAIL* CreateSphere( const VECTOR3F& Center, const float Radius, UINT32 RingsCount = 10 );
}

#endif // _3DUTILS_H