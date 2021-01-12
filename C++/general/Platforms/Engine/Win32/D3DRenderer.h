#ifndef D3DRENDERER_H
#define D3DRENDERER_H

#include <windows.h>
#include <assert.h>

#ifdef USE_DX9
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <d3d9.h>
#include <d3dx9.h>

#else
#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")

#include <d3d8.h>
#include <d3dx8.h>
#endif

#include <string>
#include "Renderer.h"

namespace RTS
{
const DWORD D3DFVF_VERTEX_SPRITE_T     = ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 );
const DWORD D3DFVF_VERTEX_SPRITE       = ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 );
const DWORD D3DFVF_VERTEX_MODEL        = ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );

class CD3DRenderer: public CRenderer{
private:
   ID3DXFont*         m_D3DXFont;
protected:
   // Objects
   HWND               m_D3DWindow;

#ifdef USE_DX9
   IDirect3D9*                m_D3D;
   IDirect3DDevice9*          m_D3DDevice;
   IDirect3DVertexBuffer9*    m_SpriteVB;
   IDirect3DIndexBuffer9*     m_SpriteIB;

   IDirect3DVertexBuffer9*    m_ModelVB;
   IDirect3DIndexBuffer9*     m_ModelIB;
#else
   IDirect3D8*                m_D3D;
   IDirect3DDevice8*          m_D3DDevice;
   IDirect3DVertexBuffer8*    m_SpriteVB;
   IDirect3DIndexBuffer8*     m_SpriteIB;
#endif

   D3DDISPLAYMODE             m_D3DDisplayMode;
   D3DPRESENT_PARAMETERS      m_D3DParams;

   // Settings
   D3DCOLOR                   m_BackgroundColor;
   RWORD                      m_MaxSpriteVertices;
   RWORD                      m_MaxModelVertices;

   // State
   DWORD                      m_CurrentVertexType;
   VERTEX_SPRITE*             m_SpriteVBData;
   DWORD*                     m_SpriteIBData;
   VERTEX_MODEL*              m_ModelVBData;
   DWORD*                     m_ModelIBData;
   RWORD                      m_LastSpriteVertexIndex;
   RWORD                      m_LastModelVertexIndex;
   RWORD                      m_LastModelIndexIndex;

   HRESULT InitializeD3D( void );
   HRESULT FinalizeD3D( void );
   HRESULT CreateBuffers( void );
   HRESULT SetState( RENDER_STATE_SET StateSet );
   
   HRESULT ResetDevice( void );
   HRESULT RenderBatch( void );

   HRESULT UnlockSpriteBuffers( void );
   HRESULT LockSpriteBuffers( void );
   HRESULT UnlockModelBuffers( void );
   HRESULT LockModelBuffers( void );

   virtual bool PreRenderScene( void );
   virtual bool RenderScene( void );
   virtual bool PostRenderScene( void );

   virtual bool ClearLights( void );
public:

   CD3DRenderer( CEngine* Engine );
   virtual ~CD3DRenderer( void );

   void* GetEngine( void ) { return m_Engine; }
#ifdef USE_DX9
   IDirect3DDevice9* GetDevice( void ) { return m_D3DDevice; }
#else
   IDirect3DDevice8* GetDevice( void ) { return m_D3DDevice; }
#endif

   virtual bool Initialize( void );
   virtual void Finalize( void );

   virtual bool Start( void );

   void ResetD3DDevice( void );

   virtual bool RenderModel( CModel* Model, 
                        const VECTOR3F& Position, 
                        const VECTOR3F& Scale, 
                        const VECTOR3F& Rotation );

   virtual bool RenderSprite( CSprite* Sprite, 
                        const VECTOR3F& Position, 
                        const VECTOR3F& Scale, 
                        const VECTOR3F& Rotation,
                        COLOR Color );

   virtual bool RenderLight( LIGHT* Light );

   virtual bool SetTransform( TRANSFORM* Transform );
};
}

#endif // D3DRENDERER_H