#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "DeleteDialog.h"
#include "Status.h"


extern struct PluginStartupInfo Info;

const int DlgW = 76;
const int DlgH = 12;

HANDLE hDeleteThread = NULL;

int OpenDeleteDialog()
{	
	hDeleteThread = NULL;

	struct FarDialogItem DialogItems[IDLast];

	static const struct InitDialogItem InitItems [] = {
		DI_DOUBLEBOX,	3,				1,			DlgW-4,		DlgH-2,		0,	SDeleteCaption,
		DI_TEXT,		5,				2,			DlgW-7,		0,			0,	SDeleteSDelete,
		DI_EDIT,		5,				3,			70,			0,			DIF_HISTORY,SDeleteCaption,
		DI_TEXT,		0,				4,			DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
		
        DI_CHECKBOX,	5,				DlgH-7,		0,			0,			0,SDeleteCBReadOnly,
        DI_CHECKBOX,	5,				DlgH-6,		0,			0,			0,SDeleteCBAbortOnError,
        DI_CHECKBOX,	5,				DlgH-5,		0,			0,			0,SDeleteCBRecycle,
        
		DI_TEXT,		0,				DlgH-4,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
		DI_BUTTON,		DlgW/2 - 10,	DlgH-3,		0,			0,			0, SDeleteBOk,
		DI_BUTTON,		DlgW/2 + 2,		DlgH-3,		0,			0,			0, SDeleteBCancel
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));


	static char *BProcessDeleteDeleteHistoryName = "BProcessCopy";
	DialogItems[IDEdit].History = BProcessDeleteDeleteHistoryName;

	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, DeleteDlgProc,0);
}

long WINAPI DeleteDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
	static int PrevFocused;
	static int bLBoxPosChanged;

	PanelInfo PInfo;
	PanelInfo AnotherPInfo;
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &PInfo);
	Info.Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, &AnotherPInfo);
  
//  MOUSE_EVENT_RECORD m_record;
	long FocusedControl;

  // Запомним активный элемент диалога
  FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

  switch (Msg) {
	case DN_BTNCLICK:
		if(Param1 == IDOk)
		{
			//Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
			//OpenProgressDialog();
			
			MakeDeleteXML(hDlg, PInfo, AnotherPInfo);
		}
		break;
    case DN_CTLCOLORDLGITEM:
      break;

    case DN_INITDIALOG:
		{	
			Info.SendDlgMessage(hDlg, DM_SETTEXTPTR, IDEdit, (long)PInfo.CurDir);

			Info.SendDlgMessage(hDlg, DM_SETFOCUS, IDOk, 0);

			std::stringstream strstrm;
			
			strstrm << GetMsg(SDeleteSDelete) << " ";

			if(PInfo.SelectedItemsNumber > 1)
			{
				strstrm << PInfo.SelectedItemsNumber;
				strstrm << GetMsg(SDeleteSItems);
			}
			else if(PInfo.SelectedItemsNumber == 1)
			{
				strstrm << "\"" << PInfo.SelectedItems[0].FindData.cFileName << "\"";
				//strstrm << GetMsg(SDelteSTo);
			}
			else
				Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
						
			FarDialogItemData fdid;
			std::string str = strstrm.str();
			fdid.PtrData = const_cast<char*>(str.c_str());
			fdid.PtrLength = str.length() + 1;

			Info.SendDlgMessage(hDlg, DM_SETTEXT, IDString, (long)&fdid);
		}
      return FALSE;

    case DN_KILLFOCUS:
      break;
    
    case DN_DRAWDLGITEM:
    break;
  
    case DN_CLOSE:
      // Закрываем если нажат Cancel или ESC
      //return(Param1 < 0 || Param1 == ICancel);
		break;
  }

  // Передадим управление стандартному обработчику
  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/*
DWORD MakeDeleteXML(PanelInfo& PInfo, PanelInfo& AnotherPInfo)
{
	kru_string xmlBuffer;
	kru_string xmlHeader = TEXT("<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n<script>\n");
	//kru_string xmlHeader = TEXT("<script>\n");
	kru_string xmlFooter = TEXT("</script>");
	
	kru_string srcPath = PInfo.CurDir;

	if(srcPath[srcPath.length() - 1] != TEXT('\\'))
		srcPath += TEXT("\\");

	xmlBuffer += xmlHeader;


	for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
	{
		kru_string xmlElement;

		
		if(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			xmlElement = TEXT("<DeleteFile filename=\"");
		else
			xmlElement = TEXT("<DeleteFile filename=\"");			
	
		char ansibuff[MAX_PATH];
		OemToChar(PInfo.SelectedItems[i].FindData.cFileName, ansibuff);
		xmlElement += srcPath + ansibuff + TEXT("\"");
			xmlElement += TEXT(" flags=\"none\"/>\n");
		xmlBuffer += xmlElement;
	}

	xmlBuffer += xmlFooter;
	MessageBox(NULL, xmlBuffer.c_str(), "Test", 0);

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
		return GetLastError();

	SendCommandToPipe(hPipe, PCC_TAKE_XML);
	HRESULT hr = SendXMLToPipe(hPipe, xmlBuffer.c_str(), xmlBuffer.length());

	CloseHandle(hPipe);
	return hr;
}*/

DWORD MakeDeleteXML(HANDLE hDlg, PanelInfo& PInfo, PanelInfo& AnotherPInfo)
{
	std::wstring xmlBuffer;
	std::wstring xmlHeader = L"<script>\n";
	std::wstring xmlFooter = L"</script>";

	size_t textsize = Info.SendDlgMessage(hDlg, DM_GETTEXT, IDEdit, 0) + 1;
	char text[MAX_PATH] = {0};

	Info.SendDlgMessage(hDlg, DM_GETTEXTPTR, IDEdit, (long)text);
	text[textsize + 1] = 0;

	//std::wstring srcPath = get_full_wide_name(PInfo.CurDir);
	//std::wstring destPath = get_full_wide_name(AnotherPInfo.CurDir);

	std::wstring srcPath = to_unicode(std::string(text),  std::locale(".OCP"));
	add_back_slash(srcPath);
	//std::wstring destPath = to_unicode(std::string(text),  std::locale(".OCP"));
	
	xmlBuffer += xmlHeader;

	DWORD dwFlags = 0;

	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, IDCBReadOnly, 0) == BSTATE_CHECKED)
		dwFlags |= flgEvenReadOnly;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, IDCBAbortOnError, 0) == BSTATE_CHECKED)
		dwFlags |= flgAbortOnError;
	if(Info.SendDlgMessage(hDlg, DM_GETCHECK, IDCBRecycle, 0) == BSTATE_CHECKED)
		dwFlags |= flgRecycle;

	std::wstringstream flagsstrm;
	flagsstrm << dwFlags;

	if(dwFlags & flgRecycle)
	{
		std::wstring xmlElement;
		xmlElement = L"<DeleteFile filename=\"";
		std::wstring files;
		for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
		{
			wchar_t filename[MAX_PATH] = {0};
		
			std::string path = PInfo.CurDir;
			path += "\\";
			GetWideName(path.c_str(), &PInfo.SelectedItems[i].FindData, filename);

			files += srcPath + filename;
			files += L"|";
		}
		files += L"|";
		xmlElement += files;
		xmlElement += L"\" flags=\"" + flagsstrm.str() + L"\"/>\n";
		xmlBuffer += xmlElement;
	}
	else
	{
		for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
		{
			std::wstring xmlElement;

			if(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{					
				xmlElement = L"<DeleteFolder filename=\"";
			}
			else			
				xmlElement = L"<DeleteFile filename=\"";			

			wchar_t filename[MAX_PATH] = {0};

			std::string path = PInfo.CurDir;
			path += "\\";
			GetWideName(path.c_str(), &PInfo.SelectedItems[i].FindData, filename);

			xmlElement += srcPath + filename + L"\"";
			//xmlElement += srcPath + widebuff + L"\"";
			xmlElement += L" flags=\"" + flagsstrm.str() + L"\"/>\n";
			xmlBuffer += xmlElement;
		}
	}

	

	xmlBuffer += xmlFooter;
//	MessageBoxW(NULL, xmlBuffer.c_str(), L"Test", 0);



	HANDLE hPipe = OpenPipe();
	if(!hPipe)
		return S_FALSE;

	try
	{
		SendCommandToPipe(hPipe, PCC_TAKE_XML);
		SendXMLToPipe(hPipe, (char*)xmlBuffer.c_str(), (xmlBuffer.length())*sizeof(wchar_t));
		//SendXMLToPipe(hPipe, xmlBuffer.c_str(), xmlBuffer.length());

		GetHandleFromPipe(hPipe, hDeleteThread);

		DWORD server_confirm = 0;
		GetCommandFromPipe(hPipe, server_confirm);

		if(server_confirm != PSC_OK)
			throw ClosePipeException(TEXT(""));

		SendCommandToPipe(hPipe, PCC_END_SESSION);
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