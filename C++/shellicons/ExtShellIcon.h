#if !defined(AFX_MFCSHELLICONHOST_H__7C53BCC0_F10D_411C_BE74_000A3D1A09D6__INCLUDED_)
#define AFX_MFCSHELLICONHOST_H__7C53BCC0_F10D_411C_BE74_000A3D1A09D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MfcShellIconHost.h : header file
//

#include "ShellIconsAPI.h"
#include "ShellIcon.h"

#define _SHELLICONS_USE_STL

#ifdef _SHELLICONS_USE_STL
	#include <map>
#else
	#include "MapDecl.h"
#endif

class CExtShellIcon;
class CExtShellIconHost;


struct _ShellIconNote
{
	virtual LRESULT note(CExtShellIconHost* pHost, CExtShellIcon* pIcon, UINT uMsg) = 0;
};

typedef LRESULT (*PFNShellIconNote)(CExtShellIconHost* pHost, CExtShellIcon* pIcon, UINT uMsg);

class SHELLICONS_API CExtShellIcon: protected CShellIcon
{
protected:
	friend class CExtShellIconHost;

	CExtShellIcon(CExtShellIconHost* pHost, UINT nTrayID);
	virtual ~CExtShellIcon();

	LRESULT Callback(UINT uMsg);

public:
	inline CExtShellIconHost* getHost() const
		{ return m_pHost; }
	inline UINT getTrayID() const
		{ return CShellIcon::getTrayID(); }
	
	inline PFNShellIconNote getCallbackFn() const
		{ return m_pfnCallback; }
	inline void setCallbackFn(PFNShellIconNote pfn)
		{ m_pfnCallback = pfn; }

	inline _ShellIconNote* getCallbackItf() const
		{ return m_pCallbackItf; }
	inline void setCallbackItf(_ShellIconNote* pItf)
		{ m_pCallbackItf = pItf; }

	inline HICON getIcon() const
		{ return CShellIcon::getIcon(); }
	inline void setIcon(HICON hIcon, BOOL bOwn = true)
		{ CShellIcon::setIcon(hIcon, bOwn); }
	inline bool loadIcon(HINSTANCE hInstance, LPCTSTR sResourceID)
		{ return CShellIcon::loadIcon(hInstance, sResourceID); }
	inline bool loadIcon(HINSTANCE hInstance, WORD wResourceID)
		{ return CShellIcon::loadIcon(hInstance, wResourceID); }

	inline LPCTSTR getToolTip() const
		{ return CShellIcon::getToolTip(); }
	inline void setToolTip(LPCTSTR sText)
		{ CShellIcon::setToolTip(sText); }

	inline bool show(bool bShow)
		{ return bShow ? CShellIcon::addIcon() : CShellIcon::removeIcon(); }
	inline bool update()
		{ return CShellIcon::update(); }
	inline bool isReady() const
		{ return CShellIcon::isReady(); }
	inline bool isShown() const
		{ return CShellIcon::isAdded(); }

	bool getImmediate() const
		{ return CShellIcon::getImmediate(); }
	void setImmediate(bool bImmediate)
		{ CShellIcon::setImmediate(bImmediate); }

private:
	CExtShellIconHost* m_pHost;
	PFNShellIconNote m_pfnCallback;
	_ShellIconNote* m_pCallbackItf;
};

class SHELLICONS_API CExtShellIconHost
{
public:
	CExtShellIconHost();
	virtual ~CExtShellIconHost();

	virtual HWND getHWnd() const = 0;
	virtual void destroy() = 0;

	////////
	// icons
	UINT getFreeTrayID() const;

	CExtShellIcon* addIcon(UINT nID, _ShellIconNote* pItf = NULL, PFNShellIconNote pfn = NULL);

	CExtShellIcon* getIcon(UINT nID) const;

	int countIcons() const;

	bool removeIcon(UINT nID);
	bool removeIcon(CExtShellIcon* pIcon);
	int removeIcons(PFNShellIconNote pfn);
	int removeIcons(_ShellIconNote* pItf);
	void removeAllIcons();

protected:
#ifdef _SHELLICONS_USE_STL
	typedef std::map<UINT, CExtShellIcon*> mapIcons;
	mapIcons m_mapIcons;
#else
	_MapUintToPtr* m_ptrMapIcons; // interface to a simple collection
#endif

	LRESULT notify(UINT nID, UINT uMsg);
};

class SHELLICONS_API CWinShellIconHost: public CExtShellIconHost
{
public:
	CWinShellIconHost();
	virtual ~CWinShellIconHost();

	virtual HWND getHWnd() const { return m_hWnd; }
	virtual void destroy();

	bool CreateMe();

protected:
	HWND m_hWnd;

	static ATOM RegisterMyClass();
	static LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void onDestroy();

	// singleton
protected:
	struct SINGLETON
	{
		CWinShellIconHost* pGlobal;
		SINGLETON(): pGlobal(NULL) {}
		~SINGLETON()
		{
			destroy();
		}
		void destroy()
		{
			if(!pGlobal)	return;
			pGlobal->destroy();
			delete pGlobal;
		}
		CWinShellIconHost* make(bool bCreate)
		{
			if(pGlobal || !bCreate)	return pGlobal;
			pGlobal = new CWinShellIconHost;
			if(!pGlobal->CreateMe())
				destroy();
			return pGlobal;
		}
	};
	static SINGLETON singleton;
public:
	static CWinShellIconHost* Host(bool bCreate = true) { return singleton.make(bCreate); }
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCSHELLICONHOST_H__7C53BCC0_F10D_411C_BE74_000A3D1A09D6__INCLUDED_)
