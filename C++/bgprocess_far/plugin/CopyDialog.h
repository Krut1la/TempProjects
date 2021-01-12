#ifndef COPYDIALOG_H
#define COPYDIALOG_H

enum CopyDialogControls {
	ICDialogBox,
	ICString,
	ICEdit,
	ICSeparator1,

	ICRBOverwrite,
	ICRBAppend,
	ICRBSkip,
	ICRBRefresh,
	ICRBAsk,
	ICCBMove,
	ICCBCopyAccess,
	ICCBReadOnly,
	ICCBAbortOnError,

	ICCBCopyMatch,
	ICCBCopyDiff,

	ICCBModificationTime,
	ICCBCreationTime,
	ICCBSize,
	ICCBCase,
	ICCBContent,

	ICSeparator2,
	ICOk,
	ICCancel,
	ICLast
};


int OpenCopyDialog(bool move = false);
long WINAPI CopyDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);

DWORD MakeCopyXML(HANDLE hDlg, PanelInfo& PInfo, PanelInfo& AnotherPInfo);



#endif// COPYDIALOG_H