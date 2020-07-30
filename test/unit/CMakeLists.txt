# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE)
add_subdirectory(googletest)

set(TARGET vpl-utest)
set(DEPENDENT_LIBS gtest mfx)

set(SOURCE_FILES main.cpp api/session_management.cpp api/core.cpp)

add_executable(${TARGET} ${SOURCE_FILES})
target_link_libraries(${TARGET} ${DEPENDENT_LIBS})
gtest_discover_tests(${TARGET})