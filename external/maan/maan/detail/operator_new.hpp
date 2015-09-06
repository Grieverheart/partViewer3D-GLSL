#ifndef __OPERATOR_NEW_HPP
#define __OPERATOR_NEW_HPP

void* operator new(std::size_t size, lua_State* L){
    return lua_newuserdata(L, size);
}

#endif
