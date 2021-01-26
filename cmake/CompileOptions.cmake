# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set compilation options
#
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  if(UNIX)
    set(CMAKE_CXX_FLAGS "-O0 -g ${CMAKE_CXX_FLAGS}")
  endif(UNIX)
endif()

if(MSVC)
  add_link_options("/DYNAMICBASE")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    add_link_options("/HIGHENTROPYVA")
  endif()
  add_link_options("/LARGEADDRESSAWARE")
  add_link_options("/NXCOMPAT")
  add_compile_options("/GS")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"SAFESEH:NO\"")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")
  endif()
else()
  add_compile_options("-Wformat")
  add_compile_options("-Wformat-security")
  add_compile_options("-Werror=format-security")
  add_definitions("-D_FORTIFY_SOURCE=2")
  add_compile_options("-fstack-protector-strong")
  set(CMAKE_CXX_FLAGS "-z relro -z now -z noexecstack")
  add_compile_options("-Wall")
endif()
