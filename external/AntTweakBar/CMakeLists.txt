cmake_minimum_required(VERSION 2.8)

project(AntTweakBar)

set(SOURCE_FILES
    src/TwColors.cpp
    src/TwFonts.cpp
    src/TwOpenGL.cpp
    src/TwOpenGLCore.cpp
    src/TwBar.cpp
    src/TwMgr.cpp
    src/TwPrecomp.cpp
    src/LoadOGL.cpp
    src/LoadOGLCore.cpp
    src/TwEventGLFW.c)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-D_MACOSX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-D_UNIX)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(
        -D_WINDOWS
        -DTW_EXPORTS
        -DTW_NO_LIB_PRAGMA
        -DTW_NO_DIRECT3D)
endif()

include_directories("include/")

add_library(AntTweakBar SHARED ${SOURCE_FILES})
set_target_properties(AntTweakBar PROPERTIES OUTPUT_NAME AntTweakBar64)

install(TARGETS AntTweakBar LIBRARY DESTINATION ${CMAKE_SOURCE_DIR} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR} OPTIONAL)

#target_link_libraries(AntTweakBar ${OPENGL_LIBRARY})

