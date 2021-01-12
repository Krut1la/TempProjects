#include "infoexpander.h"
#include <pglog.h>

#define PACKVERSION(major,minor) MAKELONG(minor,major)

InfoExpander *infoexpander;

//Layout
extern SDLInstallerLayout *handler;
extern pGetDiskFreeSpaceExF pGetDiskFreeSpaceEx;

extern CRITICAL_SECTION InstallerCS;

BOOL LoadScript( LPCSTR XMLfilename)
{
    // Initialize the XML4C2 system
    try
    {
         XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& e)
    {
		   PG_LogERR("Error initializing XML parser: %s line=%d", XMLString::transcode(e.getMessage()), e.getSrcLine());
           return true;
    }

    SAXParser parser;
    infoexpander = new InfoExpander(XMLfilename);
    try
    {
        parser.setDocumentHandler(infoexpander);
        parser.setErrorHandler(infoexpander);
        parser.parse( XMLfilename );
    }
    catch (const XMLException& e)
    {
		   PG_LogERR("Error parsing XML file: %s line=%d", XMLString::transcode(e.getMessage()), e.getSrcLine());
           return true;
    }

    try
    {
         XMLPlatformUtils::Terminate();
    }
    catch (const XMLException& e)
    {        
		   PG_LogERR("Error terminating XML parser: %s line=%d", XMLString::transcode(e.getMessage()), e.getSrcLine());
           return true;
    }
  return true;
}


InfoExpander::InfoExpander(LPCSTR XMLfilename)
{
	InitializeCriticalSection(&InstallerCS);

	hWndMain = NULL;
	hwndEasyInstaller = NULL;
	InfoExpander::XMLfilename = XMLfilename;

	Title = "";
	Caption = "";

	FullModuleFileName = "";
	ModuleFilePath = "";
	LayoutFileName = "";
	ThemeFileName = "";
	MainDir = "";
	MainDrive = "";

	CurrentScreenShot = 1;
	CountOfScreenShots = 0;
	CurrentWizardPage = 1;
	CountOfWizardPages = 0;

	SetArchiveProgress(0);
	SetFileProgress(0);
	SetCurrentOperation(Title);

    InstallLog = NULL;
	InstallLogType = ILT_UNWISE;

    ShlwapiVersion = GetDllVersion(TEXT("Shlwapi.dll"));
	Shell32Version = GetDllVersion(TEXT("Shell32.dll"));
	Comctl32Version = GetDllVersion(TEXT("Comctl32.dll"));

	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	GetVersionEx(&versionInfo);		

	CurrentGameElement = NULL;
	ParseGameElementInProgress = false;
	GamesCount = 0;

	m_warnings = 0;
    m_errors = 0;
}

InfoExpander::~InfoExpander()
{
  if(InstallLog) CloseHandle(InstallLog);
}

// -----------------------------------------------------------------------
//  Implementations of the SAX DocumentHandler interface
// -----------------------------------------------------------------------

// Called at the end of the XML document
/* Inform the user of any errors
*/
void InfoExpander::endDocument()
{
	
	PG_LogMSG("End parsing XML document: %s warnings %d errors %d",XMLfilename.c_str(), m_warnings,m_errors);
}

// Called at the end of an element </element>
/* Based on type, add to parent
*/
void InfoExpander::endElement(const XMLCh* const name)
{
	char *elementName;
	elementName = XMLString::transcode(name);

	if(stricmp(elementName, "game") == 0)
	{
		ParseGameElementInProgress = false;
		gil.push_back(CurrentGame);
		GamesCount++;
	}
	
    return;
}

// Called when characters exist between <element> and </element>
/* if element is a type that accepts characters as text, call SetText
*/
void InfoExpander::characters(const XMLCh* const chars, const unsigned int length)
{

}

// Called at the beginning of the document
/* Initialize and clear the stack
*/
void InfoExpander::startDocument()
{

	PG_LogMSG("Start parsing XML document: %s\n", XMLfilename.c_str());
    m_warnings = 0;
    m_errors = 0;
}

// Called at the beginning of an element <element>
/*  Create the SAXElement, call the correct LoadX
*/
void InfoExpander::startElement(const XMLCh* const name, AttributeList& attributes)
{
	//HRESULT     hr;
	char *elementName = XMLString::transcode(name);
	PG_LogDBG(elementName);

	if(stricmp(elementName, "game") == 0)
	{
		if(!ParseGameElementInProgress) 
		{
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SI_GameInfo* game = new SI_GameInfo;
					CurrentGame = game;
					CurrentGame->Name = tmp;

					std::string tmp2 = tmp;
					CurrentGameElement = tmp2.c_str();
					ParseGameElementInProgress = true;

				}
			}
		}
		else
			PG_LogERR("trying create new game before game '%s' is complete", CurrentGame->Name.c_str());
	}
	else if(stricmp(elementName, "title") == 0)
	{
		if(ParseGameElementInProgress) 
		{ 
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "value") == 0)
				{
					//infoexpander->Title = XMLString::transcode(attributes.getValue(attributeName));
					CurrentGame->Title = XMLString::transcode(attributes.getValue(attributeName));
				}
			}
		}
	}//if title
	else if(stricmp(elementName, "caption") == 0)
	{
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "value") == 0)
			{
				infoexpander->Caption = XMLString::transcode(attributes.getValue(attributeName));
			}
		}
	}//if caption
  else if(stricmp(elementName, "cd") == 0)
  {
    std::string label;
    BYTE disknum = 0;
    for(unsigned int i = 0; i < attributes.getLength(); i++)
    {
      char *attributeName = XMLString::transcode(attributes.getName(i));
      if(stricmp(attributeName, "label") == 0)
      {
        label = XMLString::transcode(attributes.getValue(attributeName));
      }
      else if(stricmp(attributeName, "disk") == 0)
      {
        disknum = StrToInt(XMLString::transcode(attributes.getValue(attributeName)));
      }
    }

    //Scan for CD
    char drive[4];
    char volume[255] = {0};

    drive[0] = 'c';
    drive[1] = ':';
    drive[2] = '\\';
    drive[3] = 0;

    for (drive[0] = 'c' ; drive[0] <= 'z' ; drive[0]++)
    {
      if (GetDriveType (drive) == DRIVE_CDROM)
      {
        if(GetVolumeInformation(drive,volume,255,0,0,0,0,0))
          if(!strcmp(label.c_str(),volume))
          {
            std::string sdrive = drive;
            InstallDrivesList.push_back(sdrive);
            InstallDiskNumList.push_back(disknum);
            PG_LogMSG("Disk number %d with label '%s' in drive '%s'", disknum, label.c_str(), sdrive.c_str());
          }
      }

    }

    //InstallDrivesList.push_back(label);

  }//if cd
	else if(stricmp(elementName, "theme") == 0)
	{
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "filename") == 0)
			{
				infoexpander->ThemeFileName = XMLString::transcode(attributes.getValue(attributeName));
			}
		}
	}//if theme
	else if(stricmp(elementName, "layout") == 0)
	{
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "filename") == 0)
			{
				infoexpander->LayoutFileName = XMLString::transcode(attributes.getValue(attributeName));
			}
		}
	}//if layuot
	else if(stricmp(elementName, "install") == 0)
	{
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "filename") == 0)
			{
				infoexpander->InstallXMLFileName = XMLString::transcode(attributes.getValue(attributeName));
			}
		}
	}//if install
	else if(stricmp(elementName, "video") == 0)
	{
		if(ParseGameElementInProgress) 
		{ 
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "generation") == 0)
				{
					//infoexpander->VideoGenerationNeed = XMLString::transcode(attributes.getValue(attributeName));
					CurrentGame->VideoGenerationNeed = XMLString::transcode(attributes.getValue(attributeName));
				}
			}
		}
	}//if install
	else if(stricmp(elementName, "hddfreespace") == 0)
	{
		if(ParseGameElementInProgress) 
		{ 
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "value") == 0)
				{
					sscanf(XMLString::transcode(attributes.getValue(attributeName)),"%I64i", &infoexpander->RequiredSize);
					//infoexpander->RequiredSize = XMLString::transcode(attributes.getValue(attributeName));
				}
			}
		}
	}//if required_size
	else if(stricmp(elementName, "ram") == 0)
	{
		if(ParseGameElementInProgress) 
		{ 
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "value") == 0)
				{
					//sscanf(XMLString::transcode(attributes.getValue(attributeName)),"%d", &infoexpander->RAMNeed);
					sscanf(XMLString::transcode(attributes.getValue(attributeName)),"%d", &CurrentGame->RAMNeed);
					//infoexpander->RequiredSize = XMLString::transcode(attributes.getValue(attributeName));
				}
			}
		}
	}//if required_size
	else if(stricmp(elementName, "directx") == 0)
	{
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "value") == 0)
			{
				sscanf(XMLString::transcode(attributes.getValue(attributeName)),"%x", &infoexpander->DXVersionNeed);
			}
			if(stricmp(attributeName, "path") == 0)
			{
			    infoexpander->dxpath = XMLString::transcode(attributes.getValue(attributeName));
				infoexpander->ExpandString(&infoexpander->dxpath, &infoexpander->dxpath);
			}
			
		}
	}//if directx
	else if(stricmp(elementName, "cpu") == 0)
	{
		if(ParseGameElementInProgress) 
		{ 
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "frequency") == 0)
				{
					//sscanf(XMLString::transcode(attributes.getValue(attributeName)),"%d", &infoexpander->CPUMHZNeed);
					sscanf(XMLString::transcode(attributes.getValue(attributeName)),"%d", &CurrentGame->CPUMHZNeed);
					//infoexpander->RequiredSize = XMLString::transcode(attributes.getValue(attributeName));
				}
			}
		}
	}//if cpu
	else if(stricmp(elementName, "default_dir") == 0)
	{
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "value") == 0)
			{
				infoexpander->MainDir = XMLString::transcode(attributes.getValue(attributeName));
                infoexpander->MainDir+="\\";
                infoexpander->MainDir+=infoexpander->Title;
				infoexpander->ExpandString(&(infoexpander->MainDir),&(infoexpander->MainDir));
				if(infoexpander->MainDir.length()>=3)
				{
					std::string MainDrive = infoexpander->MainDir;
				    MainDrive.erase(3,MainDrive.length());
				    infoexpander->MainDrive = MainDrive;
				}
			}
		}
	}//if defaultdir
	else if(stricmp(elementName, "installlog") == 0)
	{
		std::string IL_Type;
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{
			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "type") == 0)
			{
				IL_Type = XMLString::transcode(attributes.getValue(attributeName));
				if(/*IL_Type == "unwise"*/stricmp(IL_Type.c_str(), "unwise") == 0) InstallLogType = ILT_UNWISE;
				else if(/*IL_Type == "xmllog"*/stricmp(IL_Type.c_str(), "xmllog") == 0) InstallLogType = ILT_XML;
				else InstallLogType = ILT_NONE;
				
			}
			else if(stricmp(attributeName, "location") == 0)
			{
				IL_Filename = XMLString::transcode(attributes.getValue(attributeName));
				infoexpander->ExpandString(&IL_Filename, &IL_Filename);
				
			}
		}
	}//if installlog
  
}//SDLInstallerLayout::startElement



// -----------------------------------------------------------------------
//  Implementations of the SAX ErrorHandler interface
// -----------------------------------------------------------------------
// warning exception handler
void InfoExpander::warning(const SAXParseException& exception)
{
  m_warnings++;
  PG_LogWRN("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}
// error exception handler
void InfoExpander::error(const SAXParseException& exception)
{
  m_errors++;
  PG_LogERR("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}
// fatal error exception handler
void InfoExpander::fatalError(const SAXParseException& exception)
{
  PG_LogERR("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}

void InfoExpander::SetArchiveProgress(UINT ap)
{
	EnterCriticalSection(&InstallerCS);
	ArchiveProgress = ap;
	//SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_PROGRESS2), PBM_SETPOS, (WPARAM)ap, 0);
	LeaveCriticalSection(&InstallerCS);
	SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_PROGRESS2), PBM_SETPOS, (WPARAM)ap, 0);
}
void InfoExpander::SetFileProgress(UINT fp)
{
	EnterCriticalSection(&InstallerCS);
	FileProgress = fp;
	//SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_PROGRESS1), PBM_SETPOS, (WPARAM)fp, 0);
    LeaveCriticalSection(&InstallerCS);
	SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_PROGRESS1), PBM_SETPOS, (WPARAM)fp, 0);
}
void InfoExpander::SetCurrentOperation(std::string co)
{
	EnterCriticalSection(&InstallerCS);
	CurrentOperation = co;
	//SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_STATIC_CO), WM_SETTEXT, 0, (LPARAM)co.c_str());
	LeaveCriticalSection(&InstallerCS);
	SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_STATIC_CO), WM_SETTEXT, 0, (LPARAM)co.c_str());
	// Request ownership of the critical section.
	/*__try 
	{
		EnterCriticalSection(&InstallerCS); 

		// Access the shared resource.
		CurrentOperation = co;
	}
	__finally 
	{
		// Release ownership of the critical section.
		LeaveCriticalSection(&InstallerCS);
	}*/


}

UINT InfoExpander::GetArchiveProgress(void)
{
	UINT rvalue;
	EnterCriticalSection(&InstallerCS);
	rvalue = ArchiveProgress;
	LeaveCriticalSection(&InstallerCS);
	return rvalue;
}
UINT InfoExpander::GetFileProgress(void)
{
	UINT rvalue;
	EnterCriticalSection(&InstallerCS);
	rvalue = FileProgress;
	LeaveCriticalSection(&InstallerCS);
	return rvalue;
}
std::string InfoExpander::GetCurrentOperation(void)
{
	std::string rvalue;
	EnterCriticalSection(&InstallerCS);
	rvalue = CurrentOperation;
	LeaveCriticalSection(&InstallerCS);
	return rvalue;
}

//Parse string. "Time is %TIME% now" = "Time is 10:20:30 now"
void InfoExpander::ExpandString(std::string *string, std::string *rvalue)
{
	//HRESULT hResult;
	char SpaceMB[21] = {0};
	//Restore originl string
	rvalue->replace(0,rvalue->length(), string->c_str());
	LPCSTR ttt= rvalue->c_str();

	DWORD start_sh = 0, end_sh = 0;

	//find first and second '%'
	start_sh = rvalue->find("%",0);
	end_sh = rvalue->find("%",start_sh + 1);

	//until last find '%'-para
	while(start_sh != rvalue->npos && end_sh != rvalue->npos)
	{

		//%TIME%
		if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%TIME%"))
		{
			char tmpbuf[128];
			_strtime(tmpbuf);
			rvalue->replace(start_sh,end_sh - start_sh + 1,tmpbuf);
		}
		//%DATE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%DATE%"))
		{
			rvalue->replace(start_sh,end_sh - start_sh + 1,"1, December");
		}
        //%TITLE%
        else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%TITLE%"))
        {
            rvalue->replace(start_sh,end_sh - start_sh + 1,infoexpander->Title.c_str());
        }
		//%CUR_DX_VER%
        else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_DX_VER%"))
        {
			std::string ver;
			switch(DXVersion)
			{
			case 0x00000000: ver = "No DirectX"; break;
			case 0x00010000: ver = "1.0"; break;
			case 0x00020000: ver = "2.0"; break;
			case 0x00030000: ver = "3.0"; break;
			case 0x00030001: ver = "3.0a"; break;
			case 0x00050000: ver = "5.0"; break;
			case 0x00060000: ver = "6.0"; break;
			case 0x00060100: ver = "6.1"; break;
			case 0x00060101: ver = "6.1a"; break;
			case 0x00070000: ver = "7.0"; break;
			case 0x00070001: ver = "7.0a"; break;
			case 0x00080000: ver = "8.0"; break;
			case 0x00080100: ver = "8.1"; break;
			case 0x00080101: ver = "8.1a"; break;
			case 0x00080102: ver = "8.1b"; break;
			case 0x00080200: ver = "8.2"; break;
			case 0x00090000: ver = "9.0"; break;
			};
            rvalue->replace(start_sh,end_sh - start_sh + 1,ver.c_str());
        }
		//%REQ_DX_VER%
        else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQ_DX_VER%"))
        {
			std::string ver;
			switch(DXVersionNeed)
			{
			case 0x00000000: ver = "No DirectX"; break;
			case 0x00010000: ver = "1.0"; break;
			case 0x00020000: ver = "2.0"; break;
			case 0x00030000: ver = "3.0"; break;
			case 0x00030001: ver = "3.0a"; break;
			case 0x00050000: ver = "5.0"; break;
			case 0x00060000: ver = "6.0"; break;
			case 0x00060100: ver = "6.1"; break;
			case 0x00060101: ver = "6.1a"; break;
			case 0x00070000: ver = "7.0"; break;
			case 0x00070001: ver = "7.0a"; break;
			case 0x00080000: ver = "8.0"; break;
			case 0x00080100: ver = "8.1"; break;
			case 0x00080101: ver = "8.1a"; break;
			case 0x00080102: ver = "8.1b"; break;
			case 0x00080200: ver = "8.2"; break;
			case 0x00090000: ver = "9.0"; break;
			};
            rvalue->replace(start_sh,end_sh - start_sh + 1,ver.c_str());
        }
		//%CUR_DRIVE_FREE_SPACE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_DRIVE_FREE_SPACE%"))
		{
			GetFreeSpaceF(SpaceMB, 21, MainDrive.c_str(), FORMAT_B);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		//%CUR_DRIVE_FREE_SPACE_KB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_DRIVE_FREE_SPACE_KB%"))
		{
			GetFreeSpaceF(SpaceMB, 21, MainDrive.c_str(), FORMAT_KB);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		//%CUR_DRIVE_FREE_SPACE_MB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_DRIVE_FREE_SPACE_MB%"))
		{
			GetFreeSpaceF(SpaceMB, 21, MainDrive.c_str(), FORMAT_MB);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		//%CUR_DRIVE_FREE_SPACE_GB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_DRIVE_FREE_SPACE_GB%"))
		{
			GetFreeSpaceF(SpaceMB, 21, MainDrive.c_str(), FORMAT_GB);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		//%MAINDIR%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%MAINDIR%"))
		{
			rvalue->replace(start_sh, end_sh - start_sh + 1, MainDir.c_str());
		}
		//%MAIN_DRIVE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%MAIN_DRIVE%"))
		{
      std::string sdrive = MainDrive;
      sdrive.erase(2,1);
			rvalue->replace(start_sh, end_sh - start_sh + 1, sdrive.c_str());
		}
    //%INST_DRIVE%
    else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%INST_DRIVE%"))
    {
      rvalue->replace(start_sh, end_sh - start_sh + 1, MainDrive.c_str());
    }
    //%LABEL_DRIVE_XXXXXXXX%
    else if((end_sh - start_sh + 1) > 12 && !rvalue->compare(start_sh,13,"%LABEL_DRIVE_"))
    {
      //Scan for CD
      char drive[4];
      char volume[255] = {0};

      drive[0] = 'c';
      drive[1] = ':';
      drive[2] = '\\';
      drive[3] = 0;

      std::string label = rvalue->c_str();
      label.erase(end_sh, label.length() - end_sh);
      label.erase(0,start_sh + 13);

      BOOL foundCD = FALSE;
      for (drive[0] = 'c' ; drive[0] <= 'z' ; drive[0]++)
      {
        if (GetDriveType (drive) == DRIVE_CDROM)
        {
          if(GetVolumeInformation(drive,volume,255,0,0,0,0,0))
            if(!strcmp(label.c_str(),volume))
            {
               std::string sdrive = drive;
               sdrive.erase(2,1);
               rvalue->replace(start_sh, end_sh - start_sh + 1, sdrive.c_str()); 
               foundCD = TRUE;
            }
        }
      }
      if(!foundCD){
        std::string sdrive = MainDrive;
        sdrive.erase(2,1);
        rvalue->replace(start_sh, end_sh - start_sh + 1, sdrive.c_str());
      }
    }
		//%CUR_WIZARD_PAGE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_WIZARD_PAGE%"))
		{
			sprintf(SpaceMB,"%d",CurrentWizardPage);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		//%COUNT_WIZARD_PAGES%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%COUNT_WIZARD_PAGES%"))
		{
			sprintf(SpaceMB,"%d",CountOfWizardPages);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %CUR_SCREENSHOT%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CUR_SCREENSHOT%"))
		{
			sprintf(SpaceMB,"%d",CurrentScreenShot);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %COUNT_SCREENSHOTS%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%COUNT_SCREENSHOTS%"))
		{
			sprintf(SpaceMB,"%d",CountOfScreenShots);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %CURRENT_OPERATION%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CURRENT_OPERATION%"))
		{
			rvalue->replace(start_sh, end_sh - start_sh + 1, GetCurrentOperation().c_str());
		}
        // %ARCHIVE_PROGRESS%
        else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%ARCHIVE_PROGRESS%"))
        {
            sprintf(SpaceMB,"%d",ArchiveProgress);
            rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
        }
        // %FILE_PROGRESS%
        else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%FILE_PROGRESS%"))
        {
            sprintf(SpaceMB,"%d",FileProgress);
            rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
        }
		// %REQUIRED_SPACE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQUIRED_SPACE%"))
		{
			GetRequiredSpace(SpaceMB, 21, FORMAT_B);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %REQUIRED_SPACE_KB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQUIRED_SPACE_KB%"))
		{
			GetRequiredSpace(SpaceMB, 21, FORMAT_KB);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %REQUIRED_SPACE_MB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQUIRED_SPACE_MB%"))
		{
			GetRequiredSpace(SpaceMB, 21, FORMAT_MB);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %REQUIRED_SPACE_GB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQUIRED_SPACE_GB%"))
		{
			GetRequiredSpace(SpaceMB, 21, FORMAT_GB);
			rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
		}
		// %CPU_STRING%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CPU_STRING%"))
		{
			rvalue->replace(start_sh, end_sh - start_sh + 1, CPUString.c_str());
		}
		// %CPU_MHZ%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CPU_MHZ%"))
		{
			sprintf(SpaceMB,"%d",CPUMHZ);
            rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
			//rvalue->replace(start_sh, end_sh - start_sh + 1, CPUMHZ.c_str());
		}
		// %REQUIRED_MHZ%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQUIRED_MHZ%"))
		{
			sprintf(SpaceMB,"%d",CPUMHZNeed);
            rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
			//rvalue->replace(start_sh, end_sh - start_sh + 1, CPUMHZ.c_str());
		}
		// %REQUIRED_RAM%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%REQUIRED_RAM%"))
		{
			sprintf(SpaceMB,"%d",RAMNeed);
            rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
			//rvalue->replace(start_sh, end_sh - start_sh + 1, CPUMHZ.c_str());
		}
		// %VIDEO_STRING%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%VIDEO_STRING%"))
		{
			rvalue->replace(start_sh, end_sh - start_sh + 1, VideoString.c_str());
		}
        // %RAM_MB%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%RAM_MB%"))
		{
			sprintf(SpaceMB,"%d",RAMMB);
            rvalue->replace(start_sh, end_sh - start_sh + 1, SpaceMB);
			//rvalue->replace(start_sh, end_sh - start_sh + 1, CPUMHZ.c_str());
		}


        //CSIDL

		// %CSIDL_ADMINTOOLS%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_ADMINTOOLS%"))
				SHReplaceCSIDL(start_sh, end_sh, CSIDL_ADMINTOOLS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_ALTSTARTUP%"))
			    SHReplaceCSIDL(start_sh, end_sh, CSIDL_ALTSTARTUP ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_APPDATA%"))
			    SHReplaceCSIDL(start_sh, end_sh, CSIDL_APPDATA ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_BITBUCKET%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_BITBUCKET,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_ADMINTOOLS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_ADMINTOOLS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_ALTSTARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_ALTSTARTUP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_APPDATA%")){
			if(Shell32Version >= PACKVERSION(5,0)){
				SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_APPDATA,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
				nSize = GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
                nSize = sprintf(tchBuffer, "%s\\All Users\\Application Data", lpszSystemInfo); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
			}
		}
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_DESKTOPDIRECTORY%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_DESKTOPDIRECTORY,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_DOCUMENTS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_DOCUMENTS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_FAVORITES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_FAVORITES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_PROGRAMS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_PROGRAMS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_STARTMENU%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_STARTMENU,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_STARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_STARTUP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_TEMPLATES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_TEMPLATES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_CONTROLS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_CONTROLS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COOKIES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COOKIES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_DESKTOP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_DESKTOP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_DESKTOPDIRECTORY%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_DESKTOPDIRECTORY,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_DRIVES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_DRIVES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_FAVORITES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_FAVORITES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_FONTS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_FONTS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_HISTORY%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_HISTORY,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_INTERNET%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_INTERNET,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_INTERNET_CACHE%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_INTERNET_CACHE ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_LOCAL_APPDATA%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_LOCAL_APPDATA ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_MYMUSIC%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_MYMUSIC,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_MYPICTURES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_MYPICTURES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_NETHOOD%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_NETHOOD,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_NETWORK%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_NETWORK,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PERSONAL%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PERSONAL,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PRINTERS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PRINTERS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PRINTHOOD%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PRINTHOOD,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROFILE%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROFILE,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROGRAM_FILES%")){
			if(Shell32Version >= PACKVERSION(5,0)){
               SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROGRAM_FILES,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
                nSize = GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
                *(lpszSystemInfo + 2) = 0;
				nSize = sprintf(tchBuffer, "%s\\Program Files",	lpszSystemInfo	); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
    		}
		}
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROGRAM_FILES_COMMON%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROGRAM_FILES_COMMON,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROGRAMS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROGRAMS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_RECENT%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_RECENT,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_SENDTO%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_SENDTO,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_STARTMENU%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_STARTMENU,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_STARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_STARTUP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_SYSTEM%")){
			if(Shell32Version >= PACKVERSION(5,0)){
               SHReplaceCSIDL(start_sh, end_sh, CSIDL_SYSTEM,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
                nSize = GetSystemDirectory(lpszSystemInfo, MAX_PATH); 
                nSize = sprintf(tchBuffer, "%s",	lpszSystemInfo	); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
    		}
		}
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_TEMPLATES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_TEMPLATES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_WINDOWS%")){
			if(Shell32Version >= PACKVERSION(5,0)){
               SHReplaceCSIDL(start_sh, end_sh, CSIDL_WINDOWS,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
                nSize = GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
                //*(lpszSystemInfo + 2) = 0;
				nSize = sprintf(tchBuffer, "%s",	lpszSystemInfo	); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
    		}
		}
	
		else
		{
			//find next and second '%'
			start_sh = rvalue->find("%",end_sh + 1);
			end_sh = rvalue->find("%",start_sh + 1);
			continue; // to skip this
		}                    // | 
		                     // | 
		                     // V
		//find next and second '%'
		start_sh = rvalue->find("%",0);
		end_sh = rvalue->find("%",start_sh + 1);
	}
}

//GetRequiredSpace
bool GetRequiredSpace(LPSTR buff, UINT bufflen, DWORD format)
{
	if(buff == NULL) return false;
	memset(buff, 0, bufflen);

	switch (format)
		{
		case FORMAT_B:   sprintf(buff,"%I64i",(infoexpander->RequiredSize));
			break; 
		case FORMAT_KB:  sprintf(buff,"%I64i",(infoexpander->RequiredSize)/(1024));
			break;
		case FORMAT_MB:  sprintf(buff,"%I64i",(infoexpander->RequiredSize)/(1024*1024));
			break;
		case FORMAT_GB:  sprintf(buff,"%I64i",(infoexpander->RequiredSize)/(1024*1024*1024));
			break;
		default:
			break;
		}
	char* tmp = new char[bufflen];
	int s = strlen(buff) - 1;
	int k = 0, i, j;
	for(i = s, j = s + s/3 ; i >= 0; i--,j--)
	{
		*(tmp + j) = *(buff+i);
		k++;
		if(k == 3 && i != 0)
		{
			j--;
			*(tmp + j) = ',';
			k = 0;
		}
	}
	*(tmp + s + s/3 + 1) = '\0';
	strcpy(buff,tmp);
	delete tmp;
	return true;
}

UINT GetFreeSpacePercent(void)
{
	HRESULT hResult;
	if (pGetDiskFreeSpaceEx)
	{
		LARGE_INTEGER i64FreeBytesAvailableToCaller;
		LARGE_INTEGER i64TotalNumberOfBytes;
		LARGE_INTEGER i64TotalNumberOfFreeBytes;  

		hResult = GetDiskFreeSpaceEx (infoexpander->MainDrive.c_str(),
			(PULARGE_INTEGER)&i64FreeBytesAvailableToCaller,
			(PULARGE_INTEGER)&i64TotalNumberOfBytes,
			(PULARGE_INTEGER)&i64TotalNumberOfFreeBytes);


		if(hResult)
		{		
			return UINT(((float)i64TotalNumberOfFreeBytes.QuadPart/(float)i64TotalNumberOfBytes.QuadPart)*100);
		}
		else return 0;
	}
	else 
	{
		DWORD dwSectPerClust;
		DWORD dwBytesPerSect;
		DWORD dwFreeClusters;
		DWORD dwTotalClusters;

		hResult = GetDiskFreeSpace (infoexpander->MainDrive.c_str(), 
			&dwSectPerClust, 
			&dwBytesPerSect,
			&dwFreeClusters, 
			&dwTotalClusters);

		if(hResult)
		{		
			
			return UINT(((float)(dwFreeClusters*dwSectPerClust*dwBytesPerSect)/(float)(dwTotalClusters*dwSectPerClust*dwBytesPerSect))*100);
		}
		else return 0;
	}
}

//GetDiskFreeSpace
bool GetFreeSpaceF(LPSTR buff, UINT bufflen, LPCTSTR disk, DWORD format)
{
	if(buff == NULL) return false;
	memset(buff, 0, bufflen);

	HRESULT hResult;
	if (pGetDiskFreeSpaceEx)
	{
		ULARGE_INTEGER i64FreeBytesAvailableToCaller;
		ULARGE_INTEGER i64TotalNumberOfBytes;
		ULARGE_INTEGER i64TotalNumberOfFreeBytes;  

		hResult = pGetDiskFreeSpaceEx (disk,
			(PULARGE_INTEGER)&i64FreeBytesAvailableToCaller,
			(PULARGE_INTEGER)&i64TotalNumberOfBytes,
			(PULARGE_INTEGER)&i64TotalNumberOfFreeBytes);


        infoexpander->FreeSize = i64TotalNumberOfFreeBytes.QuadPart;

		if(hResult)
		{		
			switch (format)
			{
			case FORMAT_B:   sprintf(buff,"%I64i",(i64TotalNumberOfFreeBytes.QuadPart));
				break; 
			case FORMAT_KB:  sprintf(buff,"%I64i",(i64TotalNumberOfFreeBytes.QuadPart)/(1024));
				break;
			case FORMAT_MB:  sprintf(buff,"%I64i",(i64TotalNumberOfFreeBytes.QuadPart)/(1024*1024));
				break;
			case FORMAT_GB:  sprintf(buff,"%I64i",(i64TotalNumberOfFreeBytes.QuadPart)/(1024*1024*1024));
				break;
			default:
				break;
			}
			char* tmp = new char[bufflen];
			int s = strlen(buff) - 1;
			int k = 0, i, j;
			for(i = s, j = s + s/3 ; i >= 0; i--,j--)
			{
				*(tmp + j) = *(buff+i);
				k++;
				if(k == 3 && i != 0)
				{
					j--;
					*(tmp + j) = ',';
					k = 0;
				}
			}
			*(tmp + s + s/3 + 1) = '\0';
			strcpy(buff,tmp);
			delete tmp;
			return true;
		}
		else return false;
	}
	else 
	{
		DWORD dwSectPerClust;
		DWORD dwBytesPerSect;
		DWORD dwFreeClusters;
		DWORD dwTotalClusters;

		hResult = GetDiskFreeSpace (disk, 
			&dwSectPerClust, 
			&dwBytesPerSect,
			&dwFreeClusters, 
			&dwTotalClusters);

		if(hResult)
		{		
			switch (format)
			{
			case FORMAT_B:   sprintf(buff,"%lu",(dwFreeClusters*dwSectPerClust*dwBytesPerSect));
				break; 
			case FORMAT_KB:  sprintf(buff,"%lu",(dwFreeClusters*dwSectPerClust*dwBytesPerSect)/(1024));
				break;
			case FORMAT_MB:  sprintf(buff,"%lu",(dwFreeClusters*dwSectPerClust*dwBytesPerSect)/(1024*1024));
				break;
			case FORMAT_GB:  sprintf(buff,"%lu",(dwFreeClusters*dwSectPerClust*dwBytesPerSect)/(1024*1024*1024));
				break;
			default:
				break;
			}
			char* tmp = new char[bufflen];
			int s = strlen(buff) - 1;
			int k = 0, i, j;
			for(i = s, j = s + s/3 ; i >= 0; i--,j--)
			{
				*(tmp + j) = *(buff+i);
				k++;
				if(k == 3 && i != 0)
				{
					j--;
					*(tmp + j) = ',';
					k = 0;
				}
			}
			*(tmp + s + s/3 + 1) = '\0';
			strcpy(buff,tmp);
			delete tmp;
			return true;
		}
		else return false;
	}

}

void SHReplaceCSIDL(DWORD start_sh, DWORD end_sh, DWORD csidl, std::string *str)
{
	LPSTR path = new char[MAX_PATH];
	LPCSTR val = str->c_str();
	LPMALLOC pMalloc = NULL;
	LPITEMIDLIST ppidl = NULL;

	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	SHGetMalloc(&pMalloc);

	//SHGetSpecialFolderPath(NULL, path, csidl, FALSE);
	SHGetSpecialFolderLocation(NULL,csidl,&ppidl);
	if(ppidl)
	{
		//LPSTR path = new char[MAX_PATH];
		SHGetPathFromIDList(ppidl,path);
	}

	str->replace(start_sh, end_sh - start_sh + 1, path);
	delete path;

	pMalloc->Release();
	CoUninitialize();
}



DWORD InfoExpander::GetDllVersion(LPCTSTR lpszDllName)
{

    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

/*Because some DLLs might not implement this function, you
  must test for it explicitly. Depending on the particular 
  DLL, the lack of a DllGetVersion function can be a useful
  indicator of the version.
*/
        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }
        
        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

