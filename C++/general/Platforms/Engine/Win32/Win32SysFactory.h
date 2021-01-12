#ifndef WIN32SYSFACTORY_H
#define WIN32SYSFACTORY_H

#include "SysFactory.h"

namespace RTS
{

class CWin32SysFactory: public CSysFactory
{
private:
protected:
public:
   CWin32SysFactory( void );
   virtual ~CWin32SysFactory( void );

   virtual VOID_PTR CreateObject( const std::wstring& Name, CEngine* Engine );
};

}

#endif // WIN32SYSFACTORY_H