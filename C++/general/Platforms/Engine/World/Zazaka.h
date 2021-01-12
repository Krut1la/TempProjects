#ifndef ZAZAKA_H
#define ZAZAKA_H

#include "Unit.h"

namespace RTS
{
class CZazaka: public CUnit
{
private:
   float dx;
   float dy;
   float dz;
   float dr;

   float m_LastTime;
protected:
   RWORD         m_ZazakaModelRef;
   COLOR         m_Color;
public:
   float kkk;
   CZazaka( void* World );
   virtual ~CZazaka( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
};
}


#endif // ZAZAKA_H