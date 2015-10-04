#include "include/register_lua_bindings.h"
#include "include/particle.h"
#include "include/shape.h"
#include "include/scene.h"
#include <vector>

extern "C"{
#include <lauxlib.h>
#include <lualib.h>
}

#include <maan/maan.hpp>

//TODO: Perhaps use these to avoid repeating code.
//namespace{
//    template<typename T>
//    struct function_traits;
//
//    template<typename R, typename U, typename C>
//    struct function_traits<R(C::*)(U)>{
//        using type = U;
//    };
//}
//
//#define DEF_SET_FUNCTION(name)\
//    template<typename T = function_traits<decltype(&Scene::name)>::type>\
//    static int luaScene_##name(lua_State* L){\
//        auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));\
//        scene->name(maan::get_LuaValue<T>(L));\
//        return 0;\
//    }
//
//#define DEF_GET_FUNCTION(name)\
//    static int luaScene_##name(lua_State* L){\
//        auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));\
//        maan::push_LuaValue(L, scene->name());\
//        return 1;\
//    }

#define ADD_FUNCTION(name)\
    {#name, luaScene_##name},

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

static int luaScene_set_clip_plane(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    auto clip_plane = maan::get_LuaValue<glm::vec4>(L);
    scene->set_clip_plane(clip_plane);
    return 0;
}

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
static int luaScene_set_ssao_blur(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    bool should_blur = lua_toboolean(L, 1);
    scene->set_ssao_blur(should_blur);
    return 0;
}

static int luaScene_is_ssao_blur_enabled(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    bool is_enabled = scene->is_ssao_blur_enabled();
    lua_pushboolean(L, is_enabled);
    return 1;
}

static int luaScene_set_ssao_num_samples(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->set_ssao_num_samples(lua_tointeger(L, 1));
    return 0;
}

static int luaScene_get_ssao_num_samples(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    lua_pushinteger(L, scene->get_ssao_num_samples());
    return 1;
}

static int luaScene_set_ssao_radius(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->set_ssao_radius(lua_tonumber(L, 1));
    return 0;
}

static int luaScene_get_ssao_radius(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    lua_pushnumber(L, scene->get_ssao_radius());
    return 1;
}

static int luaScene_set_background_color(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->set_background_color(maan::get_LuaValue<glm::vec3>(L));
    return 0;
}

static int luaScene_get_background_color(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    maan::push_LuaValue(L, scene->get_background_color());
    return 1;
}
static int luaScene_set_sky_color(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->set_sky_color(maan::get_LuaValue<glm::vec3>(L));
    return 0;
}

static int luaScene_get_sky_color(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    maan::push_LuaValue(L, scene->get_sky_color());
    return 1;
}

static int luaScene_set_light_direction(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    scene->set_light_direction(maan::get_LuaValue<glm::vec3>(L));
    return 0;
}

static int luaScene_get_light_direction(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    maan::push_LuaValue(L, scene->get_light_direction());
    return 1;
}

static int luaScene_set_projection_type(lua_State* L){
    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    lua_pushstring(L, "perspective");
    lua_pushstring(L, "orthographic");
    if(lua_rawequal(L, 1, 2)){
        scene->set_projection_type(Projection::PERSPECTIVE);
    }
    else if(lua_rawequal(L, 1, 3)){
        scene->set_projection_type(Projection::ORTHOGRAPHIC);
    }
    else{
        lua_pop(L, 2);
        printf("Projection type, '%s', not supported\n", lua_tostring(L, 1));
    }
    return 0;
}

//TODO: Implement these. We first need to register glm::mat4, but it's a bit of
//a pain in the ass to do properly because we first need to be able to overload
//operators based on argument types.

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

namespace{
    struct Sphere{};

    struct Mesh{
        void add_vertex(const Vertex& v){
            vertices_.push_back(v);
        }

        Vertex get_vertex(size_t vi)const{
            return vertices_[vi];
        }

        size_t get_num_vertices(void)const{
            return vertices_.size();
        }

        std::vector<Vertex> vertices_;
    };
}

//TODO: Add type safety
//box[], particles[], shapes[]
static int luaScene_load(lua_State* L){
    glm::mat3 box;
    for(lua_pushnil(L); lua_next(L, 1); lua_pop(L, 1)){
        long long int i = lua_tointeger(L, -2) - 1;
        box[i % 3][i / 3] = lua_tonumber(L, -1);
    }

    std::vector<Particle> particles;
    for(lua_pushnil(L); lua_next(L, 2);){
        particles.push_back(maan::get_LuaValue<Particle>(L));
    }

    //TODO: Add a is_class(lua_State* L, int idx) function to maan.
    std::vector<Shape> shapes;
    for(lua_pushnil(L); lua_next(L, 3);){
        lua_getmetatable(L, -1);
        lua_pushstring(L, "__class_id");
        lua_rawget(L, -2);
        void* class_id = lua_touserdata(L, -1);
        lua_pop(L, 2);
        auto shape = Shape();
        if(maan::detail::ClassInfo<Sphere>::get_metatable_key() == class_id){
            shape.type = Shape::SPHERE;
            lua_pop(L, 1);
        }
        else{
            shape.type = Shape::MESH;
            auto mesh = maan::get_LuaValue<Mesh>(L);
            shape.mesh.n_vertices = mesh.vertices_.size();
            shape.mesh.vertices = new Vertex[shape.mesh.n_vertices];
            int i = 0;
            for(auto vertex: mesh.vertices_){
                shape.mesh.vertices[i++] = vertex;
            }
        }
        shapes.push_back(shape);
    }

    auto scene = static_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    SimConfig config;
    config.box = box;
    config.n_part = particles.size();
    config.n_shapes = shapes.size();
    config.particles = new Particle[config.n_part];
    config.shapes = new Shape[config.n_shapes];

    {//Set particles
        int i = 0;
        for(auto particle: particles) config.particles[i++] = particle;
    }
    {//Set shapes
        int i = 0;
        for(auto shape: shapes) config.shapes[i++] = shape;
    }

    scene->load_scene(config);

    return 0;
}

bool register_lua_bindings(lua_State* L, Scene* scene){

    maan::module_(L)
        .class_<Particle>("Particle")
            .def_constructor<>()
            .def_readwrite("shape_id", &Particle::shape_id)
            .def_readwrite("rot", &Particle::rot)
            .def_readwrite("pos", &Particle::pos)
            .def_readwrite("size", &Particle::size)
            .endef()
        //Register Vertex
        .class_<Vertex>("Vertex")
            .def_constructor<const glm::vec3&, const glm::vec3&>()
            .def_readwrite("coord", &Vertex::_coord)
            .def_readwrite("normal", &Vertex::_normal)
            .endef()
        //Register shape
        .class_<Sphere>("Sphere")
            .def_constructor<>()
            .endef()
        //Register Mesh
        .class_<Mesh>("Mesh")
            .def_constructor<>()
            .def("add_vertex", &Mesh::add_vertex)
            .def("get_vertex", &Mesh::get_vertex)
            .def("get_num_vertices", &Mesh::get_num_vertices)
            .endef()
        .namespace_("glm")
            //Register vec3
            .class_<glm::vec3>("vec3")
                .def_constructor<float, float, float>()
                .def_constructor<float>()
                .def_constructor<glm::vec3>()
                .def_operator<maan::add, glm::vec3>()
                .def_operator<maan::sub, glm::vec3>()
                .def_operator<maan::mul, glm::vec3>()
                .def_operator<maan::add, float>()
                .def_operator<maan::sub, float>()
                .def_operator<maan::mul, float>()
                .def_readwrite("x", &glm::vec3::x)
                .def_readwrite("y", &glm::vec3::y)
                .def_readwrite("z", &glm::vec3::z)
                .endef()
            //Register vec4
            .class_<glm::vec4>("vec4")
                .def_constructor<float, float, float, float>()
                .def_constructor<float>()
                .def_constructor<glm::vec4>()
                .def_operator<maan::add, glm::vec4>()
                .def_operator<maan::sub, glm::vec4>()
                .def_operator<maan::mul, glm::vec4>()
                .def_operator<maan::add, float>()
                .def_operator<maan::sub, float>()
                .def_operator<maan::mul, float>()
                .def_readwrite("x", &glm::vec4::x)
                .def_readwrite("y", &glm::vec4::y)
                .def_readwrite("z", &glm::vec4::z)
                .def_readwrite("w", &glm::vec4::w)
                .endef();

    //Register scene
    //TODO: Perhaps add a function_ in lua that takes a member
    //function pointer along with a pointer to the object. Then,
    //we can easily register the functions below.
    luaL_Reg funcs[] = {
        ADD_FUNCTION(load)
        ADD_FUNCTION(zoom)
        ADD_FUNCTION(select_particle)
        ADD_FUNCTION(enable_clip_plane)
        ADD_FUNCTION(disable_clip_plane)
        ADD_FUNCTION(is_ssao_blur_enabled)
        ADD_FUNCTION(set_ssao_blur)
        ADD_FUNCTION(set_ssao_num_samples)
        ADD_FUNCTION(get_ssao_num_samples)
        ADD_FUNCTION(set_ssao_radius)
        ADD_FUNCTION(get_ssao_radius)
        ADD_FUNCTION(set_clip_plane)
        ADD_FUNCTION(get_light_intensity)
        ADD_FUNCTION(set_light_intensity)
        ADD_FUNCTION(get_light_ambient_intensity)
        ADD_FUNCTION(set_light_ambient_intensity)
        ADD_FUNCTION(get_light_diffuse_intensity)
        ADD_FUNCTION(set_light_diffuse_intensity)
        ADD_FUNCTION(get_light_specular_intensity)
        ADD_FUNCTION(set_light_specular_intensity)
        ADD_FUNCTION(get_background_color)
        ADD_FUNCTION(set_background_color)
        ADD_FUNCTION(get_sky_color)
        ADD_FUNCTION(set_sky_color)
        ADD_FUNCTION(get_light_direction)
        ADD_FUNCTION(set_light_direction)
        ADD_FUNCTION(set_projection_type)
        {NULL, NULL}
    };

    luaL_newlibtable(L, funcs);
    lua_pushlightuserdata(L, static_cast<void*>(scene));
    luaL_setfuncs(L, funcs, 1);
    lua_setglobal(L, "scene");

    return true;
}
