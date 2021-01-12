#include "Model.h"
#include "3DUtils.h"

namespace RTS
{
CSprite::CSprite( void* Resources ):
m_Resources( Resources )
{
   m_MaterialIndex = 0;
   m_Color =  ARGB( 0xFF, rand(), rand(), rand() );
}

CSprite::~CSprite( void )
{
}

bool CSprite::Render( CRenderer* Renderer )
{
   return true;
}


CModel::CModel( void* Resources ):
m_Resources( Resources )
{
}

CModel::~CModel( void )
{
   Clear();
}

void CModel::Clear( void )
{
   for( DETAILLIST::iterator iDetail = m_Details.begin();  iDetail != m_Details.end(); ++iDetail)
   {
      delete [] (*iDetail)->Mesh.Indices;
      delete [] (*iDetail)->Mesh.Vertices;
      delete (*iDetail);
   }

   m_Details.clear();
}

bool CModel::LoadFromFile( const std::wstring& FileName, UINT32 MaterialIndex )
{

   return true;
}

void CModel::Translate( float X, float Y, float Z)
{
   for( DETAILLIST::iterator iDetail = m_Details.begin();  iDetail != m_Details.end(); ++iDetail)
   {
      if ( (*iDetail)->Mesh.Vertices != NULL )
      {
         for (UINT32 iVertex = 0; iVertex < (*iDetail)->Mesh.VerticesCount;  ++iVertex)
         {
            (*iDetail)->Mesh.Vertices[iVertex].P.X += X;
            (*iDetail)->Mesh.Vertices[iVertex].P.Y += Y;
         }
      }

   }
}

bool CModel::Render( CRenderer* Renderer )
{
   return true;
}

}