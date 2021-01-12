/*#ifdef _UNICODE
    #ifndef UNICODE
        #define UNICODE
    #endif
#else
    #ifdef UNICODE
        #define _UNICODE
    #endif 
#endif*/

#include "stdafx.h"
#include "notifyicon.h"

#pragma comment(lib,"user32")

static const int MAX_TIPTEXT = 256;

//////////////////// CNotifyIcon class implementation //////////////////
////////////////////////////////////////////////////////////////////////
// Constructors

CNotifyIcon::CNotifyIcon( HWND hWnd ) :
    m_hWnd(hWnd), m_uID(0), m_hInst(NULL),
    m_uRegisteredMessage(DefMessageID()),
    m_lpszMenu(NULL),
    m_hIcon(NULL), m_lpszTip(NULL),
    AdjustNIContextMenu(0),
    OnRButtonUp(0), m_bAdded(FALSE)
{
}

CNotifyIcon::CNotifyIcon(
                         HWND      hWnd, 
                         UINT      nIDRes,
                         UINT      uRegisteredMessage,
                         HINSTANCE hInst,
                         LPCTSTR   szTip):
    m_hWnd(hWnd), m_uID(nIDRes), m_hInst(NULL),
    m_uRegisteredMessage(uRegisteredMessage ?
            uRegisteredMessage : 
            DefMessageID()),
    m_lpszMenu(NULL),
    m_hIcon(NULL), m_lpszTip(NULL),
    AdjustNIContextMenu(0),
    OnRButtonUp(0)
{
    TCHAR tip[MAX_TIPTEXT] = {0};
    
    SetMenu(hInst, MAKEINTRESOURCE(nIDRes));
    if(!szTip)
    {
        if(::LoadString(m_hInst, nIDRes, tip, MAX_TIPTEXT))
			StoreTip(szTip = tip);
	}
    m_bAdded = Add(m_hInst, MAKEINTRESOURCE(nIDRes), szTip);
}

CNotifyIcon::CNotifyIcon(
                         HWND      hWnd, 
                         UINT      nID, 
                         HINSTANCE hInst,
                         LPCTSTR   szIconRes,
                         LPCTSTR   szTip,
                         LPCTSTR   szMenuRes,
                         UINT      uRegisteredMessage):
    m_hWnd(hWnd), m_uID(nID), m_hInst(NULL),
    m_uRegisteredMessage(uRegisteredMessage ?
            uRegisteredMessage : 
            DefMessageID()),
    m_lpszMenu(NULL),
    m_hIcon(NULL), m_lpszTip(NULL),
    AdjustNIContextMenu(0),
    OnRButtonUp(0)
{
    if(szMenuRes)
        SetMenu(hInst, szMenuRes);
	if(szTip)
		StoreTip(szTip);
    m_bAdded = Add(hInst, szIconRes, szTip);
}

CNotifyIcon::CNotifyIcon(
                         HWND      hWnd, 
                         UINT      nID, 
                         HINSTANCE hInst,
                         UINT      uIconRes,
                         LPCTSTR   szTip,
                         UINT      uMenuRes,
                         UINT      uRegisteredMessage):
    m_hWnd(hWnd), m_uID(nID), m_hInst(NULL),
    m_uRegisteredMessage(uRegisteredMessage ?
            uRegisteredMessage : 
            DefMessageID()),
    m_lpszMenu(NULL),
    m_hIcon(NULL), m_lpszTip(NULL),
    AdjustNIContextMenu(0),
    OnRButtonUp(0)
{
    if(uMenuRes)
        SetMenu(hInst, MAKEINTRESOURCE(uMenuRes));
	if(szTip)
		StoreTip(szTip);
    m_bAdded = Add(hInst, MAKEINTRESOURCE(uIconRes), szTip);
}

////////////////////////////////////////////////////////////////////////
// Destructor (virtual)

CNotifyIcon::~CNotifyIcon()
{
    if(m_hIcon)
        ::DestroyIcon(m_hIcon);
    if(m_lpszTip)
        delete [] m_lpszTip;
    Remove();
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::Add

BOOL CNotifyIcon::Add(HICON hIcon, LPCTSTR szTip)  // private method
{
    if(!::IsWindow(m_hWnd))
        return FALSE;
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize           = sizeof(NOTIFYICONDATA);
    nid.hWnd             = m_hWnd;
    nid.uID              = m_uID;
    nid.uFlags           = NIF_MESSAGE | (hIcon ? NIF_ICON : 0);
    nid.uCallbackMessage = m_uRegisteredMessage;
    nid.hIcon            = hIcon;
    if(szTip)
    {
        lstrcpyn(nid.szTip, szTip, 64);
        nid.uFlags |= NIF_TIP;
        StoreTip(szTip);
    }
    return ( m_bAdded = ::Shell_NotifyIcon( NIM_ADD, &nid ) );
}

BOOL CNotifyIcon::Add(HINSTANCE hInst, LPCTSTR szIconRes, LPCTSTR szTip)
{
    if(!::IsWindow(m_hWnd))
        return FALSE;
    if(m_hIcon)
        ::DestroyIcon(m_hIcon);
    if(!(m_hIcon = ::LoadIcon(hInst, szIconRes)))
        return FALSE;
    if(szTip)
        StoreTip(szTip);
    return (m_bAdded = Add(m_hIcon, szTip ? szTip : m_lpszTip ));
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::Remove

BOOL CNotifyIcon::Remove()
{
    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hWnd;
    nid.uID    = m_uID;
    return !(m_bAdded = !::Shell_NotifyIcon(NIM_DELETE, &nid));
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::Restore

BOOL CNotifyIcon::Restore()
{
    if(!m_hIcon)
        return FALSE;
    return Add(m_hIcon, m_lpszTip);
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::Modify

BOOL CNotifyIcon::Modify(HICON hIcon, LPCTSTR szTip)  // private method
{
	if( !m_bAdded )
		return Add( hIcon, szTip );

    NOTIFYICONDATA nid;
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hWnd;
    nid.uID    = m_uID;
    nid.uFlags = hIcon ? NIF_ICON : 0;
    nid.hIcon  = hIcon;
    if(szTip)
    {
        lstrcpyn(nid.szTip, szTip, 64);
        nid.uFlags |= NIF_TIP;
        StoreTip(szTip);
    }
    return m_bAdded = ::Shell_NotifyIcon( NIM_MODIFY, &nid);
}

BOOL CNotifyIcon::Modify(HINSTANCE hInst, LPCTSTR szIconRes, LPCTSTR szTip)
{
    if(m_hIcon)
        ::DestroyIcon(m_hIcon);
    if(!(m_hIcon = ::LoadIcon(hInst, szIconRes)))
        return FALSE;
    if(szTip)
        StoreTip(szTip);
    return Modify(m_hIcon, szTip ? szTip : m_lpszTip );
}

BOOL CNotifyIcon::Modify(HINSTANCE hInst, UINT uIconRes, LPCTSTR szTip)
{
    return Modify(hInst, MAKEINTRESOURCE(uIconRes), szTip);
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::SetMenu

void CNotifyIcon::SetMenu(HINSTANCE hInst, LPCTSTR lpszMenuResource)
{
    m_hInst    = hInst ? hInst : ::GetModuleHandle(NULL);
    m_lpszMenu = lpszMenuResource;
}

void CNotifyIcon::SetMenu(HINSTANCE hInst, UINT uMenuResource)
{
    m_hInst    = hInst ? hInst : ::GetModuleHandle(NULL);
    m_lpszMenu = MAKEINTRESOURCE(uMenuResource);	
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::StoreTip

void CNotifyIcon::StoreTip(LPCTSTR szTip)  // private method
{
    if(!m_lpszTip)
    {
        m_lpszTip = new TCHAR[MAX_TIPTEXT];
        if(!m_lpszTip)
            return; // subject to change : exception should be thrown
                    // since "new" have not done it
    }
    lstrcpyn(m_lpszTip, szTip, MAX_TIPTEXT);
}

BOOL CNotifyIcon::SetTip(LPCTSTR szTip)
{
    return Modify(NULL, szTip);
}

BOOL CNotifyIcon::SetTip(HINSTANCE hInst, UINT nTipRes)
{
    TCHAR szTip[MAX_TIPTEXT];
    if(::LoadString(hInst, nTipRes, szTip, MAX_TIPTEXT))
        return SetTip(szTip);
    return FALSE;
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::IsAdded

BOOL CNotifyIcon::IsAdded()
{
    return m_bAdded;
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::OnNIContextMenu
// default MOUSE_HANDLER for OnRButtonUp event

void CNotifyIcon::OnNIContextMenu(HWND hWnd, UINT uID)  // private method
{
    POINT p;
    int   nSubMenu = 0;
    if(!m_lpszMenu)
        return;
    HINSTANCE hInst = m_hInst;
    if(!hInst)
        hInst = (HINSTANCE) ::GetModuleHandle(NULL);
    GetCursorPos(&p);
    HMENU hPopup = ::LoadMenu(hInst, m_lpszMenu);
    if(!hPopup)
        return;
    UINT  uFlags = TPM_BOTTOMALIGN | TPM_CENTERALIGN;

    // если зарегистрирована функция настройки меню, то она
    // будет вызвана

    if(AdjustNIContextMenu)
        if(!AdjustNIContextMenu(hWnd, uID, hPopup, uFlags, nSubMenu))
            return;

    HMENU hSub = ::GetSubMenu(hPopup, nSubMenu);
    if(hSub) 
    {
        ::SetForegroundWindow(hWnd);
        ::TrackPopupMenuEx(hSub, uFlags, p.x, p.y, hWnd, NULL);
        ::PostMessage(hWnd, WM_NULL, 0, 0);
    }
    ::DestroyMenu(hPopup);
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::DefMessageID

UINT CNotifyIcon::DefMessageID()
{
    return ::RegisterWindowMessage(_T("CNotifyIcon_regMsg"));
}

////////////////////////////////////////////////////////////////////////
// CNotifyIcon::Dispatch (virtual)

void CNotifyIcon::Dispatch(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static const UINT uTaskbarCreatedMessageId = 
        ::RegisterWindowMessage(_T("TaskbarCreated"));

    if(uTaskbarCreatedMessageId == msg)
    {
        if(m_bAdded)
            Restore();
        return;
    }
    
    if(!( 
        (m_hWnd == hWnd) && 
        (m_uRegisteredMessage == msg) &&
        (m_uID == wParam) && 
        (WM_RBUTTONUP == lParam) )
      )
        return;
    
    if(OnRButtonUp)
        OnRButtonUp(hWnd, wParam);
    else
        OnNIContextMenu(hWnd, wParam);
}

