#include <windows.h>
#include "Win32SysFactory.h"

#include "Engine.h"

extern RTS::CSysFactory* RTS::g_SysFactory;

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
   RTS::g_SysFactory = new RTS::CWin32SysFactory();

   RTS::CEngine* TestGame = new RTS::CEngine();

   _TRY_BEGIN
      TestGame->SetMultiThread( false );
      if( TestGame->Initialize() )
      {
         TestGame->LoadPAK(L"Test.pak");
         TestGame->Run();
         TestGame->Finalize();
      }
   _CATCH_ALL
      // Catastrofic
      MessageBox( NULL, L"Error", L"Catastrofic failure", 0 );
   _CATCH_END

   delete RTS::g_SysFactory;
   delete TestGame;
      
   return 0;
}
