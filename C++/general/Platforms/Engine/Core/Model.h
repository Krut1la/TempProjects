#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <list>
#include <string>
#include "Types.h"
#include "3DTypes.h"
#include "IRenderable.h"

namespace RTS
{
class CModel;
class CSprite;

typedef std::vector<IRenderable*> RENDERABLELIST;
typedef std::list<CModel*> MODELLIST;
typedef std::vector<CSprite*> SPRITELIST;

class CSprite: public IRenderable
{
private:
   
protected:
   void*         m_Resources;

public:
   VERTEX_SPRITE   m_Vetrices[4];
   UINT32         m_MaterialIndex;
   COLOR         m_Color;

   CSprite( void* Resources );

   virtual ~CSprite( void );

   virtual bool Render( CRenderer* Renderer );
};

class CModel: public IRenderable
{
private:
   
protected:
   void*         m_Resources;

   MODELLIST   m_SubModels;
public:
   DETAILLIST      m_Details;

   CModel( void* Resources );

   virtual ~CModel( void );

   void Clear( void );
   void Translate( float X, float Y, float Z);

   virtual bool LoadFromFile( const std::wstring& FileName, UINT32 MaterialIndex  );
   virtual bool Render( CRenderer* Renderer );
};
}

#endif // MODEL_H