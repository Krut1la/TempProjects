#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "ThreadUtils.h"
#include "MTService.h"

namespace RTS
{
class CController;

class CControllerThread: public CThread
{
   CController*   f_Controller;
public:

   THREADMETHOD f_CallBack;

   CControllerThread(bool CreateSuspended, CController* Controller);
   virtual ~CControllerThread(void);

   virtual void Execute( void );
};


class CController: public CMTService
{
   friend CControllerThread;
private:
   CMTService*         m_RendererRef;
protected:
   UINT32            m_LastTime;

   std::auto_ptr<CThread>   m_ControllerThread;

public:
   CController( CEngine* Engine);
   virtual ~CController(void);

   virtual bool Initialize();
   virtual void Finalize();
   virtual bool Start();

   bool Update( void );
};
}
#endif // CONTROLLER_H