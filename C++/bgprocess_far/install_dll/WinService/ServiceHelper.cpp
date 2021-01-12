// ServiceHelper.cpp: implementation of the CServiceHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "ServiceHelper.h"
#include "servicecontext.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServiceContext* CServiceHelper::m_servctx = NULL;
//extern wchar_t g_szServiceName[];

CServiceHelper::CServiceHelper()
{
	m_ServiceCnt = NOSERVICE;
}

CServiceHelper::~CServiceHelper()
{

}

bool CServiceHelper::AddService(LPTSTR szServiceName, PSERVICEMAIN ServMainProc)
{
	if (m_ServiceCnt == ONESERVICE)	return false;
	else{
		m_services[szServiceName] = ServMainProc;
		m_ServiceCnt = MULTIPLESERVICE;
		return true;
	}
}

BOOL CServiceHelper::Start()
{
	size_t l = m_services.size();
	SERVICE_TABLE_ENTRY* pServiceTable = 
		new SERVICE_TABLE_ENTRY[l+1];
	
	if (!pServiceTable) return FALSE;

	size_t i = 0;
	for (my_map::iterator i_service = m_services.begin(); i_service != m_services.end(); ++i_service)
	{
		pServiceTable[i].lpServiceName = (*i_service).first;
		pServiceTable[i].lpServiceProc = (*i_service).second;
		++i;
	}
	pServiceTable[i].lpServiceName = NULL;
	pServiceTable[i].lpServiceProc = NULL;

	BOOL fOk = StartServiceCtrlDispatcher(pServiceTable);
	delete[] pServiceTable;
	return fOk;
}

DWORD WINAPI CServiceHelper::HandlerEx(DWORD dwControl,DWORD dwEventType,
	PVOID pvEventData,PVOID pvContext)
{
	CServiceContext* pCtx = (CServiceContext*) pvContext;
	if (pCtx->dwCurrentState == SERVICE_PAUSED &&
		dwControl == SERVICE_CONTROL_PAUSE)
		return ERROR_INVALID_PARAMETER;
	else if (pCtx->dwCurrentState == SERVICE_RUNNING &&
		dwControl == SERVICE_CONTROL_CONTINUE)
		return ERROR_INVALID_PARAMETER;
	else
		return pCtx->ProcessControlCode(dwControl);
}

void WINAPI CServiceHelper::ServiceMain(DWORD dwArgc,LPTSTR* pszArgv)
{
	assert(m_servctx != NULL);

	if (m_servctx == NULL){
		MessageBox(NULL, _TEXT("Service context not specify"), _TEXT("Error"), 0);
		return;
	}
	ServiceMainHelper(m_servctx);
}

void CServiceHelper::ServiceMainHelper(CServiceContext *pCtx)
{
	ULONG_PTR CompKey = CServiceContext::CK_SERVICECONTROL;
	DWORD dwControl = SERVICE_CONTROL_CONTINUE;
	
	OVERLAPPED* pov;

	if (!pCtx->RegisterService()) return;

	do{
		switch(CompKey){
		case CServiceContext::CK_SERVICECONTROL:
			switch(dwControl){
			case SERVICE_CONTROL_CONTINUE:
				pCtx->OnServiceStart();
				break;
			case SERVICE_CONTROL_PAUSE:
				pCtx->OnServicePause();
				break;
			case SERVICE_CONTROL_STOP:
			case SERVICE_CONTROL_SHUTDOWN:
				pCtx->OnServiceStop();
				break;
			case SERVICE_CONTROL_PARAMCHANGE:
				pCtx->OnParamChange();
				break;
			}
			pCtx->ReportState();
			break;
		default:
			pCtx->OnUserEvent(static_cast<DWORD>(CompKey),dwControl);	
		}
		if (pCtx->dwCurrentState != SERVICE_STOPPED){
			pCtx->GetStatus(&CompKey,&dwControl,&pov);
		}
		else break;
	}
	while(true);
}
