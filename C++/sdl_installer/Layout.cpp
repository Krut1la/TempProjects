#include "layout.h"
#include "infoexpander.h"
#include <Shlwapi.h>
#include <pglog.h>


SDLInstallerLayout *handler;
extern InfoExpander *infoexpander;

//temp!!!
extern PG_Application app;
bool isLMouseDown = false;



BOOL LoadLayout( LPCSTR XMLfilename , float KoeffX, float KoeffY)
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
	handler = new SDLInstallerLayout(XMLfilename, KoeffX, KoeffY);
	try
	{
		parser.setDocumentHandler(handler);
		parser.setErrorHandler(handler);
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

// constructor
SDLInstallerLayout::SDLInstallerLayout(LPCSTR XMLfilename, float KoeffX, float KoeffY)
{
	SDLInstallerLayout::XMLfilename = XMLfilename;

	m_warnings = 0;
	m_errors = 0;
	FKoeffX = KoeffX;
	FKoeffY = KoeffY;
	CurrentPageElement = NULL;
	ParsePageElementInProgress = false;
	Wizard = new SDLInstallerWizard();
	DynamicControls = new SDL_DynamicControlsRefresh();
	ParseRBGroupInProgress = false;
	ParseLocalisationInProgress = false;
	CurrentRBGroup = NULL;
}

//! destructor
SDLInstallerLayout::~SDLInstallerLayout()
{
	delete Wizard;
	delete DynamicControls;
}

// -----------------------------------------------------------------------
//  Implementations of the SAX DocumentHandler interface
// -----------------------------------------------------------------------

// Called at the end of the XML document
/* Inform the user of any errors
*/
void SDLInstallerLayout::endDocument()
{
	PG_LogMSG("End parsing XML document: %s warnings %d errors %d",XMLfilename.c_str(), m_warnings,m_errors);

	//check CPU
	PG_Widget *CPUPlus = handler->GetWizard()->GetWidgetByName("CPUPlus");
	PG_Widget *CPUMinus = handler->GetWizard()->GetWidgetByName("CPUMinus");

	if(CPUPlus&&CPUMinus)if(infoexpander->CPUMHZ >= infoexpander->CPUMHZNeed)
	{
		CPUPlus->SetTransparency(0);
		CPUMinus->SetTransparency(255);
	}
	else
	{
		CPUPlus->SetTransparency(255);
		CPUMinus->SetTransparency(0);
	}
	//check RAM
	PG_Widget *RAMPlus = handler->GetWizard()->GetWidgetByName("RAMPlus");
	PG_Widget *RAMMinus = handler->GetWizard()->GetWidgetByName("RAMMinus");

	if(RAMPlus&&RAMMinus)if(infoexpander->RAMMB >= infoexpander->RAMNeed)
	{
		RAMPlus->SetTransparency(0);
		RAMMinus->SetTransparency(255);
	}
	else
	{
		RAMPlus->SetTransparency(255);
		RAMMinus->SetTransparency(0);
	}

	//check Video
	PG_Widget *VideoPlus = handler->GetWizard()->GetWidgetByName("VideoPlus");
	PG_Widget *VideoMinus = handler->GetWizard()->GetWidgetByName("VideoMinus");
	PG_Widget *VideoQues = handler->GetWizard()->GetWidgetByName("VideoQues");

	if(infoexpander->VideoGenerationNeed == "GeForce2")
	{
			std::string radeon  = "RADEON";
			std::string geforce = "GeForce";
			DWORD posGeForce = infoexpander->VideoString.find("GeForce");
			DWORD posRadeon = infoexpander->VideoString.find("RADEON");
			if(VideoPlus&&VideoMinus&&VideoQues)if(posGeForce != infoexpander->VideoString.npos || posRadeon != infoexpander->VideoString.npos) 
			{
			   VideoPlus->SetTransparency(0);
		       VideoQues->SetTransparency(255);
			   VideoMinus->SetTransparency(255);
	        }
	        else
	        {
		       VideoPlus->SetTransparency(255);
		       VideoQues->SetTransparency(0);
			   VideoMinus->SetTransparency(255);
	        }
		    
 	}

	//check DX
	PG_Widget *BDX = handler->GetWizard()->GetWidgetByAction("DX");
	if(BDX)if(infoexpander->DXVersion >= infoexpander->DXVersionNeed)
	{
		BDX->EnableReceiver(false);
	}
	else
	{
		BDX->EnableReceiver(true);
	}

	//there is nothing to cancel
	PG_Widget *cancel = handler->GetWizard()->GetWidgetByAction("cancel");
	if(cancel) 
	{
		cancel->EnableReceiver(false);
		cancel->SetFontAlpha(100);
    }

	PG_Widget *start = handler->GetWizard()->GetWidgetByName("BStart");
	if(start) 
	{
		start->EnableReceiver(false);
		start->SetFontAlpha(100);
		SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_START), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_START),GWL_STYLE)|WS_DISABLED);
	}
	PG_Widget *uninstall = handler->GetWizard()->GetWidgetByName("BUninstall");
	if(uninstall) 
	{
		uninstall->EnableReceiver(false);
		uninstall->SetFontAlpha(100);
		SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_UNINSTALL), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_UNINSTALL),GWL_STYLE)|WS_DISABLED);
	}

	//SetText of pathedit to default maindir
	if(infoexpander->MainDir.length()>=3)
	{
		std::string MainDrive = infoexpander->MainDir;
		MainDrive.erase(3,MainDrive.length());
		infoexpander->MainDrive = MainDrive;
	}
	PG_Widget *editpath = handler->GetWizard()->GetWidgetByName("Path");
	if(editpath) editpath->SetText(infoexpander->MainDir.c_str());

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
			   SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_START), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_START),GWL_STYLE)&~WS_DISABLED);
	        }
		}

		//clear
		//if(filename) CloseHandle(filename);
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
			   SetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_BUTTON_UNINSTALL), GWL_STYLE, GetWindowLong(GetDlgItem(infoexpander->hwndEasyInstaller,IDC_BUTTON_UNINSTALL),GWL_STYLE)&~WS_DISABLED);
			}
		}
	}
	//if(filename) CloseHandle(filename);
	FindClose(filename);

	RegCloseKey(hk);
	delete lpFindFileData;

}

// Called at the end of an element </element>
/* Based on type, add to parent
*/
void SDLInstallerLayout::endElement(const XMLCh* const name)
{
	char *elementName;
	elementName = XMLString::transcode(name);

	if(stricmp(elementName, "page") == 0)
	{
		ParsePageElementInProgress = false;
		Wizard->AddPage(CurrentPage);
		infoexpander->CountOfWizardPages++;
		//delete CurrentPageElement;
	}
	else if(stricmp(elementName, "screenshot") == 0)
	{
		ParsePageElementInProgress = false;
		Wizard->AddScreenShot(CurrentPage);
		infoexpander->CountOfScreenShots++;
		//delete CurrentPageElement;
	}
	else if(stricmp(elementName, "announce") == 0)
	{
		ParsePageElementInProgress = false;
		Wizard->AddAnnounce(CurrentPage);
		infoexpander->CountOfAnnounces++;
		//delete CurrentPageElement;
	}
	else if(stricmp(elementName, "frame") == 0)
	{
		ParsePageElementInProgress = false;
		Wizard->AddFrame(CurrentPage);
		//delete CurrentPageElement;
	}
	else if(stricmp(elementName, "rbgroup") == 0)
	{
		ParseRBGroupInProgress = false;
		CurrentRBGroup = NULL;
	}
	else if(stricmp(elementName, "localisation") == 0)
	{
		ParseLocalisationInProgress = false;
//		CurrentLocalisation = NULL;
	}

	//delete elementName;
	return;
}

// Called when characters exist between <element> and </element>
/* if element is a type that accepts characters as text, call SetText
*/
void SDLInstallerLayout::characters(const XMLCh* const chars, const unsigned int length)
{

}

// Called at the beginning of the document
/* Initialize and clear the stack
*/
void SDLInstallerLayout::startDocument()
{
	PG_LogMSG("Start parsing XML document: %s\n", XMLfilename.c_str());
	m_warnings = 0;
	m_errors = 0;
}

// Called at the beginning of an element <element>
/*  Create the SAXElement, call the correct LoadX
*/
void SDLInstallerLayout::startElement(const XMLCh* const name, AttributeList& attributes)
{
	char *elementName;
	elementName = XMLString::transcode(name);

	PG_LogDBG(elementName);

	if(stricmp(elementName, "page") == 0)
	{
		if(!ParsePageElementInProgress) 
		{
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SDLInstallerPage* page = new SDLInstallerPage(tmp);
					CurrentPage = page;

					std::string tmp2 = tmp;
					CurrentPageElement = tmp2.c_str();
					ParsePageElementInProgress = true;

				}
				else if(stricmp(attributeName, "index") == 0)
				{
					if(CurrentPage)
					{
						CurrentPage->SetIndex(StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
					}
				}
				//delete attributeName;
			}
		}
		else
			PG_LogERR("trying create new page before page '%s' is complete", CurrentPage->GetName());
	}
	else if(stricmp(elementName, "screenshot") == 0)
	{
		if(!ParsePageElementInProgress) 
		{
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SDLInstallerPage* page = new SDLInstallerPage(tmp);
					CurrentPage = page;

					std::string tmp2 = tmp;
					CurrentPageElement = tmp2.c_str();
					ParsePageElementInProgress = true;

				}
				else if(stricmp(attributeName, "index") == 0)
				{
					if(CurrentPage)
					{
						CurrentPage->SetIndex(StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
					}
				}
				//delete attributeName;
			}
		}
		else
			PG_LogERR("trying create new page before page '%s' is complete", CurrentPage->GetName());
	}
	else if(stricmp(elementName, "announce") == 0)
	{
		if(!ParsePageElementInProgress) 
		{
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SDLInstallerPage* page = new SDLInstallerPage(tmp);
					CurrentPage = page;

					std::string tmp2 = tmp;
					CurrentPageElement = tmp2.c_str();
					ParsePageElementInProgress = true;

				}
				else if(stricmp(attributeName, "index") == 0)
				{
					if(CurrentPage)
					{
						CurrentPage->SetIndex(StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
					}
				}
				//delete attributeName;
			}
		}
		else
			PG_LogERR("trying create new page before page '%s' is complete", CurrentPage->GetName());
	}
	else if(stricmp(elementName, "rbgroup") == 0)
	{
		if(!ParseRBGroupInProgress) 
		{
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					/*//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SDLInstallerPage* page = new SDLInstallerPage(tmp);
					CurrentPage = page;

					std::string tmp2 = tmp;
					CurrentPageElement = tmp2.c_str();
					*/

					ParseRBGroupInProgress = true;

				}
			}
		}
		else
			PG_LogERR("trying create new page before page '%s' is complete", CurrentPage->GetName());
	}
	else if(stricmp(elementName, "localisation") == 0)
	{
		if(!ParseLocalisationInProgress) 
		{
//			for(unsigned int i = 0; i < attributes.getLength(); i++)
//			{
//				char *attributeName = XMLString::transcode(attributes.getName(i));
//				if(stricmp(attributeName, "name") == 0)
//				{
					/*//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SDLInstallerPage* page = new SDLInstallerPage(tmp);
					CurrentPage = page;

					std::string tmp2 = tmp;
					CurrentPageElement = tmp2.c_str();
					*/

					ParseLocalisationInProgress = true;

//				}
//			}
		}
		else
			PG_LogERR("trying create new page before page '%s' is complete", CurrentPage->GetName());
	}
	else if(stricmp(elementName, "frame") == 0)
	{
		if(!ParsePageElementInProgress) 
		{
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					//get mem for page name string and save attributeName here
					LPSTR tmp = XMLString::transcode(attributes.getValue(attributeName));
					//create new page
					SDLInstallerPage* page = new SDLInstallerPage(tmp);
					CurrentPage = page;

					std::string tmp2 = tmp;
					CurrentPageElement = tmp2.c_str();
					ParsePageElementInProgress = true;

				}
				else if(stricmp(attributeName, "index") == 0)
				{
					if(CurrentPage)
					{
						CurrentPage->SetIndex(StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
					}
				}
				//delete attributeName;
			}
		}
		else
			PG_LogERR("trying create new page before page '%s' is complete", CurrentPage->GetName());
	}
	else if(stricmp(elementName, "staticframe") == 0)
	{
		if(ParsePageElementInProgress) 
		{             
			std::string s_name="\0", themestyle="\0", themeobject="\0";
			UINT x=0,y=0,w=0,h=0;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{			    
					s_name = XMLString::transcode(attributes.getValue(attributeName));
				}
				else if(stricmp(attributeName, "themeobject") == 0)
				{   
					themeobject = XMLString::transcode(attributes.getValue(attributeName));
				}
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle = XMLString::transcode(attributes.getValue(attributeName));
				}
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}
				//delete attributeName;
			}
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{
				
				/*if(s_name == "Background")
				{
      			   PG_Button *sf = new PG_Button(NULL,1, PG_Rect(x,y,w,h),"",themeobject.c_str());
				   //PG_MyButton *sf = new PG_MyButton(NULL, PG_Rect(x,y,w,h));
				   sf->LoadThemeStyle(themeobject.c_str(),themestyle.c_str());
				   sf->EnableReceiver(true);
				   CurrentPage->AddWidget(sf);
				   Wizard->AddWidget((PG_Widget*)sf, s_name, "");
				   
				}
				else
				{*/
				   PG_StaticFrame *sf = new PG_StaticFrame(NULL, PG_Rect(x,y,w,h));
				   sf->LoadThemeStyle(themeobject.c_str(),themestyle.c_str());
				   CurrentPage->AddWidget(sf);
				   Wizard->AddWidget((PG_Widget*)sf, s_name, "");
				//}

				
			}
			else {PG_LogERR("can't create widget '%s'", elementName); m_errors++;}
		}
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}
	/*  else if(stricmp(elementName, "show_screenshot") == 0)
	{
	if(ParsePageElementInProgress) 
	{             

	std::string s_name="\0";
	DWORD index = 1;
	for(unsigned int i = 0; i < attributes.getLength(); i++)
	{
	char *attributeName = XMLString::transcode(attributes.getName(i));
	if(stricmp(attributeName, "name") == 0)
	{
	s_name = XMLString::transcode(attributes.getValue(attributeName));
	}//if
	else if(stricmp(attributeName, "index") == 0)
	{			    
	index = StrToInt(XMLString::transcode(attributes.getValue(attributeName)));
	}
	}
	PG_StaticFrame *sf = NULL;            
	//sf = (PG_StaticFrame*)Wizard->GetScreenShot(index)->GetWidget(s_name.c_str());
	sf = (PG_StaticFrame*)Wizard->GetWidgetByName(s_name.c_str());
	if(sf)
	{
	CurrentPage->AddWidget(sf);
	}


	}
	else
	PG_LogERR("trying create widget '%s' without page", elementName);
	}
	*/
	else if(stricmp(elementName, "button") == 0)
	{
		if(ParsePageElementInProgress) 
		{  
			//vars for create PG_Button  
			std::string s_name="\0",s_action="\0",themeobject="\0",themestyle="\0",text="\0";
			DWORD baction = 0;
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			std::string pagename = "\0";
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "action") == 0)
				{
					s_action = XMLString::transcode(attributes.getValue(attributeName));
					if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_page") == 0)
						baction = HANDLE_PREVIOUS_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_page") == 0)
						baction = HANDLE_NEXT_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_page") == 0)
						baction = HANDLE_FIRST_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_page") == 0)
						baction = HANDLE_LAST_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "exit") == 0)
						baction = HANDLE_EXIT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "install") == 0)
						baction = HANDLE_INSTALL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "dx") == 0)
						baction = HANDLE_INSDX;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "uninstall") == 0)
						baction = HANDLE_UNINSTALL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "start") == 0)
						baction = HANDLE_START;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "browse") == 0)
						baction = HANDLE_BROWSE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "cancel") == 0)
						baction = HANDLE_CANCEL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "awlogo") == 0)
						baction = HANDLE_AWLOGO;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "WWW") == 0)
						baction = HANDLE_WWW;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "minimize") == 0)
						baction = HANDLE_MINIMIZE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "easy") == 0)
						baction = HANDLE_EASY;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_screenshot") == 0)
						baction = HANDLE_NEXT_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_screenshot") == 0)
						baction = HANDLE_PREVIOUS_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_screenshot") == 0)
						baction = HANDLE_FIRST_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_screenshot") == 0)
						baction = HANDLE_LAST_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_announce") == 0)
						baction = HANDLE_NEXT_ANNOUNCE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_announce") == 0)
						baction = HANDLE_PREVIOUS_ANNOUNCE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_announce") == 0)
						baction = HANDLE_FIRST_ANNOUNCE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_announce") == 0)
						baction = HANDLE_LAST_ANNOUNCE;

					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "requirements_page") == 0)
						baction = HANDLE_REQUIREMENTS_PAGE;

					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "description_page") == 0)
						baction = HANDLE_DESCRIPTION_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "screenshots_page") == 0)
						baction = HANDLE_SCREENSHOTS_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "cheats_page") == 0)
						baction = HANDLE_CHEATS_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "install_page") == 0)
						baction = HANDLE_INSTALL_PAGE;

					
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "text") == 0)
				{			    
					text = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					GetParamColor(XMLString::transcode(attributes.getValue(attributeName)), &tcolor);
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{					
				PG_Button *b = new PG_Button(NULL,1, PG_Rect(x,y,w,h),text.c_str(),themeobject.c_str());
				b->LoadThemeStyle(themeobject.c_str(),themestyle.c_str());
				b->SetFontSize(tsize);
				b->SetFontStyle(tstyle);
				b->SetFontColor(tcolor);
				b->SetFontAlpha(tcolor.unused);

				switch(baction)
				{
				case HANDLE_EXIT:                b->SetEventCallback(MSG_BUTTONCLICK, handle_exit, &app);
					break;
				case HANDLE_INSTALL:             b->SetEventCallback(MSG_BUTTONCLICK, handle_INSTALL, &app);
					break;
				case HANDLE_INSDX:               b->SetEventCallback(MSG_BUTTONCLICK, handle_INSDX, &app);
					break;
				case HANDLE_UNINSTALL:           b->SetEventCallback(MSG_BUTTONCLICK, handle_UNINSTALL, &app);
					break;
				case HANDLE_START:               b->SetEventCallback(MSG_BUTTONCLICK, handle_START, &app);
					break;
				case HANDLE_BROWSE:              b->SetEventCallback(MSG_BUTTONCLICK, handle_BROWSE, &app);
					break;
				case HANDLE_CANCEL:              b->SetEventCallback(MSG_BUTTONCLICK, handle_CANCEL, &app);
					break;
				case HANDLE_CHANGELANGR:         b->SetEventCallback(MSG_BUTTONCLICK, handle_CHANGELANGR, &app);
					break;
				case HANDLE_CHANGELANGE:         b->SetEventCallback(MSG_BUTTONCLICK, handle_CHANGELANGE, &app);
					break;
				case HANDLE_AWLOGO:              b->SetEventCallback(MSG_BUTTONCLICK, handle_AWLOGO, &app);
					break;
				case HANDLE_WWW:                 b->SetEventCallback(MSG_BUTTONCLICK, handle_WWW, &app);
					break;
				case HANDLE_MINIMIZE:            b->SetEventCallback(MSG_BUTTONCLICK, handle_MINIMIZE, &app);
					break;
				case HANDLE_EASY:                b->SetEventCallback(MSG_BUTTONCLICK, handle_EASY, &app);
					break;
				case HANDLE_NEXT_PAGE:           b->SetEventCallback(MSG_BUTTONCLICK, handle_next_page, &app);
					break;
				case HANDLE_PREVIOUS_PAGE:       b->SetEventCallback(MSG_BUTTONCLICK, handle_previous_page, &app);
					break;
				case HANDLE_FIRST_PAGE:          b->SetEventCallback(MSG_BUTTONCLICK, handle_first_page, &app);
					break;
				case HANDLE_LAST_PAGE:           b->SetEventCallback(MSG_BUTTONCLICK, handle_last_page, &app);
					break;
				case HANDLE_NEXT_SCREENSHOT:     b->SetEventCallback(MSG_BUTTONCLICK, handle_next_screenshot, &app);
					break;
				case HANDLE_PREVIOUS_SCREENSHOT: b->SetEventCallback(MSG_BUTTONCLICK, handle_previous_screenshot, &app);
					break;
				case HANDLE_FIRST_SCREENSHOT:    b->SetEventCallback(MSG_BUTTONCLICK, handle_first_screenshot, &app);
					break;
				case HANDLE_LAST_SCREENSHOT:     b->SetEventCallback(MSG_BUTTONCLICK, handle_last_screenshot, &app);
					break;
				case HANDLE_NEXT_ANNOUNCE:     b->SetEventCallback(MSG_BUTTONCLICK, handle_next_announce, &app);
					break;
				case HANDLE_PREVIOUS_ANNOUNCE: b->SetEventCallback(MSG_BUTTONCLICK, handle_previous_announce, &app);
					break;
				case HANDLE_FIRST_ANNOUNCE:    b->SetEventCallback(MSG_BUTTONCLICK, handle_first_announce, &app);
					break;
				case HANDLE_LAST_ANNOUNCE:     b->SetEventCallback(MSG_BUTTONCLICK, handle_last_announce, &app);
					break;

                case HANDLE_REQUIREMENTS_PAGE: b->SetEventCallback(MSG_BUTTONCLICK, handle_requirements_page, &app);
					break;
                case HANDLE_DESCRIPTION_PAGE:  b->SetEventCallback(MSG_BUTTONCLICK, handle_description_page, &app);
					break;
                case HANDLE_SCREENSHOTS_PAGE:  b->SetEventCallback(MSG_BUTTONCLICK, handle_screenshots_page, &app);
					break;
                case HANDLE_CHEATS_PAGE:       b->SetEventCallback(MSG_BUTTONCLICK, handle_cheats_page, &app);
					break;
                case HANDLE_INSTALL_PAGE:      b->SetEventCallback(MSG_BUTTONCLICK, handle_install_page, &app);
					break;  

                

				}


				CurrentPage->AddWidget(b);
				Wizard->AddWidget((PG_Widget*)b, s_name, s_action);
			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if button
	else if(stricmp(elementName, "checkbutton") == 0)
	{
		if(ParsePageElementInProgress) 
		{  
			//vars for create PG_CheckButton  
			std::string s_name="\0",s_action="\0",themeobject="\0",themestyle="\0",text="\0";
			DWORD baction = 0;
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "action") == 0)
				{
					s_action = XMLString::transcode(attributes.getValue(attributeName));
					if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_page") == 0)
						baction = HANDLE_PREVIOUS_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_page") == 0)
						baction = HANDLE_NEXT_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_page") == 0)
						baction = HANDLE_FIRST_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_page") == 0)
						baction = HANDLE_LAST_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "exit") == 0)
						baction = HANDLE_EXIT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "install") == 0)
						baction = HANDLE_INSTALL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "browse") == 0)
						baction = HANDLE_BROWSE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "cancel") == 0)
						baction = HANDLE_CANCEL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "awlogo") == 0)
						baction = HANDLE_AWLOGO;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "WWW") == 0)
						baction = HANDLE_WWW;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_screenshot") == 0)
						baction = HANDLE_NEXT_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_screenshot") == 0)
						baction = HANDLE_PREVIOUS_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_screenshot") == 0)
						baction = HANDLE_FIRST_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_screenshot") == 0)
						baction = HANDLE_LAST_SCREENSHOT;
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "text") == 0)
				{			    
					text = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					GetParamColor(XMLString::transcode(attributes.getValue(attributeName)), &tcolor);
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{					
				PG_CheckButton *cb = new PG_CheckButton(NULL,1, PG_Rect(x,y,w,h),text.c_str(),themeobject.c_str());
				cb->LoadThemeStyle(themeobject.c_str());
				cb->SetFontSize(tsize);
				cb->SetFontStyle(tstyle);
				cb->SetFontColor(tcolor);
				cb->SetFontAlpha(tcolor.unused);
				cb->SetPressed();


				CurrentPage->AddWidget(cb);

				Wizard->AddWidget((PG_Widget*)cb, s_name, s_action);

			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if
	else if(stricmp(elementName, "radiobutton") == 0)
	{
		if(ParseRBGroupInProgress && ParsePageElementInProgress) 
		{  
			//vars for create PG_CheckButton  
			std::string s_name="\0",s_action="\0",themeobject="\0",themestyle="\0",text="\0";
			DWORD baction = 0;
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "action") == 0)
				{
					s_action = XMLString::transcode(attributes.getValue(attributeName));
					if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_page") == 0)
						baction = HANDLE_PREVIOUS_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_page") == 0)
						baction = HANDLE_NEXT_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_page") == 0)
						baction = HANDLE_FIRST_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_page") == 0)
						baction = HANDLE_LAST_PAGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "exit") == 0)
						baction = HANDLE_EXIT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "install") == 0)
						baction = HANDLE_INSTALL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "browse") == 0)
						baction = HANDLE_BROWSE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "cancel") == 0)
						baction = HANDLE_CANCEL;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "cmp_russian") == 0)
						baction = HANDLE_CHANGELANGR;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "cmp_english") == 0)
						baction = HANDLE_CHANGELANGE;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "awlogo") == 0)
						baction = HANDLE_AWLOGO;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "WWW") == 0)
						baction = HANDLE_WWW;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "next_screenshot") == 0)
						baction = HANDLE_NEXT_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "prev_screenshot") == 0)
						baction = HANDLE_PREVIOUS_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "first_screenshot") == 0)
						baction = HANDLE_FIRST_SCREENSHOT;
					else if(stricmp(XMLString::transcode(attributes.getValue(attributeName)), "last_screenshot") == 0)
						baction = HANDLE_LAST_SCREENSHOT;
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "text") == 0)
				{			    
					text = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					GetParamColor(XMLString::transcode(attributes.getValue(attributeName)), &tcolor);
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{	

				PG_RadioButton *rb = new PG_RadioButton(NULL,1, PG_Rect(x,y,w,h), text.c_str(), CurrentRBGroup, themeobject.c_str());
				rb->LoadThemeStyle(themeobject.c_str());
				rb->SetFontSize(tsize);
				rb->SetFontStyle(tstyle);
				rb->SetFontColor(tcolor);
				rb->SetFontAlpha(tcolor.unused);

				switch(baction)
				{
				case HANDLE_EXIT:                rb->SetEventCallback(MSG_BUTTONCLICK, handle_exit, &app);
					break;
				case HANDLE_INSTALL:             rb->SetEventCallback(MSG_BUTTONCLICK, handle_INSTALL, &app);
					break;
				case HANDLE_UNINSTALL:           rb->SetEventCallback(MSG_BUTTONCLICK, handle_UNINSTALL, &app);
					break;
				case HANDLE_START:               rb->SetEventCallback(MSG_BUTTONCLICK, handle_START, &app);
					break;
				case HANDLE_BROWSE:              rb->SetEventCallback(MSG_BUTTONCLICK, handle_BROWSE, &app);
					break;
				case HANDLE_CANCEL:              rb->SetEventCallback(MSG_BUTTONCLICK, handle_CANCEL, &app);
					break;
				case HANDLE_CHANGELANGR:         rb->SetEventCallback(MSG_BUTTONCLICK, handle_CHANGELANGR, &app);
					break;
				case HANDLE_CHANGELANGE:         rb->SetEventCallback(MSG_BUTTONCLICK, handle_CHANGELANGE, &app);
					break;
				case HANDLE_AWLOGO:              rb->SetEventCallback(MSG_BUTTONCLICK, handle_AWLOGO, &app);
					break;
				case HANDLE_WWW:                 rb->SetEventCallback(MSG_BUTTONCLICK, handle_WWW, &app);
					break;
				case HANDLE_MINIMIZE:            rb->SetEventCallback(MSG_BUTTONCLICK, handle_MINIMIZE, &app);
					break;
				case HANDLE_EASY:                rb->SetEventCallback(MSG_BUTTONCLICK, handle_EASY, &app);
					break;
				case HANDLE_NEXT_PAGE:           rb->SetEventCallback(MSG_BUTTONCLICK, handle_next_page, &app);
					break;
				case HANDLE_PREVIOUS_PAGE:       rb->SetEventCallback(MSG_BUTTONCLICK, handle_previous_page, &app);
					break;
				case HANDLE_FIRST_PAGE:          rb->SetEventCallback(MSG_BUTTONCLICK, handle_first_page, &app);
					break;
				case HANDLE_LAST_PAGE:           rb->SetEventCallback(MSG_BUTTONCLICK, handle_last_page, &app);
					break;
				case HANDLE_NEXT_SCREENSHOT:     rb->SetEventCallback(MSG_BUTTONCLICK, handle_next_screenshot, &app);
					break;
				case HANDLE_PREVIOUS_SCREENSHOT: rb->SetEventCallback(MSG_BUTTONCLICK, handle_previous_screenshot, &app);
					break;
				case HANDLE_FIRST_SCREENSHOT:    rb->SetEventCallback(MSG_BUTTONCLICK, handle_first_screenshot, &app);
					break;
				case HANDLE_LAST_SCREENSHOT:     rb->SetEventCallback(MSG_BUTTONCLICK, handle_last_screenshot, &app);
					break;
				}

				if(!CurrentRBGroup) CurrentRBGroup = rb;
				CurrentPage->AddWidget(rb);

				Wizard->AddWidget((PG_Widget*)rb, s_name, s_action);

			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if
	else if(stricmp(elementName, "label") == 0)
	{
		if(ParsePageElementInProgress) 
		{  
			//vars for create PG_Button  
			std::string s_name="\0",themeobject="\0",themestyle="\0",text="\0",type="\0";
			DWORD baction = 0;
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "text") == 0)
				{			    
					text = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "type") == 0)
				{			    
					type = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					char *tmp = XMLString::transcode(attributes.getValue(attributeName));
					GetParamColor(tmp, &tcolor);
					//delete tmp;
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{					
				PG_Label *l = new PG_Label(NULL,PG_Rect(x,y,w,h),text.c_str(),themeobject.c_str());
				l->LoadThemeStyle(themeobject.c_str(),themestyle.c_str());
				l->SetFontSize(tsize);
				l->SetFontStyle(tstyle);
				l->SetFontColor(tcolor);
				l->SetFontAlpha(tcolor.unused);
				CurrentPage->AddWidget(l);
				if(type != "static") DynamicControls->AddDynamicLabel(text, l);
				Wizard->AddWidget((PG_Widget*)l, s_name, "");
			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if
	else if(stricmp(elementName, "richedit") == 0)
	{
		if(ParsePageElementInProgress) 
		{  
			//vars for create PG_LineEdit
			std::string s_name="\0",themeobject="\0",themestyle="\0",text="\0";
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));

				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "text") == 0)
				{			    
					text = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					char *tmp = XMLString::transcode(attributes.getValue(attributeName));
					GetParamColor(tmp, &tcolor);
					//delete tmp;
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{					
				
				PG_ListBox *lb = new PG_ListBox(NULL,PG_Rect(x,y,w,h));
				//PG_WidgetList *wl = new PG_WidgetList(NULL,PG_Rect(x,y,w,h));
				lb->SetDirtyUpdate(false);
				//wl->SetTransparency(0);
				//wl->SetBackgroundBlend(0); 

				//lb->LoadThemeStyle(themeobject.c_str());
				//lb->SetText(text.c_str());   
				lb->SetFontSize(tsize);
				lb->SetFontStyle(tstyle);
				lb->SetFontColor(tcolor);
				lb->SetFontAlpha(tcolor.unused);
					

				DWORD linelength = (w/tsize)*1.6;

				DWORD start = 0;
				DWORD end = 0;
				DWORD lineend = linelength;
				for(UINT i = 0; i <= text.length(); i++)
				{
					if(*(text.c_str() + i) == ' ') end = i;
					if(i == lineend || i == text.length())
					{
				        PG_ListBoxBaseItem *lbbi = new PG_ListBoxBaseItem(tsize + 6);
						lbbi->SetText(text.substr(start, end - start).c_str());
						lbbi->SetFontSize(tsize);
						lbbi->SetFontStyle(tstyle);
				        lbbi->SetFontColor(tcolor);
				        lbbi->SetFontAlpha(tcolor.unused);
					    lb->AddItem(lbbi);
						start = end + 1;
						i = ++end;
						lineend = i + linelength;
					}
				}
				

				
				CurrentPage->AddWidget(lb);
				std::string action="";
				Wizard->AddWidget((PG_Widget*)lb, s_name, action);
				//DynamicControls->AddDynamicLabel(text, (PG_Label*)le);
				
			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if
	else if(stricmp(elementName, "edit") == 0)
	{
		if(ParsePageElementInProgress) 
		{  
			//vars for create PG_LineEdit
			std::string s_name="\0",themeobject="\0",themestyle="\0",text="\0";
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));

				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "text") == 0)
				{			    
					text = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					char *tmp = XMLString::transcode(attributes.getValue(attributeName));
					GetParamColor(tmp, &tcolor);
					//delete tmp;
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{					
				PG_LineEdit *le = new PG_LineEdit(NULL,PG_Rect(x,y,w,h),themeobject.c_str(),255);
				le->LoadThemeStyle(themeobject.c_str(),themestyle.c_str());
				le->SetText(text.c_str());   
				le->SetFontSize(tsize);
				le->SetFontStyle(tstyle);
				le->SetFontColor(tcolor);
				le->SetFontAlpha(tcolor.unused);

				if(s_name == "Path")
				{

					le->SetEventCallback(MSG_EDITEND, handle_EDITPATH, &app);
					le->SetEventCallback(MSG_EDITBEGIN, handle_EDITPATH, &app);
				}

				CurrentPage->AddWidget(le);
				std::string action="";
				Wizard->AddWidget((PG_Widget*)le, s_name, action);
				//DynamicControls->AddDynamicLabel(text, (PG_Label*)le);
			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if
	else if(stricmp(elementName, "progressbar") == 0)
	{
		if(ParsePageElementInProgress) 
		{  
			//vars for create PG_ProgressBar
			std::string s_name="\0",themeobject="\0",themestyle="\0", s_drawpercentage="\0";
			std::string p_action = "\0";
			UINT x=0,y=0,w=0,h=0,tsize=0;
			SDL_Color tcolor;
			UINT tstyle=0;
			BOOL drawpercentage = TRUE;
			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{
					s_name = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themeobject") == 0)
				{
					themeobject= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "themestyle") == 0)
				{
					themestyle= XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "pos") == 0)
				{
					RECT rect;
					GetParamRect(XMLString::transcode(attributes.getValue(attributeName)),&rect);
					x = (int)(FKoeffX*rect.left);
					y = (int)(FKoeffY*rect.top);
					w = (int)(FKoeffX*rect.right);
					h = (int)(FKoeffY*rect.bottom);
				}//else if
				else if(stricmp(attributeName, "tsize") == 0)
				{
					tsize = (int)(FKoeffX*StrToInt(XMLString::transcode(attributes.getValue(attributeName))));
				}//else if
				else if(stricmp(attributeName, "action") == 0)
				{
					p_action = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else if(stricmp(attributeName, "percent") == 0)
				{
					s_drawpercentage = XMLString::transcode(attributes.getValue(attributeName));
					if(s_drawpercentage == "false") drawpercentage = FALSE;
				}//else if
				else if(stricmp(attributeName, "tcolor") == 0)
				{
					char *tmp = XMLString::transcode(attributes.getValue(attributeName));
					GetParamColor(tmp, &tcolor);
					//delete tmp;
				}//else if
				else if(stricmp(attributeName, "tstyle") == 0)
				{
					if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"normal"))
						tstyle = PG_FSTYLE_NORMAL;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"bold"))
						tstyle = PG_FSTYLE_BOLD;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"italic"))
						tstyle = PG_FSTYLE_ITALIC;
					else if(!stricmp(XMLString::transcode(attributes.getValue(attributeName)),"underline"))
						tstyle = PG_FSTYLE_UNDERLINE;
				}//else if
				//delete attributeName;
			}//for
			if(s_name.compare("\0")&&themeobject.compare("\0")&&themestyle.compare("\0"))
			{					
				PG_ProgressBar *pb = new PG_ProgressBar(NULL,PG_Rect(x,y,w,h),themeobject.c_str());
				pb->LoadThemeStyle(themeobject.c_str());
				pb->SetFontSize(tsize);
				pb->SetFontStyle(tstyle);
				pb->SetFontColor(tcolor);
				pb->SetFontAlpha(tcolor.unused);
				pb->SetProgress(0);
				pb->SetDrawPercentage(drawpercentage);
				CurrentPage->AddWidget(pb);
				DynamicControls->AddDynamicProgressBar(p_action, pb);


			}//if
			else {PG_LogERR("can't create widget '%s'", elementName);}// m_errors++;delete s_name;delete themestyle;delete themeobject;delete text;}

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if
	else if(stricmp(elementName, "string") == 0)
	{
		if(ParseLocalisationInProgress) 
		{  
			std::string s_name="\0",text="\0";
			SI_TextString textstring;

			for(unsigned int i = 0; i < attributes.getLength(); i++)
			{
				char *attributeName = XMLString::transcode(attributes.getName(i));
				if(stricmp(attributeName, "name") == 0)
				{					
					textstring.name = XMLString::transcode(attributes.getValue(attributeName));
				}//else if
				else 
				{			  
					std::string lang = attributeName;
					textstring.langlist.push_back(lang);
					text = XMLString::transcode(attributes.getValue(attributeName));
					textstring.textlist.push_back(text);
					
				}//else if
				
			}//for

		}//if
		else
			PG_LogERR("trying create widget '%s' without page", elementName);
	}//else if


	//delete elementName;
}//SDLInstallerLayout::startElement



// -----------------------------------------------------------------------
//  Implementations of the SAX ErrorHandler interface
// -----------------------------------------------------------------------
// warning exception handler
void SDLInstallerLayout::warning(const SAXParseException& exception)
{
	m_warnings++;
	PG_LogWRN("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}
// error exception handler
void SDLInstallerLayout::error(const SAXParseException& exception)
{
	m_errors++;
	PG_LogERR("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}
// fatal error exception handler
void SDLInstallerLayout::fatalError(const SAXParseException& exception)
{
	PG_LogERR("%s line=%d column=%d",XMLString::transcode(exception.getMessage()), exception.getLineNumber(), exception.getColumnNumber());
}

//class SDLInstallerPage routains
//constructor
SDLInstallerPage::SDLInstallerPage(LPCSTR name) 
{
	Name = name;
	Index = 0;
}

//destructor
SDLInstallerPage::~SDLInstallerPage() 
{	
	// go through and delete the items from the list
	for(WIDGETLIST::iterator i_widget = WidgetList.begin();
		i_widget != WidgetList.end();
		/*std::advance( i_widget, 1 )*/ i_widget++)
	{
		//if(*i_widget){
		delete *i_widget;
		// *i_widget = NULL;
		//}
	}
	// delete the STL objects as well
	WidgetList.erase( WidgetList.begin(), WidgetList.end() );
	//delete name string
	//delete Name;
}
/*
PG_Widget* SDLInstallerPage::GetWidget(LPCSTR name)
{
PAGELIST::iterator i_widget;

i_widget = WidgetList.begin();
while(i_widget != WidgetList.end())
{      
if((*i_widget)->GetIndex() == index)
return (*i_widget);
i_widget++;
}
return 0;
}
*/


void SDLInstallerPage::AddWidget(PG_Widget* widget)
{
	WidgetList.push_back(widget);
}

void SDLInstallerPage::ShowAll(bool alpha)
{
	WIDGETLIST::iterator i_widget;

	i_widget = WidgetList.begin();
	while(i_widget != WidgetList.end())
	{
		if(*i_widget)(*i_widget)->Show(alpha);
		i_widget++;
	}
}

void SDLInstallerPage::HideAll(bool alpha)
{
	WIDGETLIST::iterator i_widget;

	i_widget = WidgetList.begin();
	while(i_widget != WidgetList.end())
	{
		if(*i_widget)(*i_widget)->Hide(alpha);
		i_widget++;
	}
}

//class SDLInstallerWizard routains
//constructor
SDLInstallerWizard::SDLInstallerWizard()
{
	CurrentPage = 1;
	CurrentScreenShot = 1;
	CurrentAnnounce = 1;
}
//destructor
SDLInstallerWizard::~SDLInstallerWizard()
{
	// go through and delete the items from the list
	for(PAGELIST::iterator i_page = PageList.begin();
		i_page != PageList.end();
		/*std::advance( i_page, 1 )*/ i_page++)
	{
		delete *i_page;
	}
	// delete the STL objects as well
	PageList.erase( PageList.begin(), PageList.end() );

	// go through and delete the items from the list
	for(PAGELIST::iterator i_screenshot = ScreenShotList.begin();
		i_screenshot != ScreenShotList.end();
		/*std::advance( i_screenshot, 1 )*/ i_screenshot++)
	{
		delete *i_screenshot;
	}
	// delete the STL objects as well
	ScreenShotList.erase( ScreenShotList.begin(), ScreenShotList.end() );

	// go through and delete the items from the list
	for(PAGELIST::iterator i_announce = AnnounceList.begin();
		i_announce != AnnounceList.end();
		/*std::advance( i_screenshot, 1 )*/ i_announce++)
	{
		delete *i_announce;
	}
	// delete the STL objects as well
	AnnounceList.erase( AnnounceList.begin(), AnnounceList.end() );

	// go through and delete the items from the list
	for(PAGELIST::iterator i_frame = FrameList.begin();
		i_frame != FrameList.end();
		/*std::advance( i_frame, 1 )*/ i_frame++)
	{
		delete *i_frame;
	}
	// delete the STL objects as well
	FrameList.erase( FrameList.begin(), FrameList.end() );
}

PG_Widget *SDLInstallerWizard::GetWidgetByName(LPCSTR name)
{
	WIDGETLIST::iterator i_widget;
	STRINGLIST::iterator i_name;

	i_widget = AllWidgets.begin();
	i_name = AllWidgetNames.begin();
	while(i_widget != AllWidgets.end())
	{      
		if((*i_name) == name)
			return (*i_widget);
		i_widget++;
		i_name++;
	}
	return 0;
}

PG_Widget *SDLInstallerWizard::GetWidgetByAction(LPCSTR action)
{
	WIDGETLIST::iterator i_widget;
	STRINGLIST::iterator i_action;

	i_widget = AllWidgets.begin();
	i_action = AllWidgetActions.begin();
	while(i_widget != AllWidgets.end())
	{      
		if((*i_action) == action)
			return (*i_widget);
		i_widget++;
		i_action++;
	}
	return 0;
}

PG_Widget *SDLInstallerWizard::GetWidgetByNameAndAction(LPCSTR name, LPCSTR action)
{
	WIDGETLIST::iterator i_widget;
	STRINGLIST::iterator i_name;
	STRINGLIST::iterator i_action;

	i_widget = AllWidgets.begin();
	i_name = AllWidgetNames.begin();
	i_action = AllWidgetActions.begin();
	while(i_widget != AllWidgets.end())
	{      
		if((*i_name) == name && (*i_action) == action)
			return (*i_widget);
		i_widget++;
		i_name++;
		i_action++;
	}
	return 0;
}

void SDLInstallerWizard::AddPage(SDLInstallerPage* page)
{
	PageList.push_back(page);
}

void SDLInstallerWizard::AddScreenShot(SDLInstallerPage* screenshot)
{
	ScreenShotList.push_back(screenshot);
}

void SDLInstallerWizard::AddAnnounce(SDLInstallerPage* announce)
{
	AnnounceList.push_back(announce);
}

void SDLInstallerWizard::AddFrame(SDLInstallerPage* frame)
{
	FrameList.push_back(frame);
}

void SDLInstallerWizard::ShowPage(LPCSTR name, bool alpha)
{
	PAGELIST::iterator i_page;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{
		if(!strcmp((*i_page)->GetName(), name))
		{
			(*i_page)->ShowAll(alpha);
			CurrentPage = (*i_page)->GetIndex();
		}
		i_page++;
	}
}

void SDLInstallerWizard::ShowScreenShot(DWORD index, bool alpha)
{
	PAGELIST::iterator i_screenshot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{
		if((*i_screenshot)->GetIndex() == index)
		{	
			(*i_screenshot)->ShowAll(alpha);
			CurrentScreenShot = (*i_screenshot)->GetIndex();
		}
		i_screenshot++;
	}
}

void SDLInstallerWizard::ShowAnnounce(DWORD index, bool alpha)
{
	PAGELIST::iterator i_announce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{
		if((*i_announce)->GetIndex() == index)
		{	
			(*i_announce)->ShowAll(alpha);
			CurrentAnnounce = (*i_announce)->GetIndex();
		}
		i_announce++;
	}
}

void SDLInstallerWizard::ShowFrame(LPCSTR name, bool alpha)
{
	PAGELIST::iterator i_frame;

	i_frame = FrameList.begin();
	while(i_frame != FrameList.end())
	{
		if(!strcmp((*i_frame)->GetName(), name))
			(*i_frame)->ShowAll(alpha);
		i_frame++;
	}
}

void SDLInstallerWizard::HidePage(LPCSTR name, bool alpha)
{
	PAGELIST::iterator i_page;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		if(!strcmp((*i_page)->GetName(), name))
			(*i_page)->HideAll(alpha);
		i_page++;
	}
}

void SDLInstallerWizard::HideAllPages(void)
{
	PAGELIST::iterator i_page;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		(*i_page)->HideAll(false);
		i_page++;
	}
}

void SDLInstallerWizard::HideScreenShot(DWORD index, bool alpha)
{
	PAGELIST::iterator i_screenshot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{
		if((*i_screenshot)->GetIndex() == index)
			(*i_screenshot)->HideAll(alpha);
		i_screenshot++;
	}
}

void SDLInstallerWizard::HideAnnounce(DWORD index, bool alpha)
{
	PAGELIST::iterator i_announce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{
		if((*i_announce)->GetIndex() == index)
			(*i_announce)->HideAll(alpha);
		i_announce++;
	}
}

void SDLInstallerWizard::HideFrame(LPCSTR name, bool alpha)
{
	PAGELIST::iterator i_frame;

	i_frame = FrameList.begin();
	while(i_frame != FrameList.end())
	{
		if(!strcmp((*i_frame)->GetName(), name))
			(*i_frame)->HideAll(alpha);
		i_frame++;
	}
}

SDLInstallerPage* SDLInstallerWizard::GetPage(DWORD index)
{
	PAGELIST::iterator i_page;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		if((*i_page)->GetIndex() == index)
			return (*i_page);
		i_page++;
	}
	return 0;
}

SDLInstallerPage* SDLInstallerWizard::GetScreenShot(DWORD index)
{
	PAGELIST::iterator i_screenshot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{      
		if((*i_screenshot)->GetIndex() == index)
			return (*i_screenshot);
		i_screenshot++;
	}
	return 0;
}

SDLInstallerPage* SDLInstallerWizard::GetAnnounce(DWORD index)
{
	PAGELIST::iterator i_announce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{      
		if((*i_announce)->GetIndex() == index)
			return (*i_announce);
		i_announce++;
	}
	return 0;
}

SDLInstallerPage* SDLInstallerWizard::GetFrame(DWORD index)
{
	PAGELIST::iterator i_frame;

	i_frame = FrameList.begin();
	while(i_frame != FrameList.end())
	{      
		if((*i_frame)->GetIndex() == index)
			return (*i_frame);
		i_frame++;
	}
	return 0;
}

void SDLInstallerWizard::ShowNextPage(bool alpha)
{
	if(infoexpander->CurrentWizardPage != infoexpander->CountOfWizardPages)
		infoexpander->CurrentWizardPage++; 
	else return;

	PAGELIST::iterator i_page;
	DWORD tmp; tmp = CurrentPage;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		if((*i_page)->GetIndex() == tmp + 1 && CurrentPage != PageList.size())
		{
			GetPage(CurrentPage)->HideAll(alpha);
			(*i_page)->ShowAll(alpha);
			CurrentPage = tmp + 1;
		}
		i_page++;
	}
}

void SDLInstallerWizard::ShowPreviousPage(bool alpha)
{
	if(infoexpander->CurrentWizardPage != 1)
		infoexpander->CurrentWizardPage--; 
	else return;

	PAGELIST::iterator i_page;
	DWORD tmp; tmp = CurrentPage;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		if((*i_page)->GetIndex() == tmp - 1 && CurrentPage != 1)
		{
			GetPage(CurrentPage)->HideAll(alpha);
			(*i_page)->ShowAll(alpha);
			CurrentPage = tmp - 1;
		}
		i_page++;
	}
}

void SDLInstallerWizard::ShowFirstPage(bool alpha)
{
	infoexpander->CurrentWizardPage = 1; 	

	PAGELIST::iterator i_page;
	DWORD tmp; tmp = CurrentPage;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		if((*i_page)->GetIndex() == 1)
		{
			GetPage(CurrentPage)->HideAll(alpha);
			(*i_page)->ShowAll(alpha);
			CurrentPage = 1;
		}
		i_page++;
	}
}

void SDLInstallerWizard::ShowLastPage(bool alpha)
{
	infoexpander->CurrentWizardPage = infoexpander->CountOfWizardPages; 

	PAGELIST::iterator i_page;
	DWORD tmp; tmp = CurrentPage;

	i_page = PageList.begin();
	while(i_page != PageList.end())
	{      
		if((*i_page)->GetIndex() == PageList.size())
		{
			GetPage(CurrentPage)->HideAll(alpha);
			(*i_page)->ShowAll(alpha);
			CurrentPage = PageList.size();
		}
		i_page++;
	}
}

void SDLInstallerWizard::ShowNextScreenShot(bool alpha)
{
	if(infoexpander->CurrentScreenShot != infoexpander->CountOfScreenShots)
		infoexpander->CurrentScreenShot++; 
	else return;

	PAGELIST::iterator i_screenshot;
	DWORD tmp; tmp = CurrentScreenShot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{      
		if((*i_screenshot)->GetIndex() == tmp + 1 && CurrentScreenShot != ScreenShotList.size())
		{
			GetScreenShot(CurrentScreenShot)->HideAll(alpha);
			(*i_screenshot)->ShowAll(alpha);
			CurrentScreenShot = tmp + 1;
		}
		i_screenshot++;
	}
}

void SDLInstallerWizard::ShowPreviousScreenShot(bool alpha)
{
	if(infoexpander->CurrentScreenShot != 1)
		infoexpander->CurrentScreenShot--; 
	else return;

	PAGELIST::iterator i_screenshot;
	DWORD tmp; tmp = CurrentScreenShot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{      
		if((*i_screenshot)->GetIndex() == tmp - 1 && CurrentScreenShot != 1)
		{
			GetScreenShot(CurrentScreenShot)->HideAll(alpha);
			(*i_screenshot)->ShowAll(alpha);
			CurrentScreenShot = tmp - 1;
		}
		i_screenshot++;
	}
}

void SDLInstallerWizard::ShowFirstScreenShot(bool alpha)
{
	infoexpander->CurrentScreenShot = 1;

	PAGELIST::iterator i_screenshot;
	DWORD tmp; tmp = CurrentScreenShot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{      
		if((*i_screenshot)->GetIndex() == 1)
		{
			GetScreenShot(CurrentScreenShot)->HideAll(alpha);
			(*i_screenshot)->ShowAll(alpha);
			CurrentScreenShot = 1;
		}
		i_screenshot++;
	}
}

void SDLInstallerWizard::ShowLastScreenShot(bool alpha)
{
	infoexpander->CurrentScreenShot = infoexpander->CountOfScreenShots; 

	PAGELIST::iterator i_screenshot;
	DWORD tmp; tmp = CurrentScreenShot;

	i_screenshot = ScreenShotList.begin();
	while(i_screenshot != ScreenShotList.end())
	{      
		if((*i_screenshot)->GetIndex() == ScreenShotList.size())
		{
			GetScreenShot(CurrentScreenShot)->HideAll(alpha);
			(*i_screenshot)->ShowAll(alpha);
			CurrentScreenShot = ScreenShotList.size();
		}
		i_screenshot++;
	}
}
//-------------------------------------------------------------
void SDLInstallerWizard::ShowNextAnnounce(bool alpha)
{
	if(infoexpander->CurrentAnnounce != infoexpander->CountOfAnnounces)
		infoexpander->CurrentAnnounce++; 
	else return;

	PAGELIST::iterator i_announce;
	DWORD tmp; tmp = CurrentAnnounce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{      
		if((*i_announce)->GetIndex() == tmp + 1 && CurrentAnnounce != AnnounceList.size())
		{
			GetAnnounce(CurrentAnnounce)->HideAll(alpha);
			(*i_announce)->ShowAll(alpha);
			CurrentAnnounce = tmp + 1;
		}
		i_announce++;
	}
}

void SDLInstallerWizard::ShowPreviousAnnounce(bool alpha)
{
	if(infoexpander->CurrentAnnounce != 1)
		infoexpander->CurrentAnnounce--; 
	else return;

	PAGELIST::iterator i_announce;
	DWORD tmp; tmp = CurrentAnnounce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{      
		if((*i_announce)->GetIndex() == tmp - 1 && CurrentAnnounce != 1)
		{
			GetAnnounce(CurrentAnnounce)->HideAll(alpha);
			(*i_announce)->ShowAll(alpha);
			CurrentAnnounce = tmp - 1;
		}
		i_announce++;
	}
}

void SDLInstallerWizard::ShowFirstAnnounce(bool alpha)
{
	infoexpander->CurrentAnnounce = 1;

	PAGELIST::iterator i_announce;
	DWORD tmp; tmp = CurrentAnnounce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{      
		if((*i_announce)->GetIndex() == 1)
		{
			GetAnnounce(CurrentAnnounce)->HideAll(alpha);
			(*i_announce)->ShowAll(alpha);
			CurrentAnnounce = 1;
		}
		i_announce++;
	}
}

void SDLInstallerWizard::ShowLastAnnounce(bool alpha)
{
	infoexpander->CurrentAnnounce = infoexpander->CountOfAnnounces; 

	PAGELIST::iterator i_announce;
	DWORD tmp; tmp = CurrentAnnounce;

	i_announce = AnnounceList.begin();
	while(i_announce != AnnounceList.end())
	{      
		if((*i_announce)->GetIndex() == AnnounceList.size())
		{
			GetAnnounce(CurrentAnnounce)->HideAll(alpha);
			(*i_announce)->ShowAll(alpha);
			CurrentAnnounce = AnnounceList.size();
		}
		i_announce++;
	}
}

SDL_DynamicControlsRefresh::SDL_DynamicControlsRefresh()
{

}

SDL_DynamicControlsRefresh::~SDL_DynamicControlsRefresh()
{
	/*	// go through and delete the items from the list
	for(STRINGLIST::iterator i_inputstring = InputStringList.begin();
	i_inputstring != InputStringList.end();
	std::advance( i_inputstring, 1 ) )
	{
	//	(*i_inputstring).clear();
	}
	*/	// delete the STL objects as well
	InputStringList.erase( InputStringList.begin(), InputStringList.end() );

	/*	// go through and delete the items from the list
	for(STRINGLIST::iterator i_outputstring = OutputStringList.begin();
	i_outputstring != OutputStringList.end();
	std::advance( i_outputstring, 1 ) )
	{
	//	(*i_outputstring).clear();
	}
	*/	// delete the STL objects as well
	OutputStringList.erase( OutputStringList.begin(), OutputStringList.end() );

	// delete the STL objects as well
	LabelList.erase( LabelList.begin(), LabelList.end() );

}

void SDL_DynamicControlsRefresh::AddDynamicLabel(std::string string, PG_Label* label)
{
	InputStringList.push_back(string);
	OutputStringList.push_back(string);
	LabelList.push_back(label);
}

void SDL_DynamicControlsRefresh::AddDynamicProgressBar(std::string action, PG_ProgressBar* pb)
{
	ProgressBarList.push_back(pb);
	ProgressBarActionList.push_back(action);
}

void SDL_DynamicControlsRefresh::Refresh(void)
{
	STRINGLIST::iterator i_inputstring;
	STRINGLIST::iterator i_outputstring;
	LABELLIST::iterator i_label;
	PROGRESSBARLIST::iterator i_progressbar;
	STRINGLIST::iterator i_progressbaraction;

	i_outputstring = OutputStringList.begin();
	i_inputstring = InputStringList.begin();
	i_label = LabelList.begin();
	while(i_inputstring != InputStringList.end())
	{      
		infoexpander->ExpandString(&(*i_inputstring), &(*i_outputstring));
		(*i_label)->SetText((*i_outputstring).c_str());
		i_inputstring++;
		i_outputstring++;
		i_label++;
	}

	std::string FreeSpaceLabelR = "%MAIN_DRIVE%";
	std::string FreeSpaceR = "%CUR_DRIVE_FREE_SPACE_MB% Ìב";
	std::string RequiredSpaceR = "%REQUIRED_SPACE_MB% Ìב";

    infoexpander->ExpandString(&FreeSpaceLabelR, &FreeSpaceLabelR);
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_STATIC_FREE_LABEL),WM_SETTEXT,0, (WPARAM)FreeSpaceLabelR.c_str());

	infoexpander->ExpandString(&FreeSpaceR, &FreeSpaceR);
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_STATIC_FREE),WM_SETTEXT,0, (WPARAM)FreeSpaceR.c_str());

	infoexpander->ExpandString(&RequiredSpaceR, &RequiredSpaceR);
	SendMessageA(GetDlgItem(infoexpander->hwndEasyInstaller, IDC_STATIC_REQ),WM_SETTEXT,0, (WPARAM)RequiredSpaceR.c_str());





	i_progressbar = ProgressBarList.begin();
	i_progressbaraction = ProgressBarActionList.begin();
	while(i_progressbar != ProgressBarList.end())
	{      
		if((*i_progressbaraction) == "full_progress")
		{
			(*i_progressbar)->SetProgress(infoexpander->GetArchiveProgress());
		}
		else if((*i_progressbaraction) == "file_progress")
		{
			(*i_progressbar)->SetProgress(infoexpander->GetFileProgress());
		}
		else if((*i_progressbaraction) == "free_space")
		{
			(*i_progressbar)->SetProgress(GetFreeSpacePercent());
		}
		i_progressbar++;
		i_progressbaraction++;
	}

	if(infoexpander->GetArchiveProgress() > (100/infoexpander->CountOfScreenShots)*infoexpander->CurrentScreenShot
		&&infoexpander->CurrentScreenShot < infoexpander->CountOfScreenShots)
	{
		handler->GetWizard()->ShowNextScreenShot(true);
	}

}


//////
//  Utilities
//////

//SDL_Color form string "#ARGB"
void GetParamColor(LPCSTR param, SDL_Color *color)
{
	DWORD colorvalue = 0;
	sscanf(param, "#%08x", &colorvalue);
	color->unused = (UINT)(colorvalue >> 24) & 0xFF;
	color->r      = (UINT)(colorvalue >> 16) & 0xFF;
	color->g      = (UINT)(colorvalue >>  8) & 0xFF;
	color->b      = (UINT)(colorvalue >>  0) & 0xFF;

}

// Write to RECT from string "x,y,w,h". left=x, top=y, right=w, bottom=h
void GetParamRect(LPCSTR pos, RECT *rect) {

	if(pos == NULL) return;
	if(strlen(pos) == 0) return;

	char* parm;
	char *d;
	char tmp[16];
	int i=0;
	int r[4];

	r[0] = r[1] = r[2] = r[3] = 0;
	parm = strdup(pos);

	for(d = strtok(parm,","); d != NULL; d = strtok(NULL,",")) {
		if(sscanf(d, "%d%[%]", & r[i], tmp) == 2 )
			r[i];
		i++;
	}

	rect->left    = r[0];
	rect->top     = r[1];
	rect->right   = r[2];
	rect->bottom  = r[3];
	free(parm);
}





PG_MyButton::PG_MyButton(PG_Widget* parent, int id, const PG_Rect& r, const char* text, const char* style) : PG_Button(parent, id, r) {

	//Init(style);
}


bool PG_MyButton::eventMouseMotion(const SDL_MouseMotionEvent* motion) {
	//exit(0);
	if(isLMouseDown)
	{	
		//exit(0);
	//	infoexpander->hWndMain			
		MoveWindow(motion->x, motion->y);
	}
	return false;
}


bool PG_MyButton::eventMouseButtonDown(const SDL_MouseButtonEvent* button) {
	if(button->type == 2)
	   exit(0);
	//isLMouseDown = true;
	return false;
}


bool PG_MyButton::eventMouseButtonUp(const SDL_MouseButtonEvent* button) {
	if(button->type == 2)
	   isLMouseDown = false;
	return false;
}