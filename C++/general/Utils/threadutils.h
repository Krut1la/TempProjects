#ifndef THREADUTILS_H
#define THREADUTILS_H

// Includes
#include "windows.h"
#include "winbase.h"
#include "process.h"
#include <string>
#include <list>
#include "Types.h"

namespace General
{
   // Typedefs
   typedef enum THREADPRIORITY { 
      tpIdle, 
      tpLowest, 
      tpLower, 
      tpNormal, 
      tpHigher, 
      tpHighest, 
      tpTimeCritical };

   const DWORD Priorities[] = {
      THREAD_PRIORITY_IDLE, 
      THREAD_PRIORITY_LOWEST, 
      THREAD_PRIORITY_BELOW_NORMAL,
      THREAD_PRIORITY_NORMAL, 
      THREAD_PRIORITY_ABOVE_NORMAL,
      THREAD_PRIORITY_HIGHEST, 
      THREAD_PRIORITY_TIME_CRITICAL
      };
     
   typedef void (__fastcall *THREADMETHOD )( VOID_PTR );

   typedef struct _SynchronizeRecord
   {
      
   public:
      void* f_Thread;
      THREADMETHOD f_Method;
      void* f_SynchronizeException;
      VOID_PTR f_Data;
   } SYNCHRONIZERECORD;

   typedef struct  _SyncProc
   {
      SYNCHRONIZERECORD* SyncRec; 
      bool Queued;
      HANDLE Signal;
   }SYNCPROC;    

   typedef std::list<SYNCPROC*> SYNCPROCLIST;

   // Consts

   // Globals

   extern volatile DWORD   g_MainThreadID;
   extern volatile LONG   g_ThreadCount;

   // Classes

   class CSynchroObject
   {
   protected:
      CSynchroObject( void );
   public:
      virtual ~CSynchroObject( void );

      virtual void Acquire( void );
      virtual void Release( void );
   };

   typedef enum {   wrSignaled, 
               wrTimeout, 
               wrAbandoned, 
               wrError} WAITRESULT;

   class CHandleObject: public CSynchroObject
   {
   protected:
      HANDLE      f_Handle;
      int         f_LastError;
      bool      f_UseCOMWait;
   public:
      CHandleObject( bool UseCOMWait = false );
      virtual ~CHandleObject( void );
   public:
      virtual WAITRESULT WaitFor( UINT32 Timeout );

      int GetLastError( void ){ return f_LastError; }
      HANDLE GetHandle( void ){ return f_Handle; }
   };

   class  CEvent: public CHandleObject
   {
   public:
      CEvent( LPSECURITY_ATTRIBUTES EventAttributes,
            bool ManualReset,
            bool InitialState,
            const std::wstring& Name,
            bool UseCOMWait = false);
      CEvent(   bool UseCOMWait = false);

      void SetEvent( void );
      void ResetEvent( void );
   };

   class  CCriticalSection: virtual CSynchroObject
   {
   protected:
      RTL_CRITICAL_SECTION f_Section;
   public:
      CCriticalSection( void );
      virtual ~CCriticalSection( void );

      virtual void Acquire( void );
      virtual void Release( void );
      bool TryEnter( void );
      void Enter( void );
      void Leave( void );
   };


   class CThread{
   private:
      HANDLE   f_Handle;
      UINT   f_ThreadID;

      bool     f_CreateSuspended;
      bool     f_Terminated;
      bool    f_Suspended;
      bool    f_FreeOnTerminate;
      bool    f_Finished;
      int       f_ReturnValue;
   //    FOnTerminate: TNotifyEvent;
      SYNCHRONIZERECORD f_Synchronize;
   //    FFatalException: TObject;
      void CallOnTerminate( void );

      static void Synchronize(SYNCHRONIZERECORD* SyncRec, bool QueueEvent = false);

      void SetSuspended( bool Value );

      static UINT WINAPI ThreadProc( void* Thread );
       
   protected:

      void CheckThreadError( int ErrCode );
      void CheckThreadError( bool Success );
      virtual void DoTerminate( void );
      virtual void Execute( void ) = 0 ;
      void Queue( THREADMETHOD Method );
   public:
      CThread(bool CreateSuspended);
      virtual ~CThread(void);

      HANDLE GetHandle( void ) { return f_Handle; }
      bool GetTerminated( void ) { return f_Terminated; }
      bool GetSuspended( void ) { return f_Suspended; }
      void SetPriority(THREADPRIORITY Value);

      virtual void AfterConstruction(void);
      void Resume(void);
      void Suspend(void);
      void Terminate(void);
      unsigned WaitFor(void);
      void Synchronize( THREADMETHOD Method, VOID_PTR Data );
      static void Queue(void* vmt, CThread* AThread, THREADMETHOD Method);
      static void RemoveQueuedEvents(void* vmt, CThread* AThread, THREADMETHOD Method);
      static void StaticQueue(void* vmt, CThread* AThread, THREADMETHOD Method);
      static void Synchronize(void* vmt, CThread* AThread, THREADMETHOD Method);
      static void StaticSynchronize(void* vmt, CThread* AThread, THREADMETHOD Method);
   };
}

#endif // THREADUTILS_H