// MfcShellIconHost.cpp : implementation file
//

#include "stdafx.h"
#include "ExtShellIcon.h"

#ifndef _SHELLICONS_USE_STL
#include "MapImpl.h"
#endif

#include <assert.h>
#include <tchar.h>

const UINT WM_ShellIcon = WM_APP + 1;

/////////////////////////////////////////////////////////////////////////////
// CExtShellIcon

CExtShellIcon::CExtShellIcon(CExtShellIconHost* pHost, UINT nTrayID):
	m_pHost(pHost),
	m_pfnCallback(NULL),
	m_pCallbackItf(NULL)
{
	assert(pHost->getHWnd());
	CShellIcon::setTrayID(nTrayID);
	CShellIcon::setHostWindow(pHost->getHWnd());
	CShellIcon::setCallbackMessage(WM_ShellIcon);
}

CExtShellIcon::~CExtShellIcon()
{
}

LRESULT CExtShellIcon::Callback(UINT uMsg)
{
	if(m_pCallbackItf)
		return m_pCallbackItf->note(m_pHost, this, uMsg);
	if(m_pfnCallback)
		return (*m_pfnCallback)(m_pHost, this, uMsg);
	return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CExtShellIconHost

CExtShellIconHost::CExtShellIconHost()
{
#ifndef _SHELLICONS_USE_STL
	m_ptrMapIcons = new CShellIconsMap;
#endif
}

CExtShellIconHost::~CExtShellIconHost()
{
	removeAllIcons();
#ifndef _SHELLICONS_USE_STL
	delete m_ptrMapIcons;
	m_ptrMapIcons = NULL;
#endif
}

UINT CExtShellIconHost::getFreeTrayID() const
{
	UINT nFreeID = 0;
#ifdef _SHELLICONS_USE_STL
	mapIcons::const_iterator it = m_mapIcons.begin(), end = m_mapIcons.end();
	while(it != end && it->first <= nFreeID)
	{
		++nFreeID;
		++it;
	}
#else
	while(m_ptrMapIcons->take(nFreeID) != NULL)
		++nFreeID;
#endif
	return nFreeID;
}

CExtShellIcon* CExtShellIconHost::addIcon(UINT nID, _ShellIconNote* pItf /* = NULL */, PFNShellIconNote pfn /* = NULL */)
{
#ifdef _SHELLICONS_USE_STL
	if(m_mapIcons.find(nID) != m_mapIcons.end())
		return NULL;
#else
	if(m_ptrMapIcons->take(nID) != NULL)
		return NULL;
#endif

	CExtShellIcon* pIcon = new CExtShellIcon(this, nID);
	pIcon->setCallbackItf(pItf);
	pIcon->setCallbackFn(pfn);

#ifdef _SHELLICONS_USE_STL
	m_mapIcons[nID] = pIcon;
#else
	m_ptrMapIcons->add(nID, pIcon);
#endif
	return pIcon;
}

CExtShellIcon* CExtShellIconHost::getIcon(UINT nID) const
{
#ifdef _SHELLICONS_USE_STL
	mapIcons::const_iterator it = m_mapIcons.find(nID);
	return (it == m_mapIcons.end()) ? NULL : it->second;
#else
	return (CExtShellIcon*)m_ptrMapIcons->take(nID);
#endif
}

int CExtShellIconHost::countIcons() const
{
#ifdef _SHELLICONS_USE_STL
	return m_mapIcons.size();
#else
	return m_ptrMapIcons->count();
#endif
}

bool CExtShellIconHost::removeIcon(UINT nID)
{
#ifdef _SHELLICONS_USE_STL
	mapIcons::iterator it = m_mapIcons.find(nID);
	if(it == m_mapIcons.end())	return false;
	delete it->second;
	m_mapIcons.erase(it);
	return true;
#else
	CExtShellIcon* pIcon = (CExtShellIcon*)m_ptrMapIcons->drop(nID);
	if(!pIcon)	return false;
	delete pIcon;
	return true;
#endif
}

bool CExtShellIconHost::removeIcon(CExtShellIcon* pIcon)
{
	if(!pIcon)	return false;
#ifdef _SHELLICONS_USE_STL
	mapIcons::iterator it = m_mapIcons.find(pIcon->getTrayID());
	if(it == m_mapIcons.end())	return false;
	assert(it->second == pIcon);
	m_mapIcons.erase(it);
#else
	LPVOID pDropped = m_ptrMapIcons->drop(pIcon->getTrayID());
	if(!pDropped)	return false;
	assert(pDropped == pIcon);
#endif
	delete pIcon;
	return true;
}

int CExtShellIconHost::removeIcons(PFNShellIconNote pfn)
{
	int n = 0;
#ifdef _SHELLICONS_USE_STL
	mapIcons::iterator it = m_mapIcons.begin();
	while(it != m_mapIcons.end())
	{
		CExtShellIcon* pIcon = it->second;
		if(pIcon->getCallbackFn() != pfn)
			++it;
		else
		{
			mapIcons::iterator here = it;
			++it;
			delete pIcon;
			m_mapIcons.erase(it);
			++n;
		}
	}
#else
	_MapUintToPtr::_Enum* pEnum = m_ptrMapIcons->start();
	while(!pEnum->end())
	{
		CExtShellIcon* pIcon = (CExtShellIcon*)pEnum->value();
		if(pIcon->getCallbackFn() != pfn)
			pEnum->next();
		else
		{
			pEnum->remove();
			delete pIcon;
			++n;
		}
	}
	delete pEnum;
#endif
	return n;
}

int CExtShellIconHost::removeIcons(_ShellIconNote* pItf)
{
	int n = 0;

#ifdef _SHELLICONS_USE_STL
	mapIcons::iterator it = m_mapIcons.begin();
	while(it != m_mapIcons.end())
	{
		CExtShellIcon* pIcon = it->second;
		if(pIcon->getCallbackItf() != pItf)
			++it;
		else
		{
			mapIcons::iterator here = it;
			++it;
			delete pIcon;
			m_mapIcons.erase(it);
			++n;
		}
	}
#else
	_MapUintToPtr::_Enum* pEnum = m_ptrMapIcons->start();
	while(!pEnum->end())
	{
		CExtShellIcon* pIcon = (CExtShellIcon*)pEnum->value();
		if(pIcon->getCallbackItf() != pItf)
			pEnum->next();
		else
		{
			pEnum->remove();
			delete pIcon;
			++n;
		}
	}
	delete pEnum;
#endif
	return n;
}

void CExtShellIconHost::removeAllIcons()
{
#ifdef _SHELLICONS_USE_STL
	while(!m_mapIcons.empty())
	{
		mapIcons::iterator it = m_mapIcons.begin();
		delete it->second;
		m_mapIcons.erase(it);
	}
#else
	CExtShellIcon* pIcon = NULL;
	while(pIcon = (CExtShellIcon*)m_ptrMapIcons->drop())
		delete pIcon;
#endif
}


LRESULT CExtShellIconHost::notify(UINT nID, UINT uMsg)
{
	CExtShellIcon* pIcon = getIcon(nID);
	if(!pIcon)	return 0;

	return pIcon->Callback(uMsg);
}




/////////////////////////////////////////////////////////////////////////////
// CWinShellIconHost

CWinShellIconHost::CWinShellIconHost():
	m_hWnd(NULL)
{
}

CWinShellIconHost::~CWinShellIconHost()
{
}

void CWinShellIconHost::destroy()
{
	if(!m_hWnd)	return;
	::DestroyWindow(m_hWnd);
	m_hWnd = NULL;
}

bool CWinShellIconHost::CreateMe()
{
	// class
	ATOM sClass = RegisterMyClass();
	if(!sClass)	return false;

	// window
	HWND hWnd = CreateWindowEx(
		0,
		(LPCSTR)sClass,
		_T("WinShellIconHost"),
		WS_OVERLAPPED,
		0, 0, 50, 50,
		NULL, NULL,
		(HINSTANCE)NULL,
		this);
	if(!hWnd)
		return false;

	CWinShellIconHost* pThis;
//	assert(m_Hosts.Lookup(hWnd, pThis));
//	assert(this == pThis);
	pThis = (CWinShellIconHost*)GetWindowLong(hWnd, GWL_USERDATA);
	assert(pThis == this);
	return true;
}

ATOM CWinShellIconHost::RegisterMyClass()
{
	ATOM sClass = NULL;

	if(sClass)	return sClass;

	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));

	wc.lpfnWndProc = MyWindowProc;
	wc.hInstance = (HINSTANCE)NULL;
	wc.lpszClassName = _T("WinShellIconHost");
	wc.cbWndExtra = sizeof(CWinShellIconHost*);

	sClass = RegisterClass(&wc);
	return sClass;
}


void CWinShellIconHost::onDestroy()
{
	removeAllIcons();
}

LRESULT CALLBACK CWinShellIconHost::MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 1. find the window
	CWinShellIconHost* pHost;
//	if(!m_Hosts.Lookup(hWnd, pHost))
//		pHost = NULL;
	pHost = (CWinShellIconHost*)GetWindowLong(hWnd, GWL_USERDATA);

	switch(uMsg)
	{
	case WM_CREATE:
		assert(!pHost);
		{
			LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
			pHost = (CWinShellIconHost*)pCS->lpCreateParams;
			pHost->m_hWnd = hWnd;
//			m_Hosts[hWnd] = pHost;
			SetWindowLong(hWnd, GWL_USERDATA, (LONG)pHost);
		}
		break;
		
	case WM_DESTROY:
		assert(pHost);
		{
			pHost->onDestroy();
			pHost->m_hWnd = NULL;
//			m_Hosts.RemoveKey(hWnd);
		}
		break;
		
	case WM_ShellIcon:
		assert(pHost);
		{
			return pHost->notify((UINT)wParam, (UINT)lParam);
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

CWinShellIconHost::SINGLETON CWinShellIconHost::singleton;