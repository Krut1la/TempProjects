#ifndef OPENARCHIVEDIALOG_H
#define OPENARCHIVEDIALOG_H

enum OpenArchiveDialogControls {
	IOADialogBox,


	IOAOpening,
	IOAArchive,
	IOAFiles,

	IOALast
};


int OpenOpenArchiveDialog(const char* filename, ArchivePanel* archive_panel);
long WINAPI OpenArchiveDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);
DWORD RefreshOpenArchiveInfoDialog(HANDLE hDlg);
DWORD MakeOpenArchiveXML();


#endif// OPENARCHIVEDIALOG_H