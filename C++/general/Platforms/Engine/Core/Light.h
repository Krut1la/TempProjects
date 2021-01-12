#ifndef LIGHT_H
#define LIGHT_H

#include <vector>
#include <vector>
#include "Types.h"
#include "3DTypes.h"
#include "IRenderable.h"

namespace RTS
{
class CLight;

typedef std::list<CLight*> LIGHTLIST;


class CLight: public IRenderable
{
private:
   
protected:

public:

   CLight( void* Resources );

   virtual ~CLight( void );


   virtual bool Render( CRenderer* Renderer );
};
}

#endif // LIGHT_H