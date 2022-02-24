# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

#
# Set compilation options
#
if(MSVC)
  add_compile_options("$<$<CONFIG:Debug>:/D_DEBUG>")
else()
  add_compile_options("$<$<CONFIG:Debug>:-D_DEBUG -O0 -g>")
endif()

if(ENABLE_WARNING_AS_ERROR)
  message(STATUS "Warnings as errors enabled")
  set(MFX_DEPRECATED_OFF 1)
endif()

if(DEFINED ENV{MFX_DEPRECATED_OFF})
  set(MFX_DEPRECATED_OFF 1)
endif()

if(MFX_DEPRECATED_OFF)
  message(STATUS "Deprecation warnings disabled")
  add_definitions(-DMFX_DEPRECATED_OFF)
endif()

if(MSVC)
  add_link_options("/DYNAMICBASE")
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    add_link_options("/HIGHENTROPYVA")
  endif()
  add_link_options("/LARGEADDRESSAWARE")
  add_link_options("/NXCOMPAT")
  if(ENABLE_WARNING_AS_ERROR)
    add_compile_options("/WX")
  endif()
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
  if(LINUX)
    add_compile_options("-fstack-protector-strong")
    set(CMAKE_CXX_FLAGS "-z relro -z now -z noexecstack")
  endif()
  if(WIN32)
    add_compile_options("-fno-set-stack-executable") # Windows verion of "-z
                                                     # noexecstack"
  endif()
  add_compile_options("-Wall")
  if(ENABLE_WARNING_AS_ERROR)
    add_compile_options("-Werror")
  endif()
endif()
