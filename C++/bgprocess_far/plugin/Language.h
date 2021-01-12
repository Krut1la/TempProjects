#ifndef LANGUAGE_H
#define LANGUAGE_H

enum LanguageList {
	SNull,
	STitle,

	//Info
	SInfoCaption,
	SInfoMenu,
	SInfoPaused,
	SInfoAborting,
	SInfoFileExist,
	SInfoFileReadOnly,
	SInfoError,

	SProgSCopingFileSh,
	SProgSMovingFileSh,
	SProgSDeletingFileSh,
	SProgSComparingFileSh,
	SProgSExtractingArchiveSh,
	SProgSScaningSh,	

	SInfoBShow,
	SInfoBPause,
	SInfoBAbort,
	SInfoCBAll,

	//Configure
	SConfigureCaption,
	SConfigureMenu,
	
	SConfigureDefaultCopy,
	SConfigureDefaultDelete,
	SConfigureDefaultExtract,
	SConfigureSPlugin,
	SConfigureSService,

	SConfigureBOk,
	SConfigureBApply,
	SConfigureBCancel,

	//Copy
	SCopyCaption,
	SCopyMenu,
	
	SCopyRBOverwrite,
	SCopyRBAppend,
	SCopyRBSkip,
	SCopyRBRefresh,
	SCopyRBAsk,
	SCopyCBMove,
	SCopyCBCopyAccess,
	SCopyCBReadOnly,
	SCopyCBAbortOnError,

	SCopyCBMatch,
	SCopyCBDifference,
	SCopyCBModificationTime,
	SCopyCBCreationTime,
	SCopyCBSize,
	SCopyCBCase,
	SCopyCBContent,

	SCopySCopy,
	SCopySMove,
	SCopySTo,
	SCopySItemsTo,
		
	SCopyBOk,
	SCopyBCancel,

	SMoveCaption,
	SMoveMenu,

	//Delete dialog
	SDeleteCaption,
	SDeleteMenu,

	SDeleteSDelete,
	SDeleteSItems,

	SDeleteCBReadOnly,
	SDeleteCBAbortOnError,
	SDeleteCBRecycle,
	SDeleteBOk,
	SDeleteBCancel,

	SExtractCaption,
	SExtractMenu,

	SExtractRBOverwrite,
	SExtractRBAppend,
	SExtractRBSkip,
	SExtractRBRefresh,
	SExtractRBAsk,
	SExtractCBMove,
	SExtractCBCopyAccess,
	SExtractCBReadOnly,
	SExtractCBAbortOnError,

	SExtractSExtract,
	SExtractSMove,
	SExtractSTo,
	SExtractSItemsTo,
	
	SExtractBOk,
	SExtractBCancel,

	//Progress dialog
	SProgCaption,

	SProgSCopingFile,
	SProgSMovingFile,
	SProgSDeletingFile,
	SProgSComparingFile,
	SProgSExtractingArchive,
	SProgSScaning,
    SProgSTo,
	SProgSWith,

	SProgSTotal,
	SProgSProcessed,
	SProgSSpeed,

	SProgBarIndicator,
	SProgBarBackground,

	SProgBOk,
	SProgBAbort,
	SProgBPause,
	SProgBContinue,
	//----------------

	//Open Archive Dialog
	SOpenArchiveCaption,
	SOpenArchiveFiles,
	//-------------------

	SMSGWarning,
	SMSGError,
	SMSGWCFileExists,
	SMSGWCFileReadOnly,
	SMSGWCError,

	SMSGWCSource,
	SMSGWCDestination,

	SMSGWCBAll,
	SMSGWCBIgnoreAll,
	SMSGWBOverwrite,
	SMSGWBSkip,
	SMSGWBRefresh,
	SMSGWBAppend,
	SMSGWBAbort,
	SMSGWBRetry,
	SMSGWBIgnore,

	SStopService

};

#endif // LANGUAGE_H
