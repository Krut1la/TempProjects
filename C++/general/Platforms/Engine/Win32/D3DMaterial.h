#ifndef D3DMATERIAL_H
#define D3DMATERIAL_H

#include "Material.h"
#ifdef USE_DX9
#include <d3d9.h>
#else
#include <d3d8.h>
#endif


namespace RTS
{
class CD3DMaterial: public CMaterial
{
private:
   
protected:
#ifdef USE_DX9
   IDirect3DDevice9*   m_D3DDeviceRef;
#else
   IDirect3DDevice8*   m_D3DDeviceRef;
#endif
   
   RWORD            m_TextureIndex;

public:
#ifdef USE_DX9
   CD3DMaterial( IDirect3DDevice9* D3DDevice, void* Resources );
#else
   CD3DMaterial( IDirect3DDevice8* D3DDevice, void* Resources );
#endif
   

   virtual ~CD3DMaterial( void );

   void SetTextureIndex( RWORD TextureIndex ) { m_TextureIndex = TextureIndex; }

   virtual bool Set( void );
   virtual bool Reset( void );
};
}

#endif // D3DMATERIAL_H