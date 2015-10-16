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

static int luaWindow_size(lua_State* L){
    int width, height;
    auto window = static_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    glfwGetWindowSize(window, &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int luaWindow_position(lua_State* L){
    int x, y;
    auto window = static_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    glfwGetWindowPos(window, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int luaWindow_set_size(lua_State* L){
    auto window = static_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    int width = lua_tointeger(L, 1);
    int height = lua_tointeger(L, 2);
    glfwSetWindowSize(window, width, height);
    return 0;
}

static int luaWindow_set_position(lua_State* L){
    auto window = static_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    glfwSetWindowPos(window, x, y);
    return 0;
}

static int luaWindow_set_title(lua_State* L){
    auto window = static_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    const char* title = lua_tostring(L, 1);
    glfwSetWindowTitle(window, title);
    return 0;
}

bool register_lua_bindings(lua_State* L, Scene* scene, GLFWwindow* window){

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
            .endef();

    maan::module_(L, "glm")
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

    maan::module_(L, "scene")
        .function_("zoom", &Scene::zoom, scene)
        .function_("select_particle", &Scene::select_particle, scene)
        .function_("enable_clip_plane", &Scene::enable_clip_plane, scene)
        .function_("disable_clip_plane", &Scene::disable_clip_plane, scene)
        .function_("is_ssao_blur_enabled", &Scene::is_ssao_blur_enabled, scene)
        .function_("set_ssao_blur", &Scene::set_ssao_blur, scene)
        .function_("set_ssao_num_samples", &Scene::set_ssao_num_samples, scene)
        .function_("get_ssao_num_samples", &Scene::get_ssao_num_samples, scene)
        .function_("set_ssao_radius", &Scene::set_ssao_radius, scene)
        .function_("get_ssao_radius", &Scene::get_ssao_radius, scene)
        .function_("set_clip_plane", &Scene::set_clip_plane, scene)
        .function_("get_light_intensity", &Scene::get_light_intensity, scene)
        .function_("set_light_intensity", &Scene::set_light_intensity, scene)
        .function_("get_light_ambient_intensity", &Scene::get_light_ambient_intensity, scene)
        .function_("set_light_ambient_intensity", &Scene::set_light_ambient_intensity, scene)
        .function_("get_light_diffuse_intensity", &Scene::get_light_diffuse_intensity, scene)
        .function_("set_light_diffuse_intensity", &Scene::set_light_diffuse_intensity, scene)
        .function_("get_light_specular_intensity", &Scene::get_light_specular_intensity, scene)
        .function_("set_light_specular_intensity", &Scene::set_light_specular_intensity, scene)
        .function_("get_background_color", &Scene::get_background_color, scene)
        .function_("set_background_color", &Scene::set_background_color, scene)
        .function_("get_sky_color", &Scene::get_sky_color, scene)
        .function_("set_sky_color", &Scene::set_sky_color, scene)
        .function_("get_light_direction", &Scene::get_light_direction, scene)
        .function_("set_light_direction", &Scene::set_light_direction, scene);

    //maan::module_(L, "window")
    //    .function_("size", &Scene::zoom, scene);

#define ADD_FUNCTION(cls, name)\
    {#name, lua ##cls##_##name},

    {
        luaL_Reg funcs[] = {
            ADD_FUNCTION(Scene, load)
            ADD_FUNCTION(Scene, set_projection_type)
            {NULL, NULL}
        };

        lua_getglobal(L, "scene");
        lua_pushlightuserdata(L, static_cast<void*>(scene));
        luaL_setfuncs(L, funcs, 1);
        lua_pop(L, 1);
    }

    {
        luaL_Reg funcs[] = {
            ADD_FUNCTION(Window, size)
            ADD_FUNCTION(Window, position)
            ADD_FUNCTION(Window, set_size)
            ADD_FUNCTION(Window, set_position)
            ADD_FUNCTION(Window, set_title)
            {NULL, NULL}
        };

        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "window");

        lua_pushlightuserdata(L, window);
        luaL_setfuncs(L, funcs, 1);
        lua_pop(L, 1);
    }

    return true;
}
