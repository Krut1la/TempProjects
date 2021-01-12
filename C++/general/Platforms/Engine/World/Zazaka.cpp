#include "Zazaka.h"
#include "Resources.h"
#include "World.h"
#include "Engine.h"
#include "3DUtils.h"

#include <math.h>

namespace RTS
{
CZazaka::CZazaka( void* World ):
CUnit( World )
{
   m_LastTime = 0;

   m_ZazakaModelRef = 2;

   m_Position = VECTOR3F( 400.0f, 300.0f, 0.0f );
   m_Scale = VECTOR3F( 64.0f, 64.0f, 0.0f );
   m_Rotation = VECTOR3F( 0.0f, 0.0f, 0.0f );

   m_Color = ARGB( 0xFF, rand(), rand(), rand() );
//   dx = 0.1f*((float)rand() / (RAND_MAX + 1) * (2));
//   dy = 0.1f*((float)rand() / (RAND_MAX + 1) * (2));
   dx = 0.0f;
   dy = 0.0f;
   dz = 0.01f*((float)rand() / (RAND_MAX + 1) * (2));
   dr = 0.001f*((float)rand() / (RAND_MAX + 1) * (2));
}

CZazaka::~CZazaka( void )
{
}

bool CZazaka::Update( float DeltaTime, CScene* Scene )
{
   if ( !CUnit::Update( DeltaTime, Scene ) )
      return false;

   Scene->Push( &m_Color, sizeof( m_Color ) );
   Scene->Push( &m_Rotation, sizeof( VECTOR3F ) );
   Scene->Push( &m_Scale, sizeof( VECTOR3F ) );
   Scene->Push( &m_Position, sizeof( VECTOR3F ) );
   Scene->Push( &m_ZazakaModelRef, sizeof( m_ZazakaModelRef ) );
   Scene->Push( (VOID_PTR)&RENDER_STATIC_SPRITE, sizeof( RWORD ) );
   
   if ( m_LastTime + 5 < m_Time )
   {

      if ( m_Position.Y > 600.0f )
      {
         m_Position.Y = 600.0f;
         dy = -dy;
      }
      if ( m_Position.X > 800.0f )
      {
         m_Position.X = 800.0f;
         dx = -dx;
      }
      if ( m_Position.Y < 0.0f )
      {
         m_Position.Y = 0.0f;
         dy = -dy;
      }
      if ( m_Position.X < 0.0f )
      {
         m_Position.X = 0.0f;
         dx = -dx;
      }

     /* if ( m_Scale.X > 128.0f )
      {
         m_Scale.X = 128.0f;
         m_Scale.Y = 128.0f;
         dz = -dz;
      }

      if ( m_Scale.X < 32.0f )
      {
         m_Scale.X = 32.0f;
         m_Scale.Y = 32.0f;

         dz = -dz;
      }*/

      if ( m_Rotation.Z > 4*3.14f )
      {
         m_Rotation.Z = 4*3.14f;
         dr = -dr;
      }

      if ( m_Rotation.Z < 0.0f )
      {
         m_Rotation.Z = 0.0f;
         dr = -dr;
      }

      m_Position.X += dx*(m_Time - m_LastTime);
      m_Position.Y += dy*(m_Time - m_LastTime);
      //m_Scale.X += dz*(m_Time - m_LastTime);
      //m_Scale.Y += dz*(m_Time - m_LastTime);
      m_Rotation.Z += dr*(m_Time - m_LastTime);
   
      m_LastTime = m_Time;
   }

   //Scene->AddElement( m_ZazakaModelRef, m_MutatorTranslate2D, m_Left, m_Top, 1.0f, 0.0f );

   return true;
}

void CZazaka::ClearResourceRefs( void )
{

}

bool CZazaka::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}
}