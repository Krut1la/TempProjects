#ifndef GUI_H
#define GUI_H

#include "Types.h"
#include "IUpdatable.h"
#include "Control.h"
#include "Scene.h"


namespace RTS
{
const UINT32 MOUSE_MOVE = 1001;


class CGUI: public IUpdatable
{
private:
protected:
   float            m_ScreenWidth;
   float            m_ScreenHeight;

   void*            m_EngineRef;   

   CONTROLLIST      m_Controls;

   TRANSFORM      m_WorldTransform;
   TRANSFORM      m_ViewTransform;
   TRANSFORM      m_ProjTransform;

   void UpdateTransform( CScene* Scene );

public:
   CGUI( void );
   virtual ~CGUI( void );

   void* GetEngineRef( void ) { return m_EngineRef; }
   void SetEngineRef( void* EngineRef ) { m_EngineRef = EngineRef; }

   float GetScreenWidth( void ) { return m_ScreenWidth; }
   void SetScreenWidth( float ScreenWidth ) { m_ScreenWidth = ScreenWidth; }

   float GetScreenHeight( void ) { return m_ScreenHeight; }
   void SetScreenHeight( float ScreenHeight ) { m_ScreenHeight = ScreenHeight; }

   virtual bool Update( float DeltaTime, CScene* Scene );

   void AddControl( CControl* Control );

   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );
};
}

#endif // GUI_H