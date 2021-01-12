#ifndef D3DMODEL_H
#define D3DMODEL_H

#include "Model.h"
#include "D3DRenderer.h"

namespace RTS
{
class CD3DSprite: public CSprite
{
private:
protected:
public:

   CD3DSprite( void* Resources );

   virtual ~CD3DSprite( void );

   virtual bool Render( CRenderer* Renderer );
};

class CD3DModel: public CModel
{
private:
   
protected:


public:
   CD3DModel( void* Resources );

   virtual ~CD3DModel( void );

   virtual bool Render( CRenderer* Renderer );
};
}

#endif // D3DMODEL_H