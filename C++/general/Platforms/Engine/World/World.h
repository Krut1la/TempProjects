#ifndef WORLD_H
#define WORLD_H

#include "Types.h"
#include "Scene.h"
#include "Unit.h"

namespace RTS
{
class CWorld: public IUpdatable
{
private:
protected:
   UNITLIST      m_Units;

   TRANSFORM      m_WorldTransform;
   TRANSFORM      m_ViewTransform;
   TRANSFORM      m_ProjTransform;

   LIGHT         m_MainLight;

   void UpdateTransform( CScene* Scene );
   void UpdateLights( CScene* Scene );

   CWorld( void );
public:
   virtual ~CWorld( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void AddUnit( CUnit* Unit );
};
}

#endif // WORLD_H