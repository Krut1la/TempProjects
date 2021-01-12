#include "ScriptUtils.h"
#include <assert.h>

namespace General
{
/*void TestLua( void )
{
   CLuaVM vm;
   vm.InitialiseVM ();   

   CLuaDebugger dbg (vm);
   dbg.SetCount (10);

   CResourcesScript ms (vm);
   ms.CompileFile ("test.lua");

   ms.SelectScriptFunction ("CountAndCall");
   ms.AddParam (2);
   ms.Go ();

   ms.SelectScriptFunction ("divideMe");
   ms.AddParam (33);
   ms.Go (1);

   ms.SelectScriptFunction ("returnToMe");
   ms.AddParam (10);
   ms.Go (0);
}*/

static int printMessage (lua_State *lua)
{
   assert (lua_isstring (lua,1));

   const char *msg = lua_tostring (lua, 1);

   // get caller
   lua_Debug ar;
   memset (&ar, 0, sizeof(ar));
   lua_getstack (lua, 1, &ar);
   lua_getinfo (lua, "Snl", &ar);

   // debug output
   const char *str = ar.source;
   printf ("script: %s -- at %s(%d)\n", msg, str, ar.currentline);
   return 0;
}

CLuaVM::CLuaVM (void) : m_pState (NULL), m_pDbg (NULL)
{
   m_fIsOk = false;
}

CLuaVM::~CLuaVM (void)
{
   if (m_pState != NULL)
   {
      lua_close (m_pState);
   }
}

void CLuaVM::Panic (lua_State *lua)
{
}

bool CLuaVM::InitialiseVM (void)
{
   // Open Lua!
   if (Ok ()) DestroyVM ();

   m_pState = lua_open ();

   if (m_pState) 
   {
      m_fIsOk = true;

      // Load util libs into lua
     luaL_openlibs (m_pState);

      // setup global printing (trace)
      lua_pushcclosure (m_pState, printMessage, 0);
      lua_setglobal (m_pState, "trace");

      lua_atpanic (m_pState, (lua_CFunction) CLuaVM::Panic);

      return true;
   }

   return false;
}

bool CLuaVM::DestroyVM (void)
{
   if (m_pState)
   {
      lua_close (m_pState);
      m_pState = NULL;
      m_fIsOk = false;
   }
   return true;
}

bool CLuaVM::RunFile (const char *strFilename)
{
   bool fSuccess = false;
   int iErr = 0;

   if ((iErr = luaL_loadfile (m_pState, strFilename)) == 0)
   {
      // Call main...
       if ((iErr = lua_pcall (m_pState, 0, LUA_MULTRET, 0)) == 0)
       {
          fSuccess = true;
       }
   }

   if (fSuccess == false)
   {
      if (m_pDbg != NULL) m_pDbg->ErrorRun (iErr);
   }

   return fSuccess;
}

bool CLuaVM::RunBuffer (const unsigned char *pbBuffer, size_t szLen, const char *strName /* = NULL */)
{
   bool fSuccess = false;
   int iErr = 0;

   if (strName == NULL)
   {
      strName = "Temp";
   }

   if ((iErr = luaL_loadbuffer (m_pState, (const char *) pbBuffer, szLen, strName)) == 0)
   {
      // Call main...
       if ((iErr = lua_pcall (m_pState, 0, LUA_MULTRET, 0)) == 0)
       {
          fSuccess = true;
       }
   }

   if (fSuccess == false)
   {
      if (m_pDbg != NULL) m_pDbg->ErrorRun (iErr);
   }

   return fSuccess;   

}

bool CLuaVM::CallFunction (int nArgs, int nReturns /* = 0 */)
{
   bool fSuccess = false;
   
   if (lua_isfunction (m_pState, -nArgs-1))
   {
      int iErr = 0;
      iErr = lua_pcall (m_pState, nArgs, nReturns, 0);

      if (iErr == 0)
      {
         fSuccess = true;
      }
      else
      {
         if (m_pDbg != NULL) m_pDbg->ErrorRun (iErr);
      }
   }

   return fSuccess;
}

static void LuaHookCall (lua_State *lua)
{
   
   printf ("---- Call Stack ----\n");
//   printf ("[Level] [Function] [# args] [@line] [src]\n");

   lua_Debug ar;
   
   // Look at call stack
   for (int iLevel = 0; lua_getstack (lua, iLevel, &ar) != 0; ++iLevel)
   {
      if (lua_getinfo (lua, "Snlu", &ar) != 0)
      {
         printf ("%d %s %s %d @%d %s\n", iLevel, ar.namewhat, ar.name, ar.nups, ar.linedefined, ar.short_src);
      }
   }
}

static void LuaHookRet (lua_State *lua)
{
   
}

static void LuaHookLine (lua_State *lua)
{
   lua_Debug ar;
   lua_getstack (lua, 0, &ar);
   

   if (lua_getinfo (lua, "Sl", &ar) != 0)
   {
      printf ("exe %s on line %d\n", ar.short_src, ar.currentline);
   }
}

static void LuaHookCount (lua_State *lua)
{
   LuaHookLine (lua);
}

static void LuaHook (lua_State *lua, lua_Debug *ar)
{
   // Handover to the correct hook
   switch (ar->event)
   {
   case LUA_HOOKCALL:
      LuaHookCall (lua);
      break;
   case LUA_HOOKRET:
   case LUA_HOOKTAILRET:
      LuaHookRet (lua);
      break;
   case LUA_HOOKLINE:
      LuaHookLine (lua);
      break;
   case LUA_HOOKCOUNT:
      LuaHookCount (lua);
      break;
   }
}

CLuaDebugger::CLuaDebugger (CLuaVM& vm) : m_iCountMask (10), m_vm (vm)
{
   // Clear all current hooks
   if (vm.Ok ())
   {
      vm.AttachDebugger (this);
      lua_sethook ((lua_State *) vm, LuaHook, 0, m_iCountMask);
   }
}

CLuaDebugger::~CLuaDebugger (void)
{
   // Clear all current hooks
   if (m_vm.Ok ())
   {
      lua_sethook ((lua_State *) m_vm, LuaHook, 0, m_iCountMask);
   }
}

void CLuaDebugger::SetHooksFlag (int iMask)
{
   // Set hooks
   lua_sethook ((lua_State *) m_vm, LuaHook, iMask, m_iCountMask);
}

void CLuaDebugger::ErrorRun (int iErrorCode)
{
   switch (iErrorCode)
   {
   case LUA_ERRRUN:
      printf ("LUA_ERRRUN\n");
      break;
   case LUA_ERRMEM:
      printf ("LUA_ERRMEM\n");
      break;
   case LUA_ERRERR:
      printf ("LUA_ERRERR\n");
      break;
   }

   // Get the error string that appears on top of stack when a function
   // fails to run
   printf ("Error: %s\n", lua_tostring ((lua_State *) m_vm, -1));
}

static int LuaCallback (lua_State *lua)
{
   // Locate the psudo-index for the function number
   int iNumberIdx = lua_upvalueindex (1);
   int nRetsOnStack = 0;

   bool fSuccess = false;

   // Check for the "this" table
   if (lua_istable (lua, 1))
   {
      // Found the "this" table. The object pointer is at the index 0
      lua_rawgeti (lua, 1, 0);
      
      if (lua_islightuserdata (lua, -1))
      {
         // Found the pointer, need to cast it
         CLuaScript *pThis = (CLuaScript *) lua_touserdata (lua, -1);

         // Get the method index
         int iMethodIdx = (int) lua_tonumber (lua, iNumberIdx);

         // Check that the method is correct index
         assert (!(iMethodIdx > pThis->methods ()));

         // Reformat the stack so our parameters are correct
         // Clean up the "this" table
         lua_remove (lua, 1);
         // Clean up the pThis pointer
         lua_remove (lua, -1);

         // Call the class
         nRetsOnStack = pThis->ScriptCalling (pThis->vm (), iMethodIdx);

         fSuccess = true;
      }
   }

   if (fSuccess == false)
   {
      lua_pushstring (lua, "LuaCallback -> Failed to call the class function");
      lua_error (lua);
   }

   // Number of return variables
   return nRetsOnStack;
}


CLuaScript::CLuaScript (CLuaVM& vm)
 : m_vm (vm), m_nMethods (0), m_iThisRef (0), m_nArgs (0)
{
   BEGIN_LUA_CHECK (vm)
      // Create a reference to the "this" table. Each reference is unique
      lua_newtable (state);
      m_iThisRef = luaL_ref (state, LUA_REGISTRYINDEX);

      // Save the "this" table to index 0 of the "this" table
      CLuaRestoreStack rs (vm);
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);
      lua_pushlightuserdata (state, (void *) this);
      lua_rawseti (state, -2, 0);
   END_LUA_CHECK
}

CLuaScript::~CLuaScript (void)
{
   CLuaRestoreStack rs (m_vm);

   BEGIN_LUA_CHECK (m_vm)
      // Get the reference "this" table
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);

      // Clear index 0
      lua_pushnil (state);
      lua_rawseti (state, -2, 0);
   END_LUA_CHECK
}

bool CLuaScript::CompileBuffer (unsigned char *pbBuffer, size_t szLen)
{
   assert (pbBuffer != NULL && "CLuaScript::CompileBuffer ->  pbBuffer == NULL");
   assert (szLen != 0 && "CLuaScript::CompileBuffer -> szLen == 0");
   assert (m_vm.Ok () && "VM Not OK");

   // Make sure we have the correct "this" table
   CLuaThis luaThis (m_vm, m_iThisRef);

   return m_vm.RunBuffer (pbBuffer, szLen);
}

bool CLuaScript::CompileFile (const char *strFilename)
{
   assert (strFilename != NULL && "CLuaScript::CompileFile -> strFilename == NULL");
   assert (m_vm.Ok () && "VM Not OK");

   // Make sure we have the correct "this" table
   CLuaThis luaThis (m_vm, m_iThisRef);

   return m_vm.RunFile (strFilename);
}

int CLuaScript::RegisterFunction (const char *strFuncName)
{
   assert (strFuncName != NULL && "CLuaScript::RegisterFunction -> strFuncName == NULL");
   assert (m_vm.Ok () && "VM Not OK");

   int iMethodIdx = -1;

   CLuaRestoreStack rs (m_vm);

   BEGIN_LUA_CHECK (m_vm)
      iMethodIdx = ++m_nMethods;

      // Register a function with the lua script. Added it to the "this" table
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);

      // Push the function and parameters
      lua_pushstring (state, strFuncName);
      lua_pushnumber (state, (lua_Number) iMethodIdx);
      lua_pushcclosure (state, LuaCallback, 1);
      lua_settable (state, -3);

   END_LUA_CHECK

   return iMethodIdx;
}

bool CLuaScript::SelectScriptFunction (const char *strFuncName)
{
   assert (strFuncName != NULL && "CLuaScript::SelectScriptFunction -> strFuncName == NULL");
   assert (m_vm.Ok () && "VM Not OK");

   bool fSuccess = true;

   BEGIN_LUA_CHECK (m_vm)
      // Look up function name
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);
      lua_pushstring (state, strFuncName);
      lua_rawget (state, -2);
      lua_remove (state, -2);

      // Put the "this" table back
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);

      // Check that we have a valid function
      if (!lua_isfunction (state, -2))
      {
         fSuccess = false;
         lua_pop (state, 2);
      }
      else
      {
         m_nArgs = 0;
         m_strFunctionName = strFuncName;
      }
   END_LUA_CHECK
   
   return fSuccess;
}

bool CLuaScript::ScriptHasFunction (const char *strScriptName)
{
   assert (strScriptName != NULL && "CLuaScript::ScriptHasFunction -> strScriptName == NULL");
   assert (m_vm.Ok () && "VM Not OK");

   CLuaRestoreStack rs (m_vm);

   bool fFoundFunc = false;

   BEGIN_LUA_CHECK (m_vm)
      lua_rawgeti (state, LUA_REGISTRYINDEX, m_iThisRef);
      lua_pushstring (state, strScriptName);
      lua_rawget (state, -2);
      lua_remove (state, -2);

      if (lua_isfunction (state, -1))
      {
         fFoundFunc = true;
      }
   END_LUA_CHECK

   return fFoundFunc;
}

void CLuaScript::AddParam (char *string)
{
   assert (string != NULL && "CLuaScript::AddParam -> string == NULL");
   assert (m_vm.Ok () && "VM Not OK");

   BEGIN_LUA_CHECK (m_vm)
      lua_pushstring (state, string);
      ++m_nArgs;
   END_LUA_CHECK
}

void CLuaScript::AddParam (int iInt)
{
   assert (m_vm.Ok () && "VM Not OK");

   BEGIN_LUA_CHECK (m_vm)
      lua_pushnumber (state, (lua_Number) iInt);
      ++m_nArgs;
   END_LUA_CHECK
}

void CLuaScript::AddParam (float fFloat)
{
   assert (m_vm.Ok () && "VM Not OK");

   BEGIN_LUA_CHECK (m_vm)
      lua_pushnumber (state, (lua_Number) fFloat);
      ++m_nArgs;
   END_LUA_CHECK
}

bool CLuaScript::Go (int nReturns /* = 0 */)
{
   assert (m_vm.Ok () && "VM Not OK");

   // At this point there should be a parameters and a function on the
   // Lua stack. Each function get a "this" parameter as default and is
   // pushed onto the stack when the method is selected

   bool fSuccess = m_vm.CallFunction (m_nArgs + 1, nReturns);

   if (fSuccess == true && nReturns > 0)
   {
      // Check for returns
      HandleReturns (m_vm, m_strFunctionName);
      lua_pop ((lua_State *) m_vm, nReturns);
   }

   return fSuccess;
}
}