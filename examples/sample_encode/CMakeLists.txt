# ##############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET sample_encode)

set(SOURCES
 src/pipeline_encode.cpp
 src/pipeline_user.cpp
 src/sample_encode.cpp
)

add_executable(${TARGET} ${SOURCES})

target_include_directories(
  ${TARGET}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../sample_common/include>
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../libvpl/include/vpl>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if(UNIX)
  set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(${TARGET} sample_common mfx Threads::Threads dl)
else()
  target_link_libraries(${TARGET} sample_common libmfx)
endif()

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT dev)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT dev)
