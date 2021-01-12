#include "Common.h"
#include "ConfirmDialogs.h"


extern struct PluginStartupInfo Info;

const int DlgW = 69;
const int DlgH = 13;

int OpenFileExistsDialog(const wchar_t* source, const wchar_t* destination)
{	
	struct FarDialogItem DialogItems[IFELast];

	static const struct InitDialogItem InitItems [] = {
		DI_DOUBLEBOX,	3,		1,			DlgW-4,		DlgH-2,		0, SMSGWarning,

			DI_TEXT,		5,		2,			DlgW-6,			0,			0, SMSGWCFileExists,
			DI_TEXT,		5,		3,			DlgW-6,			0,			0, SNull,

			DI_TEXT,		0,		4,			DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,

			DI_TEXT,		5,		5,			DlgW-6,		0,			0, SNull,
			DI_TEXT,		5,		6,			DlgW-6,		0,			0, SNull,

			DI_TEXT,		0,		DlgH-6,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
			DI_CHECKBOX,	5,		DlgH-5,		0,			0,			0, SMSGWCBAll,
			DI_BUTTON,		5,		DlgH-3,		0,			0,			0, SMSGWBOverwrite,
			DI_BUTTON,		20,		DlgH-3,		0,			0,			0, SMSGWBSkip,
			DI_BUTTON,		30,		DlgH-3,		0,			0,			0, SMSGWBRefresh,
			DI_BUTTON,		43,		DlgH-3,		0,			0,			0, SMSGWBAppend,
			DI_BUTTON,		55,		DlgH-3,		0,			0,			0, SMSGWBAbort
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

	std::wstring dest = destination;
	if(dest.length() > DlgW-17)
	{
		size_t f_slash = dest.find_first_of(L"\\", 0);
		dest.replace(f_slash + 1, dest.length() - DlgW+19, L"...");
	}
	lstrcpyn(DialogItems[IFEFile].Data, to_multibyte(dest, std::locale(".OCP")).c_str(), MAX_PATH);

	GetFileAttr(source,DialogItems[IFESource].Data,GetMsg(SMSGWCSource));
	GetFileAttr(destination,DialogItems[IFEDestination].Data,GetMsg(SMSGWCDestination));


	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		NULL, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, FileExistsDlgProc,0);
}

long WINAPI FileExistsDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
	static int PrevFocused;
	static int bLBoxPosChanged;



	long FocusedControl;

	// Запомним активный элемент диалога
	FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

	switch (Msg) {
	case DN_BTNCLICK:
		switch(Param1)
		{
		case IFEBOverwrite:
		case IFEBSkip:
		case IFEBAppend:
		case IFEBAbort:
		case IFEBRefresh:
			{
				BOOL all = Info.SendDlgMessage(hDlg, DM_GETCHECK, IFECBAll, 0); 
				int ret = ((all)?200:100) + Param1;
				Info.SendDlgMessage(hDlg, DM_CLOSE, ret, 0);
			}
			return TRUE;
		}
		break;
	case DN_CTLCOLORDIALOG:
		return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);

	case DN_CTLCOLORDLGITEM:
		switch(Param1)
		{
		case IFEDialogBox:
			return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));

		case IFEFileExist:
		case IFESeparator1:
		case IFESeparator2:
		case IFECBAll:
		case IFESource:
		case IFEDestination:
		case IFEFile:
			return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
		case IFEBOverwrite:
		case IFEBSkip:
		case IFEBAppend:
		case IFEBAbort:
		case IFEBRefresh:
			if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == Param1)
				return (Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
			else
				return (Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
		}
		break;

	case DN_INITDIALOG:
		Info.SendDlgMessage(hDlg, DM_SETFOCUS, IFECBAll, 0);
		return FALSE;

	case DN_KEY:
		switch(Param2)
		{
		case KEY_ENTER:
			if(Param1 == IFECBAll)
			{
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, IFEBOverwrite, 0);
				return TRUE;
			}
			return FALSE;
		}
		break;

	case DN_KILLFOCUS:
		break;

	case DN_DRAWDLGITEM:
		break;

	case DN_CLOSE:
		return(Param1 < 0 || Param1 >= 100);
	}

	// Передадим управление стандартному обработчику
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

int OpenFileReadOnlyDialog(const wchar_t* destination)
{	
	struct FarDialogItem DialogItems[IFROLast];

	static const struct InitDialogItem InitItems [] = {
		DI_DOUBLEBOX,	3,		1,			DlgW-4,		DlgH-2,		0, SMSGWarning,

			DI_TEXT,		5,		2,			DlgW-6,			0,			0, SMSGWCFileReadOnly,
			DI_TEXT,		5,		3,			DlgW-6,			0,			0, SNull,

			DI_TEXT,		0,		4,			DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,

			DI_TEXT,		5,		5,			DlgW-6,		0,			0, SNull,

			DI_TEXT,		0,		DlgH-6,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
			DI_CHECKBOX,	5,		DlgH-5,		0,			0,			0, SMSGWCBAll,
			DI_BUTTON,		5,		DlgH-3,		0,			0,			0, SMSGWBOverwrite,
			DI_BUTTON,		20,		DlgH-3,		0,			0,			0, SMSGWBSkip,
			DI_BUTTON,		55,		DlgH-3,		0,			0,			0, SMSGWBAbort
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

	std::wstring dest = destination;
	if(dest.length() > DlgW-17)
	{
		size_t f_slash = dest.find_first_of(L"\\", 0);
		dest.replace(f_slash + 1, dest.length() - DlgW+19, L"...");
	}
	lstrcpyn(DialogItems[IFROFile].Data, to_multibyte(dest, std::locale(".OCP")).c_str(), MAX_PATH);

	GetFileAttr(destination,DialogItems[IFRODestination].Data,GetMsg(SMSGWCDestination));


	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		NULL, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, FileReadOnlyDlgProc,0);
}

long WINAPI FileReadOnlyDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
	static int PrevFocused;
	static int bLBoxPosChanged;

	long FocusedControl;

	// Запомним активный элемент диалога
	FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

	switch (Msg) {
	case DN_BTNCLICK:
		switch(Param1)
		{
		case IFROBOverwrite:
		case IFROBSkip:
		case IFROBAbort:
			{
				BOOL all = Info.SendDlgMessage(hDlg, DM_GETCHECK, IFROCBAll, 0); 
				int ret = ((all)?200:100) + Param1;
				Info.SendDlgMessage(hDlg, DM_CLOSE, ret, 0);
			}
			return TRUE;
		}
		break;
	case DN_CTLCOLORDIALOG:
		return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);

	case DN_CTLCOLORDLGITEM:
		switch(Param1)
		{
		case IFRODialogBox:
			return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));

		case IFROFileReadOnly:
		case IFROSeparator1:
		case IFROSeparator2:
		case IFROCBAll:
		case IFRODestination:
		case IFROFile:
			return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
		case IFROBOverwrite:
		case IFROBSkip:
		case IFROBAbort:
			if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == Param1)
				return (Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
			else
				return (Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
		}
		break;

	case DN_INITDIALOG:
		Info.SendDlgMessage(hDlg, DM_SETFOCUS, IFROCBAll, 0);
		return FALSE;

	case DN_KEY:
		switch(Param2)
		{
		case KEY_ENTER:
			if(Param1 == IFROCBAll)
			{
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, IFROBOverwrite, 0);
				return TRUE;
			}			
			return FALSE;
		}
		break;

	case DN_KILLFOCUS:
		break;

	case DN_DRAWDLGITEM:
		break;

	case DN_CLOSE:
		// Закрываем если нажат Cancel или ESC

		return(Param1 < 0 || Param1 >= 100);
	}

	// Передадим управление стандартному обработчику
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}


int OpenErrorDialog(const wchar_t* destination, const wchar_t* error_text)
{	
	struct FarDialogItem DialogItems[IERLast];

	static const struct InitDialogItem InitItems [] = {
		DI_DOUBLEBOX,	3,		1,			DlgW-4,		DlgH-2,		0, SMSGError,

			DI_TEXT,		5,		2,			DlgW-6,			0,			0, SNull,
			DI_TEXT,		5,		3,			DlgW-6,			0,			0, SNull,

			DI_TEXT,		0,		4,			DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,

			DI_TEXT,		5,		5,			DlgW-6,		0,			0, SNull,

			DI_TEXT,		0,		DlgH-6,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
			DI_CHECKBOX,	5,		DlgH-5,		0,			0,			0, SMSGWCBIgnoreAll,
			DI_BUTTON,		5,		DlgH-3,		0,			0,			0, SMSGWBRetry,
			DI_BUTTON,		20,		DlgH-3,		0,			0,			0, SMSGWBIgnore,
			DI_BUTTON,		55,		DlgH-3,		0,			0,			0, SMSGWBAbort
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

	std::wstring dest = destination;
	if(dest.length() > DlgW-17)
	{
		size_t f_slash = dest.find_first_of(L"\\", 0);
		dest.replace(f_slash + 1, dest.length() - DlgW+19, L"...");
	}
	std::string et = to_multibyte(std::wstring(error_text), std::locale(".OCP"));
	lstrcpyn(DialogItems[IERFile].Data, to_multibyte(dest, std::locale(".OCP")).c_str(), MAX_PATH);
	lstrcpyn(DialogItems[IERError].Data, et.c_str(), MAX_PATH);
	//lstrcpyn(DialogItems[IERError].Data, to_multibyte(errortext, std::locale(".OCP")).c_str(), MAX_PATH);

	GetFileAttr(destination,DialogItems[IERDestination].Data,GetMsg(SMSGWCDestination));


	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		NULL, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, ErrorDlgProc,0);
}

long WINAPI ErrorDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
	static int PrevFocused;
	static int bLBoxPosChanged;

	long FocusedControl;

	// Запомним активный элемент диалога
	FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

	switch (Msg) {
	case DN_BTNCLICK:
		switch(Param1)
		{
		case IERBRetry:
		case IERBIgnore:
		case IERBAbort:
			{
				BOOL all = Info.SendDlgMessage(hDlg, DM_GETCHECK, IERCBIgnoreAll, 0); 
				int ret = ((all)?200:100) + Param1;
				Info.SendDlgMessage(hDlg, DM_CLOSE, ret, 0);
			}
			return TRUE;
		}
		break;
	case DN_CTLCOLORDIALOG:
		return Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT);

	case DN_CTLCOLORDLGITEM:
		switch(Param1)
		{
		case IFRODialogBox:
			return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTBOXTITLE)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOXTITLE));

		case IERError:
		case IERSeparator1:
		case IERSeparator2:
		case IERCBIgnoreAll:
		case IERDestination:
		case IERFile:
			return (Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBOX)<<16)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGHIGHLIGHTTEXT)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGTEXT));
		case IERBRetry:
		case IERBIgnore:
		case IERBAbort:
			if(Info.SendDlgMessage(hDlg,DM_GETFOCUS,0,0) == Param1)
				return (Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGSELECTEDBUTTON));
			else
				return (Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, (void *)COL_WARNDIALOGHIGHLIGHTBUTTON)<<8)|(Info.AdvControl(Info.ModuleNumber,ACTL_GETCOLOR,(void *)COL_WARNDIALOGBUTTON));
		}
		break;

	case DN_INITDIALOG:
		Info.SendDlgMessage(hDlg, DM_SETFOCUS, IERCBIgnoreAll, 0);
		return FALSE;

	case DN_KEY:
		switch(Param2)
		{
		case KEY_ENTER:
			if(Param1 == IERCBIgnoreAll)
			{
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, IERBRetry, 0);
				return TRUE;
			}						
			return FALSE;
		}
		break;

	case DN_KILLFOCUS:
		break;

	case DN_DRAWDLGITEM:
		break;

	case DN_CLOSE:

		return(Param1 < 0 || Param1 >= 100);
	}

	// Передадим управление стандартному обработчику
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}