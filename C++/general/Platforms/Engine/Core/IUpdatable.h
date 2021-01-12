#ifndef IUPDATABLE_H
#define IUPDATABLE_H

#include <list>
#include "Types.h"
#include "Scene.h"

namespace RTS
{
class IUpdatable;

typedef std::list<IUpdatable*> IUPDATABLELIST;

class IUpdatable
{
protected:
   float m_Time;
public:
   virtual bool Update( float DeltaTime, CScene* Scene ) = 0;
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam ) = 0;
};
}

#endif // IUPDATABLE_H