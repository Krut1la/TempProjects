#ifndef IOCP_H
#define IOCP_H

#include "WinService.h"

class CIOCP
{
public:
	CIOCP(int MaxConcurency = -1){
		m_hcp = NULL;
		if (MaxConcurency != -1) Create(MaxConcurency);
	};
	~CIOCP(){if (m_hcp) CloseHandle(m_hcp);};

	BOOL Create(int MaxConcurency = 0){
		m_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
			NULL,0,MaxConcurency);
		return m_hcp != NULL;
	};

	BOOL AssociateDevice(HANDLE hDev,ULONG_PTR CompKey){
		BOOL fOk = CreateIoCompletionPort(hDev,
			m_hcp,CompKey,0) == m_hcp;
		return fOk;
	};

	BOOL PostStatus(ULONG_PTR CompKey,DWORD dwNumBytes = 0,
		LPOVERLAPPED pov = NULL){
		BOOL fOk = PostQueuedCompletionStatus(m_hcp,
			dwNumBytes,CompKey,pov);
		return fOk;
	};

	BOOL GetStatus(ULONG_PTR* pCompKey,PDWORD pdwNumBytes,
		LPOVERLAPPED* ppov, DWORD dwMil = INFINITE){
		BOOL fOk = GetQueuedCompletionStatus(m_hcp,pdwNumBytes,
			pCompKey,ppov,dwMil);
		return fOk;
	};
private:
	HANDLE m_hcp;
};

#endif // IOCP_H
