#include "3DUtils.h"
#include "Control.h"
#include "Engine.h"
#include "GUI.h"
#include "SysFactory.h"

namespace RTS
{
CControl::CControl( void* GUI )
{
   m_Time = 0.0f;

   m_GUIRef      = GUI;

   m_Position   = VECTOR3F( 0.0f, 0.0f, 0.0f );
   m_Scale      = VECTOR3F( 0.0f, 0.0f, 0.0f );
   m_Rotation   = VECTOR3F( 0.0f, 0.0f, 0.0f );

   m_Visible      = true;
   m_Enabled      = true;
   m_HasFocus      = false;
   m_CanHaveFocus   = true;

   m_Text         = L"Супер классные шрифты!!!!!!111";
   m_Vertical      = false;
   m_TextChanged   = true;
   m_FontRef      = ((CEngine*)((CGUI*)m_GUIRef)->GetEngineRef())->GetResources()->GetFontRef( 0 );

   m_MutatorTranslate2D = (CMutatorTranslate2D*)g_SysFactory->CreateObject(L"D3DMutatorTranslate2D", ((CEngine*)((CGUI*)m_GUIRef)->GetEngineRef()));

}

CControl::~CControl( void )
{
   delete m_MutatorTranslate2D;

   for ( CONTROLLIST::iterator iSubControl = m_SubControls.begin();
      iSubControl != m_SubControls.end();
      ++iSubControl )
   {
      delete *iSubControl;
   }

   m_SubControls.clear();
}

void CControl::SetText( const std::wstring& Text)
{
   for ( SPRITELIST::iterator iCharSprite = m_TextSprites.begin(); 
      iCharSprite != m_TextSprites.end(); 
      ++iCharSprite )
   {
      delete (*iCharSprite);
   }

   m_TextSprites.clear();
   m_TextChanged = true;
   m_Text = Text;
}

bool CControl::UpdateText( CScene* Scene )
{
/*   INT32 OffsetX = 0;
   INT32 OffsetY = 0;

   UINT32 iCharSprite = 0;

   for ( std::wstring::iterator iChar = m_Text.begin(); iChar != m_Text.end(); ++iChar )
   {
      CSprite* CharSprite;

      RASTER_CHAR RasterChar = m_FontRef->GetRasterChar( *iChar );

      if ( m_TextChanged )
      {
         CharSprite = new CSprite( NULL );

         CharSprite->m_Vetrices[0].U = (float)(RasterChar.Rect.Left)/(float)(m_FontRef->GetRaster().Width);
         CharSprite->m_Vetrices[0].V = (float)(RasterChar.Rect.Top)/(float)(m_FontRef->GetRaster().Height);
         CharSprite->m_Vetrices[1].U = (float)((RasterChar.Rect.Right + 1))/(float)(m_FontRef->GetRaster().Width);
         CharSprite->m_Vetrices[1].V = (float)(RasterChar.Rect.Top)/(float)(m_FontRef->GetRaster().Height);
         CharSprite->m_Vetrices[2].U = (float)((RasterChar.Rect.Right + 1))/(float)(m_FontRef->GetRaster().Width);
         CharSprite->m_Vetrices[2].V = (float)((RasterChar.Rect.Bottom + 1))/(float)(m_FontRef->GetRaster().Height);
         CharSprite->m_Vetrices[3].U = (float)(RasterChar.Rect.Left)/(float)(m_FontRef->GetRaster().Width);
         CharSprite->m_Vetrices[3].V = (float)((RasterChar.Rect.Bottom + 1))/(float)(m_FontRef->GetRaster().Height);

         CharSprite->m_MaterialIndex = m_FontRef->GetTextureID();

         m_TextSprites.push_back( CharSprite );
      }
      else
         CharSprite = m_TextSprites[iCharSprite];

      if ( m_Vertical )
      {
         OffsetY += RasterChar.VertOffsetY;
         OffsetX = -RasterChar.VertOffsetX;
      }
      else
      {
         OffsetY = RasterChar.HoriOffsetY;
         OffsetX -= RasterChar.HoriOffsetX;
      }

      SCENEELEMENT SceneElement;

      SceneElement.m_StaticRenderableRef = 0;
      SceneElement.m_DynamicRenderableRef = CharSprite;
      SceneElement.m_RenderableType = rtDynamicSprite;
      SceneElement.m_MutatorRef = m_MutatorTranslate2D;
      SceneElement.m_Position = VECTOR3F( m_Position.X + OffsetX, 
         m_Position.Y + OffsetY, 
         m_Position.Z );
      SceneElement.m_Scale = VECTOR3F( (float)RasterChar.Rect.Width(), 
         (float)RasterChar.Rect.Height(), 
         1.0f );
      SceneElement.m_Rotation = m_Rotation;
      SceneElement.m_Color = 0xFFFFFFFF;

      Scene->AddElement( SceneElement );

      if ( m_Vertical )
         OffsetY += RasterChar.Rect.Height();
      else
         OffsetX += RasterChar.Rect.Width();

      iCharSprite++;
   }

   m_TextChanged = false;
*/
   return true;
}

bool CControl::Update( float DeltaTime, CScene* Scene )
{
   m_Time += DeltaTime;

   UpdateText( Scene );

   for ( CONTROLLIST::iterator iSubControl = m_SubControls.begin();
      iSubControl != m_SubControls.end();
      ++iSubControl )
   {
      if ( !(*iSubControl)->Update( DeltaTime, Scene ) )
         return false;
   }

   return true;
}

void CControl::AddControl( CControl* Control )
{
   m_SubControls.push_back( Control );
}

void CControl::ClearResourceRefs( void )
{

}

bool CControl::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}

}