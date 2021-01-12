//  install.dll
//  Создание инсталляций на основе RAR-архивов с поддержкой скриптов
//
//  (c) 2001, Виктор Фонта
//  http://www.fonta.narod.ru
//  fonta@ukr.net

#define IS_NOT_OPENED           0
#define IS_OPENED               1
#define IS_CREATING_FOLDER      2
#define IS_INSTALLING_FILE      3
#define IS_WAITING_DISK         4
#define IS_EDITING_REGISTRY     5
#define IS_CREATING_LINKS       6
#define IS_ROLLING_BACK         7
#define IS_FAILED               8
#define IS_COMPLETE             9
#define IS_ERRORR               10

#define IR_OK                   0
#define IR_ERR_OPEN_SCRIPT      100
#define IR_ERR_CLOSE_SCRIPT     101
#define IR_ERR_SCRIPT_SYNTAX    102
#define IR_ERR_CREATE_DIR       103
#define IR_ERR_CREATE_LOG       104
#define IR_ERR_EDIT_REGISTY     105
#define IR_ERR_ALREADY_STARTED  106
#define IR_ERR_NOT_OPENED       107
#define IR_ERR_NOT_STARTED      108
#define IR_ERR_CANCELED         109

#define IR_ERAR_END_ARCHIVE     10
#define IR_ERAR_NO_MEMORY       11
#define IR_ERAR_BAD_DATA        12
#define IR_ERAR_BAD_ARCHIVE     13
#define IR_ERAR_UNKNOWN_FORMAT  14
#define IR_ERAR_EOPEN           15
#define IR_ERAR_ECREATE         16
#define IR_ERAR_ECLOSE          17
#define IR_ERAR_EREAD           18
#define IR_ERAR_EWRITE          19
#define IR_ERAR_SMALL_BUF       20




//HANDLE __stdcall OpenInstallation(PCHAR Script);
//long int __stdcall StartInstallation(HANDLE HInst);
//long int __stdcall CancelInstallation(HANDLE HInst);
//long int __stdcall CloseInstallation(HANDLE HInst);
//void __stdcall SetInstDestinationPath(HANDLE HInst, PCHAR DestinationPath);
//PCHAR __stdcall GetInstDestinationPath(HANDLE HInst);
//PCHAR __stdcall GetInstScriptFile(HANDLE HInst);
//PCHAR __stdcall GetInstTitle(HANDLE HInst);
//long int __stdcall GetInstState(HANDLE HInst);
//long int __stdcall GetInstPercentDone(HANDLE HInst);
//long int __stdcall GetInstLastError(HANDLE HInst);
//PCHAR __stdcall GetInstComment(HANDLE HInst);

