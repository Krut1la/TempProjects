#include "Renderer.h"
#include "Engine.h"
#include "StringUtils.h"

namespace RTS
{
CRenderThread::CRenderThread( bool CreateSuspended, 
                            CRenderer* Renderer ):
CThread(CreateSuspended ), 
f_Renderer( Renderer ),
f_CallBack( NULL )
{

}

CRenderThread::~CRenderThread( void )
{
   
}

void CRenderThread::Execute( void )
{
   while (!GetTerminated())
   {
      f_Renderer->Render();
      f_Renderer->m_ExecutionCount++;
   }

   f_Renderer->m_Engine->GetSysLogger()->Log( L"End RenderThread", CMessageMSG() );
}

CRenderer::CRenderer( CEngine* Engine):
CMTService( Engine ),
m_PrimaryScene( NULL ),
m_SecondaryScene( NULL ),
m_LockSeconaryScene( NULL ),
m_FullScreen( true ),
m_WireFrame( false ),
m_Width( 1680 ),
m_Height( 1050 ),
m_BPP( 32 )
{
   m_Name = L"Renderer";

   m_FPS = 0;
   m_Frames = 0;
   m_Last = 0;

   m_CurrentMaterialIndex   = 0;
   m_CurrentLightIndex      = 0;
   m_CurrentPrimitiveType  = ptTriangleList;

   m_NeedsRenderBatch      = false;
}

CRenderer::~CRenderer( void )
{

}

bool CRenderer::Initialize( void )
{
   if ( !(CMTService::Initialize()) )
      return false;

   m_LockSeconaryScene = std::auto_ptr<CCriticalSection>( new CCriticalSection() );
   m_SecondaryIsFreeEvent = std::auto_ptr<CEvent>( new CEvent(NULL, true, false, L"isFree", false) );
   m_SecondaryIsReadyEvent = std::auto_ptr<CEvent>( new CEvent(NULL, true, false, L"isReady", false) );

   m_SecondaryIsFreeEvent->SetEvent();
   m_SecondaryIsReadyEvent->ResetEvent();

   m_PrimaryScene = std::auto_ptr<CScene>( new CScene() );
   m_SecondaryScene = std::auto_ptr<CScene>( new CScene() );

   m_PrimarySceneRef = m_PrimaryScene.get();
   m_SecondarySceneRef = m_SecondaryScene.get();


   return true;    
}

void CRenderer::Finalize( void )
{
   ((CEngine*)m_Engine)->GetSysLogger()->Log( L"\t Secodary rendered " + IntToStr( m_SecondaryScene->m_RenderTimes ) + L" times", CMessageMSG() );
   ((CEngine*)m_Engine)->GetSysLogger()->Log( L"\t Primary rendered " + IntToStr( m_PrimaryScene->m_RenderTimes ) + L" times", CMessageMSG() );
   CMTService::Finalize();
}

bool CRenderer::Start( void )
{
   if ( m_Engine->GetMultiThread() ) 
   {
      m_Engine->GetSysLogger()->Log( L"Start RenderThread", CMessageMSG() );
      m_Thread = std::auto_ptr<CRenderThread>( new CRenderThread(true, this) );
   
      //m_Thread->SetPriority( tpHigher );
      m_Thread->Resume();
   }
   else
   {
      Render();
      m_ExecutionCount++;
   }

   return true;
}

bool CRenderer::Render()
{
   if ( !PreRenderScene() )
      return false;

   if ( !RenderScene() )
      return false;

   if ( !PostRenderScene() )
      return false;

   return true;
}

bool CRenderer::PreRenderScene()
{
   ClearLights();

   if ( m_Engine->GetMultiThread() ) 
   {
      if ( m_SecondaryIsReadyEvent->WaitFor( 200 ) == WAIT_TIMEOUT )
         return false;
      m_SecondaryIsFreeEvent->ResetEvent();
   }

   return true;
}

bool CRenderer::RenderScene()
{
   while ( !m_SecondarySceneRef->IsEmpty() )
   {
      IRenderable* Renderable = NULL;
      switch ( *(RWORD*)m_SecondarySceneRef->Pop( sizeof( RWORD ) ) )
      {
      case RENDER_STATIC_SPRITE:
         {
         Renderable = m_Engine->GetResources()->GetRenderableRef( *(RWORD*)m_SecondarySceneRef->Pop(sizeof( RWORD )) );

         VECTOR3F Pos = *(VECTOR3F*)m_SecondarySceneRef->Pop( sizeof( VECTOR3F ));
         VECTOR3F Sc = *(VECTOR3F*)m_SecondarySceneRef->Pop(sizeof( VECTOR3F ));
         VECTOR3F Rot = *(VECTOR3F*)m_SecondarySceneRef->Pop(sizeof( VECTOR3F ));
         COLOR Col = *(COLOR*)m_SecondarySceneRef->Pop(sizeof( COLOR ));
/*         RenderSprite( (CSprite*)Renderable ,
            *(VECTOR3F*)m_SecondarySceneRef->Pop(),
            *(VECTOR3F*)m_SecondarySceneRef->Pop(),
            *(VECTOR3F*)m_SecondarySceneRef->Pop(),
            *(COLOR*)m_SecondarySceneRef->Pop() );
            */
         RenderSprite( (CSprite*)Renderable ,
            Pos,
            Sc,
            Rot,
            Col );
            
         break;
         }
      case RENDER_STATIC_MODEL:
         {
         Renderable = m_Engine->GetResources()->GetRenderableRef( *(UINT32*)m_SecondarySceneRef->Pop(sizeof( UINT32 )) );

         VECTOR3F Pos = *(VECTOR3F*)m_SecondarySceneRef->Pop( sizeof( VECTOR3F ));
         VECTOR3F Sc = *(VECTOR3F*)m_SecondarySceneRef->Pop(sizeof( VECTOR3F ));
         VECTOR3F Rot = *(VECTOR3F*)m_SecondarySceneRef->Pop(sizeof( VECTOR3F ));
/*         RenderSprite( (CSprite*)Renderable ,
            *(VECTOR3F*)m_SecondarySceneRef->Pop(),
            *(VECTOR3F*)m_SecondarySceneRef->Pop(),
            *(VECTOR3F*)m_SecondarySceneRef->Pop(),
            *(COLOR*)m_SecondarySceneRef->Pop() );
            */
         RenderModel( (CModel*)Renderable ,
            Pos,
            Sc,
            Rot);
            
         break;
         }
      }
   }

/*   for( UINT32 iElement = 0; iElement < m_SecondarySceneRef->m_LastElement; ++iElement)
   {
      IRenderable* Renderable = NULL;

      if ( m_SecondarySceneRef->m_Elements[iElement].m_MutatorRef != NULL )
      {
         ((CMutator*)(m_SecondarySceneRef->m_Elements[iElement].m_MutatorRef))->Mutate( m_SecondarySceneRef->m_Elements[iElement], Renderable, this );
      }

      switch ( m_SecondarySceneRef->m_Elements[iElement].m_RenderableType )
      {
      case taticModel:
         Renderable = m_Engine->GetResources()->GetRenderableRef( m_SecondarySceneRef->m_Elements[iElement].m_StaticRenderableRef );
         RenderModel( (CModel*)Renderable,
            m_SecondarySceneRef->m_Elements[iElement].m_Position,
            m_SecondarySceneRef->m_Elements[iElement].m_Scale,
            m_SecondarySceneRef->m_Elements[iElement].m_Rotation );
         break;
      case taticSprite:
         Renderable = m_Engine->GetResources()->GetRenderableRef( m_SecondarySceneRef->m_Elements[iElement].m_StaticRenderableRef );
         RenderSprite( (CSprite*)Renderable,
            m_SecondarySceneRef->m_Elements[iElement].m_Position,
            m_SecondarySceneRef->m_Elements[iElement].m_Scale,
            m_SecondarySceneRef->m_Elements[iElement].m_Rotation,
            m_SecondarySceneRef->m_Elements[iElement].m_Color
            );
         break;
      case rtDynamicSprite:
         RenderSprite( (CSprite*)m_SecondarySceneRef->m_Elements[iElement].m_DynamicRenderableRef,
            m_SecondarySceneRef->m_Elements[iElement].m_Position,
            m_SecondarySceneRef->m_Elements[iElement].m_Scale,
            m_SecondarySceneRef->m_Elements[iElement].m_Rotation,
            m_SecondarySceneRef->m_Elements[iElement].m_Color
            );
         break;
      case rtDynamicLight:
         RenderLight( (LIGHT*)m_SecondarySceneRef->m_Elements[iElement].m_DynamicRenderableRef );
         break;
      case rtTransform:
         SetTransform( (TRANSFORM*)m_SecondarySceneRef->m_Elements[iElement].m_DynamicRenderableRef );
         break;
      }


      if ( m_SecondarySceneRef->m_Elements[iElement].m_MutatorRef != NULL )
      {
         ((CMutator*)(m_SecondarySceneRef->m_Elements[iElement].m_MutatorRef))->DeMutate( m_SecondarySceneRef->m_Elements[iElement], Renderable, this );
      }
   }*/

   return true;
}

bool CRenderer::PostRenderScene()
{
   if ( m_Engine->GetMultiThread() ) 
   {
      m_SecondaryIsReadyEvent->ResetEvent();
//      UnlockSecondaryScene();

      m_SecondaryIsFreeEvent->SetEvent();
   }

   // TODO: Replace GetTickCount
   if (m_Last == 0)
      m_Last = GetTickCount();

   m_Frames++;

   if ( GetTickCount() - m_Last > 1000)
   {
      m_FPS = m_Frames;
      m_Last = 0;
      m_Frames = 0;
   }

   return true;
}

bool CRenderer::ClearLights( void )
{

   return true;
}

void CRenderer::SwapScenes( void )
{
   CScene* TempSceneRef = m_SecondarySceneRef;
   m_SecondarySceneRef = m_PrimarySceneRef;
   m_PrimarySceneRef = TempSceneRef;
}

bool CRenderer::RenderModel( CModel* Model, 
                     const VECTOR3F& Position, 
                     const VECTOR3F& Scale, 
                     const VECTOR3F& Rotation )
{
   return true;
}

bool CRenderer::RenderSprite( CSprite* Sprite, 
                     const VECTOR3F& Position, 
                     const VECTOR3F& Scale, 
                     const VECTOR3F& Rotation,
                     COLOR Color )
{
   return true;
}

bool CRenderer::RenderLight( LIGHT* Light )
{
   return true;
}

bool CRenderer::SetTransform(  TRANSFORM* Transform )
{
   return true;
}

bool CRenderer::DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 )
{
   if ( CMTService::DoCommand( Command, Data1, Data2 ) )
      return true;

   switch ( Command )
   {
   case MTC_SET_DEVICE_SIZE:
      m_Width      = *(UINT32*)Data1;
      m_Height   = *(UINT32*)Data2;
      return true;
   case MTC_GET_PRIMARY_SCENE:
      *(CScene**)Data1   = m_PrimarySceneRef;
      return true;
   case MTC_GET_SECONDARY_SCENE:
      *(CScene**)Data1 = m_SecondarySceneRef;
      return true;
   case MTC_GET_SECONDARY_ISFREE:
      *(CEvent**)Data1 = m_SecondaryIsFreeEvent.get();
      return true;
   case MTC_GET_SECONDARY_ISREADY:
      *(CEvent**)Data1 = m_SecondaryIsReadyEvent.get();
      return true;
   case MTC_SWAP_SCENES:
      SwapScenes();
      return true;
   default:
      return false;
   }

   return false;
}
}