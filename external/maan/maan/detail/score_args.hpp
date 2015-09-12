#ifndef __SCORE_ARGS_HPP
#define __SCORE_ARGS_HPP

namespace maan{
    namespace detail{

        template<typename T, typename Enable = void>
        struct score_arg{};

        //TODO: We need to check if this specialization is
        //correctly picked over the arithmetic one.
        template<>
        struct score_arg<bool>{
            static int score(lua_State* L, int idx){
                return lua_isboolean(L, idx)? 1: 0;
            }
        };

        //TODO: Should also implement for std::string
        template<>
        struct score_arg<const char*>{
            static int score(lua_State* L, int idx){
                return lua_isstring(L, idx)? 1: 0;
            }
        };

        template<typename T>
        struct score_arg< T, typename std::enable_if<std::is_arithmetic<T>::value>::type >{
            static int score(lua_State* L, int idx){
                return lua_isnumber(L, idx)? 1: 0;
            }
        };

        template<class T>
        struct score_arg<
            T,
            typename std::enable_if<std::is_class<
                typename std::remove_const<typename std::remove_reference<T>::type>::type
            >::value>::type
        >{
            static int score(lua_State* L, int idx){
                using type_ = typename std::remove_const<typename std::remove_reference<T>::type>::type;
                if(!lua_isuserdata(L, idx)) return 0;
                //Check if we get userdata of the expected type by comparing the metatables.
                lua_getmetatable(L, idx);
                lua_rawgetp(L, LUA_REGISTRYINDEX, detail::ClassInfo<type_>::get_metatable_key());
                int result = 0;
                if(lua_rawequal(L, -1, -2)) result = 1;
                lua_pop(L, 2);
                return result;
            }
        };

        template<typename... ArgsT>
        struct arg_scorer;

        template<>
        struct arg_scorer<>{
            static int score(lua_State* L, int idx){
                return 0;
            }
        };

        template<typename T, typename ...ArgsT>
        struct arg_scorer<T, ArgsT...>{
            static int score(lua_State* L, int idx){
                return (score_arg<T>::score(L, idx) + arg_scorer<ArgsT...>::score(L, idx + 1));
            }
        };


        template<typename ...ArgsT>
        int score_args(lua_State* L){
            int n_args = lua_gettop(L);
            if(n_args != sizeof...(ArgsT)) return 0;
            else if(n_args == 0) return 1;
            else{
                //int result = arg_scorer<ArgsT...>::score(L, 1);
                //printf("---- %d, %d\n", result, n_args);
                return arg_scorer<ArgsT...>::score(L, 1);
            }
        }
    }
}

#endif
