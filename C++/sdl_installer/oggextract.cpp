#include "Oggextract.h"
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
__int64 ForOggDiff = 0;

oggextractF oggextract = NULL;

HMODULE LibraryOgg = NULL;

extern oggextractF oggextract;

std::string DestPathOgg_s;


DWORD ExtractOggArchive(LPSTR ArcName,int Mode, LPSTR DestPath)
{
	LibraryOgg = LoadLibrary("oggd.dll");
	if(!LibraryOgg) MessageBox(infoexpander->hWndMain,"Can't open oggd.dll", "Error", MB_ICONERROR);
	oggextract = (oggextractF)GetProcAddress(LibraryOgg, "oggextract");

	DestPathOgg_s = DestPath;

	HRESULT hr = NULL;

	ForOggDiff = 0;
	isUserCancel = false;
	ArchiveUnpackedSize = infoexpander->RequiredSize;

	if(oggextract) hr = oggextract(ArcName, DestPath,(UNOGGCALLBACK)callbackOgg);
	else MessageBox(infoexpander->hWndMain,"Bad oggd.dll", "Error", MB_ICONERROR);

	if(infoexpander->InstallLog)
	{
		HRESULT hr = 0;
		DWORD NumberOfBytesWriten = 0;
		//std::string xmlstring = "<file path =\"" + infoexpander->MainDir + "\\"+ HeaderData.FileName + "\"\\>\n";

		std::string xmlstring;
		//XML
		if(infoexpander->InstallLogType == ILT_XML) xmlstring = "<file path =\"" + DestPathOgg_s + "\"\\>\r\n";
		//UNWISE
		if(infoexpander->InstallLogType == ILT_UNWISE) xmlstring = "File Copy: " + DestPathOgg_s + "\r\n";

		hr = WriteFile(infoexpander->InstallLog, xmlstring.c_str(),xmlstring.length(),&NumberOfBytesWriten,NULL);
		if(!hr)
			PG_LogERR("ExtractArchive: Can't write to %s", infoexpander->IL_Filename.c_str());    
	}

	infoexpander->SetFileProgress(100);
	ArchiveUnpackedSize = 0;
	FileUnpacked = 0;
	FileUnpackedSize = 0;
	if(isUserCancel) CancelOggExtract();
	if(LibraryOgg) {FreeLibrary(LibraryOgg); LibraryOgg = NULL;}

  return hr;
}

void UserOggCancel(void)
{
	EnterCriticalSection(&InstallerCS);
	isUserCancel = TRUE;
	LeaveCriticalSection(&InstallerCS);
}

void CancelOggExtract(void)
{
	//if(hArcData) 
	{
		
		//if(LibraryOgg) FreeLibrary(LibraryOgg);
		ThrowXML(RuntimeException, XMLExcepts::NoError); //_endthreadex(-1);
	}
}


DWORD __stdcall callbackOgg(UINT msg,UINT64 UserData, UINT64 P1, LPCSTR P2)
{
	switch(msg)
	{
	case UOGG_ISCANCEL:
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
	case UOGG_SETCOMPLETE:
		if(UserData){
		infoexpander->SetFileProgress((UINT)UserData);
		ArchiveUnpacked += P1*2 - ForOggDiff;
		ForOggDiff = P1*2;
		infoexpander->SetArchiveProgress((int)(((double)ArchiveUnpacked/((double)ArchiveUnpackedSize + .0001f))*100));
		}
		break;
	case UOGG_ERROR:
		MessageBox(infoexpander->hWndMain, P2, "Oרטבka", MB_ICONERROR);
		PG_LogERR(P2);
		break;
	
	}
	return(0);
}
