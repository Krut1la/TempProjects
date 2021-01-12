#include "Resources.h"
#include "StringUtils.h"
#include "3DUtils.h"
#include "2DUtils.h"

namespace RTS
{
CResources::CResources( void )
{
}

CResources::~CResources( void )
{
   for( RENDERABLELIST::iterator iRenderable = m_Renderables.begin();  iRenderable != m_Renderables.end(); ++iRenderable)
   {
      delete (*iRenderable);
   }

   m_Renderables.clear();
}

bool CResources::LoadImageF( const std::wstring& FileName, BYTE** Buff, RWORD* Width, RWORD* Height, RWORD* BPP )
{
   return LoadPNG( FileName, Buff, Width, Height, BPP );
}

bool CResources::LoadFont( const std::wstring& FileName, RWORD* FontIndex )
{
   CFont* Font = new CFont( FileName, 22 );

   RASTER Raster = Font->GetRaster();

   RWORD TextureID;

   if ( !CreateTexture( Raster.Width, Raster.Height, tfGrayScale, Raster.Data, &TextureID ) )
   {
      delete Font;

      return false;
   }
      
   Font->SetTextureID( TextureID );

   *FontIndex = m_Fonts.size();

   m_Fonts.push_back( Font );

   return true;
}

bool CResources::LoadMaterial( const std::wstring& FileName, RWORD* MaterialIndex )
{
   return true;
}

bool CResources::CreateTexture( RWORD Width, 
                            RWORD Height,
                           TEX_FORMAT Format,
                           BYTE* Buff,
                           RWORD* TextureIndex )
{
   return true;
}

bool CResources::LoadTexture( const std::wstring& FileName, RWORD* TextureIndex )
{
   return true;
}

bool CResources::LoadPack( const std::wstring& FileName )
{
   CLuaVM vm;
   vm.InitialiseVM ();   

   CLuaDebugger dbg (vm);
   dbg.SetCount (10);

   CResourcesScript ms (vm, this);
   ms.CompileFile (WideToString(FileName).c_str());

   ms.SelectScriptFunction ("Load");
   ms.AddParam (2);
   ms.Go ();

   return true;
}

bool CResources::LoadModel( const std::wstring& FileName, RWORD* ModelIndex  )
{
   if ( FileName == L"BasketBall" )
   {
      CModel* Model = new CModel( this );

//      DETAIL* Frame = new DETAIL();
      DETAIL* Frame = CreateSphere( VECTOR3F( 0.0f, 0.0f, 0.0f ), 0.1f );

/*      Frame->Mesh.Vertices = new VERTEX_MODEL[4];

      //1
      Frame->Mesh.Vertices[0].P.X = -0.03f;
      Frame->Mesh.Vertices[0].P.Y = -0.03f;
      Frame->Mesh.Vertices[0].P.Z = 0.5f;

      Frame->Mesh.Vertices[0].N.X = 0.0f;
      Frame->Mesh.Vertices[0].N.Y = 0.0f;
      Frame->Mesh.Vertices[0].N.Z = 1.0f;

      Frame->Mesh.Vertices[0].U = 0.0f;
      Frame->Mesh.Vertices[0].V = 0.0f;

      //2
      Frame->Mesh.Vertices[1].P.X = 0.03f;
      Frame->Mesh.Vertices[1].P.Y = -0.03f;
      Frame->Mesh.Vertices[1].P.Z = 0.5f;

      Frame->Mesh.Vertices[1].N.X = 0.0f;
      Frame->Mesh.Vertices[1].N.Y = 0.0f;
      Frame->Mesh.Vertices[1].N.Z = 1.0f;

      Frame->Mesh.Vertices[1].U = 1.0f;
      Frame->Mesh.Vertices[1].V = 0.0f;

      //3
      Frame->Mesh.Vertices[2].P.X = 0.03f;
      Frame->Mesh.Vertices[2].P.Y = 0.03f;
      Frame->Mesh.Vertices[2].P.Z = 0.5f;

      Frame->Mesh.Vertices[2].N.X = 0.0f;
      Frame->Mesh.Vertices[2].N.Y = 0.0f;
      Frame->Mesh.Vertices[2].N.Z = 1.0f;

      Frame->Mesh.Vertices[2].U = 1.0f;
      Frame->Mesh.Vertices[2].V = 1.0f;

      //4
      Frame->Mesh.Vertices[3].P.X = -0.03f;
      Frame->Mesh.Vertices[3].P.Y = 0.03f;
      Frame->Mesh.Vertices[3].P.Z = 0.5f;

      Frame->Mesh.Vertices[3].N.X = 0.0f;
      Frame->Mesh.Vertices[3].N.Y = 0.0f;
      Frame->Mesh.Vertices[3].N.Z = 1.0f;

      Frame->Mesh.Vertices[3].U = 0.0f;
      Frame->Mesh.Vertices[3].V = 1.0f;

      Frame->Mesh.Indices = new INDEX32[6];

      Frame->Mesh.Indices[0] = 0;
      Frame->Mesh.Indices[1] = 1;
      Frame->Mesh.Indices[2] = 2;
      Frame->Mesh.Indices[3] = 2;
      Frame->Mesh.Indices[4] = 3;
      Frame->Mesh.Indices[5] = 0;

      Frame->Mesh.IndicesCount = 6;
      Frame->Mesh.VerticesCount = 4;
      Frame->Mesh.PrimitivesCount = 2;
      Frame->Mesh.PrimitiveType = ptTriangleList;
*/
      Frame->MaterialIndex = 4;


      Model->m_Details.push_back( Frame );


      *ModelIndex = m_Renderables.size();
      m_Renderables.push_back( Model );
   }

   if ( FileName == L"Room" )
   {
      CModel* Model = new CModel( this );

      DETAIL* Frame = new DETAIL();

      Frame->Mesh.Vertices = new VERTEX_MODEL[8];

      //1
      Frame->Mesh.Vertices[0].P.X = -1.0f;
      Frame->Mesh.Vertices[0].P.Y = -1.0f;
      Frame->Mesh.Vertices[0].P.Z = 0.0f;

      Frame->Mesh.Vertices[0].N.X = 0.0f;
      Frame->Mesh.Vertices[0].N.Y = 0.0f;
      Frame->Mesh.Vertices[0].N.Z = 1.0f;

      Frame->Mesh.Vertices[0].U = 0.0f;
      Frame->Mesh.Vertices[0].V = 0.0f;

      //2
      Frame->Mesh.Vertices[1].P.X = 1.0f;
      Frame->Mesh.Vertices[1].P.Y = -1.0f;
      Frame->Mesh.Vertices[1].P.Z = 0.0f;

      Frame->Mesh.Vertices[1].N.X = 0.0f;
      Frame->Mesh.Vertices[1].N.Y = 0.0f;
      Frame->Mesh.Vertices[1].N.Z = 1.0f;

      Frame->Mesh.Vertices[1].U = 1.0f;
      Frame->Mesh.Vertices[1].V = 0.0f;

      //3
      Frame->Mesh.Vertices[2].P.X = 1.0f;
      Frame->Mesh.Vertices[2].P.Y = 1.0f;
      Frame->Mesh.Vertices[2].P.Z = 0.0f;

      Frame->Mesh.Vertices[2].N.X = 0.0f;
      Frame->Mesh.Vertices[2].N.Y = 0.0f;
      Frame->Mesh.Vertices[2].N.Z = 1.0f;

      Frame->Mesh.Vertices[2].U = 1.0f;
      Frame->Mesh.Vertices[2].V = 1.0f;

      //4
      Frame->Mesh.Vertices[3].P.X = -1.0f;
      Frame->Mesh.Vertices[3].P.Y = 1.0f;
      Frame->Mesh.Vertices[3].P.Z = 0.0f;

      Frame->Mesh.Vertices[3].N.X = 0.0f;
      Frame->Mesh.Vertices[3].N.Y = 0.0f;
      Frame->Mesh.Vertices[3].N.Z = 1.0f;

      Frame->Mesh.Vertices[3].U = 0.0f;
      Frame->Mesh.Vertices[3].V = 1.0f;

      //5
      Frame->Mesh.Vertices[4].P.X = -1.0f;
      Frame->Mesh.Vertices[4].P.Y = -1.0f;
      Frame->Mesh.Vertices[4].P.Z = 2.0f;

      Frame->Mesh.Vertices[4].N.X = 0.0f;
      Frame->Mesh.Vertices[4].N.Y = 0.0f;
      Frame->Mesh.Vertices[4].N.Z = 1.0f;

      Frame->Mesh.Vertices[4].U = 0.0f;
      Frame->Mesh.Vertices[4].V = 0.0f;

      //6
      Frame->Mesh.Vertices[5].P.X = 1.0f;
      Frame->Mesh.Vertices[5].P.Y = -1.0f;
      Frame->Mesh.Vertices[5].P.Z = 2.0f;

      Frame->Mesh.Vertices[5].N.X = 0.0f;
      Frame->Mesh.Vertices[5].N.Y = 0.0f;
      Frame->Mesh.Vertices[5].N.Z = 1.0f;

      Frame->Mesh.Vertices[5].U = 1.0f;
      Frame->Mesh.Vertices[5].V = 0.0f;

      //7
      Frame->Mesh.Vertices[6].P.X = 1.0f;
      Frame->Mesh.Vertices[6].P.Y = 1.0f;
      Frame->Mesh.Vertices[6].P.Z = 2.0f;

      Frame->Mesh.Vertices[6].N.X = 0.0f;
      Frame->Mesh.Vertices[6].N.Y = 0.0f;
      Frame->Mesh.Vertices[6].N.Z = 1.0f;

      Frame->Mesh.Vertices[6].U = 1.0f;
      Frame->Mesh.Vertices[6].V = 1.0f;

      //8
      Frame->Mesh.Vertices[7].P.X = -1.0f;
      Frame->Mesh.Vertices[7].P.Y = 1.0f;
      Frame->Mesh.Vertices[7].P.Z = 2.0f;

      Frame->Mesh.Vertices[7].N.X = 0.0f;
      Frame->Mesh.Vertices[7].N.Y = 0.0f;
      Frame->Mesh.Vertices[7].N.Z = 1.0f;

      Frame->Mesh.Vertices[7].U = 0.0f;
      Frame->Mesh.Vertices[7].V = 1.0f;

      Frame->Mesh.Indices = new INDEX32[24];

      Frame->Mesh.Indices[0] = 0;
      Frame->Mesh.Indices[1] = 1;
      Frame->Mesh.Indices[2] = 1;
      Frame->Mesh.Indices[3] = 2;
      Frame->Mesh.Indices[4] = 2;
      Frame->Mesh.Indices[5] = 3;
      Frame->Mesh.Indices[6] = 3;
      Frame->Mesh.Indices[7] = 0;
      Frame->Mesh.Indices[8] = 4;
      Frame->Mesh.Indices[9] = 5;
      Frame->Mesh.Indices[10] = 5;
      Frame->Mesh.Indices[11] = 6;

      Frame->Mesh.Indices[12] = 6;
      Frame->Mesh.Indices[13] = 7;
      Frame->Mesh.Indices[14] = 7;
      Frame->Mesh.Indices[15] = 4;
      Frame->Mesh.Indices[16] = 0;
      Frame->Mesh.Indices[17] = 4;
      Frame->Mesh.Indices[18] = 1;
      Frame->Mesh.Indices[19] = 5;
      Frame->Mesh.Indices[20] = 2;
      Frame->Mesh.Indices[21] = 6;
      Frame->Mesh.Indices[22] = 3;
      Frame->Mesh.Indices[23] = 7;

      Frame->Mesh.IndicesCount = 24;
      Frame->Mesh.VerticesCount = 8;
      Frame->Mesh.PrimitivesCount = 12;
      Frame->Mesh.PrimitiveType = ptLineList;

      Frame->MaterialIndex = 0;


      Model->m_Details.push_back( Frame );


      *ModelIndex = m_Renderables.size();
      m_Renderables.push_back( Model );
   }

   return true;
}

bool CResources::LoadSprite( const std::wstring& FileName, RWORD* SpriteIndex  )
{
   CSprite* Sprite = new CSprite( this );

   if ( FileName == L"cursor" )
   {
      Sprite->m_MaterialIndex = 1;
      Sprite->m_Vetrices[0].U = 0.0;
      Sprite->m_Vetrices[0].V = 0.0;
      Sprite->m_Vetrices[1].U = 1.0;
      Sprite->m_Vetrices[1].V = 0.0;
      Sprite->m_Vetrices[2].U = 1.0;
      Sprite->m_Vetrices[2].V = 1.0;
      Sprite->m_Vetrices[3].U = 0.0;
      Sprite->m_Vetrices[3].V = 1.0;

   }

   if ( FileName == L"bg" )
      Sprite->m_MaterialIndex = 2;

   if ( FileName == L"zazaka" )
   {
      Sprite->m_MaterialIndex = 2;
      Sprite->m_Vetrices[0].U = 0.0;
      Sprite->m_Vetrices[0].V = 0.0;
      Sprite->m_Vetrices[1].U = 1.0;
      Sprite->m_Vetrices[1].V = 0.0;
      Sprite->m_Vetrices[2].U = 1.0;
      Sprite->m_Vetrices[2].V = 1.0;
      Sprite->m_Vetrices[3].U = 0.0;
      Sprite->m_Vetrices[3].V = 1.0;
   }

   *SpriteIndex = m_Renderables.size();
   m_Renderables.push_back( Sprite );

   return true;
}



/*void CResources::NotifyProvidables( void )
{
   for( PROVIDABLELIST::iterator iProvidable = f_Providables.begin();  
   iProvidable != f_Providables.end(); ++iProvidable)
   {
      (*iProvidable)->ClearResourceRefs();
   }
}*/

IRenderable* CResources::GetRenderableRef( RWORD RenderableIndex )
{
   // Return dummy model
   if ( RenderableIndex + 1 > m_Renderables.size() )
      return m_Renderables[0];

   return m_Renderables[RenderableIndex];
}

CMaterial* CResources::GetMaterialRef( RWORD MaterialIndex )
{
   // Return dummy material
   if ( MaterialIndex + 1 > m_Materials.size() )
      return m_Materials[0];

   return m_Materials[MaterialIndex];
}

CFont* CResources::GetFontRef( RWORD FontIndex )
{
   // Return default font
   if ( FontIndex + 1 > m_Fonts.size() )
      return m_Fonts[0];

   return m_Fonts[FontIndex];
}


}