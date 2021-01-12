#ifndef LABEL_H
#define LABEL_H

#include "Types.h"
#include "Control.h"

namespace RTS
{
class CLabel: public CControl
{
private:
protected:
   UINT32      m_ModelIndex;
public:
   CLabel( void* GUI );
   virtual ~CLabel( void );

};
}

#endif // LABEL_H