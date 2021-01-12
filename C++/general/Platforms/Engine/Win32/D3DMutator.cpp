#include "D3DMutator.h"
//#include "D3DRenderer.h"
#include "D3DModel.h"
#include "3DUtils.h"

namespace RTS
{
CD3DMutatorTranslate2D::CD3DMutatorTranslate2D( void )
{

}

CD3DMutatorTranslate2D::~CD3DMutatorTranslate2D( void )
{

}

bool CD3DMutatorTranslate2D::Mutate( const SCENEELEMENT& SceneElement,
                              IRenderable* Renderable, 
                              CRenderer* Renderer  )
{

/*   CD3DSprite* Sprite = (CD3DSprite*)Renderable;

   Sprite->m_Vetrices[0].P = CreateVector4f( SceneElement.m_Position );
   Sprite->m_Vetrices[0].C = 0;
   Sprite->m_Vetrices[0].U = 1.0f;
   Sprite->m_Vetrices[0].U = 1.0f;

   m_SavedX = (m_X != NULL)?*m_X:0.0f;
   m_SavedY = (m_Y != NULL)?*m_Y:0.0f;
   m_SavedScale = (m_Scale != NULL)?*m_Scale:1.0f;
   m_SavedRotation = (m_Rotation != NULL)?*m_Rotation:0.0f;

   D3DXMATRIX   World;
   D3DXMATRIX   Translate;
   D3DXMATRIX   Scale;
   D3DXMATRIX   Rotation;

   ((CD3DRenderer*)Renderer)->GetDevice()->GetTransform( D3DTS_WORLD, &World );

    m_OldWorld = World;

   float ScaleX = (float)m_TargetWidth/(float)m_DeviceWidth;
   float ScaleY = (float)m_TargetHeight/(float)m_DeviceHeight;
   float TransX = -1.0f + 2.0f*(float)m_SavedX/(float)m_DeviceWidth + ScaleX;
   float TransY = -1.0f + 2.0f*(float)m_SavedY/(float)m_DeviceHeight + ScaleY;

    D3DXMatrixTranslation( &Translate, TransX, -TransY, 0.0f );
   D3DXMatrixScaling( &Scale, ScaleX*(1.0f + 0.005f*m_SavedScale), ScaleY*(1.0f + 0.005f*m_SavedScale), 1.0f );
   D3DXMatrixRotationZ( &Rotation, 0.05f*(float)m_SavedRotation );
    D3DXMatrixMultiply( &World, &World, &Rotation);
    D3DXMatrixMultiply( &World, &World, &Scale);
    D3DXMatrixMultiply( &World, &World, &Translate);

*/
    //((CD3DRenderer*)Renderer)->GetDevice()->SetTransform( D3DTS_WORLD, &World );


//   Model->Translate( (float)(m_SavedX), (float)(m_SavedY), 0.0f );

   return true;
}

bool CD3DMutatorTranslate2D::Mutate( CMutator* Mutator, CRenderer* Renderer  )
{
   return true;
}

bool CD3DMutatorTranslate2D::DeMutate( const SCENEELEMENT& SceneElement,
                               IRenderable* Model, 
                               CRenderer* Renderer  )
{
//    ((CD3DRenderer*)Renderer)->GetDevice()->SetTransform( D3DTS_WORLD, &m_OldWorld );

//   Model->Translate( -(float)(m_SavedX), -(float)(m_SavedY), -0.0f );

   return true;
}
}