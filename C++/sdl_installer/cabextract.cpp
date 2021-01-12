#include "cabextract.h"
#include "infoexpander.h"

#define AUTOCALCSIZE 

// Link with SetupAPI.Lib.
#pragma comment (lib, "setupapi.lib")

#define CAB_FILE_NAME      TEXT("TestCab.Cab")

//Progress data. For external use
extern InfoExpander *infoexpander;
extern CRITICAL_SECTION InstallerCS;
extern BOOL Successfull;

extern __int64 ArchiveUnpacked;
extern __int64 ArchiveUnpackedSize;
extern __int64 FileUnpacked;
extern __int64 FileUnpackedSize;

// Global variable holding destination directory.
TCHAR g_szTargetPath[MAX_PATH];

//   void IterateCabinet(PTSTR pszCabFile);


extern BOOL isUserCancel;

int ExtractCabArchive(LPSTR ArcName,int Mode, LPSTR DestPath)
{

	LPVOID lpMsgBuf;

	lstrcpy(g_szTargetPath, DestPath);
	isUserCancel = false;


	if (  !SetupIterateCabinet(ArcName,	0, (PSP_FILE_CALLBACK)CabinetCallback, 0) )
	{
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
			GetLastError(), MAKELANGID(LANG_NEUTRAL,
			SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );

		MessageBox( NULL,(LPTSTR) lpMsgBuf,
			"CAB: ошибка",
			MB_OK | MB_ICONERROR);
		//exit(0);
	}

	return 0;
}

void UserCabCancel(void)
{
EnterCriticalSection(&InstallerCS);
isUserCancel = TRUE;
LeaveCriticalSection(&InstallerCS);
}

void CancelCabExtract(void)
{
//if(hArcData) 
{
//RARCloseArchive(hArcData);
infoexpander->SetArchiveProgress(0);
infoexpander->SetFileProgress(0);
ArchiveUnpacked = 0;
ArchiveUnpackedSize = 0;
FileUnpacked = 0;
FileUnpackedSize = 0;
//hArcData = NULL;
}
}

/*
int ListArchive(char *ArcName)
{
HANDLE hArcData;
int RHCode,PFCode;
char CmtBuf[16384];
struct RARHeaderDataEx HeaderData;
struct RAROpenArchiveDataEx OpenArchiveData;

memset(&OpenArchiveData,0,sizeof(OpenArchiveData));
OpenArchiveData.ArcName=ArcName;
OpenArchiveData.CmtBuf=CmtBuf;
OpenArchiveData.CmtBufSize=sizeof(CmtBuf);
OpenArchiveData.OpenMode=RAR_OM_LIST;
hArcData=RAROpenArchiveEx(&OpenArchiveData);

if (OpenArchiveData.OpenResult!=0)
{
OutOpenArchiveError(OpenArchiveData.OpenResult,ArcName);
return OpenArchiveData.OpenResult;
}

ShowArcInfo(OpenArchiveData.Flags,ArcName);

if (OpenArchiveData.CmtState==1)
ShowComment(CmtBuf);

RARSetCallback(hArcData,CallbackProc,0);

HeaderData.CmtBuf=CmtBuf;
HeaderData.CmtBufSize=sizeof(CmtBuf);

PG_LogMSG("File                       Size");
PG_LogMSG("-------------------------------");
while ((RHCode=RARReadHeaderEx(hArcData,&HeaderData))==0)
{
__int64 UnpSize=HeaderData.UnpSize+(((__int64)HeaderData.UnpSizeHigh)<<32);
PG_LogMSG("%-20s %10Ld ",HeaderData.FileName,UnpSize);
if (HeaderData.CmtState==1)
ShowComment(CmtBuf);
if ((PFCode=RARProcessFile(hArcData,RAR_SKIP,NULL,NULL))!=0)
{
OutProcessFileError(PFCode);
break;
}
}

if (RHCode==ERAR_BAD_DATA)
PG_LogERR("File header broken");

RARCloseArchive(hArcData);
return 0;
}


void ShowComment(char *CmtBuf)
{
PG_LogMSG("Comment:\n%s\n",CmtBuf);
}


void OutHelp(void)
{
PG_LogMSG("UNRDLL.   This is a simple example of UNRAR.DLL usage\n");
PG_LogMSG("Syntax:\n");
PG_LogMSG("UNRDLL X <Archive>     extract archive contents");
PG_LogMSG("UNRDLL T <Archive>     test archive contents");
PG_LogMSG("UNRDLL P <Archive>     print archive contents to stdout");
PG_LogMSG("UNRDLL L <Archive>     view archive contents\n");
}


void OutOpenArchiveError(int Error,char *ArcName)
{
switch(Error)
{
case ERAR_NO_MEMORY:
PG_LogERR("Not enough memory");
break;
case ERAR_EOPEN:
PG_LogERR("Cannot open %s",ArcName);
break;
case ERAR_BAD_ARCHIVE:
PG_LogERR("%s is not RAR archive",ArcName);
break;
case ERAR_BAD_DATA:
PG_LogERR("%s: archive header broken",ArcName);
break;
case ERAR_UNKNOWN:
PG_LogERR("Unknown error");
break;
}
}


void ShowArcInfo(unsigned int Flags,char *ArcName)
{
PG_LogMSG("Archive %s\n",ArcName);
PG_LogMSG("Volume:\t\t%s",(Flags & 1) ? "yes":"no");
PG_LogMSG("Comment:\t%s",(Flags & 2) ? "yes":"no");
PG_LogMSG("Locked:\t\t%s",(Flags & 4) ? "yes":"no");
PG_LogMSG("Solid:\t\t%s",(Flags & 8) ? "yes":"no");
PG_LogMSG("New naming:\t%s",(Flags & 16) ? "yes":"no");
PG_LogMSG("Authenticity:\t%s",(Flags & 32) ? "yes":"no");
PG_LogMSG("Recovery:\t%s",(Flags & 64) ? "yes":"no");
PG_LogMSG("Encr.headers:\t%s",(Flags & 128) ? "yes":"no");
PG_LogMSG("First volume:\t%s",(Flags & 256) ? "yes":"no or older than 3.0");
PG_LogMSG("---------------------------\n");
}


void OutProcessFileError(int Error)
{
switch(Error)
{
case ERAR_UNKNOWN_FORMAT:
PG_LogERR("Unknown archive format");
MessageBox(infoexpander->hWndMain,"Неверный формат архива", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_BAD_ARCHIVE:
PG_LogERR("Bad volume");
MessageBox(infoexpander->hWndMain, "Неверный формат архива.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_ECREATE:
PG_LogERR("File create error");
MessageBox(infoexpander->hWndMain, "Невозможно создать файл.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_EOPEN:
PG_LogERR("Volume open error");
MessageBox(infoexpander->hWndMain, "Неверный том.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_ECLOSE:
PG_LogERR("File close error");
MessageBox(infoexpander->hWndMain, "Невозможно закрыть файл.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_EREAD:
PG_LogERR("Read error");
MessageBox(infoexpander->hWndMain, "Новозможно чтение.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_EWRITE:
PG_LogERR("Write error");
MessageBox(infoexpander->hWndMain, "Невозможна запись. Возможно, нет места на диске.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_BAD_DATA:
PG_LogERR("CRC error");
MessageBox(infoexpander->hWndMain, "Нет данных. Возможно, диск поврежден.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
case ERAR_UNKNOWN:
PG_LogERR("Unknown error");
MessageBox(infoexpander->hWndMain, "Неизвестная ошибка.", "RAR: ошибка", MB_OK | MB_ICONERROR );
break;
}
Successfull = FALSE;
}
*/

LRESULT WINAPI CabinetCallback ( IN PVOID pMyInstallData,
								IN UINT Notification,
								IN UINT Param1,
								IN UINT Param2 )
{
	LRESULT lRetVal = NO_ERROR;
	TCHAR szTarget[MAX_PATH];
	FILE_IN_CABINET_INFO *pInfo = NULL;
	FILEPATHS *pFilePaths = NULL;

	lstrcpy(szTarget,g_szTargetPath);

	switch(Notification)
	{
	case SPFILENOTIFY_FILEINCABINET:
		pInfo = (FILE_IN_CABINET_INFO *)Param1;
		lstrcat(szTarget, pInfo->NameInCabinet);
		lstrcpy(pInfo->FullTargetName, szTarget);
		lRetVal = FILEOP_DOIT;  // Extract the file.
		ArchiveUnpacked += pInfo->FileSize;

		infoexpander->SetFileProgress((int)(((double)FileUnpacked/((double)FileUnpackedSize + .0001f))*100));
		infoexpander->SetArchiveProgress((int)(((double)ArchiveUnpacked/((double)ArchiveUnpackedSize + .0001f))*100));
      /*
		EnterCriticalSection(&InstallerCS);
		if(isUserCancel) CancelCabExtract();
		LeaveCriticalSection(&InstallerCS);
		if(isUserCancel) ThrowXML(RuntimeException, XMLExcepts::NoError); //_endthreadex(-1);
          return(0);*/
		break;

	case SPFILENOTIFY_FILEEXTRACTED:
		pFilePaths = (FILEPATHS *)Param1;
		PG_LogMSG("Extracting %-45s",pFilePaths->Target);
		infoexpander->SetCurrentOperation(pFilePaths->Target);
		lRetVal = NO_ERROR;
		break;

	case SPFILENOTIFY_NEEDNEWCABINET: // Unexpected.
		lRetVal = NO_ERROR;
		break;
	}

	return lRetVal;
}
