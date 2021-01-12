#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "ArchivePanel.h"
#include "OpenArchiveDialog.h"
#include "Status.h"

const int BUFSIZE = 4096;
const int PIPE_TIMEOUT = 300;



extern struct PluginStartupInfo Info;

extern  HANDLE	hGetStatusEvent;
extern	bool	stopTimerThreadEvent;

extern HANDLE hMainPipe;

HANDLE hWaitForFileListThread = NULL;

//Handle from other process!
HANDLE hOpenArchiveThread = NULL;

int DlgW = 60;
const int DlgH = 7;

const char* name;

bool thread_started = false;


ArchivePanel* archive_panel;

int OpenOpenArchiveDialog(const char* filename, ArchivePanel* ap)
{	
	hOpenArchiveThread = NULL;
	thread_started = false;

	archive_panel = ap;
	name = filename;

	DlgW = strlen(filename) + 10;
	
	struct FarDialogItem DialogItems[IOALast];

	const struct InitDialogItem InitItems [] = {
			DI_DOUBLEBOX,	3,				1,			DlgW-4,		DlgH-2,		0,	SOpenArchiveCaption,

			DI_TEXT,		5,				2,			DlgW-7,		0,			0,SNull,
			DI_TEXT,		5,				3,			DlgW-7,		0,			0,SNull,
			DI_TEXT,		(DlgW/2)-3,				4,			0,		0,			0,SNull,
			
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

	if(InitStatus() != S_OK)
	{		
		CloseStatus();
		return S_FALSE;
	}

	MakeOpenArchiveXML();
	
	if(WaitForSingleObjectEx(hWaitForFileListThread, 1000, TRUE) != WAIT_TIMEOUT)
		return 0;


	thread_started = true;
	
	
	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, OpenArchiveDlgProc,0);

	CloseStatus();
}

long WINAPI OpenArchiveDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
	static int PrevFocused;
	static int bLBoxPosChanged;

  
//  MOUSE_EVENT_RECORD m_record;
	long FocusedControl;

  // Запомним активный элемент диалога
  FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

  switch (Msg) {
	case DN_BTNCLICK:
		switch(Param1)
		{

		}
		break;
    case DN_CTLCOLORDLGITEM:
      break;

	case DN_INITDIALOG:
		//std::wstring source = get_full_wide_name(name);
		hMainPipe = OpenPipe();

		if(!hMainPipe)
		{
			HRESULT hr = GetLastError();
			if(hr != S_OK)
				ErrorLookup("DN_INITDIALOG", hr);
			Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
			break;
		}
		
		SetItemTextA(hDlg, DM_SETTEXTPTR, IOAArchive, name);
		
		return FALSE;

	case DN_KEY:
		//if(Param2 == 0x07000008 && thread_started)
		if(Param2 == VK_BACK && thread_started)
		{

			if(RefreshOpenArchiveInfoDialog(hDlg) != S_OK)
			{
				HRESULT hr = GetLastError();
				if(hr != S_OK)
					ErrorLookup("GetThreadStatus", hr);
				Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
				break;
			}
		}
		break;
    case DN_KILLFOCUS:
      break;
    
    case DN_DRAWDLGITEM:
      break;
  
	case DN_CLOSE:
		if(Param1 < 0 || Param1 == 100 || Param1 >= 101)
		{
			//WaitForSingleObjectEx(hWaitForFileListThread, INFINITE, TRUE);
			CloseHandle(hWaitForFileListThread);
			stopTimerThreadEvent = true;
			SetEvent(hGetStatusEvent);

			if(hMainPipe)
			{
				SendCommandToPipe(hMainPipe, PCC_END_SESSION);
				CloseHandle(hMainPipe);
				hMainPipe = NULL;
			}
			return TRUE;
		}
		break;
  }

  SetEvent(hGetStatusEvent);
  // Передадим управление стандартному обработчику
  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

DWORD WaitForFileListThread(LPVOID lpvParam)
{
	const wchar_t* pipe_name = (const wchar_t*)lpvParam;

	HANDLE hPipe = CreateNamedPipeW( 
			pipe_name,				  // pipe name 
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
		return S_FALSE;

	BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ? 
          TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

	if (fConnected) 
	{

		//FILELIST filelist;

		DWORD cbBytesRead;
		BOOL fSuccess;

		size_t files_num = 0;

		fSuccess = ReadFile( 
			hPipe,
			&files_num,
			sizeof(size_t),
			&cbBytesRead,
			NULL);

		if (!fSuccess || cbBytesRead == 0) 
			return S_FALSE;

		//filelist.reserve(files_num);
		archive_panel->files.resize(files_num);

		fSuccess = ReadFile( 
			hPipe,
			&archive_panel->files[0],
			(DWORD)(sizeof(FILEDESC)*files_num),
			&cbBytesRead,
			NULL);

		if (!fSuccess || cbBytesRead == 0) 
			return S_FALSE;
        
	}
	else 
		// The client could not connect, so close the pipe. 
		CloseHandle(hPipe); //return S_FALSE ?

	return S_OK;
}


DWORD MakeOpenArchiveXML()
{
	std::wstring xmlBuffer;
	std::wstring xmlHeader = L"<script>\n";
	std::wstring xmlFooter = L"</script>";

	std::wstring source = get_full_wide_name(name);

	if(source[source.length() - 1] == '\\')
		source.erase(source.length() - 1, 1);


	xmlBuffer += xmlHeader;


	std::wstring xmlElement;
	xmlElement = L"<GetFileList source=\"";
	//xmlElement = L"<ExtractSevenzipArchive source=\"";

	xmlElement += source + L"\" destination=\"";

	std::wstring pipe_sufix = source;
	std::wstring pipe_name = L"\\\\.\\pipe\\gipipe_";

	std::replace(pipe_sufix.begin(), pipe_sufix.end(), L'\\', L'_');
	std::replace(pipe_sufix.begin(), pipe_sufix.end(), L'/', L'_');
	std::replace(pipe_sufix.begin(), pipe_sufix.end(), L':', L'_');

	if(pipe_sufix.length() > 10)
		pipe_sufix.erase(0, pipe_sufix.length() - 10);

	pipe_name += pipe_sufix;

	xmlElement += pipe_name;
	xmlElement += L"\" type=\"rar";
	xmlElement += L"\" flags=\"none\"/>\n";


	xmlBuffer += xmlElement;
	xmlBuffer += xmlFooter;


	hWaitForFileListThread = CreateThread( 
				NULL,              // no security attribute 
				0,                 // default stack size 
				(LPTHREAD_START_ROUTINE) WaitForFileListThread, 
				(LPVOID)pipe_name.c_str(),   // thread parameter 
				0,                 // not suspended 
				NULL);      // returns thread ID 
	
	if (hWaitForFileListThread == NULL) 
		return S_FALSE;
		

	//MessageBoxW(NULL, xmlBuffer.c_str(), L"Test", 0);


	HANDLE hPipe = OpenPipe();
	if(!hPipe)
		return S_FALSE;

	try
	{
		SendCommandToPipe(hPipe, PCC_TAKE_XML);
		SendXMLToPipe(hPipe, (char*)xmlBuffer.c_str(), (xmlBuffer.length())*sizeof(wchar_t));
		//SendXMLToPipe(hPipe, xmlBuffer.c_str(), xmlBuffer.length());

		GetHandleFromPipe(hPipe, hOpenArchiveThread);

		DWORD server_confirm = 0;
		GetCommandFromPipe(hPipe, server_confirm);

		if(server_confirm != PSC_OK)
			throw ClosePipeException(TEXT(""));

		
		SendCommandToPipe(hPipe, PCC_END_SESSION);

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
		CloseHandle(hPipe);
		return S_FALSE;
	}

	CloseHandle(hPipe);
	return S_OK;
}


DWORD RefreshOpenArchiveInfoDialog(HANDLE hDlg)
{	
	//HANDLE hPipe = OpenPipe();

	//if(!hPipe)
		//return S_FALSE;

	try
	{
		THREADS threads;
		if(GetThreadStatus(hMainPipe, threads) != S_OK)
		{
			SetLastError(S_OK);
			return S_FALSE;
		}

		//THREADDESC thrddsc = GetThreadByHandle(threads, hOpenArchiveThread);
		THREADS::iterator i_thread;
		for(i_thread = threads.begin(); i_thread != threads.end(); ++i_thread)
		{
			if((*i_thread).Handle == hOpenArchiveThread)
				break;
		}
		if(i_thread == threads.end())
		{
			SetLastError(S_OK);
			return S_FALSE;
		}

		std::stringstream strstrm;
		strstrm << (*i_thread).CurrentOperationProgress;
		SetItemTextA(hDlg, DM_SETTEXTPTR, IOAFiles, strstrm.str().c_str());
		
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