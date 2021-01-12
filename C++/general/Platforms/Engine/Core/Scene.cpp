#include "Scene.h"

namespace RTS
{
   CScene::CScene( void )
   {
      m_RenderTimes = 0;
      m_CS = 0;
      m_MaxCode = 65535;
   }

   CScene::~CScene( void )
   {
      Reset();
   }
}