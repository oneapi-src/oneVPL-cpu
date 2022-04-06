# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
# oneAPI Video Processing Library (oneVPL) main build script
if(MSVC)
  cmake_minimum_required(VERSION 3.15)
  cmake_policy(SET CMP0091 NEW)
else()
  cmake_minimum_required(VERSION 3.10.2)
endif()

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
option(BUILD_GPL_X264 "Build GPL X264" OFF)
option(BUILD_OPENH264 "Build openH264" OFF)
option(USE_EXPERIMENTAL_API "Enable oneVPL Experimental API." ON)
option(USE_ONEAPI_INSTALL_LAYOUT "Use oneAPI install layout instead of FHS" OFF)
option(USE_MSVC_STATIC_RUNTIME
       "Link MSVC runtime statically to all components." OFF)

if(USE_MSVC_STATIC_RUNTIME)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
if(USE_ONEAPI_INSTALL_LAYOUT)
  set(CMAKE_INSTALL_DOCDIR "documentation")
  set(CMAKE_INSTALL_LIBDIR "lib")
  set(ONEAPI_INSTALL_ENVDIR "env")
  set(ONEAPI_INSTALL_SYSCHECKDIR "sys_check")
  set(ONEAPI_INSTALL_MODFILEDIR "modulefiles")
  set(ONEAPI_INSTALL_EXAMPLEDIR "examples")
  set(ONEAPI_INSTALL_LICENSEDIR "licensing/oneVPL-CPU")
  set(ONEAPI_INSTALL_PYTHONDIR "python/lib")
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/oneAPIInstallDirs.cmake)

# Set POSITION_INDEPENDENT_CODE property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE true)

# Keep executable bits on shared objects when installing regardless of distro
set(CMAKE_INSTALL_SO_NO_EXE 0)

include(cmake/CompileOptions.cmake)

if(NOT BUILD_GPL_X264)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BUILD_OPENH264 ON)
  endif()
endif()

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
include(InstallRequiredSystemLibraries)

if(BUILD_TESTS)
  include(Dart)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(test/unit)
endif()

message(
  STATUS "---------------- Configuration summary ------------------------------"
)
message(STATUS "CMake:")
message(STATUS "  CMAKE_VERSION                  : ${CMAKE_VERSION}")
message(STATUS "  CMAKE_GENERATOR                : ${CMAKE_GENERATOR}")
message(STATUS "  CMAKE_BUILD_TOOL               : ${CMAKE_BUILD_TOOL}")
message(STATUS "Target:")
message(STATUS "  CMAKE_SYSTEM_NAME              : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  CMAKE_SYSTEM_VERSION           : ${CMAKE_SYSTEM_VERSION}")
message(STATUS "  CMAKE_SYSTEM_PROCESSOR         : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "General:")
message(STATUS "  CMAKE_BUILD_TYPE               : ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_TOOLCHAIN_FILE           : ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "  CMAKE_C_COMPILER               : ${CMAKE_C_COMPILER}")
message(STATUS "  CMAKE_CXX_COMPILER             : ${CMAKE_CXX_COMPILER}")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(STATUS "  Build architecture             : 64-bit")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(STATUS "  Build architecture             : 32-bit")
endif()
message(STATUS "Flags:")
message(STATUS "  CMAKE_C_FLAGS                  : ${CMAKE_C_FLAGS}")
message(STATUS "  CMAKE_CXX_FLAGS                : ${CMAKE_CXX_FLAGS}")
message(STATUS "Release flags:")
message(STATUS "  CMAKE_C_FLAGS_RELEASE          : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "  CMAKE_CXX_FLAGS_RELEASE        : ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Debug flags:")
message(STATUS "  CMAKE_C_FLAGS_DEBUG            : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "  CMAKE_CXX_FLAGS_DEBUG          : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "RelWithDebRT flags:")
message(
  STATUS "  CMAKE_C_FLAGS_RELWITHDEBRT     : ${CMAKE_C_FLAGS_RELWITHDEBRT}")
message(
  STATUS "  CMAKE_CXX_FLAGS_RELWITHDEBRT   : ${CMAKE_CXX_FLAGS_RELWITHDEBRT}")
message(STATUS "Install:")
message(STATUS "  CMAKE_INSTALL_PREFIX           : ${CMAKE_INSTALL_PREFIX}")
message(
  STATUS "  CMAKE_INSTALL_FULL_BINDIR      : ${CMAKE_INSTALL_FULL_BINDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_INCLUDEDIR  : ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_LIBDIR      : ${CMAKE_INSTALL_FULL_LIBDIR}")
message(
  STATUS "  CMAKE_INSTALL_FULL_DOCDIR      : ${CMAKE_INSTALL_FULL_DOCDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_ENVDIR     : ${ONEAPI_INSTALL_FULL_ENVDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_SYSCHECKDIR: ${ONEAPI_INSTALL_FULL_SYSCHECKDIR}"
)
message(
  STATUS "  ONEAPI_INSTALL_FULL_MODFILEDIR : ${ONEAPI_INSTALL_FULL_MODFILEDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_EXAMPLEDIR : ${ONEAPI_INSTALL_FULL_EXAMPLEDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_LICENSEDIR : ${ONEAPI_INSTALL_FULL_LICENSEDIR}")
message(
  STATUS "  ONEAPI_INSTALL_FULL_PYTHONDIR  : ${ONEAPI_INSTALL_FULL_PYTHONDIR}")
message(STATUS "Build:")
message(STATUS "  BUILD_TESTS                        : ${BUILD_TESTS}")
message(STATUS "  BUILD_GPL_X264                     : ${BUILD_GPL_X264}")
message(STATUS "  USE_EXPERIMENTAL_API               : ${USE_EXPERIMENTAL_API}")

if(MSVC)
  message(
    STATUS "  USE_MSVC_STATIC_RUNTIME        : ${USE_MSVC_STATIC_RUNTIME}")
endif()
