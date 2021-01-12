#ifndef ADAPTED_H
#define ADAPTED_H

namespace install{

	enum CallBackMSG
	{
		msgFatal = 10,
		msgError,
		msgWarning,
		msgCurrentOperationProgess,
		msgMessage,
		msgResult,
		msgRequest
	};

	enum Request
	{
		reqFileExist = 100,
		reqFileReadOnly,
		reqUnknownError
	};

	enum Answer
	{
		ansRetry = 1000,
		ansDelete,
		ansMove,
		ansAbort,
		ansSkip,
		ansOverwrite,
		ansRefresh,
		ansAppend
	};

	typedef DWORD (CALLBACK *INSTALLCALLBACK)(CallBackMSG msg,TCHAR* str,LONG P1,LONG P2, LPVOID userdata);

	typedef DWORD (__stdcall *LPADAPTED_OPERATION)(STRINGLIST& params, INSTALLCALLBACK callback, LPVOID userdata);
	typedef std::map<kru_string, LPADAPTED_OPERATION> ADAPTED_OPERATION_LIST;

	

	/*struct APIErrorException
	{
		const TCHAR* msg;
		const DWORD return_value;
		const DWORD last_error;
		APIErrorException(const TCHAR* msg, const DWORD return_value, const DWORD last_error)
		{this->msg = msg; this->return_value = return_value; this->last_error = last_error;}
	};*/

	struct APIErrorException
	{
		const TCHAR* msg;
		const DWORD return_value;
		const DWORD last_error;
		APIErrorException(const TCHAR* msg, const DWORD return_value, const DWORD last_error):
		msg(msg), return_value(return_value), last_error(last_error){}
	};


	struct UserAbortException
	{
		const TCHAR* msg;	
		UserAbortException(const TCHAR* msg):
		msg(msg){}
	};

	struct UserSkipException
	{
		const TCHAR* msg;	
		UserSkipException(const TCHAR* msg):
		msg(msg){}
	};

	void CheckError(DWORD error, INSTALLCALLBACK callback, LPVOID userdata);

}// install

#endif //ADAPTED_H