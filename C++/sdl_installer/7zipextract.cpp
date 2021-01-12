#include "7zipextract.h"
#include "infoexpander.h"

//Progress data. For external use
extern InfoExpander *infoexpander;
extern CRITICAL_SECTION InstallerCS;
extern BOOL Successfull;

extern __int64 ArchiveUnpacked;
extern __int64 ArchiveUnpackedSize;
extern __int64 FileUnpacked;
extern __int64 FileUnpackedSize;


extern BOOL isUserCancel;
__int64 For7zipDiff = 0;

typedef DWORD (*ExtractF)(LPCSTR archive, LPCSTR destpath, UN7ZIPCALLBACK callback);
ExtractF Extract7zip = NULL;

HMODULE Library7zip = NULL;


std::string DestPath7zip_s;


DWORD Extract7zipArchive(LPSTR ArcName,int Mode, LPSTR DestPath)
{
	DestPath7zip_s = DestPath;

	HRESULT hr = NULL;

	For7zipDiff = 0;
	isUserCancel = false;
	ArchiveUnpackedSize = infoexpander->RequiredSize;
	
	Library7zip = LoadLibrary("asd.dll");
	if(!Library7zip) MessageBox(infoexpander->hWndMain,"Can't open 7za.dll", "Error", MB_ICONERROR);
	Extract7zip = (ExtractF)GetProcAddress(Library7zip, "KExtract");

	if(Extract7zip) hr = Extract7zip(ArcName, DestPath,(UN7ZIPCALLBACK)callback7zip);
	else MessageBox(infoexpander->hWndMain,"Bad 7za.dll", "Error", MB_ICONERROR);

	infoexpander->SetFileProgress(100);
	ArchiveUnpackedSize = 0;
	FileUnpacked = 0;
	FileUnpackedSize = 0;
	if(isUserCancel) Cancel7zipExtract();

  return hr;
}

void User7zipCancel(void)
{
	EnterCriticalSection(&InstallerCS);
	isUserCancel = TRUE;
	LeaveCriticalSection(&InstallerCS);
}

void Cancel7zipExtract(void)
{
	//if(hArcData) 
	{
		
		if(Library7zip) FreeLibrary(Library7zip);
		ThrowXML(RuntimeException, XMLExcepts::NoError); //_endthreadex(-1);
	}
}


CALLBACK callback7zip(UINT msg,UINT64 UserData, UINT64 P1, LPCSTR P2)
{
	//char *prc_s = new char[50];
	switch(msg)
	{
	case UCM_SETTOTAL:

		if(FileUnpackedSize != UserData) FileUnpacked = 0;
		if(UserData)
			FileUnpackedSize = UserData;

        break;
	case UCM_ISCANCEL:
		{
		BOOL triger = FALSE;
		EnterCriticalSection(&InstallerCS);
		if(isUserCancel) triger = TRUE;			
		LeaveCriticalSection(&InstallerCS);
		if(triger) {
			infoexpander->SetArchiveProgress(0);
		    infoexpander->SetFileProgress(0);
		    ArchiveUnpacked = 0;
		    ArchiveUnpackedSize = 0;
		    FileUnpacked = 0;
		    FileUnpackedSize = 0;
			return 1;
		}
			//ThrowXML(RuntimeException, XMLExcepts::NoError); //_endthreadex(-1);
		}
		break;
	case UCM_SETCOMPLETE:
		if(UserData){
		
		if(FileUnpacked <= FileUnpackedSize) FileUnpacked += UserData - For7zipDiff;
		ArchiveUnpacked += UserData - For7zipDiff;
		For7zipDiff = UserData;
		if(FileUnpacked <= FileUnpackedSize) infoexpander->SetFileProgress((int)(((double)FileUnpacked/((double)FileUnpackedSize + .0001f))*100));
		infoexpander->SetArchiveProgress((int)(((double)ArchiveUnpacked/((double)ArchiveUnpackedSize + .0001f))*100));
		}
		break;
	case UCM_NAME:
		if(infoexpander->GetCurrentOperation() != P2){
			FileUnpacked = 0;
			if(infoexpander->InstallLog)
			{
				HRESULT hr = 0;
				DWORD NumberOfBytesWriten = 0;
				//std::string xmlstring = "<file path =\"" + infoexpander->MainDir + "\\"+ HeaderData.FileName + "\"\\>\n";

				std::string xmlstring;
				//XML
				if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<file path =\"" + DestPath7zip_s + "\\"+ P2 + "\"\\>\r\n";
				//UNWISE
				if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "File Copy: " + DestPath7zip_s + "\\"+ P2 + "\r\n";

				hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
				if(!hr)
					PG_LogERR("ExtractArchive: Can't write to %s", infoexpander->IL_Filename.c_str());    
			}
		}
		infoexpander->SetCurrentOperation(P2);
		
		break;	
    case UCM_ERROR:
		MessageBox(infoexpander->hWndMain, "7zip: ошибка чтения", "ошибка", MB_ICONERROR);
		break;
	
	}
	return(0);
}
