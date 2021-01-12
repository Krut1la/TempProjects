#include "D3DResources.h"
#include "D3DMaterial.h"


namespace RTS
{
CD3DResources::CD3DResources( void )
{

}

CD3DResources::~CD3DResources( void )
{

}

void CD3DResources::SetDevice( IDirect3DDevice9* D3DDevice )
{
   m_D3DDeviceRef = D3DDevice;
}

bool CD3DResources::LoadMaterial( const std::wstring& FileName, RWORD* MaterialIndex )
{
   if ( !CResources::LoadMaterial( FileName, MaterialIndex ) )
      return false;

   RWORD TextureId = 0;

   if ( FileName == L"empty" )
   {
      CD3DMaterial* Material = new CD3DMaterial( m_D3DDeviceRef, this );

      Material->SetTextureIndex( 0 );

      LoadTexture( FileName, NULL );

      *MaterialIndex = m_Materials.size();

      m_Materials.push_back( Material );

      return true;
   }

   if ( !LoadTexture( FileName, &TextureId ) )
      return false;

   CD3DMaterial* Material = new CD3DMaterial( m_D3DDeviceRef, this );

   Material->SetTextureIndex( TextureId );

   *MaterialIndex = m_Materials.size();

   m_Materials.push_back( Material );

   return true;
}

bool CD3DResources::CreateTexture( RWORD Width, 
                             RWORD Height,
                             TEX_FORMAT Format,
                             BYTE* Buff,
                             RWORD* TextureIndex )
{
   if ( TextureIndex == NULL )
   {
      m_Textures.push_back( NULL );

      return true;
   }

   if ( !CResources::CreateTexture( Width, Height, Format, Buff, TextureIndex ) )
      return false;

   if ( m_D3DDeviceRef == NULL )
      return false;

#ifdef USE_DX9
   IDirect3DTexture9* Texture;
#else
   IDirect3DTexture8* Texture;
#endif

   HRESULT hResult;

   D3DFORMAT D3DFormat;

   switch ( Format )
   {
   case tfGrayScale:
      D3DFormat = D3DFMT_L8;
      break;
   case tfARGB:
      D3DFormat = D3DFMT_A8R8G8B8;
      break;
   default:
      throw;
   }

#ifdef USE_DX9
   if ( FAILED( hResult = m_D3DDeviceRef->CreateTexture( (UINT)Width, (UINT)Height, 0, 0, D3DFormat, D3DPOOL_MANAGED, &Texture, NULL ) ) )
#else
   if ( FAILED( hResult = m_D3DDeviceRef->CreateTexture( (UINT)Width, (UINT)Height, 0, 0, D3DFormat, D3DPOOL_MANAGED, &Texture) ) )
#endif
   {
      return false;
   }

   D3DLOCKED_RECT  d3dlrect;

   if ( SUCCEEDED( Texture->LockRect( 0, &d3dlrect, NULL, 0 )))
   {
      switch ( Format )
      {
      case tfARGB:
         {
            if (d3dlrect.Pitch == Width*(32 >> 3))
               memcpy( d3dlrect.pBits, Buff, Width*Height*(32 >> 3) );
            else
               for ( UINT32 iLine = 0; iLine < Height; ++iLine )
                  memcpy( (char*)d3dlrect.pBits + iLine*d3dlrect.Pitch, Buff + iLine*Width*(32 >> 3), Width*(32 >> 3) );

            break;
         }
      case tfGrayScale:
         {
            memcpy( d3dlrect.pBits, Buff, Width*Height*(8 >> 3) );
            break;
         }
      }
      Texture->UnlockRect(0);
   }
   else
      return false;

   delete [] Buff;

   *TextureIndex = m_Textures.size();

   m_Textures.push_back( Texture );

   return true;
}

bool CD3DResources::LoadTexture( const std::wstring& FileName, RWORD* TextureIndex )
{

   if ( TextureIndex == NULL )
   {
      m_Textures.push_back( NULL );

      return true;
   }

   if ( !CResources::LoadTexture( FileName, TextureIndex ) )
      return false;

   if ( m_D3DDeviceRef == NULL )
      return false;

   BYTE* Buff = NULL;
   RWORD Width = 0;
   RWORD Height = 0;
   RWORD BPP = 0;

   if ( !LoadImageF( FileName, &Buff, &Width, &Height, &BPP ) )
      return false;


   return CreateTexture( Width, Height, tfARGB, Buff, TextureIndex );
}

#ifdef USE_DX9
IDirect3DTexture9* CD3DResources::GetTextrureRef( RWORD TextureIndex )
#else
IDirect3DTexture8* CD3DResources::GetTextrureRef( RWORD TextureIndex )
#endif
{ 
   // Return dummy texture
   if ( TextureIndex + 1 > m_Textures.size() )
      return m_Textures[0];

   return m_Textures[TextureIndex]; 
}

}