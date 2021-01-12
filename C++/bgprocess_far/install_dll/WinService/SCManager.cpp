// SCManager.cpp: implementation of the CSCManager class.
//
//////////////////////////////////////////////////////////////////////

#include "SCManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSCManager::CSCManager()
{
	m_hScm = NULL;
	m_modulename = NULL;
}

CSCManager::~CSCManager()
{
	if (m_modulename) delete m_modulename;
	if (m_hScm) CloseServiceHandle(m_hScm);
}

bool CSCManager::Open(DWORD dwAccess)
{
	m_hScm = OpenSCManager(NULL,NULL,dwAccess);
	return m_hScm != NULL;
}

BOOL CSCManager::CreateService(LPCTSTR szServiceName,LPCTSTR szDisplayName,
	LPTSTR szDescription,bool fOwnProcess,PCTSTR szDepend)
{
	assert(szServiceName);
	assert(szDisplayName);

	if (!szServiceName || !szDisplayName){
		MessageBox(NULL, _TEXT("Имя службы и ее название должны быть заданы"), _TEXT("Error"), 0);
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	if (!m_modulename){
		m_modulename = new TCHAR[MAX_PATH];
		GetModuleFileName(NULL,m_modulename,MAX_PATH);
	}

	SC_HANDLE hServ = ::CreateService(m_hScm,szServiceName,szDisplayName,
		SERVICE_CHANGE_CONFIG,
		fOwnProcess?SERVICE_WIN32_OWN_PROCESS:SERVICE_WIN32_SHARE_PROCESS,
		SERVICE_DEMAND_START,SERVICE_ERROR_IGNORE,m_modulename,
		NULL,NULL,szDepend,NULL,NULL);
	
	if (!hServ) return false;
	
	BOOL fOk = true;
	int err;
	if (szDescription){
		SERVICE_DESCRIPTION sd = {szDescription};
		fOk = ChangeServiceConfig2(hServ,SERVICE_CONFIG_DESCRIPTION,&sd);
		if (!fOk) err = GetLastError();
	}
	CloseServiceHandle(hServ);
	if (!fOk) SetLastError(err);
	return fOk;
}

BOOL CSCManager::DeleteService(LPCTSTR szServiceName,bool fStop)
{
	assert(szServiceName);

	if (!szServiceName){
		MessageBox(NULL, _TEXT("Имя службы не задано"), _TEXT("Error"), 0);
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	SC_HANDLE hServ = OpenService(m_hScm,szServiceName,
		DELETE|(fStop?SERVICE_STOP:0));

	if (!hServ) return FALSE;

	if (fStop){
		SERVICE_STATUS status;
		if (!ControlService(hServ, SERVICE_CONTROL_STOP, &status)){
			int err = GetLastError();
			if (err != ERROR_SERVICE_NOT_ACTIVE &&
				err != ERROR_SHUTDOWN_IN_PROGRESS){
				return FALSE;
			}
		}
		else
			Sleep(status.dwWaitHint);
	}

	BOOL fOk = ::DeleteService(hServ);
	int err;
	if (!fOk) err = GetLastError();
	CloseServiceHandle(hServ);
	if (!fOk) SetLastError(err);
	return fOk;
}

BOOL CSCManager::Close()
{
	if (m_hScm){
		BOOL fOk = CloseServiceHandle(m_hScm);
		m_hScm = NULL;
		return fOk;
	}
	else
		return SetLastError(ERROR_INVALID_HANDLE),FALSE;
}
