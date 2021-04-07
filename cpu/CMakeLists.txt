# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

file(STRINGS "version.txt" version_txt)
project(vpl VERSION ${version_txt})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET vplswref64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET vplswref32)
endif()

file(GLOB SOURCES src/*.cpp)

if(WIN32)
  list(APPEND SOURCES src/windows/libmfxsw.def)
endif()

add_library(${TARGET} SHARED "")

# force correct name on Windows
set_target_properties(${TARGET} PROPERTIES PREFIX "lib")

if(UNIX)
  set_target_properties(${TARGET} PROPERTIES SUFFIX ".so.1")
endif()

target_sources(${TARGET} PRIVATE ${SOURCES})

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

find_package(VPL 2.2 REQUIRED COMPONENTS api)
target_link_libraries(${TARGET} PUBLIC VPL::api)

if(BUILD_GPL_X264)
  add_definitions("-DENABLE_ENCODER_H264")
endif(BUILD_GPL_X264)

add_subdirectory(ext/ffmpeg-svt)

target_link_libraries(${TARGET} PRIVATE ffmpeg-svt)

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

if(UNIX)
  set_target_properties(${TARGET} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic,-z,defs)
endif()

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)
