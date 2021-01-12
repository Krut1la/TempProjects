#ifndef SYSFACTORY_H
#define SYSFACTORY_H

#include "Types.h"
#include "Mutator.h"
#include "Engine.h"

namespace RTS
{

class CSysFactory
{
private:
protected:
public:

   virtual VOID_PTR CreateObject( const std::wstring& Name, CEngine* Engine ) = 0;
};

extern CSysFactory* g_SysFactory;
}

#endif // SYSFACTORY_H