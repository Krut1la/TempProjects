#ifndef ACEEXTRACT
#define ACEEXTRACT

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
void CancelAceExtract(void);
void UserAceCancel(void);
//int ExtractAceArchive(LPSTR ArcName,int Mode, PTSTR pszCabFile);
int     ExtractAceArchive(INT argc, LPSTR argv[]);
/*LRESULT WINAPI CabinetCallback ( IN PVOID pMyInstallData,
                     IN UINT Notification,
                     IN UINT Param1,
                     IN UINT Param2 );

*/
#endif //ACEEXTRACT
