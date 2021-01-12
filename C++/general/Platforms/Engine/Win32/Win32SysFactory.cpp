#include "Win32SysFactory.h"
#include "D3DMutator.h"
#include "D3DRenderer.h"
#include "DirectInput.h"
#include "D3DResources.h"


namespace RTS
{

CSysFactory* g_SysFactory = NULL;

CWin32SysFactory::CWin32SysFactory( void )
{
}

CWin32SysFactory::~CWin32SysFactory( void )
{
}

VOID_PTR CWin32SysFactory::CreateObject( const std::wstring& name, CEngine* Engine )
{
   if (name == L"D3DMutatorTranslate2D")
      return new CD3DMutatorTranslate2D();
   else if (name == L"Renderer")
      return new CD3DRenderer(Engine);
   else if (name == L"Input")
      return new CDirectInput(Engine);
   else if (name == L"Resources")
      return new CD3DResources();
   else
      throw;
}

}