#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

#include "include/scene.h"
#include <GLFW/glfw3.h>
extern "C"{
#include <lua.h>
}

bool register_lua_bindings(lua_State*, Scene* scene, GLFWwindow* window);

#endif
