#include "TestWorld.h"

CTestWorld::CTestWorld( void )
{

}

CTestWorld::~CTestWorld( void )
{
}

bool CTestWorld::Update( float DeltaTime, RTS::CScene* Scene )
{
   if ( !CWorld::Update( DeltaTime, Scene ) )
      return false;

   return true;
}

bool CTestWorld::HandleInputEvent( RTS::UINT32 EventID, RTS::INT32 wParam, RTS::INT32 lParam )
{
   if ( !CWorld::HandleInputEvent( EventID, wParam, lParam ) )
      return false;

   return true;
}


