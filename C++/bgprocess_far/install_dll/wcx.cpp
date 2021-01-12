#include "../include/wcx.h"

using namespace install::wcx;

//static SEnum WcxList;
//static WCXArchive* wcxArchive;

//static bool WCXLoaded = false;

static TCHAR ModuleName[MAX_PATH * 2];
static TCHAR ModulePath[MAX_PATH * 2];

WCXModule::WCXModule(const kru_string& path, const kru_string& name)
{
	Open(path, name);
}

void WCXModule::Open(const kru_string& path, const kru_string& name)
{
	Constructed = false;
	hModule = 0;
	caps = 0;

	if((hModule = LoadLibrary(path.c_str())) == 0)
		return;

	this->name = name;

	OpenArchiveA			= reinterpret_cast<LPOPEN_ARCHIVEA>(GetProcAddress(hModule, "OpenArchive"));
	OpenArchiveW			= reinterpret_cast<LPOPEN_ARCHIVEW>(GetProcAddress(hModule, "OpenArchiveW"));
	CloseArchive			= reinterpret_cast<LPCLOSE_ARCHIVE>(GetProcAddress(hModule, "CloseArchive"));
	ReadHeaderA				= reinterpret_cast<LPREAD_HEADERA>(GetProcAddress(hModule, "ReadHeader"));
	ReadHeaderW				= reinterpret_cast<LPREAD_HEADERW>(GetProcAddress(hModule, "ReadHeaderW"));
	ProcessFileA			= reinterpret_cast<LPPROCESS_FILEA>(GetProcAddress(hModule, "ProcessFile"));
	ProcessFileW			= reinterpret_cast<LPPROCESS_FILEW>(GetProcAddress(hModule, "ProcessFileW"));
	PackFilesA				= reinterpret_cast<LPPACK_FILESA>(GetProcAddress(hModule, "PackFiles"));
	PackFilesW				= reinterpret_cast<LPPACK_FILESW>(GetProcAddress(hModule, "PackFilesW"));
	DeleteFilesA			= reinterpret_cast<LPDELETE_FILESA>(GetProcAddress(hModule, "DeleteFiles"));
	DeleteFilesW			= reinterpret_cast<LPDELETE_FILESW>(GetProcAddress(hModule, "DeleteFilesW"));
	SetChangeVolProc		= reinterpret_cast<LPSET_CHANGE_VOL_PROC>(GetProcAddress(hModule, "SetChangeVolProc"));
	SetChangeVolProcW		= reinterpret_cast<LPSET_CHANGE_VOL_PROCW>(GetProcAddress(hModule, "SetChangeVolProcW"));
	SetProcessDataProc		= reinterpret_cast<LPSET_PROCESS_DATA_PROC>(GetProcAddress(hModule, "SetProcessDataProc"));
	SetProcessDataProcW		= reinterpret_cast<LPSET_PROCESS_DATA_PROCW>(GetProcAddress(hModule, "SetProcessDataProcW"));
	ConfigurePacker			= reinterpret_cast<LPCONFIGURE_PACKER>(GetProcAddress(hModule, "ConfigurePacker"));
	GetPackerCaps			= reinterpret_cast<LPGET_PACKER_CAPS>(GetProcAddress(hModule, "GetPackerCaps"));
	CanYouHandleThisFileA	= reinterpret_cast<LPCAN_YOU_HANDLE_THIS_FILEA>(GetProcAddress(hModule, "CanYouHandleThisFile"));
	CanYouHandleThisFileW	= reinterpret_cast<LPCAN_YOU_HANDLE_THIS_FILEW>(GetProcAddress(hModule, "CanYouHandleThisFileW"));

	if(!(OpenArchiveA || OpenArchiveW)||
	   !(CloseArchive) ||
	   !(ReadHeaderA || ReadHeaderW) ||
	   !(ProcessFileA || ProcessFileW))
	{
		FreeLibrary(hModule);
		hModule = 0;
		return;
	}

	if(GetPackerCaps)
		caps = GetPackerCaps();

	Constructed = true;
}

WCXModule::~WCXModule()
{
	if(hModule)
		FreeLibrary(hModule);
}

bool WCXModule::SupportCommand(const kru_string& arcCommand)
{
	static ArcCommands arcCommands;
	return SupportCommand(arcCommands[arcCommand]);
}

bool WCXModule::SupportCommand(ArcCommand arcCommand)
{		
	switch(arcCommand)
	{
	case acExtract:
	case acExtractNoPath:
	case acTest:
		return true;
	case acDel:
		if((DeleteFilesA || DeleteFilesW) && !GetPackerCaps ||
			GetPackerCaps && caps & PK_CAPS_DELETE )
			return true;
		else
			return false;
	case acAddFiles:
	case acAddFilesAndFolders:
	case acMoveFiles:
	case acMoveFilesAndFolders:
		if((PackFilesA || PackFilesW) && !GetPackerCaps ||
			GetPackerCaps && caps & (PK_CAPS_NEW | PK_CAPS_MODIFY) )
			return true;
		else
			return false;
	}
	return false;
}

DWORD UpdateWcxList(WCXModuleList& wcxModuleList)
{
	HRESULT hr = S_OK;

	FILELIST stringList;

	hr = install::file::ScanFolder(stringList, TEXT(".\\WCX\\*.wcx"));

	if(hr != S_OK)
		return hr;

	//TCHAR mask[MAX_PATH * 2];
	//lstrcat(lstrcpy(mask, MAX_PATH * 2), TEXT("WCX\\*.wcx"));

	//WIN32_FIND_DATA find_data;

	//HANDLE find = FindFirstFile( mask, &find_data );
	//if( find == INVALID_HANDLE_VALUE )
	//return false;

	/*do
	{
		bool present = false;
		TCHAR path[sizeof( ModulePath )];
		lstrcat( lstrcat( lstrcpy( path, ModulePath ), TEXT("WCX\\")), find_data.cFileName );
		for( int i = 0; i < list->num; i++ )
			if( !lstrcmpi( list->wcx[i].name, find_data.cFileName ) )
			{
				present = true;
				break;
			}
			if( !present )
			{
//				DebugString( path );
				SWcx* wcx = LoadWcx( path, find_data.cFileName );
				if( wcx )
				{
//					DebugString( "loaded" );
					list->wcx = (SWcx*)realloc( list->wcx, sizeof( list->wcx[0] ) * (list->num + 1) );
					list->wcx[list->num] = *wcx;
					list->num++;
					free( wcx );
				}
			}
	}while( FindNextFile( find, &find_data ) );

	FindClose( find );
*/
	return hr;
}

static int __stdcall ChangeVolProc(char*, int) {return 1;}
static int __stdcall ChangeVolProcW(wchar_t*, int) {return 1;}
static int __stdcall ProcessDataProc(char* str, int bb)
{
	return 1;
}
static int __stdcall ProcessDataProcW(wchar_t* str, int bb)
{
	return 1;
}

template<class T,class A>
std::wstring to_unicode(
 std::basic_string<char,T,A> const& in,
 std::locale const& loc = std::locale())
{
 typedef std::codecvt<wchar_t,char,std::mbstate_t> facet;
 const facet& cvt = std::use_facet<facet>(loc);

 std::wstring out;
 out.reserve(in.length());

 facet::state_type state = facet::state_type();
 const char *ibuf = in.data(), *iend = in.data() + in.size();
 while(ibuf != iend)
 {
  wchar_t obuf[256], *oend;
  facet::result res = cvt.in(state,
   ibuf, iend, ibuf, obuf, obuf+256, oend=obuf);
  if( res == facet::error )
   { state = facet::state_type(); ibuf += 1; }
  out.append(obuf, oend - obuf);
  if( res == facet::error )
   out += L'?';
 }
 return out;
}

template<class T,class A>
std::string to_multibyte(
 std::basic_string<wchar_t,T,A> const& in,
 std::locale const& loc = std::locale())
{
 typedef std::codecvt<wchar_t,char,std::mbstate_t> facet;
 facet const& cvt = std::use_facet<facet>(loc);

 std::string out;
 out.reserve(in.length());

 facet::state_type state = facet::state_type();
 const wchar_t *ibuf = in.data(), *iend = in.data() + in.size();
 char obuf[256], *oend;
 while(ibuf != iend)
 {
  facet::result res = cvt.out(state,
   ibuf, iend, ibuf, obuf, obuf+256, oend=obuf);
  if( res == facet::error )
   { state = facet::state_type(); ibuf += 1; }
  out.append(obuf, oend - obuf);
  if( res == facet::error )
   out += L'?';
 }
 if( state == facet::partial ) {
  cvt.unshift(state, obuf, obuf+256, oend=obuf);
  out.append(obuf, oend - obuf);
 }
 return out;
}

WCXArchive::WCXArchive(
const kru_string& name,
const kru_string& type,
bool test,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	Constructed = false;

	handle = 0;

	this->callback = callback;
	this->userdata = userdata;

	kru_string path = TEXT(".\\WCX\\");
	path += type;
	path += TEXT(".wcx");

	wcxModule.Open(path, type);
		
	if(!wcxModule.Constructed)
		return;

	if(wcxModule.caps & PK_CAPS_UNICODE)
	{
		OpenArchiveDataW archiveData;
		ZeroMemory(&archiveData, sizeof(OpenArchiveDataW));
		archiveData.ArcName = name.c_str();
		archiveData.OpenMode = PK_OM_EXTRACT;
		if((handle = wcxModule.OpenArchiveW(&archiveData)) == 0)
			return;

		if(wcxModule.SetProcessDataProcW)
			wcxModule.SetProcessDataProcW(handle, ProcessDataProcW);
		if(wcxModule.SetChangeVolProcW)
			wcxModule.SetChangeVolProcW(handle, ChangeVolProcW);
	}
	else
	{
		OpenArchiveDataA archiveData;
		ZeroMemory(&archiveData, sizeof(OpenArchiveDataA));

		mbName = to_multibyte(name);
		//std::copy(mbName.begin(), mbName.end(), archiveData.ArcName);
		archiveData.ArcName = mbName.c_str();
		archiveData.OpenMode = PK_OM_EXTRACT;

		if((handle = wcxModule.OpenArchiveA(&archiveData)) == 0)
			return;

		if(wcxModule.SetProcessDataProc)
			wcxModule.SetProcessDataProc(handle, ProcessDataProc);
		if(wcxModule.SetChangeVolProc )
			wcxModule.SetChangeVolProc(handle, ChangeVolProc);


		/*if(!test)
		{		
			Constructed = true;
			return;
		}

		HeaderDataA headerData;
		ZeroMemory(&headerData, sizeof(HeaderDataA));

		std::copy(mbName.begin(), mbName.end(), headerData.ArcName);

		int res = wcxModule.ReadHeaderA(handle, &headerData);
		if(res != 10 && res != 22 && res)
		{
			wcxModule.CloseArchive(handle);
			return;
		}

		wcxModule.CloseArchive(handle); // reset enumerator
		if((handle = wcxModule.OpenArchiveA(&archiveData)) == 0)
			return;
		if(wcxModule.SetProcessDataProc)
			wcxModule.SetProcessDataProc(handle, ProcessDataProc);
		if(wcxModule.SetChangeVolProc)
			wcxModule.SetChangeVolProc(handle, ChangeVolProc);
			*/
	}

	Constructed = true;
}

WCXArchive::~WCXArchive()
{	
	if(handle)
		wcxModule.CloseArchive(handle);
}

STRINGLIST split(const kru_string& source, TCHAR delim = '\n' ) 
{
    STRINGLIST result;

    std::basic_istringstream<TCHAR> stream(source);
    kru_string str;

    while (std::getline(stream, str, delim))
    {
        result.push_back(str);
    }

    return result;
}


DWORD WCXArchive::Extract(
const TCHAR* files,
const TCHAR* sub_path,
const TCHAR* destination,
const TCHAR* flags,
bool path,
bool test)
{	
	HRESULT hr = S_OK;

	if(!Constructed)
		return ERROR_OBJECT_NOT_FOUND;

	kru_string _destination(destination);
	kru_string dest = (_destination[_destination.length() - 1] != TEXT('\\'))?(_destination + TEXT("\\")):_destination;
	
	STRINGLIST fileList = split(files, TEXT('|'));

	//for(STRINGLIST::iterator i_file = fileList.begin(); i_file != fileList.end(); ++i_file)
	for(;;)
	{
		if(wcxModule.caps & PK_CAPS_UNICODE)
		{
			int result = 0;
			HeaderDataW headerData;

			for(;;)
			{
				ZeroMemory(&headerData, sizeof(HeaderDataW));
				std::copy(name.begin(), name.end(), headerData.ArcName);

				result = wcxModule.ReadHeaderW(handle, &headerData);
				if(result == E_END_ARCHIVE)
					return S_OK;

				if(result)
				{
					if(callback)
					{
						switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
						{
						case ansAbort:	return file::chkAbort;
						case ansSkip:	return file::chkSkip;
						case ansRetry:	break;
						default:		return file::chkError;
						}//switch UnknownError Answer
					}
					else
						return GetLastError();
				}
				else
					break;
			}

			kru_string dir;
			if(headerData.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				dir = dest + headerData.FileName;
			}
			else
			{
				dir = dest + headerData.FileName;
				size_t slash = dir.find_last_of(TEXT("\\"), dir.length());
				dir = dir.replace(slash + 1, dir.length() - slash - 1, TEXT(""));
			}

			for(;;)
			{
				hr = file::CreateDirectoryExEx(dir.c_str());
				if(hr != S_OK)
				{
					if(callback)
					{
						switch(callback(msgRequest, NULL, reqUnknownError, hr, userdata))
						{
						case ansAbort:	return file::chkAbort;
						case ansSkip:	return file::chkSkip;
						case ansRetry:	break;
						default:		return file::chkError;
						}//switch UnknownError Answer
					}
					else
						return hr;
				}
				else
					break;
			}// loop until the problem is solved

			DWORD dwFlags = file::CopyFlagsToDWORD(flags);

			kru_string destFileName = dest + headerData.FileName;

			switch(file::FileExist(destFileName.c_str(), dwFlags, callback, userdata))
			{
			case file::chkAbort:		return E_ABORT;
			case file::chkSkip:			return S_OK;
			case file::chkContinue:	
				switch(file::FileReadOnly(destFileName.c_str(), dwFlags, callback, userdata))
				{
				case file::chkAbort:	return E_ABORT;
				case file::chkSkip:		return S_OK;
				case file::chkContinue:	break;
				case file::chkError:	return GetLastError();
				default:				return S_FALSE;
				}
				break;
			case file::chkError:

				if(	GetLastError() != ERROR_FILE_NOT_FOUND && 
					GetLastError() != ERROR_PATH_NOT_FOUND)
					return GetLastError();
				else
					break;
			default:			return S_FALSE;
			}

			result = wcxModule.ProcessFileW(handle, test ? PK_TEST : PK_EXTRACT,
				0, test ? 0 : destFileName.c_str());
			if(result != 0 && result != E_END_ARCHIVE)
			{
				return S_FALSE;
			}
			
		}
		else
		{
			int result = 0;
			HeaderDataA headerData;

			for(;;)
			{
				ZeroMemory(&headerData, sizeof(HeaderDataA));
				std::copy(mbName.begin(), mbName.end(), headerData.ArcName);

				result = wcxModule.ReadHeaderA(handle, &headerData);
				if(result == E_END_ARCHIVE)
					return S_OK;

				if(result)
				{
					if(callback)
					{
						switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
						{
						case ansAbort:	return file::chkAbort;
						case ansSkip:	return file::chkSkip;
						case ansRetry:	break;
						default:		return file::chkError;
						}//switch UnknownError Answer
					}
					else
						return GetLastError();
				}
				else
					break;
			}
			
			kru_string dir;
			if(headerData.FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				dir = (dest + to_unicode(std::string(headerData.FileName))).c_str();
			}
			else
			{
				dir = dest + to_unicode(std::string(headerData.FileName));
				size_t slash = dir.find_last_of(TEXT("\\"), dir.length());
				dir = dir.replace(slash + 1, dir.length() - slash - 1, TEXT(""));
			}
			{
				for(;;)
				{					
					hr = file::CreateDirectoryExEx(dir.c_str());
					if(hr != S_OK)
					{
						if(callback)
						{
							switch(callback(msgRequest, NULL, reqUnknownError, hr, userdata))
							{
							case ansAbort:	return file::chkAbort;
							case ansSkip:	return file::chkSkip;
							case ansRetry:	break;
							default:		return file::chkError;
							}//switch UnknownError Answer
						}
						else
							return hr;
					}
					else
						break;
				}// loop until the problem is solved

				DWORD dwFlags = file::CopyFlagsToDWORD(flags);

				std::string destFileName = to_multibyte(dest) + headerData.FileName;

				switch(file::FileExist((dest + to_unicode(std::string(headerData.FileName))).c_str(), dwFlags, callback, userdata))
				{
				case file::chkAbort:		return E_ABORT;
				case file::chkSkip:			return S_OK;
				case file::chkContinue:	
					switch(file::FileReadOnly((dest + to_unicode(std::string(headerData.FileName))).c_str(), dwFlags, callback, userdata))
					{
					case file::chkAbort:	return E_ABORT;
					case file::chkSkip:		return S_OK;
					case file::chkContinue:	break;
					case file::chkError:	return GetLastError();
					default:				return S_FALSE;
					}
					break;
				case file::chkError:
					if(	GetLastError() != ERROR_FILE_NOT_FOUND && 
						GetLastError() != ERROR_PATH_NOT_FOUND)
						return GetLastError();
					else
						break;
				default:			return S_FALSE;
				}
				
				result = wcxModule.ProcessFileA(handle, test ? PK_TEST : PK_EXTRACT,
					0, test ? 0 : destFileName.c_str());
				if(result != 0 && result != E_END_ARCHIVE)
				{
					return S_FALSE;
				}
			}
		}
	}

	return hr;
/*
	int fnum = 0;
	int len = (int)strlen(list);
	for(int pos = 0; pos <= len; pos++)
	{
		if(list[pos] == 124) fnum++;
	}
	files = (char**)malloc(sizeof(char*)*fnum);

	fnum = 0;
	int start = 0;
	for(int pos = 0; pos <= len; pos++)
	{
		if(list[pos] == 124) 
		{
			files[fnum] = (char*)malloc(pos - start + 2);
			//files[fnum] = (char*)new char[pos - start + 1];
			memcpy(files[fnum], (char*)(list + start), pos - start);
			*(files[fnum] + pos - start + 1) = 0;
			start = pos + 1;
			fnum++;
		}

	}
	//	free(files[0]);
	//delete []files[0];




	if( !fnum )
		return 1;

	bool empty = false;

	int result = 0;

	while( !empty )
	{
		tHeaderData data;
		ZeroMemory( &data, sizeof( data ) );
		lstrcpy( data.ArcName, arc->name );

		int result = arc->wcx->ReadHeader( arc->arc, &data );
		if( result )
		{
			FreeArgv( fnum, files );
			return 0;
		}
		Canonic( data.FileName );

		int i;
		char dest_name[MAX_PATH * 2];
		for( i = 0; i < fnum; i++ )
		{
			if( (path || !(data.FileAttr & FILE_ATTRIBUTE_DIRECTORY)) &&
				data.FileName && Match( data.FileName, data.FileName, sub_path, dest_name ) )
			{
				char dest_path[MAX_PATH * 2];
				if( path )
					lstrcat( lstrcat( lstrcpy( dest_path, dest ), "\\"), dest_name );
				else
				{
					lstrcpy( dest_path, dest );
					char* sym = lstrrchr( data.FileName, '\\' );
					lstrcat( lstrcat( dest_path, sym ? "" : "\\" ), sym ? sym : data.FileName );
				}
				//if( data.FileAttr & FILE_ATTRIBUTE_DIRECTORY )
				//    *lstrrchr( dest_path, '\\' ) = 0;

				DebugString( (data.FileAttr & FILE_ATTRIBUTE_DIRECTORY) ?
					"extract directory from: " : "extract from: " );
				DebugString( data.FileName );
				DebugString( (data.FileAttr & FILE_ATTRIBUTE_DIRECTORY) ?
					"extract directory to: " : "extract to: " );
				DebugString( dest_path );
				if( !CreateDir( dest_path,
					(data.FileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0 ) )
				{
					FreeArgv( fnum, files );
					return 1;
				}
				if( !(data.FileAttr & FILE_ATTRIBUTE_DIRECTORY) )
				{
					int res = arc->wcx->ProcessFile( arc->arc, test ? PK_TEST : PK_EXTRACT,
						0, test ? 0 : dest_path );
					if( res != 0 && res != E_END_ARCHIVE )
					{
						result = res;
						DebugString( "Detected error in extraction\n" );
					}

					DebugString( "Extracted, setting permissions...\n" );
					if( !test )
					{
						HANDLE f = CreateFile( dest_path, FILE_WRITE_ATTRIBUTES,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							0, OPEN_EXISTING, 0, 0 );
						if( f != INVALID_HANDLE_VALUE )
						{
							FILETIME filetime;
							FILETIME ftime;
							DosDateTimeToFileTime( (WORD)((DWORD)data.FileTime >> 16), (WORD)data.FileTime, &filetime );
							LocalFileTimeToFileTime( &filetime, &ftime );
							SetFileTime( f, &ftime, &ftime, &ftime );
							CloseHandle( f );
						}
						SetFileAttributes( dest_path, data.FileAttr );
					}
				}
				else
					arc->wcx->ProcessFile( arc->arc, PK_SKIP, 0, 0 );

				if( !(data.FileAttr & FILE_ATTRIBUTE_DIRECTORY) &&
					!lstrcmpi( files[i], dest_name ) )
				{
					free( files[i] );
					files[i] = 0;
				}
				empty = true;
				for( int k = 0; k < fnum; k++ )
					if( files[k] )
					{
						empty = false;
						break;
					}

					break;
			}
		}
		if( i >= fnum )
		{
			DebugString( data.FileName );
			DebugString( " skipping" );
			arc->wcx->ProcessFile( arc->arc, PK_SKIP, 0, 0 );
		}
	}

	FreeArgv( fnum, files );
	return result;*/
}

/*WCXArchive::Open()
{
}*/


/*
static SArc* OpenArchive( const TCHAR* name, SWcx* wcx, bool test = false )
{
//	DebugString( "OpenArchive" );
	if( !name || !wcx  )
		return 0;

	SArc arc;
	arc.wcx = wcx;
	lstrcpy( arc.name, name );

	tOpenArchiveData arc_data;
	ZeroMemory( &arc_data, sizeof( arc_data ) );

	arc_data.ArcName = (char*)name;
	arc_data.OpenMode = PK_OM_EXTRACT;

	if( (arc.arc = wcx->OpenArchive( &arc_data )) == 0 )
		return 0;

	if( wcx->SetProcessDataProc )
		wcx->SetProcessDataProc( arc.arc, ProcessDataProc );
	if( wcx->SetChangeVolProc )
		wcx->SetChangeVolProc( arc.arc, ChangeVolProc );

	if( !test )
	{
		SArc* result = (SArc*)malloc( sizeof( *result ) );
//		assert( result );
		*result = arc;
//		DebugString( "opened" );
		return result;
	}

	tHeaderData data;
	ZeroMemory( &data, sizeof( data ) );
	lstrcpy( data.ArcName, name );
	int res = wcx->ReadHeader( arc.arc, &data );
	if( res != 10 && res != 22 && res )
	{
		wcx->CloseArchive( arc.arc );
		return 0;
	}

	wcx->CloseArchive( arc.arc ); // reset enumerator
	if( (arc.arc = wcx->OpenArchive( &arc_data )) == 0 )
		return 0;
	if( wcx->SetProcessDataProc )
		wcx->SetProcessDataProc( arc.arc, ProcessDataProc );
	if( wcx->SetChangeVolProc )
		wcx->SetChangeVolProc( arc.arc, ChangeVolProc );

	SArc* result = (SArc*)malloc( sizeof( *result ) );
//	assert( result );
	*result = arc;
//	DebugString( "tested & opened" );
	return result;
}*/


INSTALL_DLL_API  DWORD install::wcx::ExtractWCXArchive(
const TCHAR* source,
const TCHAR* type,
const TCHAR* files,
const TCHAR* command,
const TCHAR* destination, 
const TCHAR* flags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	HRESULT hr = S_OK;

	WCXArchive wcxArchive(source, type, false, callback, userdata);

	if(wcxArchive.Extract(files, TEXT(""), destination, flags, true) != S_OK)
		return GetLastError();
	
	return hr;
}