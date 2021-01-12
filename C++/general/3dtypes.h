#ifndef _3DTYPES_H
#define _3DTYPES_H

#include <vector>

namespace General
{
   typedef enum {
      tfGrayScale,
      tfRGB,
      tfARGB,
      tfXRGB   
   }TEX_FORMAT;

   typedef struct {
       float R;
       float G;
       float B;
       float A;
   } COLORVALUE;

   typedef struct VECTOR3F
   {
      float X;
      float Y;
      float Z;
      inline VECTOR3F( void ){}
      inline VECTOR3F( float X, float Y, float Z ){
         this->X = X;
         this->Y = Y;
         this->Z = Z;
      }

   } VECTOR3F;

   typedef struct MATRIX4F {
      union {
         float m[4][4];
         struct {
               float        _11, _12, _13, _14;
               float        _21, _22, _23, _24;
               float        _31, _32, _33, _34;
               float        _41, _42, _43, _44;
           };
      };

      inline MATRIX4F( void ){}

      inline MATRIX4F(
         float _11, float _12, float _13, float _14,
         float _21, float _22, float _23, float _24,
         float _31, float _32, float _33, float _34,
         float _41, float _42, float _43, float _44 )
      {
         m[0][0] = _11;
         m[0][1] = _12;
         m[0][2] = _13;
         m[0][3] = _14;
         m[1][0] = _21;
         m[1][1] = _22;
         m[1][2] = _23;
         m[1][3] = _24;
         m[2][0] = _31;
         m[2][1] = _32;
         m[2][2] = _33;
         m[2][3] = _34;
         m[3][0] = _41;
         m[3][1] = _42;
         m[3][2] = _43;
         m[3][3] = _44;
      }
      static const MATRIX4F ZERO;
      static const MATRIX4F IDENTITY;

      static void CreateProjectionMatrix( const RADIAN FovY, float Aspect, float NearPlane, 
                                 float FarPlane, MATRIX4F& Dest, bool ForGpuProgram );

   } MATRIX4F;

   typedef enum {
      ttWorld,
      ttView,
      ttProj
   }TRANSFORMTYPE;

   typedef struct {
      TRANSFORMTYPE   Type;
      MATRIX4F      Matrix;
   }TRANSFORM;

   typedef struct VECTOR4F
   {
      float X;
      float Y;
      float Z;
      float W;

      inline VECTOR4F( void ){}
      inline VECTOR4F( float X, float Y, float Z, float W = 1.0f ){
         this->X = X;
         this->Y = Y;
         this->Z = Z;
         this->W = W;
      }
      inline VECTOR4F( const VECTOR3F& Vector3f )
      {
         VECTOR4F( Vector3f.X, Vector3f.Y, Vector3f.Z );
      }
   } VECTOR4F;

   typedef struct
   {
      VECTOR3F   P;
      VECTOR3F   N;
      float         U;
      float         V;
   } VERTEX_MODEL;

   typedef struct
   {
      VECTOR4F   P;
      COLOR      C;
      COLOR      S;
      float      U;
      float      V;
   } VERTEX_SPRITE_T;

   typedef struct
   {
      VECTOR3F   P;
      COLOR      C;
      COLOR      S;
      float      U;
      float      V;
   } VERTEX_SPRITE;

   typedef struct
   {
      RWORD V1Index;
      RWORD V2Index;
      RWORD V3Index;
   } FACET;

   typedef UINT32   INDEX32;
   typedef UINT16   INDEX16;

   typedef enum {   ptTriangleFan, 
                  ptTriangleList,
                  ptTriangleStrip,
                  ptLineList,
                  ptPoints
   } PRIMITIVETYPE;

   typedef struct
   {
      RWORD VerticesCount;
      RWORD IndicesCount;
      RWORD PrimitivesCount;

      VERTEX_MODEL*  Vertices;
      INDEX32*       Indices;

      PRIMITIVETYPE PrimitiveType;
   } MESH;

   typedef struct
   {
      MESH      Mesh;
      RWORD    MaterialIndex;
   } DETAIL;

   typedef enum  {
       ltPoint          = 1,
       ltSpot           = 2,
       ltDirectional    = 3
   } LIGHTTYPE;

   typedef struct
   {
      LIGHTTYPE   Type;
      COLORVALUE   Diffuse;
      COLORVALUE   Specular;
      COLORVALUE   Ambient;
      VECTOR3F    Position;
      VECTOR3F    Direction;
      float       Range;
      float       Falloff;
      float       Attenuation0;
      float       Attenuation1;
      float       Attenuation2;
      float       Theta;
      float       Phi;
   } LIGHT;

   typedef std::vector<DETAIL*> DETAILLIST;
   typedef std::vector<LIGHT*> LIGHTLIST;
}

#endif // _3DTYPES_H