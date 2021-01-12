// ShellIconNote.cpp: implementation of the CShellIconNote class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ShellIconNote.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellIconNote::CShellIconNote()
{

}

CShellIconNote::~CShellIconNote()
{

}

LRESULT CShellIconNote::note(CExtShellIconHost* pHost, CExtShellIcon* pIcon, UINT uMsg)
{
	switch(uMsg)
	{
	case WM_MOUSEMOVE:		onTrayMouseMove			(pHost, pIcon);	return 0;
	case WM_LBUTTONDOWN:	onTrayLeftButtonDown	(pHost, pIcon);	return 0;
	case WM_LBUTTONUP:		onTrayLeftButtonUp		(pHost, pIcon);	return 0;
	case WM_LBUTTONDBLCLK:	onTrayLeftButtonDblClk	(pHost, pIcon);	return 0;
	case WM_RBUTTONDOWN:	onTrayRightButtonDown	(pHost, pIcon);	return 0;
	case WM_RBUTTONUP:		onTrayRightButtonUp		(pHost, pIcon);	return 0;
	case WM_RBUTTONDBLCLK:	onTrayRightButtonDblClk	(pHost, pIcon);	return 0;
	default:	return -1;
	}
}

#define onTrayIMP(event)	\
	void CShellIconNote::onTray##event(CExtShellIconHost* pHost, CExtShellIcon* pIcon)	\
	{ onTray##event(pIcon->getTrayID()); }	\
	void CShellIconNote::onTray##event(UINT nID)	\
	{ onTray##event(); }	\
	void CShellIconNote::onTray##event()	\
	{}

onTrayIMP(MouseMove)
onTrayIMP(LeftButtonDown)
onTrayIMP(LeftButtonUp)
onTrayIMP(LeftButtonDblClk)
//onTrayIMP(RightButtonDown)
onTrayIMP(RightButtonUp)
onTrayIMP(RightButtonDblClk)

void CShellIconNote::onTrayRightButtonDown(CExtShellIconHost* pHost, CExtShellIcon* pIcon)	\
{ onTrayRightButtonDown(pIcon->getTrayID()); }	\
void CShellIconNote::onTrayRightButtonDown(UINT nID)	\
{ onTrayRightButtonDown(); }	\
void CShellIconNote::onTrayRightButtonDown()	\
{}
