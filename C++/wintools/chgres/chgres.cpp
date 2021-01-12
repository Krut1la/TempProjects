// Min.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#define heapNEW GetProcessHeap()

static const char ErrChangeDisplaySettingsEx[] = "ChangeDisplaySettingsEx";
static const char MsgConfirm1[] = "Change resolution?";
static const char MsgConfirm2[] = "Confirmation";
static const char MsgUsage1[] = "Usage: chgres.exe -t(test) -b(best: -m640 -m800 -m1024) -C(custom: width height bpp freq) -c(without confirmation)";
static const char MsgUsage2[] = "MsgUsage";
static const char ErrError[] = "Error";
//static char ErrError1[] = "1111111111111Error";

#define	MAX_NUM_ARGVS	128
int  argc;
char *argv[MAX_NUM_ARGVS];

//If debuging we are using CRT
#ifndef _DEBUG 
void * __cdecl malloc(size_t size)
{
  return HeapAlloc(heapNEW, HEAP_ZERO_MEMORY, size);
}

inline void * __cdecl operator new(size_t size)
{
  return malloc(size);
}

void __cdecl free(void *block)
{
  HeapFree(heapNEW, 0, block);
}

inline void __cdecl operator delete(void *p)
{
  free(p);
}
#endif //_DEBUG 

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

	short int i = 0;
	short int Confirm     = 0;
	short int AtleastOne  = 0;
	short int Best   = 0;
	short int Custom = 0;
	short int Test   = 0;
	short int Width  = 1024;
	short int Height = 768;
	short int BPP    = 32;
	short int Freq   = 100;
	DWORD dwFlags    = 0;
		
	ParseCommandLine (GetCommandLine());

	for(i = 1; i < argc; i++){
		if(!lstrcmp(argv[i], "-b")) {Best = 1; AtleastOne = 1;}
		if(Best){
			if(!lstrcmp(argv[i], "-m640")){
				Width  =  640;
				Height =  480;
				BPP    =   32;
			}
			if(!lstrcmp(argv[i], "-m800")){
				Width  =  800;
				Height =  600;
				BPP    =   32;
			}
			if(!lstrcmp(argv[i], "-m1024")){
				Width  = 1024;
				Height =  768;
				BPP    =   32;
			}
		}
		if(!lstrcmp(argv[i], "-C")) {Custom = 1; AtleastOne = 1;}
		if(Custom && argc >= i + 4){
			Width  = StrToInt(argv[i + 1]);
			Height = StrToInt(argv[i + 2]);
			BPP    = StrToInt(argv[i + 3]);
			Freq   = StrToInt(argv[i + 4]);
			Custom = 0;
			i+=4;
		}
		if(!lstrcmp(argv[i], "-t")) {Test |= CDS_TEST; AtleastOne = 1;}
		if(!lstrcmp(argv[i], "-c")) {Confirm = 1; AtleastOne = 1;}
	}


	if(!AtleastOne){
		MessageBox( NULL, &MsgUsage1[0], &MsgUsage2[0], MB_OK | MB_ICONINFORMATION );
		ExitProcess(-1);
	}
	
    if(!Confirm){
		if(MessageBox( NULL, &MsgConfirm1[0], &MsgConfirm2[0], MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
		ExitProcess(-1);
	}

	LPDEVMODE lpDevMode = new DEVMODE;
	lpDevMode->dmSize = sizeof(DEVMODE);
	lpDevMode->dmDriverExtra      =   1;
	lpDevMode->dmBitsPerPel       =  BPP;
	lpDevMode->dmPelsWidth        = Width;
    lpDevMode->dmPelsHeight       = Height; 
	
	if(!Best) {
	   lpDevMode->dmDisplayFrequency = Freq;
	   lpDevMode->dmFields  = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	}
	else{
		lpDevMode->dmFields  = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	}
 	

	HRESULT hr;
	hr = ChangeDisplaySettingsEx(NULL,lpDevMode,NULL,dwFlags,NULL);
		switch(hr){

		       case DISP_CHANGE_SUCCESSFUL:
				    break;
			   case DISP_CHANGE_BADMODE:
				    MessageBox( NULL, "Bad mode", ErrError, MB_OK | MB_ICONERROR );
					break;
			   default: 
				    MessageBox( NULL, &ErrChangeDisplaySettingsEx[0], ErrError, MB_OK | MB_ICONERROR );
					break;
				    
		}
    
	delete lpDevMode;

#ifdef _DEBUG
	return 0;
#endif
}


