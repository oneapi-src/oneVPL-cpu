# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

add_library(vpl-api INTERFACE)

target_include_directories(vpl-api INTERFACE .)

install(
  DIRECTORY vpl
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)
