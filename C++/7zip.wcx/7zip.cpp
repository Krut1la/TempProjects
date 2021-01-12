////////////////////////////////////////////////////////////////////////////////
// 7zip Plugin for Total Commander
// Copyright (c) 2004 Adam Strzelecki <ono@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>

#include <initguid.h>
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <process.h>
#include <Winnetwk.h>

// IIDs
#include "7zip/ICoder.h"
#include "7zip/Compress/LZ/IMatchFinder.h"

// Includes
#include "Common/StringConvert.h"
#include "7zip/Common/FileStreams.h"
#include "7zip/Archive/Common/ItemNameUtils.h"
#include "7zip/Archive/Common/OutStreamWithCRC.h"
#include "7zip/Archive/7z/7zIn.h"
#include "7zip/Archive/7z/7zItem.h"
#include "7zip/Archive/7z/7zDecode.h"
#include "7zip/Archive/7z/7zUpdate.h"
#include "7zip/Archive/7z/7zCompressionMode.h"
#include "7zip/Crypto/7zAES/7zAES.h"

#include "Windows/FileDir.h"
#include "Windows/FileFind.h"

// WCX
#include "wcxhead.h"
#include "resource.h"
#include "7zip.h"

// Basic settings
//#define TERMINATE_THREADS // This is unsafe solution because 7zip decoder may be not cleared

HINSTANCE g_hInstance = NULL;
tProcessDataProc procProcess = NULL;

const UInt64 gMaxCheckStartPosition = 1 << 20;

typedef struct _7ZHANDLE
{
    NArchive::N7z::CArchiveDatabaseEx *db;
    CMyComPtr<IInStream> inStream;
    CMyComPtr<ICryptoGetTextPassword> password;
    UInt32 iItem;
    char *fileName, *archiveName;
    UInt32 written;
    HANDLE hThread, hPushEvent, hPullEvent;
    tProcessDataProc procProcess;
    BOOL bTest;
    int iResult;
} ZHANDLE, *PZHANDLE;

////////////////////////////////////////////////////////////////////////////////

class C7zPassword:
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP

    bool _passwordIsDefined;
    UString _password;

    C7zPassword()
    {
        _passwordIsDefined = false;
        _password = L"";
    }

    STDMETHOD(CryptoGetTextPassword)(BSTR *password)
    {
        char *lpszPass = NULL;
        if(!_passwordIsDefined && (lpszPass = GetPassword()))
        {
            _password = MultiByteToUnicodeString(lpszPass);
            _passwordIsDefined = true;
        }
        if(!_passwordIsDefined) return E_FAIL;
        CMyComBSTR tempName(_password);
        *password = tempName.Detach();

        return S_OK;
    }
};

////////////////////////////////////////////////////////////////////////////////

class C7zOutStream:
  public ISequentialOutStream,
  public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP

    PZHANDLE _handle;
    UInt32 _curIndex, _startIndex;
    bool _isOpen;
    COutFileStream *_streamSpec;
    COutStreamWithCRC _hash;
    UInt32 _pos, _curSize;

    C7zOutStream(PZHANDLE handle, UInt32 startIndex)
    {
        _handle = handle;
        _curIndex = _startIndex = startIndex;
        _pos = 0;
        _isOpen = false;
        _curSize = handle->db->Files[_curIndex].UnPackSize;
        DebugString("C7zOutStream::constructor start: %d, size: %d", _startIndex, _curSize);
    }

    STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize)
    {
        UInt32 bytes = 0;

        DebugString("C7zOutStream::Write size: %d", size);

        while(bytes < size)
        {
            UInt32 chunk = size - bytes;

            DebugString("C7zOutStream::Write chunk: %d, pos: %d", chunk, _pos);

            // Check if we should abort the file extraction
            if(_handle->iResult == E_EABORTED)
            {
                if(_isOpen && !_handle->bTest)
                {
                    DebugString("C7zOutStream::Write Aborted by user");
                    _hash.Init(NULL);
                    ::DeleteFile(_handle->fileName);
                }
                if(processedSize) *processedSize = bytes;
                return E_EWRITE;
            }

            while(_curIndex > _handle->iItem)
            {
                SetEvent(_handle->hPullEvent);
                DebugString("C7zOutStream::Write SetEvent(_handle->hPullEvent) & WaitForSingleObject(_handle->hPushEvent, INFINITE)");
                WaitForSingleObject(_handle->hPushEvent, INFINITE);
                ResetEvent(_handle->hPushEvent);
                DebugString("C7zOutStream::Write ResetEvent(_handle->hPushEvent)");
            }

            // We creating new file
            if(_handle->iItem == _curIndex && !_isOpen)
            {
                if(_handle->bTest)
                    _hash.Init(NULL);
                else if(_streamSpec = new COutFileStream())
                {
                    CMyComPtr<ISequentialOutStream> streamLoc(_streamSpec);
                    if(_isOpen = _streamSpec->Create(_handle->fileName, true))
                        _hash.Init(streamLoc);
                }
            }

            if(_pos + chunk > _curSize) chunk = _curSize - _pos;
            if(_isOpen)
            {
                UInt32 localSize;
                _hash.Write((const Byte *)data + bytes, chunk, &localSize);
                _pos += localSize; bytes += localSize;
                _handle->written += localSize;
            }
            else
            {
                bytes += chunk;
                _pos += chunk;
            }

            // Check if we skipped to next file
            if(_pos >= _curSize)
            {
                DebugString("C7zOutStream::Write Close1");
                if(_isOpen) // Setup file attributes
                {
                    DebugString("C7zOutStream::Write Close2");
                    // Mark we have CRC error
                    if (_handle->db->Files[_curIndex].IsFileCRCDefined)
                        if(_handle->db->Files[_curIndex].FileCRC != _hash.GetCRC())
                            _handle->iResult = E_BAD_ARCHIVE;
                    DebugString("C7zOutStream::Write Close3");
                    // Write attributes / if not just testing
                    if(!_handle->bTest)
                    {
                        DebugString("C7zOutStream::Write Close3.1");
                        if(_handle->db->Files[_curIndex].IsLastWriteTimeDefined)
                            _streamSpec->File.SetLastWriteTime(&_handle->db->Files[_curIndex].LastWriteTime);
                        DebugString("C7zOutStream::Write Close3.2");
                        _hash.Init(NULL);
                        DebugString("C7zOutStream::Write Close3.4");
                        NWindows::NFile::NDirectory::MySetFileAttributes(_handle->fileName,
                            _handle->db->Files[_curIndex].Attributes);
                        DebugString("C7zOutStream::Write Close3.5");
                    }
                    DebugString("C7zOutStream::Write Close4");
                }
                _curIndex ++; _pos = 0;
                if(_curIndex < _handle->db->Files.Size()) _curSize = _handle->db->Files[_curIndex].UnPackSize;
                _isOpen = false;
            }
        }

        if(processedSize) *processedSize = bytes;
        DebugString("C7zOutStream::Write OK");
        return S_OK;
    }
    STDMETHOD(WritePart)(const void *data, UInt32 size, UInt32 *processedSize)
    { return Write(data, size, processedSize); }


};

////////////////////////////////////////////////////////////////////////////////
// Main DLL functions

extern "C" {

DWORD WINAPI ExtractThread(LPVOID empty)
{
    PZHANDLE handle = (PZHANDLE)empty;
    int folderIndex = handle->db->FileIndexToFolderIndexMap[handle->iItem];
    if(folderIndex != -1)
    {
        UInt32 packStreamIndex = handle->db->FolderStartPackStreamIndex[folderIndex];
        UInt64 folderStartPackPos = handle->db->GetFolderStreamPos(folderIndex, 0);
        const NArchive::N7z::CFolder &folderInfo = handle->db->Folders[folderIndex];
        UInt32 startIndex = (UInt32)handle->db->FolderStartFileIndex[folderIndex];

        C7zOutStream *outStreamSpec = new C7zOutStream(handle, startIndex);
        CMyComPtr<ISequentialOutStream> outStream(outStreamSpec);
        DebugString("ExtractThread start: %d, item: %d", startIndex, handle->iItem);

        NArchive::N7z::CDecoder decoder(true);
        HRESULT result = decoder.Decode(
            handle->inStream,
            folderStartPackPos,
            &handle->db->PackSizes[packStreamIndex],
            folderInfo,
            outStream,
            NULL, handle->password
        );
        // Mark we had a problem with this stream / file to read error / if not marked earlier
        if(result != S_OK && !handle->iResult) handle->iResult = E_EREAD;
    }
    DebugString("ExtractThread SetEvent(handle->hPullEvent)");
    //handle->hThread = NULL;       // ^^
    SetEvent(handle->hPullEvent); // This order is very important

    return 0;
}


typedef unsigned int (__stdcall *_THREADFUNC)(void*);

BOOL Extract(PZHANDLE handle)
{
    int folderIndex = handle->db->FileIndexToFolderIndexMap[handle->iItem];
    if(folderIndex == -1) // This might happen when filesize if 0
    {
        COutFileStream zeroStream;
        zeroStream.Create(handle->fileName, true);
        if(handle->db->Files[handle->iItem].IsLastWriteTimeDefined)
            zeroStream.File.SetLastWriteTime(&handle->db->Files[handle->iItem].LastWriteTime);
        zeroStream.File.Close();
        NWindows::NFile::NDirectory::MySetFileAttributes(handle->fileName,
            handle->db->Files[handle->iItem].Attributes);
        return TRUE;
    }
    UInt32 startIndex = (UInt32)handle->db->FolderStartFileIndex[folderIndex];

    // Check if we are in the next folder for next extraction / WHAT A CRAP IS IT ???
    /*
    if(handle->hThread && (handle->db->FolderStartFileIndex.Size() > folderIndex + 1)
        && handle->iItem >= (UInt32)handle->db->FolderStartFileIndex[folderIndex + 1])
    {
#ifdef TERMINATE_THREADS
        TerminateThread(handle->hThread, 0);
#else
        DebugString("Extract WaitForSingleObject(handle->hThread, INFINITE)");
        SetEvent(handle->hPushEvent);
        WaitForSingleObject(handle->hThread, INFINITE);
#endif
        handle->hThread = NULL;
    }
    */
    UInt64 written = handle->written;
    if(!handle->hThread)
    {
        ResetEvent(handle->hPushEvent); ResetEvent(handle->hPullEvent);
        DWORD dwThreadId = 0;
        DebugString("Extract CreateThread");
        written = handle->written = 0;
        if(!(handle->hThread = CreateThread(NULL, 0, ExtractThread, (LPVOID)handle, 0, &dwThreadId)))
        {
            DebugString("Extract Cannot create new thread. Exiting extraction...");
            return FALSE;
        }

		/*if(!(handle->hThread = (HANDLE)_beginthreadex(NULL, 0, (_THREADFUNC)ExtractThread, (LPVOID)handle, 0, (unsigned int*)&dwThreadId)))
		{
		    DebugString("Extract Cannot create new thread. Exiting extraction...");
            return FALSE;
        }*/

        //CloseHandle(handle->hThread);
		//_endthreadex((unsigned int)handle->hThread);
    }
    else
    {
        DebugString("ExtractThread SetEvent(handle->hPushEvent) cause thread is running");
        SetEvent(handle->hPushEvent);
    }
    while(WaitForSingleObject(handle->hPullEvent, 200) == WAIT_TIMEOUT)
        if((handle->written - written) && handle->procProcess)
        {
            // Break if user pressed cancel
            if(!handle->procProcess(handle->fileName, (handle->written - written)))
                handle->iResult = E_EABORTED;
            written = handle->written;
        }
        else if(!handle->written && handle->procProcess && !handle->procProcess(NULL, 0))
                handle->iResult = E_EABORTED;
    // Show final 100%
    if(!handle->iResult && handle->procProcess) handle->procProcess(handle->fileName, (handle->written - written));
    DebugString("Extract ResetEvent(_handle->hPullEvent)");
    ResetEvent(handle->hPullEvent);
    return TRUE;
}

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    g_hInstance = hInst;
    return TRUE;
}

void ZDeleteHandle(PZHANDLE handle);

PZHANDLE ZNewHandle(void)
{
    PZHANDLE handle = new ZHANDLE;
    ZeroMemory(handle, sizeof(ZHANDLE));
    if(!(handle->hPushEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) ||
       !(handle->hPushEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))

	//if(!(handle->hPushEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
    {
        DebugString("ZNewHandle Cannot create event handles.");
        ZDeleteHandle(handle);
        return NULL;
    }
    C7zPassword *passwordSpec = new C7zPassword();
    handle->password = passwordSpec;
    handle->hPullEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    return handle;
}

void ZDeleteHandle(PZHANDLE handle)
{
    if(!handle) return;
    if(handle->inStream) handle->inStream.Release();
    if(handle->db) delete handle->db;
    if(handle->archiveName) free(handle->archiveName);
	//if(handle->archiveName) delete[] handle->archiveName;

    if(handle->hPushEvent) CloseHandle(handle->hPushEvent);
    if(handle->hPullEvent) CloseHandle(handle->hPullEvent);
    delete handle;
}

HANDLE __stdcall OpenArchive (tOpenArchiveData *archiveData)
{
    if(!archiveData) return NULL;

    PZHANDLE handle = ZNewHandle();

    // Check if we got valid handle
    if(!handle)
    {
        archiveData->OpenResult = E_NO_MEMORY;
        return NULL;
    }

    // Check if we have valid filename
    if(!archiveData->ArcName)
    {
        archiveData->OpenResult = E_BAD_DATA;
        return NULL;
    }

    try
    {
        // Try to open archive file name
        CInFileStream *inStream = new CInFileStream();
        if(!inStream || !inStream->Open(archiveData->ArcName))
        {
            archiveData->OpenResult = E_EOPEN;
            delete inStream;
            ZDeleteHandle(handle);
            return NULL;
        }

        handle->inStream = inStream;
        NArchive::N7z::CInArchive archive;
        HRESULT result;
        if((result = archive.Open(handle->inStream, &gMaxCheckStartPosition)))
        {
            archiveData->OpenResult = E_UNKNOWN_FORMAT;
            ZDeleteHandle(handle);
            return NULL;
        }
        handle->db = new NArchive::N7z::CArchiveDatabaseEx;
        if((result = archive.ReadDatabase(*handle->db, handle->password)) != S_OK)
        {
            archiveData->OpenResult = E_UNKNOWN_FORMAT;
            ZDeleteHandle(handle);
            return NULL;
        }
        handle->db->Fill();

        if(handle->db->Files.Size() == 0)
        {
            archiveData->OpenResult = E_EOPEN;
            ZDeleteHandle(handle);
            return NULL;
        }
    }
    catch(...)
    {
        archiveData->OpenResult = E_UNKNOWN_FORMAT;
        ZDeleteHandle(handle);
        return NULL;
    }

    handle->archiveName = strdup(archiveData->ArcName);
    archiveData->OpenResult = 0;
    DebugString("OpenArchive OK");
    return (HANDLE) handle;
}

int __stdcall ReadHeader (HANDLE hArcData, tHeaderData *headerData)
{
    PZHANDLE handle = (PZHANDLE)hArcData;
    if(!handle || !handle->db) return E_BAD_DATA;

    if(handle->iItem >= handle->db->Files.Size())
    {
        DebugString("ReadHeader END");
        handle->iItem = 0;
        return E_END_ARCHIVE;
    }

    // Lookup name
    strncpy(headerData->FileName,
        (const char *)UnicodeStringToMultiByte(NArchive::NItemName::GetOSName(handle->db->Files[handle->iItem].Name)), 260);
    strncpy(headerData->ArcName, handle->archiveName, 260);

    // Attributes
    headerData->FileAttr |= ((handle->db->Files[handle->iItem].Attributes & FILE_ATTRIBUTE_DIRECTORY) ? 0x10 : 0)
                          | ((handle->db->Files[handle->iItem].Attributes & FILE_ATTRIBUTE_READONLY)  ? 0x1 : 0)
                          | ((handle->db->Files[handle->iItem].Attributes & FILE_ATTRIBUTE_HIDDEN)    ? 0x2 : 0)
                          | ((handle->db->Files[handle->iItem].Attributes & FILE_ATTRIBUTE_SYSTEM)    ? 0x4 : 0)
                          | ((handle->db->Files[handle->iItem].Attributes & FILE_ATTRIBUTE_ARCHIVE)   ? 0x20 : 0);

    // Packed size
    int folderIndex = handle->db->FileIndexToFolderIndexMap[handle->iItem];
    if (folderIndex >= 0)
    {
        const NArchive::N7z::CFolder &folderInfo = handle->db->Folders[folderIndex];
        headerData->PackSize = handle->db->GetFolderFullPackSize(folderIndex);
    }
    else
        headerData->PackSize = 0;

    // File size
    headerData->UnpSize = handle->db->Files[handle->iItem].UnPackSize;
    headerData->FileCRC = handle->db->Files[handle->iItem].IsFileCRCDefined ? handle->db->Files[handle->iItem].FileCRC : 0;

    // File time
    if(handle->db->Files[handle->iItem].IsLastWriteTimeDefined)
    {
        FILETIME ft;
        SYSTEMTIME st;
        FileTimeToLocalFileTime(&handle->db->Files[handle->iItem].LastWriteTime, &ft);
        FileTimeToSystemTime(&ft, &st);
        headerData->FileTime = (st.wYear - 1980) << 25 | st.wMonth << 21 | st.wDay << 16 | st.wHour << 11 | st.wMinute << 5 | st.wSecond / 2;
    }

    DebugString("ReadHeader OK");
    return 0;
}

int __stdcall ProcessFile (HANDLE hArcData, int operation, char *destPath, char *destName)
{
    PZHANDLE handle = (PZHANDLE)hArcData;
    if(!handle || !handle->db) return E_BAD_DATA;

    handle->fileName = destName;
    handle->iResult = E_NOT_SUPPORTED;
    handle->bTest = FALSE;
    switch(operation)
    {
        case PK_TEST:
            DebugString("ProcessFile TEST");
            handle->bTest = TRUE;
        case PK_EXTRACT:
            handle->iResult = 0;
            DebugString("ProcessFile EXTRACT");
            Extract(handle);
            break;
        case PK_SKIP:
            handle->iResult = 0;
            DebugString("ProcessFile SKIP");
            if(handle->hPushEvent) SetEvent(handle->hPushEvent);
            break;
    }

    handle->iItem ++;

	//if(handle->hThread) TerminateThread(handle->hThread, 0);
	//WaitForSingleObject(handle->hThread, INFINITE);
	//CloseHandle(handle->hThread);
    return handle->iResult;
}

#define TERMINATE_THREADS

int __stdcall CloseArchive (HANDLE hArcData)
{
    PZHANDLE handle = (PZHANDLE)hArcData;
    handle->iItem = handle->db->Files.Size();
#ifdef TERMINATE_THREADS
    if(handle->hThread) TerminateThread(handle->hThread, 0);
#else
    if(handle->hPushEvent)
    {
        DebugString("CloseArchive SetEvent(handle->hPushEvent)");
        SetEvent(handle->hPushEvent);
    }
    // If thread is running stop it !
    if(handle->hThread && handle->hPullEvent)
    {
        // Set we want to abort rest of the extraction
        handle->iResult = E_EABORTED;
        DebugString("CloseArchive WaitForSingleObject(handle->hPullEvent, INFINITE)");
        WaitForSingleObject(handle->hPullEvent, INFINITE);
    }
    DebugString("CloseArchive OK");
#endif
    ZDeleteHandle((PZHANDLE)hArcData);
    return 0;
}

int __stdcall GetPackerCaps (void)
{
    return PK_CAPS_MULTIPLE | PK_CAPS_NEW | PK_CAPS_MODIFY | PK_CAPS_BY_CONTENT | PK_CAPS_OPTIONS	/* | PK_CAPS_DELETE */;
}

void __stdcall SetChangeVolProc (HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
}

void __stdcall SetProcessDataProc (HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
    PZHANDLE handle = (PZHANDLE)hArcData;
    if((int) hArcData == -1 || !handle)
        procProcess = pProcessDataProc;
    else
        handle->procProcess = pProcessDataProc;
}

////////////////////////////////////////////////////////////////////////////////

class C7zUpdateCallback:
  public IArchiveUpdateCallback,
  public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP

    // IProgress
    STDMETHOD(SetTotal)(UInt64 size)
    {
        _total = size;
        return S_OK;
    }
    STDMETHOD(SetCompleted)(const UInt64 *completeValue)
    {
        if(!_inFileStream) return S_OK;
        if(!procProcess((char *)(const char *)_lastFile, *completeValue - _lastComplete))
        {
            if(_pbAbort) *_pbAbort = TRUE;
            return E_FAIL;
        }
        _lastComplete = *completeValue;
        return S_OK;
    }

    // IUpdateCallback
    STDMETHOD(EnumProperties)(IEnumSTATPROPSTG **enumerator) { return S_OK; }
    STDMETHOD(GetUpdateItemInfo)(UInt32 index,
      Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive) { return S_OK; }
    STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value) { return S_OK; }
    STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream)
    {
        DebugString("C7zUpdateCallback::GetStream start.");
        CInFileStream *inStreamSpec = new CInFileStream;
        CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
        UString fileName(_srcPath); fileName += (*_files)[index];
        _lastFile = UnicodeStringToMultiByte(fileName);
        DebugString("C7zUpdateCallback::GetStream index: %d, fileName: \"%s\"", index, (const char *)UnicodeStringToMultiByte(fileName));
        if(!inStreamSpec->Open(fileName))
        {
            *inStream = NULL;
            return S_FALSE;
        }
        _inFileStream = inStreamLoc;
        *inStream = inStreamLoc.Detach();
        return S_OK;
    }
    STDMETHOD(SetOperationResult)(Int32 operationResult)
    {
        _inFileStream.Release();
        DebugString("C7zUpdateCallback::SetOperationResult released.");
        return S_OK;
    }

public:
    CObjectVector<UString> *_files;
    CMyComPtr<ISequentialInStream> _inFileStream;
    UString _srcPath;
    AString _lastFile;
    UInt64 _lastComplete, _total;
    BOOL *_pbAbort;

    C7zUpdateCallback(CObjectVector<UString> *files,
        char *srcPath, BOOL *pbAbort)
    {
        DebugString("C7zUpdateCallback::constructor srcPath: %s", srcPath);
        _files = files;
        if(srcPath)
            _srcPath = MultiByteToUnicodeString(srcPath);
        else
            _srcPath = L"";
        _lastComplete = 0;
        _pbAbort = pbAbort;
    }
    ~C7zUpdateCallback()
    {
        DebugString("C7zUpdateCallback::destructor");
    }
};

int Update(char *packedFile, char *subPath, char *srcPath, char *addList, char *deleteList, int flags)
{
    // Try to open archive file name
    tOpenArchiveData arcData = {0};
    arcData.ArcName = packedFile;

    CObjectVector<NArchive::N7z::CUpdateItem> updateItems;
    CObjectVector<UString> files;
    char *iName = addList;
    int iNum = 0;
    while(iName && *iName)
    {
        int nameLen = strlen(iName);
        // Check if it's folder and then delete empty folder
        if(iName[nameLen - 1] == '\\') iName[nameLen - 1] = 0;
        // Create new file name
        AString fileName = srcPath; fileName += iName;
        NWindows::NFile::NFind::CFileInfo fileInfo;
        if(NWindows::NFile::NFind::FindFile(fileName, fileInfo))
        {
            UString newName;

            // Create valid archive filename
            if(subPath)
            {
                newName = MultiByteToUnicodeString(subPath);
                newName += L'/';
                newName += MultiByteToUnicodeString(iName);
            }
            else
                newName = MultiByteToUnicodeString(iName);

            // Add file to file list
            files.Add(MultiByteToUnicodeString(iName));

            // Create new update item
            NArchive::N7z::CUpdateItem updateItem;
            updateItem.NewProperties = true;
            updateItem.NewData = fileInfo.IsDirectory() ? false : true;
            updateItem.IndexInArchive = -1;
            updateItem.IsAnti = false;
            updateItem.Size = fileInfo.Size;
            updateItem.Name = NArchive::NItemName::MakeLegalName(newName);
            updateItem.IsDirectory = fileInfo.IsDirectory();

            updateItem.Attributes = fileInfo.Attributes;
            updateItem.AttributesAreDefined = true;
            updateItem.CreationTime = fileInfo.CreationTime;
            updateItem.CreationTimeIsDefined = true;
            updateItem.LastWriteTime = fileInfo.LastWriteTime;
            updateItem.LastWriteTimeIsDefined = true;

            DebugString("UpdateItem: Index: %d, Name: %s, Size: %d, Attibutes: %d",
                            iNum, (char *)(const char *)fileName, fileInfo.Size, fileInfo.Attributes);

            updateItem.IndexInClient = iNum ++;
            updateItems.Add(updateItem);

        }
        // Skip to the next file
        iName += nameLen + 1;
    }

    // Read settings
    COMPSETTINGS cs;
    ReadSettings(&cs);

    NArchive::N7z::CUpdateOptions options;
    NArchive::N7z::CMethodFull methodFull, methodHeader;
    NArchive::N7z::CCompressionMethodMode methodMode, headerMode;
    // Method for compressing header
    methodHeader.NumInStreams = 1;
    methodHeader.NumOutStreams = 1;
    methodHeader.MethodID = g_compMethods[0].id;
    headerMode.Methods.Add(methodHeader);
    if(cs.flags & FLAG_ENCRYPT_HEADER)
    {
        headerMode.PasswordIsDefined = (*cs.password != 0);
        if(headerMode.PasswordIsDefined) headerMode.Password = MultiByteToUnicodeString(cs.password);
    }
    // Method for compressing files
    methodFull.NumInStreams = 1;
    methodFull.NumOutStreams = 1;
    // We don't have store profile
    if(cs.level)
        methodFull.MethodID = cs.method;
    else
    {
        int i = 0; while(g_compMethods[i].name) i++;
        methodFull.MethodID = g_compMethods[i].id;
    }
    // Add LZMA properties
    if(cs.level && cs.methodID == kLZMA)
    {
        {
            NArchive::N7z::CProperty property;
            property.PropID = NCoderPropID::kNumFastBytes;
            property.Value = UInt32(cs.word);
            methodFull.CoderProperties.Add(property);
            DebugString("Property: kNumFastBytes = %u", UInt32(cs.word));
        }
        {
            NArchive::N7z::CProperty property;
            property.PropID = NCoderPropID::kDictionarySize;
            property.Value = UInt32(cs.dict);
            methodFull.CoderProperties.Add(property);
            DebugString("Property: kDictionarySize = %u", UInt32(cs.dict));
        }
    }
    // Add PPMd properties
    else if(cs.level && cs.methodID == kPPMd)
    {
        NArchive::N7z::CProperty property;
        property.PropID = NCoderPropID::kOrder;
        property.Value = UInt32(cs.word);
        methodFull.CoderProperties.Add(property);
        DebugString("Property: kOrder = %u", UInt32(cs.word));
    }
    methodMode.Methods.Add(methodFull);
    methodMode.PasswordIsDefined = (*cs.password != 0);
    if(methodMode.PasswordIsDefined) methodMode.Password = MultiByteToUnicodeString(cs.password);
    DebugString("Update: Password: %s, Using: %s", cs.password, methodMode.PasswordIsDefined ? "yes" : "no");

    options.Method = &methodMode;
    options.HeaderMethod = &headerMode;
    options.UseFilters = false;
    options.MaxFilter = 0;
    options.UseAdditionalHeaderStreams = false;
    options.CompressMainHeader = true;
    options.NumSolidFiles = (cs.flags & FLAG_CREATE_SOLID) ? UInt64(Int64(-1)) : 1;
    options.NumSolidBytes = UInt64(Int64(-1));
    options.SolidExtension = false;
    options.RemoveSfxBlock = false;
    options.VolumeMode = false;

    // Init streams
    COutFileStream *outStreamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> outStream(outStreamSpec);
    CInFileStream *inStreamSpec = new CInFileStream;
    CMyComPtr<IInStream> inStream;

    // Try to open existing archive
    NArchive::N7z::CArchiveDatabaseEx db;
    BOOL openOK = FALSE;
    char tempFile[MAX_PATH] = "\0";
    if(inStreamSpec->Open(packedFile))
    {
        NArchive::N7z::CInArchive archive;
        DebugString("7zUpdate opened file: %s", packedFile);
        if(archive.Open(inStreamSpec, &gMaxCheckStartPosition) == S_OK)
        {
            DebugString("7zUpdate opened archive: %s", packedFile);
            if(archive.ReadDatabase(db, NULL) == S_OK)
            {
                DebugString("7zUpdate reading db: %s", packedFile);
                db.Fill();
                DebugString("7zUpdate checking %d files:", db.Files.Size());
                for(int i = 0; i < db.Files.Size(); i++)
                {
                    int ui;
                    for(ui = 0; ui < updateItems.Size(); ui++)
                        if(updateItems[ui].Name == db.Files[i].Name)
                        {
                            updateItems[ui].IndexInArchive = i;
                            DebugString("                 [upd] %s", (const char *)UnicodeStringToMultiByte(db.Files[i].Name));
                            break;
                        }
                    if(ui == updateItems.Size())
                    {
                        char *delName = deleteList;
                        BOOL toDelete = FALSE;
                        while(delName && *delName)
                        {
                            if(NArchive::NItemName::MakeLegalName(MultiByteToUnicodeString(delName)) == db.Files[i].Name)
                            {
                                toDelete = TRUE;
                                DebugString("                 [del] %s", delName);
                                break;
                            }
                            delName += strlen(delName) + 1;
                        }
                        if(!toDelete)
                        {
                            NArchive::N7z::CUpdateItem updateItem;
                            updateItem.NewProperties = false;
                            updateItem.NewData = false;
                            updateItem.IndexInArchive = i;
                            updateItem.IndexInClient = iNum ++;
                            updateItem.IsAnti = false;
                            updateItems.Add(updateItem);
                            DebugString("                 [old] %s", (const char *)UnicodeStringToMultiByte(db.Files[i].Name));
                        }
                    }
                }

                openOK = TRUE;
                inStream = inStreamSpec;
            }
        }
    }

    // Check if we couldn't open archive even we have something in the delete list
    if(!openOK && deleteList) return 0;

    // Check if we managed to open old archive / then create temp file for it
    if(openOK)
    {
        int i;
        for(i = 0; i < 1 << 8; i++)
        {
            _snprintf(tempFile, MAX_PATH, "%s.tmp%d", packedFile, i);
            DebugString("7zUpdate creating temp: %s", tempFile);
            if(outStreamSpec->Create(tempFile, false))
                break;
        }
        if(i == 1 << 8) return E_ECREATE;
    }
    else if(!outStreamSpec->Create(packedFile, false))
        return E_ECREATE;

    BOOL bAbort = FALSE;
    C7zUpdateCallback *updateCallbackSpec = new C7zUpdateCallback(&files, srcPath, &bAbort);
    CMyComPtr<IArchiveUpdateCallback> updateCallback(updateCallbackSpec);
    HRESULT hResult = NArchive::N7z::Update(
        openOK ? inStream : NULL,
        openOK ? &db : NULL,
        updateItems, outStream, updateCallback, options);

    DebugString("NArchive::N7z::Update: %x", hResult);
    inStream.Release();
    DebugString("7zUpdate: Releasing stream in");
    outStream.Release();
    DebugString("7zUpdate: Releasing stream out");

    // Get rid of temp file
    if(*tempFile && (hResult != S_OK || !::MoveFileEx(tempFile, packedFile, MOVEFILE_REPLACE_EXISTING)))
    {
        ::DeleteFile(tempFile);
        DebugString("7zUpdate: Cannot move %s to %s.", tempFile, packedFile);
    }
    // Get rid of file that's packed badly
    if(!*tempFile && hResult != S_OK)
        ::DeleteFile(packedFile);
    // If move files requested
    if(hResult == S_OK && (flags & PK_PACK_MOVE_FILES))
    {
        for(int i = 0; i < files.Size(); i++)
            ::DeleteFileW(files[i]);
    }

    DebugString("7zUpdate DONE");

    return (hResult == E_NOTIMPL) ? E_NOT_SUPPORTED : (hResult != S_OK && !bAbort) ? E_EWRITE : 0;
}

int __stdcall PackFiles (char *packedFile, char *subPath, char *srcPath, char *addList, int flags)
{
    DebugString("PackFiles PackedFile: %s, SubPath: %s, SrcPath: %s, AddList: %s, Flags: %d",
                            packedFile, subPath, srcPath, addList, flags);
    return Update(packedFile, subPath, srcPath, addList, NULL, flags);
}

int __stdcall DeleteFiles (char *packedFile, char *deleteList)
{
    DebugString("DeleteFiles PackedFile: %s, DeleteList: %s",
                            packedFile, deleteList);
    return Update(packedFile, NULL, NULL, NULL, deleteList, 0);
}

BOOL __stdcall CanYouHandleThisFile (char *fileName)
{
    // Try to open archive file name
    CInFileStream *inStreamSpec = new CInFileStream();
    CMyComPtr<IInStream> inStream(inStreamSpec);
    if(!inStreamSpec->Open(fileName))
        return FALSE;

    NArchive::N7z::CInArchive archive;
    HRESULT result;
    if((result = archive.Open(inStream, &gMaxCheckStartPosition)) != S_OK)
        return FALSE;

    return TRUE;
}

BOOL CALLBACK ConfigDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void __stdcall ConfigurePacker (HWND parent, HINSTANCE dllInstance)
{
    DialogBox(dllInstance, MAKEINTRESOURCE(IDD_CONFIG), parent, ConfigDialog);
}

}
