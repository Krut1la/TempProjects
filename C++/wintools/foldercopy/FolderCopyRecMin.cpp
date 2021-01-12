#include "stdafx.h"

#define heapNEW GetProcessHeap()
#define	MAX_NUM_ARGVS	128

//Static strings
static const char BackSlash[]       = "\\";
static const char Mask[]            = "\\*.*";
static const char ErrCreateWindow[] = "Couldn't Create Window";
static const char ExeFile[]         = "\\setupeasy ";
static const char Fargus[]          = "Fargus";
static const char Script[]          = "install.scr";
static const char Bin[]             = "bin";
static const char Mutex[]           = "FargusChildren";


HWND  hwndLoading;

HANDLE CheckForALMutex;

char TempPath[MAX_PATH];

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

//String fill
void lstrfill(LPSTR string, BYTE byte){

	for(unsigned int i = 0; i < strlen(string); i++){
		*(string + i) = byte;
	}
}

//CopyFolder
//Example:
//        ...
//        CopyFolder("C:\\Program Files\\SourceFolder", "D:\\DestinationFolder");
//        ...
//Returns:
//         0 - sucusfull
//        -1 - error
int CopyFolder(LPCSTR SourcePath, LPCSTR DestinationPath)
{
	LPWIN32_FIND_DATA lpFindFileData = new WIN32_FIND_DATA;
	HANDLE FileName;
	HRESULT hresult;
	
	//Create destination folder
	CreateDirectory(DestinationPath ,NULL);

	//Make search string C:\\Program Files\\SourceFolder\\*.*
	LPSTR FindFolder = new char[lstrlen(SourcePath) + lstrlen(Mask) + 1];
	lstrfill(FindFolder, 0);
	lstrcat(FindFolder, SourcePath);
	lstrcat(FindFolder, Mask);
	
	//find first file or folder
	FileName = FindFirstFile(FindFolder,lpFindFileData);
	if(FileName == INVALID_HANDLE_VALUE){
	   delete lpFindFileData;
	   delete FindFolder;
	   return -1;
	}//if
	
	for(;;){
		if(lstrcmp(lpFindFileData->cFileName, ".") && lstrcmp(lpFindFileData->cFileName, "..")){
			if(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
                  //Make string C:\\Program Files\\SourceFolder\\foundfolder
				  LPSTR SourceSubFolder = new char[strlen(SourcePath) +
					                               strlen(BackSlash)       +
												   strlen(lpFindFileData->cFileName) + 1];
				  lstrfill(SourceSubFolder, 0);
				  lstrcat(SourceSubFolder, SourcePath);
				  lstrcat(SourceSubFolder, BackSlash);
				  lstrcat(SourceSubFolder, lpFindFileData->cFileName);
				
				  //Make string D:\\DestinationFolder\\foundfolder
				  LPSTR DestinationSubFolder = new char[strlen(DestinationPath) +
					                               strlen(BackSlash)       +
												   strlen(lpFindFileData->cFileName) + 1];
				  lstrfill(DestinationSubFolder, 0);
				  lstrcat(DestinationSubFolder, DestinationPath);
				  lstrcat(DestinationSubFolder, BackSlash);
				  lstrcat(DestinationSubFolder, lpFindFileData->cFileName);

				  //Recurcive call
				  CopyFolder(SourceSubFolder, DestinationSubFolder);
				  delete SourceSubFolder;
				  delete DestinationSubFolder;
			}//if
			   else{
                  //Make string C:\\Program Files\\SourceFolder\\foundfile
        		  LPSTR SourceFile = new char[strlen(SourcePath) +
					                               strlen(BackSlash)       +
												   strlen(lpFindFileData->cFileName) + 1];
				  lstrfill(SourceFile, 0);
				  lstrcat(SourceFile, SourcePath);
				  lstrcat(SourceFile, BackSlash);
				  lstrcat(SourceFile, lpFindFileData->cFileName);

				  //Make string D:\\DestinationFolder\\foundfile
				  LPSTR DestinationFile = new char[strlen(DestinationPath) +
					                               strlen(BackSlash)       +
												   strlen(lpFindFileData->cFileName) + 1];
				  lstrfill(DestinationFile, 0);
				  lstrcat(DestinationFile, DestinationPath);
				  lstrcat(DestinationFile, BackSlash);
				  lstrcat(DestinationFile, lpFindFileData->cFileName);
				  //Clear all atributes if file already exist
                  SetFileAttributes(DestinationFile, FILE_ATTRIBUTE_NORMAL);
				  //Copy file
				  CopyFile(SourceFile, DestinationFile,false);
				  delete SourceFile;
				  delete DestinationFile;
			    }//else
		}//if Equal
		
		//Find next file or folder
		if(FindNextFile(FileName,lpFindFileData) == NULL){
	       hresult = GetLastError();
		   //if no mathc, then exit from search cicle
		   if(hresult == ERROR_NO_MORE_FILES) break;
		}//if 

		
	
	}//for(;;)


	//release all
	FindClose(FileName);
	delete lpFindFileData;
	delete FindFolder;
	return 0;
}


int LoadingWindow(void)
{
	//Check for first  instance
	hwndLoading = CreateDialog(0, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);
    if (!hwndLoading) {
        MessageBox( 0, ErrCreateWindow, "Error", MB_ICONERROR);
    }

    ShowWindow( hwndLoading, SW_SHOWDEFAULT );

	UpdateWindow(hwndLoading);
	SetFocus (hwndLoading);

	return 0;
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

	//Check for allready launched
	CheckForALMutex = CreateMutex(NULL, false,Mutex);
	if(GetLastError() == ERROR_ALREADY_EXISTS){
		ReleaseMutex(CheckForALMutex);
		ExitProcess(-1);
	}

	
	LoadingWindow();


	LPSTR ExePath = new char[MAX_PATH];

	GetModuleFileName(NULL,ExePath,MAX_PATH);

	//Make from path\filename.exe path\ 
	for(unsigned int i = strlen(ExePath) - 1; i >= 0 ; i--){
		if(*(ExePath + i) == '\\') { *(ExePath + i + 1) = 0; break;}
	}


	GetTempPath(MAX_PATH, TempPath);
	//Make string %TempPath%\Fargus
	LPSTR DestinationFolder = new char[strlen(TempPath) + strlen(Fargus) + 1];
	lstrfill(DestinationFolder, 0);
	lstrcat(DestinationFolder, TempPath);
	lstrcat(DestinationFolder, Fargus);

	//Make string %TempPath%\Fargus\setup.exe %ExePath%\install.scr
	LPSTR Execute = new char[strlen(TempPath)       + 
		                     strlen(Fargus)     +
							 strlen(ExeFile) +
							 strlen(ExePath)        +
							 strlen("install.scr") + 1];
	lstrfill(Execute, 0);
	lstrcat(Execute, TempPath);
	lstrcat(Execute, Fargus);
	lstrcat(Execute, ExeFile);
	lstrcat(Execute, ExePath);
	lstrcat(Execute, Script);


    //Make string %ExePath%\bin
	LPSTR BinPath = new char[strlen(ExePath) + strlen(Bin) + 1];
	lstrfill(BinPath, 0);
	lstrcat(BinPath, ExePath);
	lstrcat(BinPath, Bin);
	


	CopyFolder(BinPath, DestinationFolder);
	
		
	SetCurrentDirectory(DestinationFolder);
	WinExec(Execute,SW_SHOWNORMAL);

	EndDialog(hwndLoading, NULL);

	ReleaseMutex(CheckForALMutex);
	delete DestinationFolder;
	delete Execute;
	delete ExePath;
	delete BinPath;

#ifdef _DEBUG
	return 0;
#endif
}





