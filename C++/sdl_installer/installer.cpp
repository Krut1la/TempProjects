#include "installer.h"
#include "layout.h"
#include <pglog.h>
#include "infoexpander.h"


extern InfoExpander *infoexpander;
extern pGetFileSizeExF pGetFileSizeEx;
ExtractF Extract;

Installer *installer;
HANDLE InstallerThread = NULL;
unsigned InstallerThreadId; 
CRITICAL_SECTION InstallerCS;

BOOL Successfull = TRUE;



//Layout
extern SDLInstallerLayout *handler;

std::string CurrentOperation;
UINT ArchiveProgress = 0;
UINT FileProgress    = 0;

//oggextractF oggextract = NULL;

//HMODULE LibraryOgg = NULL;


unsigned _stdcall ThreadProc(void *argv)
{
//	LibraryOgg = LoadLibrary("oggd.dll");
//	if(!LibraryOgg) MessageBox(infoexpander->hWndMain,"Can't open oggd.dll", "Error", MB_ICONERROR);
//	oggextract = (oggextractF)GetProcAddress(LibraryOgg, "oggextract");

	//_beginthread();
	LPCSTR XMLfilename = infoexpander->InstallXMLFileName.c_str();
	// Initialize the XML4C2 system
	try
	{
		XMLPlatformUtils::Initialize();
	}

	catch (const XMLException& e)
	{
		PG_LogERR("Error initializing XML parser: %s line=%d", XMLString::transcode(e.getMessage()), e.getSrcLine());
		return TRUE;
	}

	SAXParser parser;
	installer = new Installer(XMLfilename);
	try
	{
		parser.setDocumentHandler(installer);
		parser.setErrorHandler(installer);
		parser.parse( XMLfilename );
	}
	catch (const XMLException& e)
	{
		PG_LogERR("Error parsing XML file: %s line=%d", XMLString::transcode(e.getMessage()), e.getSrcLine());
		return TRUE;
	}

	try
	{
		XMLPlatformUtils::Terminate();
	}
	catch (const XMLException& e)
	{        

		PG_LogERR("Error terminating XML parser: %s line=%d", XMLString::transcode(e.getMessage()), e.getSrcLine());
		return TRUE;
	}
	return TRUE;
}

// constructor
Installer::Installer(LPCSTR XMLfilename)
{
	Installer::XMLfilename = XMLfilename;
	IsCurrentComponentInstall = false;
	m_warnings = 0;
	m_errors = 0;
}

//! destructor
Installer::~Installer()
{
	ComponentList.erase(ComponentList.begin(), ComponentList.end());
//	if(LibraryOgg) { FreeLibrary(LibraryOgg); LibraryOgg = NULL;}
}

// -----------------------------------------------------------------------
//  Implementations of the SAX DocumentHandler interface
// -----------------------------------------------------------------------

// Called at the end of the XML document
/* Inform the user of any errors
*/
void Installer::endDocument()
{
	infoexpander->SetArchiveProgress(100);
	PG_LogMSG("Installation complete");

	PG_RadioButton *eng = NULL;
	eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");

	if(eng && eng->GetPressed())
	{
		if(Successfull) infoexpander->SetCurrentOperation("Installation complete!");
		else infoexpander->SetCurrentOperation("Game was installed not complete!");
	}
	else
	{
		if(Successfull) infoexpander->SetCurrentOperation("Игра успешно установлена!");
		else infoexpander->SetCurrentOperation("Игра установлена неполностью!");
	}


	PG_LogMSG("End parsing XML document: %s warnings %d errors %d",XMLfilename.c_str(), m_warnings,m_errors);

	PG_Widget *cancel = handler->GetWizard()->GetWidgetByAction("cancel");
	if(cancel) 
	{
		cancel->EnableReceiver(false);
		cancel->SetFontAlpha(100);
		cancel->Redraw();
		//SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_EXIT), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_EXIT),GWL_STYLE)&(~WS_DISABLED));

	}
	PG_Widget *ready = handler->GetWizard()->GetWidgetByName("BReady");
	if(ready) 
	{
		ready->EnableReceiver(true);
		ready->SetFontAlpha(255);
		ready->Redraw();
		//SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_EXIT), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_EXIT),GWL_STYLE)&(~WS_DISABLED));
	}

	PG_Widget *start = handler->GetWizard()->GetWidgetByName("BStart");
	if(start) 
	{
		start->EnableReceiver(false);
		start->SetFontAlpha(100);
		//SendMessageA(GetDlgItem(hwndEasyInstaller, IDC_BUTTON_START), PBM_SETPOS, (WPARAM)ap, 0);
		SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_START), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_START),GWL_STYLE)|WS_DISABLED);

	}
	PG_Widget *uninstall = handler->GetWizard()->GetWidgetByName("BUninstall");
	if(uninstall) 
	{
		uninstall->EnableReceiver(false);
		uninstall->SetFontAlpha(100);
		SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_UNINSTALL), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_UNINSTALL),GWL_STYLE)|WS_DISABLED);
	}

	//query for previous installation
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
		if(filename == INVALID_HANDLE_VALUE || filename == NULL) 
		{
			PG_Widget *start = handler->GetWizard()->GetWidgetByName("BStart");
			if(start) 
			{
				start->EnableReceiver(false);
				start->SetFontAlpha(100);
				SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_START), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_START),GWL_STYLE)|WS_DISABLED);
			}
		}
		else
		{
			PG_Widget *start = handler->GetWizard()->GetWidgetByName("BStart");
			if(start) 
			{
				start->EnableReceiver(true);
				start->SetFontAlpha(255);
				SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_START), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_START),GWL_STYLE)&(~WS_DISABLED));
			}
		}

		//clear
		//if(filename)
		FindClose(filename);
		filename = NULL;
		memset(buffer,0,255);
		cbbuffer = 255;

		//query for uninstal.exe
		RegQueryValueEx(hk,"UninstallString",0, NULL, &buffer[0], &cbbuffer);
		filename = FindFirstFile((LPCSTR)buffer,lpFindFileData);
		if(filename == INVALID_HANDLE_VALUE || filename == NULL) 
		{
			PG_Widget *uninstall = handler->GetWizard()->GetWidgetByName("BUninstall");
			if(uninstall) 
			{
				uninstall->EnableReceiver(false);
				uninstall->SetFontAlpha(100);
				SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_UNINSTALL), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_UNINSTALL),GWL_STYLE)|WS_DISABLED);
			}
		}
		else
		{
			PG_Widget *uninstall = handler->GetWizard()->GetWidgetByName("BUninstall");
			if(uninstall) 
			{
				uninstall->EnableReceiver(true);
				uninstall->SetFontAlpha(255);
				SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_UNINSTALL), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_UNINSTALL),GWL_STYLE)&(~WS_DISABLED));
			}
		}
	}
	//if(filename) CloseHandle(filename);
	FindClose(filename);

	RegCloseKey(hk);
	delete lpFindFileData;

	if(infoexpander->InstallLog) CloseHandle(infoexpander->InstallLog);

}

// Called at the end of an element </element>
/* Based on type, add to parent
*/
void Installer::endElement(const XMLCh* const name)
{
	char *elementName;
	elementName = XMLString::transcode(name);

	if(stricmp(elementName, "component") == 0)
	{
		IsCurrentComponentInstall = false;
	}
	return;
}

// Called when characters exist between <element> and </element>
/* if element is a type that accepts characters as text, call SetText
*/
void Installer::characters(const XMLCh* const chars, const unsigned int length)
{

}

// Called at the beginning of the document
/* Initialize and clear the stack
*/
void Installer::startDocument()
{
	HRESULT hr = 0;
	PG_LogMSG("Start parsing XML document: %s\n", XMLfilename.c_str());
	m_warnings = 0;
	m_errors = 0;


	//handler->GetWizard()->ShowFirstScreenShot(true);
	if(infoexpander->InstallLogType != ILT_NONE)
	{

		DWORD NumberOfBytesWriten = 0;
		//infoexpander->InstallLog = CreateFile((infoexpander->MainDir + "\\uninstall.xml").c_str(),GENERIC_WRITE,NULL,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		//CreateDirectory(infoexpander->MainDir.c_str(),NULL);
		CreateDirectoryExEx(infoexpander->MainDir.c_str());
		std::string IL_path = (infoexpander->MainDir +"\\" + infoexpander->IL_Filename);
		infoexpander->InstallLog = CreateFile(IL_path.c_str(),GENERIC_WRITE,NULL,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

		if(infoexpander->InstallLog == INVALID_HANDLE_VALUE)
			PG_LogERR("startDocument: Can't create %s", infoexpander->IL_Filename.c_str());
		else{
			PG_LogMSG("startDocument: %s created", infoexpander->IL_Filename.c_str());    

			std::string header;
			//XML
			if(infoexpander->InstallLogType == ILT_XML) header = "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\r\n";
			//UNWISE
			else if(infoexpander->InstallLogType == ILT_UNWISE) header = "***  Installation Started %DATE% %TIME%  ***\r\nTitle: %TITLE%\r\nSource: %INST_DRIVE%\r\n"; 

			infoexpander->ExpandString(&header, &header);

			hr = WriteFile(infoexpander->InstallLog, header.c_str(),header.length(),&NumberOfBytesWriten,NULL);

			if(!hr)
				PG_LogERR("startDocument: Can't write to %s", infoexpander->IL_Filename.c_str());    
		}// else INVALID_HANDLE_VALUE
	}// if
	//Maindir in log
	if(infoexpander->InstallLog)
	{
		HRESULT hr = 0;
		DWORD NumberOfBytesWriten = 0;

		std::string xmlstring;
		//XML
		if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<dir =\"" + infoexpander->MainDir + "\"\\>\\r\n";
		//UNWISE
		if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "Made Dir: " + infoexpander->MainDir + "\r\n";

		hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
		if(!hr)
			PG_LogERR("shortcut: Can't write to %s", infoexpander->IL_Filename.c_str());    
	}
}

// Called at the beginning of an element <element>
/*  Create the SAXElement, call the correct LoadX
*/
void Installer::startElement(const XMLCh* const name, AttributeList& attributes)
{
	HRESULT     hr;
	char *elementName = XMLString::transcode(name);
	PG_LogDBG(elementName);

	if(stricmp(elementName, "component") == 0)
	{
		std::string name, versiontype, versiondata, i_default;
		for(unsigned int i = 0; i < attributes.getLength(); i++)
		{

			char *attributeName = XMLString::transcode(attributes.getName(i));
			if(stricmp(attributeName, "name") == 0)
			{
				name = XMLString::transcode(attributes.getValue(attributeName));
				CurrentComponentParse = name;
			}
			if(stricmp(attributeName, "versiontype") == 0)
			{
				versiontype = XMLString::transcode(attributes.getValue(attributeName));
			}
			if(stricmp(attributeName, "versiondata") == 0)
			{
				versiondata = XMLString::transcode(attributes.getValue(attributeName));
			}
			else if(stricmp(attributeName, "default") == 0)
			{
				i_default = XMLString::transcode(attributes.getValue(attributeName));
				if(i_default == "true") IsCurrentComponentInstall = true;
				//else IsCurrentComponentInstall = false;
				std::string a = "cmp_" + name;
				PG_RadioButton *rb = NULL;
				rb = (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction(a.c_str());
				if(rb)
					if(rb->GetPressed()) IsCurrentComponentInstall = true;
					else IsCurrentComponentInstall = false;
                 
                if(versiontype == "MajorVersion")
				{
					if(infoexpander->versionInfo.dwMajorVersion == StrToInt(versiondata.c_str()))
						IsCurrentComponentInstall = true;
				}
				else if(versiontype == "MinorVersion")
				{
					if(infoexpander->versionInfo.dwMinorVersion == StrToInt(versiondata.c_str()))
						IsCurrentComponentInstall = true;
				}
				else if(versiontype == "BuiltNumber")
				{
					if(infoexpander->versionInfo.dwBuildNumber == StrToInt(versiondata.c_str()))
						IsCurrentComponentInstall = true; 
				}
				else if(versiontype == "PlatformID")
				{
					if(versiondata == "VER_PLATFORM_WIN32s")
					{
						if(infoexpander->versionInfo.dwPlatformId == VER_PLATFORM_WIN32s)
							IsCurrentComponentInstall = true; 
					}
					else if(versiondata == "VER_PLATFORM_WIN32_WINDOWS")
					{
						if(infoexpander->versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
							IsCurrentComponentInstall = true; 
					}
					if(versiondata == "VER_PLATFORM_WIN32_NT")
					{
						if(infoexpander->versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
							IsCurrentComponentInstall = true; 
					}
					
				}

			}
		}

	}
	if(IsCurrentComponentInstall)
	{
		if(stricmp(elementName, "archive") == 0)
		{
			std::string type, srcpath, srcpath_not_exp, destpath, disk;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
		 {

			 char *attributeName = XMLString::transcode(attributes.getName(i));
			 if(stricmp(attributeName, "type") == 0)
			 {
				 type = XMLString::transcode(attributes.getValue(attributeName));
				 //srcpath = srcpath_not_exp;
				 //infoexpander->ExpandString(&srcpath,&srcpath);

			 }else	if(stricmp(attributeName, "source") == 0)
				{
					srcpath_not_exp = XMLString::transcode(attributes.getValue(attributeName));
					//srcpath = srcpath_not_exp;
					//infoexpander->ExpandString(&srcpath,&srcpath);

				}
			 else if(stricmp(attributeName, "destination") == 0)
				{
					destpath = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&destpath,&destpath);
				}
			 else if(stricmp(attributeName, "disk") == 0)
			 {
				 disk = XMLString::transcode(attributes.getValue(attributeName));
			 }
		 }
		 if(type == "rar" || type == "")
			 while(TRUE)
			 {
				 srcpath = srcpath_not_exp;
				 infoexpander->ExpandString(&srcpath,&srcpath);
				 if(ExtractArchive((LPSTR)srcpath.c_str(), EXTRACT, (LPSTR)destpath.c_str()) == ERAR_EOPEN)
				 {
					 PG_RadioButton *eng = NULL;
					 eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");

					 size_t startf = srcpath.find_last_of("\\",srcpath.length());
		             srcpath.replace(0,startf+1,"");
					 if(eng && eng->GetPressed())
					  {
						  std::string message = "Need file "+srcpath+".\nInsert disk № "+disk+" and press ОК.\nor press Cancel for skip.";
						  std::string title = "Insert disk № " + disk;
						  if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						  {   
							  Successfull = FALSE;
							  break;
						  }

					  }
					  else
					  {
						  std::string message = "Для продолжения нужен файл "+srcpath+".\nВставьте диск № "+disk+" и нажмите ОК.\nЧтобы пропустить нажмите Отмена.";
						  std::string title = "Вставте диск № " + disk;
						  if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						  {   
							  Successfull = FALSE;
							  break;
						  }
					  }

				 }
				 else break;
			 }
		 else if(type == "ace")
			 while(TRUE)
			 {
				 srcpath = srcpath_not_exp;
				 infoexpander->ExpandString(&srcpath,&srcpath);
				 destpath += "\\";
				 char *argv[] = {"","x",(LPSTR)srcpath.c_str(),(LPSTR)destpath.c_str()};
				 if(ExtractAceArchive(4, argv) == ACE_ERROR_OPEN)
				 {
					 PG_RadioButton *eng = NULL;
					 eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");

					 size_t startf = srcpath.find_last_of("\\",srcpath.length());
					 srcpath.replace(0,startf+1,"");
					 if(eng && eng->GetPressed())
					 {
						 std::string message = "Need file "+srcpath+".\nInsert disk № "+disk+" and press ОК.\nor press Cancel for skip.";
						 std::string title = "Insert disk № " + disk;
						 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						 {   
							 Successfull = FALSE;
							 break;
						 }

					 }
					 else
					 {
						 std::string message = "Для продолжения нужен файл "+srcpath+".\nВставьте диск № "+disk+" и нажмите ОК.\nЧтобы пропустить нажмите Отмена.";
						 std::string title = "Вставте диск № " + disk;
						 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						 {   
							 Successfull = FALSE;
							 break;
						 }
					 }

				 }
				 else break;
			 }    
		 else if(type == "cab")
			 while(TRUE)
			 {

				 srcpath = srcpath_not_exp;
				 infoexpander->ExpandString(&srcpath,&srcpath);
                 ExtractCabArchive((LPSTR)srcpath.c_str(), EXTRACT, (LPSTR)((destpath + "\\").c_str()));
				 if(0)//ExtractCabArchive((LPSTR)srcpath.c_str(), EXTRACT, (LPSTR)destpath.c_str()) == ERAR_EOPEN)
				 {
					 PG_RadioButton *eng = NULL;
					 eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");

					 size_t startf = srcpath.find_last_of("\\",srcpath.length());
					 srcpath.replace(0,startf+1,"");
					 if(eng && eng->GetPressed())
					 {
						 std::string message = "Need file "+srcpath+".\nInsert disk № "+disk+" and press OK.\nor press Cancel for skip.";
						 std::string title = "Insert disk ¦ " + disk;
						 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						 {   
							 Successfull = FALSE;
							 break;
						 }

					 }
					 else
					 {
						 std::string message = "Для продолжения нужен файл "+srcpath+".\nВставьте диск № "+disk+" и нажмите ОК.\nЧтобы пропустить нажмите Отмена.";
						 std::string title = "Вставте диск № " + disk;
						 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						 {   
							 Successfull = FALSE;
							 break;
						 }
					 }

				 }
				 else break;
			 }
		 else if(type == "ogg")
			 while(TRUE)
			 {

				 std::string outputpath;
				 srcpath = srcpath_not_exp;
				 infoexpander->ExpandString(&srcpath,&srcpath);
				 //char *Argum[] = {"","-Q",(LPSTR)srcpath.c_str()};
				 outputpath = destpath;
				 DWORD lastslash = 0;
				 DWORD startoff = 0;
				 for(UINT i = 0; i < outputpath.length(); i++){
					 lastslash = outputpath.find("\\",startoff);
					 if(lastslash != outputpath.npos)
						 startoff = lastslash + 1;
					 else {
						 lastslash = startoff - 1;
						 break;
					 }
				 }
				 		 
				 if (lastslash != outputpath.npos) 
				 outputpath = outputpath.replace(lastslash+1,destpath.length(),"");
				 CreateDirectoryExEx(outputpath.c_str());
				 infoexpander->SetCurrentOperation(destpath);

				 if(ExtractOggArchive((LPSTR)srcpath.c_str(),0,(LPSTR)destpath.c_str()) == 1)				 
				 {
					 PG_RadioButton *eng = NULL;
					 eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");



					 size_t startf = srcpath.find_last_of("\\",srcpath.length());
					 srcpath.replace(0,startf+1,"");
					 if(eng && eng->GetPressed())
					 {
						 std::string message = "Need file "+srcpath+".\nInsert disk № "+disk+" and press OK.\nor press Cancel for skip.";
						 std::string title = "Insert disk ¦ " + disk;
						 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						 {   
							 Successfull = FALSE;
							 break;
						 }

					 }
					 else
					 {
						 std::string message = "Для продолжения нужен файл "+srcpath+".\nВставьте диск № "+disk+" и нажмите ОК.\nЧтобы пропустить нажмите Отмена.";
						 std::string title = "Вставте диск № " + disk;
						 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
						 {   
							 Successfull = FALSE;
							 break;
						 }
					 }

				 }
				 else {
					 //ArchiveUnpacked += GetFileSize()
                     
					 break;
				 }
			 }
			 else if(type == "7zip")
					 while(TRUE)
					 {

						 srcpath = srcpath_not_exp;
						 infoexpander->ExpandString(&srcpath,&srcpath);

						 if(Extract7zipArchive((LPSTR)srcpath.c_str(),0,(LPSTR)destpath.c_str()) == E7ZIP_EOPEN)
						 {
							 PG_RadioButton *eng = NULL;
							 eng	= (PG_RadioButton*)handler->GetWizard()->GetWidgetByAction("cmp_english");

							 size_t startf = srcpath.find_last_of("\\",srcpath.length());
							 srcpath.replace(0,startf+1,"");
							 if(eng && eng->GetPressed())
							 {
								 std::string message = "Need file "+srcpath+".\nInsert disk № "+disk+" and press OK.\nor press Cancel for skip.";
								 std::string title = "Insert disk ¦ " + disk;
								 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
								 {   
									 Successfull = FALSE;
									 break;
								 }

							 }
							 else
							 {
								 std::string message = "Для продолжения нужен файл "+srcpath+".\nВставьте диск № "+disk+" и нажмите ОК.\nЧтобы пропустить нажмите Отмена.";
								 std::string title = "Вставте диск № " + disk;
								 if(MessageBox( infoexpander->hWndMain, message.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION ) == IDCANCEL)
								 {   
									 Successfull = FALSE;
									 break;
								 }
							 }

						 }
						 else break;
					 }

		}
		else if(stricmp(elementName, "execute") == 0)
		{
			std::string target, workdir, cmd;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "target") == 0)
				{
					target = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&target,&target);

				}
				else if(stricmp(attributeName, "workdir") == 0)
				{
					workdir = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&workdir,&workdir);
				}
				else if(stricmp(attributeName, "cmd") == 0)
				{
					cmd = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&cmd,&cmd);
				}
			}//for

			static STARTUPINFO inst_si;
            static PROCESS_INFORMATION inst_pi;

			inst_si.cb = sizeof(inst_si);
	        inst_si.dwFlags = STARTF_USESHOWWINDOW;
	        inst_si.wShowWindow = SW_SHOWNORMAL;
		
	        if(!CreateProcess(target.c_str(),(LPSTR
				)cmd.c_str(),NULL,NULL,false,CREATE_DEFAULT_ERROR_MODE,NULL,workdir.c_str(),&inst_si,&inst_pi))
	        {
				MessageBox( infoexpander->hWndMain, "execute: Немогу запустить файл.", "Ошибка", MB_ICONERROR);
				PG_LogERR("execute:error: can't execute file %s %s (workdir = %s)", target.c_str(), cmd.c_str(), workdir.c_str());
	        }
			else
			{
			  PG_LogERR("execute: execute file %s %s (workdir = %s)", target.c_str(), cmd.c_str(), workdir.c_str());
	          WaitForSingleObject(inst_pi.hProcess,INFINITE);
			}


		}//else if execute
		else if(stricmp(elementName, "delete") == 0)
		{
			std::string target;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "target") == 0)
				{
					target = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&target,&target);

				}
			}//for
			
			DeleteFile(target.c_str());
	        
		}//else if delete
		else if(stricmp(elementName, "move") == 0)
		{
			std::string target, destination;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "target") == 0)
				{
					target = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&target,&target);

				}
				else if(stricmp(attributeName, "destination") == 0)
				{
					destination = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&destination,&destination);

				}
			}//for

			if(!MoveFile(target.c_str(), destination.c_str()))
				PG_LogERR("MoveFile: error moving (%s to %s) = %d", target.c_str(), destination.c_str(), GetLastError());
			else
				PG_LogMSG("MoveFile: moving (%s to %s)", target.c_str(), destination.c_str());
    }//else if move
		else if(stricmp(elementName, "addfont") == 0)
		{
			std::string target, dest;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "target") == 0)
				{
					target = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&target,&target);

				}
				if(stricmp(attributeName, "dest") == 0)
				{
					dest = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&dest,&dest);

				}
			}//for

			if(!CopyFile(target.c_str(),dest.c_str(), FALSE)) 
				//MessageBox(infoexpander->hWndMain, "AddFont:CopyFile: Не могу скопировать файл.", "Error", MB_ICONERROR);
				PG_LogERR("AddFont:CopyFile: Не могу скопировать файл.");
 			else if(!AddFontResource(dest.c_str()))
                //MessageBox(infoexpander->hWndMain, "AddFont:AFR: Не могу установить шрифт.", "Error", MB_ICONERROR);     
				PG_LogERR("AddFont:AFR: Не могу установить шрифт.");
            else SendMessageA(HWND_BROADCAST, WM_FONTCHANGE, 0,0);
						
		

		}//else if addfont
		else if(stricmp(elementName, "shortcut") == 0)
		{
			//making Group
			std::string grouppath = "%CSIDL_PROGRAMS%\\" + infoexpander->Title;
			infoexpander->ExpandString(&grouppath, &grouppath);
			CreateDirectory(grouppath.c_str(), NULL);
			//LPCSTR aaaaaaaaa = grouppath.c_str();

			if(infoexpander->InstallLog)
			{
				HRESULT hr = 0;
				DWORD NumberOfBytesWriten = 0;

				std::string xmlstring;
				//XML
				if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<dir =\"" + grouppath + "\"\\>\\r\n";
				//UNWISE
				if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "Made Dir: " + grouppath + "\r\n";

				hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
				if(!hr)
					PG_LogERR("shortcut: Can't write to %s", infoexpander->IL_Filename.c_str());    
			}

			std::string target,destpath, workdir, cmd, iconpath; DWORD iconid;
			std::wstring wdestpath;   
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "target") == 0)
				{
					target = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&target,&target);

				}
				else if(stricmp(attributeName, "destination") == 0)
				{
					destpath = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&destpath,&destpath);
					DWORD len = destpath.length();
					WCHAR *wc = new WCHAR[len*2 + 1];
					MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, destpath.c_str(), len+1, wc, len*2+1);
					wdestpath = wc;
					delete wc;
				}
				else if(stricmp(attributeName, "workdir") == 0)
				{
					workdir = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&workdir,&workdir);
				}
				else if(stricmp(attributeName, "cmd") == 0)
				{
					cmd = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&cmd,&cmd);
				}
				else if(stricmp(attributeName, "iconpath") == 0)
				{
					iconpath = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&iconpath,&iconpath);
				}
				else if(stricmp(attributeName, "iconid") == 0)
				{
					iconid = StrToInt(XMLString::transcode(attributes.getValue(attributeName)));
				}
			}//for

			//Create shortcut
			CurrentOperation = destpath;

			IShellLink* pISL;
			IPersistFile* pIPF;

			CoInitialize(NULL);

			hr = CoCreateInstance ( CLSID_ShellLink,  // CLSID CO-класса
				NULL,    // агрегация не используется
				CLSCTX_INPROC_SERVER,  // тип сервера
				IID_IShellLink,   // IID интерфейса
				(void**) &pISL );  // Указатель на наш интерфейсный указатель

			if ( SUCCEEDED ( hr ) )
			{
				if (
					SUCCEEDED(hr = pISL->SetPath(target.c_str())) &&
					SUCCEEDED(hr = pISL->SetArguments(cmd.c_str())) &&
					SUCCEEDED(hr = pISL->SetWorkingDirectory(workdir.c_str())) &&
					SUCCEEDED(hr = pISL->SetIconLocation(iconpath.c_str(), iconid))
					)
				{
					// 4. Получение второго интерфейса (IPersistFile) от объекта COM.
					hr = pISL->QueryInterface ( IID_IPersistFile, (void**) &pIPF );
					if (SUCCEEDED(hr))
					{					
						hr = pIPF->Save ( wdestpath.c_str(), FALSE );
						//hr = pIPF->Save ( L"D:\\(temp)\\test\\Джеймс Бонд.lnk", FALSE );
						if (!SUCCEEDED(hr)) PG_LogWRN("Can't create %s",destpath.c_str());
						else PG_LogMSG("Creating shortcut %s", destpath.c_str());

						// 6a. Освобождение интерфейса IPersistFile.
						pIPF->Release();
					}//if
				}//if
				else PG_LogWRN("Object %s does not exist",target.c_str());
				// Здесь можно вызывать методы, используя pISL.
			}//if
			else
			{
				// Невозможно создать объект COM. hr присвоен код ошибки.
			}//else
			pISL->Release();
			CoUninitialize();

			if(infoexpander->InstallLog)
			{
				HRESULT hr = 0;
				DWORD NumberOfBytesWriten = 0;

				std::string xmlstring;
				//XML
				if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<file path =\"" + destpath + "\"\\>\\r\n";
				//UNWISE
				if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "File Copy: " + destpath + "\r\n";

				hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
				if(!hr)
					PG_LogERR("shortcut: Can't write to %s", infoexpander->IL_Filename.c_str());    
			}

		}//else if shortcut
		else if(stricmp(elementName, "registry") == 0)
		{
			std::string srootkey, key, valuename, data; DWORD type;
			HKEY rootkey;

			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "rootkey") == 0)
				{
					if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_CLASSES_ROOT") == 0)
						rootkey = HKEY_CLASSES_ROOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_CURRENT_CONFIG") == 0)
						rootkey = HKEY_CURRENT_CONFIG;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_CURRENT_USER") == 0)
						rootkey = HKEY_CURRENT_USER;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_LOCAL_MACHINE") == 0)
						rootkey = HKEY_LOCAL_MACHINE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_USERS") == 0)
						rootkey = HKEY_USERS;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_PERFORMANCE_DATA") == 0)
						rootkey = HKEY_PERFORMANCE_DATA;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "HKEY_DYN_DATA") == 0)
						rootkey = HKEY_DYN_DATA;
					srootkey = XMLString::transcode(attributes.getValue(attributeName));
				}
				else if(stricmp(attributeName, "key") == 0)
				{
					key = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&key, &key);
				}
				else if(stricmp(attributeName, "valuename") == 0)
				{
					valuename = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&valuename, &valuename);
				}
				else if(stricmp(attributeName, "data") == 0)
				{
					//std::string expanded_str = XMLString::transcode(attributes.getValue(attributeName));
					data = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&data, &data);
				}
				else if(stricmp(attributeName, "type") == 0)
				{				
					if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_BINARY") == 0)
						type = REG_BINARY;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_DWORD") == 0)
						type = REG_DWORD;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_SZ") == 0)
						type = REG_SZ;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_DWORD_LITTLE_ENDIAN") == 0)
						type = REG_DWORD_LITTLE_ENDIAN;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_DWORD_BIG_ENDIAN") == 0)
						type = REG_DWORD_BIG_ENDIAN;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_EXPAND_SZ") == 0)
						type = REG_EXPAND_SZ;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_LINK") == 0)
						type = REG_LINK;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_MULTI_SZ") == 0)
						type = REG_MULTI_SZ;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_NONE") == 0)
						type = REG_NONE;
					//	else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_QWORD") == 0)
					//	   type = REG_QWORD;
					//	else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_QWORD_LITTLE_ENDIAN") == 0)
					//	   type = REG_QWORD_LITTLE_ENDIAN;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "REG_RESOURCE_LIST") == 0)
						type = REG_RESOURCE_LIST;
				}//else if
			}//for

			//Create registry
			std::string backslash = "\\"; 
			std::string fullkey;
			fullkey += srootkey;
			fullkey += backslash;
			fullkey += key;
			fullkey += backslash;
			fullkey += valuename;
			CurrentOperation = fullkey;

			HKEY hk;

			hr = RegCreateKeyEx(rootkey,key.c_str(),NULL,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&hk,NULL);
			if(SUCCEEDED(hr))
			{
				switch(type)
				{
				case REG_BINARY: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
				case REG_DWORD:
					DWORD dwdata;
					sscanf(data.c_str(),"%Ld",&dwdata);
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)&dwdata,sizeof(DWORD));
					break;
				case REG_SZ: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
					//		case REG_DWORD_LITTLE_ENDIAN: 
					//			hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					//			break;
				case REG_DWORD_BIG_ENDIAN: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
				case REG_EXPAND_SZ:
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
				case REG_LINK: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
				case REG_MULTI_SZ: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
				case REG_NONE: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
					//	case REG_QWORD: 
					//		hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					//		break;
					//	case REG_QWORD_LITTLE_ENDIAN: 
					//		hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					//		break;
				case REG_RESOURCE_LIST: 
					hr = RegSetValueEx(hk,valuename.c_str(),NULL,type, (LPBYTE)data.c_str(),data.size());
					break;
				default:
					break;
				}//switch
				if(!SUCCEEDED(hr))
					PG_LogERR("Can't create key %s", valuename.c_str());
				else
				{
					PG_LogMSG("Creating key %s", fullkey.c_str());
					if(infoexpander->InstallLog)
					{
						//RegDB Key: Software\Fargus\NFSHP2
						//RegDB Val: NFSHP2
						//RegDB Name: Title
						//RegDB Type: 0
						//RegDB Root: 2

						HRESULT hr = 0;
						DWORD NumberOfBytesWriten = 0;

						std::string xmlstring;
						//XML
						//if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<dir =\"" + infoexpander->MainDir + "\"\\>\\r\n";
						//UNWISE
						if(infoexpander->InstallLogType == ILT_UNWISE)
						{
							xmlstring = "RegDB Key: " + key + "\r\n"
								+ "RegDB Val: " + data + "\r\n" 
								+ "RegDB Name: " + valuename + "\r\n"
								+ "RegDB Type: " + "0" + "\r\n"
								+ "RegDB Root: " + "2" + "\r\n";
						}

						hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
						if(!hr)
							PG_LogERR("registry: Can't write to %s", infoexpander->IL_Filename.c_str());    
					}//if loging
				}//else

			}//if
			else
				PG_LogERR("Can't create key %s", valuename.c_str());
			RegCloseKey(hk);

		}//else if registry
		else if(stricmp(elementName, "inifile") == 0)
		{
			std::string srcpath, destpath, type;

			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "source") == 0)
				{
					srcpath = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&srcpath,&srcpath);    
				}
				else if(stricmp(attributeName, "destination") == 0)
				{
					destpath = XMLString::transcode(attributes.getValue(attributeName));
					infoexpander->ExpandString(&destpath,&destpath);    
				}
				else if(stricmp(attributeName, "type") == 0)
				{
					type = XMLString::transcode(attributes.getValue(attributeName));
				}

			}//for

			if(type == "file")
			{
				char* filebuff = NULL;
				LARGE_INTEGER filesize;
				DWORD NumberOfBytesRead = 0;
				HANDLE FileToExpand = NULL;

				std::string filestring;

				//open source file

				FileToExpand = CreateFile(srcpath.c_str(),GENERIC_READ,NULL,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
				if(FileToExpand == INVALID_HANDLE_VALUE)
					PG_LogERR("Can't open file '%s'", srcpath.c_str());
				else{
					HRESULT hr = 0; 

					//check for GetFileSizeEx
					if(pGetFileSizeEx)
						pGetFileSizeEx(FileToExpand, &filesize);
					else
						filesize.LowPart = GetFileSize(FileToExpand, NULL);

					//mem for reading file
					filebuff = new char[filesize.LowPart + 1];
					if(!filebuff)
						PG_LogERR("Not enough memory for file '%s'", srcpath.c_str());
					else{
						memset(filebuff,0, filesize.LowPart + 1); 
						hr = ReadFile(FileToExpand, filebuff, filesize.LowPart, &NumberOfBytesRead, NULL);
						if(!hr)
							PG_LogERR("Can't read file '%s'", srcpath.c_str());
						else{
							DWORD NumberOfBytesWriten = 0;
							HANDLE FileToWrite = NULL;

							//copy file to std::string
							filestring = filebuff;

							//expand file
							infoexpander->ExpandString(&filestring,&filestring);

							//create new file
							FileToWrite = CreateFile(destpath.c_str(),GENERIC_WRITE,NULL,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
							if(FileToWrite == INVALID_HANDLE_VALUE)
								PG_LogERR("Can't create file '%s'", destpath.c_str());
							else{
								hr = WriteFile(FileToWrite, filestring.c_str(),filestring.length(),&NumberOfBytesWriten,NULL);
								if(!hr)
									PG_LogERR("Can't write file '%s'", destpath.c_str());
							}
							if(FileToWrite) CloseHandle(FileToWrite);

						}

					}
					//release
					if(filebuff) delete filebuff;
					if(FileToExpand) CloseHandle(FileToExpand);

				}
				if(infoexpander->InstallLog)
				{
					HRESULT hr = 0;
					DWORD NumberOfBytesWriten = 0;

					std::string xmlstring;
					//XML
					if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<file path =\"" + destpath + "\"\\>\\r\n";
					//UNWISE
					if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "File Copy: " + destpath + "\r\n";

					hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
					if(!hr)
						PG_LogERR("inifile: Can't write to %s", infoexpander->IL_Filename.c_str());    
				}
			}


		}//else if ini
	}//if component need to install


}//SDLInstallerLayout::startElement



// -----------------------------------------------------------------------
//  Implementations of the SAX ErrorHandler interface
// -----------------------------------------------------------------------
// warning exception handler
void Installer::warning(const SAXParseException& exception)
{
	m_warnings++;
	PG_LogWRN("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}
// error exception handler
void Installer::error(const SAXParseException& exception)
{
	m_errors++;
	PG_LogERR("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}
// fatal error exception handler
void Installer::fatalError(const SAXParseException& exception)
{
	PG_LogERR("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}

BOOL GetState(INSTALLSTATE &is)
{
	is.Progress = ArchiveProgress;
	is.CurrentOperationProgress = FileProgress;
	is.CurrentOperation = CurrentOperation;
	return TRUE;
}

BOOL CreateDirectoryExEx(LPCSTR path)
{
	LPSTR tmp = new char[strlen(path) + 1];
	strcpy(tmp, path);

	for(UINT i = 3; i < strlen(path); i++)
	{
		if(*(path + i) == '\\')
		{
			*(tmp + i) = 0;
			CreateDirectory(tmp, NULL);
			strcpy(tmp, path);
		}
	}

	CreateDirectory(path, NULL);
	delete tmp;
	return TRUE;
}