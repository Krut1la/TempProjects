#include "D3DRenderer.h"
#include "DirectInput.h"
#include "Engine.h"
#include "StringUtils.h"
#include "SystemUtils.h"
#include "3DUtils.h"

namespace RTS
{

void __fastcall ResetDeviceFromMainThread(VOID_PTR Data)
{
   ((CD3DRenderer*)Data)->ResetD3DDevice();
   
}

CD3DRenderer::CD3DRenderer( CEngine* Engine ):
CRenderer( Engine )
{
   m_BackgroundColor         = clBlack;
   m_CurrentVertexType       = D3DFVF_VERTEX_SPRITE;
   m_LastSpriteVertexIndex   = 0;
   m_LastModelVertexIndex    = 0;
   m_LastModelIndexIndex     = 0;
   m_MaxSpriteVertices       = 40000;
   m_MaxModelVertices        = 100000;
}

CD3DRenderer::~CD3DRenderer( void )
{
}

bool CD3DRenderer::Initialize( void )
{   
   if ( !CRenderer::Initialize() ) 
      return false;

   m_Engine->GetSysLogger()->Log( L"Initialize D3DRenderer", CMessageMSG() );

   CMTService* InputService = m_Engine->FindService(L"Input");

   if ( InputService == NULL)
   {
      m_Engine->GetSysLogger()->Log( L"\tTrying to initialize Input before Render", CMessageERR() );
      return false;
   }

   InputService->DoCommand( MTC_GET_WINDOW_HANDLE, &m_D3DWindow, NULL );

   if ( m_D3DWindow == NULL)
   {
      m_Engine->GetSysLogger()->Log( L"\tBad Window handle", CMessageERR() );
      return false;
   }

   if ( InitializeD3D() != S_OK ){
      m_Engine->GetSysLogger()->Log( L"\tInitializeD3D failed", CMessageERR() );
      return false;
   }

#ifdef USE_DX9
   D3DXCreateFont(m_D3DDevice,
                   24,
                   0,
                   FW_NORMAL,
                   1,
                   false,
                   DEFAULT_CHARSET,
                   OUT_DEFAULT_PRECIS,
                   ANTIALIASED_QUALITY,
                   DEFAULT_PITCH | FF_DONTCARE,
                   L"Arial",
                   &m_D3DXFont);
#endif

   return true;
}

void CD3DRenderer::Finalize( void )
{
   m_Engine->GetSysLogger()->Log( L"Finalize D3DRenderer", CMessageMSG() );

   CRenderer::Finalize();

   if (FinalizeD3D() != S_OK){
      m_Engine->GetSysLogger()->Log( L"\tFinalizeD3D failed", CMessageERR() );
   }
}

HRESULT CD3DRenderer::InitializeD3D( void )
{
   m_Engine->GetSysLogger()->Log( L"Initialize D3D9", CMessageMSG() );

   HRESULT hResult;

#ifdef USE_DX9
   m_D3D = Direct3DCreate9( D3D_SDK_VERSION );
#else
   m_D3D = Direct3DCreate8( D3D_SDK_VERSION );
#endif

   if (m_D3D == NULL)
   {
      m_Engine->GetSysLogger()->Log( L"\tDirect3DCreate9 failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   m_D3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &m_D3DDisplayMode );

   ZeroMemory( &m_D3DParams, sizeof( m_D3DParams ) );
    m_D3DParams.Windowed          = !m_FullScreen;

   if ( m_FullScreen )
      m_D3DParams.SwapEffect = D3DSWAPEFFECT_FLIP;
   else
      m_D3DParams.SwapEffect = D3DSWAPEFFECT_COPY;

   m_D3DParams.BackBufferFormat = D3DFMT_UNKNOWN;

   if ( m_Width == 0)
   {
      m_D3DParams.BackBufferWidth = m_D3DDisplayMode.Width;
      m_Width    = m_D3DDisplayMode.Width;
   }
   else
      m_D3DParams.BackBufferWidth = (UINT)m_Width;

   if ( m_Height == 0)
   {
      m_D3DParams.BackBufferHeight = m_D3DDisplayMode.Height;
      m_Height = m_D3DDisplayMode.Height;
   }
   else
      m_D3DParams.BackBufferHeight = (UINT)m_Height;

   m_D3DParams.BackBufferCount     = 1;
   m_D3DParams.hDeviceWindow       = m_D3DWindow;

#ifdef USE_DX9
   m_D3DParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
#else
   
#endif

   m_D3DParams.EnableAutoDepthStencil = TRUE;
   m_D3DParams.AutoDepthStencilFormat = D3DFMT_D16;

	m_D3DParams.BackBufferFormat = m_D3DDisplayMode.Format;

    if( FAILED( hResult = m_D3D->CreateDevice( D3DADAPTER_DEFAULT, 
                            D3DDEVTYPE_HAL, 
                            m_D3DWindow,
                            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                            &m_D3DParams, &m_D3DDevice ) ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateDevice failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   if( FAILED( hResult = CreateBuffers() ) )
      return hResult;

   if( FAILED( hResult = SetState( RSS_DEFAULT ) ) )
      return hResult;
   
   return S_OK;
}

HRESULT CD3DRenderer::CreateBuffers( void )
{
   HRESULT hResult;

#ifdef USE_DX9
   if( FAILED( hResult = m_D3DDevice->CreateVertexBuffer((UINT)m_MaxSpriteVertices*sizeof(VERTEX_SPRITE),  
                                          D3DUSAGE_WRITEONLY, 
                                          D3DFVF_VERTEX_SPRITE, 
                                          D3DPOOL_DEFAULT, 
                                          &m_SpriteVB, 
                                          NULL ) ) )
#else
   if( FAILED( hResult = m_D3DDevice->CreateVertexBuffer( m_MaxSpriteVertices*sizeof(VERTEX_SPRITE),  
                                          D3DUSAGE_WRITEONLY, 
                                          D3DFVF_VERTEX_SPRITE, 
                                          D3DPOOL_DEFAULT, 
                                          &m_SpriteVB   ) ) )
#endif
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateVertexBuffer failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

#ifdef USE_DX9
   if( FAILED( hResult = m_D3DDevice->CreateIndexBuffer( (UINT)(m_MaxSpriteVertices>>1)*3*sizeof(WORD),  
                                          D3DUSAGE_WRITEONLY, 
                                          D3DFMT_INDEX16, 
                                          D3DPOOL_DEFAULT, 
                                          &m_SpriteIB, 
                                          NULL ) ) )
#else
   if( FAILED( hResult = m_D3DDevice->CreateIndexBuffer( (m_MaxSpriteVertices>>1)*3*sizeof(WORD),  
                                          D3DUSAGE_WRITEONLY, 
                                          D3DFMT_INDEX16, 
                                          D3DPOOL_DEFAULT, 
                                          &m_SpriteIB   ) ) )
#endif
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateVertexBuffer failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   WORD *pIndices, n = 0;
#ifdef USE_DX9
   if( FAILED( m_SpriteIB->Lock( 0, 0, (void**)&pIndices, 0 ) ) )
#else
   if( FAILED( m_SpriteIB->Lock( 0, 0, (BYTE**)&pIndices, 0 ) ) )
#endif
   {
      return E_FAIL;
   }

   for(UINT32 i = 0; i < (m_MaxSpriteVertices>>2); i++) {
      *pIndices++ = n;
      *pIndices++ = n + 1;
      *pIndices++ = n + 2;
      *pIndices++ = n + 2;
      *pIndices++ = n + 3;
      *pIndices++ = n;
      n += 4;
   }

   m_SpriteIB->Unlock();


#ifdef USE_DX9
   if( FAILED( hResult = m_D3DDevice->CreateVertexBuffer( (UINT)m_MaxModelVertices*sizeof(VERTEX_MODEL),  
                                          D3DUSAGE_WRITEONLY, 
                                          D3DFVF_VERTEX_MODEL, 
                                          D3DPOOL_DEFAULT, 
                                          &m_ModelVB, 
                                          NULL ) ) )
#else

#endif
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateVertexBuffer failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

#ifdef USE_DX9
   if( FAILED( hResult = m_D3DDevice->CreateIndexBuffer( (UINT)(m_MaxModelVertices>>1)*3*sizeof(DWORD),  
                                          D3DUSAGE_WRITEONLY, 
                                          D3DFMT_INDEX32, 
                                          D3DPOOL_DEFAULT, 
                                          &m_ModelIB, 
                                          NULL ) ) )
#else

#endif
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateVertexBuffer failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   return S_OK;
}

HRESULT CD3DRenderer::SetState( RENDER_STATE_SET StateSet )
{
//   HRESULT hResult;

   m_D3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
   m_D3DDevice->SetRenderState( D3DRS_LIGHTING, false );
   m_D3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

#ifdef USE_DX9
   m_D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
   m_D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
#else
   m_D3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
   m_D3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
#endif

   m_D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
   m_D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
   m_D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

   m_D3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
   m_D3DDevice->SetRenderState( D3DRS_ALPHAREF,        0x01 );
   m_D3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );

   m_D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
   m_D3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
   m_D3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

   m_D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
   m_D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
   m_D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

/*   D3DVIEWPORT9 vp;
   vp.X      = 0;
   vp.Y      = 0;
   vp.Width  = m_D3DParams.BackBufferWidth;
   vp.Height = m_D3DParams.BackBufferHeight;
   vp.MinZ   = 0.0f;
   vp.MaxZ   = 20.0f;

   hResult = m_D3DDevice->SetViewport( &vp );
*/
    D3DMATERIAL9 mtrl;
    ZeroMemory( &mtrl, sizeof( D3DMATERIAL9 ) );
    mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
    mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
    mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
    mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
    m_D3DDevice->SetMaterial( &mtrl );

#ifdef USE_DX9
   m_D3DDevice->SetFVF( D3DFVF_VERTEX_SPRITE );
#else
   m_D3DDevice->SetVertexShader( D3DFVF_VERTEX_SPRITE );
#endif

//    m_D3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

   return S_OK;
}

HRESULT CD3DRenderer::FinalizeD3D( void )
{
   m_Engine->GetSysLogger()->Log( L"Finalize D3D9", CMessageMSG() );

#ifdef USE_DX9
   SAFE_RELEASE(m_D3DXFont);
#else
#endif
   SAFE_RELEASE(m_ModelIB);
   SAFE_RELEASE(m_ModelVB);
   SAFE_RELEASE(m_SpriteIB);
   SAFE_RELEASE(m_SpriteVB);
   SAFE_RELEASE(m_D3DDevice);
   SAFE_RELEASE(m_D3D);

   return S_OK;
}


bool CD3DRenderer::Start( void )
{
   if ( !CRenderer::Start() )
      return true; 

   return true;
}

void CD3DRenderer::ResetD3DDevice( void )
{
   m_D3DDevice->Reset( &m_D3DParams );
}

HRESULT CD3DRenderer::ResetDevice( void )
{
   HRESULT hResult;

   if ( FAILED( hResult = m_D3DDevice->SetIndices( NULL ) ) )
      return hResult;

   if ( FAILED( hResult = m_D3DDevice->SetStreamSource( 0, NULL, 0, 0 ) ) )
      return hResult;

   SAFE_RELEASE(m_ModelIB);
   SAFE_RELEASE(m_ModelVB);
   SAFE_RELEASE(m_SpriteIB);
   SAFE_RELEASE(m_SpriteVB);   

   m_D3DXFont->OnLostDevice();
   m_D3DXFont->OnResetDevice();

   if (m_Engine->GetMultiThread())
      m_Thread.get()->Synchronize(ResetDeviceFromMainThread, this);
   else if ( FAILED( hResult = m_D3DDevice->Reset( &m_D3DParams ) ) )
      return hResult;

   if ( FAILED( hResult = CreateBuffers() ) )
      return hResult;

   if ( FAILED( hResult = SetState( RSS_DEFAULT ) ) )
      return hResult;

   return S_OK;
}

bool CD3DRenderer::PreRenderScene( void )
{
   if ( !CRenderer::PreRenderScene() )
      return false;

   if ( m_D3DDevice == NULL)
   {
      m_Engine->GetSysLogger()->Log( L"\tD3DDevice is not created", CMessageERR() );
      return false;
   }

   return true;
}

bool CD3DRenderer::RenderScene( void )
{
   HRESULT hResult;

   // Test the cooperative level to see if it's okay to render
   if FAILED( hResult = m_D3DDevice->TestCooperativeLevel() )
   {
      // The device has been lost but cannot be reset at this time.
      // So wait until it can be reset.
      if ( hResult == D3DERR_DEVICELOST )
      {
         Sleep( 50 );
         return true;
      }

      // If we are windowed, read the desktop format and 
      // ensure that the Direct3D device is using the same format 
      // since the user could have changed the desktop bitdepth 
      // TODO: handle it if needed

      if ( hResult == D3DERR_DEVICENOTRESET )
      {
         if( FAILED( hResult = ResetDevice() ) )
         {
            if ( hResult == D3DERR_DEVICELOST )
            {
               // The device was lost again, so continue waiting until it can be reset.
               Sleep( 50 );
               return true;
            }
            else
            {               
               ((CEngine*)m_Engine)->GetSysLogger()->Log( L"\tD3DDevice could not be reset. Catastrofic", CMessageFAT() );
               Stop();
               ((CEngine*)m_Engine)->SetTerminated( true );
               return true;
            }
         }
      }
   }

   m_D3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, m_BackgroundColor, 1.0f, 0);


   if SUCCEEDED(m_D3DDevice->BeginScene())
   {
      if ( !CRenderer::RenderScene() )
         return false;

      if ( m_LastSpriteVertexIndex > 0 || m_LastModelVertexIndex > 0 )
         RenderBatch();
      m_LastSpriteVertexIndex = 0;
      m_LastModelVertexIndex = 0;
      m_LastModelIndexIndex = 0;

      ::RECT rc;
      rc.left = 0;
      rc.top = 0;
      rc.right = 640;
      rc.bottom = 480;

#ifdef USE_DX9
      m_D3DXFont->DrawText(NULL,
      (L"FPS: " + IntToStr( m_FPS )).c_str(),
                            -1,
                            &rc,
                            DT_LEFT || DT_NOCLIP,
                            0xffffffff);
#else
#endif

      m_D3DDevice->EndScene();
   }

   return true;
}

bool CD3DRenderer::PostRenderScene( void )
{
   if ( !CRenderer::PostRenderScene() )
      return false;

   m_D3DDevice->Present(NULL, NULL, 0, NULL);

   return true;
}

bool CD3DRenderer::ClearLights( void )
{
   if ( !CRenderer::ClearLights() )
      return false;

   HRESULT hResult;

   for ( UINT32 iLight = 0; iLight < m_CurrentLightIndex; ++iLight )
   {
      if ( FAILED( hResult = m_D3DDevice->LightEnable( iLight, FALSE ) ) )
         return false;
   }

   m_CurrentLightIndex = 0;

   return true;
}

HRESULT CD3DRenderer::UnlockSpriteBuffers( void )
{
   HRESULT hResult;

   if ( FAILED( hResult = m_SpriteVB->Unlock() ) )
   {
      return hResult;
   }

   return S_OK;
}

HRESULT CD3DRenderer::LockSpriteBuffers( void )
{
   HRESULT hResult;

#ifdef USE_DX9
   if ( FAILED( hResult = m_SpriteVB->Lock( 0, 0, (void**)&m_SpriteVBData, 0 ) ) )
#else
   if ( FAILED( hResult = m_SpriteVB->Lock( 0, 0, (BYTE**)&m_SpriteVBData, 0 ) ) )
#endif
   {
      return hResult;
   }

   return S_OK;
}

HRESULT CD3DRenderer::UnlockModelBuffers( void )
{
   HRESULT hResult;

   if ( FAILED( hResult = m_ModelVB->Unlock() ) )
   {
      return hResult;
   }

   if ( FAILED( hResult = m_ModelIB->Unlock() ) )
   {
      return hResult;
   }

   return S_OK;
}

HRESULT CD3DRenderer::LockModelBuffers( void )
{
   HRESULT hResult;

#ifdef USE_DX9
   if ( FAILED( hResult = m_ModelVB->Lock( 0, 0, (void**)&m_ModelVBData, 0 ) ) )
#else
   if ( FAILED( hResult = m_ModelVB->Lock( 0, 0, (BYTE**)&m_ModelVBData, 0 ) ) )
#endif
   {
      return hResult;
   }

#ifdef USE_DX9
   if ( FAILED( hResult = m_ModelIB->Lock( 0, 0, (void**)&m_ModelIBData, 0 ) ) )
#else
   if ( FAILED( hResult = m_ModelIB->Lock( 0, 0, (BYTE**)&m_ModelIBData, 0 ) ) )
#endif
   {
      return hResult;
   }

   return S_OK;
}

bool CD3DRenderer::RenderSprite( CSprite* Sprite, 
                     const VECTOR3F& Position, 
                     const VECTOR3F& Scale, 
                     const VECTOR3F& Rotation,
                     COLOR Color
                     )
{

   HRESULT hResult;

   if ( m_LastSpriteVertexIndex == 0)
   {
      if ( FAILED( hResult = LockSpriteBuffers() ) )
         return false;
   }

   if ( ( m_CurrentMaterialIndex != Sprite->m_MaterialIndex ) ||
      ( m_CurrentPrimitiveType != ptTriangleList ) ||
      ( m_CurrentVertexType != D3DFVF_VERTEX_SPRITE ) )
   {
      if ( m_LastSpriteVertexIndex > 0 || m_LastModelVertexIndex > 0 )
      {
         if ( FAILED( hResult = RenderBatch() ) )
         {
            return false;
         }

         if ( m_LastSpriteVertexIndex > 0 )
            if ( FAILED( hResult = LockSpriteBuffers() ) )
               return false;
      }

      m_CurrentMaterialIndex = Sprite->m_MaterialIndex;
      m_CurrentPrimitiveType = ptTriangleList;
      m_CurrentVertexType = D3DFVF_VERTEX_SPRITE;

      m_LastSpriteVertexIndex = 0;
   }

   float tx1, ty1, tx2, ty2;

   const float Width = (float)m_Width; 
   const float Height = (float)m_Height;
   const float HalfWidth = Width*0.5f; 
   const float HalfHeight = Height*0.5f;

   const float HalfPixelY = 0.5f/Width;
   const float HalfPixelX = 0.5f/Height;
   const float HalfScreenX = 1.0f;
   const float HalfScreenY = 1.0f;

   const float ScaleX_T = Scale.X/HalfWidth;
   const float ScaleY_T = Scale.Y/HalfHeight;

   const float PosX_T = Position.X/HalfWidth - HalfScreenX - HalfPixelY;
   const float PosY_T = -(Position.Y/HalfHeight - HalfScreenY ) + HalfPixelY;


   float CenterX = ScaleX_T*0.5f;
   float CenterY = ScaleY_T*0.5f;

   tx1 = CenterX;
   ty1 = CenterX;
   tx2 = CenterX;
   ty2 = CenterX;

   if (Rotation.Z != 0.0f)
   {
      float sint, cost;

      cost = cosf( Rotation.Z );
      sint = sinf( Rotation.Z );

//      m_SpriteVBData[m_LastSpriteVertexIndex + 0].P = CreateVector4f( Position.X + (tx1*cost - ty1*sint), Position.Y + (tx1*cost + ty1*sint), Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 1].P = CreateVector4f( Position.X + (tx2*sint - ty1*cost), Position.Y + (tx2*sint + ty1*cost), Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 2].P = CreateVector4f( Position.X + (tx2*cost - ty2*sint), Position.Y + (tx2*cost + ty2*sint), Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 3].P = CreateVector4f( Position.X + (tx1*sint - ty2*cost), Position.Y + (tx1*sint + ty2*cost), Position.Z );

      m_SpriteVBData[m_LastSpriteVertexIndex + 0].P = VECTOR3F( PosX_T + (tx1*cost + ty1*sint) + ScaleX_T*0.5f,           PosY_T  +(tx1*cost - ty1*sint) - ScaleY_T*0.5f           , Position.Z );
      m_SpriteVBData[m_LastSpriteVertexIndex + 1].P = VECTOR3F( PosX_T + (tx2*sint - ty1*cost) + ScaleX_T- ScaleX_T*0.5f, PosY_T + (tx2*sint + ty1*cost) - ScaleY_T*0.5f           , Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 2].P = VECTOR3F( PosX_T + (tx2*cost + ty2*sint) + ScaleX_T- ScaleX_T*0.5f, PosY_T + (tx2*cost - ty2*sint) - ScaleY_T+ ScaleY_T*0.5f, Position.Z );
//        m_SpriteVBData[m_LastSpriteVertexIndex + 3].P = VECTOR3F( PosX_T + (tx1*sint + ty2*cost) + ScaleX_T*0.5f,           PosY_T + (tx1*sint - ty2*cost) - ScaleY_T+ ScaleY_T*0.5f, Position.Z );

//      m_SpriteVBData[m_LastSpriteVertexIndex + 0].P = VECTOR3F( PosX_T,            PosY_T           , Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 1].P = VECTOR3F( PosX_T + ScaleX_T, PosY_T           , Position.Z );
      m_SpriteVBData[m_LastSpriteVertexIndex + 2].P = VECTOR3F( PosX_T + ScaleX_T, PosY_T - ScaleY_T, Position.Z );
      m_SpriteVBData[m_LastSpriteVertexIndex + 3].P = VECTOR3F( PosX_T,            PosY_T - ScaleY_T, Position.Z );
   }
   else
   {
//      m_SpriteVBData[m_LastSpriteVertexIndex + 0].P = CreateVector4f( Position.X - CenterX - HalfPixel, Position.Y - CenterY - HalfPixel, Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 1].P = CreateVector4f( Position.X + CenterX - HalfPixel, Position.Y - CenterY - HalfPixel, Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 2].P = CreateVector4f( Position.X + CenterX - HalfPixel, Position.Y + CenterY , Position.Z );
//      m_SpriteVBData[m_LastSpriteVertexIndex + 3].P = CreateVector4f( Position.X - CenterX - HalfPixel, Position.Y + CenterY , Position.Z );

      m_SpriteVBData[m_LastSpriteVertexIndex + 0].P = VECTOR3F( PosX_T,            PosY_T           , Position.Z );
      m_SpriteVBData[m_LastSpriteVertexIndex + 1].P = VECTOR3F( PosX_T + ScaleX_T, PosY_T           , Position.Z );
      m_SpriteVBData[m_LastSpriteVertexIndex + 2].P = VECTOR3F( PosX_T + ScaleX_T, PosY_T - ScaleY_T, Position.Z );
      m_SpriteVBData[m_LastSpriteVertexIndex + 3].P = VECTOR3F( PosX_T,            PosY_T - ScaleY_T, Position.Z );
   }

//   m_SpriteVBData[m_LastSpriteVertexIndex + 0].C = Color;
//   m_SpriteVBData[m_LastSpriteVertexIndex + 1].C = Color;
//   m_SpriteVBData[m_LastSpriteVertexIndex + 2].C = Color;
//   m_SpriteVBData[m_LastSpriteVertexIndex + 3].C = Color;

   m_SpriteVBData[m_LastSpriteVertexIndex + 0].C = 0xFFFFFFFF;
   m_SpriteVBData[m_LastSpriteVertexIndex + 1].C = 0xFFFFFFFF;
   m_SpriteVBData[m_LastSpriteVertexIndex + 2].C = 0xFFFFFFFF;
   m_SpriteVBData[m_LastSpriteVertexIndex + 3].C = 0xFFFFFFFF;

   m_SpriteVBData[m_LastSpriteVertexIndex + 0].U = Sprite->m_Vetrices[0].U;
   m_SpriteVBData[m_LastSpriteVertexIndex + 0].V = Sprite->m_Vetrices[0].V;
   m_SpriteVBData[m_LastSpriteVertexIndex + 1].U = Sprite->m_Vetrices[1].U;
   m_SpriteVBData[m_LastSpriteVertexIndex + 1].V = Sprite->m_Vetrices[1].V;
   m_SpriteVBData[m_LastSpriteVertexIndex + 2].U = Sprite->m_Vetrices[2].U;
   m_SpriteVBData[m_LastSpriteVertexIndex + 2].V = Sprite->m_Vetrices[2].V;
   m_SpriteVBData[m_LastSpriteVertexIndex + 3].U = Sprite->m_Vetrices[3].U;
   m_SpriteVBData[m_LastSpriteVertexIndex + 3].V = Sprite->m_Vetrices[3].V;

   m_LastSpriteVertexIndex += 4;

   return true;
}

HRESULT CD3DRenderer::RenderBatch( void )
{
   HRESULT hResult;

#ifdef USE_DX9
   if ( FAILED( hResult = m_D3DDevice->SetFVF( m_CurrentVertexType ) ) )
#else
   if ( FAILED( hResult = m_D3DDevice->SetVertexShader( m_CurrentVertexType ) ) )   
#endif
   {
      return hResult;
   }

   if ( !m_WireFrame )
      m_Engine->GetResources()->GetMaterialRef( m_CurrentMaterialIndex )->Set();

   switch ( m_CurrentVertexType )
   {
   case D3DFVF_VERTEX_SPRITE:
      if ( FAILED( hResult = UnlockSpriteBuffers() ) )
         return false;

#ifdef USE_DX9
      if ( FAILED( hResult = m_D3DDevice->SetStreamSource( 0, m_SpriteVB, 0, sizeof( VERTEX_SPRITE ) ) ) )
#else
      if ( FAILED( hResult = m_D3DDevice->SetStreamSource( 0, m_SpriteVB, sizeof( VERTEX_SPRITE ) ) ) )
#endif
      {
         return hResult;
      }

#ifdef USE_DX9
      if ( FAILED( hResult = m_D3DDevice->SetIndices( m_SpriteIB ) ) )
#else
      if ( FAILED( hResult = m_D3DDevice->SetIndices( m_SpriteIB, 0 ) ) )
#endif
      {
         return hResult;
      }

#ifdef USE_DX9
      if ( FAILED( hResult = m_D3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, (UINT)m_LastSpriteVertexIndex, 0, (UINT)m_LastSpriteVertexIndex>>1 ) ) )
#else
      if ( FAILED( hResult = m_D3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, (UINT)m_LastSpriteVertexIndex, 0, (UINT)m_LastSpriteVertexIndex>>1 ) ) )
#endif
      {
         return hResult;
      }
      break;
   case D3DFVF_VERTEX_MODEL:
      if ( FAILED( hResult = UnlockModelBuffers() ) )
         return false;
#ifdef USE_DX9
      if ( FAILED( hResult = m_D3DDevice->SetStreamSource( 0, m_ModelVB, 0, sizeof( VERTEX_MODEL ) ) ) )
#else
      if ( FAILED( hResult = m_D3DDevice->SetStreamSource( 0, m_ModelVB, sizeof( VERTEX_MODEL ) ) ) )
#endif
      {
         return hResult;
      }

#ifdef USE_DX9
      if ( FAILED( hResult = m_D3DDevice->SetIndices( m_ModelIB ) ) )
#else
      if ( FAILED( hResult = m_D3DDevice->SetIndices( m_ModelIB, 0 ) ) )
#endif
      {
         return hResult;
      }

      D3DPRIMITIVETYPE D3DPrimitiveType;
      RWORD PrimCount = 0;

      switch ( m_CurrentPrimitiveType )
      {
      case ptTriangleStrip:
            D3DPrimitiveType = D3DPT_TRIANGLESTRIP;
            PrimCount = 0;
            break;
      case ptTriangleList:
            D3DPrimitiveType = D3DPT_TRIANGLELIST;
            PrimCount = m_LastModelIndexIndex/3;
            break;
      case ptTriangleFan:
            D3DPrimitiveType = D3DPT_TRIANGLEFAN;
            PrimCount = 0;
            break;
      case ptLineList:
            D3DPrimitiveType = D3DPT_LINELIST;
            PrimCount = m_LastModelIndexIndex>>1;
            break;
      case ptPoints:
            D3DPrimitiveType = D3DPT_POINTLIST;
            PrimCount = m_LastModelVertexIndex;
            break;
      }

#ifdef USE_DX9
      if ( FAILED( hResult = m_D3DDevice->DrawIndexedPrimitive( D3DPrimitiveType, 0, 0, (UINT)m_LastModelVertexIndex, 0, (UINT)PrimCount ) ) )
#else
      if ( FAILED( hResult = m_D3DDevice->DrawIndexedPrimitive( D3DPrimitiveType, 0, (UINT)m_LastModelVertexIndex, 0, (UINT)PrimCount ) ) )
#endif
      {
         return hResult;
      }
      break;
   }

   return S_OK;
}

bool CD3DRenderer::RenderModel( CModel* Model, 
                     const VECTOR3F& Position, 
                     const VECTOR3F& Scale, 
                     const VECTOR3F& Rotation )
{
   HRESULT hResult;

   for( DETAILLIST::iterator iDetail = Model->m_Details.begin();  
   iDetail != Model->m_Details.end(); ++iDetail)
   {

      if ( m_LastModelVertexIndex == 0)
      {
         if ( FAILED( hResult = LockModelBuffers() ) )
            return false;
      }

      if ( ( m_CurrentMaterialIndex != (*iDetail)->MaterialIndex ) ||
         ( m_CurrentPrimitiveType != (*iDetail)->Mesh.PrimitiveType ) ||
         ( m_CurrentVertexType != D3DFVF_VERTEX_MODEL ) )
      {
         if ( m_LastSpriteVertexIndex > 0 || m_LastModelVertexIndex > 0 )
         {
            if ( FAILED( hResult = RenderBatch() ) )
            {
               return false;
            }

            if ( m_LastModelVertexIndex > 0 )
               if ( FAILED( hResult = LockModelBuffers() ) )
                  return false;
         }

         m_CurrentMaterialIndex = (*iDetail)->MaterialIndex;

         if ( !m_WireFrame )
            m_CurrentPrimitiveType = (*iDetail)->Mesh.PrimitiveType;
         else
            m_CurrentPrimitiveType = ptLineList;

         m_CurrentVertexType = D3DFVF_VERTEX_MODEL;

         m_LastModelVertexIndex = 0;
         m_LastModelIndexIndex = 0;
      }

      // TODO: Optimize coping of data
      for ( RWORD iVertex = 0; iVertex < (*iDetail)->Mesh.VerticesCount; ++iVertex )
      {
         (*(m_ModelVBData + m_LastModelVertexIndex + iVertex)).P = (*iDetail)->Mesh.Vertices[iVertex].P + Position;
         (*(m_ModelVBData + m_LastModelVertexIndex + iVertex)).N = (*iDetail)->Mesh.Vertices[iVertex].N;
         (*(m_ModelVBData + m_LastModelVertexIndex + iVertex)).U = (*iDetail)->Mesh.Vertices[iVertex].U;
         (*(m_ModelVBData + m_LastModelVertexIndex + iVertex)).V = (*iDetail)->Mesh.Vertices[iVertex].V;
      }

      if ( !m_WireFrame )
      {

         for ( RWORD iIndex = 0; iIndex < (*iDetail)->Mesh.IndicesCount; ++iIndex )
         {
            (*(m_ModelIBData + m_LastModelIndexIndex + iIndex)) = (*iDetail)->Mesh.Indices[iIndex] + m_LastModelVertexIndex;
         }

         m_LastModelIndexIndex += (*iDetail)->Mesh.IndicesCount;
      }
      else
      {
         switch ( (*iDetail)->Mesh.PrimitiveType )
         {
            case ptTriangleList:
               {
                  RWORD iLineIndex = 0;

                  for ( RWORD iIndex = 0; iIndex < (*iDetail)->Mesh.IndicesCount; iIndex += 3 )
                  {
                     (*(m_ModelIBData + m_LastModelIndexIndex + iLineIndex + 0)) = (*iDetail)->Mesh.Indices[iIndex + 0] + m_LastModelVertexIndex;
                     (*(m_ModelIBData + m_LastModelIndexIndex + iLineIndex + 1)) = (*iDetail)->Mesh.Indices[iIndex + 1] + m_LastModelVertexIndex;
                     (*(m_ModelIBData + m_LastModelIndexIndex + iLineIndex + 2)) = (*iDetail)->Mesh.Indices[iIndex + 1] + m_LastModelVertexIndex;
                     (*(m_ModelIBData + m_LastModelIndexIndex + iLineIndex + 3)) = (*iDetail)->Mesh.Indices[iIndex + 2] + m_LastModelVertexIndex;
                     (*(m_ModelIBData + m_LastModelIndexIndex + iLineIndex + 4)) = (*iDetail)->Mesh.Indices[iIndex + 2] + m_LastModelVertexIndex;
                     (*(m_ModelIBData + m_LastModelIndexIndex + iLineIndex + 5)) = (*iDetail)->Mesh.Indices[iIndex + 0] + m_LastModelVertexIndex;

                     iLineIndex += 6;
                  }

                  m_LastModelIndexIndex += (*iDetail)->Mesh.IndicesCount*2;
               }
               break;
         }
      }

      m_LastModelVertexIndex += (*iDetail)->Mesh.VerticesCount;
   }

   return true;
}

bool CD3DRenderer::RenderLight( LIGHT* Light )
{
   if ( !CRenderer::RenderLight( Light ) )
      return false;

    /*vecDir = D3DXVECTOR3(cosf(timeGetTime()/350.0f),
                         1.0f,
                         sinf(timeGetTime()/350.0f) );
    D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );*/

    D3DLIGHT9 D3DLight;
    ZeroMemory( &D3DLight, sizeof( D3DLIGHT9 ) );

   switch ( Light->Type )
   {
   case ltDirectional:
      D3DLight.Type = D3DLIGHT_DIRECTIONAL;
      break;
   case ltPoint:
      D3DLight.Type = D3DLIGHT_POINT;
      break;
   case ltSpot:
      D3DLight.Type = D3DLIGHT_SPOT;
      break;
   }

   D3DLight.Diffuse.a = Light->Diffuse.A;
   D3DLight.Diffuse.r = Light->Diffuse.R;
   D3DLight.Diffuse.g = Light->Diffuse.G;
   D3DLight.Diffuse.b = Light->Diffuse.B;

   D3DLight.Specular.a = Light->Specular.A;
   D3DLight.Specular.r = Light->Specular.R;
   D3DLight.Specular.g = Light->Specular.G;
   D3DLight.Specular.b = Light->Specular.B;

   D3DLight.Ambient.a = Light->Ambient.A;
   D3DLight.Ambient.r = Light->Ambient.R;
   D3DLight.Ambient.g = Light->Ambient.G;
   D3DLight.Ambient.b = Light->Ambient.B;

   VECTOR3F N = NormalizeVector( Light->Direction );

   D3DLight.Direction.x = N.X;
   D3DLight.Direction.y = N.Y;
   D3DLight.Direction.z = N.Z;
      D3DLight.Range = Light->Range;

   HRESULT hResult;

    if ( FAILED( hResult = m_D3DDevice->SetLight( m_CurrentLightIndex, &D3DLight ) ) )
      return false;

    m_D3DDevice->LightEnable( m_CurrentLightIndex, TRUE );
    m_D3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    m_D3DDevice->SetRenderState( D3DRS_AMBIENT, 0x00909090 );


   m_CurrentLightIndex++;

   return true;
}

bool CD3DRenderer::SetTransform( TRANSFORM* Transform )
{
   if ( !CRenderer::SetTransform( Transform ) )
      return false;

   if ( m_LastSpriteVertexIndex > 0 || m_LastModelVertexIndex > 0 )
      RenderBatch();
   m_LastSpriteVertexIndex = 0;
   m_LastModelVertexIndex = 0;
   m_LastModelIndexIndex = 0;

   D3DXMATRIXA16 M;
   memcpy( &M.m[0][0], &Transform->Matrix.m[0][0], sizeof( float )*16 );

   switch ( Transform->Type )
   {
   case ttWorld:
      m_D3DDevice->SetTransform( D3DTS_WORLD, &M );
      break;
   case ttView:
      m_D3DDevice->SetTransform( D3DTS_VIEW, &M );
      break;
   case ttProj:
      m_D3DDevice->SetTransform( D3DTS_PROJECTION, &M );
      break;
   }

   return true;
}
}