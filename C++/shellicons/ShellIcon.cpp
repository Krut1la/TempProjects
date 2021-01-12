// ShellIcon.cpp: implementation of the CShellIcon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellIcon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const UINT INVALID_TRAY_ID = (UINT)(-1);

CShellIcon::CShellIcon()
{
	init();
}

CShellIcon::~CShellIcon()
{
	setTrayID(0);	// remove the window
	setIcon(NULL);	// release the icon
	init();
}

/////////////////
// identification

// window
HWND CShellIcon::getHostWindow() const
{
	return m_nid.hWnd;
}

bool CShellIcon::setHostWindow(HWND hWnd)
{
	if(hWnd == m_nid.hWnd)
		return true;	// nothing to do

	removeIcon();
	m_nid.hWnd = hWnd;
	if(!hWnd)
		return true;

	return update();
}

// icon id
UINT CShellIcon::getTrayID() const
{
	return m_nid.uID;
}

bool CShellIcon::setTrayID(UINT uID)
{
	if(uID == m_nid.uID)
		return true;

	removeIcon();
	m_nid.uID = uID;
	if(!uID)
		return true;

	return m_bImmediate ? update() : true;
}

///////////////
// notification

// callback message
UINT CShellIcon::getCallbackMessage() const
{
	return m_nid.uCallbackMessage;
}

void CShellIcon::setCallbackMessage(UINT uMsg)
{
	if(uMsg == m_nid.uCallbackMessage)
		return;
	
	m_nid.uCallbackMessage = uMsg;
	m_nid.uFlags |= NIF_MESSAGE;

	if(m_bImmediate)	update();
}

//////////////////
// icon properties

// icon image
HICON CShellIcon::getIcon() const
{
	return m_nid.hIcon;
}

void CShellIcon::setIcon(HICON hIcon, BOOL bOwn)
{
	if(hIcon == INVALID_HANDLE_VALUE)
		hIcon = NULL;

	if(hIcon == m_nid.hIcon)
		return;

	// release previous handle
	if(m_nid.hIcon && m_bOwnIcon)
		DestroyIcon(m_nid.hIcon);
	
	m_nid.hIcon = hIcon;
	m_nid.uFlags |= NIF_ICON;
	m_bOwnIcon = bOwn;

	if(m_bImmediate)	update();
}

bool CShellIcon::loadIcon(HINSTANCE hInstance, LPCTSTR sResourceID)
{
	HICON hIcon = ::LoadIcon(hInstance, sResourceID);
	if(!hIcon)
		return false;
	setIcon(hIcon);
	return true;
}

bool CShellIcon::loadIcon(HINSTANCE hInstance, WORD wResourceID)
{
	return loadIcon(hInstance, MAKEINTRESOURCE(wResourceID));
}

// tool tips
LPCTSTR CShellIcon::getToolTip() const
{
	return m_nid.szTip;
}

#define countof(array) (sizeof(array)/sizeof(array[0]))

void CShellIcon::setToolTip(LPCTSTR sText)
{
	if(!sText)
		m_nid.szTip[0] = 0;
	else
		lstrcpyn(m_nid.szTip, sText, countof(m_nid.szTip)-1);
	m_nid.uFlags |= NIF_TIP;

	if(m_bImmediate)	update();
}

///////////
// commands
bool CShellIcon::addIcon()
{
	if(m_bAdded)	// repeated call
		return false;

	if(!isReady())
		return false;

	if(!(m_nid.uFlags & NIF_ICON))
		return false;
	
	if(!Shell_NotifyIcon(NIM_ADD, &m_nid))
		return false;

	m_bAdded = true;
	return true;
}

bool CShellIcon::removeIcon()
{
	if(!m_bAdded)	// repeated call
		return false;

	if(!isIdentified())
		return false;

	if(!Shell_NotifyIcon(NIM_DELETE, &m_nid))
		return false;

	m_bAdded = false;
	return true;
}

bool CShellIcon::modifyIcon()
{
	if(!m_bAdded)
		return false;

	if(!isIdentified())
		return false;

	if(!isModified())	// nothing to do
		return true;

	if(!Shell_NotifyIcon(NIM_MODIFY, &m_nid))
		return false;

	m_nid.uFlags = 0;
	return true;
}

bool CShellIcon::update()
{
	if(!isAdded())
		return addIcon();

	if(!isReady())
		return removeIcon();

	if(isModified())
		return modifyIcon();

	return false;	// nothing to do
}

bool CShellIcon::isAdded() const
{
	return m_bAdded;
}

bool CShellIcon::isIdentified() const
{
	return m_nid.hWnd && m_nid.uID != INVALID_TRAY_ID;
}

bool CShellIcon::isReady() const
{
	return isIdentified() && m_nid.hIcon;
}

bool CShellIcon::isModified() const
{
	return m_nid.uFlags != 0;
}

// automatic modification
bool CShellIcon::getImmediate() const
{
	return m_bImmediate;
}

void CShellIcon::setImmediate(bool bImmediate)
{
	m_bImmediate = bImmediate;

	if(m_bImmediate)	update();
}

void CShellIcon::init()
{
	m_bOwnIcon = true;
	m_bImmediate = true;
	m_bAdded = false;

	memset(&m_nid, 0, sizeof(m_nid));
	m_nid.cbSize = sizeof(m_nid);
	m_nid.uID = INVALID_TRAY_ID;
}
