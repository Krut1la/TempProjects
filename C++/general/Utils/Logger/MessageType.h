#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H

#include <string>
#include "Types.h"

namespace RTS
{
class CMessageType{
  protected:
    std::wstring  f_Name;

   explicit CMessageType(const std::wstring& Name):f_Name(Name){};
  public:
   virtual ~CMessageType(void){};

   std::wstring GetName(void) const { return f_Name; };
};

class CMessageERR: public CMessageType{
  public:
     explicit CMessageERR(void): CMessageType(L"ERR"){};
};

class CMessageWRN: public CMessageType{
  public:
     explicit CMessageWRN(void): CMessageType(L"WRN"){};
};

class CMessageDBG: public CMessageType{
  public:
     explicit CMessageDBG(void): CMessageType(L"DBG"){};
};

class CMessageFAT: public CMessageType{
  public:
     explicit CMessageFAT(void): CMessageType(L"FAT"){};
};

class CMessageMSG: public CMessageType{
  public:
     explicit CMessageMSG(void): CMessageType(L"MSG"){};
};
}
#endif // MESSAGETYPE_H