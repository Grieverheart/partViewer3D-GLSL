#include "include/register_lua_bindings.h"
#include "include/scene.h"

extern "C"{
#include <lauxlib.h>
#include <lualib.h>
}

#include <maan/class_.hpp>

//box[], shapes[], particles[]
static int luaScene_load_scene(lua_State* L){

    for(lua_pushnil(L); lua_next(L, 1); lua_pop(L, 1)){
        printf("%f - %f\n", lua_tonumber(L, -2), lua_tonumber(L, -1));
    }

    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    return 0;
}

//double dz
static int luaScene_zoom(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double dz = lua_tonumber(L, 1);
    scene->zoom(dz);
    return 0;
}

//double x, double y
static int luaScene_select_particle(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    scene->select_particle(x, y);
    return 0;
}

//static int luaScene_set_projection_type(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_view_matrix(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_projection_matrix(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_model_matrix(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_set_clip_plane(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}

static int luaScene_enable_clip_plane(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->enable_clip_plane();
    return 0;
}

static int luaScene_disable_clip_plane(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->disable_clip_plane();
    return 0;
}

//static int luaScene_get_light_direction(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//
//static int luaScene_set_light_direction(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}

static int luaScene_get_light_specular_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = scene->get_light_specular_intensity();
    lua_pushnumber(L, intensity);
    return 1;
}

static int luaScene_set_light_specular_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = lua_tonumber(L, 1);
    scene->set_light_specular_intensity(intensity);
    return 0;
}

static int luaScene_get_light_diffuse_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = scene->get_light_diffuse_intensity();
    lua_pushnumber(L, intensity);
    return 1;
}

static int luaScene_set_light_diffuse_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = lua_tonumber(L, 1);
    scene->set_light_diffuse_intensity(intensity);
    return 0;
}

static int luaScene_get_light_ambient_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = scene->get_light_ambient_intensity();
    lua_pushnumber(L, intensity);
    return 1;
}

static int luaScene_set_light_ambient_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = lua_tonumber(L, 1);
    scene->set_light_ambient_intensity(intensity);
    return 0;
}

static int luaScene_get_light_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = scene->get_light_intensity();
    lua_pushnumber(L, intensity);
    return 1;
}

static int luaScene_set_light_intensity(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    double intensity = lua_tonumber(L, 1);
    scene->set_light_intensity(intensity);
    return 0;
}
//static int luaScene_set_ssao_blur(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_is_ssao_blur_enabled(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_set_ssao_radius(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_ssao_radius(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_set_ssao_num_samples(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_ssao_num_samples(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_set_sky_color(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_sky_color(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_set_background_color(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}
//static int luaScene_get_background_color(lua_State* L){
//    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
//    return 0;
//}

bool register_lua_bindings(lua_State* L, Scene* scene){
    using namespace maan;
    //Register vec3
    class_<glm::vec3>(L, "vec3")
        .def_constructor<float, float, float>();

    //Register quat
    //Register Particle
    //Register shape

    //Register scene

    luaL_Reg funcs[] = {
        {"scene_load", luaScene_load_scene},
        {"scene_zoom", luaScene_zoom},
        {"scene_select_particle", luaScene_select_particle},
        {"scene_enable_clip_plane", luaScene_enable_clip_plane},
        {"scene_disable_clip_plane", luaScene_disable_clip_plane},
        {"scene_get_light_intensity", luaScene_get_light_intensity},
        {"scene_set_light_intensity", luaScene_set_light_intensity},
        {"scene_get_light_ambient_intensity", luaScene_get_light_ambient_intensity},
        {"scene_set_light_ambient_intensity", luaScene_set_light_ambient_intensity},
        {"scene_get_light_diffuse_intensity", luaScene_get_light_diffuse_intensity},
        {"scene_set_light_diffuse_intensity", luaScene_set_light_diffuse_intensity},
        {"scene_get_light_specular_intensity", luaScene_get_light_specular_intensity},
        {"scene_set_light_specular_intensity", luaScene_set_light_specular_intensity},
        {NULL, NULL}
    };

    lua_pushglobaltable(L);
    lua_pushlightuserdata(L, static_cast<void*>(scene));
    luaL_setfuncs(L, funcs, 1);

    return true;
}
