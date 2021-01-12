#include "Font.h"
#include <fstream>

namespace RTS
{
CFont::CFont( const std::wstring& FileName, 
               RWORD FontSize ):
m_FileName( FileName ),
m_FontSize( FontSize )
{
   GenerateFont();
}

CFont::~CFont( void )
{
}

bool CFont::GenerateFont( void )
{
   // TODO: Try to detect optimal size
   m_Raster.Width = 512;
   m_Raster.Height = 512;

   m_Raster.Data = new BYTE[m_Raster.Width*m_Raster.Width];

   wchar_t CharMap[] = { L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H',
                    L'I', L'J', L'K', L'L', L'M', L'N', L'O', L'P',
                    L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X', 
                    L'Y', L'Z',
                    L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h', 
                    L'i', L'j', L'k', L'l', L'm', L'n', L'o', L'p', 
                    L'q', L'r', L's', L't', L'u', L'v', L'w', L'x', 
                    L'y', L'z',
                    L'À', L'Á', L'Â', L'Ã', L'Ä', L'Å', L'¨', L'Æ', 
                    L'Ç', L'È', L'É', L'Ê', L'Ë', L'Ì', L'Í', L'Î', 
                    L'Ï', L'Ð', L'Ñ', L'Ò', L'Ó', L'Ô', L'Õ', L'Ö', 
                    L'×', L'Ø', L'Ù', L'Ü', L'Û', L'Ú', L'Ý', L'Þ', 
                    L'ß',
                    L'à', L'á', L'â', L'ã', L'ä', L'å', L'¸', L'æ', 
                    L'ç', L'è', L'é', L'ê', L'ë', L'ì', L'í', L'î', 
                    L'ï', L'ð', L'ñ', L'ò', L'ó', L'ô', L'õ', L'ö', 
                    L'÷', L'ø', L'ù', L'ü', L'û', L'ú', L'ý', L'þ', 
                    L'ÿ',
                    L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', 
                    L'9', L'0', 
                    L'~', L'!', L'@', L'#', L'$', L'%', L'^', L'&',
                    L'*', L'(', L')', L'_', L'+', L'=', L'-', L'`',
                    L',', L'.', L'/', L';', L'\'', L'\"', L'\\', L'|',
                    L'[', L']', L'{', L'}', L' '};
                    
//   wchar_t CharMap[] = { L' ' };

   RASTER_CHAR RasterChar;

   for ( RWORD iChar = 0; iChar < sizeof( CharMap )/sizeof( wchar_t ); ++iChar )
   {
      RasterChar.Char = CharMap[iChar];
      m_RasterCharMap[CharMap[iChar]] = RasterChar;
   }
   
   CRasterAllocator RasterAllocator( ratAtlas, m_Raster.Width, m_Raster.Height );
   
   if ( !RenderCharMap( m_FileName, 
                   m_FontSize, 
                   false,
                   false,
                   false,
                   m_Raster.Width, 
                   m_Raster.Height, 
                   &m_Raster.Data, 
                   m_RasterCharMap, 
                   RasterAllocator, 
                   0 ) )
      return false;

      std::ofstream ofs;

#ifdef _DEBUG
   ofs.open( (m_FileName + L".raw").c_str(), std::ios::binary);

   ofs.write ( (char*)m_Raster.Data, m_Raster.Width*m_Raster.Height);

   ofs.close();
#endif

   return true;
}

RASTER_CHAR CFont::GetRasterChar( const wchar_t Char )
{
   return m_RasterCharMap[Char];
}


}