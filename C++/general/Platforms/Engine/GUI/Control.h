#ifndef CONTROL_H
#define CONTROL_H

#include "Types.h"
#include <vector>
#include "Font.h"
#include "Mutator.h"
#include "IUpdatable.h"
#include "IProvidable.h"

namespace RTS
{
class CControl;

typedef std::vector<CControl*> CONTROLLIST;
typedef enum { taLeft, taRight, taTop, taBottom, taCenter } TEXTALIGN;

class CControl: public IUpdatable, virtual IProvidable
{
private:
protected:
   void*            m_GUIRef;

   CMutatorTranslate2D*   m_MutatorTranslate2D;

   VECTOR3F         m_Position;
   VECTOR3F         m_Scale;
   VECTOR3F         m_Rotation;
   COLOR            m_Color;

   bool            m_Visible;
   bool            m_Enabled;
   bool            m_HasFocus;
   bool            m_CanHaveFocus;

   std::wstring      m_Text;
   bool            m_Vertical;
   TEXTALIGN         m_Align;
   CFont*         m_FontRef;
   SPRITELIST      m_TextSprites;
   UINT32         m_TextColor;
   bool            m_TextChanged;

   CONTROLLIST      m_SubControls;

   bool UpdateText( CScene* Scene );

   CControl( void* GUI );
public:
   virtual ~CControl( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   VECTOR3F GetPosition( void ) { return m_Position; }
   void SetPosition( const VECTOR3F& Position) { m_Position = Position; }
   VECTOR3F GetScale( void ) { return m_Scale; }
   void SetScale( const VECTOR3F& Scale) { m_Scale = Scale; }
   VECTOR3F GetRotation( void ) { return m_Rotation; }
   void SetRotation( const VECTOR3F& Rotation) { m_Rotation = Rotation; }

   std::wstring GetText( void ) { return m_Text; }
   void SetText( const std::wstring& Text);

   void AddControl( CControl* Control );

   virtual void ClearResourceRefs( void );
};
}

#endif // CONTROL_H