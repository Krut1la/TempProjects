#ifndef TESTWORLD_H
#define TESTWORLD_H

#include "World.h"

class CTestWorld: public RTS::CWorld
{
private:
protected:

public:
   CTestWorld( void );
   virtual ~CTestWorld( void );

   virtual bool Update( float DeltaTime, RTS::CScene* Scene );
   virtual bool HandleInputEvent( RTS::UINT32 EventID, RTS::INT32 wParam, RTS::INT32 lParam );

};

#endif // TESTWORLD_H