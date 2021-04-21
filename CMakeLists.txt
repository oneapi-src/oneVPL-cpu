# ##############################################################################
# Copyright (C) Intel Corporation
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

#
# Project configuration options
#

# Project options
option(BUILD_TESTS "Build tests." ON)
option(USE_ONEAPI_INSTALL_LAYOUT "Use oneAPI install layout instead of FHS" OFF)

if(USE_ONEAPI_INSTALL_LAYOUT)
  set(CMAKE_INSTALL_DOCDIR "documentation")
  set(CMAKE_INSTALL_LIBDIR "lib")
  set(ONEAPI_INSTALL_ENVDIR "env")
  set(ONEAPI_INSTALL_SYSCHECKDIR "sys_check")
  set(ONEAPI_INSTALL_MODFILEDIR "modulefiles")
  set(ONEAPI_INSTALL_EXAMPLEDIR "examples")
  set(ONEAPI_INSTALL_LICENSEDIR "licensing")
  set(ONEAPI_INSTALL_PYTHONDIR "python/lib")
endif()

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
  DESTINATION ${ONEAPI_INSTALL_LICENSEDIR}
  COMPONENT license)

install(
  FILES LICENSE
  RENAME license.txt
  DESTINATION ${ONEAPI_INSTALL_LICENSEDIR}
  COMPONENT license)

include(cmake/PackageTarget.cmake)

if(BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test/unit)
endif()
