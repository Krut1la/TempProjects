#include <windows.h>

#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "aceextract.h"
#include "infoexpander.h"

#include "unace.h"

INT     UserPrompt(LPSTR PromptStr, LPSTR Param, BOOL IsRequest);

#define   FILELISTSIZE   32768  // pretty much for this this example:
                                // only the commandline can be used to
                                // specify files..
#define   COMMENTBUFSIZE 8192   // comments may be up to 32k in size
                                // increase it if you want to put that
                                // large comments to archives, or if
                                // you want to receive all of these large
                                // comments (ACE_COMMENT_SMALLBUF returned
                                // if comment does not fit into buffer)



//Progress data. For external use
extern InfoExpander *infoexpander;
extern CRITICAL_SECTION InstallerCS;
extern BOOL Successfull;

extern __int64 ArchiveUnpacked;
extern __int64 ArchiveUnpackedSize;
extern __int64 FileUnpacked;
extern __int64 FileUnpackedSize;

__int64 ForDiff = 0;

extern BOOL isUserCancel;

std::string DestPath_s;

HINSTANCE AceDllInstance;
CHAR      FileList[FILELISTSIZE];
CHAR      CommentBuf[COMMENTBUFSIZE];

INT (__stdcall *ACEInitDllProc)(pACEInitDllStruc DllData);
INT (__stdcall *ACEReadArchiveDataProc)(LPSTR ArchiveName, pACEReadArchiveDataStruc ArchiveData);
INT (__stdcall *ACEListProc)(LPSTR ArchiveName, pACEListStruc List);
INT (__stdcall *ACETestProc)(LPSTR ArchiveName, pACETestStruc Test);
INT (__stdcall *ACEExtractProc)(LPSTR ArchiveName, pACEExtractStruc Extract);


/*-----------------HandleStateStartArchive-------------------------------*/

INT     HandleStateStartArchive(pACECallbackArchiveStruc Archive)
{
LPSTR     ActionStr;

  switch (Archive->Operation)
  {
    case ACE_CALLBACK_OPERATION_LIST:
    {
      ActionStr = "Listing %s\n\n";

      break;
    }

    case ACE_CALLBACK_OPERATION_TEST:
    {
      ActionStr = "Testing %s\n\n";

      break;
    }

    case ACE_CALLBACK_OPERATION_EXTRACT:
    {
      ActionStr = "Extracting %s";
	  
      break;
    }

    default:
    {
      ActionStr = "unknown operation on %s";

      break;
    }
  }

  PG_LogMSG(ActionStr, Archive->ArchiveData->ArchiveName);

  
  return ACE_CALLBACK_RETURN_OK;
}

/*-----------------HandleStateStartFile----------------------------------*/

INT     HandleStateStartFile(pACECallbackArchivedFileStruc ArchivedFile)
{
LPSTR     ActionStr;

  switch (ArchivedFile->Operation)
  {
    case ACE_CALLBACK_OPERATION_LIST:
    {
      ActionStr = "Found";

      break;
    }

    case ACE_CALLBACK_OPERATION_TEST:
    {
      ActionStr = "Testing";

      break;
    }

    case ACE_CALLBACK_OPERATION_ANALYZE:
    {
      ActionStr = "Analyzing";

      break;
    }

    case ACE_CALLBACK_OPERATION_EXTRACT:
    {
      ActionStr = "Extracting";

      break;
    }

    default:
    {
      ActionStr = "unknown operation on";

      break;
    }
  }

  PG_LogMSG("  %s %s  (%I64d byte uncompressed, %I64d byte compressed)",
         ActionStr, ArchivedFile->FileData->SourceFileName,
         ArchivedFile->FileData->Size,
         ArchivedFile->FileData->CompressedSize);

  if(infoexpander->InstallLog)
  {
	  HRESULT hr = 0;
	  DWORD NumberOfBytesWriten = 0;
	  std::string xmlstring;
	  //XML
	  if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<file path =\"" + DestPath_s + ArchivedFile->FileData->SourceFileName + "\"\\>\r\n";
	  //UNWISE
	  if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "File Copy: " + DestPath_s + ArchivedFile->FileData->SourceFileName + "\r\n";

	  hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
	  if(!hr)
		  PG_LogERR("ExtractArchive: Can't write to %s", infoexpander->IL_Filename.c_str());    
  }

  //std::string fullpath = infoexpander->MainDir;
  //fullpath += "\\"; 
  //fullpath += ArchivedFile->FileData->SourceFileName;
  std::string fullpath = ArchivedFile->FileData->SourceFileName;
  infoexpander->SetCurrentOperation(fullpath);

  return ACE_CALLBACK_RETURN_OK;
}

/*-----------------InfoProc callback-------------------------------------*/

INT __stdcall InfoProc(pACEInfoCallbackProcStruc Info)
{
LPSTR     InfoStr;

  switch (Info->Global.Code)
  {
    case ACE_CALLBACK_INFO_FILELISTCREATE:
    {
      InfoStr = "creating file list";

      break;
    }

    case ACE_CALLBACK_INFO_FILELISTCREATEEND:
    {
      InfoStr = "finished creating file list";

      break;
    }

    case ACE_CALLBACK_INFO_FILELISTADD:
    {
      InfoStr = "adding file to file list";

      break;
    }

    default:
    {
      InfoStr = "";

      break;
    }
  }

  PG_LogMSG("Info: %s", InfoStr);

  return ACE_CALLBACK_RETURN_OK;
}

/*-----------------HandleErrorGlobal-------------------------------------*/

INT     HandleErrorGlobal(pACECallbackGlobalStruc Error)
{
LPSTR     ErrorStr;
INT       Result;

  Result = ACE_CALLBACK_RETURN_OK;

  switch (Error->Code)
  {
    case ACE_CALLBACK_ERROR_MEMORY:
    {
      ErrorStr = "not enough memory";

      break;
    }

    case ACE_CALLBACK_ERROR_UNCSPACE:
    {
      ErrorStr = "could not detect available space on network drive";

      break;
    }

    default:
    {
      ErrorStr = "unknown";
      Result = ACE_CALLBACK_RETURN_CANCEL;

      break;
    }
  }

  PG_LogERR("Error: %s", ErrorStr);

  return Result;
}

/*-----------------HandleErrorArchive------------------------------------*/

INT     HandleErrorArchive(pACECallbackArchiveStruc Error)
{
LPSTR     ErrorStr;
INT       Result;
BOOL      DoPrompt;

  Result   = ACE_CALLBACK_RETURN_OK;
  DoPrompt = 0;

  switch (Error->Code)
  {
    case ACE_CALLBACK_ERROR_AV:
    {
      ErrorStr = "AV of archive %s invalid";

      break;
    }

    case ACE_CALLBACK_ERROR_OPENARCHIVEREAD:
    {
      ErrorStr = "could not open archive %s for reading";

      break;
    }

    case ACE_CALLBACK_ERROR_READARCHIVE:
    {
      ErrorStr = "error reading from archive %s";

      break;
    }

    case ACE_CALLBACK_ERROR_ARCHIVEBROKEN:
    {
      ErrorStr = "archive %s is broken";

      break;
    }

    case ACE_CALLBACK_ERROR_NOFILES:
    {
      ErrorStr = "no files specified";

      break;
    }

    case ACE_CALLBACK_ERROR_ISNOTANARCHIVE:
    {
      ErrorStr = "file is not an ACE archive";

      break;
    }

    case ACE_CALLBACK_ERROR_HIGHERVERSION:
    {
      ErrorStr = "this Dll version is not able to handle the archive";

      break;
    }

    default:
    {
      ErrorStr = "unknown";
      Result   = ACE_CALLBACK_RETURN_CANCEL;

      break;
    }
  }

  MessageBox(infoexpander->hWndMain, ErrorStr, "ACE: ошибка", MB_OK | MB_ICONERROR );

  EnterCriticalSection(&InstallerCS);
  if(isUserCancel) CancelAceExtract();
  LeaveCriticalSection(&InstallerCS);
  if(isUserCancel) ThrowXML(RuntimeException, XMLExcepts::NoError);
  Successfull = FALSE;

  if (DoPrompt)
  {
    return UserPrompt(ErrorStr, Error->ArchiveData->ArchiveName, 0);
  }
  else
  {
    PG_LogERR("Error: ");
    PG_LogERR(ErrorStr, Error->ArchiveData->ArchiveName);
    
    return Result;
  }
}

/*-----------------HandleErrorArchivedFile-------------------------------*/

INT     HandleErrorArchivedFile(pACECallbackArchivedFileStruc Error)
{
LPSTR     ErrorStr;
INT       Result;

  Result   = ACE_CALLBACK_RETURN_OK;

  switch (Error->Code)
  {
    case ACE_CALLBACK_ERROR_CREATIONNAMEINUSE:
    {
      ErrorStr = "could not extract %s: name used by directory";

      break;
    }

    case ACE_CALLBACK_ERROR_WRITE:
    {
      ErrorStr = "error writing %s";

      break;
    }

    case ACE_CALLBACK_ERROR_OPENWRITE:
    {
      ErrorStr = "error opening %s for writing";

      break;
    }

    case ACE_CALLBACK_ERROR_METHOD:
    {
      ErrorStr = "compression method not known to this Dll version";

      break;
    }

    case ACE_CALLBACK_ERROR_EXTRACTSPACE:
    {
      ErrorStr = "not enough space to extract %s";

      break;
    }

    case ACE_CALLBACK_ERROR_CREATION:
    {
      ErrorStr = "creation of %s failed (write-protection?)";

      break;
    }

    default:
    {
      ErrorStr = "unknown";
      Result   = ACE_CALLBACK_RETURN_CANCEL;

      break;
    }
  }

  PG_LogERR("Error: ");
  PG_LogERR(ErrorStr, Error->FileData->SourceFileName);
  //PG_LogERR("\n");

  return Result;
}

/*-----------------HandleErrorRealFile-----------------------------------*/

INT     HandleErrorRealFile(pACECallbackRealFileStruc Error)
{
LPSTR     ErrorStr;
INT       Result;

  Result   = ACE_CALLBACK_RETURN_OK;

//  switch (Error->Code)
  //{
  //case:
  //  default:
    {
       ErrorStr = "unknown";
       Result   = ACE_CALLBACK_RETURN_CANCEL;
    }
  //};

  PG_LogERR("Error: ");
  PG_LogERR(ErrorStr, Error->FileName);
  //PG_LogERR("\n");

  return Result;
}

/*-----------------HandleErrorSpace--------------------------------------*/

INT     HandleErrorSpace(pACECallbackSpaceStruc Error)
{
LPSTR     ErrorStr;
INT       Result;

  ErrorStr = "";
  Result   = ACE_CALLBACK_RETURN_OK;

 // switch (Error->Code)
 // {
 //   default:
 //   {
      ErrorStr = "unknown";
      Result   = ACE_CALLBACK_RETURN_CANCEL;
 //   }
//  }

  PG_LogERR("Error: ");
  PG_LogERR(ErrorStr, Error->Directory);
  //PG_LogERR("\n");

  return Result;
}

/*-----------------HandleErrorSFXFile------------------------------------*/

INT     HandleErrorSFXFile(pACECallbackSFXFileStruc Error)
{
LPSTR     ErrorStr;
INT       Result;

  ErrorStr = "";
  Result   = ACE_CALLBACK_RETURN_OK;

 // switch (Error->Code)
 // {
  //  default:
 //   {
      ErrorStr = "unknown";
      Result   = ACE_CALLBACK_RETURN_CANCEL;
 //   }
 // }

  PG_LogERR("Error: ");
  PG_LogERR(ErrorStr, Error->SFXFileName);
  //PG_LogERR("\n");

  return Result;
}

/*-----------------HandleRequestGlobal-----------------------------------*/

INT     HandleRequestGlobal(pACECallbackGlobalStruc Request)
{
LPSTR     RequestStr;

  RequestStr = "";

 // switch (Request->Code)
//  {
 ///   default:
 //   {
      PG_LogMSG("unknown request");

      return ACE_CALLBACK_RETURN_CANCEL;
  //  }
 // }

  return ACE_CALLBACK_RETURN_CANCEL;
}

/*-----------------HandleRequestArchive----------------------------------*/

INT     HandleRequestArchive(pACECallbackArchiveStruc Request)
{
LPSTR     RequestStr;

  RequestStr = "";

  switch (Request->Code)
  {
    case ACE_CALLBACK_REQUEST_CHANGEVOLUME:
    {
      RequestStr = "ready to process next volume %s";

      break;
    }

    default:
    {
      PG_LogMSG("unknown request");

      return ACE_CALLBACK_RETURN_CANCEL;
    }
  }

  return UserPrompt(RequestStr, Request->ArchiveData->ArchiveName, 1);
}

/*-----------------HandleRequestArchivedFile-----------------------------*/

INT     HandleRequestArchivedFile(pACECallbackArchivedFileStruc Request)
{
LPSTR     RequestStr;

  RequestStr = "";

  switch (Request->Code)
  {
    case ACE_CALLBACK_REQUEST_OVERWRITE:
    {
      RequestStr = "overwrite existing file %s";

      break;
    }

    case ACE_CALLBACK_REQUEST_PASSWORD:
    {
      RequestStr = "%s is encrypted, using \x22testpassword\x22 as password";
      Request->GlobalData->DecryptPassword = "testpassword";

      break;
    }

    default:
    {
      PG_LogMSG("unknown request");

      return ACE_CALLBACK_RETURN_CANCEL;
    }
  }

  return UserPrompt(RequestStr, Request->FileData->SourceFileName, 1);
}

/*-----------------HandleRequestRealFile---------------------------------*/

INT     HandleRequestRealFile(pACECallbackRealFileStruc Request)
{
LPSTR     RequestStr;

  RequestStr = "";

 // switch (Request->Code)
 // {
  //  default:
  //  {
      PG_LogMSG("unknown request");

       return ACE_CALLBACK_RETURN_CANCEL;
  //  }
 // }

  return UserPrompt(RequestStr, Request->FileName, 1);
}

/*-----------------ErrorProc callback------------------------------------*/

INT __stdcall ErrorProc(pACEErrorCallbackProcStruc Error)
{
  switch (Error->StructureType)
  {
    case ACE_CALLBACK_TYPE_GLOBAL:
    {
      return HandleErrorGlobal(&Error->Global);
    }

    case ACE_CALLBACK_TYPE_ARCHIVE:
    {
      return HandleErrorArchive(&Error->Archive);
    }

    case ACE_CALLBACK_TYPE_ARCHIVEDFILE:
    {
      return HandleErrorArchivedFile(&Error->ArchivedFile);
    }

    case ACE_CALLBACK_TYPE_REALFILE:
    {
      return HandleErrorRealFile(&Error->RealFile);
    }

    case ACE_CALLBACK_TYPE_SPACE:
    {
      return HandleErrorSpace(&Error->Space);
    }

    case ACE_CALLBACK_TYPE_SFXFILE:
    {
      return HandleErrorSFXFile(&Error->SFXFile);
    }

    default:
    {
      return ACE_CALLBACK_RETURN_CANCEL;
    }
  }
}

/*-----------------RequestProc callback----------------------------------*/

INT __stdcall RequestProc(pACERequestCallbackProcStruc Request)
{
  switch (Request->StructureType)
  {
    case ACE_CALLBACK_TYPE_GLOBAL:
    {
      return HandleRequestGlobal(&Request->Global);
    }

    case ACE_CALLBACK_TYPE_ARCHIVE:
    {
      return HandleRequestArchive(&Request->Archive);
    }

    case ACE_CALLBACK_TYPE_ARCHIVEDFILE:
    {
      return HandleRequestArchivedFile(&Request->ArchivedFile);
    }

    case ACE_CALLBACK_TYPE_REALFILE:
    {
      return HandleRequestRealFile(&Request->RealFile);
    }

    default:
    {
      return ACE_CALLBACK_RETURN_CANCEL;
    }
  }
}

/*-----------------StateProc callback------------------------------------*/

INT __stdcall StateProc(pACEStateCallbackProcStruc State)
{
  switch (State->StructureType)
  {
    case ACE_CALLBACK_TYPE_ARCHIVE:
    {
      if (State->Archive.Code == ACE_CALLBACK_STATE_STARTARCHIVE)
      {
        return HandleStateStartArchive(&State->Archive);
      }

      break;
    }

    case ACE_CALLBACK_TYPE_ARCHIVEDFILE:
    {
      switch (State->ArchivedFile.Code)
      {
        case ACE_CALLBACK_STATE_STARTFILE:
        {
          return HandleStateStartFile(&State->ArchivedFile);
        }

        case ACE_CALLBACK_STATE_ENDNOCRCCHECK:
        {
        }
      }

      break;
    }

    case ACE_CALLBACK_TYPE_PROGRESS:
    {
      if (State->Progress.Code == ACE_CALLBACK_STATE_PROGRESS)
      {
        /*PG_LogMSG("    processed: %I64d of %I64d bytes (%I64d of %I64d bytes)\n",
               State->Progress.ProgressData->FileProcessedSize,
               State->Progress.ProgressData->FileSize,
               State->Progress.ProgressData->TotalProcessedSize,
               State->Progress.ProgressData->TotalSize);
			   */
		  FileUnpacked = State->Progress.ProgressData->FileProcessedSize;
		  ArchiveUnpacked += State->Progress.ProgressData->TotalProcessedSize - ForDiff;
		  ForDiff = State->Progress.ProgressData->TotalProcessedSize;
		  FileUnpackedSize = State->Progress.ProgressData->FileSize;
	      //ArchiveUnpackedSize = State->Progress.ProgressData->TotalSize;
		  infoexpander->SetFileProgress((int)(((double)FileUnpacked/((double)FileUnpackedSize + .0001f))*100));
		  infoexpander->SetArchiveProgress((int)(((double)ArchiveUnpacked/((double)ArchiveUnpackedSize + .0001f))*100));

		  EnterCriticalSection(&InstallerCS);
		  if(isUserCancel) CancelAceExtract();
		  LeaveCriticalSection(&InstallerCS);
		  if(isUserCancel) ThrowXML(RuntimeException, XMLExcepts::NoError); //_endthreadex(-1);
      }

      break;
    }

    case ACE_CALLBACK_TYPE_CRCCHECK:
    {
      if (State->CRCCheck.Code == ACE_CALLBACK_STATE_ENDCRCCHECK)
      {
        PG_LogMSG("   %s", State->CRCCheck.CRCOk ? "CRC OK" : "CRC-check error");
		if(!State->CRCCheck.CRCOk) 
		   MessageBox(infoexpander->hWndMain, "CRC ошибка.", "ACE: ошибка", MB_OK | MB_ICONERROR );
      }
      break;
    }
  }

  return ACE_CALLBACK_RETURN_OK;
}

/*-----------------UserPrompt--------------------------------------------*/

INT     UserPrompt(LPSTR PromptStr, LPSTR Param, BOOL IsRequest)
{
CHAR Ch;

  //PG_LogMSG(IsRequest ? "Request: " : "Error: ");
  //PG_LogMSG(PromptStr, Param);
  //PG_LogMSG(IsRequest ? "? (Yes, No, Cancel)" : "! (Ok, Cancel)");
  //fflush(stdout);

  do
  {
    //Ch = getchar();
	  Ch = 0x79;

  } while (!strchr(IsRequest ? "yYnNcC" : "oOcC", Ch));

  PG_LogMSG("\n");

  switch (Ch)
  {
    case 'o':
    case 'O':
    case 'y':
    case 'Y':
    {
      return ACE_CALLBACK_RETURN_OK;
    }

    case 'n':
    case 'N':
    {
      return ACE_CALLBACK_RETURN_NO;
    }
  }

  return ACE_CALLBACK_RETURN_CANCEL;
}

/*-----------------CallAceInitDll----------------------------------------*/

void    CallAceInitDll(void)
{
tACEInitDllStruc
          DllData;

  memset(&DllData, 0, sizeof(DllData));   // set all fields to zero

  DllData.GlobalData.MaxArchiveTestBytes = 0x1ffFF; // search for archive
                                                    // header in first 128k
                                                    // of file
  DllData.GlobalData.MaxFileBufSize      = 0x2ffFF; // read/write buffer size
                                                    // is 256k

  DllData.GlobalData.Comment.BufSize     = sizeof(CommentBuf);
  DllData.GlobalData.Comment.Buf         = CommentBuf; // set comment bufffer
                                                       // to receive comments
                                                       // of archive and/or
                                                       // set comments

  DllData.GlobalData.TempDir             = "C:\\TEMP"; // set temp dir

  // set callback function pointers
  DllData.GlobalData.InfoCallbackProc    = InfoProc;
  DllData.GlobalData.ErrorCallbackProc   = ErrorProc;
  DllData.GlobalData.RequestCallbackProc = RequestProc;
  DllData.GlobalData.StateCallbackProc   = StateProc;

  ACEInitDllProc(&DllData);
}

/*-----------------FileExists--------------------------------------------*/

BOOL    FileExists(LPSTR FileName)
{
struct stat Info;

  return !stat(FileName, &Info);
}

/*-----------------CallACEList-------------------------------------------*/

INT     CallACEList(LPSTR ArchiveName)
{
tACEListStruc
          List;

  memset(&List, 0, sizeof(List));     // set all fields to zero

  List.Files.SourceDir   = "";        // archive main directory is
                                      // base directory for FileList
  List.Files.FileList    = FileList;  // set FileList
  List.Files.ExcludeList = "";        // no files to exclude
  List.Files.FullMatch   = 0;         // also list files partially matching
                                      // (for instance: list DIR1\TEST.DAT
                                      //  if FileList specifies TEST.DAT)

  return ACEListProc(ArchiveName, &List);
}

/*-----------------CallACETest-------------------------------------------*/

INT     CallACETest(LPSTR ArchiveName)
{
tACETestStruc
          Test;

  memset(&Test, 0, sizeof(Test));     // set all fields to zero

  Test.Files.SourceDir   = "";        // archive main directory is
                                      // base directory for FileList
  Test.Files.FileList= FileList;      // set FileList
  Test.Files.ExcludeList = "";        // no files to exclude
  Test.Files.FullMatch   = 0;         // also test files partially matching

  Test.DecryptPassword   = "";        // no encrypted file expected

  return ACETestProc(ArchiveName, &Test);
}

/*-----------------CallACEExtract----------------------------------------*/

INT     CallACEExtract(LPSTR ArchiveName, LPSTR DestinationDir, BOOL DoExcludePath)
{
tACEExtractStruc
          Extract;

  memset(&Extract, 0, sizeof(Extract));          // set all fields to zero

  Extract.Files.SourceDir      = "";             // archive main directory is
                                                 // base dir for FileList
  Extract.Files.FileList       = FileList;       // set FileList
  Extract.Files.ExcludeList    = "";             // no files to exclude
  Extract.Files.FullMatch      = 0;              // also extract files
                                                 // partially matching

  Extract.DestinationDir       = DestinationDir; // directory to extract to
  Extract.ExcludePath          = DoExcludePath;  // extract files with path?
  Extract.DecryptPassword      = "";             // no encrypted file expected

  
  return ACEExtractProc(ArchiveName, &Extract);
}

/*-----------------LoadAceDll--------------------------------------------*/

BOOL    LoadAceDll(void)
{
  AceDllInstance = LoadLibrary("UnAceV2.Dll");

  if (AceDllInstance)
  {
    if (!(ACEInitDllProc               = (INT (__stdcall *)(pACEInitDllStruc DllData))
              GetProcAddress(AceDllInstance, "ACEInitDll"))
        || !(ACEReadArchiveDataProc    = (INT (__stdcall *)(LPSTR ArchiveName, pACEReadArchiveDataStruc ArchiveData))
              GetProcAddress(AceDllInstance, "ACEReadArchiveData"))
        || !(ACEListProc               = (INT (__stdcall *)(LPSTR ArchiveName, pACEListStruc List))
              GetProcAddress(AceDllInstance, "ACEList"))
        || !(ACETestProc               = (INT (__stdcall *)(LPSTR ArchiveName, pACETestStruc Test))
              GetProcAddress(AceDllInstance, "ACETest"))
        || !(ACEExtractProc            =  (INT (__stdcall *)(LPSTR ArchiveName, pACEExtractStruc Extract))
              GetProcAddress(AceDllInstance, "ACEExtract")))
    {
      PG_LogERR("Could not find function in UnAceV2.Dll!");
      FreeLibrary(AceDllInstance);

      return FALSE;
    }
  }

  return AceDllInstance != NULL;
}

/*-----------------FreeAceDll--------------------------------------------*/

void    FreeLoadAceDll(void)
{
  FreeLibrary(AceDllInstance);
}

/*-----------------PrintHelp---------------------------------------------*/

void    PrintHelp(void)
{
  PG_LogMSG("\nUsage: Example <command> <archive> [<files>] [destination-dir]\n\n"
         "  commands:\n"
         "    e    extract to current directory\n"
         "    l    list\n"
         "    t    test\n"
         "    x    extract with full path\n\n");
}

/*-----------------main--------------------------------------------------*/

void UserAceCancel(void)
{
	EnterCriticalSection(&InstallerCS);
	isUserCancel = TRUE;
	LeaveCriticalSection(&InstallerCS);
}

void CancelAceExtract(void)
{
	//if(hArcData) 
	{
		FreeLoadAceDll();
		infoexpander->SetArchiveProgress(0);
		infoexpander->SetFileProgress(0);
		ArchiveUnpacked = 0;
		ArchiveUnpackedSize = 0;
		FileUnpacked = 0;
		FileUnpackedSize = 0;
	}
}

//int ExtractAceArchive(LPSTR ArcName,int Mode, PTSTR pszCabFile);
int     ExtractAceArchive(INT argc, LPSTR argv[])
{
tACEReadArchiveDataStruc
          ArchiveData;
LPSTR     ArchiveName,
          DestinationDir;
CHAR      Command;
INT       Result;
BOOL      Exists;


ArchiveUnpackedSize = infoexpander->RequiredSize;
ForDiff = 0;

isUserCancel = false;

Command = tolower(argv[1][0]);

  if (!Command ||!strchr("eltx", Command))
  {
    PrintHelp();

    return 1;
  }

  if (!LoadAceDll())
  {
    PG_LogERR("Could not find UnAceV2.Dll (must be in system, applicaton's or current directory)!");

    return 1;
  }

  CallAceInitDll();

  ArchiveName = argc > 2 ? argv[2] : "";

  FileList[0]    = 0;
  DestinationDir = "";

  while (argc > 3)
  {
    argc--;

    if (argv[argc][strlen(argv[argc]) - 1] == '\\')
    {
      DestPath_s = DestinationDir = argv[argc];
	  
    }
    else
    {
      strcat(FileList, argv[argc]);
      strcat(FileList, "\xd");
    }
  }

  if (!FileList[0])
  {
    strcpy(FileList, "*");
  }

  Exists = FileExists(ArchiveName);

  if (Exists)
  {
    ACEReadArchiveDataProc(ArchiveName, &ArchiveData);

    if (!ArchiveData.ArchiveData)
    {
      PG_LogERR("\nIs no ACE-archive: %s\n", ArchiveName);

      return 0;
    }
  }
  else
  {
    PG_LogERR("\nACE-archive does not exist: %s\n", ArchiveName);

    return ACE_ERROR_OPEN;
  }

  switch (Command)
  {
    case 'e':
    {
      Result = CallACEExtract(ArchiveName, DestinationDir, 1);

      break;
    }

    case 'l':
    {
      Result = CallACEList(ArchiveName);

      break;
    }

    case 't':
    {
      Result = CallACETest(ArchiveName);

      break;
    }

    case 'x':
    {
      Result = CallACEExtract(ArchiveName, DestinationDir, 0);

      break;
    }
  }

  FreeLoadAceDll();

  infoexpander->SetFileProgress(100);
  //ArchiveUnpacked = 0;
  ArchiveUnpackedSize = 0;
  FileUnpacked = 0;
  FileUnpackedSize = 0;

  PG_LogMSG("ActiveACE operation return code: %d", Result);

  return Result;
}
