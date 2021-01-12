#include "DirectInput.h"
#include "Engine.h"
#include "SystemUtils.h"
#include <assert.h>

namespace RTS
{

CDirectInputThread::CDirectInputThread( bool CreateSuspended, 
                              CDirectInput* DirectInput ):
CThread(CreateSuspended ), 
f_DirectInput( DirectInput ),
f_CallBack( NULL )
{

}

CDirectInputThread::~CDirectInputThread( void )
{

}

void __fastcall TestTest(VOID_PTR Data)
{
   MSG msg;
//   while( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) ) 
//   while( GetMessage( &msg, NULL, 0, 0 ) ) 
   {
      //GetMessage( &msg, NULL, 0, 0 );
      PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE );

      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }
}

void CDirectInputThread::Execute( void )
{
   while ( !GetTerminated() )
   {
      {
         _TRY_BEGIN
            if( f_DirectInput->m_Engine->GetMultiThread() )
               Synchronize(TestTest, NULL);
            else
               TestTest(NULL);
         _CATCH_ALL
            Terminate();
         _CATCH_END
      }
      if ( f_DirectInput->GetActive() )
      {
         f_DirectInput->ReadMouseData();
         f_DirectInput->ReadKeyboardData();
         //f_DirectInput->ReadJoysticData();
      }
      if( f_DirectInput->m_Engine->GetMultiThread() )
         Sleep(10);
      //f_DirectInput->ReadNetworkData(); 

      f_DirectInput->m_ExecutionCount++;
   }


   f_DirectInput->m_Engine->GetSysLogger()->Log( L"End InputThread", CMessageMSG() );
}




CDirectInput::CDirectInput( CEngine* Engine ):
CInput( Engine ),
m_Window( NULL ),
m_WindowTitle( L"" ), 
m_BufferSize( 16 ),
m_DI8( NULL ),
m_MouseDevice( NULL ),
m_Active( true )
{
   m_hInstance = (HINSTANCE)GetModuleHandle( NULL );

   GetModuleFileName( NULL, m_szExePath, MAX_PATH );

   m_hIcon = ExtractIcon( m_hInstance, m_szExePath, 0 ); 
}

CDirectInput::~CDirectInput( void )
{

}

bool CDirectInput::Initialize( void )
{
   if ( !CInput::Initialize() )
      return true;    

   m_Engine->GetSysLogger()->Log( L"Initialize DirectInput", CMessageMSG() );

   if ( InitializeWindow() != S_OK ){
      m_Engine->GetSysLogger()->Log( L"\tInitializeWindow failed", CMessageERR() );
      return false;
   }

   if ( FAILED( DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
      IID_IDirectInput8, (VOID**)&m_DI8, NULL ) ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tDirectInput8Create failed", CMessageMSG() );
      return false;
   }

   if ( FAILED( InitializeMouse() ) ) 
      return false;

   if ( FAILED( InitializeKeyboard() ) ) 
      return false;

   return true;    
}

void CDirectInput::Finalize( void )
{
   m_Engine->GetSysLogger()->Log( L"Finalize DirectInput", CMessageMSG() );

   if ( FAILED( FinalizeMouse() ) ) 
      m_Engine->GetSysLogger()->Log( L"\tFinalizeMouse failed", CMessageERR() );


   if ( FAILED( FinalizeKeyboard() ) ) 
      m_Engine->GetSysLogger()->Log( L"\tFinalizeKeyboard failed", CMessageERR() );

   SAFE_RELEASE(m_DI8);

   if (FinalizeWindow() != S_OK){
      m_Engine->GetSysLogger()->Log( L"\tFinalizeWindow failed", CMessageERR() );
   }
}

void CDirectInput::ActivateWin32Mouse( void )
{
   int         Width;
   int         Height;
   ::RECT      WindowRect;

   Width = GetSystemMetrics (SM_CXSCREEN);
   Height = GetSystemMetrics (SM_CYSCREEN);

   GetWindowRect ( m_Window, &WindowRect);
   if ( WindowRect.left < 0 )
      WindowRect.left = 0;
   if ( WindowRect.top < 0 )
      WindowRect.top = 0;
   if ( WindowRect.right >= Width )
      WindowRect.right = Width - 1;
   if ( WindowRect.bottom >= Height - 1 )
      WindowRect.bottom = Height - 1;

   SetCapture ( m_Window );
   ClipCursor ( &WindowRect );
   while (ShowCursor ( FALSE ) >= 0)
      ;
}

void CDirectInput::DeactivateWin32Mouse( void )
{
   ClipCursor( NULL);
   ReleaseCapture();
   while ( ShowCursor( TRUE ) < 0 )
      ;
}

void CDirectInput::ActivateMouse( void )
{
   ActivateWin32Mouse();
}

void CDirectInput::DeactivateMouse( void )
{
   if( m_MouseDevice ) 
      m_MouseDevice->Unacquire();
   DeactivateWin32Mouse();
}

void CDirectInput::Activate( void )
{
   SetActive( true );

   ActivateMouse();
}

void CDirectInput::Deactivate( void )
{
   SetActive( false );

   DeactivateMouse();
}

LRESULT CALLBACK CDirectInput::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

   CDirectInput* DirectInput = (CDirectInput*)(GetProp( hWnd, L"InputServiceInst"));

   if ( DirectInput != NULL )
   {
      switch( uMsg )
      {    
      case WM_ACTIVATE:
         {
            int   Active, Minimized;

            Active = LOWORD(wParam);
            Minimized = (BOOL) HIWORD(wParam);

            if ( !Active || Minimized )
               DirectInput->Deactivate();
         
            break;
         }
      case INPUT_ACTIVATE:
         {

            break;
         }
      case INPUT_DEACTIVATE:
         {
            DirectInput->Deactivate();
            break;
         }
      case WM_LBUTTONDOWN:
         {
            DirectInput->Activate();
            return 0;
         }
      case WM_CLOSE:
         {
            DestroyWindow( hWnd );
            return 0;
         }

      case WM_DESTROY:
         DirectInput->Stop();
         PostQuitMessage( 0 );
         break;
      }
   }

   return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

HRESULT CDirectInput::InitializeWindow( void )
{
   m_Engine->GetSysLogger()->Log( L"Initialize D3DWindow", CMessageMSG() );

   // Register the windows class
   WNDCLASS wndClass;
   wndClass.style = CS_DBLCLKS;
   wndClass.lpfnWndProc = WndProc;
   wndClass.cbClsExtra = 0;
   wndClass.cbWndExtra = 0;
   wndClass.hInstance = m_hInstance;
   wndClass.hIcon = m_hIcon;
   wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
   wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   wndClass.lpszMenuName = NULL;
   wndClass.lpszClassName = L"Direct3DWindowClass";

   if( !RegisterClass( &wndClass ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tRegisterClass failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   // Create the render window
   m_Window = CreateWindowEx( 0, L"Direct3DWindowClass", m_WindowTitle.c_str(), WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE,
      100, 100, 1680 + GetSystemMetrics(SM_CXFIXEDFRAME)*2, 1050 + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + GetSystemMetrics(SM_CYCAPTION), 0,
      NULL, m_hInstance, NULL );

/*   m_Window = CreateWindowEx(WS_EX_TOPMOST, L"Direct3DWindowClass", m_WindowTitle.c_str(), WS_POPUP|WS_VISIBLE,
         0, 0, 0, 0,
         NULL, NULL, m_hInstance, NULL);
*/
   if( m_Window == NULL )
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateWindow failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   if ( !SetProp( m_Window, L"InputServiceInst", this ))
   {
      m_Engine->GetSysLogger()->Log( L"\tSetProp failed. ERR[%d]", CMessageERR() );
      return E_FAIL;
   }

   ShowWindow(m_Window, SW_SHOWNORMAL);

   return S_OK;
}

HRESULT CDirectInput::FinalizeWindow( void )
{
   m_Engine->GetSysLogger()->Log(L"Finalize D3DWindow", CMessageMSG());

   return S_OK;
}

HRESULT CDirectInput::InitializeMouse( void )
{
   HRESULT hr;

   m_Engine->GetSysLogger()->Log( L"Initialize Mouse device", CMessageMSG() );

   if ( FAILED( hr = m_DI8->CreateDevice( GUID_SysMouse, &m_MouseDevice, NULL ) ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateDevice failed", CMessageMSG() );
      return hr;
   }

   if ( FAILED( hr = m_MouseDevice->SetDataFormat( &c_dfDIMouse2 ) ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tSetDataFormat failed", CMessageMSG() );
      return hr;
   }


//   if ( FAILED( hr = m_MouseDevice->SetCooperativeLevel( m_Window, DISCL_EXCLUSIVE | DISCL_FOREGROUND )))
   if ( FAILED( hr = m_MouseDevice->SetCooperativeLevel( m_Window, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND )))
   {
      m_Engine->GetSysLogger()->Log( L"\tSetCooperativeLevel failed", CMessageMSG() );
      return hr;
   }

   DIPROPDWORD dipdw;
   dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
   dipdw.diph.dwObj        = 0;
   dipdw.diph.dwHow        = DIPH_DEVICE;
   dipdw.dwData            = m_BufferSize; // Arbitary buffer size

   if( FAILED( hr = m_MouseDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
      return hr;

   ActivateMouse();
   m_MouseDevice->Acquire();
   
   return S_OK;
}

HRESULT CDirectInput::InitializeKeyboard( void )
{
   HRESULT hr;

   m_Engine->GetSysLogger()->Log( L"Initialize Keyboard device", CMessageMSG() );

   if ( FAILED( hr = m_DI8->CreateDevice( GUID_SysKeyboard, &m_KeyboardDevice, NULL ) ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tCreateDevice failed", CMessageMSG() );
      return hr;
   }

   if ( FAILED( hr = m_KeyboardDevice->SetDataFormat( &c_dfDIKeyboard ) ) )
   {
      m_Engine->GetSysLogger()->Log( L"\tSetDataFormat failed", CMessageMSG() );
      return hr;
   }

//   if ( FAILED( hr = m_KeyboardDevice->SetCooperativeLevel( m_Window, DISCL_EXCLUSIVE | DISCL_FOREGROUND )))
    if ( FAILED( hr = m_KeyboardDevice->SetCooperativeLevel( m_Window, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND )))
   {
      m_Engine->GetSysLogger()->Log( L"\tSetCooperativeLevel failed", CMessageMSG() );
      return hr;
   }

   DIPROPDWORD dipdw;
   dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
   dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
   dipdw.diph.dwObj        = 0;
   dipdw.diph.dwHow        = DIPH_DEVICE;
   dipdw.dwData            = m_BufferSize; // Arbitary buffer size

   if( FAILED( hr = m_KeyboardDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
      return hr;

   m_KeyboardDevice->Acquire();

   return S_OK;
}

HRESULT CDirectInput::FinalizeMouse( void )
{
   m_Engine->GetSysLogger()->Log( L"Finalize Mouse device", CMessageMSG() );

   if( m_MouseDevice ) 
      m_MouseDevice->Unacquire();

   SAFE_RELEASE(m_MouseDevice);

   return S_OK;
}

HRESULT CDirectInput::FinalizeKeyboard( void )
{
   m_Engine->GetSysLogger()->Log( L"Finalize Keyboard device", CMessageMSG() );

   if( m_KeyboardDevice ) 
      m_KeyboardDevice->Unacquire();

   SAFE_RELEASE(m_KeyboardDevice);

   return S_OK;
}

bool CDirectInput::Start( void )
{
   if ( !CInput::Start() )
      return false; 

   if ( m_Engine->GetMultiThread() ) 
   {
      m_Engine->GetSysLogger()->Log( L"Start InputThread", CMessageMSG() );
      m_Thread = std::auto_ptr<CDirectInputThread>( new CDirectInputThread(true, this) );

      m_Thread->SetPriority( tpHigher );
      m_Thread->Resume();
   }
   else
   {
      MSG msg;
      while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE)) 
      {
         if ( !GetMessage (&msg, NULL, 0, 0) )
            m_Engine->SetTerminated( true );

         TranslateMessage (&msg);
         DispatchMessage (&msg);
      }

      if ( GetActive())
      {
         ReadMouseData();
         ReadKeyboardData();
      }

      m_ExecutionCount++;
   }
   return true;
}

HRESULT CDirectInput::ReadMouseData( void )
{
   DIDEVICEOBJECTDATA didod[ 16 ];  // Receives buffered data 
   DWORD              dwElements;
   DWORD              i;
   HRESULT            hr;

   if( NULL == m_MouseDevice ) 
      return E_FAIL;

   dwElements = m_BufferSize;
   hr = m_MouseDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
      didod, &dwElements, 0 );
   if( hr != DI_OK ) 
   {
      hr = m_MouseDevice->Acquire();
    //  while( hr == DIERR_INPUTLOST ) 
    //     hr = m_MouseDevice->Acquire();

      return S_OK; 
   }

   if( FAILED(hr) )  
      return hr;

   for( i = 0; i < dwElements; i++ ) 
   {
      switch( didod[ i ].dwOfs )
      {
      case DIMOFS_BUTTON0:
         break;

      case DIMOFS_BUTTON1:
         break;

      case DIMOFS_BUTTON2:
         break;

      case DIMOFS_BUTTON3:
         break;

      case DIMOFS_BUTTON4:
         break;

      case DIMOFS_BUTTON5:
         break;

      case DIMOFS_BUTTON6:
         break;

      case DIMOFS_BUTTON7:
         break;

      case DIMOFS_X:
         break;

      case DIMOFS_Y:
         break;

      case DIMOFS_Z:
         break;

      default:
         break;
      }

      switch( didod[ i ].dwOfs )
      {
      case DIMOFS_BUTTON0:
      case DIMOFS_BUTTON1:
      case DIMOFS_BUTTON2:
      case DIMOFS_BUTTON3:
      case DIMOFS_BUTTON4:
      case DIMOFS_BUTTON5:
      case DIMOFS_BUTTON6:
      case DIMOFS_BUTTON7:
         //                if( didod[ i ].dwData & 0x80 )
         //                    StringCchCat( strNewText, 128, TEXT("U ") );
         //                else
         //                    StringCchCat( strNewText, 128, TEXT("D ") );
         break;

      case DIMOFS_X:
      case DIMOFS_Y:
      case DIMOFS_Z:
         {
            break;
         }
      }
   }


   // read the raw delta counter and ignore
   // the individual sample time / values
   DIMOUSESTATE2      state;

   hr = m_MouseDevice->GetDeviceState(   sizeof(DIMOUSESTATE2), &state);

   if ( FAILED( hr ) ) 
   {
      return hr;
   }

//   if ( (state.lX > 0) | (state.lY > 0) ) 
   SendEventToUpdatables( MOUSE_MOVE, state.lX, state.lY );

   return S_OK;
}

HRESULT CDirectInput::ReadKeyboardData( void )
{
   if( NULL == m_KeyboardDevice ) 
      return E_FAIL;

   //    TCHAR              strLetter[10];    
   DIDEVICEOBJECTDATA didod[ 16 ];  // Receives buffered data 
   ::DWORD              dwElements;
   ::DWORD              i;
   HRESULT            hr;

   dwElements = m_BufferSize;
   hr = m_KeyboardDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
      didod, &dwElements, 0 );
   if( hr != DI_OK ) 
   {        
      hr = m_KeyboardDevice->Acquire();
    //  while( hr == DIERR_INPUTLOST ) 
         hr = m_KeyboardDevice->Acquire();

      return S_OK; 
   }

   for( i = 0; i < dwElements; i++ ) 
   {
      switch ( didod[i].dwOfs )
      {
      case DIK_ESCAPE:
         {
            m_Engine->SetTerminated( true );
            if ( m_Thread.get() != NULL )
               m_Thread->Terminate();
            break;
         }
      case 0x29:
         {
            if( m_KeyboardDevice ) 
               m_KeyboardDevice->Unacquire();

            SetActive( false );
            PostMessage(m_Window, INPUT_DEACTIVATE, 0, 0);

            break;
         }
      }
   }

   return S_OK;
}

bool CDirectInput::DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 )
{
   if ( CInput::DoCommand( Command, Data1, Data2 ) )
      return true;

   switch ( Command )
   {
   case MTC_SET_DEVICE_SIZE:
      m_WindowWidth   = *(UINT32*)Data1;
      m_WindowHeight   = *(UINT32*)Data2;
      return true;
   case MTC_SET_WINDOW_NAME:
      m_WindowTitle   = (wchar_t*)Data1;
      return true;
   case MTC_GET_WINDOW_HANDLE:
      *(HWND*)Data1 = m_Window;
      return true;
   default:
      return false;
   }

   return false;
}
}