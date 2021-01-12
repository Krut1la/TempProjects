// ShellIcon.h: interface for the CShellIcon class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHELLICON_H__127EA057_6900_4F91_B775_9E560ECB5E6D__INCLUDED_)
#define AFX_SHELLICON_H__127EA057_6900_4F91_B775_9E560ECB5E6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ShellIconsAPI.h"
#include <ShellAPI.h>

class SHELLICONS_API CShellIcon
{
public:
	// constructor
	CShellIcon();

	// destructor
	virtual ~CShellIcon();

public:
	/////////////////
	// identification

	// window
	HWND getHostWindow() const;
	bool setHostWindow(HWND hWnd);	// return false if already set up

	// icon id
	UINT getTrayID() const;
	bool setTrayID(UINT uID);	// return false if already set up

	///////////////
	// notification

	// callback message
	UINT getCallbackMessage() const;
	void setCallbackMessage(UINT uMsg);

	//////////////////
	// icon properties

	// icon image
	HICON getIcon() const;
	void setIcon(HICON hIcon, BOOL bOwn = true);	// takes/releases the handle
	bool loadIcon(HINSTANCE hInstance, LPCTSTR sResourceID);
	bool loadIcon(HINSTANCE hInstance, WORD wResourceID);

	// tool tips
	LPCTSTR getToolTip() const;
	void setToolTip(LPCTSTR sText);

	///////////
	// commands
	bool addIcon();
	bool removeIcon();
	bool modifyIcon();
	bool update();

	bool isAdded() const;
	bool isIdentified() const;
	bool isReady() const;
	bool isModified() const;

	// automatic application
	bool getImmediate() const;
	void setImmediate(bool bImmediate);

protected:
	NOTIFYICONDATA m_nid;
	bool m_bOwnIcon;	// icon will be destroyed on change
	bool m_bImmediate;
	bool m_bAdded;
	void init();
};

#endif // !defined(AFX_SHELLICON_H__127EA057_6900_4F91_B775_9E560ECB5E6D__INCLUDED_)
