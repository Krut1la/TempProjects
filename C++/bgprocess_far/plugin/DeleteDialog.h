#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

enum DeleteDialogControls {
	IDDialogBox,
	IDString,
	IDEdit,
	IDSeparator1,

	IDCBReadOnly,
	IDCBAbortOnError,
	IDCBRecycle,
	
	IDSeparator2,
	IDOk,
	IDCancel,
	IDLast
};


int OpenDeleteDialog();
long WINAPI DeleteDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);

DWORD MakeDeleteXML(HANDLE hDlg, PanelInfo& PInfo, PanelInfo& AnotherPInfo);



#endif// DELETEDIALOG_H