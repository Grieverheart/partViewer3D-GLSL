#include "include/scene.h"
#include "include/event_manager.h"
#include "include/events.h"
#include "include/register_lua_bindings.h"
#include "include/perfmon.h"
#include <GLFW/glfw3.h>
#include <cstdio>
extern "C"{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

static bool running = true;

static Scene* scene          = nullptr;
static EventManager* evt_mgr = nullptr;
static lua_State* L          = nullptr;

static void call_lua_OnInit(lua_State* L, int argc, char* argv[]){
    lua_getglobal(L, "OnInit");
    if(!lua_isnil(L, -1)){
        lua_createtable(L, argc, 0);
        for(int i = 0; i < argc; ++i){
            lua_pushinteger(L, i + 1);
            lua_pushstring(L, argv[i]);
            lua_rawset(L, -3);
        }
        if(lua_pcall(L, 1, 1, 0)){
            printf("There was an error.\n %s\n", lua_tostring(L, -1));
        }
    }
    else lua_pop(L, 1);
}

static void call_lua_OnFrame(lua_State* L){
    lua_getglobal(L, "OnFrame");
    if(!lua_isnil(L, -1)){
        if(lua_pcall(L, 0, 0, 0)){
            printf("There was an error.\n %s\n", lua_tostring(L, -1));
        }
    }
    else lua_pop(L, 1);
}

static void call_lua_OnKey(lua_State* L, int key, int action, int mods){
    lua_getglobal(L, "OnKey");
    if(!lua_isnil(L, -1)){
        lua_pushinteger(L, key);
        lua_pushinteger(L, action);
        lua_pushinteger(L, mods);
        if(lua_pcall(L, 3, 0, 0)){
            printf("There was an error.\n %s\n", lua_tostring(L, -1));
        }
    }
    else lua_pop(L, 1);
}

static void call_lua_OnMouseClick(lua_State* L, float x, float y, int button, int action, int mods){
    lua_getglobal(L, "OnMouseClick");
    if(!lua_isnil(L, -1)){
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        lua_pushinteger(L, button);
        lua_pushinteger(L, action);
        lua_pushinteger(L, mods);
        if(lua_pcall(L, 5, 0, 0)){
            printf("There was an error.\n %s\n", lua_tostring(L, -1));
        }
    }
    else lua_pop(L, 1);
}

static void call_lua_OnMouseMotion(lua_State* L, float x, float y){
    lua_getglobal(L, "OnMouseMotion");
    if(!lua_isnil(L, -1)){
        lua_pushnumber(L, x);
        lua_pushnumber(L, y);
        if(lua_pcall(L, 2, 0, 0)){
            printf("There was an error.\n %s\n", lua_tostring(L, -1));
        }
    }
    else lua_pop(L, 1);
}

static void call_lua_OnMouseScroll(lua_State* L, float dz){
    lua_getglobal(L, "OnMouseScroll");
    if(!lua_isnil(L, -1)){
        lua_pushnumber(L, dz);
        if(lua_pcall(L, 1, 0, 0)){
            printf("There was an error.\n %s\n", lua_tostring(L, -1));
        }
    }
    else lua_pop(L, 1);
}

//TODO: Move TW events to Gui class

////////////GLUT Keyboard Function Wrappers/////////////
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    call_lua_OnKey(L, key, action, mods);
    switch(key){
    case GLFW_KEY_ESCAPE:
        if(action == GLFW_PRESS) running = false;
        break;
    default:
        break;
    }
}

////////////////GLUT Mouse Function Wrappers////////////////

static void on_mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    evt_mgr->queueEvent(new MouseClickEvent(x, y, button, action, mods));
}

static void on_mouse_motion_callback(GLFWwindow* window, double x, double y){
    evt_mgr->queueEvent(new MouseMotionEvent(x, y));
}

static void scroll_callback(GLFWwindow* window, double x, double y){
    evt_mgr->queueEvent(new MouseScrollEvent(y));
}

////////////////////////////////////////////////////////

static void reshape(GLFWwindow* window, int width, int height){
    if(evt_mgr) evt_mgr->queueEvent(new WindowSizeEvent(width, height));
}

static void glfw_error_callback(int error, const char* description){
    fputs(description, stderr);
}

int main(int argc,char *argv[] ){

    int width  = 600;
    int height = 600;

    glfwSetErrorCallback(glfw_error_callback);

	if(!glfwInit()) return 1;

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(width, height, "partviewerGLSL", NULL, NULL);
    if(!window){
        glfwTerminate();
        return 1;
    }

	glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, on_mouse_motion_callback);
    glfwSetMouseButtonCallback(window, on_mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwMakeContextCurrent(window);

    //TODO: Add try catch for Scene construction
    scene   = new Scene(width, height);
    evt_mgr = new EventManager();

    evt_mgr->addHandler([=](const Event& event){
        auto wsize_event = static_cast<const WindowSizeEvent&>(event);
        scene->wsize_changed(wsize_event.width, wsize_event.height);
    }, EVT_WINDOW_SIZE_CHANGED);

    evt_mgr->addHandler([=](const Event& event){
        auto click_event = static_cast<const MouseClickEvent&>(event);
        call_lua_OnMouseClick(L, click_event.x, click_event.y, click_event.button, click_event.action, click_event.mods);
    }, EVT_MOUSE_CLICK);

    evt_mgr->addHandler([=](const Event& event){
        auto motion_event = static_cast<const MouseMotionEvent&>(event);
        call_lua_OnMouseMotion(L, motion_event.x, motion_event.y);
    }, EVT_MOUSE_MOTION);

    evt_mgr->addHandler([=](const Event& event){
        auto scroll_event = static_cast<const MouseScrollEvent&>(event);
        call_lua_OnMouseScroll(L, scroll_event.dz);
    }, EVT_MOUSE_SCROLL);


    L = luaL_newstate();
	luaL_openlibs(L);

    register_lua_bindings(L, scene, window);

    if(luaL_dofile(L, "test.lua")){
        printf("There was an error.\n %s\n", lua_tostring(L, -1));
    }


    PerfMon perf;
    double start_time = perf.get_time_ns();

    call_lua_OnInit(L, argc, argv);

    printf("%f\n", 1.0e-9 * (perf.get_time_ns() - start_time));

    start_time = perf.get_time_ns();
    int frame = 0;

    while(!glfwWindowShouldClose(window) && running){
        ++frame;
        evt_mgr->processQueue();
        scene->process();

        scene->render();

        call_lua_OnFrame(L);

        glfwSwapBuffers(window);
        glfwPollEvents();
        if(frame % 100 == 0) printf("%f\n", 1.0e9 * frame / (perf.get_time_ns() - start_time));
    }

	lua_close(L);

    //TODO: Make these unique pointers.
    delete evt_mgr;
    delete scene;

    glfwDestroyWindow(window);

    glfwTerminate();

	return 1;
}
