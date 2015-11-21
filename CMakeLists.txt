cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
#set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

project(partviewer3D-GLSL)


set(SOURCE_FILES
    src/main.cpp
    src/coord_parser.cpp
    src/scene.cpp
    src/event_manager.cpp
    src/shadowmap.cpp
    src/g-buffer.cpp
    src/accumulator.cpp
    src/light.cpp
    src/grid.cpp
    src/obj_parser.cpp
    src/shader.cpp
    src/register_lua_bindings.cpp
    src/ssao.cpp
    src/Text/FontManager.cpp)

add_executable(main ${SOURCE_FILES})

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Wno-comment -Wno-strict-aliasing -O3 -std=c++11 -g -march=native")
endif()

if(UNIX)
    set_target_properties(main PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/")
endif()

add_subdirectory("${CMAKE_SOURCE_DIR}/external/lua-5.2.4")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glew-1.12.0")
#set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
set(BUILD_SHARED_LIBS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glfw-3.1.1")

find_package(OpenGL REQUIRED)
#find_package(Lua REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/external/lua-5.2.4/src"
    "${CMAKE_SOURCE_DIR}/external/glfw-3.1.1/include"
    "${CMAKE_SOURCE_DIR}/external/glm-0.9.6.3"
    "${CMAKE_SOURCE_DIR}/external/maan"
    "${CMAKE_SOURCE_DIR}/external/glew-1.12.0/include")

set(EXTERNAL_LIBS
    glfw
    liblua
    GLEW_shared)

target_link_libraries(main ${OPENGL_gl_LIBRARY} ${EXTERNAL_LIBS})

install(TARGETS main DESTINATION ${CMAKE_SOURCE_DIR})

