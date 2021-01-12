#include "../include/stdafx.h"
#include "../include/adapted.h"

void CheckError(DWORD error, INSTALLCALLBACK callback, LPVOID userdata)
{
	if(callback)
	{
		switch(callback(msgRequest, NULL, reqUnknownError, error, userdata))
		{
		case ansAbort:	throw UserAbortException(TEXT(""));
		case ansSkip:	throw UserSkipException(TEXT(""));
		case ansRetry:	return;
		default:		throw APIErrorException(TEXT("Unknown answer from user"), error, GetLastError());
		}
	}
	else
		throw APIErrorException(TEXT(""), error, GetLastError());
}
