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

set(SOURCE_FILES
    main.cpp
    api/adapter.cpp
    api/x_getvideoparam.cpp
    api/core.cpp
    api/x_query.cpp
    api/session_management.cpp
    api/memory.cpp
    api/x_init_reset_close.cpp
    api/smart_dispatcher.cpp
    api/x_xframeasync.cpp
    api/x_queryiosurf.cpp
    api/decodeheader.cpp
    api/x_notimplemented.cpp)

add_executable(${TARGET} ${SOURCE_FILES})
target_link_libraries(${TARGET} ${DEPENDENT_LIBS})
gtest_discover_tests(${TARGET})
