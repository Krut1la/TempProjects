#include "3DUtils.h"
#include "Cursor.h"
#include "Resources.h"
#include "GUI.h"
#include "Engine.h"

namespace RTS
{
CCursor::CCursor( void* GUI ):
CControl( GUI )
{
   m_CursorModelRef = 0;

   m_Scale = VECTOR3F( 32.0f, 32.0f, 1.0f );
   m_Color = 0xFFFFFFFF;

//   m_MutatorTranslate2D->SetTargetWidth( m_Width );
//   m_MutatorTranslate2D->SetTargetHeight( m_Height );
}

CCursor::~CCursor( void )
{
}

bool CCursor::Update( float DeltaTime, CScene* Scene )
{
   CControl::Update( DeltaTime, Scene );

//   SCENEELEMENT SceneElement;

   Scene->Push( &m_Color, sizeof( m_Color ) );
   Scene->Push( &m_Rotation, sizeof( VECTOR3F ) );
   Scene->Push( &m_Scale, sizeof( VECTOR3F ) );
   Scene->Push( &m_Position, sizeof( VECTOR3F ) );
   Scene->Push( &m_CursorModelRef, sizeof( m_CursorModelRef ) );
   Scene->Push( (VOID_PTR)&RENDER_STATIC_SPRITE, sizeof( RWORD ) );

//   SceneElement.m_StaticRenderableRef = m_CursorModelRef;
//   SceneElement.m_DynamicRenderableRef = NULL;
//   SceneElement.m_RenderableType = taticSprite;
//   SceneElement.m_MutatorRef = m_MutatorTranslate2D;
//   SceneElement.m_Position = m_Position;
//   SceneElement.m_Scale = m_Scale;
//   SceneElement.m_Rotation = m_Rotation;
//   SceneElement.m_Color = 0xFFFFFFFF;

//   Scene->AddElement( SceneElement );

   return true;
}

void CCursor::ClearResourceRefs( void )
{
//   m_CursorModelRef = NULL;
}

bool CCursor::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   switch ( EventID )
   {
      case MOUSE_MOVE:
         {
            m_Position.X += wParam;
            m_Position.Y += lParam;

            if ( m_Position.X < 0) m_Position.X = 0;
            if ( m_Position.Y < 0) m_Position.Y = 0;
            if ( m_Position.X > ((CGUI*)m_GUIRef)->GetScreenWidth() ) m_Position.X = ((CGUI*)m_GUIRef)->GetScreenWidth();
            if ( m_Position.Y > ((CGUI*)m_GUIRef)->GetScreenHeight() ) m_Position.Y = ((CGUI*)m_GUIRef)->GetScreenHeight();

            break;
         }
   }

   return true;
}
}