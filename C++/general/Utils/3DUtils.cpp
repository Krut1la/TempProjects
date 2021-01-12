#include "3DUtils.h"
#include "MathUtils.h"

namespace General
{
const MATRIX4F MATRIX4F::ZERO(
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0 );

const MATRIX4F MATRIX4F::IDENTITY(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1 );

COLOR RGBA( BYTE R, BYTE G, BYTE B, BYTE A )
{
   COLOR Result = (((COLOR)R)<<24) + (((COLOR)G)<<16) + (((COLOR)B<<8)) + A;

   return Result;
}

COLOR ABGR( BYTE A, BYTE B, BYTE G, BYTE R )
{
   COLOR Result = (((COLOR)A)<<24) + (((COLOR)B)<<16) + (((COLOR)G<<8)) + R;

   return Result;
}

COLOR ARGB( BYTE A, BYTE R, BYTE G, BYTE B )
{
   COLOR Result = (((COLOR)A)<<24) + (((COLOR)R)<<16) + (((COLOR)G<<8)) + B;

   return Result;
}

VECTOR3F operator+( const VECTOR3F& Vector3f_1, const VECTOR3F& Vector3f_2 )
{
   VECTOR3F Result;

   Result.X = Vector3f_1.X + Vector3f_2.X;
   Result.Y = Vector3f_1.Y + Vector3f_2.Y;
   Result.Z = Vector3f_1.Z + Vector3f_2.Z;

   return Result;
}

VECTOR3F operator*( const float Scalar, const VECTOR3F& Vector3f )
{
   VECTOR3F Result;

   Result.X = Vector3f.X * Scalar;
   Result.Y = Vector3f.Y * Scalar;
   Result.Z = Vector3f.Z * Scalar;

   return Result;
}

float VectorLength( const VECTOR3F& Vector3f )
{               
   float Result = Vector3f.X*Vector3f.X + 
              Vector3f.Y*Vector3f.Y + 
              Vector3f.Z*Vector3f.Z;

   return sqrt( Result );
}

VECTOR3F NormalizeVector( const VECTOR3F& Vector3f )
{
   float Length = VectorLength( Vector3f );
   if ( Length != 0.0f ) 
   {
      Length = 1.0f / Length;
   }

   return Length*Vector3f;
}

DETAIL* CreateSphere( const VECTOR3F& Center, const float Radius, UINT32 RingsCount )
{
   DETAIL* Detail = new DETAIL();

    UINT32      VerticesCount = (RingsCount*(2*RingsCount+1)+2);
    UINT32      IndicesCount  = 6*(RingsCount*2)*((RingsCount-1)+1);

   Detail->Mesh.Vertices = new VERTEX_MODEL[VerticesCount];
   Detail->Mesh.Indices = new INDEX32[IndicesCount];

    // Counters
    UINT32 x;
   UINT32 y;
   UINT32 iVertex = 0; 
   UINT32 iIndex = 0;

    // Angle deltas for constructing the sphere's vertices
    float fDAng   = (float)(M_PI / RingsCount);
    float fDAngY0 = fDAng;

    // Make the middle of the sphere
    for( y = 0; y < RingsCount; y++ )
    {
        float y0 = (float)cosf(fDAngY0);
        float r0 = (float)sinf(fDAngY0);
        float tv = (1.0f - y0)/2;

        for( x = 0; x < (RingsCount*2) + 1; x++ )
        {
            float fDAngX0 = x*fDAng;
        
            VECTOR3F v = VECTOR3F( r0*(float)sinf(fDAngX0), y0, r0*(float)cosf(fDAngX0) );
            float tu = 1.0f - x/(RingsCount*2.0f);

            Detail->Mesh.Vertices[iVertex].P = Radius*v;
         Detail->Mesh.Vertices[iVertex].N = v;
         Detail->Mesh.Vertices[iVertex].U = tu;
         Detail->Mesh.Vertices[iVertex].V = tv;

            iVertex++;
        }
        fDAngY0 += fDAng;
    }

    for( y = 0; y < RingsCount - 1; y++ )
    {
        for( x = 0; x < (RingsCount*2); x++ )
        {
            Detail->Mesh.Indices[iIndex++] = (INDEX32)( (y + 0)*(RingsCount*2+1) + (x + 0) );
            Detail->Mesh.Indices[iIndex++] = (INDEX32)( (y + 1)*(RingsCount*2+1) + (x + 0) );
            Detail->Mesh.Indices[iIndex++] = (INDEX32)( (y + 0)*(RingsCount*2+1) + (x + 1) );
            Detail->Mesh.Indices[iIndex++] = (INDEX32)( (y + 0)*(RingsCount*2+1) + (x + 1) );
            Detail->Mesh.Indices[iIndex++] = (INDEX32)( (y + 1)*(RingsCount*2+1) + (x + 0) ); 
            Detail->Mesh.Indices[iIndex++] = (INDEX32)( (y + 1)*(RingsCount*2+1) + (x + 1) );
        }
    }

    // Make top and bottom
    VECTOR3F vy = VECTOR3F( 0.0f, 1.0f, 0.0f );
    UINT32 wNorthVtx = iVertex;

   Detail->Mesh.Vertices[iVertex].P = Radius*vy;
   Detail->Mesh.Vertices[iVertex].N = vy;
   Detail->Mesh.Vertices[iVertex].U = 0.5f;
   Detail->Mesh.Vertices[iVertex].V = 0.0f;

    iVertex++;
    UINT32 wSouthVtx = iVertex;

   Detail->Mesh.Vertices[iVertex].P = -Radius*vy;
   Detail->Mesh.Vertices[iVertex].N = -1.0*vy;
   Detail->Mesh.Vertices[iVertex].U = 0.5f;
   Detail->Mesh.Vertices[iVertex].V = 1.0f;

    iVertex++;

    for( x = 0; x < (RingsCount*2); x++ )
    {
        INDEX32 p1 = wSouthVtx;
        INDEX32 p2 = (INDEX32)( (y)*(RingsCount*2 + 1) + (x + 1) );
        INDEX32 p3 = (INDEX32)( (y)*(RingsCount*2 + 1) + (x + 0) );

        Detail->Mesh.Indices[iIndex++] = p1;
        Detail->Mesh.Indices[iIndex++] = p3;
        Detail->Mesh.Indices[iIndex++] = p2;
    }

    for( x = 0; x <(RingsCount*2); x++ )
    {
        INDEX32 p1 = wNorthVtx;
        INDEX32 p2 = (INDEX32)( (0)*(RingsCount*2 + 1) + (x + 1) );
        INDEX32 p3 = (INDEX32)( (0)*(RingsCount*2 + 1) + (x + 0) );

        Detail->Mesh.Indices[iIndex++] = p1;
        Detail->Mesh.Indices[iIndex++] = p3;
        Detail->Mesh.Indices[iIndex++] = p2;
    }

   Detail->Mesh.VerticesCount = iVertex;
   Detail->Mesh.IndicesCount = iIndex;
   Detail->Mesh.PrimitiveType = ptTriangleList;

   return Detail;
}

/*
D3DMATRIX 
CD3DRenderer::GetProjectionMatrix(const float near_plane, // Distance to near clipping 
                                         // plane
                 const float far_plane,  // Distance to far clipping 
                                         // plane
                 const float fov_horiz,  // Horizontal field of view 
                                         // angle, in radians
                 const float fov_vert)   // Vertical field of view 
                                         // angle, in radians
{
    float    h, w, Q;

    w = (float)1/(float)tan(fov_horiz*0.5);  // 1/tan(x) == cot(x)
    h = (float)1/(float)tan(fov_vert*0.5);   // 1/tan(x) == cot(x)
    Q = far_plane/(far_plane - near_plane);

    D3DMATRIX ret;
    ZeroMemory(&ret, sizeof(ret));


   ret._11 = w;
   ret._22 = h;
   ret._33 = Q;
   ret._21 = -Q*near_plane;
   ret._12 = 1;
    
    return ret;
}   // End of ProjectionMatrix
*/

void MATRIX4F::CreateProjectionMatrix( const RADIAN FovY, float Aspect, float NearPlane, 
   float FarPlane, MATRIX4F& Dest, bool ForGpuProgram )
{
   RADIAN theta ( FovY * 0.5f );
   float h = 1 / tanf(theta);
   float w = h / Aspect;
   float q, qn;

   if (FarPlane == 0)
   {
      q = 1 - 0.00001f;
      qn = NearPlane * (0.00001f - 1);
   }
   else
   {
      q = FarPlane / ( FarPlane - NearPlane );
      qn = -q * NearPlane;
   }

   Dest = MATRIX4F::ZERO;
   Dest.m[0][0] = w;
   Dest.m[1][1] = h;

   if (ForGpuProgram)
   {
      Dest.m[2][2] = -q;
      Dest.m[3][2] = -1.0f;
   }
   else
   {
      Dest.m[2][2] = q;
      Dest.m[3][2] = 1.0f;
   }

   Dest.m[2][3] = qn;
}
}