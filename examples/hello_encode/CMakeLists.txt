# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET hello_encode)

set(SOURCES main.cpp)

add_executable(${TARGET} ${SOURCES})

target_link_libraries(${TARGET} mfx)
