#include "stdafx.h"
/*#ifdef _UNICODE
    #ifndef UNICODE
        #define UNICODE
    #endif
#else
    #ifdef UNICODE
        #define _UNICODE
    #endif 
#endif*/

#include "notifyiconex.h"

void CNotifyIconEx::Dispatch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CNotifyIcon::Dispatch(hWnd, msg, wParam, lParam);

    if(!(
        (m_hWnd == hWnd) && 
        (m_uRegisteredMessage == msg) &&
        (m_uID == wParam) )
      )
        return;

    switch(lParam)
    {
    case WM_MOUSEMOVE:      if(OnMouseMove)
                                OnMouseMove(hWnd, wParam);
                            break;
    case WM_LBUTTONDOWN:    if(OnLButtonDown)
                                OnLButtonDown(hWnd, wParam);
                            break;
    case WM_LBUTTONUP:      if(OnLButtonUp)
                                OnLButtonUp(hWnd, wParam);
                            break;
    case WM_LBUTTONDBLCLK:  if(OnLButtonDblclk)
                                OnLButtonDblclk(hWnd, wParam);
                            break;
    case WM_RBUTTONDOWN:    if(OnRButtonDown)
                                OnRButtonDown(hWnd, wParam);
                            break;
    case WM_RBUTTONDBLCLK:  if(OnRButtonDblclk)
                                OnRButtonDblclk(hWnd, wParam);
                            break;
    }
}
