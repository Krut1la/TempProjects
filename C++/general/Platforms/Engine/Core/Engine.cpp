#include "Engine.h"
#include "FileLogger.h"
#include "SysFactory.h"
#include "Input.h"
#include "Controller.h"

namespace RTS
{

CEngine::CEngine(void)
{
   m_MultiThread   = false;
   m_Terminated   = false;
}

CEngine::~CEngine(void)
{
}


bool CEngine::Initialize()
{
   m_SysLogger = std::auto_ptr<CFileLogger>( new CFileLogger( L"System.log" ) );

   m_SysLogger->Log(L"Log started", CMessageMSG());

   m_Resources = std::auto_ptr<CResources>( (CResources*)g_SysFactory->CreateObject(L"Resources", this) );

   m_SysLogger->Log(L"Load settings", CMessageMSG());

   CInput* Input = g_SysFactory->CreateObject(L"Input", this);

   m_Services.push_back( Input );
   m_Services.push_back( new CController( this ) );
   m_Services.push_back( g_SysFactory->CreateObject(L"Renderer", this) );

   //Input->DoCommand( MTC_SET_DEVICE_SIZE, &ScreenWidth, &ScreenHeight );
   //Input->DoCommand( MTC_SET_WINDOW_NAME, (VOID_PTR)WindowName.c_str(), NULL );

//   m_World = std::auto_ptr<CTestWorld>( new CTestWorld(  ) );
//   m_GUI   = std::auto_ptr<RTS::CGUI>( new RTS::CGUI(  ) );

   Input->AddUpdatable( m_World.get() );
   Input->AddUpdatable( m_GUI.get() );

   for ( MTSERVICELIST::iterator iService = m_Services.begin();
      iService != m_Services.end();
      ++iService)
   {
      if ( !(*iService)->Initialize() )
         return false;
   }


   //m_GUI->SetEngineRef( this );
   
   return true;
}

void CEngine::Finalize()
{
   for ( MTSERVICELIST::iterator iService = m_Services.begin();
      iService != m_Services.end();
      ++iService)
   {
      (*iService)->Finalize();
   }

   for ( MTSERVICELIST::iterator iService = m_Services.begin();
      iService != m_Services.end();
      ++iService)
   {
      delete (*iService);
   }

   m_Services.clear();
}

CMTService* CEngine::FindService( const std::wstring& Name)
{
   for ( MTSERVICELIST::iterator iService = m_Services.begin();
      iService != m_Services.end();
      ++iService)
   {
      if ( (*iService)->GetName() == Name )
         return *iService;
   }

   return NULL;
}

void CEngine::Run()
{
   if ( GetMultiThread() )
   {
      m_SysLogger->Log(L"Start multithread mode...", CMessageMSG());
      _TRY_BEGIN
         for ( MTSERVICELIST::iterator iService = m_Services.begin();
            iService != m_Services.end();
            ++iService)
         {
            (*iService)->Start();
         }
      _CATCH_ALL
         m_SysLogger->Log(L"\tMain loop failed", CMessageFAT());
      _CATCH_END

         // Wait till the input service exits;
       FindService(L"Input")->WaitFor();

       for ( MTSERVICELIST::iterator iService = m_Services.begin();
          iService != m_Services.end();
          ++iService)
       {
          (*iService)->Stop();
          (*iService)->WaitFor();
       }
       m_SysLogger->Log(L"Stop multithread mode...", CMessageMSG());
   }
   else
   {
      m_SysLogger->Log(L"Start singlethread mode...", CMessageMSG());
      while ( !m_Terminated )
      {
         _TRY_BEGIN
            for ( MTSERVICELIST::iterator iService = m_Services.begin();
               iService != m_Services.end();
               ++iService)
            {
               (*iService)->Start();
            }
         _CATCH_ALL
            m_SysLogger->Log(L"\tMain loop failed", CMessageFAT());
         _CATCH_END
      }
      m_SysLogger->Log(L"Stopg singlethread mode...", CMessageMSG());
   }
}

bool CEngine::LoadPAK( const std::wstring& FileName )
{
   return true;
}



}