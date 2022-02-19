# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

file(STRINGS "version.txt" version_txt)
project(vpl VERSION ${version_txt})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(TARGET vplswref64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET vplswref32)
endif()

# add lib/<arch> to find_package path on windows
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(CMAKE_LIBRARY_ARCHITECTURE x86)
endif()

file(GLOB SOURCES src/*.cpp)

if(WIN32)
  list(APPEND SOURCES src/windows/libmfxsw.def)
endif()

option(BUILD_GPL_X264 "Build X264 with GPL License" OFF)
option(BUILD_OPENH264 "Build openH264 with BSD License" OFF)

add_library(${TARGET} SHARED "")

if(USE_EXPERIMENTAL_API)
  add_definitions(-DONEVPL_EXPERIMENTAL)
endif()

# force correct name on Windows
set_target_properties(${TARGET} PROPERTIES PREFIX "lib")

if(UNIX)
  set_target_properties(${TARGET} PROPERTIES SUFFIX ".so.1")
endif()

target_sources(${TARGET} PRIVATE ${SOURCES})

if(POLICY CMP0074)
  # ignore warning of VPL_ROOT in find_package search path
  cmake_policy(SET CMP0074 OLD)
endif()

find_package(VPL 2.4 REQUIRED COMPONENTS api)
message(STATUS "Found VPL (version ${VPL_VERSION})")
target_link_libraries(${TARGET} PUBLIC VPL::api)

if(NOT DEFINED ENV{VPL_BUILD_DEPENDENCIES})
  set(VPL_DEP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../_deps)
  message(STATUS "Using default destination located at ${VPL_DEP_DIR}")
else()
  set(VPL_DEP_DIR $ENV{VPL_BUILD_DEPENDENCIES})
  message(STATUS "Using VPL_BUILD_DEPENDENCIES located at ${VPL_DEP_DIR}")
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(SVTHEVCENC_LIB ${VPL_DEP_DIR}/lib/libSvtHevcEnc.a)
  set(SVTAV1ENC_LIB ${VPL_DEP_DIR}/lib/libSvtAv1Enc.a)
  set(DAV1D_LIB ${VPL_DEP_DIR}/lib/libdav1d.a)
endif()
set(AVCODEC_LIB ${VPL_DEP_DIR}/lib/libavcodec.a)
set(AVUTIL_LIB ${VPL_DEP_DIR}/lib/libavutil.a)
set(AVFILTER_LIB ${VPL_DEP_DIR}/lib/libavfilter.a)
set(SWSCALE_LIB ${VPL_DEP_DIR}/lib/libswscale.a)

if(BUILD_GPL_X264)
  set(H264_ENC_LIB ${VPL_DEP_DIR}/lib/libx264.a)
  if(NOT EXISTS ${H264_ENC_LIB})
    message(FATAL_ERROR "Could not find x264 libraries")
  else()
    message(STATUS "Building with GPL x264 for AVC implementation")
    add_definitions("-DENABLE_ENCODER_X264")
  endif()
elseif(BUILD_OPENH264)
  set(H264_ENC_LIB ${VPL_DEP_DIR}/lib/libopenh264.a)
  if(NOT EXISTS ${H264_ENC_LIB})
    message(FATAL_ERROR "Could not find openh264 libraries")
  else()
    message(STATUS "Building with openH264 for AVC implementation")
    add_definitions("-DENABLE_ENCODER_OPENH264")
  endif()
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  if(NOT EXISTS ${SVTHEVCENC_LIB}
     OR NOT EXISTS ${SVTHEVCENC_LIB}
     OR NOT EXISTS ${DAV1D_LIB})
    message(FATAL_ERROR "Could not find expected FFmpeg libraries")
  endif()
endif()

if(NOT EXISTS ${AVCODEC_LIB}
   OR NOT EXISTS ${AVUTIL_LIB}
   OR NOT EXISTS ${AVFILTER_LIB}
   OR NOT EXISTS ${SWSCALE_LIB})
  message(FATAL_ERROR "Could not find expected FFmpeg libraries")
endif()

if(WIN32)
  if(NOT DEFINED ENV{MSYS_ROOT})
    message(STATUS "MSYS_ROOT not defined, assuming C:/tools/msys64")
    set(MSYS_ROOT C:/tools/msys64)
  else()
    set(MSYS_ROOT $ENV{MSYS_ROOT})
  endif()

  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(mingw_name mingw64)
    set(gcc_name x86_64-w64-mingw32)
    set(GCC_SEH_DLL ${MSYS_ROOT}/${mingw_name}/bin/libgcc_s_seh-1.dll)
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(mingw_name mingw32)
    set(gcc_name i686-w64-mingw32)
    set(GCC_SEH_DLL ${MSYS_ROOT}/${mingw_name}/bin/libgcc_s_dw2-1.dll)
  else()
    message(STATUS "${CMAKE_SIZEOF_VOID_P} * 8 bit")
    message(FATAL_ERROR "Cannot identify library folder")
  endif()

  execute_process(COMMAND ${MSYS_ROOT}/${mingw_name}/bin/gcc.exe --version
                  OUTPUT_VARIABLE gcc_version_text)
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" gcc_version ${gcc_version_text})

  set(GCC_LIB
      ${MSYS_ROOT}/${mingw_name}/lib/gcc/${gcc_name}/${gcc_version}/libgcc.a)
  set(GCC_EH_LIB
      ${MSYS_ROOT}/${mingw_name}/lib/gcc/${gcc_name}/${gcc_version}/libgcc_eh.a)
  set(STDCPP_LIB
      ${MSYS_ROOT}/${mingw_name}/lib/gcc/${gcc_name}/${gcc_version}/libstdc++.a)
  set(BCRYPT_LIB ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libbcrypt.a)
  set(MINGW32_LIB ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libmingw32.a)
  set(MINGWEX_LIB ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libmingwex.a)
  set(PTHREAD_IMPLIB
      ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libpthread.dll.a)

  set(PTHREAD_DLL ${MSYS_ROOT}/${mingw_name}/bin/libwinpthread-1.dll)
  set(STDCPP_DLL ${MSYS_ROOT}/${mingw_name}/bin/libstdc++-6.dll)

  if(NOT EXISTS ${GCC_LIB})
    message(FATAL_ERROR "Could not find expected MinGW GCC runtime libraries")
  endif()
  if(NOT EXISTS ${GCC_EH_LIB})
    message(
      FATAL_ERROR "Could not find expected MinGW GCC EH runtime libraries")
  endif()
  if(NOT EXISTS ${STDCPP_LIB})
    message(
      FATAL_ERROR "Could not find expected MinGW stdc++ runtime libraries")
  endif()
  if(NOT EXISTS ${BCRYPT_LIB})
    message(
      FATAL_ERROR "Could not find expected MinGW BCrypt runtime libraries")
  endif()
  if(NOT EXISTS ${MINGW32_LIB})
    message(
      FATAL_ERROR "Could not find expected MinGW MinGW32 runtime libraries")
  endif()
  if(NOT EXISTS ${MINGWEX_LIB})
    message(
      FATAL_ERROR "Could not find expected MinGW MinGWEX runtime libraries")
  endif()
  if(NOT EXISTS ${PTHREAD_DLL})
    message(
      FATAL_ERROR "Could not find expected MinGW PThread runtime libraries")
  endif()
  if(NOT EXISTS ${GCC_SEH_DLL})
    message(
      FATAL_ERROR "Could not find expected MinGW GCC SEH runtime libraries")
  endif()
  if(NOT EXISTS ${STDCPP_DLL})
    message(FATAL_ERROR "Could not find expected MinGW stdc++ runtime Dll")
  endif()

  target_link_libraries(
    ${TARGET} PRIVATE ${GCC_LIB} ${GCC_EH_LIB} ${STDCPP_LIB} ${BCRYPT_LIB}
                      ${MINGW32_LIB} ${MINGWEX_LIB})

  if(NOT EXISTS ${PTHREAD_IMPLIB})
    message(FATAL_ERROR "Could not find expected MinGW runtime libraries")
  endif()
  target_link_libraries(${TARGET} PRIVATE ${PTHREAD_IMPLIB})

  install(
    PROGRAMS ${PTHREAD_DLL} ${STDCPP_DLL} ${GCC_SEH_DLL}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime)
  # copy pthread dll to output directory so bins work without setting paths
  add_custom_target(mingw-pthread)
  add_dependencies(${TARGET} mingw-pthread)
  add_custom_command(
    TARGET mingw-pthread
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
  add_custom_command(
    TARGET mingw-pthread
    COMMAND
      ${CMAKE_COMMAND} -E copy ${PTHREAD_DLL} ${STDCPP_DLL} ${GCC_SEH_DLL}
      ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(UNIX)
  add_subdirectory(ext/ffmpeg-svt)
  target_link_libraries(${TARGET} PRIVATE ffmpeg-svt)
else()
  target_include_directories(${TARGET} PRIVATE ${VPL_DEP_DIR}/include)
  target_link_directories(${TARGET} PRIVATE ${VPL_DEP_DIR}/lib)

  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    target_link_libraries(
      ${TARGET} PRIVATE ${AVCODEC_LIB} ${AVUTIL_LIB} ${AVFILTER_LIB}
                        ${SWSCALE_LIB} Threads::Threads)
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_link_libraries(
      ${TARGET}
      PRIVATE ${AVCODEC_LIB}
              ${AVUTIL_LIB}
              ${AVFILTER_LIB}
              ${SWSCALE_LIB}
              ${SVTHEVCENC_LIB}
              ${SVTAV1ENC_LIB}
              ${DAV1D_LIB}
              Threads::Threads)
  endif()

  target_link_libraries(${TARGET} PRIVATE ${H264_ENC_LIB} ${BCRYPT_LIB}
                                          Threads::Threads)
endif()

target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                             ${CMAKE_CURRENT_BINARY_DIR})

target_compile_definitions(
  ${TARGET}
  PRIVATE -DVPL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
          -DVPL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
          -DVPL_VERSION_PATCH=${PROJECT_VERSION_PATCH})

if(UNIX)
  set_target_properties(${TARGET} PROPERTIES LINK_FLAGS -Wl,-Bsymbolic,-z,defs)
endif()

install(
  TARGETS ${TARGET}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT runtime
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)
