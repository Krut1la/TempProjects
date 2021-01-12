#include "stdafx.h"
#include "PipeProtocol.h"

THREADDESC& GetThreadByHandle(THREADS& Threads, HANDLE hThread)
{
	THREADS::iterator i_thread;
	for(i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		if((*i_thread).Handle == hThread)
			break;
	}
	return *i_thread;
}

DWORD SendXMLToPipe(HANDLE hPipe, const char* xmlBuffer, DWORD xmlBufferSize)
{	
	DWORD cbBytesWrite;
	BOOL fSuccess;

	fSuccess = WriteFile( 
			hPipe,
			&xmlBufferSize,
			sizeof(DWORD),
			&cbBytesWrite,
			NULL);

	if (! fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendXMLToPipe:WriteFile:xmlBufferSize"), 0, GetLastError()); 
	
	fSuccess = WriteFile( 
			hPipe,
			xmlBuffer,
			xmlBufferSize,
			&cbBytesWrite,
			NULL);

	if (! fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendXMLToPipe:WriteFile:xmlBuffer"), 0, GetLastError()); 
	
	return S_OK;
}

DWORD GetStatusFromPipe(HANDLE hPipe, THREADS &Threads)
{
	BOOL fSuccess;
	DWORD cbBytesRead = 0;

	DWORD activeThreads = 0;

	fSuccess = ReadFile(hPipe, &activeThreads, sizeof(DWORD), &cbBytesRead, NULL);

	if (! fSuccess || cbBytesRead == 0) 
		throw PipeErrorException(TEXT("GetStatusFromPipe:ReadFile:activeThreads"), 0, GetLastError()); 

	if(activeThreads == 0)
		return S_FALSE;

	for(DWORD i = 0; i < activeThreads && i < MAX_THREADS; ++i)
	{
		THREADDESC thrddsc;

		fSuccess = ReadFile(hPipe, &thrddsc, sizeof(THREADDESC), &cbBytesRead, NULL);

		if (!fSuccess || cbBytesRead == 0) 
			throw PipeErrorException(TEXT("GetStatusFromPipe:ReadFile:thrddsc"), 0, GetLastError()); 

		Threads.push_back(thrddsc);
	}

	return S_OK;
}



DWORD SendStatusToPipe(HANDLE hPipe, THREADS& Threads)
{	
	DWORD cbBytesWrite;
	BOOL fSuccess;

	DWORD activeThreads = 0;
	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		if((*i_thread).Status != PTS_NONE)
		{
			++activeThreads;
		}
	}

	fSuccess = WriteFile(
		hPipe,
		&activeThreads,
		sizeof(DWORD),
		&cbBytesWrite,
		NULL);

	if (! fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendStatusToPipe:WriteFile:activeThreads"), 0, GetLastError()); 

	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		if((*i_thread).Status != PTS_NONE)
		{	
			//THREADDESC thrddsc = (*i_thread);
			fSuccess = WriteFile(
				hPipe,
				&(*i_thread),
				sizeof(THREADDESC),
				&cbBytesWrite,
				NULL);

			if (! fSuccess || cbBytesWrite == 0) 
			throw PipeErrorException(TEXT("SendStatusToPipe:WriteFile:*i_thread"), 0, GetLastError()); 
		}
	}
	
	return S_OK; 
}

DWORD GetXMLFromPipe(HANDLE hPipe, char*& xmlBuffer, DWORD& xmlBufferSize)
{	
	DWORD cbBytesRead;
	BOOL fSuccess;

	fSuccess = ReadFile( 
			hPipe,
			&xmlBufferSize,
			sizeof(DWORD),
			&cbBytesRead,
			NULL);

	if (!fSuccess || cbBytesRead == 0) 
		throw PipeErrorException(TEXT("GetXMLFromPipe:ReadFile:xmlBufferSize"), 0, GetLastError()); 
	
    xmlBuffer = new char[xmlBufferSize];

	if(!xmlBuffer)
		return S_FALSE; 

	fSuccess = ReadFile( 
			hPipe,
			xmlBuffer,
			xmlBufferSize,
			&cbBytesRead,
			NULL);

	if (! fSuccess || cbBytesRead == 0) 
	{
		delete[] xmlBuffer;
		throw PipeErrorException(TEXT("GetXMLFromPipe:ReadFile:xmlBuffer"), 0, GetLastError()); 
	}

	return S_OK;
}

DWORD SendCommandToPipe(HANDLE hPipe, DWORD cmd)
{
	BOOL fSuccess;
	DWORD cbBytesWrite = 0;

	fSuccess = WriteFile(hPipe, &cmd, sizeof(DWORD), &cbBytesWrite, NULL);

	if (! fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendCommandToPipe:WriteFile:cmd"), 0, GetLastError()); 

	return S_OK;
}

DWORD GetCommandFromPipe(HANDLE hPipe, DWORD& cmd)
{		
	DWORD cbBytesRead;
	BOOL fSuccess;

	fSuccess = ReadFile(hPipe, &cmd, sizeof(DWORD),	&cbBytesRead, NULL);

	if (! fSuccess || cbBytesRead == 0) 
		throw PipeErrorException(TEXT("GetCommandFromPipe:ReadFile:cmd"), 0, GetLastError()); 

 	return S_OK;
}

/*DWORD SendUserAnswerToPipe(DWORD threadId, DWORD answer)
{
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\gipipe"); 

	HANDLE hPipe = CreateFile(
		lpszPipename,
		FILE_ALL_ACCESS, 
		FILE_SHARE_WRITE,
		NULL, 
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 
		NULL);

	if(!hPipe)
		throw PipeErrorException(TEXT("SendUserAnswerToPipe:CreateFile:hPipe"), 0, GetLastError()); 

		
	SendCommandToPipe(hPipe, PCC_TAKE_ANSWER);

	BOOL fSuccess;
	DWORD cbBytesWrite = 0;

	fSuccess = WriteFile(hPipe, &threadId, sizeof(DWORD), &cbBytesWrite, NULL);

	if(!fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendUserAnswerToPipe:WriteFile:threadId"), 0, GetLastError()); 

	fSuccess = WriteFile(hPipe, &answer, sizeof(DWORD), &cbBytesWrite, NULL);

	if(!fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendUserAnswerToPipe:WriteFile:answer"), 0, GetLastError()); 


	DWORD server_confirm;
	GetCommandFromPipe(hPipe, server_confirm);
		
	if(server_confirm != PSC_OK)
		return S_FALSE; 

//	CloseHandle(hPipe);
	return S_OK;
}*/

/*DWORD GetUserAnswerFromPipe(HANDLE hPipe, DWORD& threadId, DWORD& answer)
{
	DWORD cbBytesRead;
	BOOL fSuccess;

	fSuccess = ReadFile(hPipe, &threadId, sizeof(DWORD), &cbBytesRead, NULL);

	if(!fSuccess || cbBytesRead == 0) 
		throw PipeErrorException(TEXT("GetUserAnswerFromPipe:ReadFile:threadId"), 0, GetLastError()); 

	fSuccess = ReadFile(hPipe, &answer, sizeof(DWORD), &cbBytesRead, NULL);

	if(!fSuccess || cbBytesRead == 0) 
		throw PipeErrorException(TEXT("GetUserAnswerFromPipe:ReadFile:answer"), 0, GetLastError()); 

 	return S_OK;
}*/

HANDLE	OpenPipe()
{
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\gipipe"); 

	HANDLE hPipe = NULL;

	for(;;)
	{
		hPipe = CreateFile(
			lpszPipename,
			FILE_ALL_ACCESS, 
			FILE_SHARE_WRITE,
			NULL, 
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, 
			NULL);

		// Break if the pipe handle is valid. 

		if (hPipe != INVALID_HANDLE_VALUE) 
			break; 

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY) 
			return NULL;
			//throw PipeErrorException(TEXT("OpenPipe:CreateFile"), 0, GetLastError()); 

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000) ) 
			return NULL;
			//throw PipeErrorException(TEXT("OpenPipe:WaitNamedPipe"), 0, GetLastError()); 
	}
	return hPipe;
}

DWORD SendHandleToPipe(HANDLE hPipe, HANDLE handle)
{
	BOOL fSuccess;
	DWORD cbBytesWrite = 0;

	fSuccess = WriteFile(hPipe, &handle, sizeof(HANDLE), &cbBytesWrite, NULL);

	if (!fSuccess || cbBytesWrite == 0) 
		throw PipeErrorException(TEXT("SendHandleToPipe:WriteFile:handle"), 0, GetLastError()); 

	return S_OK;
}

DWORD GetHandleFromPipe(HANDLE hPipe, HANDLE& handle)
{
	DWORD cbBytesRead;
	BOOL fSuccess;

	fSuccess = ReadFile(hPipe, &handle, sizeof(HANDLE),	&cbBytesRead, NULL);

	if(!fSuccess || cbBytesRead == 0) 
		throw PipeErrorException(TEXT("GetHandleFromPipe:ReadFile:handle"), 0, GetLastError()); 

 	return S_OK;
}