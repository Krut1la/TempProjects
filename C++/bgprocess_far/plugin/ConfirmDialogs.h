#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

enum FileExistsDialogControls {
	IFEDialogBox,
	IFEFileExist,
	IFEFile,
	IFESeparator1,

	IFESource,
	IFEDestination,

	IFESeparator2,

	IFECBAll,

	IFEBOverwrite,
	IFEBSkip,
	IFEBRefresh,
	IFEBAppend,
	IFEBAbort,

	IFELast
};

enum FileReadOnlyDialogControls {
	IFRODialogBox,
	IFROFileReadOnly,
	IFROFile,
	IFROSeparator1,

	IFRODestination,

	IFROSeparator2,

	IFROCBAll,

	IFROBOverwrite,
	IFROBSkip,
	IFROBAbort,

	IFROLast
};

enum ErrorDialogControls {
	IERDialogBox,
	IERError,
	IERFile,
	IERSeparator1,

	IERDestination,

	IERSeparator2,

	IERCBIgnoreAll,

	IERBRetry,
	IERBIgnore,
	IERBAbort,

	IERLast
};


int OpenFileExistsDialog(const wchar_t* source, const wchar_t* destination);
int OpenFileReadOnlyDialog(const wchar_t* destination);
int OpenErrorDialog(const wchar_t* destination, const wchar_t* errortext);
long WINAPI FileExistsDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);
long WINAPI FileReadOnlyDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);
long WINAPI ErrorDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);


#endif// CONFIRMDIALOG_H