# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################
cmake_minimum_required(VERSION 3.10.2)

set(TARGET ffmpeg-svt)

add_library(${TARGET} INTERFACE)

if(NOT DEFINED ENV{VPL_BUILD_DEPENDENCIES})
  set(VPL_DEP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../_deps)
  message(STATUS "Using default destination located at ${VPL_DEP_DIR}")
else()
  set(VPL_DEP_DIR $ENV{VPL_BUILD_DEPENDENCIES})
  message(STATUS "Using VPL_BUILD_DEPENDENCIES located at ${VPL_DEP_DIR}")
endif()

option(BUILD_GPL_X264 "Build X264 with GPL License" OFF)

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
  set(X264_LIB ${VPL_DEP_DIR}/lib/libx264.a)
  if(NOT EXISTS ${X264_LIB})
    message(FATAL_ERROR "Could not find expected FFmpeg libraries")
  endif()
endif(BUILD_GPL_X264)

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

if(MSVC)
  if(NOT DEFINED ENV{MSYS_ROOT})
    message(STATUS "MSYS_ROOT not defined, assuming C:/tools/msys64")
    set(MSYS_ROOT C:/tools/msys64)
  else()
    set(MSYS_ROOT $ENV{MSYS_ROOT})
  endif()

  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(mingw_name mingw64)
    set(gcc_name x86_64-w64-mingw32)
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(mingw_name mingw32)
    set(gcc_name i686-w64-mingw32)
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
  set(BCRYPT_LIB ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libbcrypt.a)
  set(MINGW32_LIB ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libmingw32.a)
  set(MINGWEX_LIB ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libmingwex.a)
  set(PTHREAD_IMPLIB
      ${MSYS_ROOT}/${mingw_name}/${gcc_name}/lib/libpthread.dll.a)
  set(PTHREAD_DLL ${MSYS_ROOT}/${mingw_name}/bin/libwinpthread-1.dll)

  if(NOT EXISTS ${GCC_LIB})
    message(FATAL_ERROR "Could not find expected MinGW GCC runtime libraries")
  endif()
  if(NOT EXISTS ${GCC_EH_LIB})
    message(
      FATAL_ERROR "Could not find expected MinGW GCC EH runtime libraries")
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

  target_link_libraries(
    ${TARGET} INTERFACE ${GCC_LIB} ${GCC_EH_LIB} ${BCRYPT_LIB} ${MINGW32_LIB}
                        ${MINGWEX_LIB})

  if(NOT EXISTS ${PTHREAD_IMPLIB})
    message(FATAL_ERROR "Could not find expected MinGW runtime libraries")
  endif()
  target_link_libraries(${TARGET} INTERFACE ${PTHREAD_IMPLIB})

  install(
    PROGRAMS ${PTHREAD_DLL}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime)
  # copy pthread dll to output directory so bins work without setting paths
  add_custom_target(mingw-pthread)
  add_dependencies(${TARGET} mingw-pthread)
  add_custom_command(
    TARGET mingw-pthread
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
  add_custom_command(
    TARGET mingw-pthread
    COMMAND ${CMAKE_COMMAND} -E copy ${PTHREAD_DLL}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)

endif()

set_property(TARGET ${TARGET} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                                       ${VPL_DEP_DIR}/include)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if(MSVC)
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    target_link_libraries(
      ${TARGET} INTERFACE ${AVCODEC_LIB} ${AVUTIL_LIB} ${AVFILTER_LIB}
                          ${SWSCALE_LIB} Threads::Threads)
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_link_libraries(
      ${TARGET}
      INTERFACE ${AVCODEC_LIB}
                ${AVUTIL_LIB}
                ${AVFILTER_LIB}
                ${SWSCALE_LIB}
                ${SVTHEVCENC_LIB}
                ${SVTAV1ENC_LIB}
                ${DAV1D_LIB}
                Threads::Threads)
  endif()
else()
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    target_link_libraries(
      ${TARGET} INTERFACE ${AVCODEC_LIB} ${AVUTIL_LIB} ${AVFILTER_LIB}
                          ${SWSCALE_LIB} Threads::Threads ${CMAKE_DL_LIBS})
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_link_libraries(
      ${TARGET}
      INTERFACE ${AVCODEC_LIB}
                ${AVUTIL_LIB}
                ${AVFILTER_LIB}
                ${SWSCALE_LIB}
                ${SVTHEVCENC_LIB}
                ${SVTAV1ENC_LIB}
                ${DAV1D_LIB}
                Threads::Threads
                ${CMAKE_DL_LIBS})
  endif()
endif()

if(BUILD_GPL_X264)
  if(MSVC)
    target_link_libraries(${TARGET} INTERFACE ${X264_LIB} Threads::Threads)
  else()
    target_link_libraries(${TARGET} INTERFACE ${X264_LIB} Threads::Threads
                                              ${CMAKE_DL_LIBS})
  endif()
endif(BUILD_GPL_X264)
