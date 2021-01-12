#ifndef FONT_H
#define FONT_H

#include "Types.h"
#include "FontUtils.h"

namespace RTS
{
class CFont
{
private:
protected:
   std::wstring      m_FileName;
   RWORD            m_FontSize;

   RASTER_CHAR_MAP      m_RasterCharMap;

   RWORD            m_TextureID;

   RASTER            m_Raster;

   bool GenerateFont( void );

public:
   CFont( const std::wstring& FileName, 
           RWORD FontSize );
   virtual ~CFont( void );

   bool CreateString( const std::wstring& Str, RECT_LIST& TexCoords );

   RASTER GetRaster( void ) { return m_Raster; }

   RWORD GetTextureID( void ) { return m_TextureID; }
   void SetTextureID( RWORD TextureID ) { m_TextureID = TextureID; }
   RASTER_CHAR GetRasterChar( const wchar_t Char );
};

typedef std::vector<CFont*> FONT_LIST;
}

#endif // FONT_H