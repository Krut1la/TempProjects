// ServiceContext.cpp: implementation of the CServiceContext class.
//
//////////////////////////////////////////////////////////////////////

#include "ServiceContext.h"
#include "ServiceHelper.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//extern wchar_t g_szServiceName[];

CServiceContext::CServiceContext()
{
	fAllowPauseCont = fAllowParamChange = false;
	fInteractWithDesktop = false;
	fOwnProcess = true;
	InitializeCriticalSection(&m_cs);
}

CServiceContext::~CServiceContext()
{
	DeleteCriticalSection(&m_cs);
}

DWORD CServiceContext::ProcessControlCode(DWORD dwCode)
{
	DWORD dwRetCode = ERROR_CALL_NOT_IMPLEMENTED;
	bool fPost = false;
	switch (dwCode){
		case 140:
			
			{
			
			LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe"); 
			LPCTSTR USER_FEK_EVENT = TEXT("MyPipeEvent");
			HANDLE hEvent = CreateEvent(NULL,TRUE,FALSE,USER_FEK_EVENT);

			HANDLE hPipe = CreateFile(lpszPipename, GENERIC_READ,NULL,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

			char buffer[100];

	        DWORD NumberOfBytesWriten = 0;
	        ReadFile(hPipe, buffer, 10, &NumberOfBytesWriten, NULL);
			if(hPipe == INVALID_HANDLE_VALUE)
				MessageBox(NULL,L"WRONG!!!",L"Simple service", MB_DEFAULT_DESKTOP_ONLY);
			else{
				MessageBox(NULL,L"GOOD!!!",L"Simple service", MB_DEFAULT_DESKTOP_ONLY);
			    MessageBoxA(NULL,buffer,"Simple service", MB_DEFAULT_DESKTOP_ONLY);
			}

			
			if(hPipe) CloseHandle(hPipe);
			/*if (!WaitNamedPipe(lpszPipename, 20000)) 
			{ 
				MessageBox(NULL,L"Could not close pipe!",L"Simple service", MB_DEFAULT_DESKTOP_ONLY);
				break;
			} */

			SetEvent(hEvent);
			break;
			}
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		SetState(SERVICE_STOPPED,2000);
		fPost = true;
		break;
	case SERVICE_CONTROL_PAUSE:
		if (fAllowPauseCont){
			SetState(SERVICE_PAUSED,2000);
			fPost = true;
		}
		break;
	case SERVICE_CONTROL_CONTINUE:
		if (fAllowPauseCont){
			SetState(SERVICE_RUNNING,2000);
			fPost = true;
		}
		break;
	case SERVICE_CONTROL_INTERROGATE:
		ReportStatus();
		break;
	case SERVICE_CONTROL_PARAMCHANGE:
		if (fAllowParamChange){
			fPost = true;
		}
	}
	if (fPost){
		if (!PostStatus(CK_SERVICECONTROL,dwCode)){
			dwRetCode = ERROR_FUNCTION_FAILED;
		}
		else{
			dwRetCode = NO_ERROR;
		}
	}
	return dwRetCode;
}

BOOL CServiceContext::SetState(DWORD dwState, DWORD dwWait)
{
	DWORD dwPendState;
	switch (dwState){
	case SERVICE_STOPPED:
		dwPendState = SERVICE_STOP_PENDING;
		break;
	case SERVICE_RUNNING:
		dwPendState = (dwCurrentState == SERVICE_PAUSED)?
			SERVICE_CONTINUE_PENDING:SERVICE_START_PENDING;
		break;
	case SERVICE_PAUSED:
		dwPendState = SERVICE_PAUSE_PENDING;
		break;
	default:
		dwPendState = 0;
	}

	dwCheckPoint = 1;
	dwWaitHint = dwWait;
	
	dwWin32ExitCode = NO_ERROR;
	dwServiceSpecificExitCode = 0;

	if (dwPendState){
		dwCurrentState = dwPendState;
		EnterCriticalSection(&m_cs);
		return ReportStatus();
	}
	return true;
}

BOOL CServiceContext::ReportState()
{
	DWORD dwUltState;
	switch(dwCurrentState){
	case SERVICE_START_PENDING:
	case SERVICE_CONTINUE_PENDING:
		dwUltState = SERVICE_RUNNING;
		break;
	case SERVICE_STOP_PENDING:
		dwUltState = SERVICE_STOPPED;
		break;
	case SERVICE_PAUSE_PENDING:
		dwUltState = SERVICE_PAUSED;
		break;
	default:
		dwUltState = 0;
	}

	dwCheckPoint = dwWaitHint = 0;

	dwWin32ExitCode = NO_ERROR;
	dwServiceSpecificExitCode = 0;

	if (dwUltState != 0){
		dwCurrentState = dwUltState;
		BOOL fOk = ReportStatus();
		LeaveCriticalSection(&m_cs);
		return fOk;
	}

	return true;
}

BOOL CServiceContext::RegisterService()
{
	m_hss = RegisterServiceCtrlHandlerEx(m_szServiceName,
		CServiceHelper::HandlerEx,this);
	if (m_hss != NULL){
		dwServiceType = fOwnProcess?SERVICE_WIN32_OWN_PROCESS:
			SERVICE_WIN32_SHARE_PROCESS;
		if (fInteractWithDesktop)
			dwServiceType |= SERVICE_INTERACTIVE_PROCESS;

		dwCurrentState = SERVICE_START_PENDING;
		dwControlsAccepted = SERVICE_ACCEPT_STOP;
		dwWin32ExitCode = NO_ERROR;
		dwServiceSpecificExitCode = 0;
		dwCheckPoint = 0;
		dwWaitHint = 2000;
		EnterCriticalSection(&m_cs);
		return Create();//Создаем порт завершения
	}
	else
		return FALSE;
}

void CServiceContext::OnServiceStart()
{
	
}

void CServiceContext::OnServiceStop()
{

}

void CServiceContext::OnParamChange()
{

}

void CServiceContext::OnUserEvent(DWORD dwEventCode, DWORD dwSubCode)
{

}

void CServiceContext::OnServicePause()
{

}
