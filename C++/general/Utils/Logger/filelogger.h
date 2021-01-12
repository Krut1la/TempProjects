#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <string>
#include "Logger.h"
#include <fstream>

namespace RTS
{
class CFileLogger: public CLogger{
  protected:

   std::wofstream    m_Out;


  public:
    CFileLogger(const std::wstring& FileName, const std::wstring& Location = L"");
    virtual ~CFileLogger(void);

   virtual void Log(const std::wstring& name, const CMessageType& MessageType, UINT32 Level = LOGGER_THRESHOLD_NORMAL);
   
};
}

#endif // FILELOGGER_H