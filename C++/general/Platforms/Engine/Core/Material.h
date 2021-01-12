#ifndef MATERIAL_H
#define MATERIAL_H

#include "Types.h"
#include <vector>

namespace RTS
{
class CMaterial;

typedef std::vector<CMaterial*> MATERIALLIST;

class CMaterial
{
private:
   
protected:
   void*      m_ResourcesRef;

   CMaterial( void* Resources );
public:

   virtual ~CMaterial( void );

   virtual bool Set( void );
   virtual bool Reset( void );
};
}

#endif // MATERIAL_H