#ifndef CURSOR_H
#define CURSOR_H

#include "Control.h"

namespace RTS
{
class CCursor: public CControl
{
private:
protected:
   RWORD         m_CursorModelRef;
public:
   CCursor( void* GUI );
   virtual ~CCursor( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
};
}


#endif // CURSOR_H