#ifndef PATCH_H
#define PATCH_H

#include <install.h>

#define maxint 0x7fffffff
#define ESC     0xA7
#define MOD     0xA6
#define INS     0xA5
#define DEL     0xA4
#define EQL     0xA3
#define BKT     0xA2

INSTALL_DLL_API  DWORD gi_PatchFile(LPCSTR filename, LPCSTR patchfilename, LPCSTR newname);


#endif //PATCH_H