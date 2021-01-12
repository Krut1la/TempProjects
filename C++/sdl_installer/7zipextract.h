#ifndef SZIPEXTRACT
#define SZIPEXTRACT

#include <windows.h>
#include <stdio.h>
#include <string>
#include <process.h>
#include <ctype.h>
#include <7zip.h>
#include <pglog.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>


DWORD Extract7zipArchive(LPSTR ArcName,int Mode, LPSTR DestPath);
void Cancel7zipExtract(void);
void User7zipCancel(void);
CALLBACK callback7zip(UINT msg,UINT64 UserData, UINT64 P1, LPCSTR P2);


#endif //SZIPEXTRACT
