#ifndef SCMANAGER_H
#define SCMANAGER_H

#include "WinService.h"

class CSCManager
{
	//Public functions
public:
	BOOL Close();
	BOOL DeleteService(LPCTSTR szServiceName,bool fStop = true);
	BOOL CreateService(LPCTSTR szServiceName,LPCTSTR szDisplayName,
		LPTSTR szDescription,bool fOwnProcess = true,PCTSTR szDepend = NULL);
	bool Open(DWORD dwAccess);
	
	CSCManager();
	~CSCManager();

	//Public attributes
public:
	PTSTR m_modulename;

	//private attributes
private:
	SC_HANDLE m_hScm;
};

#endif // SCMANAGER_H
