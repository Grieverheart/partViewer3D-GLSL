cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(partviewer3D-GLSL)


set(SOURCE_FILES
    src/main.cpp
    src/perfmon.cpp
    src/coord_parser.cpp
    src/mouse.cpp
    src/opengl_3.cpp
    src/shadowmap.cpp
    src/g-buffer.cpp
    src/light.cpp
    src/mesh.cpp
    src/obj_parser.cpp
    src/shader.cpp
    src/ssao.cpp)

add_executable(main ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "-Wall -O3 -std=c++11 -g -march=native")

add_subdirectory("${CMAKE_SOURCE_DIR}/external/glew-1.12.0" EXCLUDE_FROM_ALL)
#set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries")
set(BUILD_SHARED_LIBS ON)
add_subdirectory("${CMAKE_SOURCE_DIR}/external/glfw-3.1.1" EXCLUDE_FROM_ALL)

find_package(OpenGL REQUIRED)
find_package(AntTweakBar REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${ANT_TWEAK_BAR_INCLUDE_PATH}
    "${CMAKE_SOURCE_DIR}/external/glfw-3.1.1/include"
    "${CMAKE_SOURCE_DIR}/external/glm"
    "${CMAKE_SOURCE_DIR}/external/glew-1.12.0/include")

set(LIBS
    ${OPENGL_LIBRARIES}
    ${ANT_TWEAK_BAR_LIBRARY}
    glfw
    GLEW_shared
    )

target_link_libraries(main ${LIBS})

install(TARGETS main DESTINATION ${CMAKE_SOURCE_DIR})
