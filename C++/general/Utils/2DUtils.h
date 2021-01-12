#ifndef _2DUTILS_H
#define _2DUTILS_H

#include <vector>
#include "Types.h"
#include "SystemUtils.h"

namespace General
{

   typedef struct RASTER
   {
      BYTE*          Data;
      UINT32         Width;
      UINT32         Height;

      RASTER( void ) {
         Data = NULL;
      };
   }RASTER;

   typedef struct RECT
   {
      UINT32    Left;
      UINT32    Top;
      UINT32    Right;
      UINT32    Bottom;

      UINT32 Width( void ) { return Right - Left + 1; }
      UINT32 Height( void ) { return Bottom - Top + 1; }
      RECT Offset( INT32 X, INT32 Y ){
         return RECT( Left + X, Top + Y, Right + X, Bottom + Y );
      }
      RECT( void ){}
      RECT( UINT32 Left, UINT32 Top, UINT32 Right, UINT32 Bottom ){
         this->Left = Left;
         this->Top = Top;
         this->Right = Right;
         this->Bottom = Bottom;
      }
      bool operator==( const RECT& rc2 ) const
      {
         return  ( this->Left ==  rc2.Left ) &&
            ( this->Top ==  rc2.Top ) &&
            ( this->Right ==  rc2.Right ) &&
            ( this->Bottom ==  rc2.Bottom );
      }

      bool operator!=( const RECT& rc2 ) const
      { 
         return !(*this == rc2); 
      }
   } RECT;

   typedef std::vector<RECT> RECT_LIST;

   //bool operator==( const RECT& rc1, const RECT& rc2 );
   //bool operator!=( const RECT& rc1, const RECT& rc2 );

   bool LoadPNG( const std::wstring& FileName, BYTE** Buff, RWORD* Width, RWORD* Height, RWORD* BPP );


   typedef std::vector<RECT> RASTER_LIST;

   typedef enum {
      ratGrid,
      ratAtlas
   }RASTERALLOC_TYPE;

   typedef struct RASTER_NODE {
      bool         HasChildren;
      bool         IsFull;

      RASTER_NODE*   Child[2];
      RECT           Rect;
      UINT32         Id;

      RASTER_NODE( void ) {
         IsFull      = false;
         HasChildren = false;
         Child[0]    = NULL;
         Child[1]    = NULL;
      }
   }RASTER_NODE;

   typedef std::vector<RASTER_NODE*> RASTER_NODE_LIST;

   class CRasterAllocator
   {
   protected:
      RASTERALLOC_TYPE     m_Type;
      UINT32               m_Width;
      UINT32               m_Height;
      RASTER_NODE_LIST     m_Nodes;
      RASTER_NODE*         m_RootNode;
      UINT32               m_RastersCount;

      void CreateRoot( void );
      void ClearNodes( void );

      bool InsertRec( RASTER_NODE* Node, UINT32 Width, UINT32 Height, RECT* Rect );

   public:
      CRasterAllocator( RASTERALLOC_TYPE Type, UINT32 Width, UINT32 Height );
      ~CRasterAllocator( void );

      UINT32 GetRastersCount( void ) { return (UINT32)m_Nodes.size(); }

      bool AddRaster( UINT32 Width, UINT32 Height, RECT* Rect );
      bool IsFull( void ) { return m_RootNode->IsFull; }

      void Clear( void );
   };

}
#endif // _2DUTILS_H