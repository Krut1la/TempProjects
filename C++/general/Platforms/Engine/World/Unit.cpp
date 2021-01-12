
#include "Unit.h"
#include "3DUtils.h"

namespace RTS
{
CUnit::CUnit( void* World ):
m_WorldRef( World )
{
   m_Time = 0.0f;

   m_Position = VECTOR3F( 0.0f, 0.0f, 0.0f );
   m_Scale = VECTOR3F( 0.0f, 0.0f, 0.0f );
   m_Rotation = VECTOR3F( 0.0f, 0.0f, 0.0f );
}

CUnit::~CUnit( void )
{
}

bool CUnit::Update( float DeltaTime, CScene* Scene )
{
   m_Time += DeltaTime;

   return true;
}

void CUnit::ClearResourceRefs( void )
{
   
}

bool CUnit::HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam )
{
   return true;
}

void CUnit::AddUnit( CUnit* Unit )
{
//   m_SubControls.push_back( Control );
}
}