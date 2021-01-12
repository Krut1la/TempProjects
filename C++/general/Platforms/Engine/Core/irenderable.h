#ifndef IRENDERABLE_H
#define IRENDERABLE_H

namespace RTS
{
class CRenderer;

class IRenderable
{
public:
   virtual bool Render( CRenderer* Renderer ) = 0;
};
}

#endif // IRENDERABLE_H