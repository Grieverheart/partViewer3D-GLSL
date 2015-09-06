#ifndef __CONVERSIONS_HPP
#define __CONVERSIONS_HPP

#include "detail/class_info.hpp"
#include "detail/operator_new.hpp"

namespace maan{
    namespace detail{
        int typeerror(lua_State *L, int narg, const char *expected_name, const char *got_name){
            const char *msg = lua_pushfstring(L, "%s expected, got %s", expected_name, got_name);
            return luaL_argerror(L, narg, msg);
        }

        template<typename T>
        const char* get_class_name(lua_State* L){
            lua_rawgetp(L, LUA_REGISTRYINDEX, ClassInfo<T>::get_metatable_key());
            lua_pushstring(L, "__name");
            lua_rawget(L, -2);
            const char* name = lua_tostring(L, -1);
            lua_pop(L, 2);
            return name;
        }

        const char* get_class_name(lua_State* L, int idx){
            lua_pushstring(L, "__name");
            lua_rawget(L, idx - 1);
            const char* name = lua_tostring(L, -1);
            lua_pop(L, 1);
            return name;
        }
    }

    template<class condition, typename R = void >
    using EnableIf = typename std::enable_if<condition::value, R>::type;

    template<class T>
    EnableIf<std::is_floating_point<T>,
    T> get_LuaValue(lua_State* L){
        T val = lua_tonumber(L, -1);
        lua_pop(L, 1);
        return val;
    }

    template<class T>
    EnableIf<std::is_integral<T>,
    void> push_LuaValue(lua_State* L, T val){
        lua_pushinteger(L, val);
    }


    template<class T>
    EnableIf<std::is_integral<T>,
    T> get_LuaValue(lua_State* L){
        T val = lua_tointeger(L, -1);
        lua_pop(L, 1);
        return val;
    }

    template<class T>
    EnableIf<std::is_floating_point<T>,
    void> push_LuaValue(lua_State* L, T val){
        lua_pushnumber(L, val);
    }


    //TODO: We need to somehow check if the userdata has the correct type
    //One way is to use dynamic cast. Another one is to add a __class tag
    //in the metatable.
    template<
        class T,
        typename type_ = typename std::remove_const<typename std::remove_reference<T>::type>::type
    >
    EnableIf<std::is_class<type_>,
    T> get_LuaValue(lua_State* L){
        //We didn't get a userdatum! Raise error.
        if(!lua_isuserdata(L, -1)){
            const char* expected_name = detail::get_class_name<type_>(L);
            detail::typeerror(L, -1, expected_name, luaL_typename(L, -1));
        }

        //Check if we get userdata of the expected type by comparing the metatables.
        lua_getmetatable(L, -1);
        lua_rawgetp(L, LUA_REGISTRYINDEX, detail::ClassInfo<type_>::get_metatable_key());
        if(!lua_rawequal(L, -1, -2)){
            const char* expected_name = detail::get_class_name(L, -1);
            lua_pop(L, 1);
            const char* got_name = detail::get_class_name(L, -1);
            lua_pop(L, 1);
            detail::typeerror(L, -1, expected_name, got_name);
        }
        lua_pop(L, 2);

        const type_* val = static_cast<const type_*>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        return *val;
    }

    template<class T>
    EnableIf<std::is_class<T>,
    void> push_LuaValue(lua_State* L, const T& val){
        create_LuaObject<T>(L, val);
        lua_rawgetp(L, LUA_REGISTRYINDEX, detail::ClassInfo<T>::get_metatable_key());
        lua_setmetatable(L, -2);
    }


    template<class T>
    EnableIf<std::is_pointer<T>,
    T> get_LuaValue(lua_State* L){
        T val = static_cast<T>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        return val;
    }

    template<class T>
    EnableIf<std::is_pointer<T>,
    void> push_LuaValue(lua_State* L, T val){
        lua_pushuserdata(L, val);
    }
}

#endif
