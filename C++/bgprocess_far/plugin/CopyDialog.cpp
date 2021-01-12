#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "CopyDialog.h"
#include "Status.h"


extern FARSTANDARDFUNCTIONS FSF;

extern struct PluginStartupInfo Info;
PanelInfo PInfo;
PanelInfo AnotherPInfo;
	

const int DlgW = 76;
const int DlgH = 19;

bool Move;

//Handle from other process!
HANDLE hCopyThread = NULL;

int OpenCopyDialog(bool move)
{	
	hCopyThread = NULL;

	Move = move;

	struct FarDialogItem DialogItems[ICLast];

	static const struct InitDialogItem InitItems [] = {
	DI_DOUBLEBOX,	3,				1,			DlgW-4,		DlgH-2,		0,	SNull,
	DI_TEXT,		5,				2,			DlgW-10,	0,			0,	SNull,
	DI_EDIT,		5,				3,			70,			0,			DIF_HISTORY,SCopyCaption,
	DI_TEXT,		0,				4,			DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,

	DI_RADIOBUTTON,	5,				5,			0,			0,			DIF_GROUP,SCopyRBOverwrite,
    DI_RADIOBUTTON,	5,				6,			0,			0,			0,SCopyRBAppend,
    DI_RADIOBUTTON,	5,				7,			0,			0,			0,SCopyRBSkip,
    DI_RADIOBUTTON,	5,				8,			0,			0,			0,SCopyRBRefresh,
    DI_RADIOBUTTON,	5,				9,			0,			0,			0,SCopyRBAsk,
    DI_CHECKBOX,	5,				11,			0,			0,			0,SCopyCBMove,
    DI_CHECKBOX,	5,				12,			0,			0,			0,SCopyCBCopyAccess,
    DI_CHECKBOX,	5,				13,			0,			0,			0,SCopyCBReadOnly,
    DI_CHECKBOX,	5,				14,			0,			0,			0,SCopyCBAbortOnError,

	DI_CHECKBOX,	44,				5,			0,			0,			0,SCopyCBMatch,
	DI_CHECKBOX,	44,				6,			0,			0,			0,SCopyCBDifference,
	DI_CHECKBOX,	44,				8,			0,			0,			0,SCopyCBModificationTime,
	DI_CHECKBOX,	44,				9,			0,			0,			0,SCopyCBCreationTime,
	DI_CHECKBOX,	44,				10,			0,			0,			0,SCopyCBSize,
	DI_CHECKBOX,	44,				11,			0,			0,			0,SCopyCBCase,
	DI_CHECKBOX,	44,				12,			0,			0,			0,SCopyCBContent,
      
	DI_TEXT,		0,				DlgH-4,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
	DI_BUTTON,		DlgW/2 - 10,	DlgH-3,		0,			0,			0, SCopyBOk,
	DI_BUTTON,		DlgW/2 + 2,		DlgH-3,		0,			0,			0, SCopyBCancel
	};

	

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));

	static char *BProcessCopyCopyHistoryName="BProcessCopy";
	DialogItems[ICEdit].History = BProcessCopyCopyHistoryName;

	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &PInfo);
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, &AnotherPInfo);

	if(PInfo.Plugin || AnotherPInfo.Plugin)
		return -1;

	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, CopyDlgProc,0);
}

void MoveCopy(HANDLE hDlg)
{
	std::stringstream strstrm;

	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBMove, 0) == BSTATE_CHECKED)
	{				
		SetItemTextA(hDlg, DM_SETTEXTPTR, ICDialogBox, GetMsg(SMoveCaption));
		strstrm << GetMsg(SCopySMove);

	}
	else
	{
		SetItemTextA(hDlg, DM_SETTEXTPTR, ICDialogBox, GetMsg(SCopyCaption));
		strstrm << GetMsg(SCopySCopy);
	}

	if(PInfo.SelectedItemsNumber > 1)
	{
		strstrm << PInfo.SelectedItemsNumber;
		strstrm << GetMsg(SCopySItemsTo);
	}
	else if(PInfo.SelectedItemsNumber == 1)
	{
		strstrm << "\"" << PInfo.SelectedItems[0].FindData.cFileName << "\"";
		strstrm << GetMsg(SCopySTo);
	}
	else
		Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
						
	std::string str = strstrm.str();
	Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, ICString, (long)str.c_str());
}

long WINAPI CopyDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
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
		case ICOk:
			MakeCopyXML(hDlg, PInfo, AnotherPInfo);
			Info.SendDlgMessage(hDlg, DM_CLOSE, ICOk, 0);
			return TRUE;
		case ICCBMove:
			MoveCopy(hDlg);			
			break;
		case ICCancel:
			Info.SendDlgMessage(hDlg, DM_CLOSE, ICCancel, 0);
			break;
		}
		break;
    case DN_CTLCOLORDLGITEM:
      break;

    case DN_INITDIALOG:

		Info.SendDlgMessage(hDlg, DM_SETFOCUS, ICEdit, 0);
		Info.SendDlgMessage(hDlg, DM_SETCHECK, ICRBAsk, BSTATE_CHECKED);

		Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, ICEdit, (long)AnotherPInfo.CurDir);

		if(Move)
			Info.SendDlgMessage(hDlg, DM_SETCHECK, ICCBMove, BSTATE_CHECKED);

		MoveCopy(hDlg);

		return TRUE;

    case DN_KILLFOCUS:
      break;

	  case DN_KEY:
		switch(Param2)
		{
		case KEY_ENTER:
			if(Param1 != ICCancel)
			{
				Info.SendDlgMessage(hDlg, DM_SETFOCUS, ICOk, 0);
				Info.SendDlgMessage(hDlg, DN_BTNCLICK, ICOk, 0);
				return TRUE;
			}
			return FALSE;
		}
		break;
    
    case DN_DRAWDLGITEM:
    break;
  
    case DN_CLOSE:
		if(Param1 < 0 || Param1 == ICOk || Param1 == ICCancel|| Param1 == 100)
		{
			return TRUE;
		}
		else return FALSE;
  }

  // Передадим управление стандартному обработчику
  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}



DWORD MakeCopyXML(HANDLE hDlg, PanelInfo& PInfo, PanelInfo& AnotherPInfo)
{
	std::wstring xmlBuffer;
	std::wstring xmlHeader = L"<script>\n";
	std::wstring xmlFooter = L"</script>";


	size_t textsize = Info.SendDlgMessage(hDlg, DM_GETTEXT, ICEdit, 0) + 1;
	char text[MAX_PATH] = {0};

	Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, ICEdit, (long)text);
	text[textsize + 1] = 0;

	std::wstring srcPath = to_unicode(std::string(PInfo.CurDir),  std::locale(".OCP"));
	std::wstring destPath = to_unicode(std::string(text),  std::locale(".OCP"));
	add_back_slash(srcPath);
	add_back_slash(destPath);

	
	//std::wstring srcPath = get_full_wide_name(PInfo.CurDir);
	//std::wstring destPath = get_full_wide_name(AnotherPInfo.CurDir);
	/*size_t textsize = Info.SendDlgMessage(hDlg, DM_GETTEXT, ICEdit, 0) + 1;
	char text[MAX_PATH] = {0};
    
	Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, ICEdit, (long)text);
	text[textsize + 1] = 0;

	char exp[MAX_PATH] = {0}; 
	DWORD res;
    res = FSF.ExpandEnvironmentStr(text, exp, sizeof(exp));

	if((exp[0] != '\\' && exp[1] != '\\') || exp[1] != ':')
	{
	}
*/
	//std::wstring destPath = get_full_wide_name(exp);
	

	/*char SrcA[MAX_PATH]; wchar_t SrcW[MAX_PATH];
	char DestA[MAX_PATH]; wchar_t DestW[MAX_PATH];
	if(!PInfo.Plugin)
	{
		strcpy(SrcA,PInfo.CurDir);
		strcpy(DestA,PInfo.CurDir);
		UNCPath(SrcA);
		UNCPath(DestA);
		FSF.AddEndSlash(SrcA);
		FSF.AddEndSlash(DestA);
		MultiByteToWideChar(CP_OEMCP,0,SrcA,-1,SrcW,MAX_PATH);
		MultiByteToWideChar(CP_OEMCP,0,DestA,-1,DestW,MAX_PATH);
	}
	else
	{
		SrcA[0]=0;
		SrcW[0]=0;
		DestA[0]=0;
		DestW[0]=0;
	}

	std::wstring destPath = DestW;
	std::wstring srcPath = SrcW;*/



	//if(destPath.)
	
	xmlBuffer += xmlHeader;

	if(PInfo.SelectedItemsNumber == 1)
	{
		wchar_t filename[MAX_PATH];
				
		std::string path = PInfo.CurDir;
		path += "\\";
		GetWideName(path.c_str(), &PInfo.SelectedItems[0].FindData, filename);

		xmlBuffer += L"<SetJobName name=\"" + srcPath  + filename +  L"\"/>\n";
	}	
	else 
		xmlBuffer += L"<SetJobName name=\"" + srcPath  + L"*\"/>\n";

	xmlBuffer += L"<SetCurrentFolder source=\"" + srcPath  + L"\"/>\n";
		

	DWORD dwFlags = 0;

	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBAsk, 0) != BSTATE_CHECKED)
		dwFlags |= flgAsk;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBOverwrite, 0) == BSTATE_CHECKED)
		dwFlags |= flgOverwrite;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBAppend, 0) == BSTATE_CHECKED)
		dwFlags |= flgAppend;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBSkip, 0) == BSTATE_CHECKED)
		dwFlags |= flgSkip;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBRefresh, 0) == BSTATE_CHECKED)
		dwFlags |= flgRefresh;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBMove, 0) == BSTATE_CHECKED)
		dwFlags |= flgMove;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyAccess, 0) == BSTATE_CHECKED)
		dwFlags |= flgCopyAccessRights;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBReadOnly, 0) == BSTATE_CHECKED)
		dwFlags |= flgEvenReadOnly;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBAbortOnError, 0) == BSTATE_CHECKED)
		dwFlags |= flgAbortOnError;

	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBContent, 0) == BSTATE_CHECKED)
		dwFlags |= flgContents;

	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyMatch, 0) == BSTATE_CHECKED)
		dwFlags |= flgMatch;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyDiff, 0) == BSTATE_CHECKED)
		dwFlags |= flgDiff;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBModificationTime, 0) == BSTATE_CHECKED)
		dwFlags |= flgModificationTime;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCreationTime, 0) == BSTATE_CHECKED)
		dwFlags |= flgCreationTime;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCase, 0) == BSTATE_CHECKED)
		dwFlags |= flgCase;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBSize, 0) == BSTATE_CHECKED)
		dwFlags |= flgSize;
	
	std::wstringstream flagsstrm;
	flagsstrm << dwFlags;

	if(	(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyMatch, 0) == BSTATE_CHECKED) ||
		(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyDiff, 0) == BSTATE_CHECKED))
	{

		std::wstring sources;
		std::wstring outputs;

		for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
		{
			if(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wchar_t filename[MAX_PATH];
				
				std::string path = PInfo.CurDir;
				path += "\\";
				GetWideName(path.c_str(), &PInfo.SelectedItems[i].FindData, filename);

				sources += srcPath + filename + L"|";
				outputs += destPath + filename + L"|";
			}
		}

		std::wstring xmlElement;
		xmlElement = L"<CopyComparedFolders sources=\"";
		xmlElement += sources;
		xmlElement += L"\" matchoutput=\"";
		xmlElement += destPath + L"match";
		xmlElement += L"\" diffoutputs=\"";
		xmlElement += outputs;
		xmlElement += L"\" mask=\"*.*";
		xmlElement += L"\" flags=\"" + flagsstrm.str() + L"\"/>\n";
		xmlBuffer += xmlElement;
	}
	else
	{

		for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
		{
			std::wstring xmlElement;

			if(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				xmlElement = L"<CopyFolder source=\"";
			else
				xmlElement = L"<CopyFile source=\"";			


			wchar_t filename[MAX_PATH];
			//wchar_t WideFullName[MAX_PATH];

			//std::wstring path = WideName;
			//get_path(path);

			std::string path = PInfo.CurDir;
			path += "\\";
			GetWideName(path.c_str(), &PInfo.SelectedItems[i].FindData, filename);

			//std::wstring wstr = to_unicode(std::string(PInfo.SelectedItems[i].FindData.cFileName),  std::locale(".OCP"));

			//kru_string filename = to_multibyte(wstr,  std::locale(".ACP"));

			xmlElement += srcPath + filename + L"\" destination=\"";
			xmlElement += destPath + filename;
			xmlElement += L"\" flags=\"" + flagsstrm.str() + L"\"/>\n";
			xmlBuffer += xmlElement;

		}
	}

	xmlBuffer += xmlFooter;
//	MessageBoxW(NULL, xmlBuffer.c_str(), L"Test", 0);


	//std::wstring destPath = AnotherPInfo.CurDir;


/*	kru_string xmlBuffer;
	kru_string xmlHeader = TEXT("<script>\n");
	kru_string xmlFooter = TEXT("</script>");
	kru_string destPath = AnotherPInfo.CurDir;

	if(destPath[destPath.length() - 1] != TEXT('\\'))
		destPath += TEXT("\\");

	kru_string srcPath = PInfo.CurDir;

	if(srcPath[srcPath.length() - 1] != TEXT('\\'))
		srcPath += TEXT("\\");

	xmlBuffer += xmlHeader;

	DWORD dwFlags = 0;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBOverwrite, 0) == BSTATE_CHECKED)
		dwFlags |= flgOverwrite;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBAppend, 0) == BSTATE_CHECKED)
		dwFlags |= flgAppend;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBSkip, 0) == BSTATE_CHECKED)
		dwFlags |= flgSkip;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBRefresh, 0) == BSTATE_CHECKED)
		dwFlags |= flgRefresh;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICRBAsk, 0) == BSTATE_CHECKED)
		dwFlags |= flgAsk;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBMove, 0) == BSTATE_CHECKED)
		dwFlags |= flgMove;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyAccess, 0) == BSTATE_CHECKED)
		dwFlags |= flgCopyAccessRights;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBReadOnly, 0) == BSTATE_CHECKED)
		dwFlags |= flgEvenReadOnly;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBAbortOnError, 0) == BSTATE_CHECKED)
		dwFlags |= flgAbortOnError;

	std::stringstream strstrm;
	strstrm << dwFlags;

	if(	(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyMatch, 0) == BSTATE_CHECKED) ||
		(Info.SendDlgMessage(hDlg, DM_GETCHECK, ICCBCopyDiff, 0) == BSTATE_CHECKED))
	{
		kru_string sources;
		kru_string outputs;

		for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
		{
			if(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				sources += srcPath + PInfo.SelectedItems[i].FindData.cFileName + TEXT("|");
				outputs += destPath + PInfo.SelectedItems[i].FindData.cFileName + TEXT("|");
			}
		}

		kru_string xmlElement;
		xmlElement = TEXT("<CopyComparedFolders sources=\"");
		xmlElement += sources;
		xmlElement += TEXT("\" matchoutput=\"");
		xmlElement += destPath + TEXT("match");
		xmlElement += TEXT("\" diffoutputs=\"");
		xmlElement += outputs;
		xmlElement += TEXT("\" mask=\"*.*");
		xmlElement += TEXT("\" flags=\"") + strstrm.str() + TEXT("\"/>\n");
		xmlBuffer += xmlElement;
	}
	else
	{

		for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
		{
			kru_string xmlElement;

			if(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				xmlElement = TEXT("<CopyFolder source=\"");
			else
				xmlElement = TEXT("<CopyFile source=\"");			


			wchar_t WideName[MAX_PATH];
			//wchar_t WideFullName[MAX_PATH];

			GetWideName(srcPath.c_str(), &PInfo.SelectedItems[i].FindData, WideName);

			std::wstring wstr = to_unicode(std::string(PInfo.SelectedItems[i].FindData.cFileName),  std::locale(".OCP"));

			kru_string filename = to_multibyte(wstr,  std::locale(".ACP"));

			xmlElement += srcPath + filename + TEXT("\" destination=\"");
			xmlElement += destPath + filename;
			xmlElement += TEXT("\" flags=\"") + strstrm.str() + TEXT("\"/>\n");
			xmlBuffer += xmlElement;

		}
	}

	xmlBuffer += xmlFooter;
	MessageBox(NULL, xmlBuffer.c_str(), "Test", 0);

	std::wstring wxmlBuffer = to_unicode(xmlBuffer,  std::locale(".ACP"));

*/
	HANDLE hPipe = OpenPipe();
	if(!hPipe)
		return S_FALSE;

	try
	{
		SendCommandToPipe(hPipe, PCC_TAKE_XML);
		SendXMLToPipe(hPipe, (char*)xmlBuffer.c_str(), (xmlBuffer.length())*sizeof(wchar_t));
		//SendXMLToPipe(hPipe, xmlBuffer.c_str(), xmlBuffer.length());

		GetHandleFromPipe(hPipe, hCopyThread);

		DWORD server_confirm = 0;
		GetCommandFromPipe(hPipe, server_confirm);

		if(server_confirm != PSC_OK)
			throw ClosePipeException(TEXT(""));

		SendCommandToPipe(hPipe, PCC_END_SESSION);

		//server_confirm = 0;
		//GetCommandFromPipe(hPipe, server_confirm);

		//if(server_confirm != PSC_OK)
		//	throw ClosePipeException(TEXT(""));

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