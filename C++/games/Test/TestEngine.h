#ifndef TESTENGINE_H
#define TESTENGINE_H

#include "FileLogger.h"
#include "Engine.h"
#include "D3DRenderer.h"
#include "DirectInput.h"
#include "Controller.h"

#include "TestWorld.h"

class CTestEngine: public RTS::CEngine{
protected:
   
public:
    CTestEngine(void);
    virtual ~CTestEngine(void);
    virtual bool Initialize(void);
    virtual void Finalize(void);
   virtual void Run(void);

   virtual bool LoadPAK( const std::wstring& FileName );
};

#endif // TESTENGINE_H