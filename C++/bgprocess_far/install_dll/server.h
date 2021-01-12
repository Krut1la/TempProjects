#ifndef SERVER_H
#define SERVER_H

#include <install.h>

INSTALL_DLL_API  DWORD gi_DllRegisterServer(LPCSTR filename);
INSTALL_DLL_API  DWORD gi_DllUnregisterServer(LPCSTR filename);


#endif //SERVER_H