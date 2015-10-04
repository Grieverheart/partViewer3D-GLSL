#ifndef __FUNCTION__HPP
#define __FUNCTION__HPP

#include "detail/lift.hpp"
#include "detail/__gc.hpp"
#include "create.hpp"
#include "convert.hpp"
#include "detail/score_args.hpp"

namespace maan{
    namespace detail{
        //TODO: Perhaps we should change the get_LuaValue functions to accept an
        //index. This would mean we need a way to index the arguments in the
        //function below.
        //namespace shit{
        //    template<class Sequence, typename...ArgsT>
        //    struct get_args;

        //    template<template<int...> class Sequence, int...Is, typename...ArgsT>
        //    struct get_args<Sequence<Is...>, ArgsT...>{
        //        static std::tuple<ArgsT...> get(lua_State* L){
        //            return std::forward_as_tuple(get_LuaValue<ArgsT>(L, Is)...);
        //        }
        //    };
        //}

        //template<typename...ArgsT>
        //std::tuple<ArgsT...> get_args(lua_State* L){
        //    return shit::get_args<typename detail::generator<sizeof...(ArgsT)>::type, ArgsT...>::get(L);
        //}

        template<typename...ArgsT>
        std::tuple<ArgsT...> get_args(lua_State* L){
            return std::forward_as_tuple(get_LuaValue<ArgsT>(L)...);
        }

        struct Functor{
            Functor(void):
                next_(nullptr)
            {}

            virtual ~Functor(void){
                delete next_;
            }

            virtual int call(lua_State* L) = 0;
            virtual int score(lua_State* L) = 0;

            Functor* get_next(void){
                return next_;
            }

            void set_next(Functor* next){
                next_ = next;
            }

            Functor* next_;
        };

        static int call_overloadable_functor(lua_State* L){
            auto func = static_cast<Functor*>(lua_touserdata(L, lua_upvalueindex(1)));
            decltype(func) max_func;
            int max = 0;
            do{
                int score = func->score(L);
                if(score > max){
                    max = score;
                    max_func = func;
                }
            }while((func = func->get_next()));
            if(max > 0) return max_func->call(L);
            //TODO: Here we should raise an error.
            return 0;
        }

        template<class T>
        struct OverloadableFunctor;

        //TODO: Perhaps we could somehow add a next pointer to chain overloads.
        template<class R, typename...ArgsT>
        struct OverloadableFunctor<R(ArgsT...)>: Functor{
            using F = std::function<R(ArgsT...)>;
            OverloadableFunctor(F func):
                Functor(), func_(func)
            {}

            int call(lua_State* L){
                push_LuaValue(L, lift(func_, get_args<ArgsT...>(L)));
                return 1;
            }

            int score(lua_State* L){
                return detail::score_args<ArgsT...>(L);
            }

            F func_;
            static const int n_args_ = sizeof...(ArgsT);
        };

        template<typename...ArgsT>
        struct OverloadableFunctor<void(ArgsT...)>: Functor{
            using F = std::function<void(ArgsT...)>;
            OverloadableFunctor(F func):
                Functor(), func_(func)
            {}

            int call(lua_State* L){
                lift(func_, get_args<ArgsT...>(L));
                return 0;
            }

            int score(lua_State* L){
                return detail::score_args<ArgsT...>(L);
            }

            F func_;
            static const int n_args_ = sizeof...(ArgsT);
        };
    }

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, R (*func)(ArgsT...)){
        using F = detail::OverloadableFunctor<T>;

        lua_getglobal(L, name);
        if(!lua_isnil(L, -1)){
            lua_getupvalue(L, -1, 1);
            auto base_functor = static_cast<detail::Functor*>(lua_touserdata(L, -1));
            lua_pop(L, 2);
            while(base_functor->get_next()){
                base_functor = base_functor->get_next();
            }
            base_functor->set_next(new F(std::function<T>(func)));
        }
        else{
            lua_pop(L, 1);
            create_LuaGCObject<F>(L, std::function<T>(func));
            lua_pushcclosure(L, detail::call_overloadable_functor, 1);
            lua_setglobal(L, name);
        }
    }

    template<class R, typename...ArgsT, typename T = R(ArgsT...)>
    void function_(lua_State* L, const char* name, std::function<R(ArgsT...)> func){
        using F = detail::OverloadableFunctor<T>;

        lua_getglobal(L, name);
        if(!lua_isnil(L, -1)){
            lua_getupvalue(L, -1, 1);
            auto base_functor = static_cast<detail::Functor*>(lua_touserdata(L, -1));
            lua_pop(L, 2);
            while(base_functor->get_next()){
                base_functor = base_functor->get_next();
            }
            base_functor->set_next(new F(func));
        }
        else{
            lua_pop(L, 1);
            create_LuaGCObject<F>(L, func);
            lua_pushcclosure(L, detail::call_overloadable_functor, 1);
            lua_setglobal(L, name);
        }
    }
}

#endif
