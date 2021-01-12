#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "Status.h"

extern struct PluginStartupInfo Info;

HANDLE	hWaitebleTimer 		= NULL;
HANDLE	hConsole			= NULL;
HANDLE	hTimerThread 		= NULL;
HANDLE	hGetStatusEvent 	= NULL;

//HANDLE hThreadStartedEvent	= NULL;

bool	stopTimerThreadEvent 	= false;
bool	OncePassed				= false;

HANDLE hMainPipe = NULL;


DWORD InitStatus(void)
{	
	stopTimerThreadEvent = false;

	hGetStatusEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(!hGetStatusEvent)
	{
		ErrorLookup("CreateEvent", GetLastError());
		return S_FALSE;
	}

	hWaitebleTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if(!hWaitebleTimer)
	{
		ErrorLookup("CreateWaitableTimer", GetLastError());
		return S_FALSE;
	}

	/*OncePassed = false;
	hThreadStartedEvent = CreateEventW(NULL, TRUE, FALSE, L"ThreadStartedEvent");
	if(!hThreadStartedEvent)	
		return S_FALSE;
*/
	
	LARGE_INTEGER liDueTime;

    liDueTime.QuadPart = 1;

	if(!SetWaitableTimer(hWaitebleTimer, &liDueTime, 1, NULL, NULL, TRUE))
	{
		ErrorLookup("SetWaitableTimer", GetLastError());
		return S_FALSE;
	}
	
	hConsole = OpenConsoleIn();
	if(hConsole == INVALID_HANDLE_VALUE)
	{
		ErrorLookup("OpenConsoleIn", GetLastError());
		return S_FALSE;
	}

	hTimerThread = CreateThread(NULL, 0, TimerThreadProc, NULL, 0, NULL);
	if(!hTimerThread)
	{
		ErrorLookup("CreateThread", GetLastError());
		return S_FALSE;
	}
		
	return S_OK;
}

void CloseStatus(void)
{	
	//WriteConsoleKey(hConsole,VK_BACK,0);
	FlushConsoleInputBuffer(hConsole);

	if(hTimerThread)			CloseHandle(hTimerThread);
	if(hWaitebleTimer)			CancelWaitableTimer(hWaitebleTimer);
	if(hWaitebleTimer)			CloseHandle(hWaitebleTimer);
	if(hGetStatusEvent)			CloseHandle(hGetStatusEvent);
	if(hConsole)				CloseHandle(hConsole);
	//if(hThreadStartedEvent)		CloseHandle(hThreadStartedEvent);

	hTimerThread = NULL;
	hWaitebleTimer = NULL;
	hWaitebleTimer = NULL;
	hGetStatusEvent = NULL;
	hConsole = NULL;
	//hThreadStartedEvent = NULL;
}

DWORD WINAPI TimerThreadProc(LPVOID lpParameter)
{
	while(!stopTimerThreadEvent)
	{		
		/*if(!OncePassed)
		{
			WaitForSingleObjectEx(hThreadStartedEvent, INFINITE, TRUE);
			OncePassed = true;
		}*/
		WaitForSingleObjectEx(hWaitebleTimer, INFINITE, TRUE);
		//WriteConsoleKey(hConsole,VK_BACK,LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED);
		WriteConsoleKey(hConsole,VK_BACK,0);
		//WriteConsoleKey(hConsole,VK_BACK,0);
		//WriteConsoleKey(hConsole,VK_BACK,0);
		//WriteConsoleKey(hConsole,VK_BACK,0);

		WaitForSingleObjectEx(hGetStatusEvent, INFINITE, TRUE);
		ResetEvent(hGetStatusEvent);
	}
	return S_OK;
}


DWORD AbortThread(HANDLE hPipe, HANDLE thread, bool pause, bool all)
{
	//HANDLE hPipe = OpenPipe();
	//if(!hPipe)
		//return S_FALSE;

	try
	{
		THREADS threads;
		SendCommandToPipe(hPipe, PCC_GET_STATUS);

		if(GetStatusFromPipe(hPipe, threads) != S_OK)
			return S_FALSE;
			//throw ClosePipeException(TEXT(""));
        
		if(all)
		{
			if(pause)
                SendCommandToPipe(hPipe, PCC_PAUSE_ALL);
			else
				SendCommandToPipe(hPipe, PCC_ABORT_ALL);
		}
		else
		{
			if(pause)
                SendCommandToPipe(hPipe, PCC_PAUSE_THREAD);
			else
				SendCommandToPipe(hPipe, PCC_ABORT_THREAD);

			SendHandleToPipe(hPipe, thread);
		}

		DWORD server_confirm;
		GetCommandFromPipe(hPipe, server_confirm);

		if(server_confirm != PSC_OK)
			return S_FALSE;
			//throw ClosePipeException(TEXT(""));

		//SendCommandToPipe(hPipe, PCC_END_SESSION);

		//server_confirm = 0;
		//GetCommandFromPipe(hPipe, server_confirm);

		//if(server_confirm != PSC_OK)
			//throw ClosePipeException(TEXT(""));
				
	}
	catch(PipeErrorException peex)
	{
		ErrorLookup(peex.msg, peex.last_error);
	}
	catch(ClosePipeException cpex)
	{
		//CloseHandle(hPipe);
		return S_FALSE;
	}
			
	//CloseHandle(hPipe);
	return S_OK;
}

DWORD GetThreadStatus(HANDLE hPipe, THREADS& threads)
{
	//HANDLE hPipe = OpenPipe();
	//if(!hPipe)
		//return S_FALSE;

	try
	{
		SendCommandToPipe(hPipe, PCC_GET_STATUS);
		HRESULT hr = GetStatusFromPipe(hPipe, threads);
		
		if(hr != S_OK)
			return S_FALSE;
		//	throw ClosePipeException(TEXT(""));

		//SendCommandToPipe(hPipe, PCC_END_SESSION);

		//DWORD server_confirm = 0;
		//GetCommandFromPipe(hPipe, server_confirm);

		//if(server_confirm != PSC_OK)
		//	throw ClosePipeException(TEXT(""));
	}
	catch(PipeErrorException peex)
	{
		ErrorLookup(peex.msg, peex.last_error);
		return S_FALSE;
	}
	//catch(ClosePipeException cpex)
	//{
		//CloseHandle(hPipe);
		//return S_FALSE;
	//}
			
	//CloseHandle(hPipe);
	return S_OK;
}