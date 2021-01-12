#ifndef FILE_H
#define FILE_H

//#include "install.h"


namespace install{
	namespace file{

		typedef std::pair<DWORD, kru_string> FILEDESC;
		typedef std::vector<FILEDESC> FILELIST;

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
			flgMove = 512
		};

		enum Check{
			chkAbort = 10000,
			chkSkip,
			chkContinue,
			chkError
		};

		DWORD CALLBACK CopyProgressRoutine(
			LARGE_INTEGER TotalFileSize,
			LARGE_INTEGER TotalBytesTransferred,
			LARGE_INTEGER StreamSize,
			LARGE_INTEGER StreamBytesTransferred,
			DWORD dwStreamNumber,
			DWORD dwCallbackReason,
			HANDLE hSourceFile,
			HANDLE hDestinationFile,
			LPVOID lpData
			);

		DWORD CopyFlagsToDWORD(const TCHAR* flags);
		DWORD ScanFolder(FILELIST &stringList, kru_string folderName);
		DWORD CreateDirectoryExEx(kru_string path);

		/*DWORD FileNewer(
			const TCHAR* source,
			const TCHAR* destination,
			DWORD dwFlags,
			INSTALLCALLBACK callback,
			LPVOID userdata);
		DWORD FileReadOnly(
			const TCHAR* filename,
			DWORD dwFlags,
			INSTALLCALLBACK callback,
			LPVOID userdata);
		DWORD FileExist(
			const TCHAR* filename,
			DWORD dwFlags,
			INSTALLCALLBACK callback,
			LPVOID userdata);

		INSTALL_DLL_API  DWORD CopyFile(
			const TCHAR* filename,
			const TCHAR* destination,
			const TCHAR* flags,
			INSTALLCALLBACK callback = NULL,
			LPVOID userdata = NULL);
		INSTALL_DLL_API  DWORD CopyFolder(
			const TCHAR* foldername, 
			const TCHAR* destination, 
			const TCHAR* flags,
			INSTALLCALLBACK callback = NULL,
			LPVOID userdata = NULL);
		INSTALL_DLL_API  DWORD DeleteFile(
			const TCHAR* filename, 
			const TCHAR* flags,
			INSTALLCALLBACK callback = NULL,
			LPVOID userdata = NULL);
		INSTALL_DLL_API  DWORD DeleteFolder(
			const TCHAR* foldername, 
			const TCHAR* flags,
			INSTALLCALLBACK callback = NULL,
			LPVOID userdata = NULL);*/

		INSTALL_DLL_API DWORD DeleteFile(
			STRINGLIST& params,
			INSTALLCALLBACK callback = NULL,
			LPVOID userdata = NULL);
	}
}


#endif //FILE_H