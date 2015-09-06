#ifndef __GARBAGE_COLLECT_HPP
#define __GARBAGE_COLLECT_HPP

namespace maan{namespace detail{
    template<class T>
    int __gc(lua_State* L){
        T* obj = static_cast<T*>(lua_touserdata(L, -1));
        obj->~T();
        return 0;
    }
}}

#endif
