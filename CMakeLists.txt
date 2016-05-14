cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(partviewer3D-GLSL)

set(CMAKE_MACOSX_RPATH 1)

set(SOURCE_FILES
    src/main.cpp
    src/scene.cpp
    src/event_manager.cpp
    src/shadowmap.cpp
    src/g-buffer.cpp
    src/accumulator.cpp
    src/light.cpp
    src/grid.cpp
    src/shader.cpp
    src/register_lua_bindings.cpp
    src/ssao.cpp
    src/Text/FontManager.cpp)

add_executable(main ${SOURCE_FILES})

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR
   ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wno-comment -Wno-strict-aliasing -O3 -std=c++11 -g -march=native")
endif()

if(UNIX AND NOT APPLE)
    set_target_properties(main PROPERTIES
        INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/")
elseif(APPLE)
    set_target_properties(main PROPERTIES
        INSTALL_RPATH "@executable_path/")
endif()

if ( WIN32 AND NOT CYGWIN )
    set(LUA_BUILD_AS_DLL ON)
endif()
add_subdirectory("${CMAKE_SOURCE_DIR}/external/lua-5.2.4")
set(BUILD_SHARED_LIBS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glfw-3.1.1")
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glew-1.13.0/build/cmake")

find_package(OpenGL REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/external/lua-5.2.4/src"
    "${CMAKE_SOURCE_DIR}/external/glfw-3.1.1/include"
    "${CMAKE_SOURCE_DIR}/external/glew-1.13.0/include"
    "${CMAKE_SOURCE_DIR}/external/glm-0.9.6.3"
    "${CMAKE_SOURCE_DIR}/external/maan")

set(EXTERNAL_LIBS
    glfw
    glew
    liblua)

target_link_libraries(main ${OPENGL_gl_LIBRARY} ${EXTERNAL_LIBS})

install(TARGETS main DESTINATION ${CMAKE_SOURCE_DIR})

