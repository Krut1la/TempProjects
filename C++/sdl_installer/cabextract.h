#ifndef CABEXTRACT
#define CABEXTRACT

#include <windows.h>
#include <stdio.h>
#include <string>
#include <process.h>
#include <ctype.h>
#include <unrar.h>
#include <pglog.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <Setupapi.h>

/*bool LoadUnRarLibrary(void);
void CancelExtract(void);
void UserCancel(void);
int ListArchive(char *ArcName);
void ShowComment(char *CmtBuf);
void OutHelp(void);
void OutOpenArchiveError(int Error,char *ArcName);
void ShowArcInfo(unsigned int Flags,char *ArcName);
void OutProcessFileError(int Error);
*/
void CancelCabExtract(void);
void UserCabCancel(void);
int ExtractCabArchive(LPSTR ArcName,int Mode, PTSTR pszCabFile);
LRESULT WINAPI CabinetCallback ( IN PVOID pMyInstallData,
                     IN UINT Notification,
                     IN UINT Param1,
                     IN UINT Param2 );


#endif //CABEXTRACT
