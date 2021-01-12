// WinService.cpp : Defines the entry point for the application.
//

#define _WIN32_WINNT 0x0501

#include "WinService.h"

#include "mwcontext.h"
#include "ServiceContext.h"
#include "servicehelper.h"
#include "scmanager.h"


#include "..\\..\\include\\install.h"
/*
void WINAPI ServiceMain1(DWORD dwArgc,PWSTR* pszArgv)
{
	CServiceContext ctx;					//Создаем контекст службы
	ctx.fOwnProcess = false;				//Служб в процессе две
	ctx.fInteractWithDesktop = false;		//С рабочим столом не взаиможействуем
	ctx.m_szServiceName = _TEXT("SServ1");		//Имя службы
	CServiceHelper::ServiceMainHelper(&ctx);//Вызываем функцию, которая делает ВСЕ
}

void WINAPI ServiceMain2(DWORD dwArgc,PWSTR* pszArgv)
{
	CServiceContext ctx;
	ctx.fOwnProcess = false;
	ctx.fInteractWithDesktop = false;
	ctx.m_szServiceName = _TEXT("SServ2");
	CServiceHelper::ServiceMainHelper(&ctx);
}
*/

const int MAX_THREADS = 10;
const int BUFSIZE = 4096;
const int PIPE_TIMEOUT = 300;

enum PipeStatus
{
	isNone = 0,
	isWaitingForCommand,
	isWaitingForBufferSize,
	isWaitingForBuffer
};

enum PipeCommand
{
	cmdBAD = 10,
	cmdGetStatus,
	cmdXML,
	cmdXMLBUFFERSIZE,
	cmdXMLBUFFER
};

enum PipeAnswer
{
	ansBAD = 100,
	ansOK,
};

struct THREADDESC{
	HANDLE Handle;
	DWORD Status;
	DWORD CurrentOperationType;
	TCHAR CurrentOperation[MAX_PATH];
	DWORD CurrentOperationProgress;
	DWORD FullProgress;
	//void GetLikeBuffer(vector
};

typedef std::vector<THREADDESC> THREADS;

THREADS Threads(MAX_THREADS);



VOID InstanceThread(LPVOID); 

VOID GetAnswerToRequest(LPTSTR, LPTSTR, LPDWORD); 
DWORD GetCommandFromPipe(HANDLE hPipe);
BOOL SendAnswerToPipe(HANDLE hPipe, PipeAnswer pa);
BOOL SendStatusToPipe(HANDLE hPipe);
DWORD CreateInstallThread(void);

PipeStatus ps = isWaitingForCommand;

void MyErrExit(LPCTSTR msg)
{
	MessageBox(NULL, msg, TEXT("Error"), 0);
	exit(0);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

	BOOL fConnected; 
	DWORD dwThreadId; 
	HANDLE hPipe, hThread; 
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\gipipe"); 

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and the loop is repeated. 

	for (;;) 
	{ 
		hPipe = CreateNamedPipe( 
			lpszPipename,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFSIZE,                  // output buffer size 
			BUFSIZE,                  // input buffer size 
			PIPE_TIMEOUT,             // client time-out 
			NULL);                    // no security attribute 

		if (hPipe == INVALID_HANDLE_VALUE) 
			MyErrExit(TEXT("Служба не смогла стартовать, поскольку не удалось создать PIPE")); 

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function returns 
		// zero, GetLastError returns ERROR_PIPE_CONNECTED. 

		fConnected = ConnectNamedPipe(hPipe, NULL) ? 
          TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		if (fConnected) 
		{ 
			// Create a thread for this client. 
			hThread = CreateThread( 
				NULL,              // no security attribute 
				0,                 // default stack size 
				(LPTHREAD_START_ROUTINE) InstanceThread, 
				(LPVOID) hPipe,    // thread parameter 
				0,                 // not suspended 
				&dwThreadId);      // returns thread ID 

			if (hThread == NULL) 
				MessageBox(NULL, TEXT("В лог:Службе не удалось создать поток."), TEXT("Error"), 0);
			else
				CloseHandle(hThread);
		} 
		else 
			// The client could not connect, so close the pipe. 
			CloseHandle(hPipe); 
	} 



	return 0;

	/*int nArgc = __argc;
	LPWSTR* ppArgv = CommandLineToArgvW(GetCommandLine(),&nArgc);
	if (nArgc < 2){
		CMWContext mwctx;
		mwctx.fInteractWithDesktop = false;
		mwctx.fOwnProcess = true;			//Служба в процессе одна
		mwctx.m_szServiceName = L"SServ";

		CServiceHelper serv;
		serv.SetServiceCtx(&mwctx);			//Можно не определять глобальную ServiceMain!
		BOOL fOk = serv.Start();
		return fOk;					//Все службы остановлены, выход их программы
	}
	for (int i = 1;i < nArgc;i++)
	{
		if ((ppArgv[i][0] == L'-') || (ppArgv[i][0] == L'/')){
		wchar_t buf[100];
			if (!lstrcmpi(&ppArgv[i][1],L"install")){
				
				CSCManager scm;
				if (!scm.Open(SC_MANAGER_CREATE_SERVICE)){	//Открываем SCM для создания службы
					int err = GetLastError();
					swprintf(buf,L"Не могу открыть SCManager\nКод ошибки: %X",err);
					MessageBox(NULL,buf,L"Error",0);
					return 1;
				}
				
				if (!scm.CreateService(L"SServ",L"SimpleService",
					L"Простой сервис номер 1",false))
				{
					int err = GetLastError();
					swprintf(buf,L"Не могу создать службу %s\nКод ошибки: %X",
						L"SimpleService",err);
					MessageBox(NULL,buf,L"Error",0);
					return 1;
				}
				
				MessageBox(NULL,L"Служба успешно инсталирована",L"OK",0);
			}
			else if (!lstrcmpi(&ppArgv[i][1],L"remove")){

				CSCManager scm;
				if (!scm.Open(SC_MANAGER_ALL_ACCESS)){	//Открываем SCM для удаления службы
					int err = GetLastError();
					swprintf(buf,L"Не могу открыть SCManager\nКод ошибки: %X",err);
					MessageBox(NULL,buf,L"Error",0);
					return 1;
				}

				if (!scm.DeleteService(L"SServ")){
					int err = GetLastError();
					swprintf(buf,L"Не могу удалить службу %s\nКод ошибки: %X",
						L"SimpleService",err);
					MessageBox(NULL,buf,L"Error",0);
					return 1;
				}

				MessageBox(NULL,L"Служба успешно удалена",L"OK",0);
			}
			else{
				MessageBox(NULL,L"Неизвестная команда",L"Error",0);
			}
		}
	}
	return 0;

	*/
}

VOID InstanceThread(LPVOID lpvParam) 
{ 
	HRESULT hr = S_OK;
	//CHAR chRequest[BUFSIZE]; 
	//CHAR chReply[BUFSIZE]; 
	//DWORD cbBytesRead, cbReplyBytes, cbWritten; 
	//BOOL fSuccess; 

	// The thread's parameter is a handle to a pipe instance. 
	HANDLE hPipe = (HANDLE)lpvParam;

	
	
		
	while (1) 
	{ 
		// Read client requests from the pipe. 
		switch(ps)
		{
		case isNone:
			break;
		case isWaitingForCommand:
			switch(GetCommandFromPipe(hPipe))
			{
			case cmdGetStatus:
				//MessageBox(NULL, "XML", "PipeServer", 0);
				//SendAnswerToPipe(hPipe, ansOK);
				SendStatusToPipe(hPipe);
				break;
			case cmdXML:
				//MessageBox(NULL, TEXT("XML"), TEXT("PipeServer"), 0);
				//SendAnswerToPipe(hPipe, ansOK);
				hr = CreateInstallThread();
				if(hr != S_OK)
					MessageBox(NULL, TEXT("CreateInstallThread::Error"), TEXT("PipeServer"), 0);
				break;
			case cmdXMLBUFFERSIZE:
				MessageBox(NULL, TEXT("XMLBUFFERSIZE"), TEXT("PipeServer"), 0);
				//SendAnswerToPipe(hPipe, XUEMAIL);
				break;
			case cmdXMLBUFFER:
				MessageBox(NULL, TEXT("XMLBUFFER"), TEXT("PipeServer"), 0);
				//SendAnswerToPipe(hPipe, XUEMAILSIZE);
				break;
			default:
		        break;
			};
			break;
		case isWaitingForBufferSize:
			break;
        case isWaitingForBuffer:
			break;
		}
		break;
/*		// Write the reply to the pipe. 
		fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			chReply,      // buffer to write from 
			cbReplyBytes, // number of bytes to write 
			&cbWritten,   // number of bytes written 
			NULL);        // not overlapped I/O 


		if (! fSuccess || cbReplyBytes != cbWritten) break; 
		*/
	} 

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	

	FlushFileBuffers(hPipe); 
	DisconnectNamedPipe(hPipe); 
	CloseHandle(hPipe); 
}

DWORD GetCommandFromPipe(HANDLE hPipe)
{	
	DWORD command = 0;
	DWORD cbBytesRead;
	BOOL fSuccess;

	fSuccess = ReadFile( 
			hPipe,        // handle to pipe 
			&command,    // buffer to receive data 
			sizeof(DWORD),      // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

	if (! fSuccess || cbBytesRead == 0) 
			return cmdBAD; 
    
	return command;
}

BOOL SendAnswerToPipe(HANDLE hPipe, PipeAnswer pa)
{
	DWORD answer = (DWORD)pa;
	DWORD cbBytesWrite;
	BOOL fSuccess;


	fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			&answer,    // buffer to receive data 
			sizeof(DWORD),      // size of buffer 
			&cbBytesWrite, // number of bytes read 
			NULL);        // not overlapped I/O 

	if (! fSuccess || cbBytesWrite == 0) 
			return FALSE; 
	return TRUE; 
}

BOOL SendStatusToPipe(HANDLE hPipe)
{	
	DWORD cbBytesWrite;
	BOOL fSuccess;

	DWORD activeThreads = 0;
	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		if((*i_thread).Status != isNone)
		{
			++activeThreads;
		}
	}

	fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			&activeThreads,    // buffer to receive data 
			sizeof(DWORD),      // size of buffer 
			&cbBytesWrite, // number of bytes read 
			NULL);        // not overlapped I/O 

	if (! fSuccess || cbBytesWrite == 0) 
			return FALSE; 

	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		if((*i_thread).Status != isNone)
		{
			fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			&(*i_thread),    // buffer to receive data 
			sizeof(THREADDESC),      // size of buffer 
			&cbBytesWrite, // number of bytes read 
			NULL);        // not overlapped I/O 

			if (! fSuccess || cbBytesWrite == 0) 
			return FALSE; 
		}
	}

/*	size_t bufferSize = sizeof(DWORD);

	DWORD activeThreads = 0;
	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		++activeThreads;
		if((*i_thread).Status != isNone)
		{
			bufferSize += sizeof(HANDLE);
			bufferSize += sizeof(DWORD)*4;
			bufferSize += sizeof(size_t);
			bufferSize += sizeof(TCHAR)*(DWORD)(*i_thread).CurrentOperation.length();
		}
	}
	char *buffer = new char[bufferSize];

	size_t pos = 0;

	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		if((*i_thread).Status != isNone)
		{
			memcpy((buffer + pos), &activeThreads, sizeof(DWORD)); pos += sizeof(DWORD);
			memcpy((buffer + pos), &((*i_thread).Handle), sizeof(HANDLE)); pos += sizeof(HANDLE);
			memcpy((buffer + pos), &((*i_thread).Status), sizeof(DWORD)); pos += sizeof(DWORD);
			memcpy((buffer + pos), &((*i_thread).CurrentOperationType), sizeof(DWORD)); pos += sizeof(DWORD);
			size_t strlenbt = sizeof(TCHAR)*(*i_thread).CurrentOperation.length();
			memcpy((buffer + pos), &strlenbt, sizeof(size_t)); pos += sizeof(size_t);
			memcpy((buffer + pos), ((*i_thread).CurrentOperation.c_str()), strlenbt); pos += strlenbt;
			memcpy((buffer + pos), &((*i_thread).CurrentOperationProgress), sizeof(DWORD)); pos += sizeof(DWORD);
		}
	}

	DWORD cbBytesWrite;
	BOOL fSuccess;


	fSuccess = WriteFile( 
			hPipe,        // handle to pipe 
			buffer,    // buffer to receive data 
			(DWORD)bufferSize,      // size of buffer 
			&cbBytesWrite, // number of bytes read 
			NULL);        // not overlapped I/O 

	delete[] buffer;

	if (! fSuccess || cbBytesWrite == 0) 
			return FALSE; 
	return TRUE; 
	*/
	
	return TRUE; 
}

DWORD CALLBACK InstallCallback(CallBackMSG msg, TCHAR* str, LONG P1, LONG P2, LPVOID userdata)
{
	//char *sdfsdfsd = (char*)str;
	switch(msg)
	{
	case msgFatal:
	     MessageBox(NULL, str, TEXT("Fatal error"), 0);
		 break;
	case msgError:
	     MessageBox(NULL, str, TEXT("Error"), 0);
		 break;
    case msgResult:
	     MessageBox(NULL, str, TEXT("Result"), 0);
		 break;
	case msgWarning:
	     MessageBox(NULL, str, TEXT("Warning"), 0);
		 break;
	};

	return S_OK;
}

VOID InstallThread(LPVOID lpvParam) 
{
	HRESULT hr = S_OK;
	THREADDESC &thrddsc = (THREADDESC&)lpvParam;

	thrddsc.Handle = GetCurrentThread();
	thrddsc.Status = isWaitingForCommand;

	std::ifstream ifs("D:\\krutila\\prg\\GameInstaller\\script3.xml", std::ifstream::in | std::ifstream::binary);
	//std::ifstream ifs("C:\\Program Files\\Common Files\\Microsoft Shared\\VSA\\7.1\\VsaEnv\\_vsaenv.xml");
	
	std::istreambuf_iterator<char> b1(ifs);
	std::istreambuf_iterator<char> b2;
	std::string buffer(b1, b2);
			
	hr = InstallFromXMLBuffer(buffer.c_str(), buffer.length(), InstallCallback, lpvParam);

	if(hr != S_OK)
		MessageBox(NULL, TEXT("InstallFromXMLBuffer::Error"), TEXT("PipeServer"), 0);


	thrddsc.Status = isNone;
}

DWORD CreateInstallThread(void)
{
	HANDLE hThread;
	THREADS::iterator mythread;

	for(mythread = Threads.begin(); mythread != Threads.end(); ++mythread)
	{
		if((*mythread).Status == isNone)
			break;
	}

	if(mythread == Threads.end())
	{
		MessageBox(NULL, TEXT("To Log: максимум потоков"), TEXT("Error"), 0);
		return S_FALSE;
	}

	hThread = CreateThread( 
				NULL,              // no security attribute 
				0,                 // default stack size 
				(LPTHREAD_START_ROUTINE) InstallThread, 
				(LPVOID)&(*mythread),    // thread parameter 
				0,                 // not suspended 
				NULL);      // returns thread ID 

	if (hThread == NULL) 
	{
		MessageBox(NULL, TEXT("В лог:Службе не удалось создать поток инсталляции."), TEXT("Error"), 0);
		return S_FALSE;
	}
	return S_OK;
}