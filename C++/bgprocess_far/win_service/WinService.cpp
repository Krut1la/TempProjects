// WinService.cpp : Defines the entry point for the application.
//

#define _WIN32_WINNT 0x0501

#include "stdafx.h"
#include "WinService.h"

#include "mwcontext.h"
#include "ServiceContext.h"
#include "servicehelper.h"
#include "scmanager.h"

#include "NotifyIconEx.h"


//const int MAX_THREADS = 10;
const int BUFSIZE = 4096;
const int PIPE_TIMEOUT = 300;

THREADS Threads(MAX_THREADS);


void MyErrExit(LPCTSTR msg)
{
	MessageBox(NULL, msg, TEXT("Error"), 0);
	exit(0);
}


HINSTANCE hInst = NULL;

CNotifyIconEx* nix      = NULL;

BOOL CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			nix  = new CNotifyIconEx(hDlg, IDI_ICON2, WM_NOTIFYICON);
			BOOL sdf = nix->Modify(GetModuleHandle(NULL), IDI_ICON2, TEXT("sdfsdfsdf"));
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCLOSE:
			SendMessage(hDlg,WM_COMMAND,IDOK,0);
			break;
		}
		break;
	}
	return 0;
}


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	hInst = hInstance;

	//CreateDialogBox

	//DWORD asd = DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), HWND_DESKTOP, (DLGPROC)DlgProc);

	//HRESULT hr = GetLastError();

	for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
	{
		ZeroMemory(&(*i_thread), sizeof(THREADDESC));
		(*i_thread).Status = PTS_NONE;
	}

	/*
	THREADDESC th1 = Threads[0];
	THREADDESC th2 = Threads[1];
	THREADDESC th3 = Threads[2];
	THREADDESC th4 = Threads[3];
	THREADDESC th5 = Threads[4];
	THREADDESC th6 = Threads[5];
	THREADDESC th7 = Threads[6];
	THREADDESC th8 = Threads[7];
	THREADDESC th9 = Threads[8];
	THREADDESC th10 = Threads[9];
	*/



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
	// The thread's parameter is a handle to a pipe instance. 
	HANDLE hPipe = (HANDLE)lpvParam;

	bool stop_service = false;

	try
	{
		for(;;)
		{ 
			DWORD command;
			GetCommandFromPipe(hPipe, command);
			switch(command)
			{
			case PCC_STOP_SERVICE:
				//SendCommandToPipe(hPipe, PSC_OK);
				stop_service = true;
				throw EndSessionException(TEXT(""));
			case PCC_END_SESSION:
				//SendCommandToPipe(hPipe, PSC_OK);
 				throw EndSessionException(TEXT(""));

			case PCC_ABORT_ALL:
				for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
				{
					if((*i_thread).Status == PTS_PAUSED)
					{
						(*i_thread).Status = PTS_WAITING_FOR_COMMAND;
						ResumeThread((*i_thread).Handle);
					}
					//if(	(((*i_thread).Status == PTS_WAITING_FOR_ANSWER_FILEEXISTS	||
					//	(*i_thread).Status == PTS_WAITING_FOR_ANSWER_FILEREADONLY) ||
					//	(*i_thread).Status == PTS_WAITING_FOR_ANSWER_ERROR))
					{						
						HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, (*i_thread).AnswerEvent);
						if(!hEvent)	
							throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_ALL:CreateEvent"), 0, GetLastError()); 
						if(!SetEvent(hEvent))
							throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_ALL:SetEvent"), 0, GetLastError()); 
						
							(*i_thread).Answer[ANSWER] = PCA_ABORT;
							(*i_thread).ClientCommand = command;
						//SendCommandToPipe(hPipe, PSC_OK);
						continue;
					}
		
				}
				SendCommandToPipe(hPipe, PSC_OK);
				break;
			case PCC_PAUSE_ALL:
				for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
				{
					//if(command == PCC_PAUSE_ALL)
					{
						if((*i_thread).Status == PTS_WAITING_FOR_COMMAND)
						{
							(*i_thread).Status = PTS_PAUSED;
							SuspendThread((*i_thread).Handle);
						}
						else if((*i_thread).Status == PTS_PAUSED)
						{
							(*i_thread).Status = PTS_WAITING_FOR_COMMAND;
							ResumeThread((*i_thread).Handle);
						}
						
					}
					/*if((*i_thread).Status == PTS_PAUSED ||
						((command == PCC_ABORT_ALL) && ((*i_thread).Status == PTS_WAITING_FOR_ANSWER_FILEEXISTS	||
						(*i_thread).Status == PTS_WAITING_FOR_ANSWER_FILEREADONLY) ||
						(*i_thread).Status == PTS_WAITING_FOR_ANSWER_ERROR))
					{						
						HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, (*i_thread).AnswerEvent);
						if(!hEvent)	
							throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_ALL:CreateEvent"), 0, GetLastError()); 
						if(!SetEvent(hEvent))
							throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_ALL:SetEvent"), 0, GetLastError()); 

						if(command == PCC_ABORT_ALL)
						{
							(*i_thread).Answer = PCA_ABORT;
							(*i_thread).ClientCommand = command;
						}
						//SendCommandToPipe(hPipe, PSC_OK);
						continue;
					}
					(*i_thread).ClientCommand = command;*/
				}
				SendCommandToPipe(hPipe, PSC_OK);
				break;
			case PCC_ABORT_THREAD:
				{
					HANDLE hThread;
					GetHandleFromPipe(hPipe, hThread);
					THREADDESC& thrddsc = GetThreadByHandle(Threads, hThread);

					if(thrddsc.Status == PTS_PAUSED)
					{
						thrddsc.Status = PTS_WAITING_FOR_COMMAND;
						ResumeThread(thrddsc.Handle);
					}

					{						
						HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, thrddsc.AnswerEvent);
						if(!hEvent)	
							throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_THREAD:CreateEvent"), 0, GetLastError()); 
						if(!SetEvent(hEvent))
							throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_THREAD:SetEvent"), 0, GetLastError()); 

						thrddsc.Answer[ANSWER] = PCA_ABORT;
						thrddsc.ClientCommand = command;
					}
				}
				SendCommandToPipe(hPipe, PSC_OK);
				break;
			case PCC_PAUSE_THREAD:
				{
					HANDLE hThread;
					GetHandleFromPipe(hPipe, hThread);
					THREADDESC& thrddsc = GetThreadByHandle(Threads, hThread);
					if(command == PCC_PAUSE_THREAD)
					{
						if(thrddsc.Status == PTS_WAITING_FOR_COMMAND)
						{
							thrddsc.Status = PTS_PAUSED;
							SuspendThread(thrddsc.Handle);
						}
						else if(thrddsc.Status == PTS_PAUSED)
						{
							thrddsc.Status = PTS_WAITING_FOR_COMMAND;
							ResumeThread(thrddsc.Handle);
						}
						
					}
					


					/*	if(thrddsc.Status == PTS_PAUSED ||
							((command == PCC_ABORT_THREAD) && (thrddsc.Status == PTS_WAITING_FOR_ANSWER_FILEEXISTS	||
															   thrddsc.Status == PTS_WAITING_FOR_ANSWER_FILEREADONLY) ||
															   thrddsc.Status == PTS_WAITING_FOR_ANSWER_ERROR))
						{						


							HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, thrddsc.AnswerEvent);
							if(!hEvent)	
								throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_THREAD:CreateEvent"), 0, GetLastError()); 
							if(!SetEvent(hEvent))
								throw PipeErrorException(TEXT("InstanceThread:PCC_PAUSE_THREAD:SetEvent"), 0, GetLastError()); 

							if(command == PCC_ABORT_THREAD)
							{
								thrddsc.Answer = PCA_ABORT;
								thrddsc.ClientCommand = command;
							}
							SendCommandToPipe(hPipe, PSC_OK);
							break;
						}*/
					//}
					//if
					//thrddsc.ClientCommand = command;
					SendCommandToPipe(hPipe, PSC_OK);
				}
				break;
			case PCC_TAKE_ANSWER:
				{
					HANDLE hThread;
					GetHandleFromPipe(hPipe, hThread);
					
					for(THREADS::iterator i_thread = Threads.begin(); i_thread != Threads.end(); ++i_thread)
					{
						if((*i_thread).Handle == hThread)
						{
							GetCommandFromPipe(hPipe, (*i_thread).Answer[ANSWER]);
						}
					}
					SendCommandToPipe(hPipe, PSC_OK);
				}
				break;
			case PCC_GET_STATUS:
				SendStatusToPipe(hPipe, Threads);
				break;
			case PCC_TAKE_XML:
				{
					char* xmlBuffer;
					DWORD xmlBufferSize;
					GetXMLFromPipe(hPipe, xmlBuffer, xmlBufferSize);

					HANDLE thread = NULL; 

					if(CreateInstallThread(xmlBuffer, xmlBufferSize, thread) != S_OK)
						MessageBox(NULL, TEXT("CreateInstallThread::Error"), TEXT("PipeServer"), 0);

					SendHandleToPipe(hPipe, thread);
					SendCommandToPipe(hPipe, PSC_OK);
				}
				break;
			default:
				break;
			}
		}
	}
	catch(PipeErrorException peex)
	{
		//SendCommandToPipe(hPipe, PSC_BAD);
		if(peex.last_error != ERROR_BROKEN_PIPE)
			ErrorLookup(peex.msg, peex.last_error);
	}
	catch(EndSessionException esex)
	{
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 
	FlushFileBuffers(hPipe); 
	DisconnectNamedPipe(hPipe); 
	CloseHandle(hPipe); 
	if(stop_service)
		exit(0);
}

DWORD WaitForUserAnswer(THREADDESC* thrddsc, int target)
{
	if(!thrddsc)
		return S_FALSE;

	HRESULT hr = S_OK;

	std::basic_stringstream<TCHAR> strstrm;
	strstrm << TEXT("Global\\giUserAnswerEvent") << thrddsc->Handle;
	HANDLE hEvent = CreateEvent(NULL,FALSE, FALSE, strstrm.str().c_str());
	if(!hEvent)
		return GetLastError();
	lstrcpy(thrddsc->AnswerEvent, strstrm.str().c_str());
	thrddsc->Status = target;
	hr = WaitForSingleObject(hEvent, INFINITE);
	thrddsc->Status = PTS_WAITING_FOR_COMMAND;
	SetEvent(hEvent);
	if(hr == WAIT_FAILED)
		return GetLastError();
	if(!ResetEvent(hEvent))
		return GetLastError();
	if(!CloseHandle(hEvent))
		return GetLastError();
  	
	return hr;
}

DWORD CALLBACK InstallCallback(DWORD msg,const TCHAR* str,LONG P1,LONG P2, LPVOID userdata)
{		
	THREADDESC* thrddsc = static_cast<THREADDESC*>(userdata);

	if(thrddsc)
	{	
	/*	if(thrddsc->ClientCommand == PCC_PAUSE_THREAD || thrddsc->ClientCommand == PCC_PAUSE_ALL)
		{

			//CreateThread
			WaitForUserAnswer(thrddsc, PTS_PAUSED);
			//thrddsc->ClientCommand = 0;
		}*/
		if(thrddsc->ClientCommand == PCC_ABORT_THREAD || thrddsc->ClientCommand == PCC_ABORT_ALL)
		{
			thrddsc->Status = PTS_ABORTING;
			return AFA_ABORT;
		}
		//thrddsc->ClientCommand = 0;
		
	}

	switch(msg)
	{
	case AFCMSG_CO_PROGRESS:
		if(thrddsc)
            thrddsc->CurrentOperationProgress = P1;
		break;
	case AFCMSG_COFROM:
		if(thrddsc)
		{
			ZeroMemory(thrddsc->CurrentOperationFrom, MAX_PATH*sizeof(TCHAR));
            lstrcpyn(thrddsc->CurrentOperationFrom, str, MAX_PATH);
		}
		break;
	case AFCMSG_COTO:
		if(thrddsc)
		{
			ZeroMemory(thrddsc->CurrentOperationTo, MAX_PATH*sizeof(TCHAR));
            lstrcpyn(thrddsc->CurrentOperationTo, str, MAX_PATH);
		}
		break;
	case AFCMSG_CO:
		if(thrddsc)
		{
			thrddsc->CurrentOperationType = P1;
			ZeroMemory(thrddsc->Operation, MAX_PATH*sizeof(TCHAR));
            lstrcpyn(thrddsc->Operation, str, MAX_PATH);
		}
		break;
	case AFCMSG_JOBNAME:
		if(thrddsc)
		{			
			ZeroMemory(thrddsc->JobName, MAX_PATH*sizeof(TCHAR));
            lstrcpyn(thrddsc->JobName, str, MAX_PATH);
		}
		break;
	case AFCMSG_REQUEST:
		 switch(P1)
		 {
		 case AFR_FILEEXISTS:
			 if(!(thrddsc->All[ANSWER_FILE_EXISTS]))
			 {
				 WaitForUserAnswer(thrddsc, PTS_WAITING_FOR_ANSWER_FILEEXISTS);
				 thrddsc->Answer[ANSWER_FILE_EXISTS] = thrddsc->Answer[ANSWER];
				 if(thrddsc->Answer[ANSWER_FILE_EXISTS] & HIGH_BIT)
				 {
					 thrddsc->All[ANSWER_FILE_EXISTS] = TRUE;
					 thrddsc->Answer[ANSWER_FILE_EXISTS] &= ~HIGH_BIT;
				 }
			 }
			 switch(thrddsc->Answer[ANSWER_FILE_EXISTS])
			 {
				case PCA_OK:		/*MessageBox(NULL, TEXT("OK"), TEXT(""), 0);*/return AFA_OK;
				case PCA_ABORT:		/*MessageBox(NULL, TEXT("Abort"), TEXT(""), 0);*/return AFA_ABORT;
				case PCA_SKIP:		/*MessageBox(NULL, TEXT("Skpi"), TEXT(""), 0);*/return AFA_SKIP;
				case PCA_OVERWRITE:	/*MessageBox(NULL, TEXT("Overwrite"), TEXT(""), 0);*/return AFA_OVERWRITE;
				case PCA_REFRESH:	/*MessageBox(NULL, TEXT("Refresh"), TEXT(""), 0);*/return AFA_REFRESH;
				case PCA_APPEND:	/*MessageBox(NULL, TEXT("Append"), TEXT(""), 0);*/return AFA_APPEND;
				case PCA_DELETE:	/*MessageBox(NULL, TEXT("OKDelete"), TEXT(""), 0);*/return AFA_DELETE;
				case PCA_MOVE:		/*MessageBox(NULL, TEXT("Move"), TEXT(""), 0);*/return AFA_MOVE;
			 };				 
			 break;
		 case AFR_FILEREADONLY:
			 //MessageBox(NULL, TEXT("File is Read-only!"), _TEXT("Request"), 0);
			 if(!(thrddsc->All[ANSWER_FILE_READONLY]))
			 {
				 WaitForUserAnswer(thrddsc, PTS_WAITING_FOR_ANSWER_FILEREADONLY);
				 thrddsc->Answer[ANSWER_FILE_READONLY] = thrddsc->Answer[ANSWER];
				 if(thrddsc->Answer[ANSWER_FILE_READONLY] & HIGH_BIT)
				 {
					 thrddsc->All[ANSWER_FILE_READONLY] = TRUE;
					 thrddsc->Answer[ANSWER_FILE_READONLY] &= ~HIGH_BIT;
				 }
			 }
			 
			 switch(thrddsc->Answer[ANSWER_FILE_READONLY])
			 {
				case PCA_ABORT:		/*MessageBox(NULL, TEXT("Abort"), TEXT(""), 0);*/return AFA_ABORT;
				case PCA_SKIP:		/*MessageBox(NULL, TEXT("Skpi"), TEXT(""), 0);*/return AFA_SKIP;
				case PCA_OVERWRITE:	/*MessageBox(NULL, TEXT("Overwrite"), TEXT(""), 0);*/return AFA_OVERWRITE;
			 };		
			 break;
		 case AFR_ERROR:
			 //MessageBox(NULL, TEXT("File is Read-only!"), _TEXT("Request"), 0);
			 if(!(thrddsc->All[ANSWER_ERROR]))
			 {
				 thrddsc->CurrentError = P2;
				 WaitForUserAnswer(thrddsc, PTS_WAITING_FOR_ANSWER_ERROR);
				 thrddsc->Answer[ANSWER_ERROR] = thrddsc->Answer[ANSWER];
				 if(thrddsc->Answer[ANSWER_ERROR] & HIGH_BIT)
				 {
					 thrddsc->All[ANSWER_ERROR] = TRUE;
					 thrddsc->Answer[ANSWER_ERROR] &= ~HIGH_BIT;
				 }
			 }
			 
			 switch(thrddsc->Answer[ANSWER_ERROR])
			 {
				case PCA_RETRY:		/*MessageBox(NULL, TEXT("Abort"), TEXT(""), 0);*/return AFA_RETRY;
				case PCA_ABORT:		/*MessageBox(NULL, TEXT("Abort"), TEXT(""), 0);*/return AFA_ABORT;
				case PCA_SKIP:		/*MessageBox(NULL, TEXT("Skpi"), TEXT(""), 0);*/return AFA_SKIP;
			 };		
			 break;
		 }
		 return -1;
	case AFCMSG_FATAL:
	     MessageBox(NULL, str, _TEXT("Fatal error"), 0);
		 break;
	case AFCMSG_ERROR:
	     MessageBox(NULL, str, _TEXT("Error"), 0);
		 break;
    case AFCMSG_RESULT:
	     MessageBox(NULL, str, _TEXT("Result"), 0);
		 break;
	case AFCMSG_WARNING:
	     MessageBox(NULL, str, _TEXT("Warning"), 0);
		 break;
	};

	return S_OK;
}



VOID InstallThread(LPVOID lpvParam) 
{
	HRESULT hr = S_OK;
	THREADDESC *thrddsc = (THREADDESC*)lpvParam;
	
	thrddsc->Status = PTS_WAITING_FOR_COMMAND;
	thrddsc->ClientCommand = 0;

	//nix->Modify(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2), TEXT("sdfsdfsdf"));
	
	//MessageBox(NULL, TEXT("InstallThread"), TEXT(""), 0);
/*	std::ifstream ifs("D:\\krutila\\prg\\GameInstaller\\script3.xml", std::ifstream::in | std::ifstream::binary);
	//std::ifstream ifs("C:\\Program Files\\Common Files\\Microsoft Shared\\VSA\\7.1\\VsaEnv\\_vsaenv.xml");
	
	std::istreambuf_iterator<char> b1(ifs);
	std::istreambuf_iterator<char> b2;
	std::string buffer(b1, b2);
*/			
	hr = InstallFromXMLBuffer(thrddsc->xmlBuffer, thrddsc->xmlBufferSize, InstallCallback, lpvParam);


	ZeroMemory(thrddsc->CurrentOperationFrom, MAX_PATH*sizeof(TCHAR));
	ZeroMemory(thrddsc->CurrentOperationTo, MAX_PATH*sizeof(TCHAR));
	ZeroMemory(thrddsc->Operation, MAX_PATH*sizeof(TCHAR));
	ZeroMemory(thrddsc->AnswerEvent, MAX_PATH*sizeof(TCHAR));
	
	if(hr != S_OK)
		MessageBox(NULL, TEXT("InstallFromXMLBuffer::Error"), TEXT("PipeServer"), 0);

	if(thrddsc->xmlBuffer)
		delete[] thrddsc->xmlBuffer;

	thrddsc->Status = PTS_NONE;
	thrddsc->CurrentOperationType = 0;
}

DWORD CreateInstallThread(const char* xmlBuffer, DWORD xmlBufferSize, HANDLE& thread)
{
	HANDLE hThread;
	THREADS::iterator mythread;

	for(mythread = Threads.begin(); mythread != Threads.end(); ++mythread)
	{
		if((*mythread).Status == PTS_NONE)
			break;
	}

	if(mythread == Threads.end())
	{
		MessageBox(NULL, TEXT("To Log: максимум потоков"), TEXT("Error"), 0);
		return S_FALSE;
	}
	(*mythread).CurrentOperationProgress = 0;
	(*mythread).FullProgress = 0;
	(*mythread).Answer[0] = 0;
	(*mythread).Answer[1] = 0;
	(*mythread).Answer[2] = 0;
	(*mythread).Answer[3] = 0;

	(*mythread).All[0] = FALSE;
	(*mythread).All[1] = FALSE;
	(*mythread).All[2] = FALSE;
	(*mythread).All[3] = FALSE;
	
	(*mythread).xmlBuffer = xmlBuffer;
	(*mythread).xmlBufferSize = xmlBufferSize;

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

	(*mythread).Handle = hThread;
	thread = hThread;

	/*HANDLE hThreadStartedEvent = CreateEventW(NULL, TRUE, FALSE, L"ThreadStartedEvent");
	if(!hThreadStartedEvent)	
		return S_FALSE;

	if(!SetEvent(hThreadStartedEvent))
	{
		CloseHandle(hThreadStartedEvent);
		return S_FALSE;
	}
	if(!ResetEvent(hThreadStartedEvent))
	{
		CloseHandle(hThreadStartedEvent);
		return S_FALSE;
	}

	CloseHandle(hThreadStartedEvent);*/
	return S_OK;
}

