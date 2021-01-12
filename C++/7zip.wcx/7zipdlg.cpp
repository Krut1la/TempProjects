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

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>

#include <initguid.h>
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Winnetwk.h>

#include "7zip/Archive/7z/7zMethodID.h"
#include "7zip.h"

#include "resource.h"

extern "C" {

////////////////////////////////////////////////////////////////////////////////////////////
// Variables

#ifdef COMPRESS_LZMA
static NArchive::N7z::CMethodID k_LZMA = { { 0x3, 0x1, 0x1 }, 3 };
#endif

#ifdef COMPRESS_PPMD
static NArchive::N7z::CMethodID k_PPMd = { { 0x3, 0x4, 0x1 }, 3 };
#endif

#ifdef COMPRESS_DEFLATE
static NArchive::N7z::CMethodID k_Deflate = { { 0x4, 0x1, 0x8 }, 3 };
#endif

#ifdef COMPRESS_BZIP2
static NArchive::N7z::CMethodID k_BZip2 = { { 0x4, 0x2, 0x2 }, 3 };
#endif

#ifdef COMPRESS_COPY
static NArchive::N7z::CMethodID k_Copy = { { 0x0 }, 1 };
#endif

COMPMETHOD g_compMethods[] = {
#ifdef COMPRESS_LZMA
    {"LZMA", k_LZMA, kLZMA},
#endif
#ifdef COMPRESS_PPMD
    {"PPMd", k_PPMd, kPPMd},
#endif
#ifdef COMPRESS_BZIP2
    {"BZip2", k_BZip2, kBZip2},
#endif
#ifdef COMPRESS_DEFLATE
    {"Deflate", k_Deflate, kDeflate},
#endif
    {NULL, k_Copy, kCopy}
};

COMPLEVEL g_compLevels[] = {
    {L"Store", 0x02000D81},
    {L"Fastest", 0x02000D85},
    { 0, 0 },
    {L"Fast", 0x02000D84},
    { 0, 0 },
    {L"Normal", 0x02000D82},
    { 0, 0 },
    {L"Maximum", 0x02000D83},
    { 0, 0 },
    {L"Ultra", 0x02000D86},
    { 0, 0 },{ 0, 0 }
};

////////////////////////////////////////////////////////////////////////////////
// Language settings

// Default language (group)
char szLang[64] = "lang";
UINT cp = CP_ACP;

char *BIN(LPCTSTR fileName)
{
	static char szModule1[_MAX_PATH];
	static char szModule2[_MAX_PATH];
	char* pszFileName;

	::GetModuleFileName(g_hInstance, szModule1, _MAX_PATH);
	::GetFullPathName(szModule1, _MAX_PATH, szModule2, &pszFileName);
	strcpy(pszFileName, fileName);
	return szModule2;
}

char *TranslateDefA(int id, LPWSTR wDef)
{
	static char szTrans[255], szKey[32];
    static WCHAR wTrans[255];
    ltoa(id, szKey, 10);
    ZeroMemory(wTrans, sizeof(wTrans));
    if(GetPrivateProfileString(szLang, szKey, NULL, szTrans,
        sizeof(szTrans), BIN("7zip.lng")))
        return szTrans;
    if(wDef)
    {
        WideCharToMultiByte(cp, 0, wDef, wcslen(wDef), szTrans, sizeof(szTrans), NULL, NULL);
        return szTrans;
    }
    return NULL;
}

LPWSTR TranslateDefW(int id, LPWSTR wDef)
{
	static char szTrans[255], szKey[32];
    static WCHAR wTrans[255];
    ltoa(id, szKey, 10);
    ZeroMemory(wTrans, sizeof(wTrans));
    if(GetPrivateProfileString(szLang, szKey, NULL, szTrans,
        sizeof(szTrans), BIN("7zip.lng"))
        && MultiByteToWideChar(cp, 0, szTrans, strlen(szTrans), wTrans, sizeof(wTrans)))
        return wTrans;
    return wDef;
}

inline char *TranslateA(int id)
{
    DebugString("Translate(%d) %s", id, TranslateDefA(id, NULL));
    return TranslateDefA(id, NULL);
}

inline LPWSTR TranslateW(int id)
{
    DebugString("Translate(%d) %s", id, TranslateDefA(id, NULL));
    return TranslateDefW(id, NULL);
}

static BOOL CALLBACK TranslateDialogEnumProc(HWND hwnd, LPARAM lParam)
{
    int id = lParam;
    LPWSTR wTrans; char *szTrans;
    if(IsWindowUnicode(hwnd) && (wTrans = TranslateW(GetDlgCtrlID(hwnd) + id)))
        SetWindowTextW(hwnd, wTrans);
    else if(szTrans = TranslateA(GetDlgCtrlID(hwnd) + id))
        SetWindowTextA(hwnd, szTrans);
	return TRUE;
}

BOOL TranslateDialog(HWND hwndDlg, int id)
{
    cp = GetPrivateProfileInt(szLang, "codepage", CP_ACP, BIN("7zip.lng"));
    LPWSTR wTrans; char *szTrans;
    if(IsWindowUnicode(hwndDlg) && (wTrans = TranslateW(id)))
        SetWindowTextW(hwndDlg, wTrans);
    else if(szTrans = TranslateA(id))
        SetWindowTextA(hwndDlg, szTrans);
    EnumChildWindows(hwndDlg, TranslateDialogEnumProc, (LPARAM)id);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Misc functions

int AddDictionarySizeL(HWND hwndDlg, UInt32 size, bool kilo, bool maga)
{
  UInt32 sizePrint = size;
  if (kilo)
    sizePrint >>= 10;
  else if (maga)
    sizePrint >>= 20;
  char s[40];
  ltoa(sizePrint, s, 10);
  if (kilo)
    lstrcat(s, TEXT(" K"));
  else if (maga)
    lstrcat(s, TEXT(" M"));
  else
    lstrcat(s, TEXT(" "));
  lstrcat(s, TEXT("B"));
  int index = SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_ADDSTRING, 0, (LPARAM)s);
  SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_SETITEMDATA, (WPARAM)index, (LPARAM)size);
  return index;
}
int AddDictionarySize(HWND hwndDlg, UInt32 size)
{
  if (size > 0)
  {
    if ((size & 0xFFFFF) == 0)
      return AddDictionarySizeL(hwndDlg, size, false, true);
    if ((size & 0x3FF) == 0)
      return AddDictionarySizeL(hwndDlg, size, true, false);
  }
  return AddDictionarySizeL(hwndDlg, size, false, false);
}
int AddOrder(HWND hwndDlg, UInt32 size)
{
  char s[40];
  ltoa(size, s, 10);
  int index = SendDlgItemMessage(hwndDlg, IDC_WORD_SIZE, CB_ADDSTRING, 0, (LPARAM)s);
  SendDlgItemMessage(hwndDlg, IDC_WORD_SIZE, CB_SETITEMDATA, (WPARAM)index, (LPARAM)size);
  return index;
}
int GetMethodID(HWND hwndDlg)
{
    int index = SendDlgItemMessage(hwndDlg, IDC_COMP_METHOD, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if(index == -1) return g_compMethods[0].num;

    return g_compMethods[index].num;
}
int GetLevel2(HWND hwndDlg)
{
    int index = SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if(index == -1) return 5;
    return SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETITEMDATA, (WPARAM)index, (LPARAM)0);
}
void SetNearestSelectComboBox(HWND hwndDlg, int id, UInt32 value)
{
  for (int i = SendDlgItemMessage(hwndDlg, id, CB_GETCOUNT, (WPARAM)0, (LPARAM)0) - 1; i >= 0; i--)
    if ((UInt32)SendDlgItemMessage(hwndDlg, id, CB_GETITEMDATA, (WPARAM)i, (LPARAM)0) <= value)
    {
      SendDlgItemMessage(hwndDlg, id, CB_SETCURSEL, (WPARAM)i, (LPARAM)0);
      return;
    }
  if (SendDlgItemMessage(hwndDlg, id, CB_GETCOUNT, (WPARAM)0, (LPARAM)0) > 0)
      SendDlgItemMessage(hwndDlg, id, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}

void UpdateCombos(HWND hwndDlg)
{
  SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  SendDlgItemMessage(hwndDlg, IDC_WORD_SIZE, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
  int methodID = GetMethodID(hwndDlg);
  int level = GetLevel2(hwndDlg);
  UInt32 defaultDictionary = UInt32(-1);
  UInt32 defaultOrder = UInt32(-1);
  // Dictionary size
  switch (methodID)
  {
    case kLZMA:
    {
      if (defaultDictionary == UInt32(-1))
      {
        if (level >= 9)
          defaultDictionary = (32 << 20);
        else if (level >= 7)
          defaultDictionary = (8 << 20);
        else if (level >= 5)
          defaultDictionary = (2 << 20);
        else
          defaultDictionary = (32 << 10);
      }
      int i;
      AddDictionarySize(hwndDlg, 32 << 10);
      for (i = 20; i < 28; i++)
        for (int j = 0; j < 2; j++)
        {
          if (i == 20 && j > 0)
            continue;
          UInt32 dictionary = (1 << i) + (j << (i - 1));
          AddDictionarySize(hwndDlg, dictionary);
        }
      SetNearestSelectComboBox(hwndDlg, IDC_DICT_SIZE, defaultDictionary);
      break;
    }
    case kPPMd:
    {
      if (defaultDictionary == UInt32(-1))
      {
        if (level >= 9)
          defaultDictionary = (192 << 20);
        else if (level >= 7)
          defaultDictionary = (64 << 20);
        else if (level >= 5)
          defaultDictionary = (24 << 20);
        else
          defaultDictionary = (4 << 20);
      }
      int i;
      for (i = 20; i < 31; i++)
        for (int j = 0; j < 2; j++)
        {
          if (i == 20 && j > 0)
            continue;
          UInt32 dictionary = (1 << i) + (j << (i - 1));
          if (dictionary >= (1 << 31))
            continue;
          AddDictionarySize(hwndDlg, dictionary);
        }
      SetNearestSelectComboBox(hwndDlg, IDC_DICT_SIZE, defaultDictionary);
      break;
    }
    case kDeflate:
    {
      AddDictionarySize(hwndDlg, 32 << 10);
      SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
      break;
    }
    case kDeflate64:
    {
      AddDictionarySize(hwndDlg, 64 << 10);
      SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
      break;
    }
    case kBZip2:
    {
      AddDictionarySize(hwndDlg, 900 << 10);
      SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
      break;
    }
  }
  // Word size
  switch (methodID)
  {
    case kLZMA:
    {
      if (defaultOrder == UInt32(-1))
      {
        if (level >= 7)
          defaultOrder = 64;
        else
          defaultOrder = 32;
      }
      int i;
      for (i = 3; i < 8; i++)
        for (int j = 0; j < 2; j++)
        {
          UInt32 order = (1 << i) + (j << (i - 1));
          if (order < 255)
            AddOrder(hwndDlg, order);
        }
      AddOrder(hwndDlg, 255);
      SetNearestSelectComboBox(hwndDlg, IDC_WORD_SIZE, defaultOrder);
      break;
    }
    case kPPMd:
    {
      if (defaultOrder == UInt32(-1))
      {
        if (level >= 9)
          defaultOrder = 32;
        else if (level >= 7)
          defaultOrder = 16;
        else if (level >= 5)
          defaultOrder = 6;
        else
          defaultOrder = 4;
      }
      int i;
      AddOrder(hwndDlg, 2);
      AddOrder(hwndDlg, 3);
      for (i = 2; i < 8; i++)
        for (int j = 0; j < 4; j++)
        {
          UInt32 order = (1 << i) + (j << (i - 2));
          if (order < 32)
            AddOrder(hwndDlg, order);
        }
      AddOrder(hwndDlg, 32);
      SetNearestSelectComboBox(hwndDlg, IDC_WORD_SIZE, defaultOrder);
      break;
    }
    case kDeflate:
    case kDeflate64:
    {
      if (defaultOrder == UInt32(-1))
      {
        if (level >= 7)
          defaultOrder = 64;
        else
          defaultOrder = 32;
      }
      int i;
      for (i = 3; i < 8; i++)
        for (int j = 0; j < 2; j++)
        {
          UInt32 order = (1 << i) + (j << (i - 1));
          if (order < 255)
            AddOrder(hwndDlg, order);
        }
      AddOrder(hwndDlg, 255);
      SetNearestSelectComboBox(hwndDlg, IDC_WORD_SIZE, defaultOrder);
      break;
    }
    case kBZip2:
    {
      AddOrder(hwndDlg, 1);
      SetNearestSelectComboBox(hwndDlg, IDC_WORD_SIZE, defaultOrder);
      break;
    }
  }
 //SetMemoryUsage();
}
void UpdatePassword(HWND hwndDlg)
{
    SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_SETPASSWORDCHAR, (WPARAM)(IsDlgButtonChecked(hwndDlg, IDC_SHOW_PASSWORD) ? 0 : '*'), (LPARAM)0);
    InvalidateRect(GetDlgItem(hwndDlg, IDC_PASSWORD), NULL, TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Config dialog

BOOL CALLBACK ConfigDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DIALOGDATA *dat;
    dat = (DIALOGDATA *)GetWindowLong(hwndDlg, GWL_USERDATA);

    switch (msg) {
        case WM_INITDIALOG:
        {
            // dat = (GGUSERUTILDLGDATA  *)lParam;
            TranslateDialog(hwndDlg, IDD_CONFIG);
			if(dat = (DIALOGDATA *)malloc(sizeof(DIALOGDATA)))
			{
                SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)dat);
				// Make bold title font
				LOGFONT lf;
				HFONT hNormalFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_GETFONT, 0, 0);
				GetObject(hNormalFont, sizeof(lf), &lf);
				lf.lfWeight = FW_BOLD;
				dat->hBoldFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_SETFONT, (WPARAM)dat->hBoldFont, 0);
			}
            COMPSETTINGS cs;
            ReadSettings(&cs);
            // Fill in the compression list
            for(int i = 0; g_compMethods[i].name; i ++)
            {
                int index = SendDlgItemMessage(hwndDlg, IDC_COMP_METHOD, CB_ADDSTRING, 0, (LPARAM)g_compMethods[i].name);
                if(g_compMethods[i].num == cs.methodID) SendDlgItemMessage(hwndDlg, IDC_COMP_METHOD, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
            }

            for(int i = 0; g_compLevels[i].id || g_compLevels[i + 1].id; i ++)
                if(g_compLevels[i].name)
                {
                    int count = SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
                    int index = SendDlgItemMessageW(hwndDlg, IDC_COMP_LEVEL, CB_ADDSTRING, 0, (LPARAM)TranslateDefW(IDS_COMP_LEVEL + i, g_compLevels[i].name));
                    if(count == SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETCOUNT, (WPARAM)0, (LPARAM)0))
                        index = SendDlgItemMessageA(hwndDlg, IDC_COMP_LEVEL, CB_ADDSTRING, 0, (LPARAM)TranslateDefA(IDS_COMP_LEVEL + i, g_compLevels[i].name));
                    SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
                    if(i == cs.level) SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_SETCURSEL, (WPARAM)index, (LPARAM)0);
                }

            UpdateCombos(hwndDlg);
            SetNearestSelectComboBox(hwndDlg, IDC_DICT_SIZE, cs.dict);
            SetNearestSelectComboBox(hwndDlg, IDC_WORD_SIZE, cs.word);

            CheckDlgButton(hwndDlg, IDC_SHOW_PASSWORD, cs.flags & FLAG_SHOW_PASSWORD);
            CheckDlgButton(hwndDlg, IDC_USE_NEXT, cs.flags & FLAG_USE_NEXT);
            CheckDlgButton(hwndDlg, IDC_ENCRYPT_HEADER, cs.flags & FLAG_ENCRYPT_HEADER);
            CheckDlgButton(hwndDlg, IDC_CREATE_SOLID, cs.flags & FLAG_CREATE_SOLID);

            SetDlgItemText(hwndDlg, IDC_PASSWORD, cs.password);
            SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_SETLIMITTEXT, (WPARAM)(sizeof(cs.password) - 1), (LPARAM)0);
            UpdatePassword(hwndDlg);

            {
                BOOL bEnable = (SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) != 0);
                EnableWindow(GetDlgItem(hwndDlg, IDC_COMP_METHOD), bEnable);
                EnableWindow(GetDlgItem(hwndDlg, IDC_DICT_SIZE), bEnable);
                EnableWindow(GetDlgItem(hwndDlg, IDC_WORD_SIZE), bEnable);
            }

            return TRUE;
            break;
        }
		case WM_CTLCOLORSTATIC:
            if((GetDlgItem(hwndDlg, IDC_TITLE) == (HWND)lParam)
				|| (GetDlgItem(hwndDlg, IDC_SUBTITLE) == (HWND)lParam)
                || (GetDlgItem(hwndDlg, IDI_ICON) == (HWND)lParam))
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(NULL_BRUSH);
			}
			break;
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDC_COMP_LEVEL:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        BOOL bEnable = (SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETCURSEL, (WPARAM)0, (LPARAM)0) != 0);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_COMP_METHOD), bEnable);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_DICT_SIZE), bEnable);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_WORD_SIZE), bEnable);
                    }
                case IDC_COMP_METHOD:
                    if(HIWORD(wParam) == CBN_SELCHANGE)
                        UpdateCombos(hwndDlg);

                    break;
                case IDC_SHOW_PASSWORD:
                    UpdatePassword(hwndDlg);
                    break;
                case IDOK:
                {
                    COMPSETTINGS cs;
                    ZeroMemory(&cs, sizeof(COMPSETTINGS));
                    int index;
                    // Method
                    index = SendDlgItemMessage(hwndDlg, IDC_COMP_METHOD, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    cs.methodID = g_compMethods[index].num;
                    // Level
                    index = SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    cs.level = SendDlgItemMessage(hwndDlg, IDC_COMP_LEVEL, CB_GETITEMDATA, (WPARAM)index, (LPARAM)0);
                    index = SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    cs.dict = SendDlgItemMessage(hwndDlg, IDC_DICT_SIZE, CB_GETITEMDATA, (WPARAM)index, (LPARAM)0);
                    index = SendDlgItemMessage(hwndDlg, IDC_WORD_SIZE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
                    cs.word = SendDlgItemMessage(hwndDlg, IDC_WORD_SIZE, CB_GETITEMDATA, (WPARAM)index, (LPARAM)0);
                    // Flags
                    cs.flags = (IsDlgButtonChecked(hwndDlg, IDC_SHOW_PASSWORD) ? FLAG_SHOW_PASSWORD : 0)
                             | (IsDlgButtonChecked(hwndDlg, IDC_USE_NEXT) ? FLAG_USE_NEXT : 0)
                             | (IsDlgButtonChecked(hwndDlg, IDC_ENCRYPT_HEADER) ? FLAG_ENCRYPT_HEADER : 0)
                             | (IsDlgButtonChecked(hwndDlg, IDC_CREATE_SOLID) ? FLAG_CREATE_SOLID : 0);
                    GetDlgItemText(hwndDlg, IDC_PASSWORD, cs.password, sizeof(cs.password));

                    WriteSettings(&cs);
                    EndDialog(hwndDlg, IDOK);
                    break;
                }
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    break;
            }
            break;
        }
        case WM_NOTIFY:
        {
            break;
        }
        case WM_DESTROY:
            if(dat)
            {
				if(dat->hBoldFont) DeleteObject(dat->hBoldFont);
                free(dat);
            }
            break;
    }
    return FALSE;
}

BOOL WriteSettings(COMPSETTINGS *cs)
{
#ifdef USE_REGISTRY
    HKEY hKey = NULL; DWORD dwDisp = 0;
    if(RegCreateKeyEx(
        HKEY_CURRENT_USER,
        "SOFTWARE\\Adam Strzelecki\\7Zip Plugin",
        0,
        NULL,
        0,
        KEY_ALL_ACCESS,
        NULL,
        &hKey,
        &dwDisp) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, "CompMethod", 0, REG_DWORD, (LPBYTE)&cs->methodID, sizeof(DWORD));
        RegSetValueEx(hKey, "CompLevel", 0, REG_DWORD, (LPBYTE)&cs->level, sizeof(DWORD));
        RegSetValueEx(hKey, "DictSize", 0, REG_DWORD, (LPBYTE)&cs->dict, sizeof(DWORD));
        RegSetValueEx(hKey, "WordSize", 0, REG_DWORD, (LPBYTE)&cs->word, sizeof(DWORD));
        RegSetValueEx(hKey, "Flags", 0, REG_DWORD, (LPBYTE)&cs->flags, sizeof(DWORD));
        RegSetValueEx(hKey, "Password", 0, REG_SZ, (LPBYTE)&cs->password, sizeof(cs->password));
        return TRUE;
    }
    return FALSE;
#else
    char *szFile = BIN("7zip.ini"), szVal[64];

    ltoa(cs->methodID, szVal, 10); WritePrivateProfileString("settings", "CompMethod", szVal, szFile);
    ltoa(cs->level, szVal, 10); WritePrivateProfileString("settings", "CompLevel", szVal, szFile);
    ltoa(cs->dict, szVal, 10); WritePrivateProfileString("settings", "DictSize", szVal, szFile);
    ltoa(cs->word, szVal, 10); WritePrivateProfileString("settings", "WordSize", szVal, szFile);
    ltoa(cs->flags, szVal, 10); WritePrivateProfileString("settings", "Flags", szVal, szFile);
    WritePrivateProfileString("settings", "Password", cs->password, szFile);
    return TRUE;
#endif
}

BOOL ReadSettings(COMPSETTINGS *cs)
{
#ifdef USE_REGISTRY
    HKEY hKey = NULL;
    // Default settings for LZMA / Normal
    cs->methodID = kLZMA;
    cs->method = k_LZMA;
    cs->level = 5;
    cs->dict = (2 << 20);
    cs->word = 32;
    strcpy(cs->password, "");
    cs->flags = FLAG_CREATE_SOLID;

    // Open registry key
    if(RegOpenKeyEx(
        HKEY_CURRENT_USER,
        "SOFTWARE\\Adam Strzelecki\\7Zip Plugin",
        0,
        KEY_READ,
        &hKey) == ERROR_SUCCESS)
    {
        COMPSETTINGS lcs; DWORD cbData = sizeof(DWORD); DWORD dwType;
        if(RegQueryValueEx(hKey, "CompMethod", NULL, &dwType, (LPBYTE)&lcs.methodID, &cbData) != ERROR_SUCCESS || dwType != REG_DWORD) return FALSE;
        for(int i = 0; g_compMethods[i].name; i++) if(g_compMethods[i].num == lcs.methodID) lcs.method = g_compMethods[i].id;
        if(RegQueryValueEx(hKey, "CompLevel", NULL, &dwType, (LPBYTE)&lcs.level, &cbData) != ERROR_SUCCESS || dwType != REG_DWORD) return FALSE;
        if(RegQueryValueEx(hKey, "DictSize", NULL, &dwType, (LPBYTE)&lcs.dict, &cbData) != ERROR_SUCCESS || dwType != REG_DWORD) return FALSE;
        if(RegQueryValueEx(hKey, "WordSize", NULL, &dwType, (LPBYTE)&lcs.word, &cbData) != ERROR_SUCCESS || dwType != REG_DWORD) return FALSE;
        if(RegQueryValueEx(hKey, "Flags", NULL, &dwType, (LPBYTE)&lcs.flags, &cbData) != ERROR_SUCCESS || dwType != REG_DWORD) return FALSE;
        cbData = sizeof(lcs.password);
        if(RegQueryValueEx(hKey, "Password", NULL, &dwType, (LPBYTE)&lcs.password, &cbData) != ERROR_SUCCESS || dwType != REG_SZ) return FALSE;
        memcpy(cs, &lcs, sizeof(COMPSETTINGS));
        if(!(cs->flags & FLAG_USE_NEXT))
        {
            COMPSETTINGS lcs;
            memcpy(&lcs, cs, sizeof(COMPSETTINGS));
            strcpy(lcs.password, "");
            WriteSettings(&lcs);
        }
        return TRUE;
    }
    return FALSE;
#else
    char *szFile = BIN("7zip.ini");
    cs->methodID = GetPrivateProfileInt("settings", "CompMethod", kLZMA, szFile);
    for(int i = 0; g_compMethods[i].name; i++) if(g_compMethods[i].num == cs->methodID) cs->method = g_compMethods[i].id;
    cs->level = GetPrivateProfileInt("settings", "CompLevel", 5, szFile);
    cs->dict = GetPrivateProfileInt("settings", "DictSize", (2 << 20), szFile);
    cs->word = GetPrivateProfileInt("settings", "WordSize", 32, szFile);
    cs->flags = GetPrivateProfileInt("settings", "Flags", FLAG_CREATE_SOLID, szFile);
    GetPrivateProfileString("settings", "Password", "", cs->password, 64, szFile);
    if(!(cs->flags & FLAG_USE_NEXT))
        WritePrivateProfileString("settings", "Password", "", szFile);
    return TRUE;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////
// Proc: Password

BOOL CALLBACK PasswordDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DIALOGDATA *dat;
    dat = (DIALOGDATA *)GetWindowLong(hwndDlg, GWL_USERDATA);

    switch (msg) {
        case WM_INITDIALOG:
        {
            char *szPasswordBuffer = (char *)lParam;
            TranslateDialog(hwndDlg, IDD_PASSWORD);
			if(dat = (DIALOGDATA *)malloc(sizeof(DIALOGDATA)))
			{
                SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)dat);
				// Make bold title font
				LOGFONT lf;
				HFONT hNormalFont = (HFONT)SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_GETFONT, 0, 0);
				GetObject(hNormalFont, sizeof(lf), &lf);
				lf.lfWeight = FW_BOLD;
				dat->hBoldFont = CreateFontIndirect(&lf);
				SendDlgItemMessage(hwndDlg, IDC_TITLE, WM_SETFONT, (WPARAM)dat->hBoldFont, 0);
                // Store password buffer pointer
                dat->szPassword = szPasswordBuffer;
			}

            SendDlgItemMessage(hwndDlg, IDC_PASSWORD, EM_SETLIMITTEXT, (WPARAM)PASSWORD_SIZE, (LPARAM)0);
            //UpdatePassword(hwndDlg);

            return TRUE;
            break;
        }
		case WM_CTLCOLORSTATIC:
            if((GetDlgItem(hwndDlg, IDC_TITLE) == (HWND)lParam)
				|| (GetDlgItem(hwndDlg, IDC_SUBTITLE) == (HWND)lParam)
                || (GetDlgItem(hwndDlg, IDI_ICON) == (HWND)lParam))
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(NULL_BRUSH);
			}
			break;
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                    if(dat) GetDlgItemText(hwndDlg, IDC_PASSWORD, dat->szPassword, PASSWORD_SIZE);
                    EndDialog(hwndDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    break;
            }
            break;
        }
        case WM_NOTIFY:
        {
            break;
        }
        case WM_DESTROY:
            if(dat)
            {
				if(dat->hBoldFont) DeleteObject(dat->hBoldFont);
                free(dat);
            }
            break;
    }
    return FALSE;
}

char *GetPassword()
{
    static char szPassword[PASSWORD_SIZE + 1] = "\0";

    if(DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PASSWORD), NULL, PasswordDialog, (LPARAM)szPassword) == IDOK)
        return szPassword;
    else
        return NULL;
}

}
