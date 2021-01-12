#include "TestEngine.h"
#include "ScriptUtils.h"
#include "Cursor.h"
#include "Image.h"
#include "Zazaka.h"
#include "BasketBall.h"
#include "Room.h"
#include "D3DResources.h"
#include "D3DRenderer.h"
#include "DirectInput.h"
#include <math.h>


CTestEngine::CTestEngine(void)
{

}

CTestEngine::~CTestEngine(void)
{

}

bool CTestEngine::Initialize()
{
   UINT32 ScreenWidth  = 1680;
   UINT32 ScreenHeight = 1050;

   std::wstring WindowName = L"";

   m_SysLogger = std::auto_ptr<RTS::CFileLogger>( new RTS::CFileLogger( L"System.log" ) );

   m_SysLogger->Log(L"Log started", RTS::CMessageMSG());

   RTS::CDirectInput*   DirectInput;
   RTS::CD3DRenderer*   D3DRenderer;
   RTS::CController*   Controller;

   m_Resources   = std::auto_ptr<RTS::CResources>( new RTS::CD3DResources() );

   DirectInput = new RTS::CDirectInput( this );
   m_Services.push_back( DirectInput );

   DirectInput->DoCommand( RTS::MTC_SET_DEVICE_SIZE, &ScreenWidth, &ScreenHeight );
   DirectInput->DoCommand( RTS::MTC_SET_WINDOW_NAME, (RTS::VOID_PTR)WindowName.c_str(), NULL );

   Controller = new RTS::CController( this );
   m_Services.push_back( Controller );

   D3DRenderer = new RTS::CD3DRenderer( this );
   m_Services.push_back( D3DRenderer );

   D3DRenderer->DoCommand( RTS::MTC_SET_DEVICE_SIZE, &ScreenWidth, &ScreenHeight );

   m_World = std::auto_ptr<CTestWorld>( new CTestWorld(  ) );
   m_GUI   = std::auto_ptr<RTS::CGUI>( new RTS::CGUI(  ) );

   DirectInput->AddUpdatable( m_World.get() );
   DirectInput->AddUpdatable( m_GUI.get() );

   if ( !CEngine::Initialize() )
      return false;

   m_GUI->SetScreenWidth( (float)ScreenWidth );
   m_GUI->SetScreenHeight( (float)ScreenHeight );

   m_Resources->SetDevice( D3DRenderer->GetDevice() );

   return true;
}

void CTestEngine::Finalize()
{
   CEngine::Finalize();

   m_SysLogger->Log(L"Log finished", RTS::CMessageMSG());
}

void CTestEngine::Run()
{
   CEngine::Run();
}

bool CTestEngine::LoadPAK( const std::wstring& FileName )
{
   m_Resources->LoadPack( L"Resources.lua" );

   RTS::RWORD TextureID = 0;
   RTS::RWORD RenderableID = 0;
   RTS::RWORD FontID = 0;

   m_Resources->LoadFont(L"arial.ttf", &FontID);

   m_Resources->LoadMaterial(L"empty", &TextureID);

   m_Resources->LoadMaterial(L"cursor1.png", &TextureID);
   m_Resources->LoadSprite( L"cursor", &RenderableID );

   m_Resources->LoadMaterial(L"galaxy.png", &TextureID);
   m_Resources->LoadSprite( L"bg", &RenderableID );

   m_Resources->LoadMaterial(L"zazaka.png", &TextureID);
   m_Resources->LoadSprite( L"zazaka", &RenderableID );

   m_Resources->LoadMaterial(L"BasketBall.png", &TextureID);
   m_Resources->LoadModel( L"BasketBall", &RenderableID );

   m_Resources->LoadModel( L"Room", &RenderableID );

   //RTS::CImage* Image = new RTS::CImage( m_GUI.get(), 0 );
   //m_GUI->AddControl( Image );

   /*RTS::CRoom* Room = new RTS::CRoom( m_World.get() );

   m_World->AddUnit( Room );

   for ( int ii = 10; ii > 0; ii--)
   {
      RTS::CBasketBall* BasketBall = new RTS::CBasketBall( m_World.get() );

      m_World->AddUnit( BasketBall );
   } */
   

   for ( int ii = 1; ii > 0; ii--)
   {
      RTS::CZazaka* Zazaka = new RTS::CZazaka( m_World.get() );

      m_World->AddUnit( Zazaka );
   }
   

   RTS::CCursor* Cursor = new RTS::CCursor( m_GUI.get() );
   m_GUI->AddControl( Cursor );


   return true;
}

