# Copyright (C) 2007-2013 LuaDist.
# Created by Peter Drahoš, Peter Kapec
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( lua C )
cmake_minimum_required ( VERSION 2.8 )

## CONFIGURATION
# Default configuration (we assume POSIX by default)
set ( LUA_PATH "LUA_PATH") #Environment variable to use as package.path.
set ( LUA_CPATH "LUA_CPATH") #Environment variable to use as package.cpath.
set ( LUA_INIT "LUA_INIT") #Environment variable for initial script.

set ( LUA_ANSI OFF ) #"Use only ansi features." 
set ( LUA_USE_RELATIVE_LOADLIB ON ) #Use modified loadlib.c with support for relative paths on posix systems. 
set ( LUA_COMPAT_ALL ON ) #Enable backwards compatibility options. 
set ( LUA_IDSIZE 60 ) #Gives the maximum size for the description of the source. 

#2DO: LUAI_* and LUAL_* settings, for now defaults are used.
set ( LUA_DIRSEP "/" )
set ( LUA_MODULE_SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX} )
set ( LUA_LDIR ${INSTALL_LMOD} )
set ( LUA_CDIR ${INSTALL_CMOD} )

if ( LUA_USE_RELATIVE_LOADLIB )
  # This will set up relative paths to lib
  string ( REGEX REPLACE "[^!/]+" ".." LUA_DIR "!/${INSTALL_BIN}/" )
else ( )
  # Direct path to installation
  set ( LUA_DIR ${CMAKE_INSTALL_PREFIX}) #Destination from which modules will be resolved. See INSTALL_LMOD and INSTALL_CMOD. 
endif ( )

set ( LUA_PATH_DEFAULT "./?.lua;${LUA_DIR}${LUA_LDIR}/?.lua;${LUA_DIR}${LUA_LDIR}/?/init.lua" )
set ( LUA_CPATH_DEFAULT "./?${LUA_MODULE_SUFFIX};${LUA_DIR}${LUA_CDIR}/?${LUA_MODULE_SUFFIX};${LUA_DIR}${LUA_CDIR}/loadall${LUA_MODULE_SUFFIX}" )

if ( WIN32 AND NOT CYGWIN )
  # Windows systems
  set ( LUA_WIN ON ) #Windows specific build. 
  set ( LUA_BUILD_AS_DLL ${BUILD_SHARED_LIBS} ) #Build Lua library as Dll. 

  # Paths (Double escapes ne  option needed)
  set ( LUA_DIRSEP "\\\\" )
  string ( REPLACE " /" ${LUA_DIRSEP} LUA_DIR "${LUA_DIR}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_LDIR "${LUA_LDIR}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_CDIR "${LUA_CDIR}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_PATH_DEFAULT "${LUA_PATH_DEFAULT}" )
  string ( REPLACE "/" ${LUA_DIRSEP} LUA_CPATH_DEFAULT "${LUA_CPATH_DEFAULT}" )
else ( )
  # Posix systems (incl. Cygwin)
  set ( LUA_USE_POSIX ON ) #Use POSIX features. 
  set ( LUA_USE_DLOPEN ON ) #Use dynamic linker to load modules. 
  set ( LUA_USE_MKSTEMP ON ) #Use mkstep. 
  set ( LUA_USE_ISATTY ON ) #Use tty. 
  set ( LUA_USE_POPEN ON ) #Use popen. 
  set ( LUA_USE_ULONGJMP ON ) #Use ulongjmp 
  set ( LUA_USE_GMTIME_R ON ) #Use GTIME_R 
  # Apple and Linux specific
  if ( LINUX OR APPLE )
      set ( LUA_USE_STRTODHEX ON ) #Assume 'strtod' handles hexa formats 
      set ( LUA_USE_AFORMAT ON ) #Assume 'printf' handles 'aA' specifiers 
      set ( LUA_USE_LONGLONG ON ) #Assume support for long long 
  endif ( )
endif ( )

## SETUP
# Setup needed variables and libraries
if ( LUA_USE_POSIX )
  # On POSIX Lua links to standard math library "m"
  list ( APPEND LIBS m )
  add_definitions(-DLUA_USE_POSIX)
endif ( )

if ( LUA_USE_DLOPEN )
  # Link to dynamic linker library "dl"
    find_library ( DL_LIBRARY NAMES dl )
    if ( DL_LIBRARY )
    list ( APPEND LIBS ${DL_LIBRARY} )
    endif ( )
    add_definitions(-DLUA_USE_DLOPEN)
endif ( )


# Sources and headers
include_directories ( src ${CMAKE_CURRENT_BINARY_DIR} )
set ( SRC_CORE src/lapi.c src/lcode.c src/lctype.c src/ldebug.c src/ldo.c src/ldump.c
  src/lfunc.c src/lgc.c src/llex.c src/lmem.c src/lobject.c src/lopcodes.c src/lparser.c
  src/lstate.c src/lstring.c src/ltable.c src/ltm.c src/lundump.c src/lvm.c src/lzio.c )
set ( SRC_LIB src/lauxlib.c src/lbaselib.c src/lbitlib.c src/lcorolib.c src/ldblib.c
  src/liolib.c src/lmathlib.c src/loslib.c src/lstrlib.c src/ltablib.c src/linit.c src/loadlib.c)
set ( SRC_LUA src/lua.c )
set ( SRC_LUAC src/luac.c )

## BUILD
# Create lua library
if(${CMAKE_C_COMPILER_ID} MATCHES "GNU" OR
   ${CMAKE_C_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_C_FLAGS "-Wall -O3")
endif()
add_library ( liblua SHARED ${SRC_CORE} ${SRC_LIB} ${LUA_DLL_RC} ${LUA_DEF} )
target_link_libraries ( liblua ${LIBS} )
set_target_properties ( liblua PROPERTIES OUTPUT_NAME lua CLEAN_DIRECT_OUTPUT 1 )
if ( LUA_BUILD_AS_DLL )
  set_target_properties ( liblua PROPERTIES COMPILE_DEFINITIONS LUA_BUILD_AS_DLL )
endif ()

install(TARGETS liblua LIBRARY DESTINATION ${CMAKE_SOURCE_DIR} RUNTIME DESTINATION ${CMAKE_SOURCE_DIR})
