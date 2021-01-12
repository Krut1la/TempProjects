#ifndef INIFILE_H
#define INIFILE_H

#include <install.h>

INSTALL_DLL_API  DWORD gi_WriteINI(LPCSTR section, LPCSTR valuename, LPCSTR data, LPCSTR filename);

#endif //INIFILE_H