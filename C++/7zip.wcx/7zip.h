////////////////////////////////////////////////////////////////////////////////
// 7zip Plugin for Total Commander
// Copyright (c) 2004 Adam Strzelecki <ono@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

extern "C" {

typedef struct
{
    HFONT hBoldFont;
    char *szPassword;
} DIALOGDATA;

typedef struct
{
    char *name;
    NArchive::N7z::CMethodID id;
    int num;
} COMPMETHOD;

typedef struct
{
    LPWSTR name;
    int id;
    int langID;
} COMPLEVEL;

enum EMethodID
{
  kCopy,
  kLZMA,
  kPPMd,
  kBZip2,
  kDeflate,
  kDeflate64
};

extern HINSTANCE g_hInstance;
extern COMPMETHOD g_compMethods[];

#define FLAG_SHOW_PASSWORD      1
#define FLAG_USE_NEXT           2
#define FLAG_ENCRYPT_HEADER     4
#define FLAG_CREATE_SOLID       8

#define PASSWORD_SIZE           64
typedef struct
{
    int methodID;
    NArchive::N7z::CMethodID method;
    int level;
    UInt32 dict;
    UInt32 word;
    DWORD flags;
    char password[PASSWORD_SIZE + 1];
} COMPSETTINGS;

BOOL ReadSettings(COMPSETTINGS *cs);
BOOL WriteSettings(COMPSETTINGS *cs);
char *GetPassword();

#ifndef __DEBUG__
#define DebugString
#else
static char _debugStr[255];
#define STRING_AUX( x ) #x
#define STRING(x) STRING_AUX(x)
#define DebugString(x,...) snprintf(_debugStr,255,__FILE__ ":" STRING( __LINE__ ) " " x,##__VA_ARGS__),OutputDebugString(_debugStr)
#endif
}
