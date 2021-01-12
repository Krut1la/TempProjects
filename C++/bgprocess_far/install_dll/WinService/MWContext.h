// MWContext.h: interface for the CMWContext class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MWCONTEXT_H
#define MWCONTEXT_H

#include "ServiceContext.h"

class CMWContext : public CServiceContext  
{
public:
	CMWContext();
	virtual ~CMWContext();
	void OnServiceStart();
	void OnServiceStop();
};

#endif // MWCONTEXT_H
