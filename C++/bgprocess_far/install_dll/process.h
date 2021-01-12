#ifndef PROCESS_H
#define PROCESS_H

#include <install.h>

INSTALL_DLL_API  DWORD gi_Exec(LPCSTR filename, LPCSTR cmd, LPCSTR workdir, int ShowCmd);
INSTALL_DLL_API  DWORD gi_ShellExec(LPCSTR foldername, LPCSTR cmd, LPCSTR workdir, int ShowCmd);



#endif //PROCESS_H