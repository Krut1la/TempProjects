#include "ThreadUtils.h"
namespace General
{
   volatile LONG     g_ThreadCount     = 0;
   volatile DWORD    g_MainThreadID    = GetCurrentThreadId();
   volatile HANDLE   g_SyncEvent       = NULL;
   void*             g_WakeMainThread  = NULL;
   SYNCPROCLIST*     g_SyncList        = NULL;
   CRITICAL_SECTION  g_ThreadLock;

   void InitThreadSynchronization( void )
   {
      InitializeCriticalSection( &g_ThreadLock );

      g_SyncEvent = CreateEvent( NULL, true, false, L'' );
      if ( g_SyncEvent == 0 )
         throw; //RaiseLastOSError;
   }

   void DoneThreadSynchronization( void )
   {
      DeleteCriticalSection( &g_ThreadLock );
      CloseHandle( g_SyncEvent );
   }


   void SignalSyncEvent( void )
   {
      SetEvent( g_SyncEvent );
   }

   void ResetSyncEvent( void )
   {
      ResetEvent( g_SyncEvent );
   }

   void WaitForSyncEvent( int Timeout )
   {
      if ( WaitForSingleObject( g_SyncEvent, Timeout ) == WAIT_OBJECT_0 )
         ResetSyncEvent();
   }

   bool CheckSynchronize( int Timeout = 0 )
   {
      bool Result = false;

      SYNCPROC* SyncProc;
      SYNCPROCLIST* LocalSyncList = NULL;

      if ( GetCurrentThreadId() != g_MainThreadID )
         throw; //raise EThread.CreateResFmt(@SCheckSynchronizeError, [GetCurrentThreadID]);
      if ( Timeout > 0 )
         WaitForSyncEvent( Timeout );
      else
         ResetSyncEvent();

      EnterCriticalSection( &g_ThreadLock );

      //_TRY_BEGIN
      LocalSyncList = (SYNCPROCLIST*)InterlockedExchange((volatile LONG*)&g_SyncList, (LONG)LocalSyncList);
      //_TRY_BEGIN
      Result = (LocalSyncList != NULL) && (LocalSyncList->size() > 0);
      if ( Result )
      {
         while ( LocalSyncList->size() > 0 )
         {
            SyncProc = *(LocalSyncList->begin());
            LocalSyncList->remove( SyncProc );
            LeaveCriticalSection( &g_ThreadLock );
            //_TRY_BEGIN
            _TRY_BEGIN
               SyncProc->SyncRec->f_Method(SyncProc->SyncRec->f_Data);
            _CATCH_ALL
               if ( !SyncProc->Queued )
                  SyncProc->SyncRec->f_SynchronizeException = NULL;// AcquireExceptionObject();
               else
                  throw;
            _CATCH_END;
            //_CATCH_ALL
            EnterCriticalSection( &g_ThreadLock );
            //_CATCH_END;
            if ( !SyncProc->Queued )
               SetEvent( SyncProc->Signal );
            else
            {
               delete SyncProc->SyncRec;
               delete SyncProc;
            }
         }
      }
      //_CATCH_ALL
      delete LocalSyncList;
      //_CATCH_END
      //_CATCH_ALL
      LeaveCriticalSection( &g_ThreadLock );
      //_CATCH_END

      return Result;
   }

   CSynchroObject::CSynchroObject( void )
   {
   }

   CSynchroObject::~CSynchroObject( void )
   {
   }

   void CSynchroObject::Acquire( void )
   {
   }

   void CSynchroObject::Release( void )
   {
   }



   CCriticalSection::CCriticalSection( void )
   {
      InitializeCriticalSection( &f_Section );
   }

   CCriticalSection::~CCriticalSection( void )
   {
      DeleteCriticalSection( &f_Section );
   }

   void CCriticalSection::Acquire( void )
   {
      EnterCriticalSection( &f_Section );
   }

   void CCriticalSection::Release( void )
   {
      LeaveCriticalSection( &f_Section );
   }

   bool CCriticalSection::TryEnter( void )
   {
#if(_WIN32_WINNT >= 0x0400)
      return (TryEnterCriticalSection( &f_Section ) != 0);
#else
      return false;
#endif
   }

   void CCriticalSection::Enter( void )
   {
      Acquire();
   }

   void CCriticalSection::Leave( void )
   {
      Release();
   }


   CHandleObject::CHandleObject( bool UseCOMWait )
   {
      f_UseCOMWait = UseCOMWait;
      f_Handle = NULL;
      f_LastError = 0;
   }

   CHandleObject::~CHandleObject( void )
   {
      if ( f_Handle != NULL )
         CloseHandle( f_Handle );
   }

   WAITRESULT CHandleObject::WaitFor( UINT32 Timeout )
   {
      WAITRESULT   wr;

      if ( f_UseCOMWait) 
      {
         /*      DWORD      Index;
         switch ( CoWaitForMultipleHandles(0, Timeout, 1, f_Handle, Index ) )
         {
         case S_OK:
         wr = wrSignaled;
         break;
         case RPC_S_CALLPENDING:
         case RPC_E_TIMEOUT: 
         wr = wrTimeout;
         break;
         default:
         wr = wrError;
         f_LastError = ::GetLastError();
         }*/

         wr = wrError;
      } 
      else
      {
         switch ( WaitForSingleObject( GetHandle(), Timeout) )
         {
         case WAIT_ABANDONED:
            wr = wrAbandoned;
            break;
         case WAIT_OBJECT_0:
            wr = wrSignaled;
            break;
         case WAIT_TIMEOUT: 
            wr = wrTimeout;
            break;
         case WAIT_FAILED:
            wr = wrError;
            f_LastError = ::GetLastError();
            break;
         default:
            wr = wrError;
         }
      }

      return wr;
   }

   CEvent::CEvent( LPSECURITY_ATTRIBUTES EventAttributes,
      bool ManualReset,
      bool InitialState,
      const std::wstring& Name,
      bool UseCOMWait):
   CHandleObject( UseCOMWait )
   {
      f_Handle = CreateEvent( EventAttributes, ManualReset, InitialState, Name.c_str() );
   }

   CEvent::CEvent(   bool UseCOMWait)
   {
      CEvent::CEvent( NULL,
         true,
         false,
         L"",
         UseCOMWait);
   }

   void CEvent::SetEvent( void )
   {
      ::SetEvent( f_Handle );
   }

   void CEvent::ResetEvent( void )
   {
      ::ResetEvent( f_Handle );
   }


   CThread::CThread( bool CreateSuspended )
   {
      if ( g_ThreadCount == 0 )
         InitThreadSynchronization();

      InterlockedIncrement(&g_ThreadCount);

      f_FreeOnTerminate = false;
      f_Finished = false;
      f_ReturnValue = 0;
      f_Terminated = false;
      f_Suspended = CreateSuspended;
      f_CreateSuspended = CreateSuspended;
      f_Handle = (HANDLE)_beginthreadex(NULL, 0, &ThreadProc, (void*)this, CREATE_SUSPENDED, &f_ThreadID);

      if ( f_Handle == 0 ) 
         throw;
      // TODO: throw something

      AfterConstruction();
   }

   CThread::~CThread( void )
   {
      if (( f_ThreadID != 0 ) && ( !f_Finished ))
      {
         Terminate();

         if ( f_CreateSuspended )
            Resume();

         WaitFor();
      }

      RemoveQueuedEvents( NULL, this, NULL );

      if ( f_Handle != 0 )
         CloseHandle( f_Handle );

      InterlockedDecrement( &g_ThreadCount );

      if ( g_ThreadCount == 0 )
         DoneThreadSynchronization();
   }

   UINT WINAPI CThread::ThreadProc( void* Thread )
   {
      CThread* ThisThread = (CThread*)Thread;

      bool FreeThread = false;
      int Result = 0;

      if ( !ThisThread->f_Terminated )
      {
         _TRY_BEGIN
            ThisThread->Execute();
         _CATCH_ALL
            //         Thread->FFatalException = AcquireExceptionObject;
            _CATCH_END
      }

      FreeThread = ThisThread->f_FreeOnTerminate;
      Result = ThisThread->f_ReturnValue;
      ThisThread->DoTerminate();
      ThisThread->f_Finished = true;
      SignalSyncEvent();

      if ( FreeThread ) 
         delete ThisThread;

      _endthreadex( Result );

      return Result;
   }

   void CThread::AfterConstruction( void )
   {
      if (!f_CreateSuspended)
         Resume();
   }

   void CThread::DoTerminate( void )
   {

   }

   void CThread::Resume(void)
   {
      int SuspendCount;

      SuspendCount = ResumeThread( HANDLE( f_Handle ));
      //  CheckThreadError(SuspendCount >= 0);
      if ( SuspendCount == 1 )
         f_Suspended = false;
   }

   void CThread::Suspend(void)
   {
   }

   void CThread::Terminate(void)
   {
      f_Terminated = true;
   }

   unsigned CThread::WaitFor(void)
   {
      HANDLE H[2];

      DWORD WaitResult;
      MSG Msg;

      H[0] = HANDLE(f_Handle);

      if ( GetCurrentThreadId() == g_MainThreadID ) 
      {
         WaitResult = 0;
         H[1] = g_SyncEvent;
         do
         {
            // This prevents a potential deadlock if the background thread
            //  does a SendMessage to the foreground thread 

            if ( WaitResult == WAIT_OBJECT_0 + 2 ) 
               PeekMessage(&Msg, 0, 0, 0, PM_NOREMOVE);

            WaitResult = MsgWaitForMultipleObjects(2, &H[0], false, 1000, QS_SENDMESSAGE);

            //      CheckThreadError(WaitResult <> WAIT_FAILED);
            if ( WaitResult == WAIT_OBJECT_0 + 1 )
               CheckSynchronize();
         }
         while ( WaitResult != WAIT_OBJECT_0 );
      }
      else 
         WaitForSingleObject( H[0], INFINITE );

      //  CheckThreadError(GetExitCodeThread(H[0], Result));


      return 0;
   }

   void CThread::Queue(void* vmt, CThread* AThread, THREADMETHOD Method)
   {
   }

   void CThread::RemoveQueuedEvents(void* vmt, CThread* AThread, THREADMETHOD Method)
   {
      SYNCPROC* SyncProc;

      EnterCriticalSection( &g_ThreadLock );
      //_TRY_BEGIN
      if ( g_SyncList != NULL )
      {
         //      for (size_t iSyncProc = (g_SyncList)->size() - 1; iSyncProc >= 0; -- iSyncProc)
         for (SYNCPROCLIST::reverse_iterator i_SyncProc = g_SyncList->rbegin(); i_SyncProc != g_SyncList->rend(); i_SyncProc++)
         {
            SyncProc = *(i_SyncProc);
            if ((SyncProc->Signal == 0) &&
               (((AThread != NULL) && (SyncProc->SyncRec->f_Thread == AThread))/* ||
                                                                               ((AMethod != NULL) && (TMethod(SyncProc.SyncRec.FMethod).Code = TMethod(AMethod).Code) and
                                                                               (TMethod(SyncProc.SyncRec.FMethod).Data = TMethod(AMethod).Data))*/))
            {
               g_SyncList->remove(*i_SyncProc);
               delete SyncProc->SyncRec;
               delete SyncProc;
            }
         }
      }
      //_CATCH_ALL
      LeaveCriticalSection( &g_ThreadLock );
      //_CATCH_END
   }

   void CThread::StaticQueue(void* vmt, CThread* AThread, THREADMETHOD Method)
   {
   }

   void CThread::Synchronize(void* vmt, CThread* AThread, THREADMETHOD Method)
   {
   }

   void CThread::StaticSynchronize(void* vmt, CThread* AThread, THREADMETHOD Method)
   {
   }

   void CThread::Synchronize( THREADMETHOD Method, VOID_PTR Data )
   {
      f_Synchronize.f_Thread = this;
      f_Synchronize.f_SynchronizeException = NULL;
      f_Synchronize.f_Method = Method;
      f_Synchronize.f_Data = Data;

      Synchronize(&f_Synchronize);
   }

   void CThread::Synchronize(SYNCHRONIZERECORD* SyncRec, bool QueueEvent)
   {
      SYNCPROC SyncProc;
      SYNCPROC* SyncProcPtr;

      if ( GetCurrentThreadId() == g_MainThreadID )
         SyncRec->f_Method(SyncRec->f_Data);
      else
      {
         if ( QueueEvent )
            SyncProcPtr = new SYNCPROC();
         else
            SyncProcPtr = &SyncProc;
         if ( !QueueEvent )
            SyncProcPtr->Signal = CreateEvent( NULL, true, false, NULL );
         else
            SyncProcPtr->Signal = 0;
         //_TRY_BEGIN
         EnterCriticalSection( &g_ThreadLock );
         //_TRY_BEGIN
         SyncProcPtr->Queued = QueueEvent;
         if ( g_SyncList == NULL )
            g_SyncList = new SYNCPROCLIST();
         SyncProcPtr->SyncRec = SyncRec;
         g_SyncList->push_back(SyncProcPtr);
         SignalSyncEvent();
         if ( g_WakeMainThread != NULL )
            Sleep(1); // Call g_WakeMainThread
         if ( !QueueEvent )
         {
            LeaveCriticalSection( &g_ThreadLock );
            //_TRY_BEGIN
            WaitForSingleObject( SyncProcPtr->Signal, INFINITE);
            //_CATCH_ALL
            EnterCriticalSection( &g_ThreadLock );
            //_CATCH_END
         }
         //_CATCH_ALL
         LeaveCriticalSection( &g_ThreadLock );
         //_CATCH_END
         //_CATCH_ALL
         if ( !QueueEvent )
            CloseHandle(SyncProcPtr->Signal);
         //_CATCH_END
         if ( !QueueEvent && (SyncRec->f_SynchronizeException != NULL) )
            throw; //raise ASyncRec.FSynchronizeException;
      }
   }

   void CThread::SetPriority( THREADPRIORITY Value )
   {
      SetThreadPriority(f_Handle, Priorities[Value]);
   }
}