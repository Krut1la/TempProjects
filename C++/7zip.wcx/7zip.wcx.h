// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ZIPWCX_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ZIPWCX_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ZIPWCX_EXPORTS
#define ZIPWCX_API __declspec(dllexport)
#else
#define ZIPWCX_API __declspec(dllimport)
#endif

// This class is exported from the 7zip.wcx.dll
class ZIPWCX_API Czipwcx {
public:
	Czipwcx(void);
	// TODO: add your methods here.
};

extern ZIPWCX_API int nzipwcx;

ZIPWCX_API int fnzipwcx(void);
