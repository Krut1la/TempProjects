#include "Room.h"
#include "Resources.h"
#include "World.h"
#include "Engine.h"
#include "3DUtils.h"

namespace RTS
{
CRoom::CRoom( void* World ):
CUnit( World )
{
   m_RoomModelRef = 4;
}

CRoom::~CRoom( void )
{
}

bool CRoom::Update( float DeltaTime, CScene* Scene )
{
   if ( !CUnit::Update( DeltaTime, Scene ) )
      return false;

/*   SCENEELEMENT SceneElement;

   SceneElement.m_StaticRenderableRef = m_RoomModelRef;
   SceneElement.m_DynamicRenderableRef = NULL;
   SceneElement.m_RenderableType = taticModel;
   SceneElement.m_MutatorRef = NULL;
   SceneElement.m_Position = m_Position;
   SceneElement.m_Scale = m_Scale;
   SceneElement.m_Rotation = m_Rotation;
   SceneElement.m_Color = 0xFFFFFFFF;

   Scene->AddElement( SceneElement );
*/
   return true;
}

void CRoom::ClearResourceRefs( void )
{

}

bool CRoom::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}
}