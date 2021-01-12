#include "Common.h"

extern HANDLE hHeap;
extern struct PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;

/*
#ifdef FROM_CMD
void * __cdecl realloc(void *block, size_t size)
{
  if (block)
    return HeapReAlloc(hHeap,HEAP_ZERO_MEMORY,block,size);
  else
    return HeapAlloc(hHeap,HEAP_ZERO_MEMORY, size);
}

void __cdecl free(void *block)
{
  HeapFree(hHeap, 0, block);
}

void _pure_error_ () {};
#endif*/

const char *GetMsg(int MsgId)
{
  return (Info.GetMsg(Info.ModuleNumber, MsgId));
}

void InitDialogItems(const struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber)
{
  const struct InitDialogItem *PInit = Init;
  struct FarDialogItem *PItem = Item;
  for (int i = 0; i < ItemsNumber; i++, PItem++, PInit++) {
    PItem->Type = PInit->Type;
    PItem->X1 = PInit->X1;
    PItem->Y1 = PInit->Y1;
    PItem->X2 = PInit->X2;
    PItem->Y2 = PInit->Y2;
    PItem->Focus = 0;
    PItem->Selected = 0;
    PItem->Flags = PInit->Flags;
    PItem->DefaultButton = 0;
    lstrcpyA(PItem->Data,PInit->Data != -1 ? GetMsg(PInit->Data) : "");
  }
}

void SetItemTextA(HANDLE hDlg, int msg, int control, const char* data)
{		
	//FarDialogItemData fdid;
	//fdid.PtrData = const_cast<char*>(data);
	//fdid.PtrLength = lstrlen(data);
	Info.SendDlgMessage(hDlg, msg, control, (long)data);
}

void SetItemTextW(HANDLE hDlg, int msg, int control, const wchar_t* data)
{
	//FarDialogItemData fdid;
	std::wstring wstr = data;
	std::string str = to_multibyte(wstr, std::locale(".OCP"));
	//fdid.PtrData = const_cast<char*>(str.c_str());
	//fdid.PtrLength = str.length();
	Info.SendDlgMessage(hDlg, msg, control, (long)str.c_str());
}

int ShowMessage(unsigned int Flags, const char *HelpTopic, int MsgID)
{
  const char *MsgItems[] = {GetMsg(STitle), GetMsg(MsgID)};
  return Info.Message(Info.ModuleNumber, Flags, HelpTopic,
    MsgItems, sizeof(MsgItems) / sizeof(MsgItems[0]), 1);
}


int MessageFileExistW(const wchar_t* filename)
{
	std::wstring wstr = filename;
	std::string str = to_multibyte(wstr, std::locale(".OCP"));
	return MessageFileExistA(str.c_str());
}

int MessageFileExistA(const char* filename)
{
	const char *Msg[9];

	Msg[0] = GetMsg(SMSGWarning);
	Msg[1] = GetMsg(SMSGWCFileExists);
    Msg[2] = filename;

	Msg[3] = GetMsg(SMSGWBOverwrite);
	Msg[4] = GetMsg(SMSGWBSkip);
	Msg[5] = GetMsg(SMSGWBRefresh);
	Msg[6] = GetMsg(SMSGWBAppend);
	Msg[7] = GetMsg(SMSGWBAbort);
	Msg[8] = "TestAll";

	return Info.Message(Info.ModuleNumber,
               FMSG_ERRORTYPE | FMSG_WARNING,
               NULL,
               Msg,
               sizeof(Msg)/sizeof(Msg[0]),
               6);

}

int MessageFileReadOnlyW(const wchar_t* filename)
{
	std::wstring wstr = filename;
	std::string str = to_multibyte(wstr, std::locale(".OCP"));
	return MessageFileReadOnlyA(str.c_str());
}

int MessageFileReadOnlyA(const char* filename)
{
	const char *Msg[6];

	Msg[0] = GetMsg(SMSGWarning);
	Msg[1] = GetMsg(SMSGWCFileReadOnly);
    Msg[2] = filename;

	Msg[3] = GetMsg(SMSGWBOverwrite);
	Msg[4] = GetMsg(SMSGWBSkip);
	Msg[5] = GetMsg(SMSGWBAbort);

	return Info.Message(Info.ModuleNumber,
               FMSG_ERRORTYPE | FMSG_WARNING,
               NULL,
               Msg,
               sizeof(Msg)/sizeof(Msg[0]),
               3);

}

HANDLE OpenConsoleIn(void)
{
  return CreateFileW(L"CONIN$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
}

void WriteConsoleKey(HANDLE console,DWORD Key,DWORD State)
{
  INPUT_RECORD event; DWORD written;
  event.EventType=KEY_EVENT;
  event.Event.KeyEvent.bKeyDown=TRUE;
  event.Event.KeyEvent.wRepeatCount=1;
  event.Event.KeyEvent.wVirtualKeyCode=(WORD)Key;
  event.Event.KeyEvent.wVirtualScanCode=0;
  event.Event.KeyEvent.uChar.UnicodeChar=0;
  event.Event.KeyEvent.dwControlKeyState=State;
  WriteConsoleInputW(console,&event,1,&written);
}

void GetFileAttr(const wchar_t *file, char *buffer, const char *format)
{
  unsigned long long size=0;
  SYSTEMTIME mod;
  memset(&mod,0,sizeof(SYSTEMTIME));
  buffer[0]=0;
  WIN32_FIND_DATAW find; HANDLE hFind;
  hFind=FindFirstFileW(file,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    FindClose(hFind);
    size=(unsigned long long)find.nFileSizeLow+(unsigned long long)find.nFileSizeHigh*4294967296ULL;
    FILETIME local;
    FileTimeToLocalFileTime(&find.ftLastWriteTime,&local);
    FileTimeToSystemTime(&local,&mod);
	sprintf(buffer, format, size,mod.wDay,mod.wMonth,mod.wYear,mod.wHour,mod.wMinute,mod.wSecond);
  }
}

const char *GetRealName(const WIN32_FIND_DATAA *src)
{
  WIN32_FIND_DATAA find,find_ok; HANDLE hFind; BOOL Res;
  hFind=FindFirstFileA(src->cFileName,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {
    memcpy(&find_ok,&find,sizeof(find));
    Res=FindNextFileA(hFind,&find);
    FindClose(hFind);
    if((!Res)&&(!_stricmp(src->cAlternateFileName,find_ok.cAlternateFileName)))
    {
      return src->cFileName;
    }
  }
  if(src->cAlternateFileName[0])
  {
    hFind=FindFirstFileA(src->cAlternateFileName,&find);
    if(hFind!=INVALID_HANDLE_VALUE)
    {
      FindClose(hFind);
      return src->cAlternateFileName;
    }
  }
  return NULL;
}

bool GetWideNameDirect(const char *Root,const char *src,wchar_t *dest)
{
  char FileNameA[MAX_PATH]; wchar_t FileNameW[MAX_PATH]; WIN32_FIND_DATAW find;
  strcpy(FileNameA,Root);
  strcat(FileNameA,src);
  MultiByteToWideChar(CP_OEMCP,0,FileNameA,-1,FileNameW,MAX_PATH);
  HANDLE hFind=FindFirstFileW(FileNameW,&find);
  if(hFind!=INVALID_HANDLE_VALUE)
  {

    FindClose(hFind);
    wcscpy(dest,find.cFileName);
  } else return false;
  return true;
}

bool GetWideName(const char *Root,const WIN32_FIND_DATAA *src,wchar_t *dest)
{
  const char *RealFileName=GetRealName(src);
  if(!RealFileName) return false;
  return GetWideNameDirect(Root,RealFileName,dest);
}

std::string get_path(const std::string& filename)
{
	std::string ret = filename;
	size_t last_slash = ret.find_last_of(TEXT('\\'), ret.length());
	if(last_slash == ret.npos)
		return "";
	ret.replace(last_slash, ret.length() - last_slash, TEXT(""));
	return ret;
}

std::string get_file(const std::string& filename)
{
	std::string ret = filename;
	size_t last_slash = ret.find_last_of(TEXT('\\'), ret.length());
	if(last_slash == ret.npos)
		return "";
	ret.replace(0, last_slash + 1, TEXT(""));
	return ret;
}

std::wstring get_full_wide_name(const char* str)
{		
	std::wstring ret;
	std::string path = str;
	if(path.length() < 2)
		return L"";

	size_t asd = path.length();
	if((path[path.length() - 1] == '\\') && path.length() > 3)
		path.erase(path.length() - 1, 1);

	for(;;)
	{		
		std::string tmp_path  = get_path(path);
		std::string tmp_file  = get_file(path);

		if(path[0] == '\\' && path[1] == '\\')
		{
			size_t first_slash = path.find_first_of(L'\\', 2);
			size_t second_slash = path.find_first_of(L'\\', first_slash + 1);
			if(second_slash == path.npos)
			{
				ret = to_unicode(path, std::locale(".OCP")) + L"\\" + ret;
				break;
			}
		}
		else if(path.length() == 3)
		{
			ret = to_unicode(path);
				break;
		}

		wchar_t widebuff[MAX_PATH];
		if(!GetWideNameDirect((tmp_path + "\\").c_str(), tmp_file.c_str(), widebuff))
			return L"";
		ret = widebuff + (L"\\" + ret);
		path = tmp_path;

		if(tmp_path.length() <= 3)
		{
			ret = to_unicode(tmp_path, std::locale(".OCP")) + L"\\" + ret;
			break;
		}
	}
	return ret;
}

#define MY_COMBINE_PATH \
FSF.AddEndSlash(oem_buff); \
strcpy(res_buff,oem_buff); \
strcat(res_buff,path+3); \
strcpy(path,res_buff);
void UNCPath(char *path)
{
  char buff[MAX_PATH],oem_buff[MAX_PATH],res_buff[MAX_PATH],device[3]=" :"; DWORD size;
  for(int i=0;i<2;i++)
  {
    if((strlen(path)>2)&&(path[1]==':'))
    {
      device[0]=path[0]; size=sizeof(buff);
      if(WNetGetConnection(device,buff,&size)==NO_ERROR)
      {
        CharToOem(buff,oem_buff);
        MY_COMBINE_PATH
      }
      else if(QueryDosDevice(device,buff,sizeof(buff)))
        if(!strncmp(buff,"\\??\\",4))
        {
          CharToOem(buff+4,oem_buff);
          MY_COMBINE_PATH
        }
    }
  }
  //extract symlinks only for local disk
  if((strlen(path)>2)&&(path[1]==':'))
    if((unsigned int)FSF.ConvertNameToReal(path,NULL,0)<sizeof(buff))
    {
      FSF.ConvertNameToReal(path,buff,sizeof(buff));
      strcpy(path,buff);
    }
  for(unsigned int i=0;i<strlen(path);i++) if(path[i]=='/') path[i]='\\';
}
#undef MY_COMBINE_PATH

void NormalizeName(int width,int msg,char *filename,char *dest)
{
  int msg_len=strlen(GetMsg(msg))-2;
  char truncated[MAX_PATH];
  strcpy(truncated,filename);
  FSF.TruncPathStr(truncated,width-msg_len);
  sprintf(dest,GetMsg(msg),truncated);
}

void NormalizeNameW(int width,int msg,wchar_t *filename,wchar_t *dest)
{
  int msg_len=strlen(GetMsg(msg))-2;
  wchar_t truncated[MAX_PATH],mask[MAX_PATH];
  MultiByteToWideChar(CP_OEMCP,0,GetMsg(msg),-1,mask,sizeof(mask));
  wcscpy(truncated,filename);
  TruncPathStrW(truncated,width-msg_len);
  swprintf(dest,mask,truncated);
}

wchar_t *TruncPathStrW(wchar_t *Str,int MaxLength)
{
  if(Str&&MaxLength>3)
  {
    int init_len=wcslen(Str);
    if(init_len>MaxLength)
    {
      wchar_t *ptr=wcschr(Str,'\\');
      if(ptr==Str&&Str[1]=='\\')
      {
        ptr++;
        for(int i=0;i<2;i++)
          if(ptr)
          {
            ptr++;
            ptr=wcschr(ptr,'\\');
          }
          else break;
      }
      if(ptr&&(MaxLength-(ptr-Str)-1)>3)
        ptr++;
      else
        ptr=Str;
      for(int i=0;i<3;i++,ptr++) *ptr='.';
      int copy_len=MaxLength-(ptr-Str)+1;
      for(int i=0;i<copy_len;i++,ptr++) *ptr=*(ptr+init_len-MaxLength);
    }
  }
  return Str;
}

std::vector<std::wstring> split(const std::wstring& source, wchar_t delim) 
{
    std::vector<std::wstring> result;

    std::basic_istringstream<wchar_t> stream(source);
    std::wstring str;

    while (std::getline(stream, str, delim))
    {
        result.push_back(str);
    }

    return result;
}