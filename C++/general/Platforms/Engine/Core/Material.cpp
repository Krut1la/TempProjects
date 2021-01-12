#include "Material.h"

namespace RTS
{
CMaterial::CMaterial( void* Resources ):
m_ResourcesRef( Resources )
{
}

CMaterial::~CMaterial( void )
{
}

bool CMaterial::Set( void )
{
   return true;
}

bool CMaterial::Reset( void )
{
   return true;
}
}