#ifndef WCX_H
#define WCX_H

#include "install.h"
#include "file.h"

namespace install
{
	namespace wcx
	{

		// Error codes returned to calling application
		const int E_END_ARCHIVE				 = 10;	// No more files in archive
		const int E_NO_MEMORY				 = 11;	// Not enough memory
		const int E_BAD_DATA				 = 12;	// Data is bad
		const int E_BAD_ARCHIVE				 = 13;	// CRC error in archive data
		const int E_UNKNOWN_FORMAT			 = 14;	// Archive format unknown
		const int E_EOPEN					 = 15;	// Cannot open existing file
		const int E_ECREATE					 = 16;	// Cannot create file
		const int E_ECLOSE					 = 17;	// Error closing file
		const int E_EREAD					 = 18;	// Error reading from file
		const int E_EWRITE					 = 19;	// Error writing to file
		const int E_SMALL_BUF				 = 20;	// Buffer too small
		const int E_EABORTED				 = 21;	// Function aborted by user
		const int E_NO_FILES				 = 22;	// No files found
		const int E_TOO_MANY_FILES			 = 23;	// Too many files to pack
		const int E_NOT_SUPPORTED			 = 24;	// Function not supported

		// flags for unpacking
		const int PK_OM_LIST				 = 0;
		const int PK_OM_EXTRACT				 = 1;

		// flags for ProcessFile
		const int PK_SKIP					 = 0;	// Skip this file
		const int PK_TEST					 = 1;	// Test file integrity
		const int PK_EXTRACT				 = 2;	// Extract to disk

		// Flags passed through ChangeVolProc
		const int PK_VOL_ASK				 = 0;	// Ask user for location of next volume
		const int PK_VOL_NOTIFY				 = 1;	// Notify app that next volume will be unpacked

		// Flags for packing

		// For PackFiles
		const int PK_PACK_MOVE_FILES		 = 1;	// Delete original after packing
		const int PK_PACK_SAVE_PATHS		 = 2;	// Save path names of files

		const int PK_CAPS_NEW				 = 1;	// Can create new archives
		const int PK_CAPS_MODIFY			 = 2;	// Can modify exisiting archives
		const int PK_CAPS_MULTIPLE			 = 4;	// Archive can contain multiple files
		const int PK_CAPS_DELETE			 = 8;	// Can delete files
		const int PK_CAPS_OPTIONS			 = 16;	// Has options dialog
		const int PK_CAPS_MEMPACK			 = 32;	// Supports packing in memory
		const int PK_CAPS_BY_CONTENT		 = 64;	// Detect archive type by content
		const int PK_CAPS_SEARCHTEXT		 = 128;	// Allow searching for text in archives
													// created with this plugin}
		const int PK_CAPS_HIDE				 = 256;	// Show as normal files (hide packer
		const int PK_CAPS_UNICODE			 = 512;	// Supports UNICODE

		// Flags for packing in memory 
		const int MEM_OPTIONS_WANTHEADERS	 = 1;	// Return archive headers with packed data

		// Errors returned by PackToMem 
		const int MEMPACK_OK				 = 0;	// Function call finished OK, but there is more data 
		const int MEMPACK_DONE				 = 1;	// Function call finished OK, there is no more data  


		typedef struct {
			char ArcName[MAX_PATH];
			char FileName[MAX_PATH];
			int Flags;
			int PackSize;
			int UnpSize;
			int HostOS;
			int FileCRC;
			int FileTime;
			int UnpVer;
			int Method;
			int FileAttr;
			char* CmtBuf;
			int CmtBufSize;
			int CmtSize;
			int CmtState;
		} HeaderDataA;

		typedef struct {
			wchar_t ArcName[MAX_PATH];
			wchar_t FileName[MAX_PATH];
			int Flags;
			int PackSize;
			int UnpSize;
			int HostOS;
			int FileCRC;
			int FileTime;
			int UnpVer;
			int Method;
			int FileAttr;
			char* CmtBuf;
			int CmtBufSize;
			int CmtSize;
			int CmtState;
		} HeaderDataW;

		typedef struct {
			const char* ArcName;
			int OpenMode;
			int OpenResult;
			char* CmtBuf;
			int CmtBufSize;
			int CmtSize;
			int CmtState;
		} OpenArchiveDataA;

		typedef struct {
			const wchar_t* ArcName;
			int OpenMode;
			int OpenResult;
			char* CmtBuf;
			int CmtBufSize;
			int CmtSize;
			int CmtState;
		} OpenArchiveDataW;

		typedef struct {
			int size;
			DWORD PluginInterfaceVersionLow;
			DWORD PluginInterfaceVersionHi;
			char DefaultIniName[MAX_PATH];
		} PackDefaultParamStruct;

		// Definition of callback functions called by the DLL
		// Ask to swap disk for multi-volume archive
		typedef int (__stdcall *LPCHANGE_VOL_PROC)(char *ArcName,int Mode);
		typedef int (__stdcall *LPCHANGE_VOL_PROCW)(wchar_t *ArcName,int Mode);
		// Notify that data is processed - used for progress dialog
		typedef int (__stdcall *LPPROCESS_DATA_PROC)(char *FileName,int Size);
		typedef int (__stdcall *LPPROCESS_DATA_PROCW)(wchar_t *FileName,int Size);


		enum ArcCommand
		{
			acExtract = 0,
			acExtractNoPath,
			acTest,
			acDel,
			acComment,
			acCommentFiles,
			acConvertToSFX,
			acLock,
			acAddRecoveryRecord,
			acRecover,
			acAddFiles,
			acMoveFiles,
			acAddFilesAndFolders,
			acMoveFilesAndFolders,
			acAllFilesMask
		};

		typedef struct{
			size_t pos;
			//__int64 pos;
			BYTE byte;
		}WriteElement;

		typedef HANDLE (__stdcall *LPOPEN_ARCHIVEA)(OpenArchiveDataA* archiveData);
		typedef HANDLE (__stdcall *LPOPEN_ARCHIVEW)(OpenArchiveDataW* archiveData);

		typedef int    (__stdcall *LPCLOSE_ARCHIVE)(HANDLE handle);

		typedef int    (__stdcall *LPREAD_HEADERA)(HANDLE handle, HeaderDataA* headerData);
		typedef int    (__stdcall *LPREAD_HEADERW)(HANDLE handle, HeaderDataW* headerData);

		typedef int    (__stdcall *LPPROCESS_FILEA)(HANDLE handle, int operation, const char* destPath, const char* destName);
		typedef int    (__stdcall *LPPROCESS_FILEW)(HANDLE handle, int operation, const wchar_t* destPath, const wchar_t* destName);

		typedef int    (__stdcall *LPPACK_FILESA)(const char* packedFile, const char* subPath, const char* srcPath, const char* addList, int flags);
		typedef int    (__stdcall *LPPACK_FILESW)(wchar_t* packedFile, wchar_t* subPath, wchar_t* srcPath, wchar_t* addList, int flags);

		typedef int    (__stdcall *LPDELETE_FILESA)(char* packedFile, char* deleteList);
		typedef int    (__stdcall *LPDELETE_FILESW)(wchar_t* packedFile, wchar_t* deleteList);

		typedef void   (__stdcall *LPSET_CHANGE_VOL_PROC)(HANDLE handle, LPCHANGE_VOL_PROC changeVolProc);
		typedef void   (__stdcall *LPSET_CHANGE_VOL_PROCW)(HANDLE handle, LPCHANGE_VOL_PROCW changeVolProc);

		typedef void   (__stdcall *LPSET_PROCESS_DATA_PROC)(HANDLE handle, LPPROCESS_DATA_PROC processDataProc);
		typedef void   (__stdcall *LPSET_PROCESS_DATA_PROCW)(HANDLE handle, LPPROCESS_DATA_PROCW processDataProc);

		typedef void   (__stdcall *LPCONFIGURE_PACKER)(HWND parent, HINSTANCE dllInstance);
		typedef int    (__stdcall *LPGET_PACKER_CAPS)();
		typedef BOOL   (__stdcall *LPCAN_YOU_HANDLE_THIS_FILEA)(const char* fileName);
		typedef BOOL   (__stdcall *LPCAN_YOU_HANDLE_THIS_FILEW)(const wchar_t* fileName);

		class ArcCommands{
			std::map<kru_string, ArcCommand> commands;
		public:
			ArcCommands(){
				commands[_TEXT("acExtract")] = acExtract;
				commands[_TEXT("acExtractNoPath")] = acExtractNoPath;
				commands[_TEXT("acTest")] = acTest;
				commands[_TEXT("acDel")] = acDel;
				commands[_TEXT("acComment")] = acComment;
				commands[_TEXT("acCommentFiles")] = acCommentFiles;
				commands[_TEXT("acConvertToSFX")] = acConvertToSFX;
				commands[_TEXT("acLock")] = acLock;
				commands[_TEXT("acAddRecoveryRecord")] = acAddRecoveryRecord;
				commands[_TEXT("acRecover")] = acRecover;
				commands[_TEXT("acAddFiles")] = acAddFiles;
				commands[_TEXT("acMoveFiles")] = acMoveFiles;
				commands[_TEXT("acAddFilesAndFolders")] = acAddFilesAndFolders;
				commands[_TEXT("acMoveFilesAndFolders")] = acMoveFilesAndFolders;
				commands[_TEXT("acAllFilesMask")] = acAllFilesMask;
			}
			
			ArcCommand& operator[](const kru_string& rhs)
			{
				return commands[rhs];
			}
		};

		class WCXArchive;

		class WCXModule
		{
			HMODULE hModule;
			kru_string name;
			int     caps;
			
			LPOPEN_ARCHIVEA				OpenArchiveA;
			LPOPEN_ARCHIVEW				OpenArchiveW;
			LPCLOSE_ARCHIVE				CloseArchive;
			LPREAD_HEADERA				ReadHeaderA;
			LPREAD_HEADERW				ReadHeaderW;
			LPPROCESS_FILEA				ProcessFileA;
			LPPROCESS_FILEW				ProcessFileW;
			LPPACK_FILESA				PackFilesA;
			LPPACK_FILESW				PackFilesW;
			LPDELETE_FILESA				DeleteFilesA;
			LPDELETE_FILESW				DeleteFilesW;
			LPSET_CHANGE_VOL_PROC		SetChangeVolProc;
			LPSET_CHANGE_VOL_PROCW		SetChangeVolProcW;
			LPSET_PROCESS_DATA_PROC		SetProcessDataProc;
			LPSET_PROCESS_DATA_PROCW	SetProcessDataProcW;
			LPCONFIGURE_PACKER			ConfigurePacker;
			LPGET_PACKER_CAPS			GetPackerCaps;
			LPCAN_YOU_HANDLE_THIS_FILEA CanYouHandleThisFileA;
			LPCAN_YOU_HANDLE_THIS_FILEW CanYouHandleThisFileW;

		public:
			friend WCXArchive;
			bool Constructed;
			bool WCXModule::SupportCommand(ArcCommand command);
			bool WCXModule::SupportCommand(const kru_string& command);
			void Open(const kru_string& path, const kru_string& name);
			WCXModule(){}
			WCXModule(const kru_string& path, const kru_string& name);
			~WCXModule();
		};


		class WCXArchive
		{
			HANDLE		handle;
			WCXModule	wcxModule;
			kru_string	name;
			std::string mbName;
			INSTALLCALLBACK callback;
			LPVOID userdata;

		public:
			bool Constructed;
			DWORD WCXArchive::Extract(
				const TCHAR* files,
				const TCHAR* sub_path,
				const TCHAR* dest,
				const TCHAR* flags,
				bool path,
				bool test = false);
			WCXArchive(
				const kru_string& name,
				const kru_string& type,
				bool test = false,
				INSTALLCALLBACK callback = NULL,
				LPVOID userdata = NULL);
			~WCXArchive();
		};

		typedef std::vector<WCXModule> WCXModuleList;
		/*struct SEnum
		{
			WCXModule*	wcx;
			int			num;
		};*/

		INSTALL_DLL_API  DWORD ExtractWCXArchive(
			const TCHAR* source,
			const TCHAR* type,
			const TCHAR* files,
			const TCHAR* command,
			const TCHAR* destination, 
			const TCHAR* flags, 
			INSTALLCALLBACK callback,
			LPVOID userdata);

	}//namespace wcx
}//namespace install
#endif //WCX_H