# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

project(oneVPL VERSION 2021.1.7)

#
# Project configuration options
#

# Project options
option(OPTION_BUILD_TESTS "Build tests." ON)
option(OPTION_BUILD_DOCS "Build documentation." OFF)
option(OPTION_BUILD_EXAMPLES "Build examples." OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/oneAPIInstallDirs.cmake)

# Keep executable bits on shared objects when installing regardless of distro
set(CMAKE_INSTALL_SO_NO_EXE 0)

include(cmake/CompileOptions.cmake)

add_subdirectory(libvpl)
add_subdirectory(examples)

include(cmake/PackageTarget.cmake)
