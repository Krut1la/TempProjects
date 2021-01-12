#ifndef SERVICEHELPER_H
#define SERVICEHELPER_H

#include "WinService.h"
#include "ServiceContext.h"	// Added by ClassView


typedef void (__stdcall *PSERVICEMAIN)(DWORD,LPTSTR*);

class CServiceHelper
{
	//public enum
public:
	enum {
		NOSERVICE,
		ONESERVICE,
		MULTIPLESERVICE
	};
	
	friend CServiceContext;

	//public functions
public:
	
	bool SetServiceCtx(CServiceContext* pCtx)
	{
		bool fOk = true;
		m_servctx = pCtx;
		if (AddService(pCtx->m_szServiceName,ServiceMain))
			m_ServiceCnt = ONESERVICE;
		else
			fOk = false;
		return fOk;
	};
	
	BOOL Start();
	
	bool AddService(LPTSTR szServiceName, PSERVICEMAIN ServMainProc);
	
	CServiceHelper();
	virtual ~CServiceHelper();

	static void ServiceMainHelper(CServiceContext* pCtx);

	//private attributes
private:
	typedef std::map<LPTSTR,PSERVICEMAIN> my_map;
	my_map m_services;
	int m_ServiceCnt;
	static CServiceContext* m_servctx;

	//private functions
private:

	static void WINAPI ServiceMain(DWORD dwArgc,LPTSTR* pszArgv);
	
	static DWORD WINAPI HandlerEx(DWORD dwControl,DWORD dwEventType,PVOID pvEventData,PVOID pvContext);

};

#endif // SERVICEHELPER_H
