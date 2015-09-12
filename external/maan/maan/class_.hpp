#ifndef __CLASS_CREATOR_HPP
#define __CLASS_CREATOR_HPP

#include "detail/lift.hpp"
#include "detail/class_info.hpp"
#include "detail/__gc.hpp"
#include "detail/score_args.hpp"
#include "create.hpp"
#include "function_.hpp"

namespace maan{
    int lua_ClassProperty(lua_State* L){
        return 0;
    }

    namespace detail{
        template<class T, typename...ArgsT>
        struct OverloadableConstructor: Functor{
            OverloadableConstructor(void):
                Functor()
            {}

            int call(lua_State* L){
                lift(create_LuaObject<T, ArgsT...>, L, detail::get_args<ArgsT...>(L));
                return 1;
            }

            int score(lua_State* L){
                return detail::score_args<ArgsT...>(L);
            }

            static const int n_args_ = sizeof...(ArgsT);
        };
    }

#define DEF_BINARY_OPERATOR(name_, _op_)\
    struct name_{\
        template<class T, class U>\
        static auto execute(T&& a, U&& b) -> decltype(a _op_ b) {\
            return a _op_ b;\
        }\
\
        static constexpr const char* name = "__"#name_;\
    }

    DEF_BINARY_OPERATOR(add, +);
    DEF_BINARY_OPERATOR(sub, -);
    DEF_BINARY_OPERATOR(mul, *);
    DEF_BINARY_OPERATOR(div, /);
    DEF_BINARY_OPERATOR(eq, ==);
    DEF_BINARY_OPERATOR(lt, <);
    DEF_BINARY_OPERATOR(gt, >);
    DEF_BINARY_OPERATOR(le, <=);
    DEF_BINARY_OPERATOR(ge, >=);

    template<class T>
    class class_{
    public:
        using type_ = T;

        ~class_(void){
            //Since we had the metatable on top of the stack all the time,
            //we have to pop it
            lua_pop(L_, 1);
        }

        class_(lua_State* L, const char* name):
            name_(name), L_(L)
        {
            lua_newtable(L_);
            lua_pushvalue(L_, -1);
            lua_rawsetp(L_, LUA_REGISTRYINDEX, detail::ClassInfo<T>::get_metatable_key());

            lua_pushstring(L_, "__index");
            lua_pushcfunction(L_, __index);
            lua_rawset(L_, -3);

            lua_pushstring(L_, "__newindex");
            lua_pushcfunction(L_, __newindex);
            lua_rawset(L_, -3);

            lua_pushstring(L, "__gc");
            lua_pushcfunction(L, detail::__gc<T>);
            lua_rawset(L, -3);

            lua_pushstring(L_, "__class_id");
            lua_pushlightuserdata(L_, const_cast<void*>(detail::ClassInfo<T>::get_metatable_key()));
            lua_rawset(L_, -3);

            lua_pushstring(L_, "__name");
            lua_pushstring(L_, name_);
            lua_rawset(L_, -3);
        }

        template<typename ...ArgsT>
        class_& def_constructor(void){
            using F = detail::OverloadableConstructor<type_, ArgsT...>;

            lua_getglobal(L_, name_);
            if(!lua_isnil(L_, -1)){
                lua_getupvalue(L_, -1, 1);
                auto base_functor = static_cast<detail::Functor*>(lua_touserdata(L_, -1));
                lua_pop(L_, 2);
                while(base_functor->get_next()){
                    base_functor = base_functor->get_next();
                }
                base_functor->set_next(new F());
            }
            else{
                lua_pop(L_, 1);
                create_LuaGCObject<F>(L_);
                lua_pushcclosure(L_, detail::call_overloadable_functor, 1);
                lua_setglobal(L_, name_);
            }

            return *this;
        }

        template<typename R, typename ...ArgsT, typename F = R(ArgsT...)>
        class_& def(const char* name, R (type_::*fun_ptr)(ArgsT...)){
            std::function<std::function<F>(type_*)> temp = [=](type_* object){
                return [fun_ptr, object](ArgsT... args){
                    return (object->*fun_ptr)(args...);
                };
            };

            luaL_Reg funcs[] = {
                {name, call_member<R, ArgsT...>::func},
                {NULL, NULL}
            };

            create_LuaGCObject(L_, temp);
            luaL_setfuncs(L_, funcs, 1);

            return *this;
        }

        template<typename R, typename ...ArgsT, typename F = R(ArgsT...)>
        class_& def(const char* name, R (type_::*fun_ptr)(ArgsT...)const){
            std::function<std::function<F>(type_*)> temp = [=](const type_* object){
                return [fun_ptr, object](ArgsT... args){
                    return (object->*fun_ptr)(args...);
                };
            };

            luaL_Reg funcs[] = {
                {name, call_member<R, ArgsT...>::func},
                {NULL, NULL}
            };

            create_LuaGCObject(L_, temp);
            luaL_setfuncs(L_, funcs, 1);

            return *this;
        }


        template<typename op, typename U>
        class_& def_operator(void){
            using F = OverloadableBinaryOperator<op, U>;

            lua_pushstring(L_, op::name);
            lua_rawget(L_, 1);
            if(!lua_isnil(L_, -1)){
                lua_getupvalue(L_, -1, 1);
                auto base_functor = static_cast<detail::Functor*>(lua_touserdata(L_, -1));
                lua_pop(L_, 2);
                while(base_functor->get_next()){
                    base_functor = base_functor->get_next();
                }
                base_functor->set_next(new F());
            }
            else{
                lua_pop(L_, 1);
                create_LuaGCObject<F>(L_);
                lua_pushcclosure(L_, detail::call_overloadable_functor, 1);
                lua_pushstring(L_, op::name);
                lua_pushvalue(L_, -2);
                lua_rawset(L_, 1);
                lua_pop(L_, 1);
            }

            return *this;
        }

        template<class M>
        class_& def_readwrite(const char* name, M type_::*var_ptr){
            lua_pushstring(L_, name);

            auto getter = std::function<M(type_*)>([var_ptr](type_* object){
                return object->*var_ptr;
            });

            auto setter = std::function<void(type_*, const M&)>([var_ptr](type_* object, const M& val){
                object->*var_ptr = val;
            });

            create_LuaGCObject(L_, getter);
            lua_pushcclosure(L_, get_var<M>, 1);

            create_LuaGCObject(L_, setter);
            lua_pushcclosure(L_, set_var<M>, 1);

            lua_pushcclosure(L_, lua_ClassProperty, 2);
            lua_rawset(L_, -3);

            return *this;
        }

        template<class M>
        class_& def_readonly(const char* name, M type_::*var_ptr){
            lua_pushstring(L_, name);

            auto getter = std::function<M(type_*)>([var_ptr](type_* object){
                return object->*var_ptr;
            });

            create_LuaGCObject(L_, getter);
            lua_pushcclosure(L_, get_var<M>, 1);

            lua_pushcclosure(L_, lua_ClassProperty, 1);
            lua_rawset(L_, -3);
            return *this;
        }


        template<typename R, typename ...ArgsT>
        struct call_member{
            static int func(lua_State* L){
                type_* object = static_cast<type_*>(lua_touserdata(L, 1));
                auto func = *static_cast<std::function<std::function<R(ArgsT...)>(type_*)>*>(
                    lua_touserdata(L, lua_upvalueindex(1))
                );
                auto result = detail::lift(func(object), detail::get_args<ArgsT...>(L));
                push_LuaValue(L, result);
                return 1;
            }
        };

        template<typename ...ArgsT>
        struct call_member<void, ArgsT...>{
            static int func(lua_State* L){
                type_* object = static_cast<type_*>(lua_touserdata(L, 1));
                auto func = *static_cast<std::function<std::function<void(ArgsT...)>(type_*)>*>(
                    lua_touserdata(L, lua_upvalueindex(1))
                );
                detail::lift(func(object), detail::get_args<ArgsT...>(L));
                return 0;
            }
        };

        template<class M>
        static int get_var(lua_State* L){
            type_* object = static_cast<type_*>(lua_touserdata(L, 1));
            auto getter = *static_cast<std::function<M(type_*)>*>(
                lua_touserdata(L, lua_upvalueindex(1))
            );
            push_LuaValue(L, getter(object));
            return 1;
        }

        template<class M>
        static int set_var(lua_State* L){
            type_* object = static_cast<type_*>(lua_touserdata(L, 1));
            auto setter = *static_cast<std::function<void(type_*, M)>*>(
                lua_touserdata(L, lua_upvalueindex(1))
            );
            lua_pop(L, 1);
            setter(object, get_LuaValue<M>(L));
            return 0;
        }

        static int __index(lua_State* L){                     //userdata, index
            lua_getmetatable(L, 1);                           //userdata, index, metatable
            lua_pushvalue(L, 2);                              //userdata, index, metatable, index
            lua_rawget(L, -2);                                //userdata, index, metatable[index]
            if(lua_isnil(L, -1)) return 1;                    //
                                                              //
            if(lua_tocfunction(L, -1) == lua_ClassProperty){  //userdata, index, closure
                lua_getupvalue(L, -1, 1);                     //userdata, index, closure, func_1
                lua_pushvalue(L, 1);                          //userdata, index, func, func_1, userdata
                lua_call(L, 1, 1);
            }
            return 1;
        }

        static int __newindex(lua_State* L){  //userdata, index, value
            lua_getmetatable(L, 1);           //userdata, index, value, metatable
            lua_pushvalue(L, 2);              //userdata, index, value, metatable, index
            lua_rawget(L, -2);                //userdata, index, value, metatable, metatable[index]
            if(lua_isnil(L, -1)){             //
                lua_pop(L, 1);                //userdata, index, value, metatable
                lua_insert(L, 2);             //userdata, metatable, index, value
                lua_rawset(L, -3);            //userdata, metatable[index] = value
                return 0;
            }

            if(lua_tocfunction(L, -1) == lua_ClassProperty){
                lua_getupvalue(L, -1, 2);
                lua_remove(L, 2);
                lua_insert(L, 1);
                lua_pop(L, 1);
                lua_call(L, lua_gettop(L) - 1, 0);
                return 0;
            }

            lua_pop(L, 1);
            lua_insert(L, 2);
            lua_rawset(L, 2);
            return 0;
        }

    private:
        template<typename op, class U>
        struct OverloadableBinaryOperator: detail::Functor{
            OverloadableBinaryOperator(void):
                Functor()
            {}

            int call(lua_State* L){
                if(lua_isuserdata(L, 1)){
                    lua_getmetatable(L, 1);
                    lua_rawgetp(L, LUA_REGISTRYINDEX, detail::ClassInfo<type_>::get_metatable_key());
                    bool is_type = lua_rawequal(L, -1, -2);
                    lua_pop(L, 2);
                    if(is_type){
                        U b      = get_LuaValue<U>(L);
                        type_& a = *get_LuaValue<type_*>(L);
                        push_LuaValue(L, op::execute(a, b));
                        return 1;
                    }
                }

                type_ a = get_LuaValue<type_>(L);
                U b     = get_LuaValue<U>(L);
                push_LuaValue(L, op::execute(a, b));
                return 1;
            }

            int score(lua_State* L){
                return detail::score_args<type_, U>(L);
            }

            static const int n_args_ = 2;
        };

        const char* name_;
        lua_State* L_;
    };
}

#endif
