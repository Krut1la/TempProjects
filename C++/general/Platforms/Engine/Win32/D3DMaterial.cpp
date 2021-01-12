#include "D3DMaterial.h"
#include "D3DResources.h"

namespace RTS
{
#ifdef USE_DX9
CD3DMaterial::CD3DMaterial( IDirect3DDevice9* D3DDevice,  void* Resources ):
#else
CD3DMaterial::CD3DMaterial( IDirect3DDevice8* D3DDevice,  void* Resources ):
#endif

CMaterial( Resources ),
m_D3DDeviceRef( D3DDevice )
{
}

CD3DMaterial::~CD3DMaterial( void )
{
}

bool CD3DMaterial::Set( void )
{
   if ( !CMaterial::Set() )
      return false;
   
   m_D3DDeviceRef->SetTexture(0, ((CD3DResources*)m_ResourcesRef)->GetTextrureRef( m_TextureIndex ) );

   return true;
}

bool CD3DMaterial::Reset( void )
{
   if ( !CMaterial::Reset() )
      return false;

   return true;
}
}