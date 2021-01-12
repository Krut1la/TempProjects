#ifndef INFODIALOG_H
#define INFODIALOG_H

enum InfoDialogControls {
	IIDialogBox,
	IIListBox,

	IISeparator1,
	IIAll,
	IIShow,
	IIPause,
	IIAbort,
	IILast
};

DWORD OpenInfoDialog();
long WINAPI InfoDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);
DWORD RefreshInfoDialog(HANDLE hDlg);

#endif// INFODIALOG_H