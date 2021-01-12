#ifndef SCENE_H
#define SCENE_H

#include "Types.h"
#include <assert.h>
#include <memory.h>

namespace RTS
{
   // Render commands
   const RWORD   RENDER_STATIC_SPRITE   = 1001;
   const RWORD   RENDER_STATIC_MODEL      = 1002;

   class CScene
   {
      RWORD               m_MaxCode;
      RWORD               m_Code[65535];
      RWORD               m_CS;
   public:
      RWORD               m_RenderTimes;

      CScene( void );
      ~CScene( void );

      bool IsEmpty( void ) { return m_CS == 0; }
      RWORD GetCS( void ) { return m_CS; }

      void Reset( void ){ m_CS = 0; }

      void Push( const VOID_PTR Object, RWORD Size ){
         assert( ( m_CS < m_MaxCode ) );

         memcpy( &m_Code[m_CS], Object, Size );
         m_CS += Size;
      }

      VOID_PTR Pop( RWORD Size ){
         assert( ( m_CS > 0 ) );

         m_CS -= Size;
         return &m_Code[m_CS];
      }
   };
}

#endif // SCENE_H