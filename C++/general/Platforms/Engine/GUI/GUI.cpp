#include "GUI.h"

namespace RTS
{
CGUI::CGUI( void ):
m_ScreenWidth( 0 ),
m_ScreenHeight( 0 )
{
   m_WorldTransform.Type = ttWorld;
   m_ViewTransform.Type = ttView;
   m_ProjTransform.Type = ttProj;

   m_WorldTransform.Matrix = MATRIX4F::IDENTITY;
   m_ProjTransform.Matrix = MATRIX4F::IDENTITY;
   m_ViewTransform.Matrix = MATRIX4F::IDENTITY;
}

CGUI::~CGUI( void )
{
   for ( CONTROLLIST::iterator iControl = m_Controls.begin();
   iControl != m_Controls.end();
   ++iControl )
   {
      delete *iControl;
   }

   m_Controls.clear();
}

void CGUI::UpdateTransform( CScene* Scene )
{
/*   SCENEELEMENT SceneElement;

   SceneElement.m_StaticRenderableRef = 0;
   SceneElement.m_DynamicRenderableRef = &m_WorldTransform;
   SceneElement.m_RenderableType = rtTransform;
   SceneElement.m_MutatorRef = NULL;
   SceneElement.m_Color = 0;

   Scene->AddElement( SceneElement );

   SceneElement.m_DynamicRenderableRef = &m_ViewTransform;
   Scene->AddElement( SceneElement );

   SceneElement.m_DynamicRenderableRef = &m_ProjTransform;
   Scene->AddElement( SceneElement );
*/
}

bool CGUI::Update( float DeltaTime, CScene* Scene )
{
   UpdateTransform( Scene );

   for ( CONTROLLIST::iterator iControl = m_Controls.begin();
   iControl != m_Controls.end();
   ++iControl )
   {
      if ( !(*iControl)->Update( DeltaTime, Scene ) )
         return false;
   }

   return true;
}

void CGUI::AddControl( CControl* Control )
{
   m_Controls.push_back( Control );
}

bool CGUI::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   /*switch ( EventID )
   {
      case MOUSE_MOVE:
         {
            break;
         }
   }*/

   for( CONTROLLIST::iterator iControl = m_Controls.begin();  iControl != m_Controls.end(); ++iControl)
   {
      if ( !(*iControl)->HandleInputEvent( EventID, wParam, lParam ) )
         return false;
   }

   return true;
}
}