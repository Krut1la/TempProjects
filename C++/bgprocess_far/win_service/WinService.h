#pragma once

#include "resource.h"

const UINT   WM_NOTIFYICON = WM_APP + 0x200;

/*enum PipeStatus
{
	isNone = 0,
	isWaitingForCommand,
	isWaitingForXML,
	isWaitingForAnswerFileExist,
	isWaitingForAnswerFileReadOnly,
	isWaitingForAnswerError
};

enum PipeCommand
{
	cmdBad = 10,
	cmdGetStatus,
	cmdAnswer,
	cmdXML
};


struct THREADDESC{
	DWORD CurrentOperationProgress;
	HANDLE Handle;
	DWORD Status;
	const char* xmlBuffer;//don't use in client
	DWORD xmlBufferSize;//don't use in client
	DWORD CurrentOperationType;
	TCHAR CurrentOperation[MAX_PATH];
	TCHAR CurrentAPIOperation[MAX_PATH];
	TCHAR CurrentError[MAX_PATH];
	DWORD FullProgress;
	HANDLE AnswerEvent;
	DWORD Answer;
};

typedef std::vector<THREADDESC> THREADS;
*/

VOID InstanceThread(LPVOID); 

DWORD CreateInstallThread(const char* xmlBuffer, DWORD xmlBufferSize, HANDLE& thread);
DWORD WaitForUserAnswer(THREADDESC* thrddsc, int target);
