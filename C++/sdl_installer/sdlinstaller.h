#ifndef MAIN_H
#define MAIN_H

// disable long class name warnings for STL
#pragma warning(disable : 4786)
#pragma warning(disable : 4788)

#ifndef _DEBUG 
 #pragma comment(linker,"/MERGE:.rdata=.text")
 #pragma comment(linker,"/FILEALIGN:512 /SECTION:.text,EWRX /IGNORE:4078")
 //#pragma comment(linker,"/ENTRY:New_WinMain")
 //#pragma comment(linker,"/NODEFAULTLIB")
#endif

#include <windows.h>
#include <winuser.h>

//#include <iostream>
//#include <string>
//static const basic_string <char>::size_type npos = -1;



//#include "win32.h"
#include <shlobj.h>
#include <objbase.h>
#include <Psapi.h>
#include <gl\gl.h>
#include <process.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include <pglog.h>
//#include <SDL_mixer.h>
#include <paragui.h>
#include <pgapplication.h>
#include <pgbutton.h>
#include <pgwidget.h>
#include <pgcheckbutton.h>
#include <pglabel.h>
//#include <pgstaticframe.h>
#include <pgcompat.h>
#include <pgwidgetlist.h>
#include <pglineedit.h>
#include <pgprogressbar.h>
//#include <rendertextrect.h>

//#include <zstr.h>

//#include "Main.h"
#include "resource.h"
//#include "BrowseForFolder.h"
#include "getdxver.h"
#include "layout.h"
#include "installer.h"
#include "infoexpander.h"
#include "rarextract.h"
#include "cpu_info.h"

#define BUTTON_AWLOGO       0x999 
#define BUTTON_QUIT	    	0x1
#define BUTTON_INSTALL  	0x2
#define BUTTON_NEXT	    	0x3
#define BUTTON_PREV	    	0x4
#define SCREENSHOT	    	0x5
#define CONSOLE	    	    0x6
#define CONSOLE2    	    0x7
#define CONSOLE3    	    0x71
#define CONSOLETEXT1   	    0x8
#define CONSOLETEXT2   	    0x9
#define CONSOLETEXT3   	    0x10
#define CONSOLETEXT4   	    0x11
#define BUTTON_FULLSCREEN   0x12

#define BUTTON_UNINSTALL	0x101
#define BUTTON_START    	0x102
#define BUTTON_WWW      	0x103

#define BUTTON_AGREE    	0x201

#define BUTTON_INSTALLM  	0x307
#define BUTTON_PREVM    	0x308
#define BUTTON_BROWSE    	0x301
#define PATHTEXTBOX         0x302 
#define LISTBOX_FOLDER      0x303
#define COMBOBOX_FOLDER     0x304
#define BROWSEBOX           0x305
#define BUTTON_OK       	0x306
#define BUTTON_QUICKT       0x309
#define BUTTON_DIRECTX      0x310

#define BUTTON_READY    	0x401
#define BUTTON_CANCEL    	0x402
#define BUTTON_CANCEL2    	0x403
#define BUTTON_YES       	0x404
#define BUTTON_NO       	0x405
#define PBAR            	0x406
#define CONFIRM_BOX        	0x407
#define TEXTBOX_CURRENTFILE 0x408

#define BUTTON_MULTIINTRO	0x0E
#define BUTTON_M	        0x10E
#define BUTTON_INTRO	0x0F
#define BUTTON_ABOUT	0x10
#define BUTTON_FEATURES	0x11
#define BUTTON_GLJ		0x12
#define BUTTON_EDIT		0x13

#define BUTTON_SLIDER	0x15

#define BUTTON_TEXTFIELD	0x2A


typedef std::list<std::string> COMBOPATHLIST;

BOOL my_eventMouseButtonDown(const SDL_MouseButtonEvent *button);

VOID CALLBACK RefreshThreadProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
void InitFail(const char *message);
BOOL InitSDL(void);
BOOL InitParaGuiObject(void);
BOOL InitGL(void);
void SysQuit(void);
void SysRestart(void);
void LoadInstallerSettings(void);
void SaveInstallerSettings(void);
void GetPaths(char* argv0);
BOOL InitAudio(void);
void DeleteAll(void);
void CalculateLayout(void);
BOOL ShowMisc(void);
BOOL HideAll(void);
BOOL HideAllScreenShot(void);
BOOL ShowWellcomeFrame(void);
BOOL HideWellcomeFrame(void);
BOOL ShowLicenseFrame(void);
BOOL HideLicenseFrame(void);
BOOL ShowMainFrame(void);
BOOL HideMainFrame(void);
BOOL ShowFinalFrame(void);
BOOL HideFinalFrame(void);
void SetButtonState(int id, BOOL state, BOOL show);
int LoadingWindow(void);
int AWWindow(void);
BOOL CheckDX(void);
BOOL CheckGL(void);
BOOL CheckAll(void);
void CheckHDD(void);
int EasyWindow(void);
BOOL CALLBACK EasyInstallerDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif MAIN_H

