#ifndef DIRECTINPUT_H
#define DIRECTINPUT_H

#define DIRECTINPUT_VERSION 0x0800

#include "Input.h"

#include <dinput.h>

namespace RTS
{
const UINT INPUT_ACTIVATE   = WM_USER + 1984;
const UINT INPUT_DEACTIVATE = WM_USER + 1985;

const MTSERVICE_COMMAND MTC_SET_WINDOW_NAME         = 1001;
const MTSERVICE_COMMAND MTC_GET_WINDOW_HANDLE      = 1002;

class CDirectInput;

class CDirectInputThread: public CThread
{
   CDirectInput*   f_DirectInput;
public:

   THREADMETHOD f_CallBack;

   CDirectInputThread(bool CreateSuspended, CDirectInput* DirectInput);
   virtual ~CDirectInputThread(void);

   virtual void Execute( void );
};

class CDirectInput: public CInput{
   friend CDirectInputThread;
protected:
   // Settings
   UINT               m_BufferSize;

   // Objects
   WCHAR               m_szExePath[MAX_PATH];
   HINSTANCE            m_hInstance;
   HICON               m_hIcon;
   HWND               m_Window;
   UINT32            m_WindowWidth;
   UINT32            m_WindowHeight;
   std::wstring         m_WindowTitle;
   LPDIRECTINPUT8         m_DI8;         
   LPDIRECTINPUTDEVICE8   m_MouseDevice;
   LPDIRECTINPUTDEVICE8   m_KeyboardDevice;

   // State
   bool               m_Active;

   HRESULT InitializeWindow( void );
   HRESULT FinalizeWindow( void );
   HRESULT InitializeMouse( void );
   HRESULT InitializeKeyboard( void );
   HRESULT FinalizeMouse( void );
   HRESULT FinalizeKeyboard( void );

   void ActivateWin32Mouse( void );
   void DeactivateWin32Mouse( void );
   void ActivateMouse( void );
   void DeactivateMouse( void );

   static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
public:
   CDirectInput( CEngine* Engine );
   virtual ~CDirectInput( void );

   bool GetActive( void ) { return m_Active; }
   void SetActive( bool Active ) { m_Active = Active; }
   HWND GetWindow( void ) { return m_Window; }

   virtual bool Initialize( void );
   virtual void Finalize( void );

   virtual bool Start( void );

   virtual bool DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 );

   void Activate( void );
   void Deactivate( void );

   HRESULT ReadMouseData( void );
   HRESULT ReadKeyboardData( void );

};
}

#endif // DIRECTINPUT_H