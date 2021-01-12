#ifndef RENDERER_H
#define RENDERER_H

#include "MTService.h"
#include "Scene.h"
#include "Model.h"

namespace RTS
{
const MTSERVICE_COMMAND MTC_SET_DEVICE_SIZE           = 2001;
const MTSERVICE_COMMAND MTC_GET_PRIMARY_SCENE         = 2002;
const MTSERVICE_COMMAND MTC_GET_SECONDARY_SCENE       = 2003;
const MTSERVICE_COMMAND MTC_GET_SECONDARY_ISFREE      = 2004;
const MTSERVICE_COMMAND MTC_GET_SECONDARY_ISREADY     = 2005;
const MTSERVICE_COMMAND MTC_SWAP_SCENES               = 2006;

typedef enum {
   RSS_DEFAULT   = 0,
   RSS_NOLIGHT   = 1,
   RSS_GUI       = 2,
   RSS_WORLD     = 4,
   RSS_TEXT      = 8,
}RENDER_STATE_SET;

class CRenderer;

class CRenderThread: public CThread
{
   CRenderer*   f_Renderer;
public:

   THREADMETHOD f_CallBack;

   CRenderThread( bool CreateSuspended, CRenderer* Renderer );
   virtual ~CRenderThread( void );

   virtual void Execute( void );
};

class CRenderer: public CMTService
{
   friend CRenderThread;
private:
   UINT32                           m_Frames;
   UINT32                           m_Last;
protected:
   // Objects
   std::auto_ptr<CScene>            m_PrimaryScene;
   std::auto_ptr<CScene>            m_SecondaryScene;
   std::auto_ptr<CCriticalSection>  m_LockSeconaryScene;
   std::auto_ptr<CEvent>            m_SecondaryIsReadyEvent;
   std::auto_ptr<CEvent>            m_SecondaryIsFreeEvent;
   LIGHTLIST                        m_Lights;

   CScene*                          m_PrimarySceneRef;
   CScene*                          m_SecondarySceneRef;

   // Settings
   bool                             m_FullScreen;
   bool                             m_WireFrame;
   RWORD                            m_Width;
   RWORD                            m_Height;
   RWORD                            m_BPP;
   
   // State
   RWORD                            m_FPS;
   RWORD                            m_CurrentMaterialIndex;
   PRIMITIVETYPE                    m_CurrentPrimitiveType;
   RWORD                            m_CurrentLightIndex;
   bool                             m_NeedsRenderBatch;

   virtual bool PreRenderScene( void );
   virtual bool RenderScene( void );
   virtual bool PostRenderScene( void );

   virtual bool ClearLights( void );

   void SwapScenes( void );


   virtual bool RenderModel( CModel* Model, 
                        const VECTOR3F& Position, 
                        const VECTOR3F& Scale, 
                        const VECTOR3F& Rotation );

   virtual bool RenderSprite( CSprite* Sprite, 
                        const VECTOR3F& Position, 
                        const VECTOR3F& Scale, 
                        const VECTOR3F& Rotation,
                         COLOR Color);

   virtual bool RenderLight( LIGHT* Light );

   virtual bool SetTransform( TRANSFORM* Transform );
   bool Render( void );



   CRenderer( CEngine* Engine );
public:
   virtual ~CRenderer( void );

   virtual bool Initialize( void );
   virtual void Finalize( void );
   virtual bool Start( void );
   virtual bool DoCommand( MTSERVICE_COMMAND Command, VOID_PTR Data1, VOID_PTR Data2 );
};
}

#endif // RENDERER_H