cmake_minimum_required(VERSION 2.8)

project(partviewer3D-GLSL)

include_directories(.)
set(CMAKE_CXX_FLAGS "-Wall -O3 -g -march=native")

set(SOURCE_FILES
    src/main.cpp
    src/coord_parser.cpp
    src/keyboard.cpp
    src/mouse.cpp
    src/opengl_3.cpp
    src/shadowmap.cpp
    src/g-buffer.cpp
    src/light.cpp
    src/mesh.cpp
    src/obj_parser.cpp
    src/shader.cpp
    src/ssao.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(AntTweakBar REQUIRED)
find_package(GLM REQUIRED)

include_directories(${GLM_INCLUDE_DIRS})

set(LIBS
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${ANT_TWEAK_BAR_LIBRARY}
    )

add_executable(main ${SOURCE_FILES})
target_link_libraries(main ${LIBS})

