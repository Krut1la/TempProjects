#ifndef MUTATOR_H
#define MUTATOR_H

#include <vector>
#include "Types.h"
#include "Renderer.h"
#include "Model.h"
#include "SceneElement.h"
#include "IUpdatable.h"
#include "IProvidable.h"

namespace RTS
{
class CMutator;

typedef std::vector<CMutator*> MUTATORLIST;


class CMutator
{
private:
protected:
public:
   virtual bool Mutate( const SCENEELEMENT& SceneElement,
                  IRenderable* Renderable, 
                  CRenderer* Renderer ) = 0;

   virtual bool Mutate( CMutator* Mutator, CRenderer* Renderer  ) = 0;

   virtual bool DeMutate( const SCENEELEMENT& SceneElement, 
                  IRenderable* Renderable, 
                  CRenderer* Renderer ) = 0;
};

class CMutatorTranslate2D: public CMutator
{
private:
protected:
   float*      m_X;
   float*      m_Y;
   float*      m_Scale;
   float*      m_Rotation;

   float         m_SavedX;
   float         m_SavedY;
   float         m_SavedScale;
   float         m_SavedRotation;

   float      m_TargetWidth;      
   float      m_TargetHeight;
   float      m_DeviceWidth;      
   float      m_DeviceHeight;

   CMutatorTranslate2D( void );
public:

   void SetXY( float* X, float* Y ) { m_X = X; m_Y = Y; }
   void SetScale( float* Scale ) { m_Scale = Scale; }
   void SetRotation( float* Rotation ) { m_Rotation = Rotation; }

   void SetTargetWidth( float TargetWidth ) { m_TargetWidth = TargetWidth; }
   void SetTargetHeight( float TargetHeight ) { m_TargetHeight = TargetHeight; }
   void SetDeviceWidth( float DeviceWidth ) { m_DeviceWidth = DeviceWidth; }
   void SetDeviceHeight( float DeviceHeight ) { m_DeviceHeight = DeviceHeight; }

   virtual ~CMutatorTranslate2D( void );
};
}

#endif // MUTATOR_H