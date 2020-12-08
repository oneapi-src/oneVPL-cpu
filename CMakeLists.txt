# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script
cmake_minimum_required(VERSION 3.10.2)

set(CMAKE_NO_SYSTEM_FROM_IMPORTED
    ON
    CACHE BOOL "Do not treat contents of the
  INTERFACE_INCLUDE_DIRECTORIES of IMPORTED targets as system includes" FORCE)

file(STRINGS "version.txt" version_txt)
project(oneVPL-cpu VERSION ${version_txt})
if(NOT "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  message(FATAL_ERROR "Unsupported architecture: only 64-bit supported")
endif()

#
# Project configuration options
#

# Project options
option(OPTION_BUILD_TESTS "Build tests." ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/oneAPIInstallDirs.cmake)

# Keep executable bits on shared objects when installing regardless of distro
set(CMAKE_INSTALL_SO_NO_EXE 0)

include(cmake/CompileOptions.cmake)

add_subdirectory(cpu)

install(
  FILES third-party-programs.txt
  DESTINATION ${CMAKE_INSTALL_LICENSEDIR}
  COMPONENT license)

install(
  FILES LICENSE
  RENAME license.txt
  DESTINATION ${CMAKE_INSTALL_LICENSEDIR}
  COMPONENT license)

include(cmake/PackageTarget.cmake)

option(vpl_build_tests "Build the tests" ON)
if(vpl_build_tests)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test/unit)
endif()
