#include "../include/stdafx.h"
#include "../include/adapted.h"
#include "../include/file.h"

struct ExtraUserData{
	LPVOID callback;
	LPVOID userdata;
};

DWORD CALLBACK install::file::CopyProgressRoutine(
  LARGE_INTEGER TotalFileSize,
  LARGE_INTEGER TotalBytesTransferred,
  LARGE_INTEGER StreamSize,
  LARGE_INTEGER StreamBytesTransferred,
  DWORD dwStreamNumber,
  DWORD dwCallbackReason,
  HANDLE hSourceFile,
  HANDLE hDestinationFile,
  LPVOID lpData)
{
	if(lpData != NULL)
	{
		ExtraUserData *eud = static_cast<ExtraUserData*>(lpData);

		static_cast<INSTALLCALLBACK>(eud->callback)(
			msgCurrentOperationProgess,
			NULL,
			static_cast<DWORD>(100.0f*(static_cast<float>(TotalBytesTransferred.LowPart)/static_cast<float>(TotalFileSize.LowPart))),
			0,
			static_cast<INSTALLCALLBACK>(eud->userdata));
	}
	return S_OK;
}

//
DWORD install::file::CopyFlagsToDWORD(const TCHAR* flags)
{
	DWORD dwFlags = 0;
	_stscanf(flags, TEXT("%d"), &dwFlags);
	return dwFlags;
}

DWORD install::file::CreateDirectoryExEx(kru_string path)
{
	HRESULT hr = S_OK;

	if(path[path.length() - 1] != TEXT('\\'))
		path += TEXT("\\");
	size_t slash = path.find(TEXT("\\"), 3);

	while(slash != path.npos)
	{
		if(CreateDirectory(path.substr(0, slash).c_str(), NULL) == 0)
			if(GetLastError() != ERROR_ALREADY_EXISTS)
			    return GetLastError();
		slash = path.find(TEXT("\\"), slash + 1);
	}
		
	return hr;
}

DWORD install::file::ScanFolder(FILELIST &stringList, kru_string folderName)
{
	HRESULT hr = S_OK;
	HANDLE file;
	LPWIN32_FIND_DATA lpFindFileData = new WIN32_FIND_DATA;
	
	if(folderName[folderName.length() - 1] == TEXT('\\'))
		folderName.erase(folderName.end() - 1);

	//find first file or folder
	file = FindFirstFile((folderName + TEXT("\\*.*")).c_str(), lpFindFileData);
	if(file == INVALID_HANDLE_VALUE)
	{
	   delete lpFindFileData;
	   return GetLastError();
	}

	while(true)
	{
		kru_string fileName = lpFindFileData->cFileName;

		if(fileName != TEXT(".") && fileName != TEXT(".."))
		{				
			FILEDESC filedesc(
				lpFindFileData->dwFileAttributes,
				folderName + TEXT("\\") + fileName);
			stringList.push_back(filedesc);

			if(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{		
				hr = ScanFolder(stringList, filedesc.second);
				if(hr != ERROR_NO_MORE_FILES)
					break;
			}
		}

		if(FindNextFile(file, lpFindFileData) == NULL){
			hr = GetLastError();
			//if no match, then exit from search circle
			break;
		}//if
	}

	//release all
	FindClose(file);
	delete lpFindFileData;
	return hr;
}

/*DWORD install::file::FileNewer(
const TCHAR* source,
const TCHAR* destination,
DWORD dwFlags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	if(dwFlags & flgRefresh)
	{
		for(;;)
		{
			HANDLE destHandle = CreateFile(destination, FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
			if(destHandle != NULL)
			{
				FILETIME destLastWriteTime;
				if(GetFileTime(destHandle, NULL, NULL, &destLastWriteTime))
				{
					HANDLE srcHandle = CreateFile(source, FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
					if(destHandle != NULL)
					{
						FILETIME srcLastWriteTime;
						if(GetFileTime(srcHandle, NULL, NULL, &srcLastWriteTime))
						{
							HANDLE srcHandle = CreateFile(source, FILE_ALL_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 

							if(CompareFileTime(&destLastWriteTime, &srcLastWriteTime) == 1)
								return chkSkip;
							else
								return chkContinue;
						}
					}
				}
			}

			if(callback)
			{
				switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
				{
				case ansAbort:	return chkAbort;
				case ansSkip:	return chkSkip;
				case ansRetry:	break;
				default:		return chkAbort;
				}//switch UnknownError Answer
			}
			else
				return chkError;
		}
	}//if Refresh
	return chkContinue;
}

DWORD install::file::FileReadOnly(
const TCHAR* filename,
DWORD dwFlags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	DWORD fa = GetFileAttributes(filename);
	if(fa & FILE_ATTRIBUTE_READONLY)
	{
		if(callback)
		{
			switch((dwFlags & flgEvenReadOnly)?ansOverwrite:callback(msgRequest, NULL, reqFileReadOnly, 0, userdata))
			{
			case ansAbort:				return chkAbort;
			case ansSkip:				return chkSkip;
			case ansDelete:
			case ansMove:
			case ansOverwrite:
			case ansRefresh:
			case ansAppend:
				for(;;)
				{
					if(!SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL))
					{
						switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
						{
						case ansAbort:	return chkAbort;
						case ansSkip:	return chkSkip;
						case ansRetry:	break;
						default:		return chkError;
						}//switch UnknownError Answer
					}
					else
						break;
				}// loop until the problem is solved
				return chkContinue;
			default:
				return chkError;
			}//switch Read-only Answer
		}//if callback
		else if(dwFlags & flgEvenReadOnly)
		{
			if(!SetFileAttributes(filename, FILE_ATTRIBUTE_NORMAL))
				return chkError;
			else
				return chkContinue;
		}
		else
			return chkContinue;
	}//if destination Read-only
	else
		return chkContinue;
}

DWORD install::file::FileExist(
const TCHAR* filename,
DWORD dwFlags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	DWORD fa = GetFileAttributes(filename);
	if(fa != INVALID_FILE_ATTRIBUTES)//if destination exist
	{
		if(!(dwFlags & flgAsk))
		{
			if(callback)
			{				
				switch(callback(msgRequest,NULL, reqFileExist, 0, userdata))
				{
				case ansAbort:			return chkAbort;
				case ansSkip:			return chkSkip;
				case ansDelete:
				case ansMove:
				case ansOverwrite:
				case ansRefresh:
				case ansAppend:			return chkContinue;
				default:				return chkError;
				}
			}
		}//if Ask
		else if(dwFlags & flgSkip)
		{
			return chkSkip;
		}

		if(!(dwFlags & flgOverwrite))
		{
			return chkAbort;//We dont know what we must to do with existing destination,
							//so abort.
		}


		return FileReadOnly(filename, dwFlags, callback, userdata);
		
		
	}
	else
		return chkError;
}

DWORD install::file::CopyFile(
const TCHAR* source, 
const TCHAR* destination,
const TCHAR* flags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	HRESULT hr = S_OK;

	DWORD dwFlags = CopyFlagsToDWORD(flags);

	switch(FileExist(destination, dwFlags, callback, userdata))
	{
	case chkAbort:		return E_ABORT;
	case chkSkip:		return S_OK;
	case chkContinue:	
		switch(FileReadOnly(destination, dwFlags, callback, userdata))
		{
		case chkAbort:		return E_ABORT;
		case chkSkip:		return S_OK;
		case chkContinue:	break;
		case chkError:		return GetLastError();
		default:			return S_FALSE;
		}
		break;
	case chkError:
		if(	GetLastError() != ERROR_FILE_NOT_FOUND && 
			GetLastError() != ERROR_PATH_NOT_FOUND)
			return GetLastError();
		else
			break;
	default:			return S_FALSE;
	}

	if(dwFlags & flgMove)
	{
		switch(FileReadOnly(source, dwFlags, callback, userdata))
		{
		case chkAbort:		return E_ABORT;
		case chkSkip:		return S_OK;
		case chkContinue:	break;
		case chkError:		return GetLastError();
		default:			return S_FALSE;
		}

		for(;;)
		{
			if(::DeleteFile(destination))
			{
				break;
			}
			if(callback)
			{
				switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
				{
				case ansAbort:	return E_ABORT;
				case ansSkip:	return S_OK;
				case ansRetry:	break;
				default:		return S_FALSE;
				}//switch UnknownError Answer
			}
			else
				return GetLastError();
		}
	}

	if(dwFlags & flgAppend)
	{
		return S_OK;//To skip Overwrite!
	}

	switch(FileNewer(source, destination, dwFlags, callback, userdata))
	{
	case chkAbort:		return E_ABORT;
	case chkSkip:		return S_OK;
	case chkContinue:	break;
    case chkError:		return GetLastError();
	default:			return S_FALSE;
	}

	ExtraUserData eud;

	eud.callback = callback;
	eud.userdata = userdata;
	
	for(;;)
	{
		if(dwFlags & flgMove)
		{
			if(MoveFileWithProgress(source, destination, CopyProgressRoutine, &eud, 0))
			{
				break;
			}
		}
		else if(CopyFileEx(source, destination, CopyProgressRoutine, &eud, NULL, 0))
		{
			break;
		}
		if(callback)
		{
			switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
			{
			case ansAbort:	return E_ABORT;
			case ansSkip:	return S_OK;
			case ansRetry:	break;
			default:		return S_FALSE;
			}//switch UnknownError Answer
		}
		else
			return GetLastError();
	}

	return hr;
}

DWORD install::file::CopyFolder(
const TCHAR* foldername,
const TCHAR* destination,
const TCHAR* flags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	HRESULT hr = S_OK;

	FILELIST stringList;
	
	FILEDESC filedesc(FILE_ATTRIBUTE_DIRECTORY,	destination);
	stringList.push_back(filedesc);

	hr = ScanFolder(stringList, foldername);

	for(FILELIST::iterator i_str = stringList.begin(); i_str != stringList.end(); ++i_str)
	{
		kru_string destFileName = (*i_str).second;
		destFileName.replace(0, _tcslen(foldername), destination);

		if((*i_str).first & FILE_ATTRIBUTE_DIRECTORY)
		{
			for(;;)
			{
				hr = CreateDirectoryExEx(destFileName.c_str());
				if(hr != S_OK)
				{
					if(callback)
					{
						switch(callback(msgRequest, NULL, reqUnknownError, hr, userdata))
						{
						case ansAbort:	return chkAbort;
						case ansSkip:	return chkSkip;
						case ansRetry:	break;
						default:		return chkError;
						}//switch UnknownError Answer
					}
					else
						return hr;
				}
				else
					break;
			}// loop until the problem is solved
		}
		else
		{			
			hr = CopyFile((*i_str).second.c_str(), destFileName.c_str(), flags, callback, userdata);
		    if(hr != S_OK)
			    break;
		}
		
		//MessageBox(NULL, (*i_str).c_str(), TEXT("test"), NULL);
	}

	return hr;
}
*/

DWORD install::file::DeleteFile(
const TCHAR* filename,
const TCHAR* flags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	HRESULT hr = S_OK;

	DWORD dwFlags = CopyFlagsToDWORD(flags);

	switch(FileReadOnly(filename, dwFlags, callback, userdata))
	{
	case chkAbort:		return E_ABORT;
	case chkSkip:		return S_OK;
	case chkContinue:	break;
	case chkError:		return GetLastError();
	default:			return S_FALSE;
	}

	for(;;)
	{
		if(::DeleteFile(filename))
		{
			break;
		}
		if(callback)
		{
			switch(callback(msgRequest, NULL, reqUnknownError, GetLastError(), userdata))
			{
			case ansAbort:	return E_ABORT;
			case ansSkip:	return S_OK;
			case ansRetry:	break;
			default:		return S_FALSE;
			}//switch UnknownError Answer
		}
		else
			return GetLastError();
	}
	return hr;
}
/*
DWORD install::file::DeleteFolder(
const TCHAR* foldername,
const TCHAR* flags,
INSTALLCALLBACK callback,
LPVOID userdata)
{
	HRESULT hr = S_OK;

	FILELIST stringList;

	stringList.push_back(FILEDESC(FILE_ATTRIBUTE_DIRECTORY, foldername));

	hr = ScanFolder(stringList, foldername);

	for(FILELIST::iterator i_str = stringList.begin(); i_str != stringList.end(); ++i_str)
	{		
		if(!((*i_str).first & FILE_ATTRIBUTE_DIRECTORY))
		{
			hr = DeleteFile((*i_str).second.c_str(), flags, callback, userdata);
			if(hr != S_OK)
				break;
		}
	}

	std::sort(stringList.rbegin(), stringList.rend());
	
	for(FILELIST::iterator i_str = stringList.begin(); i_str != stringList.end(); ++i_str)
	{		
		if((*i_str).first & FILE_ATTRIBUTE_DIRECTORY)
		{
			//MessageBox(NULL, (*i_str).second.c_str(), _TEXT("test"), NULL);
			if(!RemoveDirectory((*i_str).second.c_str()))
				return GetLastError();
		}
	}

	return hr;
}*/