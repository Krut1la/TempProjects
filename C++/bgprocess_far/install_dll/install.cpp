// install_dll.cpp : Defines the entry point for the DLL application.
//
#include "../include/stdafx.h"
#include "../include/adapted.h"
#include "../include/install.h"
//#include "../include/file.h"
//#include "../include/registry.h"
//#include "../include/wcx.h"

using namespace install;

DWORD CALLBACK Installer::InstallPreCallback(CallBackMSG msg, TCHAR* str, LONG P1, LONG P2, LPVOID userdata)
{
	//char *sdfsdfsd = (char*)str;
	if(userdata != NULL)
	{
		Installer* installer = static_cast<Installer*>(userdata);
		switch(msg)
		{
		case msgCurrentOperationProgess:
			return installer->InstallCallback(msg, str, P1, P2, installer->userdata);
		case msgRequest:
			return installer->InstallCallback(msg, str, P1, P2, installer->userdata);
		default:
			break;
		};
	}

	return S_FALSE;
}

void XMLCALL
Installer::startElement(void *_userData, const XML_Char *_element, const XML_Char **_attrs)
{
	HRESULT hr;

	Installer* installer = static_cast<Installer*>(_userData);

	kru_string element = _element;

	/*int i = 0;

	Installer* installer = static_cast<Installer*>(_userData);

	kru_string element = _element;


	AttributeMap attrs = installer->elements[element];
	size_t n_attrs = attrs.size();

	if(attrs.size() != 0)
	{
		while(*(_attrs + i))
		{
			kru_string attr = *(_attrs + i);
			kru_string attrtext = *(_attrs + i + 1);
			installer->ExpandString(attrtext);

			attrs[attr] = attrtext;

			i+=2;
		}

		kru_string missing_attrs = _TEXT("");

		AttributeMap::iterator i_attr;
		for(i_attr = attrs.begin(); i_attr != attrs.end(); ++i_attr)
		{
			if((*i_attr).second == _TEXT(""))
			{
				missing_attrs += (*i_attr).first;
				break;
			}
		}

		if(attrs.size() != n_attrs || missing_attrs != _TEXT(""))
		{

			if(installer->InstallCallback)
				installer->InstallCallback(msgWarning, 
				(TCHAR*)(element + _TEXT(": missing attributes(" + missing_attrs + _TEXT(")"))).c_str(),
				0,
				0,
				installer->userdata);
		}
		else
		{
			if(element == _TEXT("CreateRegistryValue"))
			{
				hr = install::registry::CreateRegistryValue(
					attrs[_TEXT("rootkey")].c_str(),
					attrs[_TEXT("key")].c_str(),
					attrs[_TEXT("valuename")].c_str(),
					attrs[_TEXT("type")].c_str(),
					attrs[_TEXT("data")].c_str());
			}
			else if(element == _TEXT("DeleteRegistryValue"))
			{
				hr = install::registry::DeleteRegistryValue(
					attrs[_TEXT("rootkey")].c_str(),
					attrs[_TEXT("key")].c_str(),
					attrs[_TEXT("valuename")].c_str());
			}
			else if(element == _TEXT("DeleteRegistryKey"))
			{
				hr = install::registry::DeleteRegistryKey(
					attrs[_TEXT("rootkey")].c_str(),
					attrs[_TEXT("key")].c_str(),
					attrs[_TEXT("subkey")].c_str());
			}
			else if(element == _TEXT("CopyFile"))
			{
				hr = install::file::CopyFile(
					attrs[_TEXT("source")].c_str(),
					attrs[_TEXT("destination")].c_str(),
					attrs[_TEXT("flags")].c_str(),
					InstallPreCallback,
					_userData);
			}
			else if(element == _TEXT("CopyFolder"))
			{
				hr = install::file::CopyFolder(
					attrs[_TEXT("source")].c_str(),
					attrs[_TEXT("destination")].c_str(),
					attrs[_TEXT("flags")].c_str(),
					InstallPreCallback,
					_userData);
			}
			else if(element == _TEXT("DeleteFile"))
			{
				hr = install::file::DeleteFile(
					attrs[_TEXT("target")].c_str(),
				    attrs[_TEXT("flags")].c_str(),
					InstallPreCallback,
					_userData);
			}
			else if(element == _TEXT("DeleteFolder"))
			{
				hr = install::file::DeleteFile(
					attrs[_TEXT("target")].c_str(),
				    attrs[_TEXT("flags")].c_str(),
					InstallPreCallback,
					_userData);
			}
			else if(element == _TEXT("ExtractWCXArchive"))
			{
				hr = install::wcx::ExtractWCXArchive(
					attrs[_TEXT("source")].c_str(),
					attrs[_TEXT("type")].c_str(),
					attrs[_TEXT("files")].c_str(),
					attrs[_TEXT("command")].c_str(),
				    attrs[_TEXT("destination")].c_str(),
					attrs[_TEXT("flags")].c_str(),
					InstallPreCallback,
					_userData);
			}

			//if(hr != S_OK)
			{
				kru_string error_text;
				LPTSTR s;
							
				// получаем текстовое описание ошибки
				BOOL fOk = FormatMessage(
					FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
					NULL, hr, 0,
					(LPTSTR)&s, 0, NULL);
			
				if (s != NULL) {
					error_text = s;
					LocalFree(s);
				} else {
					error_text = _TEXT("Error number not found.");
				}

				if(installer->InstallCallback)
					installer->InstallCallback(msgResult,
					(TCHAR*)(element + _TEXT(": ") + error_text).c_str(), 
					0, 
					hr,
				    installer->userdata);
			}
		}// if-else attributes
	}// if element*/
}

void XMLCALL
Installer::endElement(void *userData, const XML_Char *name)
{
  int *depthPtr = static_cast<int*>(userData);
  *depthPtr -= 1;
}

Installer::Installer(const char* xmlbuffer, int bufflen, INSTALLCALLBACK ic, LPVOID userdata)
//InstallCallback(ic)
{	
	//adaptedOperationList[TEXT("DeleteFile")] = install::file::DeleteFile;
	//Set attributs
/*	AttributeMap attrs;

	attrs[_TEXT("rootkey")] =  _TEXT("");
	attrs[_TEXT("key")] =  _TEXT("");
	attrs[_TEXT("valuename")] =  _TEXT("");
	attrs[_TEXT("type")] =  _TEXT("");
	attrs[_TEXT("data")] =  _TEXT("");
	elements[_TEXT("CreateRegistryValue")] =  attrs;
	attrs.clear();

	attrs[_TEXT("rootkey")] =  _TEXT("");
	attrs[_TEXT("key")] =  _TEXT("");
	attrs[_TEXT("valuename")] =  _TEXT("");
	elements[_TEXT("DeleteRegistryValue")] =  attrs;
	attrs.clear();

	attrs[_TEXT("rootkey")] =  _TEXT("");
	attrs[_TEXT("key")] =  _TEXT("");
	attrs[_TEXT("subkey")] =  _TEXT("");
	elements[_TEXT("DeleteRegistryKey")] =  attrs;
	attrs.clear();

	attrs[_TEXT("source")] =  _TEXT("");
	attrs[_TEXT("destination")] =  _TEXT("");
	attrs[_TEXT("flags")] =  _TEXT("");
	elements[_TEXT("CopyFile")] =  attrs;
	attrs.clear();

	attrs[_TEXT("source")] =  _TEXT("");
	attrs[_TEXT("destination")] =  _TEXT("");
	attrs[_TEXT("flags")] =  _TEXT("");
	elements[_TEXT("CopyFolder")] =  attrs;
	attrs.clear();

	attrs[_TEXT("target")] =  _TEXT("");
	attrs[_TEXT("flags")] =  _TEXT("");
	elements[_TEXT("DeleteFile")] =  attrs;
	attrs.clear();

	attrs[_TEXT("target")] =  _TEXT("");
	attrs[_TEXT("flags")] =  _TEXT("");
	elements[_TEXT("Deletefolder")] =  attrs;
	attrs.clear();

	attrs[_TEXT("source")] =  _TEXT("");
	attrs[_TEXT("type")] =  _TEXT("");
	attrs[_TEXT("files")] =  _TEXT("");
	attrs[_TEXT("command")] =  _TEXT("");
	attrs[_TEXT("destination")] =  _TEXT("");
	attrs[_TEXT("flags")] =  _TEXT("");
	elements[_TEXT("ExtractWCXArchive")] =  attrs;
	attrs.clear();
	*/
	//----------------------------------------------------

    //Initialize Callback---------------------------------
	InstallCallback = ic;
	this->userdata = userdata;
	//----------------------------------------------------

	//Initialize XML Parser-------------------------------
	//parser = XML_ParserCreate(_TEXT("UTF-16"));
	parser = XML_ParserCreate(NULL);
	this->xmlbuffer = xmlbuffer;
	this->bufflen = bufflen;
	XML_SetUserData(parser, this);
	XML_SetElementHandler(parser, startElement, endElement);
	//----------------------------------------------------
}

Installer::~Installer()
{
	XML_ParserFree(parser);
}

DWORD Installer::Install()
{
	if (XML_Parse(parser, xmlbuffer, bufflen, 1) == XML_STATUS_ERROR) {
		int asasa = XML_GetErrorCode(parser);
		if(InstallCallback)
		InstallCallback(msgFatal, (TCHAR*)XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser), 0, userdata);
		return S_FALSE;
	}
	return S_OK;
}

DWORD InstallFromXMLBuffer(const char* xmlbuffer, int bufflen, INSTALLCALLBACK ic, LPVOID userdata)
{
	Installer installer(xmlbuffer, bufflen, ic, userdata);
	
	return installer.Install();
}


//Parse string. "Time is %TIME% now" = "Time is 10:20:30 now"
//void ExpandString(std::string *string, std::string *rvalue)
DWORD Installer::ExpandString(kru_string& str)
{
	HRESULT hr = S_OK;
	//char SpaceMB[21] = {0};
	//Restore originl string
	//rvalue->replace(0,rvalue->length(), string->c_str());
	//LPCSTR ttt= rvalue->c_str();

	size_t i_start = 0, i_end = 0;

	//find first and second '%'
	i_start = str.find(_TEXT("%"), 0);
	i_end = str.find(_TEXT("%"), i_start + 1);

	//until last find '%'-para
	while(i_start != str.npos && i_end != str.npos)
	{

		kru_string substr = str.substr(i_start,i_end - i_start + 1);
		kru_string var = _TEXT("");
		//%TIME%
		if(substr == _TEXT("%TIME%"))
		{
			TCHAR tmpbuf[128];
//			_tstrtime(tmpbuf);
			var = tmpbuf;
			//rvalue->replace(start_sh,end_sh - start_sh + 1,tmpbuf);
		}
		/*//%DATE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%DATE%"))
		{
			var = _TEXT("1, December");
			//rvalue->replace(start_sh,end_sh - start_sh + 1,"1, December");
		}
		//%TITLE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%TITLE%"))
		{
			//rvalue->replace(start_sh,end_sh - start_sh + 1,infoexpander->Title.c_str());
		}
		//%MAINDIR%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%MAINDIR%"))
		{
//			rvalue->replace(start_sh, end_sh - start_sh + 1, MainDir.c_str());
		}		
		//%INST_DRIVE%
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%INST_DRIVE%"))
		{
//			rvalue->replace(start_sh, end_sh - start_sh + 1, MainDrive.c_str());
		}
		//%LABEL_DRIVE_XXXXXXXX%
		else if((end_sh - start_sh + 1) > 12 && !rvalue->compare(start_sh,13,"%LABEL_DRIVE_"))
		{
			//Scan for CD
			char drive[4];
			char volume[255] = {0};

			drive[0] = 'c';
			drive[1] = ':';
			drive[2] = '\\';
			drive[3] = 0;

			std::string label = rvalue->c_str();
			label.erase(end_sh, label.length() - end_sh);
			label.erase(0,start_sh + 13);

			BOOL foundCD = FALSE;
			for (drive[0] = 'c' ; drive[0] <= 'z' ; drive[0]++)
			{
				if (GetDriveType (drive) == DRIVE_CDROM)
				{
					if(GetVolumeInformation(drive,volume,255,0,0,0,0,0))
						if(!strcmp(label.c_str(),volume))
						{
							std::string sdrive = drive;
							sdrive.erase(2,1);
							rvalue->replace(start_sh, end_sh - start_sh + 1, sdrive.c_str()); 
							foundCD = TRUE;
						}
				}
			}
			if(!foundCD){
//				std::string sdrive = MainDrive;
//				sdrive.erase(2,1);
//				rvalue->replace(start_sh, end_sh - start_sh + 1, sdrive.c_str());
			}
		}
		//CSIDL
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_ADMINTOOLS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_ADMINTOOLS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_ALTSTARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_ALTSTARTUP ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_APPDATA%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_APPDATA ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_BITBUCKET%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_BITBUCKET,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_ADMINTOOLS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_ADMINTOOLS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_ALTSTARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_ALTSTARTUP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_APPDATA%")){
			if(Shell32Version >= PACKVERSION(5,0)){
				SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_APPDATA,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
				nSize = GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
				nSize = sprintf(tchBuffer, "%s\\All Users\\Application Data", lpszSystemInfo); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
			}
		}
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_DESKTOPDIRECTORY%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_DESKTOPDIRECTORY,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_DOCUMENTS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_DOCUMENTS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_FAVORITES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_FAVORITES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_PROGRAMS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_PROGRAMS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_STARTMENU%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_STARTMENU,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_STARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_STARTUP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COMMON_TEMPLATES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COMMON_TEMPLATES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_CONTROLS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_CONTROLS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_COOKIES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_COOKIES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_DESKTOP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_DESKTOP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_DESKTOPDIRECTORY%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_DESKTOPDIRECTORY,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_DRIVES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_DRIVES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_FAVORITES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_FAVORITES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_FONTS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_FONTS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_HISTORY%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_HISTORY,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_INTERNET%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_INTERNET,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_INTERNET_CACHE%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_INTERNET_CACHE ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_LOCAL_APPDATA%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_LOCAL_APPDATA ,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_MYMUSIC%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_MYMUSIC,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_MYPICTURES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_MYPICTURES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_NETHOOD%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_NETHOOD,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_NETWORK%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_NETWORK,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PERSONAL%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PERSONAL,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PRINTERS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PRINTERS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PRINTHOOD%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PRINTHOOD,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROFILE%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROFILE,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROGRAM_FILES%")){
			if(Shell32Version >= PACKVERSION(5,0)){
				SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROGRAM_FILES,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
				nSize = GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
				*(lpszSystemInfo + 2) = 0;
				nSize = sprintf(tchBuffer, "%s\\Program Files",	lpszSystemInfo	); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
			}
		}
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROGRAM_FILES_COMMON%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROGRAM_FILES_COMMON,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_PROGRAMS%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_PROGRAMS,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_RECENT%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_RECENT,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_SENDTO%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_SENDTO,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_STARTMENU%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_STARTMENU,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_STARTUP%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_STARTUP,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_SYSTEM%")){
			if(Shell32Version >= PACKVERSION(5,0)){
				SHReplaceCSIDL(start_sh, end_sh, CSIDL_SYSTEM,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
				nSize = GetSystemDirectory(lpszSystemInfo, MAX_PATH); 
				nSize = sprintf(tchBuffer, "%s",	lpszSystemInfo	); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
			}
		}
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_TEMPLATES%"))
			SHReplaceCSIDL(start_sh, end_sh, CSIDL_TEMPLATES,rvalue);
		else if(!rvalue->compare(start_sh,end_sh - start_sh + 1,"%CSIDL_WINDOWS%")){
			if(Shell32Version >= PACKVERSION(5,0)){
				SHReplaceCSIDL(start_sh, end_sh, CSIDL_WINDOWS,rvalue);
			}else{
				//Use an alternate approach for older DLL versions.
				LPTSTR lpszSystemInfo;
				TCHAR tchBuffer2[MAX_PATH];
				TCHAR tchBuffer[MAX_PATH];
				int nSize;               

				lpszSystemInfo = tchBuffer2; 

				// Get and display the system directory. 
				nSize = GetWindowsDirectory(lpszSystemInfo, MAX_PATH); 
				//*(lpszSystemInfo + 2) = 0;
				nSize = sprintf(tchBuffer, "%s",	lpszSystemInfo	); 
				rvalue->replace(start_sh, end_sh - start_sh + 1, tchBuffer);
			}
		}


		
		*/
        size_t varlen = var.length();
		size_t substrlen = substr.length();

        str.replace(i_start, i_end - i_start + 1, var);

		//find next and second '%'
		i_start = str.find(_TEXT("%"), i_end + 1 + (varlen - substrlen));
		i_end = str.find(_TEXT("%"), i_start);


		//else
		//{
			//find next and second '%'
			//start_sh = rvalue->find("%",end_sh + 1);
			//end_sh = rvalue->find("%",start_sh + 1);
			//continue; // to skip this
		//}                    // | 
		//                   // | 
		//                   // V
		//find next and second '%'
		//start_sh = rvalue->find("%",0);
		//end_sh = rvalue->find("%",start_sh + 1);
	}// while

	return hr;
}

/*void SHReplaceCSIDL(size_t start_sh, size_t end_sh, DWORD csidl, std::string *str)
{
	LPSTR path = new char[MAX_PATH];
	LPCSTR val = str->c_str();
	LPMALLOC pMalloc = NULL;
	LPITEMIDLIST ppidl = NULL;

	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	SHGetMalloc(&pMalloc);

	//SHGetSpecialFolderPath(NULL, path, csidl, FALSE);
	SHGetSpecialFolderLocation(NULL,csidl,&ppidl);
	if(ppidl)
	{
		//LPSTR path = new char[MAX_PATH];
		SHGetPathFromIDList(ppidl,path);
	}

	str->replace(start_sh, end_sh - start_sh + 1, path);
	delete path;

	pMalloc->Release();
	CoUninitialize();
}
*/
/*
DWORD GetDllVersion(LPCTSTR lpszDllName)
{

    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

  //Because some DLLs might not implement this function, you
  //must test for it explicitly. Depending on the particular 
  //DLL, the lack of a DllGetVersion function can be a useful
  //indicator of the version.

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }
        
        FreeLibrary(hinstDll);
    }
    return dwVersion;
}*/