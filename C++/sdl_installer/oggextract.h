#ifndef OGGEXTRACT
#define OGGEXTRACT

#include <windows.h>
#include <stdio.h>
#include <string>
#include <process.h>
#include <ctype.h>
#include <myogglib.h>
#include <pglog.h>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include "installer.h"


DWORD ExtractOggArchive(LPSTR ArcName,int Mode, LPSTR DestPath);
void CancelOggExtract(void);
void UserOggCancel(void);
DWORD __stdcall callbackOgg(UINT msg,UINT64 UserData, UINT64 P1, LPCSTR P2);
    
#endif //OGGEXTRACT