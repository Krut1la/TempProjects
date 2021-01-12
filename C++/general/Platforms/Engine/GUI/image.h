#ifndef IMAGE_H
#define IMAGE_H

#include "Control.h"


namespace RTS
{
class CImage: public CControl
{
private:
protected:
   UINT32         m_ImageModelRef;
public:
   CImage( void* GUI, UINT32 MaterialRef);
   virtual ~CImage( void );

   virtual bool Update( float DeltaTime, CScene* Scene );
   virtual bool HandleInputEvent( UINT32 EventID, INT32 wParam, INT32 lParam );

   virtual void ClearResourceRefs( void );
};
}


#endif // IMAGE_H