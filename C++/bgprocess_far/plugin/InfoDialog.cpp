#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "InfoDialog.h"
#include "Status.h"
#include "ProgressDialog.h"

extern struct PluginStartupInfo Info;

extern  HANDLE	hGetStatusEvent;

extern	bool	stopTimerThreadEvent;

extern HANDLE hMainPipe;

const int DlgW = 76;
const int DlgH = 15;

extern HANDLE hInfoThread;

DWORD OpenInfoDialog()
{	
	hInfoThread = NULL;

	struct FarDialogItem DialogItems[IILast];

	static const struct InitDialogItem InitItems [] = {
	DI_DOUBLEBOX,	3,				1,			DlgW-4,		DlgH-2,	0, SInfoCaption,
	DI_LISTBOX,		4,				2,			DlgW-5,		DlgH-7,	DIF_LISTNOBOX|DIF_LISTWRAPMODE,SNull,

   	DI_TEXT,		0,				DlgH-4,			DlgW-4,	0,		DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
	DI_CHECKBOX,	5,				DlgH-3,			0,		0,		0, SInfoCBAll,
	DI_BUTTON,		DlgW/2 -20,		DlgH-3,			0,		0,		0, SInfoBShow,
	DI_BUTTON,		DlgW/2 -10,		DlgH-3,			0,		0,		0, SInfoBPause,
	DI_BUTTON,		DlgW/2 +10,		DlgH-3,			0,		0,		0, SInfoBAbort
	};
	
	InitDialogItems(InitItems, DialogItems, IILast);
	

	if(InitStatus() != S_OK)
	{		
		CloseStatus();
		return S_FALSE;
	}
	
	Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems)/sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, InfoDlgProc,0);

	CloseStatus();


	return S_OK;
}


long WINAPI InfoDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
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
		case IIPause:
		case IIAbort:
			{
				int strId = Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, IIListBox, 0);
				HANDLE thread = (HANDLE)Info.SendDlgMessage(hDlg, DM_LISTGETDATA, IIListBox, strId);

				if(AbortThread(hMainPipe, thread,
					(Param1 == IIPause),
					(Info.SendDlgMessage(hDlg, DM_GETCHECK, IIAll, 0) == BSTATE_CHECKED))!= S_OK)
				{
					HRESULT hr = GetLastError();
					if(hr != S_OK)
						ErrorLookup("GetThreadStatus", hr);
					Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
				}
			}
			break;
		case IIShow:
			//stopTimerThreadEvent = true;
			//SetEvent(hGetStatusEvent);
			int strId = Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, IIListBox, 0);
			hInfoThread = (HANDLE)Info.SendDlgMessage(hDlg, DM_LISTGETDATA, IIListBox, strId);
			//InfoThreadId = Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, IIListBox, 0);
			Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
			return TRUE;

		}
		break;
	case DN_CTLCOLORDLGITEM:
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

	case DN_CTLCOLORDLGLIST:
		/*if(Param1 == IIListBox)
		{
			FarListColors *Colors=(FarListColors *)Param2;
			int ColorIndex[]={COL_MENUTEXT,COL_MENUTEXT,COL_MENUTITLE,COL_MENUTEXT,COL_MENUHIGHLIGHT,COL_MENUTEXT,COL_MENUSELECTEDTEXT,COL_MENUSELECTEDHIGHLIGHT,COL_MENUSCROLLBAR,COL_MENUDISABLEDTEXT};
			int Count=sizeof(ColorIndex);
			if(Count>Colors->ColorCount) Count=Colors->ColorCount;
			for(int i=0;i<Count;i++)
				Colors->Colors[i]=Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)(ColorIndex[i]));
			return TRUE;
		}*/
		break;

	case DN_KEY:
		if(Param1 == IIListBox)
		{
			switch(Param2)
			{
			case KEY_DEL:
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, IIAbort, 0);
				break;
			case KEY_SPACE:
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, IIPause, 0);
				break;
			case KEY_ENTER:
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, IIShow, 0);
				//OpenProgressDialog(Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, IIListBox, 0));
				break;
			}
		}
		//if(Param2 == 0x07000008)
		if(Param2 == VK_BACK)
		{

			if(RefreshInfoDialog(hDlg) != S_OK)
			{
				HRESULT hr = GetLastError();
				if(hr != S_OK)
					ErrorLookup("GetThreadStatus", hr);
				Info.SendDlgMessage(hDlg, DM_CLOSE, 100, 0);
				break;
			}
		}
		//SetEvent(hGetStatusEvent);
		break;

	case DN_KILLFOCUS:
		break;

	case DN_CLOSE:
		if(Param1 < 0 || Param1 == 100 || Param1 >= 101)
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
		else
			return FALSE;
		
		break;
	}
        	
	SetEvent(hGetStatusEvent);
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

bool miganie = false;
DWORD RefreshInfoDialog(HANDLE hDlg)
{
	//HRESULT hr;

	THREADS threads;
	if(GetThreadStatus(hMainPipe, threads) != S_OK)
	{
		SetLastError(S_OK);
		return S_FALSE;
	}

	Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, FALSE, 0);

	FarListPos flp = {0, 0};
	
	flp.SelectPos = Info.SendDlgMessage(hDlg, DM_LISTGETCURPOS, IIListBox, 0);
	 

	Info.SendDlgMessage(hDlg, DM_LISTDELETE, IIListBox, (long)NULL);

	//int a = -1;

	//FarList fl;
	//FarListItem fli;
    //fl.ItemsNumber = 1;
	//fl.Items = &fli;
	//fli.
	
	

	for(THREADS::iterator i_thread = threads.begin(); i_thread != threads.end(); ++i_thread)
	{
		std::stringstream strstrm;

		if((*i_thread).FullProgress < 100)	strstrm << " ";
		if((*i_thread).FullProgress < 10)	strstrm << " ";
		strstrm << (*i_thread).FullProgress << "|";
		if((*i_thread).CurrentOperationProgress < 100)	strstrm << " ";
		if((*i_thread).CurrentOperationProgress < 10)	strstrm << " ";
		strstrm << (*i_thread).CurrentOperationProgress << "|";

		switch((*i_thread).CurrentOperationType)
		{
		case AFOT_COPY:
			strstrm << GetMsg(SProgSCopingFileSh);			
			break;
		case AFOT_MOVE:
			strstrm << GetMsg(SProgSMovingFileSh);			
			break;
		case AFOT_DELETE:
			strstrm << GetMsg(SProgSDeletingFileSh);			
			break;
		case AFOT_COMPARE:
			strstrm << GetMsg(SProgSComparingFileSh);			
			break;
		case AFOT_EXTRACT:
			strstrm << GetMsg(SProgSExtractingArchiveSh);			
			break;
		case AFOT_SCAN:
			strstrm << GetMsg(SProgSScaningSh);			
			break;
		}
		strstrm << "|";			

		std::string op = to_multibyte(std::wstring((*i_thread).JobName));
		size_t len = op.length();
		if(op.length() > DlgW-13)
		{
			size_t f_slash = op.find_first_of("\\", 0);
			op.replace(f_slash + 1, op.length() - DlgW+23, "...");
		}
		
		if((*i_thread).Status != PTS_WAITING_FOR_COMMAND)
		{
			if(miganie)
			{
				switch((*i_thread).Status)
				{
				case PTS_WAITING_FOR_ANSWER_FILEEXISTS:
					strstrm << GetMsg(SInfoFileExist);
					break;
				case PTS_WAITING_FOR_ANSWER_FILEREADONLY:
					strstrm << GetMsg(SInfoFileReadOnly);
					break;
				case PTS_WAITING_FOR_ANSWER_ERROR:
					strstrm << GetMsg(SInfoError);
					break;
				case PTS_PAUSED:
					strstrm << GetMsg(SInfoPaused);
					break;
				case PTS_ABORTING:
					strstrm << GetMsg(SInfoAborting);
					break;
				}
			}
			else
				strstrm << op;
		}
		else
			strstrm << op;

		std::string str = strstrm.str();
		//fli.Flags = 0;
		//memset(fli.Text, 0, sizeof(fli.Text));
		//lstrcpyn(fli.Text, str.c_str(), sizeof(fli.Text));
		int strId = Info.SendDlgMessage(hDlg, DM_LISTADDSTR, IIListBox, (long)str.c_str());

		FarListItemData flid;
		flid.Index = strId;
		flid.DataSize = sizeof(HANDLE);
		flid.Data = (LPVOID)(*i_thread).Handle;
		flid.Reserved = 0;

		Info.SendDlgMessage(hDlg, DM_LISTSETDATA, IIListBox, (long)&flid);
	}

	miganie = !miganie;

	Info.SendDlgMessage(hDlg, DM_LISTSETCURPOS, IIListBox, (long)&flp);
	
	Info.SendDlgMessage(hDlg, DM_ENABLEREDRAW, TRUE, 0);

	return S_OK;
}