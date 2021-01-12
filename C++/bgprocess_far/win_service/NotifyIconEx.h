////////////////////////////////////////////////////////////////////////
//  (MFC independent)
//  class CNotifyIconEx 
//  Version 1.07 (Sept 21 2001)
//  Igor_Vartanov, FIDO: 2:5025/16.26 AKA /38.70
//               e-mail: ivartanov@rsdn.ru
//
////////////////////////////////////////////////////////////////////////
#ifndef IV_NOTIFYICONEX_class_H
#define IV_NOTIFYICONEX_class_H

#include "notifyicon.h"

class CNotifyIconEx : public CNotifyIcon  
{
public:
    virtual void Dispatch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    CNotifyIconEx(
                         HWND      hWnd, 
                         UINT      nIDRes, 
                         UINT      uRegisteredMessage = 0,
                         HINSTANCE hInst = NULL,
                         LPCTSTR   szTip = NULL)
                         : CNotifyIcon(hWnd, nIDRes, 
                                uRegisteredMessage,
                                hInst, szTip) 
                            {
                                OnLButtonDown   =
                                OnLButtonUp     =
                                OnLButtonDblclk =
                                OnRButtonDown   =
                                OnRButtonDblclk =
                                OnMouseMove     = NULL;
                            };
    CNotifyIconEx(
                         HWND      hWnd, 
                         UINT      nID, 
                         HINSTANCE hInst,
                         LPCTSTR   szIconRes,
                         LPCTSTR   szTip     = NULL,
                         LPCTSTR   szMenuRes = NULL,
                         UINT      uRegisteredMessage = 0)
                         : CNotifyIcon(hWnd, nID, hInst,
                                szIconRes, szTip, szMenuRes,
                                uRegisteredMessage) 
                            {
                                OnLButtonDown   =
                                OnLButtonUp     =
                                OnLButtonDblclk =
                                OnRButtonDown   =
                                OnRButtonDblclk =
                                OnMouseMove     = NULL;
                            };
    CNotifyIconEx(
                         HWND      hWnd, 
                         UINT      nID, 
                         HINSTANCE hInst,
                         UINT      uIconRes,
                         LPCTSTR   szTip    = NULL,
                         UINT      uMenuRes = 0,
                         UINT      uRegisteredMessage = 0)
                         : CNotifyIcon(hWnd, nID, hInst,
                                uIconRes, szTip, uMenuRes,
                                uRegisteredMessage) 
                            {
                                OnLButtonDown   =
                                OnLButtonUp     =
                                OnLButtonDblclk =
                                OnRButtonDown   =
                                OnRButtonDblclk =
                                OnMouseMove     = NULL;
                            };

    MOUSE_HANDLER    OnLButtonDown;
    MOUSE_HANDLER    OnLButtonUp;
    MOUSE_HANDLER    OnLButtonDblclk;
    MOUSE_HANDLER    OnRButtonDown;
    MOUSE_HANDLER    OnRButtonDblclk;
    MOUSE_HANDLER    OnMouseMove;
};

#endif 
