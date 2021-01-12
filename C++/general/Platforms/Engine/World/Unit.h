#ifndef UNIT_H
#define UNIT_H

#include <vector>
#include "Types.h"
#include "3DTypes.h"
#include "IUpdatable.h"
#include "IProvidable.h"

namespace RTS
{
class CUnit;

typedef std::vector<CUnit*> UNITLIST;


class CUnit: public IUpdatable, public IProvidable
{
private:
protected:
   void*         m_WorldRef;

   VECTOR3F      m_Position;
   VECTOR3F      m_Scale;
   VECTOR3F      m_Rotation;

   CUnit( void* World );
public:
   virtual ~CUnit( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
   virtual void AddUnit( CUnit* Unit );
};
}

#endif // UNIT_H