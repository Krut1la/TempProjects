#include "sdlinstaller.h"
#include <conio.h>


#undef ZLAYA
#undef NEITRAL
#undef CHILDREN
#undef GOLD
//#define ZLAYA
#define NEITRAL
//#define CHILDREN
//#define GOLD

bool isdown = false;
DWORD wx, wy;
RECT rc;

//win32
HINSTANCE hInstance = NULL;
//HWND hWnd = NULL;
HWND  hwndLoading = NULL;
HWND  hwndAW = NULL;

HBRUSH hBrush = NULL; 
BOOL  EasyActive = FALSE;
BOOL  Schon = FALSE;
BOOL  MyCBN_SELCHANGE = FALSE;
SDL_SysWMinfo sswmi = {0};
HDC AWDC;
HDC winDC;
HBITMAP AWBMP;

HDC FargusDC;
HDC FarguswinDC;
HBITMAP FargusBMP;

HANDLE CheckForALMutex;
//BROWSEINFO lpbi;

SDL_Color hddlow_sdlcolor;
bool schon = false;

STARTUPINFO si;
PROCESS_INFORMATION pi;

HWND glCheckHWnd = NULL;
HINSTANCE hInst = NULL;
HDC glCheckHDC = NULL;
HGLRC rv = NULL;

COMBOPATHLIST ComboPathList;

pGetDiskFreeSpaceExF pGetDiskFreeSpaceEx;
pGetFileSizeExF pGetFileSizeEx;

int WinWidth  = 939;
int WinHeight =  524;
float KoeffX  =    1;
float KoeffY  =    1;

int RSpeed = 500, RStart, REnd;

//static strings
static const char ShutdownName[] = "SeShutdownPrivilege";
static const char ErrExitWindowsEx[] = "ExitWindowsEx";
//static const char MsgConfirm1[] = "Shut down the system?";
static const char MsgConfirm1[] = "После установки игры, компьютер необходимо перезагрузить. Сделать это сейчас?";
//static const char MsgConfirm2[] = "Confirmation";
static const char MsgConfirm2[] = "Подтверждение";
static const char ErrError[] = "Error";

//Layout
extern SDLInstallerLayout *handler;
//Installer
extern Installer *installer;
extern HANDLE InstallerThread;
extern unsigned InstallerThreadId;
extern CRITICAL_SECTION InstallerCS;

extern InfoExpander *infoexpander;


//ParaGui objects
PG_Application app;

DWORD instH = NULL;


void InitFail(const char *message)
{
	MessageBox( NULL, message, "Ошибка", MB_OK | MB_ICONERROR );
}

void DeleteAll(void)
{
  // Release resources used by the critical section object.
  DeleteCriticalSection(&InstallerCS);
  CloseHandle(InstallerThread);

  EndDialog (infoexpander->hwndEasyInstaller, TRUE); infoexpander->hwndEasyInstaller = 0;
  delete handler; 
  delete installer; 
  delete infoexpander;
  if(AWDC) DeleteDC(AWDC);
//  ReleaseDC(winDC);
  if(AWBMP) DeleteObject(AWBMP);
  ReleaseMutex(CheckForALMutex);
}

void GetPaths(char* argv0)
{
	char *filepart = NULL;
    char *retval = (char *) malloc(sizeof (TCHAR) * (MAX_PATH + 1));
    DWORD buflen = GetModuleFileName(NULL, retval, MAX_PATH + 1);
	char *ptr;

    retval[buflen] = '\0';  /* does API always null-terminate the string? */

	infoexpander->FullModuleFileName = retval;

	/* make sure the string was not truncated. */
    if (strcmp(&retval[buflen - 4], ".exe") == 0)
    {
        ptr = strrchr(retval, '\\');
        if (ptr != NULL)
        {
            *(ptr + 1) = '\0';  /* chop off filename. */

            /* free up the bytes we didn't actually use. */            
            retval = (char *) realloc(retval, strlen(retval) + 1);
            if (retval != NULL)
				infoexpander->ModuleFilePath = retval;
			    free(retval);
			    return;
        } /* if */
    } /* if */

    /* if any part of the previous approach failed, try SearchPath()... */
    buflen = SearchPath(NULL, argv0, NULL, buflen, NULL, NULL);
    retval = (char *) realloc(retval, buflen);
	if(!retval) PG_LogERR("getExePath: OUT_OF_MEMORY");
	SearchPath(NULL, argv0, NULL, buflen, retval, &filepart);
	infoexpander->ModuleFilePath = retval;
	free(retval);
	return;
}

PARAGUI_CALLBACK(handle_exit) {
  PARAGUI_CALLBACK(handle_CANCEL);
  SysQuit();
  return TRUE;
}

PARAGUI_CALLBACK(handle_AWLOGO) {
  //if(!hwndAW) AWWindow();

	SysQuit();
	PG_Widget *bg = handler->GetWizard()->GetWidgetByName("Background");
	bg->StartWidgetDrag();

  return TRUE;
}

PARAGUI_CALLBACK(handle_INSTALL) {
	PG_Widget *editpath = handler->GetWizard()->GetWidgetByName("Path");
    if(editpath) infoexpander->MainDir = editpath->GetText();
	if(infoexpander->MainDir.length()>=3)
			{
			std::string MainDrive = infoexpander->MainDir;
		    MainDrive.erase(3,MainDrive.length());
		    infoexpander->MainDrive = MainDrive;
			}

  handler->GetWizard()->ShowNextPage();
  handler->GetWizard()->ShowFirstScreenShot();

  PG_Widget *cancel = handler->GetWizard()->GetWidgetByAction("cancel");
  if(cancel) 
  {
	  cancel->EnableReceiver(true);
	  cancel->SetFontAlpha(255);
	  cancel->Redraw();
  }
  PG_Widget *ready = handler->GetWizard()->GetWidgetByName("BReady");
  if(ready) 
  {
	  ready->EnableReceiver(false);
	  ready->SetFontAlpha(100);
	  ready->Redraw();
  }

  //handler->GetWizard()->GetWidgetByAction("cancel")->EnableReceiver(true);
 // handler->GetWizard()->GetWidgetByName("BReady")->EnableReceiver(false);
	
	if(!InstallerThread) 
	{
		InstallerThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadProc, NULL, 0,&InstallerThreadId );
    	SetThreadPriority(InstallerThread, THREAD_PRIORITY_NORMAL);
	}
	    
    return TRUE;
}

PARAGUI_CALLBACK(handle_CANCEL) {
	UserCancel();
	//TerminateThread(InstallerThread, -1);
	WaitForSingleObject( InstallerThread, INFINITE );

	CloseHandle(InstallerThread);
	InstallerThread = NULL;
	handler->GetWizard()->ShowPreviousPage();
    handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);

	return TRUE;
}

PARAGUI_CALLBACK(handle_MINIMIZE) {
	ShowWindow(infoexpander->hWndMain,SW_MINIMIZE);
	return TRUE;
}

PARAGUI_CALLBACK(handle_CHANGELANGR) {
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_RADIO_RUSSIAN),BM_SETCHECK,BST_CHECKED,0);
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_RADIO_ENGLISH),BM_SETCHECK,BST_UNCHECKED,0);
	PG_Widget *w = NULL; 
	w = handler->GetWizard()->GetWidgetByName("BInstall"); 	    if(w) w->SetText("УСТАНОВИТЬ");
	w = handler->GetWizard()->GetWidgetByName("BStart"); 	    if(w) w->SetText("ИГРАТЬ");
	w = handler->GetWizard()->GetWidgetByName("BUninstall"); 	if(w) w->SetText("УДАЛИТЬ");
	w = handler->GetWizard()->GetWidgetByName("BQuit"); 	    if(w) w->SetText("ВЫХОД");
	w = handler->GetWizard()->GetWidgetByName("lAnnounce"); 	if(w) w->SetText("ФАРГУС ПРЕДСТАВЛЯЕТ");
	w = handler->GetWizard()->GetWidgetByName("BMInstall"); 	if(w) w->SetText("УСТАНОВИТЬ");
	w = handler->GetWizard()->GetWidgetByName("BPrev"); 	    if(w) w->SetText("НАЗАД");
	w = handler->GetWizard()->GetWidgetByName("BBrowse"); 	    if(w) w->SetText("ВЫБОР ПАПКИ");
	w = handler->GetWizard()->GetWidgetByName("L01"); 	        if(w) w->SetText("Игра будет установлена в:");
	w = handler->GetWizard()->GetWidgetByName("L02"); 	        if(w) w->SetText("Требуется свободного места:");
	w = handler->GetWizard()->GetWidgetByName("L03"); 	        if(w) w->SetText("Свободно на диске:");
	w = handler->GetWizard()->GetWidgetByName("L4"); 	        if(w) w->SetText("Выполняется:");

	w = handler->GetWizard()->GetWidgetByName("L01f"); 	        if(w) w->SetText("Игра будет установлена в:");
	w = handler->GetWizard()->GetWidgetByName("L02f"); 	        if(w) w->SetText("Требуется свободного места:");
	w = handler->GetWizard()->GetWidgetByName("L03f"); 	        if(w) w->SetText("Свободно на диске:");
	w = handler->GetWizard()->GetWidgetByName("L4f"); 	        if(w) w->SetText("Выполняется:");

	w = handler->GetWizard()->GetWidgetByName("BReady"); 	    if(w) w->SetText("ГОТОВО");
	w = handler->GetWizard()->GetWidgetByName("BCancel"); 	    if(w) w->SetText("ОТМЕНА");
	
	return TRUE;
}
PARAGUI_CALLBACK(handle_CHANGELANGE) {
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_RADIO_ENGLISH),BM_SETCHECK,BST_CHECKED,0);
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_RADIO_RUSSIAN),BM_SETCHECK,BST_UNCHECKED,0);
	PG_Widget *w = NULL;
    w = handler->GetWizard()->GetWidgetByName("BInstall"); 	    if(w) w->SetText("INSTALL");
	w = handler->GetWizard()->GetWidgetByName("BStart"); 	    if(w) w->SetText("START");
	w = handler->GetWizard()->GetWidgetByName("BUninstall"); 	if(w) w->SetText("UNINSTALL");
	w = handler->GetWizard()->GetWidgetByName("BQuit"); 	    if(w) w->SetText("QUIT");
	w = handler->GetWizard()->GetWidgetByName("lAnnounce"); 	if(w) w->SetText("    FARGUS PRESENTS");
	w = handler->GetWizard()->GetWidgetByName("BMInstall"); 	if(w) w->SetText("INSTALL");
	w = handler->GetWizard()->GetWidgetByName("BPrev"); 	    if(w) w->SetText("BACK");
	w = handler->GetWizard()->GetWidgetByName("BBrowse"); 	    if(w) w->SetText("FOLDER SELECTION");
	w = handler->GetWizard()->GetWidgetByName("L01"); 	        if(w) w->SetText("Game will be installed in:");
	w = handler->GetWizard()->GetWidgetByName("L02"); 	        if(w) w->SetText("Required space:");
	w = handler->GetWizard()->GetWidgetByName("L03"); 	        if(w) w->SetText("Free space:");
	w = handler->GetWizard()->GetWidgetByName("L4"); 	        if(w) w->SetText("In progress:");

	w = handler->GetWizard()->GetWidgetByName("L01f"); 	        if(w) w->SetText("Game will be installed in:");
	w = handler->GetWizard()->GetWidgetByName("L02f"); 	        if(w) w->SetText("Required space:");
	w = handler->GetWizard()->GetWidgetByName("L03f"); 	        if(w) w->SetText("Free space:");
	w = handler->GetWizard()->GetWidgetByName("L4f"); 	        if(w) w->SetText("In progress:");

	w = handler->GetWizard()->GetWidgetByName("BReady"); 	    if(w) w->SetText("READY");
	w = handler->GetWizard()->GetWidgetByName("BCancel"); 	    if(w) w->SetText("CANCEL");
    return TRUE;
}

PARAGUI_CALLBACK(handle_EASY) {
	//app.UnlockScreen();
	//SDL_UpdateRect(app.GetScreen(), 0, 0,WinWidth,WinHeight);

		EasyActive = TRUE;Schon = TRUE;
	return TRUE;
}

PARAGUI_CALLBACK(handle_previous_page) {
	CheckHDD();
	handler->GetWizard()->ShowPreviousPage();
	return TRUE;
}

PARAGUI_CALLBACK(handle_next_page) {

  CheckHDD();
  handler->GetWizard()->ShowNextPage();
  handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);
  handler->GetWizard()->HideAnnounce(infoexpander->CurrentAnnounce);
  return TRUE;
}

PARAGUI_CALLBACK(handle_requirements_page) {

  CheckHDD();
  if(strcmp(handler->GetWizard()->GetPage(handler->GetWizard()->GetCurrentPage())->GetName(), "requirements") != 0)
  {
	  handler->GetWizard()->HideAllPages();
	  handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);
      handler->GetWizard()->ShowPage("requirements");//ShowNextPage();
  }
	return TRUE;
}
PARAGUI_CALLBACK(handle_description_page) {

  CheckHDD();
  if(strcmp(handler->GetWizard()->GetPage(handler->GetWizard()->GetCurrentPage())->GetName(), "description") != 0)
  {
     handler->GetWizard()->HideAllPages();
	 handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);
     handler->GetWizard()->ShowPage("description");
  }
  return TRUE;
}
PARAGUI_CALLBACK(handle_screenshots_page) {

  CheckHDD();
  if(strcmp(handler->GetWizard()->GetPage(handler->GetWizard()->GetCurrentPage())->GetName(), "screenshots") != 0)
  {
     handler->GetWizard()->HideAllPages();
     handler->GetWizard()->ShowPage("screenshots");
	 handler->GetWizard()->ShowFirstScreenShot();
  }
  return TRUE;
}
PARAGUI_CALLBACK(handle_cheats_page) {

  CheckHDD();
  if(strcmp(handler->GetWizard()->GetPage(handler->GetWizard()->GetCurrentPage())->GetName(), "cheats") != 0)
  {
	  handler->GetWizard()->HideAllPages();
	  handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);
      handler->GetWizard()->ShowPage("cheats");//ShowNextPage();
  }
  return TRUE;
}
PARAGUI_CALLBACK(handle_install_page) {

  CheckHDD();
  if(strcmp(handler->GetWizard()->GetPage(handler->GetWizard()->GetCurrentPage())->GetName(), "install") != 0)
  {
	  handler->GetWizard()->HideAllPages();
	  handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);
      handler->GetWizard()->ShowPage("install");//ShowNextPage();
  }
  return TRUE;
}



PARAGUI_CALLBACK(handle_first_page) {
	handler->GetWizard()->ShowFirstPage();
  handler->GetWizard()->HideScreenShot(infoexpander->CurrentScreenShot);
  handler->GetWizard()->HideAnnounce(infoexpander->CurrentAnnounce);
  handler->GetWizard()->ShowFirstAnnounce();
	return TRUE;
}

PARAGUI_CALLBACK(handle_last_page) {
	handler->GetWizard()->ShowLastPage();
	return TRUE;
}

PARAGUI_CALLBACK(handle_previous_screenshot) {
	handler->GetWizard()->ShowPreviousScreenShot(TRUE);
	return TRUE;
}

PARAGUI_CALLBACK(handle_next_screenshot) {
	handler->GetWizard()->ShowNextScreenShot(TRUE);
	return TRUE;
}

PARAGUI_CALLBACK(handle_first_screenshot) {
	handler->GetWizard()->ShowFirstScreenShot();
	return TRUE;
}

PARAGUI_CALLBACK(handle_last_screenshot) {
	handler->GetWizard()->ShowLastScreenShot();
	return TRUE;
}
 //-------------------------------
PARAGUI_CALLBACK(handle_previous_announce) {
  handler->GetWizard()->ShowPreviousAnnounce(TRUE);
  return TRUE;
}

PARAGUI_CALLBACK(handle_next_announce) {
  handler->GetWizard()->ShowNextAnnounce(TRUE);
  return TRUE;
}

PARAGUI_CALLBACK(handle_first_announce) {
  handler->GetWizard()->ShowFirstAnnounce();
  return TRUE;
}

PARAGUI_CALLBACK(handle_last_announce) {
  handler->GetWizard()->ShowLastAnnounce();
  return TRUE;
}

PARAGUI_CALLBACK(handle_UNINSTALL) {
	//query for previous installation
	
  HKEY hk;
  LPWIN32_FIND_DATA lpFindFileData = new WIN32_FIND_DATA;
  HANDLE filename = NULL;
  unsigned char buffer[255] = {0};
  DWORD cbbuffer = 255;
  std::string subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + infoexpander->Title;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
  {
    //query for uninstal.exe
    RegQueryValueEx(hk,"UninstallString",0, NULL, &buffer[0], &cbbuffer);
    filename = FindFirstFile((LPCSTR)buffer,lpFindFileData);
    if(filename != INVALID_HANDLE_VALUE) 
    {
    	
		//if(infoexpander->InstallLog) CloseHandle(infoexpander->InstallLog);
		//if(filename) CloseHandle(filename);
		FindClose(filename);
        RegCloseKey(hk);
        delete lpFindFileData;
		WinExec((LPCSTR)buffer, SW_SHOWNORMAL);
		SysQuit();
    }
  }
  //if(filename) CloseHandle(filename);
  FindClose(filename);

  RegCloseKey(hk);
  delete lpFindFileData;
  return TRUE;
}


PARAGUI_CALLBACK(handle_START) {
  HKEY hk;
  LPWIN32_FIND_DATA lpFindFileData = new WIN32_FIND_DATA;
  HANDLE filename = NULL;
  unsigned char buffer[255] = {0};
  DWORD cbbuffer = 255;
  std::string subkey = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + infoexpander->Title;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
  {
    //query for start.exe    
    RegQueryValueEx(hk,"StartString",0, NULL, &buffer[0], &cbbuffer);
    filename = FindFirstFile((LPCSTR)buffer,lpFindFileData);
    if(filename != INVALID_HANDLE_VALUE) 
    {
    	
		//if(infoexpander->InstallLog) CloseHandle(infoexpander->InstallLog);
		//if(filename) CloseHandle(filename);
		FindClose(filename);
        RegCloseKey(hk);
        delete lpFindFileData;
		std::string path = (LPCSTR)buffer;
		DWORD start = path.find_last_of("\\",path.length());
		DWORD end   = path.length() - start;
		path.replace(start + 1,end,"");
		SetCurrentDirectory(path.c_str());
		WinExec((LPCSTR)buffer, SW_SHOWNORMAL);
		SysQuit();
    }
  }
//  if(filename) CloseHandle(filename);
  FindClose(filename);

  RegCloseKey(hk);
  delete lpFindFileData;
  return TRUE;
}

PARAGUI_CALLBACK(handle_EDITPATH)
{
  
  PG_Widget *editpath = handler->GetWizard()->GetWidgetByName("Path");
  if(editpath) infoexpander->MainDir = editpath->GetText();
  if(infoexpander->MainDir.length()>=3)
  {
    std::string MainDrive = infoexpander->MainDir;
    MainDrive.erase(3,MainDrive.length());
    infoexpander->MainDrive = MainDrive;
  }
  CheckHDD();
  return TRUE;
}
PARAGUI_CALLBACK(handle_BROWSE) {

  LPITEMIDLIST pidlRoot = NULL;
  LPITEMIDLIST pidlSelected = NULL;
  BROWSEINFO bi = {0};
  LPMALLOC pMalloc = NULL;

  CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
  SHGetMalloc(&pMalloc);

 /* if(nCSIDL)
  {
    SHGetFolderLocation(hwnd, nCSIDL, NULL, NULL, &pidlRoot);
  }
   */
//  else
  {
    pidlRoot = NULL;
  }

  // LPTSTR pszDisplayName = new TCHAR[MAX_PATH];


  //bi.hwndOwner = infoexpander->hWndMain;

  bi.pidlRoot = pidlRoot;
  //bi.pszDisplayName = pszDisplayName;
  bi.lpszTitle = "Выберите папку";
  bi.ulFlags = 0;
  bi.lpfn = NULL;
  bi.lParam = 0;
    
  pidlSelected = SHBrowseForFolder(&bi);
  //pidlSelected = SHBrowseForFolder(&bi);

 
  //return pidlSelected;


  //LPITEMIDLIST lpil;
  //lpbi = new BROWSEINFO;
  //lpil = SHBrowseForFolder(&lpbi);

  if(pidlSelected)
  {
	  LPSTR path = new char[MAX_PATH];
    SHGetPathFromIDList(pidlSelected,path);
	  infoexpander->MainDir = path;
    	 
    if(infoexpander->MainDir.length() > 3) infoexpander->MainDir+="\\";
    infoexpander->MainDir+=infoexpander->Title;
	  infoexpander->ExpandString(&(infoexpander->MainDir),&(infoexpander->MainDir));
	  if(infoexpander->MainDir.length()>=3)
			{
				std::string MainDrive = infoexpander->MainDir;
		    MainDrive.erase(3,MainDrive.length());
		    infoexpander->MainDrive = MainDrive;
			}
      PG_Widget *editpath = handler->GetWizard()->GetWidgetByName("Path");
      if(editpath) editpath->SetText(infoexpander->MainDir.c_str());
    

	  delete path;
	  CheckHDD();
  }
     
  if(pidlRoot)
  {
    pMalloc->Free(pidlRoot);
  }
  pMalloc->Release();
  CoUninitialize();
  SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_COMBO_PATH),WM_SETTEXT,0,(LPARAM)infoexpander->MainDir.c_str());
  return TRUE;
}

PARAGUI_CALLBACK(handle_WWW) {
  ShellExecute(NULL,"open","www.fargus.ua",NULL,NULL,SW_SHOWNORMAL);
  return TRUE;
}

PARAGUI_CALLBACK(handle_INSDX) {
    si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;
		
	if(!CreateProcess(infoexpander->dxpath.c_str(),NULL,NULL,NULL,false,CREATE_DEFAULT_ERROR_MODE,NULL,NULL,&si,&pi))
	{
	   MessageBox( 0, "а где инсталяха DirectX делась?", "Ощибка", MB_ICONERROR);
	}
	WaitForSingleObject(pi.hProcess,INFINITE);
  return TRUE;
}

BOOL InitParaGuiObject(void)
{	
	return TRUE;
}



BOOL InitSDL(void)
{
	
	if(!app.InitScreen(WinWidth,WinHeight, 0, SDL_SWSURFACE|SDL_NOFRAME)) return false;
	//if(!app.InitScreen(10,10, 0, SDL_SWSURFACE|SDL_NOFRAME)) return false;
	//app.LockScreen();
    //SDL_UpdateRect(app.GetScreen(), 0, 0,WinWidth,WinHeight);
	//app.UnlockScreen();
//	if(!app.LoadTheme(infoexpander->ThemeFileName.c_str())) return false;
    if(!app.LoadTheme("theme")) return false;
	app.SetCursor(app.GetTheme()->FindSurface("Pointer", "Pointer", "normal"));
	app.SetCaption(infoexpander->Caption.c_str(),"");
    SDL_GetWMInfo(&sswmi);
	//ShowWindow(sswmi.window, SW_HIDE);
    infoexpander->hWndMain = sswmi.window;
	HRGN formrgn = NULL;
	RGNDATA rd;
	
	rd.rdh.dwSize = sizeof(RGNDATAHEADER);
	rd.rdh.iType = RDH_RECTANGLES;
	rd.rdh.nCount =
		rd.rdh.nRgnSize = 5523;
	SetRect(&rd.rdh.rcBound, MAXLONG,
                      MAXLONG, 0, 0);
	

	//memcpy(





	GetWindowRgn(infoexpander->hWndMain,formrgn);
	DeleteObject(formrgn);
	/*
	POINT polyregion[] = {{73,126},{453,127},{475,141},{475,132},{562,132},
	                      {566,137},{568,131},{666,131},{671,136},{674,131},
	                      {762,130},{766,134},{770,130},{860,129},{866,136},{873,130},{975,130},
	                      {1004,183},{1002,184},{1003,185},{1007,187},{1007,581},{1004,582},{1005,584},
	                      {975,638},{874,638},{865,632},{860,638},{772,637},{767,633},{762,638},
	                      {674,638},{671,633},{666,638},{568,638},{566,632},{561,637},{474,636},{475,629},{482,622},{481,620},{453,641},{73,642}};
	*/

	POINT polyregion[] = {{73,126},{453,127},{475,141},{475,132},{562,132},
	                      {566,137},{568,131},{666,131},{671,136},{674,131},
	                      {762,130},{766,134},{770,130},{860,129},{866,136},
	                      {873,130},{975,130},{1004,183},{1002,184},{1003,185},
	                      {1007,187},{1007,581},{1004,582},{1005,584},{975,638},
	                      {874,638},{865,632},{860,638},{772,637},{767,633},{762,638},
	                      {674,638},{671,633},{666,638},{568,638},{566,632},{561,637},
	                      {474,636},{475,629},{482,622},{481,620},{453,641},{73,642}};

	for(int i = 0; i < sizeof(polyregion)/sizeof(POINT); i++)
	{
		polyregion[i].x = (LONG)(polyregion[i].x*KoeffX);
		polyregion[i].y = (LONG)(polyregion[i].y*KoeffY);
	}

	//formrgn = ExtCreateRegion(NULL, sizeof(RgnData), (const RGNDATA*)&RgnData);
	formrgn = CreatePolygonRgn((const POINT*)&polyregion, sizeof(polyregion)/sizeof(POINT), WINDING);
	//SetWindowRgn(infoexpander->hWndMain, formrgn, true);


	return TRUE;
}

void SaveInstallerSettings(void)
{
	char buffer2[255] = {0};
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_COMBO_PATH),WM_GETTEXT,(WPARAM)255,(LPARAM)&buffer2[0]);
	std::string combopath2 = buffer2;
	combopath2 = combopath2.replace(combopath2.find_last_of("\\",combopath2.length()),combopath2.length(),"");
	ComboPathList.push_back(combopath2);

	//saving controls state
	HKEY hk;
	unsigned char buffer[255] = {0};
	DWORD cbbuffer = 255;
	std::string subkey = "Software\\Fargus\\InstallerState";
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, 0, REG_OPTION_NON_VOLATILE,KEY_WRITE, NULL, &hk, NULL) == ERROR_SUCCESS)
	{
		//query for start.exe    
		std::string version;
		PG_RadioButton *eng = NULL;
		eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");
		if(eng && eng->GetPressed())
			version = "english";
		else
			version = "russian";		
		RegSetValueEx(hk,"Version", 0, REG_SZ, (CONST BYTE *)version.c_str(), version.size());
        
		if(EasyActive)
		    RegSetValueEx(hk,"Easy", 0, REG_SZ, (CONST BYTE *)"1", 2);
		else
			RegSetValueEx(hk,"Easy", 0, REG_SZ, (CONST BYTE *)"0", 2);

		COMBOPATHLIST::iterator i_path;
		UINT i = 1;
		i_path = ComboPathList.begin();
		while(i_path != ComboPathList.end() && i <= 10)
		{
			char ValueName[12] = {0};
			sprintf(ValueName,"ComboPath%d",i);
			RegSetValueEx(hk,ValueName, 0, REG_SZ, (CONST BYTE *)(*i_path).c_str(), (*i_path).size());
			  i_path++;
			  i++;
		}
	}
}

void LoadInstallerSettings(void)
{
	//saving controls state
	HKEY hk;
	unsigned char buffer[255] = {0};
	DWORD cbbuffer = 255;
	std::string subkey = "Software\\Fargus\\InstallerState";
	if(RegCreateKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, 0, REG_OPTION_NON_VOLATILE,KEY_QUERY_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS)
	{
		//query for start.exe    
		
		PG_RadioButton *eng = NULL;
		eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");
		PG_RadioButton *rus = NULL;
		rus	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_russian");
		rus->SetPressed();//make default
		HRESULT hr = RegQueryValueEx(hk,"Version", 0, NULL, &buffer[0], &cbbuffer);
		if(hr == ERROR_SUCCESS)
		{
		    std::string version = (LPCSTR)buffer;
		    if(eng&&rus)
			   if(version == "russian")
			   {
				  SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_RADIO_RUSSIAN),BM_SETCHECK,BST_CHECKED,0);
			      rus->SetPressed();
			   }
			   else
			   {
                  SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_RADIO_ENGLISH),BM_SETCHECK,BST_CHECKED,0);   
 				  eng->SetPressed();
			   }
		}

		cbbuffer = 255;
		memset(buffer,0,cbbuffer);
		hr = RegQueryValueEx(hk,"Easy", 0, NULL, &buffer[0], &cbbuffer);
		if(hr == ERROR_SUCCESS)
		{
			std::string easy = (LPCSTR)buffer;
				if(easy == "1")
				{
				    EasyActive = TRUE; Schon = TRUE;
				}
				else
				{
					EasyActive = FALSE; Schon = TRUE;
				}
		}

		UINT i = 1;
		while(TRUE)
		{
			char ValueName[12] = {0};
			cbbuffer = 255;
			memset(buffer,0,cbbuffer);
			sprintf(ValueName,"ComboPath%d",i);
			if(RegQueryValueEx(hk,ValueName, 0, NULL, &buffer[0], &cbbuffer) != ERROR_SUCCESS) 
				break;
			std::string combopath = (LPCSTR)buffer;
			ComboPathList.push_back(combopath);
			SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_COMBO_PATH),CB_ADDSTRING, NULL,(LPARAM)combopath.c_str());
			i++;
		}
	}
}

void SysQuit(void)
{
	//PG_Application* app = (PG_Application*)clientdata;

	//SaveInstallerSettings();
	
	PG_LogMSG("Quit");
	DeleteAll();
	app.Quit();
	exit(0);

}

void SysRestart(void)
{
	if(MessageBox( NULL, &MsgConfirm1[0], &MsgConfirm2[0], MB_YESNO | MB_ICONQUESTION ) == IDYES){

		PG_LogMSG("Reboot");
		DeleteAll();
		app.Quit();

		HANDLE hToken; 
		TOKEN_PRIVILEGES tkp; 

		// Get a token for this process. 

		OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); 

		// Get the LUID for the shutdown privilege. 

		LookupPrivilegeValue(NULL, &ShutdownName[0], &tkp.Privileges[0].Luid); 

		tkp.PrivilegeCount = 1;  // one privilege to set    
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

		// Get the shutdown privilege for this process. 

		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 

		if (!ExitWindowsEx(EWX_REBOOT|EWX_FORCE,0)){
			MessageBox( NULL, &ErrExitWindowsEx[0], ErrError, MB_OK | MB_ICONERROR );
		}
	}

}

BOOL ShowMisc(void)
{  
  	handler->GetWizard()->ShowFrame("misc");
	//handler->GetWizard()->ShowFirstScreenShot();
    
	handler->GetWizard()->ShowFirstPage();
  handler->GetWizard()->ShowFirstAnnounce();
    return TRUE;
}


int main( int argc, char* argv[] )
{
    //Check for allready launched
	CheckForALMutex = CreateMutex(NULL, false,"Fargus");
	if(GetLastError() == ERROR_ALREADY_EXISTS){
		ReleaseMutex(CheckForALMutex);

		//MessageBox( 0, "Mutex", " ", MB_OKCANCEL | MB_ICONQUESTION);
		exit(0);
	}

	

    pGetDiskFreeSpaceEx = (pGetDiskFreeSpaceExF)GetProcAddress( GetModuleHandle("kernel32.dll"),"GetDiskFreeSpaceExA");
    pGetFileSizeEx = (pGetFileSizeExF)GetProcAddress( GetModuleHandle("kernel32.dll"),"GetFileSizeEx");


	LoadScript("script.xml");

	CheckDX();
//	CheckGL();
	CheckAll();
	//EasyWindow();
  

	WinWidth  = GetSystemMetrics(SM_CXSCREEN);
	WinHeight = GetSystemMetrics(SM_CYSCREEN);
	//WinWidth  = 800;
	//WinHeight = 600;

	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0);
	//WinWidth = rect.right - rect.left;
	//WinHeight = rect.bottom - rect.top;

	//WinWidth  = GetSystemMetrics(SM_CXFULLSCREEN);
	//WinHeight  = GetSystemMetrics(SM_CYFULLSCREEN);

	KoeffX    = (float)(WinWidth/1024.0f);
	KoeffY    = (float)(WinHeight/768.0f);

	WinWidth  = 939;
	WinHeight = 524;


	//LoadingWindow();

	GetPaths(argv[0]);


	if(!InitSDL()) SysQuit();

	LoadLayout(infoexpander->LayoutFileName.c_str(), KoeffX , KoeffY);
	

//	LoadInstallerSettings();
	handler->GetDynamicControls()->Refresh();
	ShowMisc();
	
	//DestroyWindow(hwndLoading);

	
	//app.ShowCursor(PG_CURSOR_SOFTWARE);

    //SDL_Event event;
	MSG msg;

	SetTimer(infoexpander->hWndMain, 1, 400, RefreshThreadProc);

    
    

    //message pump
//	RStart = GetTickCount();
  /*  for (;;)
    {


		
		if(PeekMessage(&msg, infoexpander->hWndMain, 0, 0, 0))
	     //if(GetMessage(&msg, NULL, 0, 0))
		 {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		 }

         //look for an event
         if (SDL_PollEvent ( &event ) )
         {
			 		
	         //app.ProcessEvent(&event);
			 if(event.type == SDL_MOUSEBUTTONDOWN)
			 {
				 
				 
                 isdown = true;
				 wx = event.motion.x;
				 wy = event.motion.y;
			 }
			 if(event.type == SDL_MOUSEBUTTONUP)
                 isdown = false;
			 if(event.type == SDL_MOUSEMOTION)
				 if(isdown && event.motion.xrel != 0 && event.motion.yrel != 0)
				 {					 
					 GetWindowRect(infoexpander->hWndMain, &rc);
					 
					 //MoveWindow(infoexpander->hWndMain, rc.left + event.motion.xrel, rc.top + event.motion.yrel, 939, 524, true);
					 
					 MoveWindow(infoexpander->hWndMain, rc.left + event.motion.x - wx, rc.top + event.motion.y - wy, 939, 524, true);
					 //wx = event.motion.x;
				     //wy = event.motion.y;
					 continue;
					 
				 }
			 app.PumpIntoEventQueue(&event);
		    //app.DrawCursor();
			 
		 }
		 
	 //REnd = GetTickCount();
	 //if(REnd - RStart >= RSpeed){
	    //RStart = REnd;

		



//	 }
		 
		 
    }//end of message pump*/
/*	app.eventMouseButtonDown = &my_eventMouseButtonDown;
	app.Run();
*/

	PG_Application* object = static_cast<PG_Application*>(&app);
	SDL_Event event;
	//app.my_quitEventLoop = false;

	//assert(&app);

	while(1) {
		
		// pull motion events (may flood the eventqueue)
		while(SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTIONMASK) > 0)
			;
		
		/*if(enableAppIdleCalls) 
		{
			if (SDL_PollEvent(&event) == 0) 
			{
				object->eventIdle();
			} 
			else 
			{
				PG_MessageObject::PumpIntoEventQueue(&event);
			}
		} 
		else*/ 
			SDL_WaitEvent(&event);
			//look for an event
         	 		
	         //app.ProcessEvent(&event);
			 if(event.type == SDL_MOUSEBUTTONDOWN)
			 {
				 
				 
                 isdown = true;
				 wx = event.motion.x;
				 wy = event.motion.y;
			 }
			 if(event.type == SDL_MOUSEBUTTONUP)
                 isdown = false;
			 if(event.type == SDL_MOUSEMOTION)
				 if(isdown/* && event.motion.xrel != 0 && event.motion.yrel != 0*/)
				 {					 
					 GetWindowRect(infoexpander->hWndMain, &rc);
					 
					 //MoveWindow(infoexpander->hWndMain, rc.left + event.motion.xrel, rc.top + event.motion.yrel, 939, 524, true);
					 
					 MoveWindow(infoexpander->hWndMain, rc.left + event.motion.x - wx, rc.top + event.motion.y - wy, 939, 524, true);
					 //wx = event.motion.x;
				     //wy = event.motion.y;
					 //continue;
					 
				 }
		 	PG_MessageObject::PumpIntoEventQueue(&event);
			//DrawCursor();
	}

	return msg.wParam;
    //return EXIT_SUCCESS;
}

BOOL my_eventMouseButtonDown(const SDL_MouseButtonEvent *button)
{
	return TRUE;

}
	

int LoadingWindow(void)
{

	hwndLoading = CreateDialog(0, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);
    if (!hwndLoading) {
        MessageBox( 0, "Couldn't Create Window", "Ошибка", MB_ICONERROR);
    }

	ShowWindow( hwndLoading, SW_SHOWDEFAULT );

	UpdateWindow(hwndLoading);
	SetFocus (hwndLoading);

	return 0;
}

int AWWindow(void)
{
	hwndAW = CreateDialog(0, MAKEINTRESOURCE(IDD_DIALOG2), NULL, DialogProc);
    if (!hwndAW) {
        MessageBox( 0, "Couldn't Create Window", "Ошибка", MB_ICONERROR);
	}

	// Loading AW logo 
	AWBMP   = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BITMAP1));
    winDC   = GetWindowDC(hwndAW);
    AWDC   = CreateCompatibleDC(winDC); 
	SelectObject(AWDC,AWBMP);


    ShowWindow( hwndAW, SW_SHOWDEFAULT );

	UpdateWindow(hwndAW);
	SetFocus (hwndAW);

	return 0;
}

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
    LONG    lRet = 1;
    switch ( uMsg ) {
	    case WM_COMMAND:
		     switch(LOWORD(wParam))
			 {
		         case IDOK:
                      EndDialog (hwndAW, TRUE); hwndAW = 0;
		              break;
                 case IDART:
                      ShellExecute(NULL,"open","www.artemenko.com",NULL,NULL,SW_SHOWNORMAL);
				      EndDialog (hwndAW, TRUE); hwndAW = 0;
		              break;
                 case IDKRU:
                      ShellExecute(NULL,"open","www.krutila.kiev.ua",NULL,NULL,SW_SHOWNORMAL);
				      EndDialog (hwndAW, TRUE); hwndAW = 0;
                 case IDFONTA:
                      ShellExecute(NULL,"open","www.fonta.narod.ru",NULL,NULL,SW_SHOWNORMAL);
				      EndDialog (hwndAW, TRUE); hwndAW = 0;
		              break;
              } 
		     break;

        case WM_SIZE:
             break;
        case WM_CLOSE:
             EndDialog (hwndAW, TRUE); hwndAW = 0;
             break;
        case WM_DESTROY:
			 DeleteObject(AWBMP);
			 ReleaseDC(hwndAW,AWDC);
			 ReleaseDC(hwndAW,winDC);
             PostQuitMessage( 0 );
		     break;
        case WM_QUIT:
		     EndDialog (hwndAW, TRUE); hwndAW = 0;
             break;
        case WM_PAINT:
             hdc = BeginPaint(hwndAW, &ps);
             // TODO: Add any drawing code here...
             // Draw AW logo
			 BitBlt(winDC,23 ,48 ,100,100,AWDC,0,0,SRCCOPY);

			 EndPaint(hwndAW, &ps);

	    case WM_KEYDOWN:
		     switch (wParam)
			 {
		     case VK_ESCAPE:
			      EndDialog (hwndAW, TRUE); hwndAW = 0;
				  break;
		     case VK_UP:
		     case VK_DOWN:
		     case VK_LEFT:
		     case VK_RIGHT:
		     case VK_PRIOR:
		     case VK_NEXT:
		     default:
			 break;
			 }
		     break;
		break;
	}
    return FALSE;

}

BOOL CheckAll(void)
{
	PG_LogMSG("CheckAll: Start");

	PG_LogMSG("CheckAll: SysVer: MjV = %d, MiV = %d, BN = %d, PlId = %d", infoexpander->versionInfo.dwMajorVersion, infoexpander->versionInfo.dwMinorVersion, infoexpander->versionInfo.dwBuildNumber, infoexpander->versionInfo.dwPlatformId);

	DWORD i = 0;
	while(TRUE)
	{
		DISPLAY_DEVICE dd = {0};
		dd.cb = sizeof(DISPLAY_DEVICE);

		if(!EnumDisplayDevices(NULL, i, &dd, 0)) break;
		if(dd.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
		{
		   PG_LogMSG("CheckAll: DeviceName %s", dd.DeviceString);
		   std::string VideoString = (LPCSTR)dd.DeviceString;
	   	   infoexpander->VideoString = VideoString;
		}
		i++;
	}
	
	//GetS

	HKEY hk;
	std::string subkey = "Hardware\\Description\\System\\CentralProcessor\\0";
	

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
    {
/*	   unsigned char buffer[255] = {0};
       DWORD cbbuffer = 255;
       RegQueryValueEx(hk,"ProcessorNameString",0, NULL, &buffer[0], &cbbuffer);
	   PG_LogMSG("CheckAll: CPU String %s", buffer);
	   std::string CPUString = (LPCSTR)buffer;
	   CPUString.replace(0,CPUString.find_first_not_of(" "),"");
	   infoexpander->CPUString = CPUString;
	   //memset(buffer, 0, cbbuffer);
*/
       CPUInfo ci;
	   std::string CPUString = (LPCSTR)ci.GetExtendedProcessorName();
	   PG_LogMSG("CheckAll: CPU String %s", CPUString.c_str());
	   infoexpander->CPUString = CPUString;

	   CPUSpeed csp;
	   infoexpander->CPUMHZ = csp.CPUSpeedInMHz;
	   PG_LogMSG("CheckAll: CPU Frequency %d", csp.CPUSpeedInMHz);


	   MEMORYSTATUS ms;
	   GlobalMemoryStatus(&ms);
	   PG_LogMSG("CheckAll: Pysical memory %d", ms.dwTotalPhys/(1024*1024)+1);
	   infoexpander->RAMMB = ms.dwTotalPhys/(1024*1024)+1;
    }

 return TRUE;
}

BOOL CheckDX(void)
{
  HRESULT hr = 0;
  DWORD pdwDirectXVersion;
  TCHAR strDirectXVersion[255];
  int cchDirectXVersion = 255;


  hr = GetDXVersion( &pdwDirectXVersion, &strDirectXVersion[0], cchDirectXVersion);
  PG_LogMSG("CheckDX: DirectX %s installed", strDirectXVersion);
  infoexpander->DXVersion = pdwDirectXVersion;
	if(pdwDirectXVersion >= 0x00090000) return TRUE;
	else return FALSE;
}

LRESULT CALLBACK glCheckWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	PAINTSTRUCT ps;
	HDC hdc;
	
	switch (message) 
	{
		case WM_QUIT:
			 break;
		case WM_CLOSE:
             break;
		case WM_CHAR:
		     break;
        case WM_CAPTURECHANGED:
		     break;
	    case WM_LBUTTONDOWN:
		     break;
	    case WM_LBUTTONUP:
		     break;
        
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

typedef WINGDIAPI BOOL (WINAPI *glCheck_wglMakeCurrentF)(HDC hdc, HGLRC hgrlc);
glCheck_wglMakeCurrentF glCheck_wglMakeCurrent;
typedef WINGDIAPI BOOL (WINAPI *glCheck_wglDeleteContextF)(HGLRC hgrlc);
glCheck_wglDeleteContextF glCheck_wglDeleteContext;
typedef WINGDIAPI HGLRC (WINAPI *glCheck_wglCreateContextF)(HDC hdc);
glCheck_wglCreateContextF glCheck_wglCreateContext;
typedef const GLubyte *(WINAPI *glCheck_glGetStringF)(GLenum name);
glCheck_glGetStringF glCheck_glGetString;

HMODULE glCheck_Inst = NULL;

BOOL CheckGL(void)
{
	HRESULT hr;
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
			1,								// version number
			PFD_DRAW_TO_WINDOW |			// support window
			PFD_SUPPORT_OPENGL |			// support OpenGL
			PFD_DOUBLEBUFFER,				// double buffered
			PFD_TYPE_RGBA,					// RGBA type
			24,								// 24-bit color depth
			0, 0, 0, 0, 0, 0,				// color bits ignored
			0,								// no alpha buffer
			0,								// shift bit ignored
			0,								// no accumulation buffer
			0, 0, 0, 0, 					// accum bits ignored
			32,								// 32-bit z-buffer	
			0,								// no stencil buffer
			0,								// no auxiliary buffer
			PFD_MAIN_PLANE,					// main layer
			0,								// reserved
			0, 0, 0							// layer masks ignored
	};

	int  pixelFormat;

	std::string renderer;
	std::string vendor;
	std::string version;
	std::string extensions;

	PG_LogMSG("CheckGL: started");
	glCheck_Inst = LoadLibrary("opengl32.dll");

    if(!glCheck_Inst)
	{
		PG_LogERR("CheckGL:  can't load opengl32.dll");
		goto CheckGL_FALSE;
	}

    glCheck_wglMakeCurrent = (glCheck_wglMakeCurrentF)GetProcAddress(glCheck_Inst, "wglMakeCurrent");
	glCheck_wglDeleteContext = (glCheck_wglDeleteContextF)GetProcAddress(glCheck_Inst, "wglDeleteContext");
	glCheck_wglCreateContext = (glCheck_wglCreateContextF)GetProcAddress(glCheck_Inst, "wglCreateContext");
	glCheck_glGetString = (glCheck_glGetStringF)GetProcAddress(glCheck_Inst, "glGetString");
		
	PG_LogMSG("CheckGL: Creating client window...");
	hInst = GetModuleHandle(NULL);
	if(!hInst) {
		PG_LogERR("CheckGL:  can't obtain module handle");
		goto CheckGL_FALSE;
	}

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)glCheckWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)COLOR_GRAYTEXT;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "glCheck";
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	glCheckHWnd = CreateWindowEx(WS_EX_TOPMOST, "glCheck", "GLCheck", WS_POPUP|WS_VISIBLE,
      0, 0, 1, 1, NULL, NULL, hInst, NULL);
	hr = GetLastError();

    if (!glCheckHWnd)
    {
       	PG_LogERR("CheckGL:  can't create client window");
	    goto CheckGL_FALSE;
    }
	
	PG_LogMSG("CheckGL: successfull.");
		
	PG_LogMSG("CheckGL: Setting pixel format...");
	
	glCheckHDC  = GetDC(glCheckHWnd);
	rv = 0;

	if(!glCheckHDC){
		PG_LogERR("CheckGL:  GetDC failed.");
		goto CheckGL_FALSE;
	}
    pixelFormat = ChoosePixelFormat(glCheckHDC, &pfd);
	if(!pixelFormat){
		PG_LogERR("CheckGL:  ChoosePixelFormat failed.");
		goto CheckGL_FALSE;
	}
    if ( pixelFormat ) {
        if ( SetPixelFormat(glCheckHDC, pixelFormat, &pfd) ) {
            rv = glCheck_wglCreateContext( glCheckHDC );
			if(!rv) {
				PG_LogERR("CheckGL:  wglCreateContext failed.");
				goto CheckGL_FALSE;
			}
            if ( rv ) {
                if ( !glCheck_wglMakeCurrent( glCheckHDC, rv ) ) {
                    glCheck_wglDeleteContext( rv );
                    rv = 0;
					PG_LogERR("CheckGL:  wglMakeCurrent failed.");
					goto CheckGL_FALSE;
                }
            }
        }
    }
	
	PG_LogMSG("CheckGL: successfull.");

	PG_LogMSG("CheckGL: Checking hardware acceleration...");
	

	renderer   = (LPCSTR)glCheck_glGetString( GL_RENDERER );
	vendor     = (LPCSTR)glCheck_glGetString( GL_VENDOR );
	version    = (LPCSTR)glCheck_glGetString( GL_VERSION );
	extensions = (LPCSTR)glCheck_glGetString( GL_EXTENSIONS );
	
	if(renderer == "gdi generic")
	{
		PG_LogWRN("CheckGL: no hardware acceleration detected");
		goto CheckGL_FALSE;
	}
	else
	{
        PG_LogMSG("CheckGL: Renderer %s", renderer.c_str());
		PG_LogMSG("CheckGL: Vendor %s", vendor.c_str());
		PG_LogMSG("CheckGL: Version %s", version.c_str());

		DWORD start = 0, end = 0;
		end = extensions.find(" ",0);
		while(start != extensions.npos && end != extensions.npos)
		{
			std::string extension;
			LPSTR tmp = new char[end-start + 1];
			extensions.copy(tmp,end - start, start);
			*(tmp + end-start) = '\0';
            extension = tmp;
			delete tmp;
            PG_LogMSG("CheckGL: Found extension %s", extension.c_str());

			start = end + 1;
		    end = extensions.find(" ", end + 1);
		}

		PG_LogMSG("CheckGL: All successfull.\n");
		
		goto CheckGL_TRUE;
	}

CheckGL_FALSE: 
	if(glCheckHDC) ReleaseDC(glCheckHWnd, glCheckHDC);
	if(glCheck_Inst) FreeLibrary(glCheck_Inst);
  if(glCheckHWnd) DestroyWindow(glCheckHWnd);
	return FALSE; 
CheckGL_TRUE: 
	if(glCheckHDC) ReleaseDC(glCheckHWnd, glCheckHDC);
	if(glCheck_Inst) FreeLibrary(glCheck_Inst);
  if(glCheckHWnd) DestroyWindow(glCheckHWnd);
	return TRUE; 

}

void CheckHDD(void)
{
	//check HDD
	PG_Widget *HDDPlus = handler->GetWizard()->GetWidgetByName("HDDPlus");
	PG_Widget *HDDMinus = handler->GetWizard()->GetWidgetByName("HDDMinus");

	char SpaceMB[21] = {0};
	GetFreeSpaceF(SpaceMB, 21, infoexpander->MainDrive.c_str(), FORMAT_B);

	if(HDDPlus&&HDDMinus)if( infoexpander->FreeSize > infoexpander->RequiredSize)
	{
		HDDMinus->SetTransparency(255);
		HDDPlus->SetTransparency(0);
		HDDMinus->Redraw();
		HDDPlus->Redraw();
	}
	else
	{
		HDDMinus->SetTransparency(0);
		HDDPlus->SetTransparency(255);
		HDDMinus->Redraw();
		HDDPlus->Redraw();
	}
}

BOOL CALLBACK EasyInstallerDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	LONG    lRet = 1;
	switch ( uMsg ) {
		case WM_SYSCOMMAND:
			switch(wParam)
			{
			case SC_MAXIMIZE:
			//case SC_MINIMIZE:
					EasyActive = FALSE;Schon = TRUE;
	
				break;
			}
			return 0;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			
			case IDC_COMBO_PATH:
                if(HIWORD(wParam) == CBN_SELCHANGE) MyCBN_SELCHANGE = TRUE;
				break;
			case IDC_RADIO_RUSSIAN:
				{
				   PG_RadioButton *rus = NULL;
				   rus	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_russian");
				   rus->SetPressed();
				}
  			    break;
			case IDC_RADIO_ENGLISH:
				{
					PG_RadioButton *eng = NULL;
					eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");
					eng->SetPressed();
				}
				break;
			case IDC_BUTTON_START:
				handle_START(0,0,0,0);
				break;
			case IDC_BUTTON_UNINSTALL:
				handle_UNINSTALL(0,0,0,0);
				break;
			case IDC_BUTTON_INSTALL:
				handle_next_page(0,0,0,0);
				handle_INSTALL(0,0,0,0);
				break;
			case IDC_BUTTON_EXIT:
				handle_exit(0,0,0,0);
				break;
			case IDC_BUTTON_BROWSE:
				handle_BROWSE(0,0,0,0);
					break;
			} 
			break;

		case WM_SIZE:
			break;
		case WM_CLOSE:
			handle_exit(0,0,0,0);
			break;
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
		case WM_QUIT:
			handle_exit(0,0,0,0);
			break;
		/*case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		case WM_ERASEBKGND:
			if(hBrush) DeleteObject(hBrush);
			RECT rect;
			GetWindowRect(infoexpander->hwndEasyInstaller, &rect);
			OffsetRect(&rect, -rect.left, -rect.top);
			hBrush = CreateSolidBrush(RGB(205, 207, 167));
			FillRect((HDC)wParam, &rect, hBrush);
			SetBkColor((HDC)wParam, RGB(205, 207, 167));
			return (DWORD)hBrush;*/
		case WM_PAINT:
			hdc = BeginPaint(infoexpander->hwndEasyInstaller, &ps);
			//RECT crc,wrc;
			//GetClientRect(infoexpander->hwndEasyInstaller,&crc);
			//GetWindowRect(infoexpander->hwndEasyInstaller,&wrc);
			//DWORD borderw = ((wrc.right-wrc.left) - (crc.right - crc.left))/2;
			//DWORD captionw = ((wrc.bottom-wrc.top) - (crc.bottom - crc.top))/2;
			//BitBlt(FarguswinDC,borderw ,48 ,533,101,FargusDC,0,0,SRCCOPY);
			BitBlt(FarguswinDC,0 ,0 ,533,77,FargusDC,0,0,SRCCOPY);
			EndPaint(infoexpander->hwndEasyInstaller, &ps);
            break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_ESCAPE:
				handle_exit(0,0,0,0);
				break;
			case VK_UP:
			case VK_DOWN:
			case VK_LEFT:
			case VK_RIGHT:
			case VK_PRIOR:
			case VK_NEXT:
			default:
				break;
			}
			break;
	}
	return FALSE;
}

int EasyWindow(void)
{
	infoexpander->hwndEasyInstaller = CreateDialog(0, MAKEINTRESOURCE(IDD_DIALOG3), NULL, EasyInstallerDialogProc);
	if (!infoexpander->hwndEasyInstaller) {
		MessageBox( 0, "Couldn't Create Window", "Ошибка", MB_ICONERROR);
	}

	// Loading Fargus logo 
	FargusBMP   = LoadBitmap(GetModuleHandle(NULL),MAKEINTRESOURCE(IDB_BITMAP2));
	FarguswinDC   = GetDC(infoexpander->hwndEasyInstaller);
	FargusDC   = CreateCompatibleDC(FarguswinDC); 
	SelectObject(FargusDC,FargusBMP);


	//ShowWindow( infoexpander->hwndEasyInstaller, SW_SHOWDEFAULT );

	UpdateWindow(infoexpander->hwndEasyInstaller);
	SetFocus (infoexpander->hwndEasyInstaller);
	
	//	handler->GetDynamicControls()->Refresh();
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_COMBO_PATH),WM_SETTEXT,0,(LPARAM)infoexpander->MainDir.c_str());
//	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_COMBO_PATH),CB_LIMITTEXT,0,(LPARAM)260);
	
	return 0;
}

VOID CALLBACK RefreshThreadProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	handler->GetDynamicControls()->Refresh();

			
		
		PG_Label * l = NULL;
		l = (PG_Label*)handler->GetWizard()->GetWidgetByName("instL07");
		
		if(l)
		{
			if(GetFreeSpacePercent() < 5)
			{				
				SDL_Color sdlcolor;
				sdlcolor.r = 255;
				sdlcolor.g = 0;
				sdlcolor.b = 0;
				if(!schon) 
				{
					hddlow_sdlcolor = l->GetFontColor();
					schon = true;
				}
				if(l->GetFontColor().b == hddlow_sdlcolor.b && l->GetFontColor().r == hddlow_sdlcolor.r && l->GetFontColor().g == hddlow_sdlcolor.g) 
					l->SetFontColor(sdlcolor);			
				else 
					l->SetFontColor(hddlow_sdlcolor);
			}
			else
			{
				if(schon) l->SetFontColor(hddlow_sdlcolor);
			}
		}
}