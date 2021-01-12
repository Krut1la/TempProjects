#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "ExtractDialog.h"
#include "Status.h"


extern struct PluginStartupInfo Info;

const int DlgW = 76;
const int DlgH = 19;

//Handle from other process!
HANDLE hExtractThread = NULL;

int OpenExtractDialog()
{	
	struct FarDialogItem DialogItems[IELast];

	static const struct InitDialogItem InitItems [] = {
		DI_DOUBLEBOX,	3,				1,			DlgW-4,		DlgH-2,		0,	SExtractCaption,
		DI_TEXT,		5,				2,			DlgW-1,		0,			0,	SExtractSExtract,
		DI_EDIT,		5,				3,			70,			0,			DIF_HISTORY,SExtractCaption,
		DI_TEXT,		0,				4,			DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,

		DI_RADIOBUTTON,	5,				5,			0,			0,			DIF_GROUP,SExtractRBOverwrite,
        DI_RADIOBUTTON,	5,				6,			0,			0,			0,SExtractRBAppend,
        DI_RADIOBUTTON,	5,				7,			0,			0,			0,SExtractRBSkip,
        DI_RADIOBUTTON,	5,				8,			0,			0,			0,SExtractRBRefresh,
        DI_RADIOBUTTON,	5,				9,			0,			0,			0,SExtractRBAsk,
        DI_CHECKBOX,	5,				11,			0,			0,			0,SExtractCBMove,
        DI_CHECKBOX,	5,				12,			0,			0,			0,SExtractCBCopyAccess,
        DI_CHECKBOX,	5,				13,			0,			0,			0,SExtractCBReadOnly,
        DI_CHECKBOX,	5,				14,			0,			0,			0,SExtractCBAbortOnError,
        
		DI_TEXT,		0,				DlgH-4,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
		DI_BUTTON,		DlgW/2 - 10,	DlgH-3,		0,			0,			0, SExtractBOk,
		DI_BUTTON,		DlgW/2 + 2,		DlgH-3,		0,			0,			0, SExtractBCancel
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));


	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, ExtractDlgProc,0);
}

long WINAPI ExtractDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
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
		if(Param1 == IEOk)
		{
			//Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
			//OpenProgressDialog();
			
			MakeExtractXML(PInfo, AnotherPInfo, L"", L"");
		}
		break;
    case DN_CTLCOLORDLGITEM:
      break;

    case DN_INITDIALOG:
		{			
			std::stringstream strstrm;
			
			strstrm << GetMsg(SExtractSExtract);

			if(PInfo.SelectedItemsNumber > 1)
			{
				strstrm << PInfo.SelectedItemsNumber;
				strstrm << GetMsg(SExtractSItemsTo);
			}
			else if(PInfo.SelectedItemsNumber == 1)
			{
				strstrm << "\"" << PInfo.SelectedItems[0].FindData.cFileName << "\"";
				strstrm << GetMsg(SExtractSTo);
			}
			else
				Info.SendDlgMessage(hDlg, DM_CLOSE, 0, 0);
						
			FarDialogItemData fdid;
			std::string str = strstrm.str();
			fdid.PtrData = const_cast<char*>(str.c_str());
			fdid.PtrLength = str.length() + 1;

			Info.SendDlgMessage(hDlg, DM_SETTEXT, IEExtractString, (long)&fdid);
			fdid.PtrData = AnotherPInfo.CurDir;
			fdid.PtrLength = strlen(AnotherPInfo.CurDir) + 1;
			Info.SendDlgMessage(hDlg, DM_SETTEXT, IEExtractEdit, (long)&fdid);
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


typedef std::map<std::wstring, std::wstring> ARC_EXTENTION_MAP;


DWORD MakeExtractXML(PanelInfo& PInfo, PanelInfo& AnotherPInfo, const std::wstring& files, const std::wstring& archive_name)
{
	/*std::wstring xmlBuffer;
	std::wstring xmlHeader = L"<script>\n";
	std::wstring xmlFooter = L"</script>";
	std::wstring destPath = AnotherPInfo.CurDir;

	if(destPath[destPath.length() - 1] != L'\\')
		destPath += L"\\";

	std::wstring srcPath = PInfo.CurDir;

	if(srcPath[srcPath.length() - 1] != L'\\')
		srcPath += L"\\";

	xmlBuffer += xmlHeader;

	for(int i = 0; i < PInfo.SelectedItemsNumber; ++i)
	{
		if(!(PInfo.SelectedItems[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::wstring type = PInfo.SelectedItems[i].FindData.cFileName;
			type.replace(0, type.find_last_of(".", type.length()) + 1, L"");

			std::wstring xmlElement;

			xmlElement = L"<ExtractWCXArchive source=\"";
			xmlElement += srcPath + PInfo.SelectedItems[i].FindData.cFileName 
				+ L"\" type=\"" + type + L"\"";

			xmlElement += L" files=\"none\" command=\"extract\" destination=\"";

			xmlElement += destPath;
			xmlElement += L"\" flags=\"none\"/>\n";
			xmlBuffer += xmlElement;
		}
	}


	xmlBuffer += xmlFooter;
	//MessageBox(NULL, xmlBuffer.c_str(), "Test", 0);


	HANDLE hPipe = OpenPipe();
	if(!hPipe)
		return S_FALSE;

	try
	{
		SendCommandToPipe(hPipe, PCC_TAKE_XML);
		SendXMLToPipe(hPipe, (char*)xmlBuffer.c_str(), (xmlBuffer.length())*sizeof(wchar_t));

		GetHandleFromPipe(hPipe, hExtractThread);

		DWORD server_confirm = 0;
		GetCommandFromPipe(hPipe, server_confirm);

		if(server_confirm != PSC_OK)
			throw ClosePipeException(L"");

		SendCommandToPipe(hPipe, PCC_END_SESSION);

//		server_confirm = 0;
//		GetCommandFromPipe(hPipe, server_confirm);
//
//		if(server_confirm != PSC_OK)
//			throw ClosePipeException(L""));

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

	CloseHandle(hPipe);*/
	return S_OK;
}