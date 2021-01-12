#include "2DUtils.h"
#include "StringUtils.h"
#include "png.h"


namespace General
{

CRasterAllocator::CRasterAllocator( RASTERALLOC_TYPE Type, UINT32 Width, UINT32 Height ):
m_Type( Type ),
m_Width( Width ),
m_Height( Height ),
m_RastersCount( 0 )
{
   CreateRoot();
}

CRasterAllocator::~CRasterAllocator( void )
{
   ClearNodes();
}

void CRasterAllocator::CreateRoot( void )
{
   m_RootNode = new RASTER_NODE;
   m_RootNode->Rect = RECT( 0, 0, m_Width - 1, m_Height - 1 );
   m_Nodes.push_back( m_RootNode );
}

void CRasterAllocator::ClearNodes( void )
{
   for ( RASTER_NODE_LIST::iterator iRasterNode = m_Nodes.begin(); iRasterNode != m_Nodes.end(); ++iRasterNode )
      delete *iRasterNode;

   m_Nodes.clear();
}

void CRasterAllocator::Clear( void )
{
   m_RastersCount = 0;

   ClearNodes();
   CreateRoot();
}

bool CRasterAllocator::InsertRec( RASTER_NODE* Node, UINT32 Width, UINT32 Height, RECT* Rect )
{
   // Check if the node already has a raster
   if ( Node->IsFull )
      return false;

   // Try to insert into one from childeren
   if ( Node->HasChildren )
   {
      if ( InsertRec( Node->Child[0], Width, Height, Rect ) )
         return true;
      else if ( InsertRec( Node->Child[1], Width, Height, Rect ) )
      {
         // If children are full then the whole raster is full
         if ( ( Node->Child[0]->IsFull ) && ( Node->Child[1]->IsFull ) )
            Node->IsFull = true;

         return true;
      }
      else
         return false;
   }

   // Check if the raster suits
   if ( ( Width > Node->Rect.Width() ) || ( Height > Node->Rect.Height() ) )
      return false;

   // Check if the raster suits perfectly
   if ( ( Width == Node->Rect.Width() ) && ( Height == Node->Rect.Height() ) )
   {
      Node->IsFull = true;
      Node->Id = m_RastersCount;
      m_RastersCount++;

      *Rect = Node->Rect;

      return true;
   }

   // Create childs
   Node->Child[0] = new RASTER_NODE;
   m_Nodes.push_back( Node->Child[0] );

   Node->Child[1] = new RASTER_NODE;
   m_Nodes.push_back( Node->Child[1] );

   Node->HasChildren = true;

   // Choose how to split
   UINT32   dw = Node->Rect.Width() - Width;
   UINT32   dh = Node->Rect.Height() - Height;

   if ( dw > dh ) 
   {
      Node->Child[0]->Rect = RECT( Node->Rect.Left, 
         Node->Rect.Top, 
         Node->Rect.Left + Width - 1, 
         Node->Rect.Bottom );

      Node->Child[1]->Rect = RECT( Node->Rect.Left + Width, 
         Node->Rect.Top, 
         Node->Rect.Right, 
         Node->Rect.Bottom );
   }
   else
   {
      Node->Child[0]->Rect = RECT( Node->Rect.Left, 
         Node->Rect.Top, 
         Node->Rect.Right, 
         Node->Rect.Top + Height - 1 );

      Node->Child[1]->Rect = RECT( Node->Rect.Left, 
         Node->Rect.Top + Height, 
         Node->Rect.Right, 
         Node->Rect.Bottom );
   }

   return InsertRec( Node->Child[0], Width, Height, Rect );
}

bool CRasterAllocator::AddRaster( UINT32 Width, UINT32 Height, RECT* Rect )
{
   return InsertRec ( m_RootNode, Width, Height, Rect );
}


bool LoadPNG( const std::wstring& FileName, BYTE** Buff, RWORD* Width, RWORD* Height, RWORD* BPP )
{
   FILE *fp = NULL;
   char header[8];

   fopen_s(&fp, WideToString( FileName ).c_str(), "rb");
   if (!fp)
   {
      return false;
   }

   fread(header, 1, 8, fp);
   int is_png = !png_sig_cmp((png_bytep)header, 0, 8);
   if (!is_png)
   {
      return false;
   }

   png_structp png_ptr = png_create_read_struct
      (PNG_LIBPNG_VER_STRING, NULL,
      NULL, NULL);
   if (!png_ptr)
      return false;

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
   {
      png_destroy_read_struct(&png_ptr,
         (png_infopp)NULL, (png_infopp)NULL);
      return false;
   }

   png_init_io(png_ptr, fp);
   png_set_sig_bytes(png_ptr, 8);

   png_read_info(png_ptr, info_ptr);

   *Buff = new BYTE[info_ptr->height*info_ptr->width*(info_ptr->pixel_depth >> 3)];

   png_bytepp row_pointers = new png_bytep[info_ptr->height];
   for (UINT32 i=0; i<info_ptr->height; i++)
   {
      row_pointers[i] = (png_bytep)((*Buff) + i*info_ptr->width*(info_ptr->pixel_depth >> 3));
   }

   png_set_rows(png_ptr, info_ptr, row_pointers);

   png_set_bgr(png_ptr);
   png_read_image(png_ptr, row_pointers);

   delete row_pointers;

   *Width = info_ptr->width;
   *Height = info_ptr->height;
   *BPP = info_ptr->pixel_depth;

   png_destroy_read_struct(&png_ptr, &info_ptr,
      (png_infopp)NULL);

   fclose(fp);

   return true;
}

}