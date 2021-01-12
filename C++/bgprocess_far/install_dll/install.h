#ifndef INSTALL_H
#define INSTALL_H



namespace install{
//typedef std::map<kru_string, kru_string> AttributeMap;
//typedef std::map<kru_string, AttributeMap> XMLElementMap;

class Installer{
	//Attributes------------------------------------------
	//XMLElementMap elements;

	ADAPTED_OPERATION_LIST adaptedOperationList;
	//----------------------------------------------------
	      
	//Main variables -------------------------------------
	kru_string maindir;
	//----------------------------------------------------

	static DWORD CALLBACK CopyProgressRoutine(
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

	static DWORD CALLBACK InstallPreCallback(CallBackMSG msg, TCHAR* str, LONG P1, LONG P2, LPVOID userdata);
	INSTALLCALLBACK InstallCallback;
	LPVOID userdata;

	DWORD ExpandString(kru_string& str);

	//XML Parser------------------------------------------
	XML_Parser parser;
	const char* xmlbuffer;
	int bufflen;

	static void XMLCALL
    startElement(void *userData, const XML_Char *name, const XML_Char **atts);

	static void XMLCALL
    endElement(void *userData, const XML_Char *name);
	//----------------------------------------------------
public:	
	Installer(const char* xmlbuffer, int bufflen, INSTALLCALLBACK ic, LPVOID userdata);
	DWORD Install();
	~Installer();
};


//void SHReplaceCSIDL(size_t start_sh, size_t end_sh, DWORD csidl, std::string *str);
//DWORD GetDllVersion(LPCTSTR lpszDllName);


INSTALL_DLL_API DWORD InstallFromXMLBuffer(const char* xmlstring, int len, INSTALLCALLBACK ic = 0, LPVOID userdata = 0);


}// install
#endif //INSTALL_H