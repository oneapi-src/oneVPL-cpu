# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_common)

set(SOURCES
    src/avc_bitstream.cpp
    src/avc_nal_spl.cpp
    src/avc_spl.cpp
    src/base_allocator.cpp
    src/brc_routines.cpp
    src/d3d_allocator.cpp
    src/d3d_device.cpp
    src/d3d11_allocator.cpp
    src/d3d11_device.cpp
    src/decode_render.cpp
    src/general_allocator.cpp
    src/mfx_buffering.cpp
    src/parameters_dumper.cpp
    src/preset_manager.cpp
    src/sample_utils.cpp
    src/sysmem_allocator.cpp
    src/vpp_ex.cpp
    src/vm/atomic.cpp
    src/vm/shared_object.cpp
    src/vm/thread.cpp
    src/vm/thread_windows.cpp
    src/vm/time.cpp)

set(WINDOWS_LIBS dxva2.lib d3d9.lib dwmapi.lib comsuppw.lib d3d11.lib dxgi.lib)

add_library(${TARGET} STATIC)

target_sources(${TARGET} PRIVATE ${SOURCES})

if(UNIX)
  target_sources(
    sample_common
    PRIVATE src/vm/atomic_linux.cpp src/vm/shared_object_linux.cpp
            src/vm/thread_linux.cpp src/vm/time_linux.cpp)
endif()

target_include_directories(
  ${TARGET}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libvpl/include/vpl>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if(WIN32)
  target_link_libraries(${TARGET} ${WINDOWS_LIBS})
endif()

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DNOMINMAX)

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)
