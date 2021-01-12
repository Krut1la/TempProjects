#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "ProgressDialog.h"
#include "ConfirmDialogs.h"
#include "Status.h"

extern struct PluginStartupInfo Info;

const int DlgW = 76;
const int DlgH = 15;

const int ProgressBarW = DlgW-15;

extern  HANDLE	hGetStatusEvent;
extern	bool	stopTimerThreadEvent;

extern HANDLE hMainPipe;

bool progress_thread_started = false;



HANDLE hDisplayThread = NULL;

DWORD OpenProgressDialog(HANDLE thread)
{	
	progress_thread_started = false;
	hDisplayThread = thread;

	struct FarDialogItem DialogItems[IPLast];

	static const struct InitDialogItem InitItems [] = {
	DI_DOUBLEBOX,	3,				1,				DlgW-4,		DlgH-2,		0, SProgCaption,
	DI_TEXT,		5,				2,				DlgW-7,		0,		0,SNull,
	DI_TEXT,		5,				3,				DlgW-7,		0,		0,SNull,
	DI_TEXT,		5,				4,				DlgW-7,		0,		0,SProgSTo,
	DI_TEXT,		5,				5,				DlgW-7,		0,		0,SNull,
	DI_TEXT,		0,				6,				DlgW-1,		0,		DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
	DI_TEXT,		10,				7,				25,			0,		0,SProgSProcessed,
	DI_TEXT,		35,				7,				25,			0,		0,SProgSTotal,
	DI_TEXT,		55,				7,				25,			0,		0,SProgSSpeed,

	DI_TEXT,		5,				DlgH-7,			DlgW-7,		0,		0,SNull,
	DI_TEXT,		5,				DlgH-5,			DlgW-7,		0,		0,SNull,
	DI_TEXT,		0,				DlgH-4,			DlgW-1,		0,		DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
	DI_BUTTON,		DlgW/2 - 15,	DlgH-3,			0,			0,		0, SProgBOk,
	DI_BUTTON,		DlgW/2 - 6,		DlgH-3,			0,			0,		0, SProgBAbort,
	DI_BUTTON,		DlgW/2 + 5,		DlgH-3,			0,			0,		0, SProgBPause,
	};
	
	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

	
	if(InitStatus() != S_OK)
	{		
		CloseStatus();
		return S_FALSE;
	}
	
	Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, ProgressDlgProc,0);

	CloseStatus();

	return S_OK;
}


long WINAPI ProgressDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{	

	static int PrevFocused;
	static int bLBoxPosChanged;

	long FocusedControl;

	FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

	switch (Msg) {
	case DN_BTNCLICK:
		switch(Param1)
		{
		case IPPause:
		case IPAbort:
			if(AbortThread(hMainPipe, hDisplayThread,
				(Param1 == IPPause),
 				false))
			{
				HRESULT hr = GetLastError();
				if(hr != S_OK)
					ErrorLookup("GetThreadStatus", hr);
				Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
			}
			break;
		case IPOk:
			Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
			break;
		}
		break;

	case DN_INITDIALOG:
		hMainPipe = OpenPipe();

		if(!hMainPipe)
		{
			HRESULT hr = GetLastError();
			if(hr != S_OK)
				ErrorLookup("DN_INITDIALOG", hr);
			Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
		}
		break;
	case DN_KEY:
		//if(Param2 == 0x07000008)
		if(Param2 == VK_BACK)
		
		{
			if(RefreshStatus(hDlg) != S_OK)
			{
				//FlushConsoleInputBuffer(hConsole);
				//stopTimerThreadEvent = true;

				HRESULT hr = GetLastError();
				if(hr != S_OK)
					ErrorLookup("GetThreadStatus", hr);
				Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
				break;
			}
			//return FALSE;
		}
		else if(Param2 == KEY_ESC)
		{
			Info.SendDlgMessage(hDlg, DN_BTNCLICK, IPAbort, 0);
			break;
		}
		//SetEvent(hGetStatusEvent);
		break;

	case DN_CLOSE:
		if(Param1 < 0 || Param1 == IPOk || Param1 == 100)
		{
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
		//else
		//	return FALSE;
	}

	SetEvent(hGetStatusEvent);
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

std::string MakeProgressBarString(DWORD progress)
{
	std::stringstream strstrm;

	if(progress < 100)	strstrm << " ";
	if(progress < 10)	strstrm << " ";
	strstrm << progress << "% ";
	char progstr[100] = {0};
	int indicator = (ProgressBarW*progress) / 100;
	lstrcpyn(progstr, GetMsg(SProgBarIndicator), indicator + 1);
	int background =  ProgressBarW - indicator;
	lstrcpyn(progstr + indicator, GetMsg(SProgBarBackground), background + 1);
	
	strstrm << progstr;
	
	return strstrm.str();
}

DWORD RefreshStatus(HANDLE hDlg)
{
	//HRESULT hr;

//	HANDLE hPipe = OpenPipe();

//	if(!hPipe)
//		return S_FALSE;

	//try
	//{
		THREADS threads;
		if(GetThreadStatus(hMainPipe, threads) != S_OK)
		{
			SetLastError(S_OK);
			return S_FALSE;
		}

		THREADS::iterator i_thread;
		for(i_thread = threads.begin(); i_thread != threads.end(); ++i_thread)
		{
			if((*i_thread).Handle == hDisplayThread)
				break;
		}
		if(i_thread == threads.end())
		{
			SetLastError(S_OK);
			return S_FALSE;
		}
		SetItemTextA(hDlg, DM_SETTEXTPTR, IPFProgress, MakeProgressBarString((*i_thread).FullProgress).c_str());
		SetItemTextA(hDlg, DM_SETTEXTPTR, IPCOProgress, MakeProgressBarString((*i_thread).CurrentOperationProgress).c_str());

		std::wstring cofrom = (*i_thread).CurrentOperationFrom;
		if(cofrom.length() > DlgW-16)
		{
			size_t f_slash = cofrom.find_first_of(L"\\", 0);
			cofrom.replace(f_slash + 1, cofrom.length() - DlgW+19, L"...");
		}

		std::wstring coto = (*i_thread).CurrentOperationTo;
		if(coto.length() > DlgW-16)
		{
			size_t f_slash = coto.find_first_of(L"\\", 0);
			coto.replace(f_slash + 1, coto.length() - DlgW+19, L"...");
		}

		std::stringstream  strstrm;
		strstrm << GetMsg(SProgSProcessed) << " " << (*i_thread).TotalBytesProcessed;
		SetItemTextA(hDlg, DM_SETTEXTPTR, IPSProcessed, strstrm.str().c_str());

		strstrm.str("");
		strstrm << GetMsg(SProgSTotal) << " " << (*i_thread).TotalBytes;
		SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTotal, strstrm.str().c_str());

		strstrm.str("");
		strstrm << GetMsg(SProgSSpeed) << " " << (*i_thread).CurrentSpeed;
		SetItemTextA(hDlg, DM_SETTEXTPTR, IPSSpeed, strstrm.str().c_str());

		switch((*i_thread).CurrentOperationType)
		{
		case AFOT_COPY:
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSFrom, GetMsg(SProgSCopingFile));
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTo, GetMsg(SProgSTo));

			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationFrom, (wchar_t*)cofrom.c_str());
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationTo, (wchar_t*)coto.c_str());
			break;
		case AFOT_MOVE:
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSFrom, GetMsg(SProgSMovingFile));
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTo, GetMsg(SProgSTo));
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationFrom, (wchar_t*)cofrom.c_str());
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationTo, (wchar_t*)coto.c_str());
			break;
		case AFOT_DELETE:
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSFrom, GetMsg(SProgSDeletingFile));
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTo, GetMsg(SNull));
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationFrom, (wchar_t*)cofrom.c_str());
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationTo, L"");
			break;
		case AFOT_COMPARE:
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSFrom, GetMsg(SProgSComparingFile));
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTo, GetMsg(SProgSWith));
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationFrom, (wchar_t*)cofrom.c_str());
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationTo, (wchar_t*)coto.c_str());
			break;
		case AFOT_EXTRACT:
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSFrom, GetMsg(SProgSExtractingArchive));
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTo, GetMsg(SProgSTo));
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationFrom, (wchar_t*)cofrom.c_str());
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationTo, (wchar_t*)coto.c_str());
			break;
		case AFOT_SCAN:
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSFrom, GetMsg(SProgSScaning));
			SetItemTextA(hDlg, DM_SETTEXTPTR, IPSTo, GetMsg(SNull));
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationFrom, (wchar_t*)cofrom.c_str());
			SetItemTextW(hDlg, DM_SETTEXTPTR, IPCOperationTo, L"");
			break;

		}


		switch((*i_thread).Status)
		{
		case PTS_WAITING_FOR_ANSWER_FILEEXISTS:
		case PTS_WAITING_FOR_ANSWER_FILEREADONLY:
		case PTS_WAITING_FOR_ANSWER_ERROR:
			{
				int res = 0;
				if((*i_thread).Status == PTS_WAITING_FOR_ANSWER_FILEEXISTS)
				{				
					res = OpenFileExistsDialog((*i_thread).CurrentOperationFrom, (*i_thread).CurrentOperationTo);
					DWORD All = (res >= 200)?HIGH_BIT:0;

					switch((res >= 200)?(res - 200):(res - 100))
					{					
					case IFEBOverwrite:	(*i_thread).Answer[ANSWER] = PCA_OVERWRITE|All;	break;
					case IFEBSkip:		(*i_thread).Answer[ANSWER] = PCA_SKIP|All;		break;
					case IFEBRefresh:	(*i_thread).Answer[ANSWER] = PCA_REFRESH|All;	break;
					case IFEBAppend:	(*i_thread).Answer[ANSWER] = PCA_APPEND|All;	break;
					case -1:
					case IFEBAbort:
					default:
						(*i_thread).Answer[ANSWER] = PCA_ABORT|All;		break;
					};
				}
				else if((*i_thread).Status == PTS_WAITING_FOR_ANSWER_FILEREADONLY)
				{
					res = OpenFileReadOnlyDialog((*i_thread).Operation);
					DWORD All = (res >= 200)?HIGH_BIT:0;

					switch((res >= 200)?(res - 200):(res - 100))
					{

					case IFROBOverwrite:	(*i_thread).Answer[ANSWER] = PCA_OVERWRITE|All;	break;
					case IFROBSkip:		(*i_thread).Answer[ANSWER] = PCA_SKIP|All;		break;
					case -1:
					case IFROBAbort:
					default:
						(*i_thread).Answer[ANSWER] = PCA_ABORT|All;		break;
					};				
				}
				else if((*i_thread).Status == PTS_WAITING_FOR_ANSWER_ERROR)
				{
					std::wstring error_text;
					LPWSTR s;

					// получаем текстовое описание ошибки
					BOOL fOk = FormatMessageW(
						FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
						NULL, (*i_thread).CurrentError, 0,
						(LPWSTR)&s, 0, NULL);

					if (s != NULL) {
						error_text = s;
						LocalFree(s);
					} else {
						error_text = L"Error number not found.";
					}

					res = OpenErrorDialog((*i_thread).CurrentOperationTo, error_text.c_str());
					DWORD All = (res >= 200)?HIGH_BIT:0;

					switch((res >= 200)?(res - 200):(res - 100))
					{					
					case IERBRetry:		(*i_thread).Answer[ANSWER] = PCA_RETRY|All;	break;
					case IERBIgnore:	(*i_thread).Answer[ANSWER] = PCA_SKIP|All;	break;
					case -1:
					case IERBAbort:
					default:
						(*i_thread).Answer[ANSWER] = PCA_ABORT|All;		break;
					};				
				}

				SendCommandToPipe(hMainPipe, PCC_TAKE_ANSWER);
				SendHandleToPipe(hMainPipe, (*i_thread).Handle);
				SendCommandToPipe(hMainPipe, (*i_thread).Answer[ANSWER]);
				//SendCommandToPipe(hMainPipe, PCC_END_SESSION);

				DWORD server_confirm = 0;
				GetCommandFromPipe(hMainPipe, server_confirm);

				if(server_confirm != PSC_OK)
					return S_FALSE;
				//throw ClosePipeException(TEXT(""));

				HANDLE hEvent = CreateEventW(NULL, FALSE, FALSE, (*i_thread).AnswerEvent);
				if(!hEvent)	
					return S_FALSE;
				if(!SetEvent(hEvent))
					return S_FALSE;

				WaitForSingleObject(hEvent, INFINITE);
				//hEvent
			}
			break;
		}

	/*}
	catch(PipeErrorException peex)
	{
		ErrorLookup(peex.msg, peex.last_error);
	}
	catch(ClosePipeException cpex)
	{
		CloseHandle(hPipe);
		return S_FALSE;
	}

	CloseHandle(hPipe);*/
	
	return S_OK;
}