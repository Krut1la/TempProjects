#include "FontUtils.h"

#include <fstream>

namespace General
{

bool RenderCharMap( const std::wstring& FileName,
                RWORD FontSize,
               bool      Bold,      
               bool      Italic,
               bool      DropShadow,
               RWORD RasterWidth, 
               RWORD RasterHeight,
               BYTE** RasterMem,
               RASTER_CHAR_MAP& RasterCharList,
               CRasterAllocator& RasterAllocator,
               RWORD   PageID )
{
   if ( ( FileName == L"" ) || ( RasterWidth == 0 ) || ( RasterHeight == 0) )
      return false;

   FT_Library m_library;

    FT_Error err = FT_Init_FreeType(&m_library);

   if ( err != 0 )
      return false;

   RWORD font_mem_size;

   // Read ttf into memory
   std::ifstream ifs;

   ifs.open(FileName.c_str(), std::ios::binary);

    if ( ifs.fail() )
      return false;

   ifs.seekg(0, std::ios::end);
   font_mem_size = ifs.tellg();
   ifs.seekg(0, std::ios::beg);

   FT_Byte* font_mem = new FT_Byte[font_mem_size];

   ifs.read ( (char*)font_mem, font_mem_size);

   ifs.close();

   // Create Freetype face
   FT_Face        face;

    err = FT_New_Memory_Face( m_library, 
                       (const FT_Byte*)font_mem, 
                       (FT_Long)font_mem_size, 
                       0, 
                       &face);

   if ( err != 0 )
   {
      delete [] font_mem;
      FT_Done_FreeType( m_library );
      return false;
   }

   FT_Set_Char_Size( face, (FT_F26Dot6)FontSize << 6, (FT_F26Dot6)FontSize << 6, 96, 96);

    // set default encoding
    if ( !face->charmap ) 
      FT_Set_Charmap (face, face->charmaps[0]) ;

   for ( RASTER_CHAR_MAP::iterator iRasterChar = RasterCharList.begin(); iRasterChar != RasterCharList.end(); ++iRasterChar )
   {
      FT_Glyph glyph;

      FT_Load_Glyph( face, FT_Get_Char_Index( face, (*iRasterChar).second.Char ), 0 ) ;
      FT_Get_Glyph( face->glyph, &glyph );

      if ( glyph == 0 ) 
         continue;

      FT_BBox bbox ;
      FT_Glyph_Get_CBox ( glyph, FT_GLYPH_BBOX_PIXELS, &bbox ) ;

      FT_Render_Mode mode = FT_RENDER_MODE_NORMAL;

      if ( glyph->format != FT_GLYPH_FORMAT_BITMAP )
      {
         err = FT_Glyph_To_Bitmap( &glyph, mode, 0, 1 ) ;

         if ( err != 0 )
         {
            FT_Done_Glyph( glyph ) ;
            continue;
         }
      }

      FT_BitmapGlyph bitmap = reinterpret_cast<FT_BitmapGlyph> ( glyph ) ;
      FT_Bitmap*     source = &bitmap->bitmap ;

      RECT rc;

      if ( RasterAllocator.AddRaster( source->width, source->rows, &rc ) )
      {
         (*iRasterChar).second.PageID = PageID;
         (*iRasterChar).second.Rect = rc;
         if ( (*iRasterChar).second.Char != L' ' )
         {
            (*iRasterChar).second.HoriOffsetX = -face->glyph->metrics.horiBearingX/64;
            (*iRasterChar).second.HoriOffsetY = -face->glyph->metrics.horiBearingY/64;
            (*iRasterChar).second.VertOffsetX = -face->glyph->metrics.vertBearingX/64;
            (*iRasterChar).second.VertOffsetY = -face->glyph->metrics.vertBearingY/64;
         }
         else
         {
            (*iRasterChar).second.HoriOffsetX = -(INT32)FontSize;
            (*iRasterChar).second.HoriOffsetY = -(INT32)FontSize;
            (*iRasterChar).second.VertOffsetX = (INT32)FontSize;
            (*iRasterChar).second.VertOffsetY = (INT32)FontSize;
         }
         (*iRasterChar).second.Rendered = true;

         for ( RWORD iRow = 0; iRow < (UINT32)source->rows; ++iRow)
            memcpy( ( (BYTE*)*RasterMem + rc.Left + RasterWidth*( rc.Top + iRow) ), 
                  ( source->buffer + iRow*source->width ), 
                  source->width );
      }

      if ( RasterAllocator.IsFull() )
         break;


      FT_Done_Glyph (glyph) ;
   }

   FT_Done_Face( face );

   delete [] font_mem;
   FT_Done_FreeType( m_library );

   return true;
}

}
