// ServiceContext.h: interface for the CServiceContext class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SERVICECONTEXT_H
#define SERVICECONTEXT_H

#include "WinService.h"
#include "IOCP.h"

class CServiceContext : public CIOCP,
	public SERVICE_STATUS
{

	//Public enum
public:
	enum{
		CK_SERVICECONTROL
	};

	//Events
public:
	virtual void OnServicePause();
	virtual void OnUserEvent(DWORD dwEventCode,DWORD dwSubCode);
	virtual void OnParamChange();
	virtual void OnServiceStop();
	virtual void OnServiceStart();
	
	//Public functions
public:
	BOOL RegisterService();

	virtual DWORD ProcessControlCode(DWORD dwCode);

	BOOL ReportState();
	
	BOOL ReportStatus()
	{
		return SetServiceStatus(m_hss,this);
	};
	
	BOOL SetState(DWORD dwState,DWORD dwWait);
	
	void AcceptControls(DWORD dwFlags,bool fAccept = TRUE){
		if (fAccept)
			dwControlsAccepted |= dwFlags;
		else
			dwControlsAccepted &= ~dwFlags;
		
		if (dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE)
			fAllowPauseCont = fAccept;

		if (dwControlsAccepted & SERVICE_ACCEPT_PARAMCHANGE)
			fAllowParamChange = fAccept;
	};

	CServiceContext();
	virtual ~CServiceContext();


	//Public attributes
public:
	bool fInteractWithDesktop;
	bool fOwnProcess;
	LPTSTR m_szServiceName;

	//Private functions
private:

	//Private attributes
private:
	SERVICE_STATUS_HANDLE m_hss;
	CRITICAL_SECTION m_cs;
	bool fAllowPauseCont,fAllowParamChange;
};

#endif // SERVICECONTEXT_H
