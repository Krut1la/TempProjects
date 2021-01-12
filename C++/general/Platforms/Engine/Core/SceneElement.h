#ifndef SCENEELEMENT_H
#define SCENEELEMENT_H

#include <list>
#include "Types.h"
#include "3DTypes.h"
#include "IRenderable.h"

namespace RTS
{
typedef enum
{
   taticModel,
   taticSprite,
   rtDynamicModel,
   rtDynamicSprite,
   rtDynamicLight,
   rtTransform
} RENDERABLETYPE;

typedef struct
{
   UINT32         m_StaticRenderableRef;
   void*         m_DynamicRenderableRef;
   RENDERABLETYPE   m_RenderableType;

   void*         m_MutatorRef;

   VECTOR3F      m_Position;
   VECTOR3F      m_Scale;
   VECTOR3F      m_Rotation;

   COLOR         m_Color;
} SCENEELEMENT;

//typedef std::list<SCENEELEMENT> SCENEELEMENTSLIST;
}

#endif // SCENEELEMENT_H