#include "stdafx.h"

#include "ShellTools.h"

CTrackMenu::CTrackMenu()
{
	m_hWnd = NULL;
	memset(m_hMenu, 0, sizeof(m_hMenu));
	memset(m_bOwn, 0, sizeof(m_bOwn));
	m_bTracking = false;
}

CTrackMenu::~CTrackMenu()
{
	for(int i = 0; i < MENU__COUNT; i++)
		setMenu((MENU_INDEX)i, NULL);
}

HWND CTrackMenu::getHWnd() const
{
	return m_hWnd;
}

BOOL CTrackMenu::setHWnd(HWND hWnd)
{
	if(m_hWnd == hWnd)
		return true;

	if(hWnd && !IsWindow(hWnd))
		return SetLastError(ERROR_INVALID_WINDOW_HANDLE), false;

	m_hWnd = hWnd;
	return true;
}

HMENU CTrackMenu::getMenu(MENU_INDEX mi) const
{
	if(mi < 0 || mi >= MENU__COUNT)
		return NULL;

	return m_hMenu[mi];
}

BOOL CTrackMenu::setMenu(MENU_INDEX mi, HMENU hMenu, BOOL bOwn)
{
	if(mi < 0 || mi >= MENU__COUNT)
		return SetLastError(E_INVALIDARG), false;

	if(hMenu && !IsMenu(hMenu))
		return SetLastError(ERROR_INVALID_MENU_HANDLE), false;
	
	if(m_hMenu[mi] == hMenu)
		return true;

	if(m_bOwn[mi] && m_hMenu[mi])
		DestroyMenu(m_hMenu[mi]);	// ignore last error

	m_hMenu[mi] = hMenu;
	m_bOwn[mi] = bOwn;
	return true;
}

BOOL CTrackMenu::loadMenu(MENU_INDEX mi, HINSTANCE hInstance, LPCTSTR sMenuResource)
{
	if(mi < 0 || mi >= MENU__COUNT)
		return SetLastError(E_INVALIDARG), false;

	HMENU hMenuBar = LoadMenu(hInstance, sMenuResource);
	if(!hMenuBar)
		return false;	// last error is set

	HMENU hMenu = GetSubMenu(hMenuBar, 0);
	if(!hMenu)
		return SetLastError(E_FAIL), false;

	if(!RemoveMenu(hMenuBar, 0, MF_BYPOSITION))
		return false;	// last error is set

	if(!DestroyMenu(hMenuBar))
		return false;	// last error is set

	return setMenu(mi, hMenu);
}

BOOL CTrackMenu::loadMenu(MENU_INDEX mi, HINSTANCE hInstance, WORD wMenuResource)
{
	return loadMenu(mi, hInstance, MAKEINTRESOURCE(wMenuResource));
}

BOOL CTrackMenu::trackMenu(MENU_INDEX mi, int x, int y)
{
	if(mi < 0 || mi >= MENU__COUNT)
		return SetLastError(E_INVALIDARG), false;

	if(!m_hMenu[mi] || !m_hWnd)
		return SetLastError(S_FALSE), false;

	SetForegroundWindow(m_hWnd);	// ignore last error
	return TrackPopupMenu(m_hMenu[mi], 0, x, y, 0, m_hWnd, NULL);
}

BOOL CTrackMenu::trackMenu(MENU_INDEX mi, const POINT* ppt)
{
	if(!ppt)
		return SetLastError(E_POINTER), false;

	return trackMenu(mi, ppt->x, ppt->y);
}

BOOL CTrackMenu::trackMenu(MENU_INDEX mi, const POINT& pt)
{
	return trackMenu(mi, pt.x, pt.y);
}

BOOL CTrackMenu::trackMenu(MENU_INDEX mi)
{
	POINT pt;
	if(!GetCursorPos(&pt))
		return false;	// last error is set

	return trackMenu(mi, pt);
}





/////////////////////////////////////////////////////////

CHideWindow::CHideWindow()
{
	m_hWnd = NULL;
	m_nIcon = ~0;
}

CHideWindow::~CHideWindow()
{
	setHWnd(NULL);
}

HWND CHideWindow::getHWnd() const
{
	return m_hWnd;
}

BOOL CHideWindow::setHWnd(HWND hWnd)
{
	if(m_hWnd == hWnd)	return true;

	if(!IsWindow(hWnd))
		return SetLastError(ERROR_INVALID_WINDOW_HANDLE), false;

	if(!hWnd)
		removeIcon();

	m_hWnd = hWnd;
}

void CHideWindow::hide()
{
	if(!m_hWnd)
		return;

	ShowWindow(m_hWnd, SW_MINIMIZE);

	if(createIcon())
		ShowWindow(m_hWnd, SW_HIDE);
}

void CHideWindow::show()
{
	removeIcon();
	ShowWindow(m_hWnd, SW_SHOW);
	ShowWindow(m_hWnd, SW_RESTORE);
}

bool CHideWindow::update()
{
	CExtShellIconHost* pHost = CWinShellIconHost::Host(false);
	if(!pHost)
		return SetLastError(E_FAIL), false;

	CExtShellIcon* pIcon = pHost->getIcon(m_nIcon);
	if(!pIcon)
		return SetLastError(E_FAIL), false;

	////////////////////////////////
	// get information on the window

	// icon
	HICON hIcon	= (HICON)SendMessage(m_hWnd, WM_GETICON, ICON_SMALL, 0);
	if(!hIcon)
		hIcon	= (HICON)SendMessage(m_hWnd, WM_GETICON, ICON_BIG, 0);

	// text
	const nTitleSize = 64; //sizeof(NOTIFYICONDATA::szTip) / sizeof(NOTIFYICONDATA::szTip[0]);
	TCHAR szTitle[nTitleSize + 3];

	int nTitle = GetWindowText(m_hWnd, szTitle, nTitleSize + 3);
	if(nTitle > nTitleSize)
		lstrcpy(szTitle + nTitleSize - 4, TEXT("..."));

	pIcon->setIcon(hIcon, false);
	pIcon->setToolTip(szTitle);
	pIcon->show(true);
	return pIcon->isShown();
}

BOOL CHideWindow::createIcon()
{
	if(!m_hWnd)
		return SetLastError(S_FALSE), false;

	CExtShellIconHost* pHost = CWinShellIconHost::Host();
	if(!pHost)
		return false;

	// allocate an ID
	m_nIcon = pHost->getFreeTrayID();

	CExtShellIcon* pIcon = pHost->addIcon(m_nIcon, this);
	if(!pIcon)
		return false;

	if(!update())
		return false;

//	// take the system menu
//	m_TrackMenu.setHWnd(m_hWnd);
//	HMENU hSysMenu = GetSystemMenu(m_hWnd, false);
//	m_TrackMenu.setMenu(MENU_RIGHT, hSysMenu);

	return true;
}

void CHideWindow::removeIcon()
{
	CExtShellIconHost* pHost = CWinShellIconHost::Host(false);
	if(!pHost)
		return;

	CExtShellIcon* pIcon = pHost->getIcon(m_nIcon);
	if(!pIcon)
		return;

	pIcon->setIcon(NULL, false);	// do not destroy the class icon
	pHost->removeIcon(pIcon);

	m_nIcon = ~0;
}

void CHideWindow::onTrayRightButtonDown()
{
//	// track system menu
//	m_TrackMenu.trackMenu(MENU_RIGHT);
}

void CHideWindow::onTrayLeftButtonDblClk()
{
	// restore
	show();
}
