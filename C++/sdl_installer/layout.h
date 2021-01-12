#ifndef SDLINSTALLERLAYOUT
#define SDLINSTALLERLAYOUT

// disable long class name warnings for STL
#pragma warning(disable : 4786)
#pragma warning(disable : 4788)


#define HANDLE_EXIT                0x00
#define HANDLE_NEXT_PAGE           0x01
#define HANDLE_PREVIOUS_PAGE       0x02
#define HANDLE_INSTALL             0x03
#define HANDLE_INSDX               0x034
#define HANDLE_UNINSTALL           0x031
#define HANDLE_START               0x032
#define HANDLE_FIRST_PAGE          0x04
#define HANDLE_LAST_PAGE           0x05
#define HANDLE_PREVIOUS_SCREENSHOT 0x06
#define HANDLE_NEXT_SCREENSHOT     0x07
#define HANDLE_FIRST_SCREENSHOT    0x08
#define HANDLE_LAST_SCREENSHOT     0x09

#define HANDLE_PREVIOUS_ANNOUNCE 0x061
#define HANDLE_NEXT_ANNOUNCE     0x071
#define HANDLE_FIRST_ANNOUNCE    0x081
#define HANDLE_LAST_ANNOUNCE     0x091

#define HANDLE_AWLOGO              0x10
#define HANDLE_CANCEL              0x11
#define HANDLE_MINIMIZE            0x111
#define HANDLE_EASY                0x112
#define HANDLE_CHANGELANGR         0x113
#define HANDLE_CHANGELANGE         0x114
#define HANDLE_BROWSE              0x12
#define HANDLE_WWW                 0x121

#define HANDLE_SETCOMPONENT        0x13

#define HANDLE_REQUIREMENTS_PAGE        0x1001
#define HANDLE_DESCRIPTION_PAGE         0x1002
#define HANDLE_SCREENSHOTS_PAGE         0x1005
#define HANDLE_CHEATS_PAGE              0x1003
#define HANDLE_INSTALL_PAGE             0x1004




#define DINAMIC_LABEL_INSTALL_PATH    0x01
#define DINAMIC_LABEL_INSTALL_PATH    0x01

#include <windows.h>
//#include <Winbase.h>
#include <sdl.h>
#include <stack>
#include <list>
#include <time.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <paragui.h>
#include <pgwidget.h>
#include <pgbutton.h>
#include <pgcheckbutton.h>
#include <pglabel.h>
#include <pglineedit.h>
#include <pgrichedit.h>
#include <pglistbox.h>
#include <pglistboxbaseitem.h>
#include <pgwidgetlist.h>
#include <pgcompat.h>
#include <pgprogressbar.h>
#include <pgthemewidget.h>
//#include "sdlinstaller.h"


//pGetDiskFreeSpaceExF pGetDiskFreeSpaceEx;
//GetDiskFreeSpaceEx1F GetDiskFreeSpace1Ex;


PARAGUI_CALLBACK(handle_AWLOGO);
PARAGUI_CALLBACK(handle_exit);
PARAGUI_CALLBACK(handle_AWLOGO);
PARAGUI_CALLBACK(handle_INSTALL);
PARAGUI_CALLBACK(handle_INSDX);
PARAGUI_CALLBACK(handle_CANCEL);
PARAGUI_CALLBACK(handle_MINIMIZE);
PARAGUI_CALLBACK(handle_EASY);
PARAGUI_CALLBACK(handle_previous_page);
PARAGUI_CALLBACK(handle_next_page);
PARAGUI_CALLBACK(handle_first_page);
PARAGUI_CALLBACK(handle_last_page);
PARAGUI_CALLBACK(handle_previous_screenshot);
PARAGUI_CALLBACK(handle_next_screenshot);
PARAGUI_CALLBACK(handle_first_screenshot);
PARAGUI_CALLBACK(handle_last_screenshot);

PARAGUI_CALLBACK(handle_previous_announce);
PARAGUI_CALLBACK(handle_next_announce);
PARAGUI_CALLBACK(handle_first_announce);
PARAGUI_CALLBACK(handle_last_announce);

PARAGUI_CALLBACK(handle_UNINSTALL);

PARAGUI_CALLBACK(handle_START);
PARAGUI_CALLBACK(handle_BROWSE);
PARAGUI_CALLBACK(handle_WWW);

PARAGUI_CALLBACK(handle_EDITPATH);
PARAGUI_CALLBACK(handle_CHANGELANGR);
PARAGUI_CALLBACK(handle_CHANGELANGE);

PARAGUI_CALLBACK(handle_requirements_page);
PARAGUI_CALLBACK(handle_description_page);
PARAGUI_CALLBACK(handle_screenshots_page);
PARAGUI_CALLBACK(handle_cheats_page);
PARAGUI_CALLBACK(handle_install_page);



PARAGUI_CALLBACK(handle_component_set);


BOOL LoadLayout( LPCSTR XMLfilename, float KoeffX = 1, float KoeffY = 1);
void GetParamRect(LPCSTR pos, RECT *rect);
//void TranslitToCP1251(LPSTR str);
void GetParamColor(LPCSTR param, SDL_Color *color);
//char *BinToChar( const XMLCh * const string );
//void SHParseString(std::string *string, std::string *rvalue);


class SDLInstallerWizard;
class SDLInstallerPage;
class SDL_DynamicControlsRefresh;

typedef std::list<PG_Widget*> WIDGETLIST;
typedef std::list<SDLInstallerPage*> PAGELIST;
typedef std::list<std::string> STRINGLIST;
typedef std::list<PG_Label*> LABELLIST;
typedef std::list<PG_ProgressBar*> PROGRESSBARLIST;

class SDL_DynamicControlsRefresh
{
public:
		
	SDL_DynamicControlsRefresh();
	~SDL_DynamicControlsRefresh();

	void Refresh(void);
	void AddDynamicLabel(std::string string, PG_Label* label);
	void AddDynamicProgressBar(std::string action, PG_ProgressBar* pb);

/*	void SetFullModuleFileName(LPCSTR path){ FullModuleFileName = path; }
	void SetModuleFilePath(LPCSTR path){ ModuleFilePath = path; }
	void SetLayoutFileName(LPCSTR path){ LayoutFileName = path; }
	void SetInstallPath(LPCSTR path){ InstallPath = path; }
	void SetMainDrive(LPCSTR path){ MainDrive = path; }

	void SetCurrentScreenShot(UINT i){ CurrentScreenShot = i; }
	void SetCountOfScreenShots(UINT i){ CountOfScreenShots = i; }
	void SetCurrentWizardPage(UINT i){ CurrentWizardPage = i; }
	void SetCountOfWizardPages(UINT i){ CountOfWizardPages = i; }
	

	LPCSTR GetFullModuleFileName(void){ return FullModuleFileName.c_str(); }
	LPCSTR GetModuleFilePath(void){ return ModuleFilePath.c_str(); }
	LPCSTR GetLayoutFileName(void){ return LayoutFileName.c_str(); }
	LPCSTR GetInstallPath(void){ return InstallPath.c_str(); }
	LPCSTR GetMainDrive(void){ return MainDrive.c_str(); }

	UINT GetCurrentScreenShot(void){ return CurrentScreenShot; }
	UINT GetCountOfScreenShots(void){ return CountOfScreenShots; }
	UINT GetCurrentWizardPage(void){ return CurrentWizardPage; }
	UINT GetCountOfWizardPages(void){ return CountOfWizardPages; }
*/
//	void SHParseString(std::string *string, std::string *rvalue);
	

protected:
	
private:
	STRINGLIST InputStringList;
	STRINGLIST OutputStringList;
	LABELLIST LabelList;
	
	PROGRESSBARLIST ProgressBarList;
	STRINGLIST ProgressBarActionList;

	/*//paths
	std::string FullModuleFileName;
	std::string ModuleFilePath;
	std::string LayoutFileName;
	std::string InstallPath;
	std::string MainDrive;
	UINT CurrentScreenShot;
	UINT CountOfScreenShots;
	UINT CurrentWizardPage;
	UINT CountOfWizardPages;
	*/
};


class SDLInstallerPage
{
private:
	WIDGETLIST WidgetList;
	LPCSTR Name;
	DWORD Index;
public:
    PG_Widget *GetWidget(LPCSTR name);
	LPCSTR GetName(void) {return Name;}
	void SetName(LPCSTR name){Name = name;}
	DWORD GetCount(void);
	void AddWidget(PG_Widget* widget);
	void DeleteWidget(LPCSTR name);
	void ShowWidget(LPCSTR name, bool alpha = false);
	void HideWidget(LPCSTR name, bool alpha = false);
	void ShowAll(bool alpha = false);
	void HideAll(bool alpha = false);
	void SetIndex(DWORD index){ Index = index;}
	DWORD GetIndex(void){ return Index;}
	SDLInstallerPage(LPCSTR name);
	~SDLInstallerPage();
};

class SDLInstallerWizard
{
private:
	//Wizard pages container.
	//This pages will be changed by "next_page","prev_page","first_page" and "last_page" actions.
	PAGELIST PageList;

    //All other pages which can be displayed by "'name'_page" ation.
	PAGELIST FrameList;

	//Screenshots. This will be displayed during installation progress.
	PAGELIST ScreenShotList;
  PAGELIST AnnounceList;

  WIDGETLIST AllWidgets;
  STRINGLIST AllWidgetNames;
  STRINGLIST AllWidgetActions;

	//Index of wizard page, which displayed now.
  DWORD CurrentPage;
	DWORD CurrentScreenShot;
  DWORD CurrentAnnounce;
public:
  //AllWidgetList
  void AddWidget(PG_Widget* widget, std::string name, std::string action)
  {AllWidgets.push_back(widget); AllWidgetNames.push_back(name); AllWidgetActions.push_back(action);}
  PG_Widget *GetWidgetByName(LPCSTR name);
  PG_Widget *GetWidgetByAction(LPCSTR action);
  PG_Widget *GetWidgetByNameAndAction(LPCSTR name, LPCSTR action);

  //PageList
	DWORD GetCountPages(void){return PageList.size();}
	SDLInstallerPage* GetPage(DWORD index);
	void AddPage(SDLInstallerPage* page);
	void ShowPage(LPCSTR name, bool alpha = false);
	void HidePage(LPCSTR name, bool alpha = false);
	void HideAllPages(void);
	
	
  void ShowNextPage(bool alpha = false);
	void ShowPreviousPage(bool alpha = false);
	void ShowFirstPage(bool alpha = false);
	void ShowLastPage(bool alpha = false);
	void SetCurrentPage(DWORD index){ CurrentPage = index;}
	DWORD GetCurrentPage(void){ return CurrentPage;}

	//ScreenShots
	DWORD GetCountScreenShots(void){return ScreenShotList.size();}
	SDLInstallerPage* GetScreenShot(DWORD index);
	void AddScreenShot(SDLInstallerPage* page);
	void ShowScreenShot(DWORD index, bool alpha = false);
	void HideScreenShot(DWORD index, bool alpha = false);
	void ShowNextScreenShot(bool alpha = false);
	void ShowPreviousScreenShot(bool alpha = false);
	void ShowFirstScreenShot(bool alpha = false);
	void ShowLastScreenShot(bool alpha = false);
	void SetCurrentScreenShot(DWORD index){ CurrentScreenShot = index;}
	DWORD GetCurrentScreenShot(void){ return CurrentScreenShot;}

  //Announce
  DWORD GetCountAnnounces(void){return AnnounceList.size();}
  SDLInstallerPage* GetAnnounce(DWORD index);
  void AddAnnounce(SDLInstallerPage* page);
  void ShowAnnounce(DWORD index, bool alpha = false);
  void HideAnnounce(DWORD index, bool alpha = false);
  void ShowNextAnnounce(bool alpha = false);
  void ShowPreviousAnnounce(bool alpha = false);
  void ShowFirstAnnounce(bool alpha = false);
  void ShowLastAnnounce(bool alpha = false);
  void SetCurrentAnnounce(DWORD index){ CurrentAnnounce = index;}
  DWORD GetCurrentAnnounce(void){ return CurrentAnnounce;}

	//Frames
	DWORD GetCountFrames(void){return FrameList.size();}
	SDLInstallerPage* GetFrame(DWORD index);
	void AddFrame(SDLInstallerPage* page);
	void ShowFrame(LPCSTR name, bool alpha = false);
	void HideFrame(LPCSTR name, bool alpha = false);

	SDLInstallerWizard();
	~SDLInstallerWizard();
};

//! SAX parser handler for loading xml gui files.
/*! Requires the xerxces-c dll and lots of include files.\n\n
  See http://xml.apache.org/xerces-c/index.html\n\n
  GUI Data is stored intnerally in the SAXElement class\n\n
  GUI Resources are added to the UI in the LoadResourceX functions\n\n
  GUI Elements are added in endElement\n\n
*/

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

class SDLInstallerLayout : public HandlerBase
{
public:
  //! constructor
  SDLInstallerLayout(LPCSTR XMLfilename, float KoeffX = 1, float KoeffY = 1);

  //! destructor
  ~SDLInstallerLayout();

  // -----------------------------------------------------------------------
  //  Implementations of the SAX DocumentHandler interface
  // -----------------------------------------------------------------------

  //! Called at the end of the XML document
  /*! Inform the user of any errors
  */
  virtual void endDocument();

  //! Called at the end of an element </element>
  /*! Based on type, add to parent
  */
  virtual void endElement(const XMLCh* const name);

  //! Called when characters exist between <element> and </element>
  /*! if element is a type that accepts characters as text, call SetText
  */
  virtual void characters(const XMLCh* const chars, const unsigned int length);

  //! Called at the beginning of the document
  /*! Initialize and clear the stack
  */
  virtual void startDocument();

  //! Called at the beginning of an element <element>
  /*! Create the SAXElement, call the correct LoadX
  */
  virtual void startElement(const XMLCh* const name, AttributeList& attributes);



  // -----------------------------------------------------------------------
  //  Implementations of the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  //! warning exception handler
  virtual void warning(const SAXParseException& exception);
  //! error exception handler
  virtual void error(const SAXParseException& exception);
  //! fatal error exception handler
  virtual void fatalError(const SAXParseException& exception);
  
private :
 
  SDLInstallerWizard *Wizard;
  SDL_DynamicControlsRefresh *DynamicControls;
  float FKoeffX;
  float FKoeffY;
  SDLInstallerPage* CurrentPage;
  SDLInstallerPage* CurrentScreenShot;
  SDLInstallerPage* CurrentAnnounce;
  SDLInstallerPage* CurrentFrame;

  PG_RadioButton* CurrentRBGroup;
  LPCSTR CurrentPageElement;
  BOOL ParsePageElementInProgress;

  //radiobutton group
  BOOL ParseRBGroupInProgress;

  
  BOOL ParseLocalisationInProgress;

  std::string XMLfilename;

  //! number of warnings
  unsigned long m_warnings;

  //! number of errors
  unsigned long m_errors;
public:
	SDLInstallerWizard* GetWizard(void){return Wizard;}
	SDL_DynamicControlsRefresh* GetDynamicControls(void){return DynamicControls;}

};


class DECLSPEC PG_MyButton : public PG_Button  {
public:

	PG_MyButton(PG_Widget* parent, int id, const PG_Rect& r, const char* text = NULL, const char* style="Button");

	virtual bool eventMouseMotion(const SDL_MouseMotionEvent* motion);


	virtual bool eventMouseButtonDown(const SDL_MouseButtonEvent* button);

	virtual bool eventMouseButtonUp(const SDL_MouseButtonEvent* button);
};


#endif //SDLINSTALLERLAYOUT
