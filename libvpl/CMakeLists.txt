# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

file(STRINGS "version.txt" version_txt)
project(vpl VERSION ${version_txt})

set(TARGET mfxvplsw64)

if(DEFINED ENV{VPL_BUILD_CACHE_DIR})
    set(VPL_BUILD_CACHE_DIR $ENV{VPL_BUILD_CACHE_DIR})
else()
    set(VPL_BUILD_CACHE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../_vpl_cache/)
endif()
message(STATUS "VPL_BUILD_CACHE_DIR = ${VPL_BUILD_CACHE_DIR}")

set(SOURCES
    src/cpu_common.cpp
    src/cpu_decode.cpp
    src/cpu_encode.cpp
    src/cpu_vpp.cpp
    src/libmfxvplsw.cpp
    src/libmfxvplsw_core.cpp
    src/libmfxvplsw_decode.cpp
    src/libmfxvplsw_encode.cpp
    src/libmfxvplsw_vpp.cpp
    src/libmfx.def)

add_library(${TARGET} SHARED "")

# force correct name on Windows
set_target_properties(${TARGET} PROPERTIES PREFIX "lib")

# add "_d" to debug library name
set_target_properties(${TARGET} PROPERTIES DEBUG_POSTFIX "_d")

target_include_directories(
  ${TARGET}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vpl>
    $<BUILD_INTERFACE:${VPL_BUILD_CACHE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(FFMPEG_LIBS
    ${VPL_BUILD_CACHE_DIR}/lib/libavcodec.a
    ${VPL_BUILD_CACHE_DIR}/lib/libavfilter.a
    ${VPL_BUILD_CACHE_DIR}/lib/libavformat.a
    ${VPL_BUILD_CACHE_DIR}/lib/libavutil.a
    ${VPL_BUILD_CACHE_DIR}/lib/libswscale.a)

if(UNIX)
  # Linux
  set(FFMPEG_LIBS ${FFMPEG_LIBS}
      ${VPL_BUILD_CACHE_DIR}/lib/libSvtHevcEnc.so.1)

  SET_TARGET_PROPERTIES(${TARGET} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic)
else()
  # Windows
  set(FFMPEG_LIBS ${FFMPEG_LIBS}
      ${VPL_BUILD_CACHE_DIR}/lib/libSvtHevcEnc.dll.a)

  # temporary workaround for bootstrap link errors on Windows
  set(ENABLE_MINGW_WORKAROUND TRUE)
  if (ENABLE_MINGW_WORKAROUND)
    set(SOURCES ${SOURCES}
        src/mingw_wa.cpp)

    set(FFMPEG_LIBS ${FFMPEG_LIBS}
        ${VPL_BUILD_CACHE_DIR}/lib/libgcc.a
        ${VPL_BUILD_CACHE_DIR}/lib/libmingwex.a
        ${VPL_BUILD_CACHE_DIR}/lib/libmingw32.a)
  endif()
endif()

target_sources(${TARGET} PRIVATE ${SOURCES})

target_link_libraries(${TARGET} PRIVATE ${FFMPEG_LIBS})

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

if(WIN32)
  set(OUTPATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
  add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${VPL_BUILD_CACHE_DIR}/lib/libSvtHevcEnc.dll ${OUTPATH})
else()
  set(OUTPATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

  add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${VPL_BUILD_CACHE_DIR}/lib/libSvtHevcEnc.so   ${OUTPATH}
    COMMAND ${CMAKE_COMMAND} -E copy ${VPL_BUILD_CACHE_DIR}/lib/libSvtHevcEnc.so.1 ${OUTPATH})
endif()

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)
