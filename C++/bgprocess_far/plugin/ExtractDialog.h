#ifndef EXTRACTDIALOG_H
#define EXTRACTDIALOG_H

enum ExtractDialogControls {
	IEDialogBox,
	IEExtractString,
	IEExtractEdit,
	IESeparator1,

	IERBOverwrite,
	IERBAppend,
	IERBSkip,
	IERBRefresh,
	IERBAsk,
	IECBMove,
	IECBCopyAccess,
	IECBReadOnly,
	IECBAbortOnError,

	IESeparator2,
	IEOk,
	IECancel,
	IELast
};


int OpenExtractDialog();
long WINAPI ExtractDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);

DWORD MakeExtractXML(PanelInfo& PInfo, PanelInfo& AnotherPInfo, const std::wstring& files, const std::wstring& archive_name);



#endif// EXTRACTDIALOG_H