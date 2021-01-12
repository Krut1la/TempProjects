#include "FontUtilsTest.h"

#include <fstream>


void TestRasterAllocator_case6( void )
{
   CRasterAllocator* RasterAllocator = new CRasterAllocator( ratAtlas, 256, 256 );

   bool result;
   _RECT rc;

   result = RasterAllocator->AddRaster( 256, 20, &rc);
   result = RasterAllocator->AddRaster( 256, 236, &rc);

   if ( !result )
      MessageBox( 0, L"TestRasterAllocator_case6_1", L"Failed!", 0 );

   if ( !RasterAllocator->IsFull() )
      MessageBox( 0, L"TestRasterAllocator_case6_1", L"Failed!", 0 );

   delete RasterAllocator;
}

void TestRasterAllocator_case5( void )
{
   CRasterAllocator* RasterAllocator = new CRasterAllocator( ratAtlas, 256, 256 );

   bool result;
   _RECT rc;

   result = RasterAllocator->AddRaster( 256, 20, &rc);
   result = RasterAllocator->AddRaster( 256, 240, &rc);

   if ( result )
      MessageBox( 0, L"TestRasterAllocator_case5", L"Failed!", 0 );

   delete RasterAllocator;
}

void TestRasterAllocator_case4( void )
{
   CRasterAllocator* RasterAllocator = new CRasterAllocator( ratAtlas, 256, 256 );

   bool result;
   _RECT rc;

   result = RasterAllocator->AddRaster( 257, 20, &rc);

   if ( result )
      MessageBox( 0, L"TestRasterAllocator_case4", L"Failed!", 0 );

   delete RasterAllocator;
}

void TestRasterAllocator_case3( void )
{
   CRasterAllocator* RasterAllocator = new CRasterAllocator( ratAtlas, 256, 256 );

   bool result;
   _RECT rc;

   result = RasterAllocator->AddRaster( 10, 20, &rc);
   result = RasterAllocator->AddRaster( 10, 20, &rc);

   if ( !result )
      MessageBox( 0, L"TestRasterAllocator_case3", L"Failed!", 0 );

   delete RasterAllocator;
}

void TestRasterAllocator_case2( void )
{
   CRasterAllocator* RasterAllocator = new CRasterAllocator( ratAtlas, 256, 256 );

   bool result;
   _RECT rc;

   result = RasterAllocator->AddRaster( 256, 256, &rc);

   if ( !result )
      MessageBox( 0, L"TestRasterAllocator_case2_1", L"Failed!", 0 );

   if ( !RasterAllocator->IsFull() )
      MessageBox( 0, L"TestRasterAllocator_case2_2", L"Failed!", 0 );


   delete RasterAllocator;
}

void TestRasterAllocator_case1( void )
{
   CRasterAllocator* RasterAllocator = new CRasterAllocator( ratAtlas, 256, 256 );

   bool result;
   _RECT rc;
   _RECT rc2;

   rc2 = CreateRect( 0, 0, 9, 19 );

   result = RasterAllocator->AddRaster( 10, 20, &rc);

   if ( !result )
      MessageBox( 0, L"TestRasterAllocator_case1_1", L"Failed!", 0 );

   if ( rc != rc2  )
      MessageBox( 0, L"TestRasterAllocator_case1_2", L"Failed!", 0 );


   delete RasterAllocator;
}

void TestRasterAllocator( void )
{
   TestRasterAllocator_case1();
   TestRasterAllocator_case2();
   TestRasterAllocator_case3();
   TestRasterAllocator_case4();
   TestRasterAllocator_case5();
   TestRasterAllocator_case6();
//   TestRasterAllocator_case7();
//   TestRasterAllocator_case8();
}

void TestRenderCharMap_case1( void )
{
   _UINT32 TexSize = 256;

   void* Buff = new _BYTE[TexSize*TexSize];

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
                    L'[', L']', L'{', L'}'};

   RASTER_CHAR_MAP RasterCharList;

   RASTER_CHAR RasterChar;

   for ( _UINT32 iChar = 0; iChar < sizeof( CharMap )/sizeof(wchar_t); ++iChar )
   {
      RasterChar.Char = CharMap[iChar];
      RasterCharList[CharMap[iChar]] = RasterChar;
   }
   
   CRasterAllocator RasterAllocator( ratAtlas, TexSize, TexSize );

   RenderCharMap( L"arial.ttf", 22, TexSize, TexSize, &Buff, RasterCharList, RasterAllocator, 0 );

   for ( RASTER_CHAR_MAP::iterator iRasterChar = RasterCharList.begin(); iRasterChar != RasterCharList.end(); ++iRasterChar )
   {
      if ( !(*iRasterChar).second.Rendered )
      {
         MessageBox( 0, L"TestRenderCharMap_case1", L"Failed!", 0 );
         break;
      }
   }

   std::ofstream ofs;

   ofs.open( L"arial.raw", std::ios::binary);

    if ( ofs.fail() )
   {
      MessageBox( 0, L"TestRenderCharMap_case1: Could not open file", L"Error", 0 );
      return;
   }

   ofs.write ( (char*)Buff, TexSize*TexSize);

   ofs.close();

   delete [] Buff;
}

void TestRenderCharMap( void )
{
   TestRenderCharMap_case1();
}

int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
   TestRasterAllocator();
   TestRenderCharMap();

   return 0;
}

