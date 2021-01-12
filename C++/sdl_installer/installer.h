#ifndef INSTALLER
#define INSTALLER

// disable long class name warnings for STL
#pragma warning(disable : 4786)
#pragma warning(disable : 4788)

#include <windows.h>
#include <WinUser.h>
#include <Shlwapi.h> 
#include <Shlobj.h> 
#include <stack>
#include <list>
#include <time.h>
#include <unrar.h>
#include <unace.h>
#include "rarextract.h"
#include "aceextract.h"
#include "cabextract.h"
#include "7zipextract.h"
#include "oggextract.h"
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>

//BOOL Install( LPCSTR XMLfilename);
//DWORD WINAPI ThreadProc(LPVOID lpParameter);
unsigned _stdcall ThreadProc(void *argv);
BOOL CreateDirectoryExEx(LPCSTR path);

typedef BOOL (WINAPI *pGetFileSizeExF)(HANDLE hFile,
                                     PLARGE_INTEGER lpFileSize);

typedef DWORD (*oggextractF)(LPCSTR archive, LPCSTR destpath, UNOGGCALLBACK callback);

typedef void (*ExtractF)(DWORD Count);

typedef struct {
	UINT Progress;
	UINT CurrentOperationProgress;
	std::string CurrentOperation;
}INSTALLSTATE;

typedef std::list<std::string> COMPONENTLIST;
typedef std::list<BOOL> CONFIRMATIONLIST;

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

class Installer;
class Installer : public HandlerBase
{
public:
  //! constructor
  Installer(LPCSTR XMLfilename);

  //! destructor
  ~Installer();

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

public:
  std::string CurrentComponentParse;
  BOOL IsCurrentComponentInstall;
  COMPONENTLIST ComponentList;
  CONFIRMATIONLIST ConfirmationList;
	BOOL GetState(INSTALLSTATE &is);
};

#endif //INSTALLER
