#include "bindings.h"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include "World.h"


USING_NS_HIME;

static int lua_sprite_create(lua_State* L)
{
    int argc = lua_gettop(L);
    if (argc >= 5) {
        uint32_t texId = (uint32_t)luaL_checknumber(L, 1);
        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);
        float w = luaL_checknumber(L, 4);
        float h = luaL_checknumber(L, 5);
        Id entity = World::sprite_create(texId, glm::vec4(x, y, w, h));
        lua_pushnumber(L, entity);
        return 1;
    }
    return 0;
}

static int lua_entity_setPosition(lua_State* L)
{
    int argc = lua_gettop(L);
    if (argc >= 3) {
        Id id = (Id)luaL_checknumber(L, 1);
        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);
        World::entity_setPostion(id, glm::vec2(x, y));
    }
    return 0;
}

static int lua_entity_getPosition(lua_State* L)
{
    int argc = lua_gettop(L);
    if (argc >= 1) {
        Id id = (Id)luaL_checknumber(L, 1);
        auto& position = World::entity_getPosition(id);
        lua_pushnumber(L, position.x);
        lua_pushnumber(L, position.y);
        return 2;
    }
    return 0;
}

static int lua_entity_setScale(lua_State* L)
{
    int argc = lua_gettop(L);
    if (argc >= 2) {
        Id id = (Id)luaL_checknumber(L, 1);
        float s = luaL_checknumber(L, 2);
        World::entity_setScale(id, s, s);
    }
    return 0;
}

static int lua_entity_setRotation(lua_State* L)
{
    int argc = lua_gettop(L);
    if (argc >= 2) {
        Id id = (Id)luaL_checknumber(L, 1);
        float r = luaL_checknumber(L, 2);
        World::entity_setRotation(id, r);
    }
    return 0;
}

static const struct luaL_Reg hime2d_lib[] = {
    { "sprite_create", lua_sprite_create },
    { "entity_setPosition", lua_entity_setPosition },
    { "entity_getPosition", lua_entity_getPosition },
    { "entity_setScale", lua_entity_setScale },
    { "entity_setRotation", lua_entity_setRotation },
    { NULL, NULL },
};



int register_hime2d_lib(lua_State *L)
{
    luaL_openlib(L, "hime", hime2d_lib, 0);
    lua_pushlightuserdata(L, World::ffi_setPosition);
    lua_setglobal(L, "_ptr_setPosition");
    lua_pushlightuserdata(L, World::ffi_getPosition);
    lua_setglobal(L, "_ptr_getPosition");
    return 1;
}