#ifndef RESOURCES_H
#define RESOURCES_H

#include <map>
#include <string>
#include "Types.h"

#include "ScriptUtils.h"

#include "Model.h"
#include "Font.h"
#include "Material.h"
#include "IProvidable.h"


namespace RTS
{

class CResources
{
private:
protected:
   RENDERABLELIST   m_Renderables;
   MATERIALLIST     m_Materials;
   FONT_LIST        m_Fonts;

   bool LoadImageF( const std::wstring& FileName, 
      BYTE** Buff, 
      RWORD* Width, 
      RWORD* Height, 
      RWORD* BPP );

   virtual bool CreateTexture( RWORD Width, 
      RWORD Height,
      TEX_FORMAT Format,
      BYTE* Buff,
      RWORD* TextureIndex );

   CResources( void );
public:
   virtual ~CResources( void );

   virtual bool LoadPack( const std::wstring& FileName );
   virtual bool LoadFont( const std::wstring& FileName, RWORD* FontIndex );
   virtual bool LoadModel( const std::wstring& FileName, RWORD* ModelIndex );
   virtual bool LoadSprite( const std::wstring& FileName, RWORD* SpriteIndex );
   virtual bool LoadTexture( const std::wstring& FileName, RWORD* TextureIndex );
   virtual bool LoadMaterial( const std::wstring& FileName, RWORD* TextureIndex );

   IRenderable* GetRenderableRef( RWORD RenderableIndex );
   CMaterial* GetMaterialRef( RWORD MaterialIndex );
   CFont* GetFontRef( RWORD FontIndex );
};

class CResourcesScript : public CLuaScript
{
public:
   CResourcesScript (CLuaVM& vm, CResources* Resources): 
   CLuaScript (vm),
   m_ResourcesRef( Resources )
     {
        m_iMethodBase = RegisterFunction ("LoadTexture");
//        RegisterFunction ("hello2");
//        RegisterFunction ("hello3");
     }

     int ScriptCalling (CLuaVM& vm, int iFunctionNumber)
     {
        switch (iFunctionNumber - m_iMethodBase)
        {
        case 0:
           return LoadTexture (vm);
        }

        return 0;
     }

     int LoadTexture (CLuaVM& vm)
     {
//        m_ResourcesRef->LoadTexture   
        return 0;
     }

/*     int Hello2 (CLuaVM& vm)
     {
        lua_State *state = (lua_State *) vm;

        int iNumber = (int) lua_tonumber (state, -1);
        printf ("Hellow (2) -> %d\n", iNumber);
        return 0;
     }

     int Hello3 (CLuaVM& vm)
     {
        lua_State *state = (lua_State *) vm;

        int iNumber = (int) lua_tonumber (state, -1);
        printf ("Hello (3) -> %d\n", iNumber);
        lua_pushnumber (state, iNumber + 2);
        return 1;
     }
*/
     void HandleReturns (CLuaVM& vm, const char *strFunc)
     {
        if (strcmp (strFunc, "divideMe") == 0)
        {
           // frames returns an answer of the stack
           lua_State *state = (lua_State *) vm;
           int itop = lua_gettop (state);
           int iType = lua_type (state, -1);
           const char *s = lua_typename (state, iType);
           double fFrameCount = lua_tonumber (state, -1);

           printf ("frame count = %f\n", fFrameCount);
        }
     }

protected:
   CResources* m_ResourcesRef;
   int m_iMethodBase;
};
}

#endif // RESOURCES_H
