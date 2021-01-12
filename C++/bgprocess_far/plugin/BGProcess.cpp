#include "Common.h"

#ifndef UNICODE
#define UNICODE
#include "..\\PipeProtocol\\PipeProtocol.h"
#undef UNICODE
#else
#include "..\\PipeProtocol\\PipeProtocol.h"
#endif

#include "CopyDialog.h"
#include "DeleteDialog.h"
#include "ExtractDialog.h"
#include "ProgressDialog.h"
#include "InfoDialog.h"
#include "ConfigDialog.h"
#include "ArchivePanel.h"
#include "OpenArchiveDialog.h"
#include "Status.h"

FARSTANDARDFUNCTIONS FSF;

//HANDLE hHeap;


struct PluginStartupInfo Info;
HWND FarHWND;

//static struct FarDialogItem DialogItems[ILast];
ColorGroupItem *CurrentColorGroup;

HANDLE hInfoThread = NULL;

extern HANDLE hCopyThread;
extern HANDLE hExtractThread;
extern HANDLE hDeleteThread;

bool operator<(const FILEDESC& lhs, const FILEDESC& rhs)
{
	/*if(	(lhs.dwFileAttributes != rhs.dwFileAttributes) ||
	(lhs.dwReserved0 != rhs.dwReserved0) ||
	(lhs.dwReserved1 != rhs.dwReserved1) ||
	(lhs.ftCreationTime != rhs.ftCreationTime) ||
	(lhs.ftLastAccessTime != rhs.ftLastAccessTime) ||
	(lhs.ftLastWriteTime != rhs.ftLastWriteTime) ||
	(lhs. != rhs.dwFileAttributes) ||
	)
	*/
	return (wcscmp(lhs.FindData.cFileName, rhs.FindData.cFileName) == -1);
}


static void SetCaption(HANDLE hDlg, char *ThemeName)
{

}

static void ListBoxPosUpdate(HANDLE hDlg, int Method)
{

}

static BOOL ProcessControlKey(COORD *coord, long Key, int MaxX, int MaxY)
{
  static const signed char dx[8] = {0, 0, 1, -1, -127, 127, 0, 0};
  static const signed char dy[8] = {1, -1, 0, 0, 0, 0, -127, 127};
  static const WORD ExampleKeys[8] = {
    KEY_DOWN, KEY_UP, KEY_RIGHT, KEY_LEFT,
    KEY_HOME, KEY_END, KEY_PGUP, KEY_PGDN,
  };

  for (int i = 0; i < 8; i++) {
    if (Key == ExampleKeys[i]) {
      coord->X += dx[i];
      if (coord->X < 0) coord->X = 0;
      if (coord->X > MaxX - 1) coord->X = MaxX - 1;

      coord->Y += dy[i];
      if (coord->Y < 0) coord->Y = 0;
      if (coord->Y > MaxY - 1) coord->Y = MaxY - 1;
      return TRUE;
    }
  }
  return FALSE;
}

int WINAPI _export Configure(int ItemNumber)
{
	switch(ItemNumber)
	{
	case 0:
		return OpenConfigDialog();
	}
	return FALSE;
}



HANDLE WINAPI _export OpenPlugin(int OpenFrom, int Item)
{
	/*static struct KeyBarTitles KeyBar;
	memset(&KeyBar,0,sizeof(KeyBar));
	char buf1[12] = "Õóé";
	char buf2[12] = "Ïèçäà";
	KeyBar.Titles[0]=buf1;
	KeyBar.Titles[1]=buf2;
	Info->KeyBar=&KeyBar;*/
	//SetFileApisToOEM();
	FarHWND = (HWND)Info.AdvControl(Info.ModuleNumber, ACTL_GETFARHWND, NULL);

	struct FarMenuItemEx MenuItems[8];
	memset(MenuItems, 0, sizeof(MenuItems));

	for(int i = 0; i < 8; ++i) MenuItems[i].Flags = MIF_USETEXTPTR;
	
	MenuItems[0].Text.TextPtr = GetMsg(SCopyMenu);
	MenuItems[0].Flags |= MIF_SELECTED;

	MenuItems[1].Text.TextPtr = GetMsg(SMoveMenu);
	MenuItems[2].Text.TextPtr = GetMsg(SDeleteMenu);
	MenuItems[3].Text.TextPtr = GetMsg(SExtractMenu);
	MenuItems[4].Text.TextPtr = NULL;
	MenuItems[4].Flags |= MIF_SEPARATOR;
	
	MenuItems[5].Text.TextPtr = GetMsg(SConfigureMenu);
	MenuItems[6].Text.TextPtr = GetMsg(SInfoMenu);
	MenuItems[7].Text.TextPtr = GetMsg(SStopService);

	int MenuCode=Info.Menu(Info.ModuleNumber, -1, -1, 0,
		FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE|FMENU_USEEXT,
		GetMsg(STitle),NULL,
		"Contents",NULL,NULL,
		(struct FarMenuItem*)MenuItems,
		sizeof(MenuItems)/sizeof(MenuItems[0]));

	
	switch(MenuCode)
	{
	case 0:
		switch(OpenCopyDialog())
		{
		case ICCancel:			
			break;
		default:
			if(hCopyThread != NULL)
                OpenProgressDialog(hCopyThread);
			break;
		}
		break;
	case 1:
		switch(OpenCopyDialog(true))
		{
		case ICCancel:			
			break;
		default:
			if(hCopyThread != NULL)
                OpenProgressDialog(hCopyThread);
			break;
		}
		break;
	case 2:
		switch(OpenDeleteDialog())
		{
		case IDCancel:			
			break;
		default:
			if(hDeleteThread != NULL)
                OpenProgressDialog(hDeleteThread);
			break;
		}
		/*switch(OpenDeleteDialog())
		{
		case ICOk:
			OpenProgressDialog(0);
			break;
		default:
			break;
		}*/
		break;
	case 3:
		switch(OpenExtractDialog())
		{
		case IEOk:
			if(hExtractThread != NULL)
                OpenProgressDialog(hExtractThread);
			break;
		default:
			break;
		}
		break;
	case 4:
		break;
	case 5:
		break;
	case 6:
		for(;;)
		{
			OpenInfoDialog();
			if(hInfoThread != NULL)
				OpenProgressDialog(hInfoThread);
			else
				break;
		}
		break;
	case 7:
		{
			HANDLE hPipe = OpenPipe();
			if(!hPipe)
				return(INVALID_HANDLE_VALUE);

			SendCommandToPipe(hPipe, PCC_STOP_SERVICE);
			DWORD server_confirm = 0;
			GetCommandFromPipe(hPipe, server_confirm);
			if(server_confirm != PSC_OK)
				return(INVALID_HANDLE_VALUE);
		}
		break;
	default:
		return(INVALID_HANDLE_VALUE);
	}
 
    return INVALID_HANDLE_VALUE;
}

void WINAPI _export GetOpenPluginInfo(HANDLE archive_panel, struct OpenPluginInfo *Info)
{

	
	Info->StructSize = sizeof (OpenPluginInfo);
	Info->Flags = OPIF_USEHIGHLIGHTING | OPIF_USESORTGROUPS | OPIF_ADDDOTS;
	Info->CurDir = ((ArchivePanel*)archive_panel)->CurrentFolder.c_str();

	
}



void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  static char *PluginMenuStrings[1];
  PluginMenuStrings[0] = (char*)GetMsg(STitle);
  static char *PluginConfigStrings[1];
  PluginConfigStrings[0] = (char*)GetMsg(STitle);
  Info->PluginMenuStrings = PluginMenuStrings;
  Info->PluginConfigStrings = PluginConfigStrings;
  Info->StructSize = sizeof(struct PluginInfo);
  Info->PluginMenuStringsNumber = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber = sizeof(PluginConfigStrings) / sizeof(PluginConfigStrings[0]);

}

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *pInfo)
{
  Info = *pInfo;
  //hHeap = GetProcessHeap();

  ::FSF=*Info.FSF;
  ::Info.FSF=&::FSF;
}

int WINAPI _export GetMinFarVersion(void)
{
  //return(MAKEFARVERSION(1,70,1634));
	return(MAKEFARVERSION(1,70,1557));
}

HANDLE WINAPI _export OpenFilePlugin(char *name,  const unsigned char *data,  int datasize)
{	
	ArchivePanel* archive_panel = new ArchivePanel();
	archive_panel->ArchiveName = name;
	OpenOpenArchiveDialog(name, archive_panel);


	std::sort(archive_panel->files.begin(), archive_panel->files.end());

	archive_panel->CurrentFileStack.push(archive_panel->files.begin());
	//archive_panel->CurrentFileStack.push(archive_panel->files.begin());
	
	if(archive_panel->files.empty())
	{
		delete archive_panel;
		return INVALID_HANDLE_VALUE;
	}

	return (HANDLE)archive_panel;
}

void WINAPI _export ClosePlugin(HANDLE archive_panel)
{
    delete (ArchivePanel*)archive_panel;
}

int WINAPI _export GetFindData (
		HANDLE archive_panel,
		PluginPanelItem **pPanelItem,
		int *pItemsNumber,
		int OpMode
		)
{

	FILELIST& filelist = ((ArchivePanel*)archive_panel)->files;
	//std::set<FILEDESC> current_filemap;
	((ArchivePanel*)archive_panel)->current_filemap.clear();

	std::string CurDirA = ((ArchivePanel*)archive_panel)->CurrentFolder;
	std::wstring CurDirW = to_unicode(CurDirA);

	bool started = false;
	bool ended = false;
	bool first_found = false;

	for(FILELIST::iterator i_file = ((ArchivePanel*)archive_panel)->CurrentFileStack.top(); i_file != filelist.end(); ++i_file)
	{
		std::wstring FileNameW = (*i_file).FindData.cFileName;
		//std::string FileNameA = to_multibyte(FileNameW);

		if(ended) break;
		
		//if(FileNameW.length() >= CurDirW.length())
		//{
		if( (FileNameW.length() > CurDirW.length() && FileNameW[CurDirW.length()] == L'\\') ||
			(FileNameW.length() == CurDirW.length()) ||
			CurDirW == L"")
		{
			if(FileNameW.substr(0, CurDirW.length()) == CurDirW)
			{
				if(!first_found)
				{
					((ArchivePanel*)archive_panel)->CurrentFileStack.push(i_file);
					first_found = true;
				}
				if(FileNameW.find_first_of(L'\\', CurDirW.length() + 1) == FileNameW.npos)
				{			

					started = true;
					//if((std::search(FileNameW.begin(), FileNameW.end(), CurDirW.begin(), CurDirW.end()) == FileNameW.begin()
					//	&& FileNameW.find_first_of(L'\\', CurDirW.length() + 1) == FileNameW.npos))
					//|| CurDirW == L"")

					size_t start = (CurDirW == L"")?0:CurDirW.length() + 1;
					//size_t end = FileNameW.find_first_of(L"\\", start);
					std::wstring filename = FileNameW.erase(0, start);
					//std::wstring filename = FileNameW;

					if(filename == L"")
						continue;
					//WIN32_FIND_DATAW wfdW = {0};
					FILEDESC fd = {0};

					//std::copy(filename.begin(), filename.end(), wfdW.cFileName);			
					wcsncpy(fd.FindData.cFileName, filename.c_str(), MAX_PATH);
					fd.FindData.dwFileAttributes = (*i_file).FindData.dwFileAttributes;
					fd.FindData.nFileSizeLow = (*i_file).FindData.nFileSizeLow;
					fd.FindData.ftLastWriteTime = (*i_file).FindData.ftLastWriteTime;
					//current_filemap.insert(FILEDESC(wfdW, L""));
					
					fd.path = L"";
					fd.subpath = L"";
					((ArchivePanel*)archive_panel)->current_filemap.push_back(fd);
					continue;
				}
				continue;
			}
			if(started) ended = true;
		}
		
	}

	if(!first_found)
	{
		((ArchivePanel*)archive_panel)->CurrentFileStack.push(((ArchivePanel*)archive_panel)->CurrentFileStack.top());
		first_found = true;
	}

	
	//FILELIST current_filelist;
	//current_filelist.resize(current_filemap.size());
	//std::copy(current_filemap.begin(), current_filemap.end(), current_filelist.begin());

	PluginPanelItem* ret = new PluginPanelItem[((ArchivePanel*)archive_panel)->current_filemap.size()];

	ZeroMemory(ret, sizeof(PluginPanelItem)*((ArchivePanel*)archive_panel)->current_filemap.size());

	int i = 0;
	for(std::vector<FILEDESC>::iterator i_file = ((ArchivePanel*)archive_panel)->current_filemap.begin(); i_file != ((ArchivePanel*)archive_panel)->current_filemap.end(); ++i_file)
	//for(size_t i = 0; i < current_filemap.size(); ++i)
	{
		//struct FAR_FIND_DATA ffd;

		//char aFileName[MAX_PATH];

		WideCharToMultiByte(CP_OEMCP,0,(*i_file).FindData.cFileName,-1,ret[i].FindData.cFileName,MAX_PATH, NULL, NULL);

		//std::string aFileName = to_multibyte(std::wstring((*i_file).first.cFileName));

		//const char* sdfsdf = aFileName.c_str();
		//strncpy(ret[i].FindData.cFileName, aFileName, MAX_PATH);
		ret[i].FindData.dwFileAttributes = (*i_file).FindData.dwFileAttributes;
		//ret[i].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY; //ohhha-ha-ha!!!
		ret[i].FindData.nFileSizeLow = (*i_file).FindData.nFileSizeLow;

		FILETIME lFileTime;

		DWORD FileTime = (*i_file).FindData.ftLastWriteTime.dwLowDateTime;

		DosDateTimeToFileTime (HIWORD(FileTime),
			LOWORD(FileTime), &lFileTime);
		LocalFileTimeToFileTime (&lFileTime, &ret[i].FindData.ftLastWriteTime);

		ret[i].UserData = (DWORD)&(*i_file);


		//ret[i].FindData.ftLastWriteTime = current_filelist[i].first.ftLastWriteTime;
		//ffd.cFileName = ((ArchivePanel*)archive_panel)->files[i].first.cFileName
		//memcpy(&(ret[i].FindData), &ffd, sizeof(ffd));
		++i;
	}

	//ret[1].FindData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	int in = ((ArchivePanel*)archive_panel)->current_filemap.size();
	//int in = 10;

	*pPanelItem = ret;
	*pItemsNumber = in;

	
	//pPanelItem = &ret;

	return TRUE;
}

void WINAPI _export  FreeFindData (
		HANDLE archive_panel,
		PluginPanelItem *pPanelItem,
		int nItemsNumber
		)
{
	delete[] pPanelItem;
}

int WINAPI _export SetDirectory (
		HANDLE archive_panel,
		const char *Dir,
		int nOpMode
		)
{
	
	

	std::string dir = Dir;
	if(dir == "..")
	{
		if(!(nOpMode & OPM_FIND))
		{
			((ArchivePanel*)archive_panel)->CurrentFileStack.pop();
		}	
		
		((ArchivePanel*)archive_panel)->CurrentFileStack.pop();

		std::string old_cur_folder = ((ArchivePanel*)archive_panel)->CurrentFolder;
		size_t last_slash = old_cur_folder.find_last_of('\\', old_cur_folder.length());

		if(last_slash == old_cur_folder.npos)
			((ArchivePanel*)archive_panel)->CurrentFolder = "";
		else
			((ArchivePanel*)archive_panel)->CurrentFolder = old_cur_folder.erase(last_slash, old_cur_folder.length() - last_slash);
	}
	else if(dir == "\\")
		((ArchivePanel*)archive_panel)->CurrentFolder = "";
	else	
	{
		if(((ArchivePanel*)archive_panel)->CurrentFolder == "")
		{
			((ArchivePanel*)archive_panel)->CurrentFolder = Dir;
		}
		else
		{
			((ArchivePanel*)archive_panel)->CurrentFolder += "\\";
			((ArchivePanel*)archive_panel)->CurrentFolder += Dir;
		}
		
	}
	

	return TRUE;
}

int WINAPI _export GetFiles(
	HANDLE archive_panel,
	struct PluginPanelItem *PanelItem,
	int ItemsNumber,
	int Move,
	char *DestPath,
	int OpMode
	)
{
	std::wstring xmlBuffer;
	std::wstring xmlHeader = L"<script>\n";
	std::wstring xmlFooter = L"</script>";

	std::wstring destPath = to_unicode(std::string(DestPath), std::locale(".OCP"));

	std::wstring files;

	for(int i = 0; i < ItemsNumber; ++i)
	{
		files += to_unicode(((ArchivePanel*)archive_panel)->CurrentFolder, std::locale(".OCP"));
		files += (((ArchivePanel*)archive_panel)->CurrentFolder == "")?L"":L"\\" ;
		files += ((FILEDESC*)(PanelItem[i].UserData))->FindData.cFileName;
		files += L"|";
	}

	xmlBuffer += xmlHeader;
	xmlBuffer += L"<ExtractWCXArchive source=\"";
	xmlBuffer += to_unicode(((ArchivePanel*)archive_panel)->ArchiveName, std::locale(".OCP"));
	xmlBuffer += L"\"";
	xmlBuffer += L" type=\"rar\" files=\"";
	xmlBuffer += files;
	xmlBuffer += L"\"";
	xmlBuffer += L" command=\"extract\" destination=\"";
	xmlBuffer += destPath;
	xmlBuffer += L"\"";
	xmlBuffer += L" flags=\"none\"/>\n";

	xmlBuffer += xmlFooter;
	
	MessageBoxW(NULL, xmlBuffer.c_str(), L"", 0);


	HANDLE hPipe = OpenPipe();
	if(!hPipe)
		return FALSE;

	try
	{
		SendCommandToPipe(hPipe, PCC_TAKE_XML);
		SendXMLToPipe(hPipe, (char*)xmlBuffer.c_str(), (xmlBuffer.length())*sizeof(wchar_t));

		GetHandleFromPipe(hPipe, hExtractThread);

		DWORD server_confirm = 0;
		GetCommandFromPipe(hPipe, server_confirm);

		if(server_confirm != PSC_OK)
			throw ClosePipeException(TEXT(""));

		SendCommandToPipe(hPipe, PCC_END_SESSION);

//		server_confirm = 0;
//		GetCommandFromPipe(hPipe, server_confirm);
//
//		if(server_confirm != PSC_OK)
//			throw ClosePipeException(TEXT(""));

	}
	catch(PipeErrorException peex)
	{
		ErrorLookup(peex.msg, peex.last_error);
	}
	catch(ClosePipeException cpex)
	{
		CloseHandle(hPipe);
		return FALSE;
	}

	CloseHandle(hPipe);

	if(hExtractThread != NULL)
         OpenProgressDialog(hExtractThread);

	return TRUE;
}




extern "C" int WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  if (DLL_PROCESS_ATTACH == fdwReason && hinstDLL) {
    /*GetModuleFileName(hinstDLL, iniFileName, MAX_PATH);
    for (int i = lstrlen(iniFileName) - 1; i >= 0 && iniFileName[i] != '.'; i--);
    if (i < 0) i = lstrlen(iniFileName);
    lstrcpy(&iniFileName[i], PHRASE_INI);*/
  }
  return TRUE;
}
