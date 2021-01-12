#ifndef STATUS_H
#define STATUS_H

struct ClosePipeException
{
	const TCHAR* msg;
	ClosePipeException(const TCHAR* msg):
	msg(msg){}
};

DWORD AbortThread(HANDLE hPipe, HANDLE thread, bool pause = false, bool all = false);
DWORD InitStatus(void);
void CloseStatus(void);
DWORD GetThreadStatus(HANDLE hPipe, THREADS& threads);
DWORD WINAPI TimerThreadProc(LPVOID lpParameter);


#endif// STATUS_H