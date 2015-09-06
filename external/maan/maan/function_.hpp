#ifndef __FUNCTION__HPP
#define __FUNCTION__HPP

#include <functional>

#include "detail/lift.hpp"
#include "detail/__gc.hpp"
#include "create.hpp"
#include "convert.hpp"

namespace maan{
    namespace detail{
        template<typename...ArgsT>
        std::tuple<ArgsT...> get_args(lua_State* L){
            return std::forward_as_tuple(get_LuaValue<ArgsT>(L)...);
        }

        template<class T>
        struct Functor;

        template<class R, typename...ArgsT>
        struct Functor<R(ArgsT...)>{
            static int call(lua_State* L){
                typedef std::function<R(ArgsT...)> F;

                F func = *static_cast<F*>(lua_touserdata(L, lua_upvalueindex(1)));

                auto result = lift(func, get_args<ArgsT...>(L));

                push_LuaValue(L, result);

                return 1;
            }
        };

        template<class T>
        std::function<T>* create_LuaFunction(lua_State* L, std::function<T> func){
            typedef std::function<T> F;
            F* luaFunc = create_LuaObject<F>(L, func);  //..., userdata
            lua_newtable(L);                            //..., userdata, table
            lua_pushvalue(L, -1);                       //..., userdata, table, table
            lua_setmetatable(L, -3);                    //..., userdata, table

            lua_pushstring(L, "__gc");                  //..., userdata, table, "__gc"
            lua_pushcfunction(L, __gc<F>);              //..., userdata, table, "__gc", __gc<T>
            lua_rawset(L, -3);                          //..., userdata, table
            lua_pop(L, 1);                              //..., userdata

            return luaFunc;
        }
    }

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, R (&func)(ArgsT...)){
        detail::create_LuaFunction<T>(L, func);
        lua_pushcclosure(L, detail::Functor<T>::call, 1);
        lua_setglobal(L, name);
    }

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, std::function<R(ArgsT...)> func){
        detail::create_LuaFunction<T>(L, func);
        lua_pushcclosure(L, detail::Functor<T>::call, 1);
        lua_setglobal(L, name);
    }
}

#endif
