cmake_minimum_required(VERSION 3.10.2)
project(hello-dpcpp)

# This sample must be compiled and executed within oneAPI enviroment by using
# dpcpp compiler
if(DEFINED ENV{DPCT_BUNDLE_ROOT})
  message("Building hello-dpcpp with dpcpp compiler")
  if(WIN32)
    set(CMAKE_CXX_COMPILER "dpcpp-cl")
  else()
    set(CMAKE_CXX_COMPILER "dpcpp")
  endif()
else()
  message("DPCPP compiler not found, blur feature disabled in hello-dpcpp")
endif()

set(TARGET hello-dpcpp)
set(SOURCES src/hello-dpcpp.cpp)
set(VPL_NAME mfx)
set(CMAKE_MSVCIDE_RUN_PATH $ENV{PATH})
set(RUNARGS ${CMAKE_CURRENT_SOURCE_DIR}/content/cars_128x96.i420 128 96)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

# Set default build type to RelWithDebInfo if not specified
if(NOT CMAKE_BUILD_TYPE)
  message(
    STATUS "Default CMAKE_BUILD_TYPE not set using Release with Debug Info")
  set(CMAKE_BUILD_TYPE
      "RelWithDebInfo"
      CACHE
        STRING
        "Choose build type from: None Debug Release RelWithDebInfo MinSizeRel"
        FORCE)
endif()

add_executable(${TARGET} ${SOURCES})

get_directory_property(has_parent PARENT_DIRECTORY)
if(NOT has_parent)
  # only make run target available for stand-alone build
  add_custom_target(run ${TARGET} ${RUNARGS})
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(${TARGET} PRIVATE ${CMAKE_SOURCE_DIR}/api)

if(DEFINED ENV{DPCT_BUNDLE_ROOT})
  add_definitions(-DBUILD_DPCPP)
  target_link_libraries(${TARGET} mfx OpenCL sycl)
else()
  target_link_libraries(${TARGET} ${VPL_LIB} mfx)
endif()
