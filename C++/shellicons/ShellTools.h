#ifndef __SHELL_TOOLS_H__
#define __SHELL_TOOLS_H__

#include "ShellIconsAPI.h"
#include "ShellIconNote.h"

enum MENU_INDEX { MENU_LEFT, MENU_RIGHT, MENU__COUNT };

class SHELLICONS_API CTrackMenu
{
public:
	CTrackMenu();
	virtual ~CTrackMenu();

	HWND   getHWnd() const;
	BOOL   setHWnd(HWND hWnd);

	HMENU  getMenu(MENU_INDEX mi) const;
	BOOL   setMenu(MENU_INDEX mi, HMENU hMenu, BOOL bOwn = true);
	BOOL  loadMenu(MENU_INDEX mi, HINSTANCE hInstance, LPCTSTR sMenuResource);
	BOOL  loadMenu(MENU_INDEX mi, HINSTANCE hInstance, WORD wMenuResource);
	BOOL trackMenu(MENU_INDEX mi, int x, int y);
	BOOL trackMenu(MENU_INDEX mi, const POINT* ppt);
	BOOL trackMenu(MENU_INDEX mi, const POINT& pt);
	BOOL trackMenu(MENU_INDEX mi);

protected:
	HWND m_hWnd;
	HMENU m_hMenu[MENU__COUNT];
	bool m_bOwn[MENU__COUNT];
	bool m_bTracking;
};

class SHELLICONS_API CHideWindow : protected CShellIconNote
{
public:
	CHideWindow();
	virtual ~CHideWindow();

	HWND getHWnd() const;
	BOOL setHWnd(HWND hWnd);

	void hide();
	bool update();
	void show();

protected:
	HWND m_hWnd;
	UINT m_nIcon;

//	CTrackMenu m_TrackMenu;

	BOOL createIcon();
	void removeIcon();

	// CShellIconNote override
	void onTrayRightButtonDown();
	void onTrayLeftButtonDblClk();

};

#endif//__SHELL_TOOLS_H__