#include "include/register_lua_bindings.h"
#include "include/particle.h"
#include "include/shape.h"
#include "include/scene.h"
#include <glm/gtc/matrix_access.hpp>
#include <vector>

extern "C"{
#include <lauxlib.h>
#include <lualib.h>
}

#include <maan/maan.hpp>

static int luaScene_set_projection_type(lua_State* L){
    auto scene = reinterpret_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
    lua_pushstring(L, "perspective");
    lua_pushstring(L, "orthographic");
    if(lua_rawequal(L, 1, 2)){
        scene->set_projection_type(Projection::PERSPECTIVE);
    }
    else if(lua_rawequal(L, 1, 3)){
        scene->set_projection_type(Projection::ORTHOGRAPHIC);
    }
    else{
        printf("Projection type, '%s', not supported\n", lua_tostring(L, 1));
    }
    lua_pop(L, 2);
    return 0;
}

static int luaScene_raytrace(lua_State* L){
    auto scene = reinterpret_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    int pid;

    if(scene->raytrace(x, y, pid)){
        lua_pushinteger(L, pid);
    }
    else{
        lua_pushnil(L);
    }

    return 1;
}

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

//TODO: Add type safety, and logic validation
//box[], particles[], shapes[]
static int luaScene_load(lua_State* L){
    SimConfig config;

    for(lua_pushnil(L); lua_next(L, 1); lua_pop(L, 1)){
        long long int i = lua_tointeger(L, -2) - 1;
        config.box[i % 3][i / 3] = lua_tonumber(L, -1);
    }

    config.n_particles = lua_rawlen(L, 2);
    config.particles = new Particle[config.n_particles];

    size_t pid = 0;
    for(lua_pushnil(L); lua_next(L, 2);){
        config.particles[pid++] = maan::get_LuaValue<Particle>(L);
    }

    config.n_shapes = lua_rawlen(L, 3);
    config.shapes = new Shape[config.n_shapes];

    //TODO: Add a is_class(lua_State* L, int idx) function to maan.
    size_t sid = 0;
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
        config.shapes[sid++] = shape;
    }

    auto scene = reinterpret_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));

    scene->load_scene(config);

    return 0;
}

static int luaScene_raw_load(lua_State* L){
    void* raw_config = lua_touserdata(L, 1);
    if(raw_config){
        auto scene = reinterpret_cast<Scene*>(lua_touserdata(L, lua_upvalueindex(1)));
        scene->load_scene(*reinterpret_cast<SimConfig*>(raw_config));
    }
    return 0;
}

static int luaWindow_size(lua_State* L){
    int width, height;
    auto window = reinterpret_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    glfwGetWindowSize(window, &width, &height);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 2;
}

static int luaWindow_position(lua_State* L){
    int x, y;
    auto window = reinterpret_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    glfwGetWindowPos(window, &x, &y);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    return 2;
}

static int luaWindow_set_size(lua_State* L){
    auto window = reinterpret_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    int width = lua_tointeger(L, 1);
    int height = lua_tointeger(L, 2);
    glfwSetWindowSize(window, width, height);
    return 0;
}

static int luaWindow_set_position(lua_State* L){
    auto window = reinterpret_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    glfwSetWindowPos(window, x, y);
    return 0;
}

static int luaWindow_set_title(lua_State* L){
    auto window = reinterpret_cast<GLFWwindow*>(lua_touserdata(L, lua_upvalueindex(1)));
    const char* title = lua_tostring(L, 1);
    glfwSetWindowTitle(window, title);
    return 0;
}

static inline void set_lua_integer_field(lua_State* L, const char* field, int value){
    lua_pushinteger(L, value);
    lua_setfield(L, -2, field);
}

static void register_keyboard_keys(lua_State* L){
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "keyboard");

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "key");

    set_lua_integer_field(L, "space", 32);
    set_lua_integer_field(L, "'", 39 );
    set_lua_integer_field(L, ",", 44);
    set_lua_integer_field(L, "-", 45);
    set_lua_integer_field(L, ".", 46);
    set_lua_integer_field(L, "/", 47);
    set_lua_integer_field(L, "0", 48);
    set_lua_integer_field(L, "1", 49);
    set_lua_integer_field(L, "2", 50);
    set_lua_integer_field(L, "3", 51);
    set_lua_integer_field(L, "4", 52);
    set_lua_integer_field(L, "5", 53);
    set_lua_integer_field(L, "6", 54);
    set_lua_integer_field(L, "7", 55);
    set_lua_integer_field(L, "8", 56);
    set_lua_integer_field(L, "9", 57);
    set_lua_integer_field(L, ";", 59);
    set_lua_integer_field(L, "=", 61);
    set_lua_integer_field(L, "a", 65);
    set_lua_integer_field(L, "b", 66);
    set_lua_integer_field(L, "c", 67);
    set_lua_integer_field(L, "d", 68);
    set_lua_integer_field(L, "e", 69);
    set_lua_integer_field(L, "f", 70);
    set_lua_integer_field(L, "g", 71);
    set_lua_integer_field(L, "h", 72);
    set_lua_integer_field(L, "i", 73);
    set_lua_integer_field(L, "j", 74);
    set_lua_integer_field(L, "k", 75);
    set_lua_integer_field(L, "l", 76);
    set_lua_integer_field(L, "m", 77);
    set_lua_integer_field(L, "n", 78);
    set_lua_integer_field(L, "o", 79);
    set_lua_integer_field(L, "p", 80);
    set_lua_integer_field(L, "q", 81);
    set_lua_integer_field(L, "r", 82);
    set_lua_integer_field(L, "s", 83);
    set_lua_integer_field(L, "t", 84);
    set_lua_integer_field(L, "u", 85);
    set_lua_integer_field(L, "v", 86);
    set_lua_integer_field(L, "w", 87);
    set_lua_integer_field(L, "x", 88);
    set_lua_integer_field(L, "y", 89);
    set_lua_integer_field(L, "z", 90);
    set_lua_integer_field(L, "[", 91);
    set_lua_integer_field(L, "\\", 92);
    set_lua_integer_field(L, "]", 93);
    set_lua_integer_field(L, "`", 96);
    set_lua_integer_field(L, "world_1", 161);
    set_lua_integer_field(L, "world_2", 162);
    set_lua_integer_field(L, "esc", 256);
    set_lua_integer_field(L, "enter", 257);
    set_lua_integer_field(L, "tab", 258);
    set_lua_integer_field(L, "backspace", 259);
    set_lua_integer_field(L, "insert", 260);
    set_lua_integer_field(L, "delete", 261);
    set_lua_integer_field(L, "right", 262);
    set_lua_integer_field(L, "left", 263);
    set_lua_integer_field(L, "down", 264);
    set_lua_integer_field(L, "up", 265);
    set_lua_integer_field(L, "page_up", 266);
    set_lua_integer_field(L, "page_down", 267);
    set_lua_integer_field(L, "home", 268);
    set_lua_integer_field(L, "end", 269);
    set_lua_integer_field(L, "caps_lock", 280);
    set_lua_integer_field(L, "scroll_lock", 281);
    set_lua_integer_field(L, "num_lock", 282);
    set_lua_integer_field(L, "print_screen", 283);
    set_lua_integer_field(L, "pause", 284);
    set_lua_integer_field(L, "f1", 290);
    set_lua_integer_field(L, "f2", 291);
    set_lua_integer_field(L, "f3", 292);
    set_lua_integer_field(L, "f4", 293);
    set_lua_integer_field(L, "f5", 294);
    set_lua_integer_field(L, "f6", 295);
    set_lua_integer_field(L, "f7", 296);
    set_lua_integer_field(L, "f8", 297);
    set_lua_integer_field(L, "f9", 298);
    set_lua_integer_field(L, "f10", 299);
    set_lua_integer_field(L, "f11", 300);
    set_lua_integer_field(L, "f12", 301);
    set_lua_integer_field(L, "f13", 302);
    set_lua_integer_field(L, "f14", 303);
    set_lua_integer_field(L, "f15", 304);
    set_lua_integer_field(L, "f16", 305);
    set_lua_integer_field(L, "f17", 306);
    set_lua_integer_field(L, "f18", 307);
    set_lua_integer_field(L, "f19", 308);
    set_lua_integer_field(L, "f20", 309);
    set_lua_integer_field(L, "f21", 310);
    set_lua_integer_field(L, "f22", 311);
    set_lua_integer_field(L, "f23", 312);
    set_lua_integer_field(L, "f24", 313);
    set_lua_integer_field(L, "f25", 314);
    set_lua_integer_field(L, "kp0", 320);
    set_lua_integer_field(L, "kp1", 321);
    set_lua_integer_field(L, "kp2", 322);
    set_lua_integer_field(L, "kp3", 323);
    set_lua_integer_field(L, "kp4", 324);
    set_lua_integer_field(L, "kp5", 325);
    set_lua_integer_field(L, "kp6", 326);
    set_lua_integer_field(L, "kp7", 327);
    set_lua_integer_field(L, "kp8", 328);
    set_lua_integer_field(L, "kp9", 329);
    set_lua_integer_field(L, "kp.", 330);
    set_lua_integer_field(L, "kp/", 331);
    set_lua_integer_field(L, "kp*", 332);
    set_lua_integer_field(L, "kp-", 333);
    set_lua_integer_field(L, "kp+", 334);
    set_lua_integer_field(L, "kpenter", 335);
    set_lua_integer_field(L, "kp=", 336);
    set_lua_integer_field(L, "lshift", 340);
    set_lua_integer_field(L, "lctrl", 341);
    set_lua_integer_field(L, "left_alt", 342);
    set_lua_integer_field(L, "lsuper", 343);
    set_lua_integer_field(L, "rshift", 344);
    set_lua_integer_field(L, "rcontrol", 345);
    set_lua_integer_field(L, "ralt", 346);
    set_lua_integer_field(L, "rsuper", 347);
    set_lua_integer_field(L, "menu", 348);

    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "action");

    set_lua_integer_field(L, "release", 0);
    set_lua_integer_field(L, "press", 1);

    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "mod");

    set_lua_integer_field(L, "shift", 0x0001);
    set_lua_integer_field(L, "ctrl", 0x0002);
    set_lua_integer_field(L, "alt", 0x0004);
    set_lua_integer_field(L, "super", 0x0008);

    lua_pop(L, 2);
}

static void register_mouse_buttons(lua_State* L){
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "mouse");

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "button");

    set_lua_integer_field(L, "left", 0);
    set_lua_integer_field(L, "right", 1);
    set_lua_integer_field(L, "middle", 2);
    set_lua_integer_field(L, "1", 0);
    set_lua_integer_field(L, "2", 1);
    set_lua_integer_field(L, "3", 2);
    set_lua_integer_field(L, "4", 3);
    set_lua_integer_field(L, "5", 4);
    set_lua_integer_field(L, "6", 5);
    set_lua_integer_field(L, "7", 6);
    set_lua_integer_field(L, "8", 7);

    lua_pop(L, 1);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "action");

    set_lua_integer_field(L, "release", 0);
    set_lua_integer_field(L, "press", 1);

    lua_pop(L, 2);
}

//TODO: Add copy constructors
bool register_lua_bindings(lua_State* L, Scene* scene, GLFWwindow* window){

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
        .function_("cross", (glm::vec3 (*)(const glm::vec3&, const glm::vec3&)) glm::cross)
        .function_("dot", (float (*)(const glm::vec3&, const glm::vec3&)) glm::dot)
        .function_("normalize", (glm::vec3 (*)(const glm::vec3&)) glm::normalize)
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
            .endef()
        //TODO: Matrix access.
        .class_<glm::mat3>("mat3")
            .def_constructor<float>()
            .def_constructor<glm::mat3>()
            .def_constructor<glm::mat4>()
            .def_operator<maan::add, glm::mat3>()
            .def_operator<maan::sub, glm::mat3>()
            .def_operator<maan::mul, glm::mat3>()
            .def_operator<maan::mul, glm::vec3>()
            .def_operator<maan::add, float>()
            .def_operator<maan::sub, float>()
            .def_operator<maan::mul, float>()
            .endef()
        .function_("column", (glm::vec3 (*)(const glm::mat3&, int index)) glm::column)
        .function_("row", (glm::vec3 (*)(const glm::mat3&, int index)) glm::row)
        .function_("inverse", (glm::mat3 (*)(const glm::mat3&)) glm::inverse)
        .function_("transpose", (glm::mat3 (*)(const glm::mat3&)) glm::transpose)
        .class_<glm::mat4>("mat4")
            .def_constructor<float>()
            .def_constructor<glm::mat4>()
            .def_operator<maan::add, glm::mat4>()
            .def_operator<maan::sub, glm::mat4>()
            .def_operator<maan::mul, glm::mat4>()
            .def_operator<maan::mul, glm::vec4>()
            .def_operator<maan::add, float>()
            .def_operator<maan::sub, float>()
            .def_operator<maan::mul, float>()
            .endef()
        .function_("column", (glm::vec4 (*)(const glm::mat4&, int index)) glm::column)
        .function_("row", (glm::vec4 (*)(const glm::mat4&, int index)) glm::row)
        .function_("inverse", (glm::mat4 (*)(const glm::mat4&)) glm::inverse)
        .function_("transpose", (glm::mat4 (*)(const glm::mat4&)) glm::transpose);

    maan::module_(L, "scene")
        .class_<Particle>("Particle")
            .def_constructor<>()
            .def_constructor<const Particle&>()
            .def_readwrite("shape_id", &Particle::shape_id)
            .def_readwrite("rot", &Particle::rot)
            .def_readwrite("pos", &Particle::pos)
            .def_readwrite("size", &Particle::size)
            .endef()
        //TODO: Remove the vertex class
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
        .class_<Text::Properties>("TextProperties")
            .def_constructor<>()
            .def_constructor<const char*, float, const glm::vec4&, int, int>()
            .def_readwrite("color", &Text::Properties::color_)
            .def_readwrite("font", &Text::Properties::font_)
            .def_readwrite("width", &Text::Properties::width_)
            .def_readwrite("x", &Text::Properties::x_)
            .def_readwrite("y", &Text::Properties::y_)
            .endef()
        .function_("set_fov_degrees", &Scene::set_fov_degrees, scene)
        .function_("get_fov_degrees", &Scene::get_fov_degrees, scene)
        .function_("rotate", &Scene::rotate, scene)
        .function_("select_particle", &Scene::select_particle, scene)
        .function_("hide_particle", &Scene::hide_particle, scene)
        .function_("unhide_particle", &Scene::unhide_particle, scene)
        .function_("set_particle_color", &Scene::set_particle_color, scene)
        .function_("toggle_particle_point_drawing", &Scene::toggle_particle_point_drawing, scene)
        .function_("set_point_radius", &Scene::set_point_radius, scene)
        .function_("set_point_outline_radius", &Scene::set_point_outline_radius, scene)
        .function_("set_point_color", &Scene::set_point_color, scene)
        .function_("set_point_outline_color", &Scene::set_point_outline_color, scene)
        .function_("set_particle_selection_color", &Scene::set_particle_selection_color, scene)
        .function_("set_particle_selection_width_fraction", &Scene::set_particle_selection_width_fraction, scene)
        .function_("set_box_line_width", &Scene::set_box_line_width, scene)
        .function_("is_particle_selected", &Scene::is_particle_selected, scene)
        .function_("clear_particle_selections", &Scene::clear_particle_selections, scene)
        .function_("enable_clip_plane", &Scene::enable_clip_plane, scene)
        .function_("disable_clip_plane", &Scene::disable_clip_plane, scene)
        .function_("toggle_box", &Scene::toggle_box, scene)
        .function_("is_ssao_blur_enabled", &Scene::is_ssao_blur_enabled, scene)
        .function_("toggle_ssao_blur", &Scene::toggle_ssao_blur, scene)
        .function_("set_ssao_num_samples", &Scene::set_ssao_num_samples, scene)
        .function_("get_ssao_num_samples", &Scene::get_ssao_num_samples, scene)
        .function_("set_ssao_radius", &Scene::set_ssao_radius, scene)
        .function_("get_ssao_radius", &Scene::get_ssao_radius, scene)
        .function_("set_clip_plane", &Scene::set_clip_plane, scene)
        .function_("get_view_matrix", &Scene::get_view_matrix, scene)
        .function_("get_projection_matrix", &Scene::get_projection_matrix, scene)
        .function_("get_model_matrix", &Scene::get_model_matrix, scene)
        .function_("get_view_position", &Scene::get_view_position, scene)
        .function_("set_view_position", &Scene::set_view_position, scene)
        .function_("get_view_direction", &Scene::get_view_direction, scene)
        .function_("set_view_direction", &Scene::set_view_direction, scene)
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
        .function_("set_light_direction", &Scene::set_light_direction, scene)
        .function_("draw_text", &Scene::draw_text, scene)
        .function_("save_snapshot_png", &Scene::save_snapshot_png, scene);

#define ADD_CLASS_FUNCTION(cls, name)\
    {#name, lua ##cls##_##name},

    {
        luaL_Reg funcs[] = {
            ADD_CLASS_FUNCTION(Scene, load)
            ADD_CLASS_FUNCTION(Scene, raw_load)
            ADD_CLASS_FUNCTION(Scene, set_projection_type)
            ADD_CLASS_FUNCTION(Scene, raytrace)
            {NULL, NULL}
        };

        lua_getglobal(L, "scene");
        lua_pushlightuserdata(L, reinterpret_cast<void*>(scene));
        luaL_setfuncs(L, funcs, 1);
        lua_pop(L, 1);
    }

    {
        luaL_Reg funcs[] = {
            ADD_CLASS_FUNCTION(Window, size)
            ADD_CLASS_FUNCTION(Window, position)
            ADD_CLASS_FUNCTION(Window, set_size)
            ADD_CLASS_FUNCTION(Window, set_position)
            ADD_CLASS_FUNCTION(Window, set_title)
            {NULL, NULL}
        };

        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, "window");

        lua_pushlightuserdata(L, window);
        luaL_setfuncs(L, funcs, 1);
        lua_pop(L, 1);

        register_keyboard_keys(L);
        register_mouse_buttons(L);
    }

    return true;
}
