#ifndef __CREATE_HPP
#define __CREATE_HPP

#include "detail/class_info.hpp"
#include "detail/operator_new.hpp"
#include "detail/__gc.hpp"

namespace maan{
    template<class T, typename ...arg_types>
    T* create_LuaObject(lua_State* L, arg_types ...args){
        T* ret = new(L) T(args...);
        lua_rawgetp(L, LUA_REGISTRYINDEX, detail::ClassInfo<T>::get_metatable_key());
        if(!lua_isnil(L, -1)) lua_setmetatable(L, -2);
        else lua_pop(L, 1);
        return ret;
    }

    template<class T, typename ...arg_types>
    T* create_LuaGCObject(lua_State* L, arg_types ...args){
        auto object = create_LuaObject<T>(L, args...); //..., userdata
        lua_newtable(L);                               //..., userdata, table

        lua_pushstring(L, "__gc");                     //..., userdata, table, "__gc"
        lua_pushcfunction(L, detail::__gc<T>);         //..., userdata, table, "__gc", __gc<T>
        lua_rawset(L, -3);                             //..., userdata, table

        lua_setmetatable(L, -2);                       //..., userdata, table

        return object;
    }

    template<class T>
    T* create_LuaGCObject(lua_State* L, const T& arg){
        auto object = create_LuaObject<T>(L, arg);     //..., userdata
        lua_newtable(L);                               //..., userdata, table

        lua_pushstring(L, "__gc");                     //..., userdata, table, "__gc"
        lua_pushcfunction(L, detail::__gc<T>);         //..., userdata, table, "__gc", __gc<T>
        lua_rawset(L, -3);                             //..., userdata, table

        lua_setmetatable(L, -2);                       //..., userdata, table

        return object;
    }
}

#endif
