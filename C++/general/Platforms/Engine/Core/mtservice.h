#ifndef MTSERVICE_H
#define MTSERVICE_H

#include <string>
#include <list>

#include "ThreadUtils.h"

namespace RTS
{

   typedef UINT32 MTSERVICE_COMMAND;

   const MTSERVICE_COMMAND MTC_SET_NONE = 0;

   class CEngine;

   class CMTService{
   private:
   protected:
      bool               m_Initialized;

      std::wstring         m_Name;   
      std::auto_ptr<CThread>    m_Thread;

      CEngine*            m_Engine;

      CMTService( CEngine* Engine );
   public:
      UINT32   m_ExecutionCount;

      virtual ~CMTService( void );

      std::wstring GetName( void ) { return m_Name; }

      virtual bool Initialize( void );
      virtual void Finalize( void );
      virtual bool Start( void );
      virtual void Stop( void );
      virtual void WaitFor( void );

      virtual bool DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 );

   };

   typedef std::list<CMTService*> MTSERVICELIST;
}

#endif // IMTSERVICE_H