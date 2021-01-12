#ifndef SCRIPTUTILS_H
#define SCRIPTUTILS_H

#include <string>
#include "Types.h"

#define BEGIN_LUA_CHECK(vm)   lua_State *state = (lua_State *) vm; \
                              if (vm.Ok ()) { 
#define END_LUA_CHECK         }

extern "C"
{
#include "Lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
}

namespace General
{
enum
{
   DBG_MASK_CALL = LUA_MASKCALL,
   DBG_MASK_RET = LUA_MASKRET,
   DBG_MASK_LINE = LUA_MASKLINE,
   DBG_MASK_COUNT = LUA_MASKCOUNT
};

class CLuaVM;

class CLuaDebugger
{
public:
   CLuaDebugger (CLuaVM& vm);
   virtual ~CLuaDebugger (void);

   void SetHooksFlag (int iMask);
   void SetCount (int iCount) { m_iCountMask = iCount; }

   void ErrorRun (int iErrorCode);

protected:
   int m_iCountMask;
   CLuaVM& m_vm;
};

class CLuaVM
{
public:
   CLuaVM (void);
   virtual ~CLuaVM (void);

   bool InitialiseVM (void);
   bool DestroyVM (void);

   // Load and run script elements
   bool RunFile (const char *strFilename);
   bool RunBuffer (const unsigned char *pbBuffer, size_t szLen, const char *strName = NULL);

   // C-Api into script
   bool CallFunction (int nArgs, int nReturns = 0);

   // Get the state of the lua stack (use the cast operator)
   //lua_State *GetState (void) { return m_pState; }
   operator lua_State *(void) { return m_pState; }

   static void Panic (lua_State *lua);

   // Check if the VM is OK and can be used still
   virtual bool Ok (void) { return m_fIsOk; }

   // For debugging
   void AttachDebugger (CLuaDebugger *dbg) { m_pDbg = dbg; }

protected:
   lua_State *m_pState;
   bool m_fIsOk;
   CLuaDebugger *m_pDbg;
};

class CLuaThis
{
public:
   CLuaThis (CLuaVM& vm, int iRef) : m_iOldRef (0), m_vm (vm)
   {
      lua_State *state = (lua_State *) vm;
      if (vm.Ok ())
      {
         // Save the old "this" table
         lua_getglobal (state, "this");
         m_iOldRef = luaL_ref (state, LUA_REGISTRYINDEX);

         // replace it with our new one
         lua_rawgeti(state, LUA_REGISTRYINDEX, iRef);
         lua_setglobal (state, "this");
      }
   }

   virtual ~CLuaThis (void)
   {
      lua_State *state = (lua_State *) m_vm;
      if (m_iOldRef > 0 && m_vm.Ok ())
      {
         // Replace the old "this" table
         lua_rawgeti(state, LUA_REGISTRYINDEX, m_iOldRef);
         lua_setglobal (state, "this");
         luaL_unref (state, LUA_REGISTRYINDEX, m_iOldRef);
      }
   }


protected:
   int m_iOldRef;
   CLuaVM& m_vm;
};

class CLuaRestoreStack
{
public:
   CLuaRestoreStack (CLuaVM& vm) : m_pState (NULL)
   {
      m_pState = (lua_State *) vm;
      if (vm.Ok ())
      {
         m_iTop = lua_gettop (m_pState);
      }
   }

   virtual ~CLuaRestoreStack (void)
   {
      lua_settop (m_pState, m_iTop);
   }

protected:
   lua_State *m_pState;
   int m_iTop;
};

class CLuaScript
{
public:
   CLuaScript (CLuaVM& vm);
   virtual ~CLuaScript (void);

   // Compile script into Virtual Machine
   bool CompileFile (const char *strFilename);
   bool CompileBuffer (unsigned char *pbBuffer, size_t szLen);

   // Register function with Lua
   int RegisterFunction (const char *strFuncName);

   // Selects a Lua Script function to call
   bool SelectScriptFunction (const char *strFuncName);
   void AddParam (int iInt);
   void AddParam (float fFloat);
   void AddParam (char *string);

   // Runs the loaded script
   bool Go (int nReturns = 0);

   // Checks on Virtual Machine script
   bool ScriptHasFunction (const char *strScriptName);

   // Method indexing check
   int methods (void) { return m_nMethods; }
   

   // When the script calls a class method, this is called
   virtual int ScriptCalling (CLuaVM& vm, int iFunctionNumber) = 0;

   // When the script function has returns
   virtual void HandleReturns (CLuaVM& vm, const char *strFunc) = 0;

   CLuaVM& vm (void) { return m_vm; }

protected:
   int m_nMethods;
   CLuaVM& m_vm;
   int m_iThisRef;
   int m_nArgs;
   const char *m_strFunctionName;
};


void TestLua( void );
}
#endif // SCRIPTUTILS_H
