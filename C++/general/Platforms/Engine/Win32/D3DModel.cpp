#include "D3DModel.h"
#include "D3DResources.h"
#include "D3DMaterial.h"

namespace RTS
{
CD3DModel::CD3DModel( void* Resources ):
CModel( Resources )
{
}

CD3DModel::~CD3DModel( void )
{
}

bool CD3DModel::Render( CRenderer* Renderer )
{
   if ( Renderer == NULL)
      return false;

   if ( !CModel::Render( Renderer ) )
      return false;

   return true;
}
}