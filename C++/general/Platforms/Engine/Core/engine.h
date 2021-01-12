#ifndef ENGINE_H
#define ENGINE_H

#include "Types.h"
#include "Logger.h"
#include "MTService.h"
#include "Resources.h"
#include "World.h"
#include "GUI.h"

namespace RTS
{

class CEngine{
protected:
   // Settings
   bool         m_MultiThread;

   // Objects
   MTSERVICELIST              m_Services;
   std::auto_ptr<CLogger>     m_SysLogger;
   std::auto_ptr<CLogger>     m_StatLogger;
   std::auto_ptr<CResources>  m_Resources;
   std::auto_ptr<CWorld>      m_World;
   std::auto_ptr<CGUI>        m_GUI;

   // State
   bool         m_Terminated;
  
public:
   CEngine( void );
   virtual ~CEngine( void );
   virtual bool Initialize( void );
   virtual void Finalize( void );
   virtual void Run( void );

   virtual bool LoadPAK( const std::wstring& FileName );

   // Settings
   bool GetMultiThread( void ) { return m_MultiThread; }
   void SetMultiThread( bool MultiThread ) { m_MultiThread = MultiThread; }

   // Objects
   CMTService* FindService( const std::wstring& name);

   CLogger* GetSysLogger( void ) { return m_SysLogger.get(); }
   CLogger* GetStatLogger( void ) { return m_StatLogger.get(); }
   CWorld* GetWorld( void ) { return m_World.get(); }
   CGUI* GetGUI( void ) { return m_GUI.get(); }
   CResources* GetResources( void ) { return m_Resources.get(); }

   // State
   void SetTerminated( bool Terminated ) { m_Terminated = Terminated; }

};

}

#endif // ENGINE_H