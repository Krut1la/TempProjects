#include "3DUtils.h"
#include "Image.h"
#include "Resources.h"
#include "GUI.h"
#include "Engine.h"

namespace RTS
{
CImage::CImage( void* GUI, UINT32 MaterialRef ):
CControl( GUI )
{
   m_ImageModelRef = 1;
   m_Scale = VECTOR3F(   ((CGUI*)m_GUIRef)->GetScreenWidth(), 
                        ((CGUI*)m_GUIRef)->GetScreenHeight(), 
                        1.0f );
   

//   m_MutatorTranslate2D->SetTargetWidth( m_Width );
//   m_MutatorTranslate2D->SetTargetHeight( m_Height );
}

CImage::~CImage( void )
{
}

bool CImage::Update( float DeltaTime, CScene* Scene )
{
/*   SCENEELEMENT SceneElement;

   SceneElement.m_StaticRenderableRef = m_ImageModelRef;
   SceneElement.m_DynamicRenderableRef = NULL;
   SceneElement.m_RenderableType = taticSprite;
   SceneElement.m_MutatorRef = m_MutatorTranslate2D;
   SceneElement.m_Position = m_Position;
   SceneElement.m_Scale = m_Scale;
   SceneElement.m_Rotation = m_Rotation;
   SceneElement.m_Color = 0xFFFFFFFF;

   Scene->AddElement( SceneElement );
*/
   return true;
}

void CImage::ClearResourceRefs( void )
{

}

bool CImage::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}
}