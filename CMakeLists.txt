cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

project(partviewer3D-GLSL)


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

add_executable(main ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "-Wall -O3 -march=native")


find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(GLM REQUIRED)
find_package(AntTweakBar REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${GLM_INCLUDE_DIRS} ${ANT_TWEAK_BAR_INCLUDE_PATH})

set(LIBS
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${ANT_TWEAK_BAR_LIBRARY}
    )

target_link_libraries(main ${LIBS})

install(TARGETS main DESTINATION ${CMAKE_SOURCE_DIR})
