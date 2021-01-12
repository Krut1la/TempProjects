#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H


enum ProgressDialogControls {
	IPDialogBox,
	IPSFrom,
	IPCOperationFrom,
	IPSTo,
	IPCOperationTo,
	IPSeparator1,
	IPSProcessed,
	IPSTotal,
	IPSSpeed,
	IPFProgress,
	IPCOProgress,
	IPSeparator2,
	IPOk,
	IPAbort,
	IPPause,
	IPLast
};

DWORD OpenProgressDialog(HANDLE thread);
long WINAPI ProgressDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);
//DWORD GetThreadStatus(struct THREADDESC& thrddsc, int threadId);
//VOID CALLBACK ProgressTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
/*VOID CALLBACK ProgressTimerAPCProc(
  LPVOID lpArgToCompletionRoutine,
  DWORD dwTimerLowValue,
  DWORD dwTimerHighValue
);*/
DWORD RefreshStatus(HANDLE hDlg);


#endif// PROGRESSDIALOG_H