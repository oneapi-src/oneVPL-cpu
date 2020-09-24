# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

add_executable(vpl-encode vpl-encode.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-encode mfx)

add_executable(vpl-decode vpl-decode.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-decode mfx)

add_executable(vpl-vpp vpl-vpp.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-vpp mfx)

add_executable(vpl-vppenc vpl-vppenc.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-vppenc mfx)

add_executable(vpl-decenc vpl-decenc.cpp vpl-new-dispatcher.cpp)
target_link_libraries(vpl-decenc mfx)

add_executable(vpl-inspect vpl-inspect.cpp)
target_link_libraries(vpl-inspect mfx)

install(TARGETS vpl-encode vpl-decode vpl-vpp vpl-inspect
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)
