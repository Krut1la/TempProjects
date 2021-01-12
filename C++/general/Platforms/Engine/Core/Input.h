#ifndef INPUT_H
#define INPUT_H

#include "MTService.h"
#include "IUpdatable.h"

namespace RTS
{
class CMTService;

class CInput: public CMTService{
protected:
   IUPDATABLELIST      m_Updatables;

   bool SendEventToUpdatables( UINT32 EventID, INT32 wParam, INT32 lParam );

   CInput( CEngine* Engine );
public:
   virtual ~CInput(void);

   virtual bool Initialize();
   virtual void Finalize();

   virtual bool Start();

   virtual bool DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 );

   void AddUpdatable( IUpdatable* Updatable );

};
}
#endif // INPUT_H