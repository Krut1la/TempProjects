#include "BasketBall.h"
#include "Resources.h"
#include "World.h"
#include "Engine.h"
#include "3DUtils.h"

namespace RTS
{
CBasketBall::CBasketBall( void* World ):
CUnit( World )
{
   m_LastTime = 0;

   m_BasketBallModelRef = 3;

   m_Position = VECTOR3F( 0.0f, 0.0f, 0.0f );
   m_Scale = VECTOR3F( 1.0f, 1.0f, 1.0f );
   m_Rotation = VECTOR3F( 0.0f, 0.0f, 0.0f );

   dr = 0.001f*((float)rand() / (RAND_MAX + 1) * (2));

   m_Speed = VECTOR3F( 0.005f*((float)rand() / (RAND_MAX + 1) * (2)),
                       0.005f*((float)rand() / (RAND_MAX + 1) * (2)),
                       0.005f*((float)rand() / (RAND_MAX + 1) * (2)) );

   m_Gravitation = VECTOR3F( 0.0f, -0.003f, 0.0f );
}

CBasketBall::~CBasketBall( void )
{
}

bool CBasketBall::Update( float DeltaTime, CScene* Scene )
{
   if ( !CUnit::Update( DeltaTime, Scene ) )
      return false;

/*   SCENEELEMENT SceneElement;

   SceneElement.m_StaticRenderableRef = m_BasketBallModelRef;
   SceneElement.m_DynamicRenderableRef = NULL;
   SceneElement.m_RenderableType = taticModel;
   SceneElement.m_MutatorRef = NULL;
   SceneElement.m_Position = m_Position;
   SceneElement.m_Scale = m_Scale;
   SceneElement.m_Rotation = m_Rotation;
   SceneElement.m_Color = 0xFFFFFFFF;

   Scene->AddElement( SceneElement );
*/
   Scene->Push( &m_Rotation, sizeof( VECTOR3F ) );
   Scene->Push( &m_Scale, sizeof( VECTOR3F ) );
   Scene->Push( &m_Position, sizeof( VECTOR3F ) );
   Scene->Push( &m_BasketBallModelRef, sizeof( m_BasketBallModelRef ) );
   Scene->Push( (VOID_PTR)&RENDER_STATIC_MODEL, sizeof( WORD ) );

   if ( m_LastTime + 5 < m_Time )
   {

      if ( m_Position.Y > 1.0f )
      {
         m_Position.Y = 1.0f;
         m_Speed.Y = -m_Speed.Y;
         m_Speed = 0.9f*m_Speed;
      }
      if ( m_Position.X > 1.0f )
      {
         m_Position.X = 1.0f;
         m_Speed.X = -m_Speed.X;
         m_Speed = 0.9f*m_Speed;
      }
      if ( m_Position.Y < -1.0f )
      {
         m_Position.Y = -1.0f;
         m_Speed.Y = -m_Speed.Y;
         m_Speed = 0.9f*m_Speed;
      }
      if ( m_Position.X < -1.0f )
      {
         m_Position.X = -1.0f;
         m_Speed.X = -m_Speed.X;
         m_Speed = 0.9f*m_Speed;
      }

      if ( m_Position.Z > 2.0f )
      {
         m_Position.Z = 2.0f;
         m_Speed.Z = -m_Speed.Z;
         m_Speed = 0.9f*m_Speed;
      }
      if ( m_Position.Z < 0.0f )
      {
         m_Position.Z = 0.0f;
         m_Speed.Z = -m_Speed.Z;
         m_Speed = 0.9f*m_Speed;
      }

   /*   if ( m_Scale.X > 5.0f )
      {
         m_Scale.X = 5.0f;
         m_Scale.Y = 5.0f;
         dz = -dz;
      }

      if ( m_Scale.X < 0.5f )
      {
         m_Scale.X = 0.5f;
         m_Scale.Y = 0.5f;

         dz = -dz;
      }

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
*/

      m_Speed = m_Speed + m_Gravitation;

      m_Position = m_Position + m_Speed;



//      m_Scale.X += dz*(m_Time - m_LastTime);
//      m_Scale.Y += dz*(m_Time - m_LastTime);
//      m_Rotation.Z += dr*(m_Time - m_LastTime);
   
      m_LastTime = m_Time;
   }

   return true;
}

void CBasketBall::ClearResourceRefs( void )
{

}

bool CBasketBall::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}
}