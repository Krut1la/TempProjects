#include "Controller.h"
#include "Engine.h"
#include "MathUtils.h"
#include "StringUtils.h"
#include "DateTimeUtils.h"

namespace RTS
{
CControllerThread::CControllerThread( bool CreateSuspended, 
                            CController* Controller ):
CThread(CreateSuspended ), 
f_Controller( Controller ),
f_CallBack( NULL )
{

}

CControllerThread::~CControllerThread( void )
{
   
}

void CControllerThread::Execute( void )
{
   while ( !GetTerminated() )
   {
      f_Controller->Update();
      f_Controller->m_ExecutionCount++;
   }
   
   f_Controller->m_Engine->GetSysLogger()->Log( L"End ControllerThread", CMessageMSG() );
}


CController::CController( CEngine* Engine ):
CMTService( Engine ),
m_RendererRef( NULL )
{
   m_Name = L"Controller";
}

CController::~CController( void )
{
}


bool CController::Initialize( void )
{
   m_LastTime = GetTickCount();

   if ( !CMTService::Initialize() )
      return false;

   return true;
}

void CController::Finalize( void )
{
   CMTService::Finalize();
}

bool CController::Start( void )
{
   if ( !CMTService::Start() )
      return false;

   if ( (m_Engine)->GetMultiThread() ) 
   {
      m_Engine->GetSysLogger()->Log( L"Start ControllerThread", CMessageMSG() );
      m_Thread = std::auto_ptr<CControllerThread>( new CControllerThread(true, this) );
   
      m_Thread->SetPriority( tpHigher );
      m_Thread->Resume();
   }
   else
   {
      Update( );
      m_ExecutionCount++;
   }
   
   return true;
}

bool CController::Update( void )
{
   if ( m_RendererRef == NULL )
   {
      m_RendererRef = m_Engine->FindService( L"Renderer" );

      if ( m_RendererRef == NULL )
         return false;
   }

   UINT32 Delta = GetTicks() - m_LastTime;

//   if ( Delta == 0 )
   {
//      if ( m_Engine->GetMultiThread() )
//         Sleep(0);

//      return true;
   }

   CScene* PrimaryScene;
   m_RendererRef->DoCommand( MTC_GET_PRIMARY_SCENE, &PrimaryScene, NULL );

   CScene* SecondaryScene;
   m_RendererRef->DoCommand( MTC_GET_SECONDARY_SCENE, &SecondaryScene, NULL );

   CEvent* SecondaryIsFree;
   m_RendererRef->DoCommand( MTC_GET_SECONDARY_ISFREE, &SecondaryIsFree, NULL );

   CEvent* SecondaryIsReady;
   m_RendererRef->DoCommand( MTC_GET_SECONDARY_ISREADY, &SecondaryIsReady, NULL );

   if ( Delta > 50 )
      Delta = 1;

   m_LastTime = GetTicks();

   if ( m_Engine->GetMultiThread() )
   {

      PrimaryScene->Reset();

      if ( !m_Engine->GetWorld()->Update( (float)(Delta), PrimaryScene ) )
         return false;

      if ( !m_Engine->GetGUI()->Update( (float)(Delta), PrimaryScene ) )
         return false;
   }
   else
   {
      SecondaryScene->Reset();

      if ( !m_Engine->GetWorld()->Update( (float)(Delta), SecondaryScene ) )
         return false;

      if ( !m_Engine->GetGUI()->Update( (float)(Delta), SecondaryScene ) )
         return false;
   }
   
   if ( m_Engine->GetMultiThread() )
   {
      if ( SecondaryIsFree->WaitFor( 200 ) == WAIT_TIMEOUT )
         return false;

      SecondaryIsFree->ResetEvent();
   }

   if ( m_Engine->GetMultiThread() ) 
   {
      m_RendererRef->DoCommand( MTC_SWAP_SCENES, NULL, NULL );
      SecondaryIsReady->SetEvent();
   }

   return true;
}




}