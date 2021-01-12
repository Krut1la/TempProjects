#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

enum ConfigDialogControls {
	ICGDialogBox,
	ICGSpliter,
	ICGListBox,

	//CopyDialog
	ICGCopyDefault,
	ICGCopyRBOverwrite,
	ICGCopyRBAppend,
	ICGCopyRBSkip,
	ICGCopyRBRefresh,
	ICGCopyRBAsk,
	ICGCopyCBMove,
	ICGCopyCBCopyAccess,
	ICGCopyCBReadOnly,
	ICGCopyCBAbortOnError,

	
	ICGSeparator2,
	ICGOk,
	ICGApply,
	ICGCancel,

	//ICG

	ICGLast
};


int OpenConfigDialog();
long WINAPI ConfigDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);


#endif// CONFIGDIALOG_H