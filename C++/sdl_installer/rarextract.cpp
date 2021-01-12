#include "rarextract.h"
#include "infoexpander.h"

#define AUTOCALCSIZE 

//Progress data. For external use
extern InfoExpander *infoexpander;
extern CRITICAL_SECTION InstallerCS;
extern BOOL Successfull;

__int64 ArchiveUnpacked = 0;
__int64 ArchiveUnpackedSize = 0;
__int64 FileUnpacked = 0;
__int64 FileUnpackedSize = 0;


//Layout
extern SDLInstallerLayout *handler;

struct RAROpenArchiveDataEx OpenArchiveData;
HANDLE hArcData = NULL;

BOOL isUserCancel = FALSE;

int ExtractArchive(LPSTR ArcName,int Mode, LPSTR DestPath)
{

	std::string DestPath_s = DestPath;

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

  isUserCancel = false;

  if (OpenArchiveData.OpenResult!=0)
  {
    OutOpenArchiveError(OpenArchiveData.OpenResult,ArcName);
    return OpenArchiveData.OpenResult;
  }

  ShowArcInfo(OpenArchiveData.Flags,ArcName);

  if (OpenArchiveData.CmtState==1)
    ShowComment(CmtBuf);

  //RARSetCallback(hArcData,CallbackProc,(LONG)&Mode);

  HeaderData.CmtBuf=NULL;
  

  //calculate unpacked size
  /*while ((RHCode=RARReadHeaderEx(hArcData,&HeaderData))==0)
  {
	  //ArchiveUnpackedSize+=HeaderData.UnpSize;
	  ArchiveUnpackedSize += HeaderData.UnpSize+(((__int64)HeaderData.UnpSizeHigh)<<32);
	  PFCode=RARProcessFile(hArcData,RAR_SKIP,NULL,NULL);
  }
*/
  ArchiveUnpackedSize = infoexpander->RequiredSize;

  RARCloseArchive(hArcData);
  OpenArchiveData.OpenMode=RAR_OM_EXTRACT;
  hArcData=RAROpenArchiveEx(&OpenArchiveData);
  RARSetCallback(hArcData,CallbackProc,(LONG)&Mode);
  HeaderData.CmtBuf=NULL;

  while ((RHCode=RARReadHeaderEx(hArcData,&HeaderData))==0)
  {
    switch(Mode)
    {
      case EXTRACT:
        PG_LogMSG("Extracting %-45s",HeaderData.FileName);
		
        if(infoexpander->InstallLog)
        {
          HRESULT hr = 0;
          DWORD NumberOfBytesWriten = 0;
          //std::string xmlstring = "<file path =\"" + infoexpander->MainDir + "\\"+ HeaderData.FileName + "\"\\>\n";

		  std::string xmlstring;
          //XML
	      if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<file path =\"" + DestPath_s + "\\"+ HeaderData.FileName + "\"\\>\r\n";
	      //UNWISE
	      if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "File Copy: " + DestPath_s + "\\"+ HeaderData.FileName + "\r\n";

          hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
          if(!hr)
            PG_LogERR("ExtractArchive: Can't write to %s", infoexpander->IL_Filename.c_str());    
        }
        break;
      case TEST:
        PG_LogMSG("Testing %-45s",HeaderData.FileName);
        break;
      case PRINT:
        PG_LogMSG("Printing %-45s",HeaderData.FileName);
        break;
    }

	FileUnpackedSize = HeaderData.UnpSize;

	//std::string fullpath = infoexpander->MainDir;
    //fullpath += "\\"; 
	//fullpath += HeaderData.FileName;
    std::string fullpath = HeaderData.FileName; 
	infoexpander->SetCurrentOperation(fullpath);

	    PFCode=RARProcessFile(hArcData,(Mode==EXTRACT) ? RAR_EXTRACT:RAR_TEST, DestPath,NULL);
    
	FileUnpacked = 0;
    if (PFCode==0)
	{
		//infoexpander->SetFileProgress(100);
      //PG_LogMSG("Ok");
	}
    else
    {
      OutProcessFileError(PFCode);           
      break;
    }
  }

  if (RHCode==ERAR_BAD_DATA)
    PG_LogERR("File header broken");

  RARCloseArchive(hArcData);
  //infoexpander->SetArchiveProgress(100);
  infoexpander->SetFileProgress(100);
  //ArchiveUnpacked = 0;
  ArchiveUnpackedSize = 0;
  FileUnpacked = 0;
  FileUnpackedSize = 0;
  return 0;
}

void UserCancel(void)
{
	EnterCriticalSection(&InstallerCS);
	isUserCancel = TRUE;
	LeaveCriticalSection(&InstallerCS);
}

void CancelExtract(void)
{
	//if(hArcData) 
	{
		RARCloseArchive(hArcData);
		infoexpander->SetArchiveProgress(0);
		infoexpander->SetFileProgress(0);
		ArchiveUnpacked = 0;
		ArchiveUnpackedSize = 0;
		FileUnpacked = 0;
		FileUnpackedSize = 0;
		hArcData = NULL;
	}
}


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
  EnterCriticalSection(&InstallerCS);
  if(isUserCancel) CancelExtract();
  LeaveCriticalSection(&InstallerCS);
  if(isUserCancel) ThrowXML(RuntimeException, XMLExcepts::NoError);
  Successfull = FALSE;
}


int CALLBACK CallbackProc(UINT msg,LONG UserData,LONG P1,LONG P2)
{
  switch(msg)
  {
    case UCM_CHANGEVOLUME:
      if (P2==RAR_VOL_ASK)
      {
        PG_RadioButton *eng = NULL;
		eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");

		if(eng && eng->GetPressed())
		{
			std::string message = "Need file ";
			message += (char *)P1; message += ".\nInsert next disk"; message+=" and press ОК.\nor press Cancel for skip.";
			std::string title = "Insert disk";
			if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
			{   
				Successfull = FALSE;
				return -1;
			}
		}
		else
		{
			std::string message = "Для продолжения нужен файл ";
			message += (char *)P1; message += "\nВставьте следующий диск и нажмите ОК. \nЧтобы пропустить нажмите Отмена.";
			std::string title = "Вставте следующий диск";
			if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
			{   
				Successfull = FALSE;
				return -1;
			}
		}
        return 1;
      }
      return(0);
    case UCM_PROCESSDATA:
		FileUnpacked += P2;
		ArchiveUnpacked += P2;
		infoexpander->SetFileProgress((int)(((double)FileUnpacked/((double)FileUnpackedSize + .0001f))*100));
		infoexpander->SetArchiveProgress((int)(((double)ArchiveUnpacked/((double)ArchiveUnpackedSize + .0001f))*100));
      
		EnterCriticalSection(&InstallerCS);
		if(isUserCancel) CancelExtract();
		LeaveCriticalSection(&InstallerCS);
		if(isUserCancel) ThrowXML(RuntimeException, XMLExcepts::NoError); //_endthreadex(-1);
      return(0);
    case UCM_NEEDPASSWORD:
      PG_LogMSG("Password required: ");
      gets((char *)P1);
      return(0);
  }
  return(0);
}
