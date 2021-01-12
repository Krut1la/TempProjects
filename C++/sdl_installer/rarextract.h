#ifndef RAREXTRACT
#define RAREXTRACT

#include <windows.h>
#include <stdio.h>
#include <string>
#include <process.h>
#include <ctype.h>
#include <unrar.h>
#include <pglog.h>
#include <pgwidget.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
//#define STRICT

enum { EXTRACT, TEST, PRINT };

bool LoadUnRarLibrary(void);
int ExtractArchive(LPSTR ArcName,int Mode, LPSTR DestPath);
void CancelExtract(void);
void UserCancel(void);
int ListArchive(char *ArcName);
void ShowComment(char *CmtBuf);
void OutHelp(void);
void OutOpenArchiveError(int Error,char *ArcName);
void ShowArcInfo(unsigned int Flags,char *ArcName);
void OutProcessFileError(int Error);
int CALLBACK CallbackProc(UINT msg,LONG UserData,LONG P1,LONG P2);


#endif //RAREXTRACT
