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

set(SOURCE_FILES
    main.cpp
    api/test_bitstream_96x64_8bit_hevc.cpp
    api/test_bitstream_96x64_10bit_hevc.cpp
    api/test_bitstream_32x32_mjpeg.cpp
    api/x_getvideoparam.cpp
    api/core.cpp
    api/x_query.cpp
    api/session_management.cpp
    api/memory.cpp
    api/x_init_reset_close.cpp
    api/x_xframeasync.cpp
    api/x_queryiosurf.cpp
    api/decodeheader.cpp
    api/x_notimplemented.cpp)

add_executable(${TARGET} ${SOURCE_FILES})
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 14)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(STATUS "Linking vpl-utest against vplswref64")
  set(DEPENDENT_LIBS vplswref64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  message(STATUS "Linking vpl-utest against vplswref32")
  set(DEPENDENT_LIBS vplswref32)
endif()
target_link_libraries(${TARGET} ${DEPENDENT_LIBS})
find_package(VPL REQUIRED)
target_link_libraries(${TARGET} VPL::api)

target_link_libraries(${TARGET} gtest)
target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/test/unit)
# gtest_add_tests instead of gtest_discover_tests(${TARGET}) allows building
# test list without loading the dispatcher
gtest_add_tests(TARGET ${TARGET})
