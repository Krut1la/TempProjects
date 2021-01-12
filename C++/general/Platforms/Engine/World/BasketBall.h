#ifndef BASKETBALL_H
#define BASKETBALL_H

#include "Unit.h"

namespace RTS
{
class CBasketBall: public CUnit
{
private:
   float dr;

   float m_LastTime;

   VECTOR3F m_Speed;
   VECTOR3F m_Gravitation;
protected:
   UINT32         m_BasketBallModelRef;
public:
   CBasketBall( void* World );
   virtual ~CBasketBall( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
};
}


#endif // BASKETBALL_H