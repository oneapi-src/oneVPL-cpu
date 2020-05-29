# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET vpl-app)
set(SOURCES vpl-app.cpp)

set(DEPENDENCIES libmfx)

add_executable(${TARGET} ${SOURCES})
add_dependencies(${TARGET} ${DEPENDENCIES})

target_link_libraries(${TARGET} ${DEPENDENCIES})
