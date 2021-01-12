#ifndef ROOM_H
#define ROOM_H

#include "Unit.h"

namespace RTS
{
class CRoom: public CUnit
{
private:
protected:
   UINT32   m_RoomModelRef;
public:
   CRoom( void* World );
   virtual ~CRoom( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
};
}

#endif // ROOM_H