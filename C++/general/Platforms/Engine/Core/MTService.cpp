#include "MTService.h"
#include "Engine.h"
#include "StringUtils.h"

namespace RTS
{
CMTService::CMTService( CEngine* Engine ):
m_Engine( Engine ),
m_Thread( NULL ),
m_ExecutionCount( 0 )
{
   m_Initialized = false;
}

CMTService::~CMTService( void )
{
   if ( m_Initialized )
      Finalize();
}

bool CMTService::Initialize( void )
{
   m_Initialized = true;

   return true;
}

void CMTService::Finalize( void )
{
   m_Initialized = false;
   m_Engine->GetSysLogger()->Log( L"Service " + m_Name + L" was executed " + IntToStr( m_ExecutionCount ) + L" times", CMessageMSG() );
}

bool CMTService::Start( void )
{
   return true;
}

void CMTService::Stop( void )
{
   if ( m_Thread.get() != NULL)
      m_Thread->Terminate();
}

void CMTService::WaitFor( void )
{
   if ( m_Thread.get() != NULL)
      m_Thread->WaitFor();
}

bool CMTService::DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 )
{
   return false;
}
}