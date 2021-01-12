#ifndef COMMON_H
#define COMMON_H

#define _WIN32_WINNT 0x0400 

//#pragma comment(lib, "krutila.lib")

#include "Plugin.h"
#include <windows.h>
#include <Winbase.h>
//#include <Winnetwk.h>
#include <sstream>
//#include <set>
#include <stack>
#include <utility>
#include <algorithm>
#include <krutila.h>
#include <kerror.h>
//#include <ktree.h>
#include "Farkeys.h"
#include "Farcolor.h"
#include "Language.h"

//Operation type
const DWORD AFOT_COPY								= 0x7001;
const DWORD AFOT_MOVE								= 0x7002;
const DWORD AFOT_DELETE								= 0x7003;
const DWORD AFOT_COMPARE							= 0x7004;
const DWORD AFOT_EXTRACT							= 0x7005;
const DWORD AFOT_SCAN								= 0x7006;

enum Flags{
	flgNone = 0,
	flgAsk = 1,
	flgOverwrite = 2,
	flgAppend = 4,
	flgRefresh = 8,
	flgSkip = 16,
	flgEvenReadOnly = 32,
	flgMakeReadOnly = 64,
	flgCopyAccessRights = 128,
	flgAbortOnError = 256,
	flgMove = 512,
	flgRecycle = 1024,
	flgMatch = 2048,
	flgDiff = 4096,
	flgModificationTime = 8192,
	flgCreationTime = 16384,
	flgCase = 32768,
	flgSize = 65536,
	flgContents = 131072
};



#define DIM(Item) (((Item).X2-(Item).X1+1)*((Item).Y2-(Item).Y1+1))

const int UCtlL = 19;
const int UCtlT = 2;
const int UCtlW = 59;
const int UCtlH = 23;
const int UCtlR = UCtlL + UCtlW - 1;
const int UCtlB = UCtlT + UCtlH - 1;
const int ExampleW = 59;
const int ExampleH = 20;

const int GrpCnt = 6;

const char PHRASE_COLORS[] = "Colors";
const char PHRASE_THEMES[] = "Themes";
const char PHRASE_INI[] = ".ini";

struct ColorGroupItem
{
  BYTE Data[ExampleH][ExampleW + 1];
  BYTE Mask[ExampleH][ExampleW + 1];
};

struct InitDialogItem
{
  int Type;
  BYTE X1;
  BYTE Y1;
  BYTE X2;
  BYTE Y2;
  unsigned int Flags;
  int Data;
};

/*void * __cdecl realloc(void *block, size_t size);
void __cdecl free(void *block);
void _pure_error_ ();
*/
const char *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber);

void SetItemTextA(HANDLE hDlg, int msg, int control, const char* data);
void SetItemTextW(HANDLE hDlg, int msg, int control, const wchar_t* data);

int ShowMessage(unsigned int Flags, const char *HelpTopic, int MsgID);

int MessageFileExistA(const char* filename);
int MessageFileExistW(const wchar_t* filename);

int MessageFileReadOnlyA(const char* filename);
int MessageFileReadOnlyW(const wchar_t* filename);

HANDLE OpenConsoleIn(void);
void WriteConsoleKey(HANDLE console,DWORD Key,DWORD State);
void GetFileAttr(const wchar_t *file, char *buffer, const char *format);

const char *GetRealName(const WIN32_FIND_DATAA *src);
bool GetWideNameDirect(const char *Root,const char *src,wchar_t *dest);
bool GetWideName(const char *Root,const WIN32_FIND_DATAA *src,wchar_t *dest);

std::wstring get_full_wide_name(const char* str);
std::string get_file(const std::string& filename);
std::string get_path(const std::string& filename);
wchar_t *TruncPathStrW(wchar_t *Str,int MaxLength);
void NormalizeNameW(int width,int msg,wchar_t *filename,wchar_t *dest);
void NormalizeName(int width,int msg,char *filename,char *dest);
void UNCPath(char *path);

std::vector<std::wstring> split(const std::wstring& source, wchar_t delim);

#endif // COMMON_H
