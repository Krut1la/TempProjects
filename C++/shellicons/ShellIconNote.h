// ShellIconNote.h: interface for the CShellIconNote class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHELLICONNOTE_H__38B6F9C4_D590_4328_B3F2_45D0929B83AD__INCLUDED_)
#define AFX_SHELLICONNOTE_H__38B6F9C4_D590_4328_B3F2_45D0929B83AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ShellIconsAPI.h"
#include "ExtShellIcon.h"

class SHELLICONS_API CShellIconNote : public _ShellIconNote  
{
public:
	CShellIconNote();
	virtual ~CShellIconNote();

	virtual LRESULT note(CExtShellIconHost* pHost, CExtShellIcon* pIcon, UINT uMsg);

	// extended layer
	virtual void onTrayMouseMove		(CExtShellIconHost* pHost, CExtShellIcon* pIcon);
	virtual void onTrayLeftButtonDown	(CExtShellIconHost* pHost, CExtShellIcon* pIcon);
	virtual void onTrayLeftButtonUp		(CExtShellIconHost* pHost, CExtShellIcon* pIcon);
	virtual void onTrayLeftButtonDblClk	(CExtShellIconHost* pHost, CExtShellIcon* pIcon);
	virtual void onTrayRightButtonDown	(CExtShellIconHost* pHost, CExtShellIcon* pIcon);
	virtual void onTrayRightButtonUp	(CExtShellIconHost* pHost, CExtShellIcon* pIcon);
	virtual void onTrayRightButtonDblClk(CExtShellIconHost* pHost, CExtShellIcon* pIcon);

	// simplified layer
	virtual void onTrayMouseMove		(UINT nID);
	virtual void onTrayLeftButtonDown	(UINT nID);
	virtual void onTrayLeftButtonUp		(UINT nID);
	virtual void onTrayLeftButtonDblClk	(UINT nID);
	virtual void onTrayRightButtonDown	(UINT nID);
	virtual void onTrayRightButtonUp	(UINT nID);
	virtual void onTrayRightButtonDblClk(UINT nID);

	// most simpe level
	virtual void onTrayMouseMove		();
	virtual void onTrayLeftButtonDown	();
	virtual void onTrayLeftButtonUp		();
	virtual void onTrayLeftButtonDblClk	();
	virtual void onTrayRightButtonDown	();
	virtual void onTrayRightButtonUp	();
	virtual void onTrayRightButtonDblClk();
};

#endif // !defined(AFX_SHELLICONNOTE_H__38B6F9C4_D590_4328_B3F2_45D0929B83AD__INCLUDED_)
