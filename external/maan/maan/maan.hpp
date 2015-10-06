#ifndef __MAAN_HPP
#define __MAAN_HPP

#include <functional>

namespace maan{
    template<typename T>
    class class_;

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, R (*func)(ArgsT...));

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, std::function<R(ArgsT...)> func);

    template<class C, class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, R (C::*func)(ArgsT...)const, const C* singleton);

    template<class C, class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, R (C::*func)(ArgsT...), C* singleton);

    struct module_{
        module_(lua_State* L):
            L_(L)
        {
            lua_pushglobaltable(L);
        }

        module_(lua_State* L, const char* name):
            L_(L)
        {
            lua_getglobal(L_, name);
            if(lua_isnil(L_, -1)){
                lua_pop(L_, 1);
                lua_newtable(L_);
                lua_pushvalue(L_, -1);
                lua_setglobal(L_, name);
            }
        }

        //TODO: We probably need a corresponding endnamespace
        //function that pops the table.
        module_& namespace_(const char* name){
            lua_newtable(L_);
            lua_pushvalue(L_, -1);
            lua_setfield(L_, -3, name);
            return *this;
        }

        module_& end_namespace_(void){
            lua_pop(L_, 1);
            return *this;
        }

        template<typename T>
        class_<T> class_(const char* name){
            return maan::class_<T>(this, name);
        }

        template<typename T>
        module_& function_(const char* name, T func){
            maan::function_(L_, name, func);
            return *this;
        }

        template<typename T, typename C>
        module_& function_(const char* name, T func, C singleton){
            maan::function_(L_, name, func, singleton);
            return *this;
        }

        lua_State* L_;
    };
}

#include "function_.hpp"
#include "class_.hpp"

#endif
