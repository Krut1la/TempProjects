#ifndef D3DMUTATOR_H
#define D3DMUTATOR_H

#include "Mutator.h"
#ifdef USE_DX9
#include <d3d9.h>
#include <d3dx9.h>
#else
#include <d3d8.h>
#include <d3dx8.h>
#endif


namespace RTS
{
class CD3DMutatorTranslate2D: public CMutatorTranslate2D
{
private:
protected:
   D3DXMATRIX   m_OldWorld;
public:
   CD3DMutatorTranslate2D( void );
   virtual ~CD3DMutatorTranslate2D( void );

   virtual bool Mutate( const SCENEELEMENT& SceneElement,
                  IRenderable* Renderable, 
                  CRenderer* Renderer );
   virtual bool Mutate( CMutator* Mutator, CRenderer* Renderer );

   virtual bool DeMutate( const SCENEELEMENT& SceneElement,
                  IRenderable* Renderable, 
                  CRenderer* Renderer );
};
}

#endif // D3DMUTATOR_H