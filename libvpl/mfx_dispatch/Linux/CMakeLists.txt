# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET mfx)

set(SOURCES mfxloader.cpp)

add_library(${TARGET} STATIC "")

target_sources(${TARGET} PRIVATE ${SOURCES})

target_include_directories(
  ${TARGET}
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include/vpl>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)
