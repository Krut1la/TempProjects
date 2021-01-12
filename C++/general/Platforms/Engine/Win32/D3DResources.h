#ifndef D3DRESOURCES_H
#define D3DRESOURCES_H

#include "Resources.h"

#ifdef USE_DX9
#include <d3d9.h>
#include <d3dx9.h>
#else
#include <d3d8.h>
#include <d3dx8.h>
#endif

namespace RTS
{

typedef std::vector<IDirect3DTexture9*> D3DTEXTURELIST;

class CD3DResources: public CResources
{
private:
protected:

   IDirect3DDevice9*   m_D3DDeviceRef;

   D3DTEXTURELIST      m_Textures;

public:

   CD3DResources( void );
   
   virtual ~CD3DResources( void );

   void SetDevice( IDirect3DDevice9* D3DDevice );

   virtual bool CreateTexture( RWORD Width, 
                               RWORD Height,
                               TEX_FORMAT Format,
                               BYTE* Buff,
                               RWORD* TextureIndex );
   virtual bool LoadTexture( const std::wstring& FileName, RWORD* TextureIndex );
   virtual bool LoadMaterial( const std::wstring& FileName, RWORD* MaterialIndex );

   IDirect3DTexture9* GetTextrureRef( RWORD TextureIndex );
   
};
}

#endif // D3DRESOURCES_H
