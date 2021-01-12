// poweroff.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

static const char ShutdownName[] = "SeShutdownPrivilege";
static const char ErrExitWindowsEx[] = "ExitWindowsEx";
static const char MsgConfirm1[] = "Shut down system?";
static const char MsgConfirm2[] = "Conirmation";
static const char MsgUsage1[] = "Usage: poweroff.exe -p(poweroff) -s(shut down) -f(force application to close) -l(logout) -r(reboot) -h(hibernate) -c(without confirmation)";
static const char MsgUsage2[] = "MsgUsage";
static const char ErrError[] = "Error";

#define	MAX_NUM_ARGVS	128
int  argc;
char *argv[MAX_NUM_ARGVS];

void ParseCommandLine (LPSTR lpCmdLine)
{
	argc = 1;
	argv[0] = "exe";

	while (*lpCmdLine && (argc < MAX_NUM_ARGVS))
	{
		while (*lpCmdLine && ((*lpCmdLine <= 32) || (*lpCmdLine > 126)))
			lpCmdLine++;

		if (*lpCmdLine)
		{
			argv[argc] = lpCmdLine;
			argc++;

			while (*lpCmdLine && ((*lpCmdLine > 32) && (*lpCmdLine <= 126)))
				lpCmdLine++;

			if (*lpCmdLine)
			{
				*lpCmdLine = 0;
				lpCmdLine++;
			}
			
		}
	}

}

#ifdef _DEBUG
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
#else
void New_WinMain(void)
{
#endif//_DEBUG

	short int Confirm     = 0;
	short int AtleastOne  = 0;
	short int Hibernate  = 0;
	DWORD ExitFlags       = 0;
	
	ParseCommandLine (GetCommandLine());
			
	for(short int i = 1; i < argc; i++){
		if(!lstrcmp(argv[i], "-p")) {ExitFlags |= EWX_POWEROFF; AtleastOne++;}
		if(!lstrcmp(argv[i], "-s")) {ExitFlags |= EWX_SHUTDOWN; AtleastOne++;}
		if(!lstrcmp(argv[i], "-f")) {ExitFlags |= EWX_FORCE; AtleastOne++;}
		if(!lstrcmp(argv[i], "-l")) {ExitFlags |= EWX_LOGOFF; AtleastOne++;}
		if(!lstrcmp(argv[i], "-r")) {ExitFlags |= EWX_REBOOT; AtleastOne++;}
		if(!lstrcmp(argv[i], "-h")) {Hibernate = 1; AtleastOne++;}
		if(!lstrcmp(argv[i], "-c")) Confirm = 1;
	}

	if(!AtleastOne){
		MessageBox( NULL, &MsgUsage1[0], &MsgUsage2[0], MB_OK | MB_ICONINFORMATION );
		ExitProcess(-1);
	}
	
    if(!Confirm){
		if(MessageBox( NULL, &MsgConfirm1[0], &MsgConfirm2[0], MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
		ExitProcess(-1);
	}


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

	// Shut down the system

	if(Hibernate) {
		SetSystemPowerState(0,0);
		ExitProcess(0);
	}

	if (!ExitWindowsEx(ExitFlags,0)){
        MessageBox( NULL, &ErrExitWindowsEx[0], ErrError, MB_OK | MB_ICONERROR );
	}
	
	

#ifdef _DEBUG
	return 0;
#endif
}


