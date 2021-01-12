#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <install.h>

INSTALL_DLL_API  DWORD gi_CreateShortcut(LPCSTR target, LPCSTR cmd, LPCSTR workdir, LPCSTR iconpath, DWORD iconid, LPCSTR location);


#endif //SHORTCUT_H