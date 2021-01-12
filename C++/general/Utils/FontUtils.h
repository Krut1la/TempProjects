#ifndef FONTUTILS_H
#define FONTUTILS_H

#include <string>
#include <vector>
#include <map>
#include "Types.h"
#include "2DUtils.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace General
{

   typedef struct RASTER_CHAR
   {
      wchar_t Char;
      RECT    Rect;
      INT32   HoriOffsetX;
      INT32   HoriOffsetY;
      INT32   VertOffsetX;
      INT32   VertOffsetY;
      RWORD   PageID;
      bool    Rendered;

      RASTER_CHAR( void )   {
         HoriOffsetX = 0;
         HoriOffsetY = 0;
         VertOffsetX = 0;
         VertOffsetY = 0;
         Rendered = false;
      }
   }RASTER_CHAR;

   typedef std::map<wchar_t, RASTER_CHAR> RASTER_CHAR_MAP;

   bool RenderCharMap( const std::wstring& FileName,
      RWORD FontSize,
      bool  Bold,      
      bool  Italic,
      bool  DropShadow,
      RWORD RasterWidth, 
      RWORD RasterHeight,
      BYTE** RasterMem,
      RASTER_CHAR_MAP& RasterCharList,
      CRasterAllocator& RasterAllocator,
      RWORD   PageID );
}

#endif // FONTUTILS_H