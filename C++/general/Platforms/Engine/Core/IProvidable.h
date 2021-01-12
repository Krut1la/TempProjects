#ifndef IPROVIDABLE_H
#define IPROVIDABLE_H

#include <vector>

namespace RTS
{
class IProvidable;

typedef std::vector<IProvidable*> PROVIDABLELIST;

class IProvidable
{
public:
   virtual void ClearResourceRefs( void ) = 0;
};
}

#endif // IPROVIDABLE_H