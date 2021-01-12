#ifndef TESTUNIT_H
#define TESTUNIT_H

#include "Unit.h"


class CTestUnit: public CUnit
{
private:
protected:
public:
   CTestUnit( void );
   virtual ~CTestUnit( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
};

#endif // TESTUNIT_H