#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "Types.h"
#include "MessageType.h"

namespace RTS
{
const UINT32 LOGGER_THRESHOLD_TRIVIAL   = 0;
const UINT32 LOGGER_THRESHOLD_MINOR   = 1;
const UINT32 LOGGER_THRESHOLD_NORMAL   = 2;
const UINT32 LOGGER_THRESHOLD_MAJOR   = 3;
const UINT32 LOGGER_THRESHOLD_CRITICAL = 4;
const UINT32 LOGGER_THRESHOLD_FATAL   = 5;


class CLogger{
  protected:
   UINT32 f_Threshold;

  public:
   CLogger(void);
    virtual ~CLogger(void);

   UINT32 GetThreshold(void){ return f_Threshold; };
   void SetThreshold(UINT32 Threshold){ f_Threshold = Threshold; };

    virtual void Log(const std::wstring& name, const CMessageType& MessageType, UINT32 Level = LOGGER_THRESHOLD_NORMAL) = 0;
};
}

#endif // LOGGER_H