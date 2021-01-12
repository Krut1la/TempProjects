// MWContext.cpp: implementation of the CMWContext class.
//
//////////////////////////////////////////////////////////////////////

#include "MWContext.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMWContext::CMWContext()
{

}

CMWContext::~CMWContext()
{

}

void CMWContext::OnServiceStart()
{
	MessageBox(NULL,L"Start",L"Simple service", MB_DEFAULT_DESKTOP_ONLY);
}

void CMWContext::OnServiceStop()
{
	MessageBox(NULL,L"Stop",L"Simple service",MB_DEFAULT_DESKTOP_ONLY);
}
