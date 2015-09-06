#ifndef __CREATE_HPP
#define __CREATE_HPP

#include "detail/class_info.hpp"
#include "detail/operator_new.hpp"

namespace maan{
    template<class T, typename ...arg_types>
    T* create_LuaObject(lua_State* L, arg_types ...args){
        T* ret = new(L) T(args...);
        lua_rawgetp(L, LUA_REGISTRYINDEX, detail::ClassInfo<T>::get_metatable_key());
        if(!lua_isnil(L, -1)) lua_setmetatable(L, -2);
        else lua_pop(L, 1);
        return ret;
    }
}

#endif
