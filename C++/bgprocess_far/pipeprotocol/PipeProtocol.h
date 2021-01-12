#ifndef PIPEPROTOCOL_H
#define PIPEPROTOCOL_H

const DWORD MAX_THREADS = 10;


//Thread status
const DWORD PTS_NONE									= 0x1001;
const DWORD PTS_WAITING_FOR_COMMAND						= 0x1002;
const DWORD PTS_WAITING_FOR_XML							= 0x1003;
const DWORD PTS_WAITING_FOR_ANSWER_FILEEXISTS			= 0x1004;
const DWORD PTS_WAITING_FOR_ANSWER_FILEREADONLY			= 0x1005;
const DWORD PTS_WAITING_FOR_ANSWER_ERROR				= 0x1006;
const DWORD PTS_ABORTING								= 0x1007;
const DWORD PTS_PAUSED									= 0x1008;

//Client command
const DWORD PCC_GET_STATUS								= 0x2001;
const DWORD PCC_TAKE_ANSWER								= 0x2002;
const DWORD PCC_TAKE_XML								= 0x2003;
const DWORD PCC_PAUSE_THREAD							= 0x2004;
const DWORD PCC_PAUSE_ALL								= 0x2005;
const DWORD PCC_ABORT_THREAD							= 0x2006;
const DWORD PCC_ABORT_ALL								= 0x2007;
const DWORD PCC_SET_QUEUE								= 0x2008;
const DWORD PCC_END_SESSION								= 0x2009;
const DWORD PCC_STOP_SERVICE							= 0x200A;

//Client answer
const DWORD PCA_OK										= 0x3001;
const DWORD PCA_ABORT									= 0x3002;
const DWORD PCA_RETRY									= 0x3003;
const DWORD PCA_SKIP									= 0x3004;
const DWORD PCA_OVERWRITE								= 0x3005;
const DWORD PCA_REFRESH									= 0x3006;
const DWORD PCA_APPEND									= 0x3007;
const DWORD PCA_DELETE									= 0x3008;
const DWORD PCA_MOVE									= 0x3009;

const DWORD PSC_OK										= 0x8001;
const DWORD PSC_BAD										= 0x8002;

//Thread flags
const DWORD PTF_A									= 0x1001;

//Thread description
//struct TCopyData
//{

#define HIGH_BIT (DWORD(1)<<((sizeof(DWORD)<<3) - 1))

const DWORD ANSWER					= 0;
const DWORD ANSWER_FILE_EXISTS		= 1;
const DWORD ANSWER_FILE_READONLY	= 2;
const DWORD ANSWER_ERROR			= 3;

const DWORD CURRENT_API_FROM		= 0;
const DWORD CURRENT_API_TO			= 1;
const DWORD CURRENT_API_CO			= 2;

struct THREADDESC{
	HANDLE	Handle;
	DWORD	Status;
	DWORD	ClientCommand;
	DWORD	Answer[4];
	DWORD	All[4];
	const char* xmlBuffer;//don't use in client
	DWORD xmlBufferSize;//don't use in client
	DWORD CurrentOperationType;
	//TCHAR CurrentOperation[MAX_PATH];
	//TCHAR CurrentAPIOperation[MAX_PATH];
	//TCHAR CurrentError[MAX_PATH];
	wchar_t CurrentAPIOperation[MAX_PATH];
	//wchar_t CurrentError[MAX_PATH];
	DWORD CurrentError;
	DWORD FullProgress;
	DWORD CurrentOperationProgress;

	DWORD TotalBytes;
	DWORD TotalBytesProcessed;
	DWORD CurrentSpeed;

	DWORD CurrentAPI;
	wchar_t AnswerEvent[MAX_PATH];
	wchar_t JobName[MAX_PATH];
	wchar_t Operation[MAX_PATH];
	wchar_t CurrentOperationFrom[MAX_PATH];
	wchar_t CurrentOperationTo[MAX_PATH];
	//wchar_t PauseEvent[MAX_PATH];

	//union{
	//	TCopyData data;
		//TMoveData data;
		//TDeleteData data;
		
	//}OperationData;
};


struct EndSessionException
{
	const TCHAR* msg;
	EndSessionException(const TCHAR* msg):
	msg(msg){}
};

struct PipeErrorException
{
	const TCHAR* msg;
	const DWORD return_value;
	const DWORD last_error;
	PipeErrorException(const TCHAR* msg, const DWORD return_value, const DWORD last_error):
	msg(msg), return_value(return_value), last_error(last_error){}
};

typedef std::vector<THREADDESC> THREADS;

THREADDESC& GetThreadByHandle(THREADS& Threads, HANDLE hThread);

HANDLE	OpenPipe();

DWORD	SendXMLToPipe(HANDLE hPipe, const char* xmlBuffer, DWORD xmlBufferSize);
DWORD	GetXMLFromPipe(HANDLE hPipe, char*& xmlBuffer, DWORD& xmlBufferSize);

DWORD	SendCommandToPipe(HANDLE hPipe, DWORD cmd);
DWORD	GetCommandFromPipe(HANDLE hPipe, DWORD& cmd);

DWORD	SendHandleToPipe(HANDLE hPipe, HANDLE handle);
DWORD	GetHandleFromPipe(HANDLE hPipe, HANDLE& handle);

DWORD	SendStatusToPipe(HANDLE hPipe, THREADS& Threads);
DWORD	GetStatusFromPipe(HANDLE hPipe, THREADS &Threads);

/*DWORD	SendUserAnswerToPipe(DWORD threadId, DWORD answer);
DWORD	GetUserAnswerFromPipe(HANDLE hPipe, DWORD& threadId, DWORD& answer);*/

#endif// PIPEPROTOCOL_H