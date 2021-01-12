#include "Common.h"
#include "ConfigDialog.h"


extern struct PluginStartupInfo Info;

const int DlgW = 70;
const int DlgH = 25;

int OpenConfigDialog()
{	
	struct FarDialogItem DialogItems[ICGLast];

	static const struct InitDialogItem InitItems [] = {
			DI_DOUBLEBOX,	3,				1,			DlgW-4,		DlgH-2,		0,	SConfigureCaption,
			DI_SINGLEBOX,	DlgW/4,			2,			1,			DlgH-4,		0,	SNull,
			DI_LISTBOX,		4,				2,			DlgW/4 - 1,	DlgH-5,		DIF_LISTNOBOX|DIF_LISTWRAPMODE,SNull,

			//CopyDialog
			DI_TEXT,		DlgW/4 + 1,				2,			0,			0,			0,SConfigureDefaultCopy,
			DI_RADIOBUTTON,	DlgW/4 + 1,				4,			0,			0,			DIF_GROUP,SCopyRBOverwrite,
			DI_RADIOBUTTON,	DlgW/4 + 1,				5,			0,			0,			0,SCopyRBAppend,
			DI_RADIOBUTTON,	DlgW/4 + 1,				6,			0,			0,			0,SCopyRBSkip,
			DI_RADIOBUTTON,	DlgW/4 + 1,				7,			0,			0,			0,SCopyRBRefresh,
			DI_RADIOBUTTON,	DlgW/4 + 1,				8,			0,			0,			0,SCopyRBAsk,
			DI_CHECKBOX,	DlgW/4 + 1,				10,			0,			0,			0,SCopyCBMove,
			DI_CHECKBOX,	DlgW/4 + 1,				11,			0,			0,			0,SCopyCBCopyAccess,
			DI_CHECKBOX,	DlgW/4 + 1,				12,			0,			0,			0,SCopyCBReadOnly,
			DI_CHECKBOX,	DlgW/4 + 1,				13,			0,			0,			0,SCopyCBAbortOnError,

			DI_TEXT,		0,				DlgH-4,		DlgW-1,		0,			DIF_BOXCOLOR|DIF_SEPARATOR,SNull,
			DI_BUTTON,		DlgW/2 - 17,	DlgH-3,		0,			0,			0, SConfigureBOk,
			DI_BUTTON,		DlgW/2 - 8,		DlgH-3,		0,			0,			0, SConfigureBApply,
			DI_BUTTON,		DlgW/2 + 4,		DlgH-3,		0,			0,			0, SConfigureBCancel
	};

	InitDialogItems(InitItems, DialogItems, sizeof(InitItems) / sizeof(InitItems[0]));


	return Info.DialogEx(Info.ModuleNumber, -1, -1, DlgW, DlgH,
		PHRASE_COLORS, DialogItems,
		sizeof(DialogItems) / sizeof(DialogItems[0]),
		0, FDLG_NODRAWSHADOW, ConfigDlgProc,0);
}

long WINAPI ConfigDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2)
{
	static int PrevFocused;
	static int bLBoxPosChanged;

  
//  MOUSE_EVENT_RECORD m_record;
	long FocusedControl;

  // Запомним активный элемент диалога
  FocusedControl = Info.SendDlgMessage(hDlg, DM_GETFOCUS, 0, 0);

  switch (Msg) {
	case DN_BTNCLICK:
		switch(Param1)
		{
		case ICGListBox:
			break;
		}
		break;
    case DN_CTLCOLORDLGITEM:
      break;

    case DN_INITDIALOG:
		{
		FarList fl;
		FarListItem fli[5] = {0};
		lstrcpy(fli[0].Text, GetMsg(SConfigureSPlugin));
		lstrcpy(fli[1].Text, GetMsg(SConfigureSService));
		lstrcpy(fli[2].Text, GetMsg(SCopyCaption));
		lstrcpy(fli[3].Text, GetMsg(SDeleteCaption));
		lstrcpy(fli[4].Text, GetMsg(SExtractCaption));

		fl.ItemsNumber = 5;
		fl.Items = &fli[0];

		Info.SendDlgMessage(hDlg, DM_LISTADD, ICGListBox, (long)&fl);
		}
      return FALSE;

    case DN_KILLFOCUS:
      break;
    
    case DN_DRAWDLGITEM:
    break;
  
    case DN_CLOSE:
      // Закрываем если нажат Cancel или ESC
      //return(Param1 < 0 || Param1 == ICancel);
		break;
  }

  // Передадим управление стандартному обработчику
  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

void SelectPage(int pageId)
{

}