#ifndef INFOEXPANDER
#define INFOEXPANDER

#define CSIDL_DESKTOP                  0x0000       // <desktop>
#define CSIDL_INTERNET                 0x0001       // Internet Explorer (icon on desktop)
#define CSIDL_PROGRAMS                 0x0002       // Start Menu\Programs
#define CSIDL_CONTROLS                 0x0003       // My Computer\Control Panel
#define CSIDL_PRINTERS                 0x0004       // My Computer\Printers
#define CSIDL_PERSONAL                 0x0005       // My Documents
#define CSIDL_FAVORITES                0x0006       // <user name>\Favorites
#define CSIDL_STARTUP                  0x0007       // Start Menu\Programs\Startup
#define CSIDL_RECENT                   0x0008       // <user name>\Recent
#define CSIDL_SENDTO                   0x0009       // <user name>\SendTo
#define CSIDL_BITBUCKET                0x000a       // <desktop>\Recycle Bin
#define CSIDL_STARTMENU                0x000b       // <user name>\Start Menu
#define CSIDL_MYDOCUMENTS              0x000c       // logical "My Documents" desktop icon
#define CSIDL_MYMUSIC                  0x000d       // "My Music" folder
#define CSIDL_MYVIDEO                  0x000e       // "My Videos" folder
#define CSIDL_DESKTOPDIRECTORY         0x0010       // <user name>\Desktop
#define CSIDL_DRIVES                   0x0011       // My Computer
#define CSIDL_NETWORK                  0x0012       // Network Neighborhood
#define CSIDL_NETHOOD                  0x0013       // <user name>\nethood
#define CSIDL_FONTS                    0x0014       // windows\fonts
#define CSIDL_TEMPLATES                0x0015
#define CSIDL_COMMON_STARTMENU         0x0016       // All Users\Start Menu
#define CSIDL_COMMON_PROGRAMS          0X0017       // All Users\Start Menu\Programs
#define CSIDL_COMMON_STARTUP           0x0018       // All Users\Startup
#define CSIDL_COMMON_DESKTOPDIRECTORY  0x0019       // All Users\Desktop
#define CSIDL_APPDATA                  0x001a       // <user name>\Application Data
#define CSIDL_PRINTHOOD                0x001b       // <user name>\PrintHood
#define CSIDL_LOCAL_APPDATA            0x001c
// <user name>\Local Settings\Applicaiton Data (non roaming)
#define CSIDL_ALTSTARTUP               0x001d       // non localized startup
#define CSIDL_COMMON_ALTSTARTUP        0x001e       // non localized common startup
#define CSIDL_COMMON_FAVORITES         0x001f
#define CSIDL_INTERNET_CACHE           0x0020
#define CSIDL_COOKIES                  0x0021
#define CSIDL_HISTORY                  0x0022
#define CSIDL_COMMON_APPDATA           0x0023       // All Users\Application Data
#define CSIDL_WINDOWS                  0x0024       // GetWindowsDirectory()
#define CSIDL_SYSTEM                   0x0025       // GetSystemDirectory()
#define CSIDL_PROGRAM_FILES            0x0026       // C:\Program Files
#define CSIDL_MYPICTURES               0x0027       // C:\Program Files\My Pictures
#define CSIDL_PROFILE                  0x0028       // USERPROFILE
#define CSIDL_SYSTEMX86                0x0029       // x86 system directory on RISC
#define CSIDL_PROGRAM_FILESX86         0x002a       // x86 C:\Program Files on RISC
#define CSIDL_PROGRAM_FILES_COMMON     0x002b       // C:\Program Files\Common
#define CSIDL_PROGRAM_FILES_COMMONX86  0x002c       // x86 Program Files\Common on RISC
#define CSIDL_COMMON_TEMPLATES         0x002d       // All Users\Templates
#define CSIDL_COMMON_DOCUMENTS         0x002e       // All Users\Documents
#define CSIDL_COMMON_ADMINTOOLS        0x002f
// All Users\Start Menu\Programs\Administrative Tools
#define CSIDL_ADMINTOOLS               0x0030
// <user name>\Start Menu\Programs\Administrative Tools
#define CSIDL_CONNECTIONS              0x0031       // Network and Dial-up Connections
#define CSIDL_COMMON_MUSIC             0x0035       // All Users\My Music
#define CSIDL_COMMON_PICTURES          0x0036       // All Users\My Pictures
#define CSIDL_COMMON_VIDEO             0x0037       // All Users\My Video
#define CSIDL_RESOURCES                0x0038       // Resource Direcotry
#define CSIDL_RESOURCES_LOCALIZED      0x0039       // Localized Resource Direcotry

#define FORMAT_B   0x01
#define FORMAT_KB  0x02
#define FORMAT_MB  0x04
#define FORMAT_GB  0x08

#define ILT_NONE   0x00
#define ILT_XML    0x01
#define ILT_UNWISE 0x02 

#include <windows.h>
#include <Shlwapi.h> 
#include <shlobj.h>
#include <sdl.h>
#include <stack>
#include <list>
#include <time.h>
#include <pgwidget.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include "layout.h"
#include "resource.h"



// disable long class name warnings for STL
#pragma warning(disable : 4786)
#pragma warning(disable : 4788)

typedef std::list<std::string> LANGLIST;
typedef std::list<std::string> TEXTLIST;
typedef std::list<std::string> COMPONENTLIST;

typedef struct {
	std::string name;
	LANGLIST langlist;
	TEXTLIST textlist;
}SI_TextString;

typedef struct {
	std::string Title;
	std::string Name;

	COMPONENTLIST componentlist;

	//system
	std::string CPUGenerationNeed;

	DWORD CPUMHZNeed;

	std::string VideoGenerationNeed;

	DWORD RAMNeed;

	__int64 RequiredSize;

	DWORD DXVersionNeed;
}SI_GameInfo;

typedef std::list<SI_GameInfo*> GAMEINFOLIST;

typedef std::list<SI_TextString> TEXTSTRINGLIST;


typedef std::list<std::string> INSTALDRIVESLIST;
typedef std::list<BYTE> INSTALDISKNUMLIST;

bool GetFreeSpaceF(LPSTR buff, UINT bufflen, LPCTSTR disk, DWORD format);
bool GetRequiredSpace(LPSTR buff, UINT bufflen, DWORD format);
UINT GetFreeSpacePercent(void);
void SHReplaceCSIDL(DWORD start_sh, DWORD end_sh, DWORD clsid, std::string *str);
BOOL LoadScript( LPCSTR XMLfilename);
DWORD GetDllVersion(LPCTSTR lpszDllName);

typedef BOOL (WINAPI *pGetDiskFreeSpaceExF)(LPCTSTR lpDirectoryName,
											PULARGE_INTEGER lpFreeBytesAvailable,
											PULARGE_INTEGER lpTotalNumberOfBytes,
											PULARGE_INTEGER lpTotalNumberOfFreeBytes);



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

class InfoExpander;
class InfoExpander : public HandlerBase
{
public:
	//! constructor
	InfoExpander(LPCSTR XMLfilename);

	//! destructor
	~InfoExpander();

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
	std::string XMLfilename;

	//! number of warnings
	unsigned long m_warnings;

	//! number of errors
	unsigned long m_errors;

protected:


private:
	UINT ArchiveProgress;
	UINT FileProgress;

	std::string CurrentOperation;
public:

	HWND hWndMain;
	HWND hwndEasyInstaller;

	void InfoExpander::SetArchiveProgress(UINT ap);
	void InfoExpander::SetFileProgress(UINT fp);
	void InfoExpander::SetCurrentOperation(std::string co);
	UINT InfoExpander::GetArchiveProgress(void);
	UINT InfoExpander::GetFileProgress(void);
	std::string InfoExpander::GetCurrentOperation(void);

	std::string Title;
	std::string Caption;

	std::string FullModuleFileName;
	std::string ModuleFilePath;
	std::string LayoutFileName;
	std::string ThemeFileName;
	std::string InstallXMLFileName;
	std::string MainDir;
	std::string MainDrive;

	//hardware info
	std::string CPUString;
	std::string CPUGenerationNeed;
	DWORD CPUMHZ;
	DWORD CPUMHZNeed;

	std::string VideoString;
	std::string VideoGenerationNeed;

	DWORD RAMMB;
	DWORD RAMNeed;

	__int64 RequiredSize;
	__int64 FreeSize;

	DWORD DXVersion;
	DWORD DXVersionNeed;
	std::string dxpath;

	OSVERSIONINFO versionInfo;



	INSTALDRIVESLIST InstallDrivesList;
	INSTALDISKNUMLIST InstallDiskNumList;


	UINT CurrentScreenShot;
	UINT CountOfScreenShots;
	UINT CurrentAnnounce;
	UINT CountOfAnnounces;
	UINT CurrentWizardPage;
	UINT CountOfWizardPages;

	
	void ExpandString(std::string *string, std::string *rvalue);

	DWORD GetDllVersion(LPCTSTR lpszDllName);
	DWORD ShlwapiVersion;
	DWORD Shell32Version;
	DWORD Comctl32Version;

	//XMLLog for uninstaller
	HANDLE InstallLog;
	DWORD InstallLogType;
	std::string IL_Filename;

	TEXTSTRINGLIST TextStingList;

	GAMEINFOLIST gil;
	SI_GameInfo* CurrentGame;
	DWORD GamesCount;

	LPCSTR CurrentGameElement;
    BOOL ParseGameElementInProgress;

};

#endif //INFOEXPANDER
