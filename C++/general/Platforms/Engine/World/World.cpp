#include "World.h"
#include "3DUtils.h"
#include "MathUtils.h"
#include "DateTimeUtils.h"

#include <d3dx9.h>

namespace RTS
{
CWorld::CWorld( void )
{
   m_Time = 0.0f;
   m_WorldTransform.Type = ttWorld;
   m_ViewTransform.Type = ttView;
   m_ProjTransform.Type = ttProj;

   m_WorldTransform.Matrix = MATRIX4F::IDENTITY;
   m_ProjTransform.Matrix = MATRIX4F::IDENTITY;
   m_ViewTransform.Matrix = MATRIX4F::IDENTITY;

//   VECTOR3F vEyePt( 1.5f, 0.5f, -2.5f );
   //VECTOR3F vLookatPt( 0.0f, 0.0f, 0.0f );
   //VECTOR3F vUpVec( 0.0f, 1.0f, 0.0f );

   D3DXVECTOR3 vEyePt( 1.5f, 0.5f, -2.5f );
   D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
   D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );

   D3DXMATRIXA16 matView;

   D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );

   memcpy( &m_ViewTransform.Matrix.m[0][0], &matView.m[0][0], sizeof(float)*16);

   D3DXMATRIXA16 matProj;
   D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.3333f, 1.0f, 100.0f );

   memcpy( &m_ProjTransform.Matrix.m[0][0], &matProj.m[0][0], sizeof(float)*16);

//   MATRIX4F::CreateProjectionMatrix( (RADIAN)M_PI / 4, 1.3333f, 1.0f, 100.0f, m_ProjTransform.Matrix, false ); 

    memset( &m_MainLight, 0, sizeof(LIGHT) );
   m_MainLight.Diffuse.R = 1.0f;
    m_MainLight.Diffuse.G = 1.0f;
    m_MainLight.Diffuse.B = 1.0f;
    m_MainLight.Diffuse.A = 1.0f;
   m_MainLight.Range = 1000.0f;
   m_MainLight.Type = ltDirectional;

   m_MainLight.Direction = VECTOR3F(cosf(GetTicks()/350.0f),
                         1.0f,
                         sinf(GetTicks()/350.0f) );
}

CWorld::~CWorld( void )
{
   for ( UNITLIST::iterator iUnit = m_Units.begin();
   iUnit != m_Units.end();
   ++iUnit )
   {
      delete *iUnit;
   }

   m_Units.clear();
}

void CWorld::UpdateTransform( CScene* Scene )
{
   UINT32 iTime = GetTicks() % 10000;
   float fAngle = (float)(iTime * ( 2.0f * M_PI ) / 10000.0f);
//   D3DXMatrixRotationY( &matWorld, fAngle );

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

void CWorld::UpdateLights( CScene* Scene )
{
/*   SCENEELEMENT SceneElement;

   SceneElement.m_StaticRenderableRef = 0;
   SceneElement.m_DynamicRenderableRef = &m_MainLight;
   SceneElement.m_RenderableType = rtDynamicLight;
   SceneElement.m_MutatorRef = NULL;
   SceneElement.m_Color = 0;

   Scene->AddElement( SceneElement );

*/
}

bool CWorld::Update( float DeltaTime, CScene* Scene )
{
   UpdateTransform( Scene );

   UpdateLights( Scene );

   for( UNITLIST::iterator iUnit = m_Units.begin();  iUnit != m_Units.end(); ++iUnit)
   {
      if ( !(*iUnit)->Update( DeltaTime, Scene ) )
         return false;
   }

   return true;
}

bool CWorld::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}

void CWorld::AddUnit( CUnit* Unit )
{
   m_Units.push_back( Unit );
}
}