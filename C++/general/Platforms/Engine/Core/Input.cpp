#include "Input.h"

namespace RTS
{
CInput::CInput( CEngine* Engine ):
CMTService( Engine )
{
   m_Name = L"Input";
}

CInput::~CInput( void )
{

}

bool CInput::Initialize( void )
{
   if ( !CMTService::Initialize() )
      return false;

   return true;    
}

void CInput::Finalize( void )
{
   m_Updatables.clear();

   CMTService::Finalize();
}

bool CInput::Start( void )
{
   if ( !CMTService::Start() )
      return false;

   return true;
}

bool CInput::SendEventToUpdatables( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   for( IUPDATABLELIST::iterator iUpdatable = m_Updatables.begin();  iUpdatable != m_Updatables.end(); ++iUpdatable)
   {
      if ( !(*iUpdatable)->HandleInputEvent( EventID, wParam, lParam ) )
         return false;
   }

   return true;
}

void CInput::AddUpdatable( IUpdatable* Updatable )
{
   m_Updatables.push_back( Updatable );
}

bool CInput::DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 )
{
   if ( CMTService::DoCommand( Command, Data1, Data2 ) )
      return true;

   return false;
}

}