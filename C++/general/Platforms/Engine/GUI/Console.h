#ifndef CONTROL_H
#define CONTROL_H

#include "Types.h"
#include <vector>
#include "IUpdatable.h"

namespace RTS
{
class CControl;

typedef std::vector<CControl*> CONTROLLIST;

class CControl: public IUpdatable
{
private:
protected:

   CONTROLLIST      m_SubControls;

   CControl( void );
public:
   virtual ~CControl( void );

   virtual bool Update( UINT32 DeltaTime );
};
}


#endif // CONTROL_H