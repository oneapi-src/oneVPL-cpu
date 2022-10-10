# ##############################################################################
# Copyright (C) Intel Corporation
#
# SPDX-License-Identifier: MIT
# ##############################################################################

# This is WA until we have a solution to find right library location from gcc
# output.
#
# findMingwLib() looks for varLibName from varMingwPath + varGccPath folder at
# first, and if it fails then, it looks for varMingwPath + 'lib' path Because we
# observed only 2 locations for libs so far.
#
function(findMingwLib varResult varMingwPath varGccPath varLibName)
  set(fileName ${varMingwPath}/${varGccPath}/${varLibName})
  if(NOT EXISTS ${fileName})
    set(fileName ${varMingwPath}/lib/${varLibName})
  endif()
  set(${varResult}
      ${fileName}
      PARENT_SCOPE)
endfunction()

cmake_minimum_required(VERSION 3.10.2)

set(TARGET ffmpeg-codecs)

add_library(${TARGET} INTERFACE)

# Set VPL dependencies directory
if(NOT DEFINED ENV{VPL_BUILD_DEPENDENCIES})
  set(VPL_DEP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../_deps)
  message(STATUS "Using default destination located at ${VPL_DEP_DIR}")
else()
  set(VPL_DEP_DIR $ENV{VPL_BUILD_DEPENDENCIES})
  message(STATUS "Using VPL_BUILD_DEPENDENCIES located at ${VPL_DEP_DIR}")
endif()

# Set basic FFmpeg and codec libs
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(SVTHEVCENC_LIB ${VPL_DEP_DIR}/lib/libSvtHevcEnc.a)
  set(SVTAV1ENC_LIB ${VPL_DEP_DIR}/lib/libSvtAv1Enc.a)
  set(DAV1D_LIB ${VPL_DEP_DIR}/lib/libdav1d.a)

  if(NOT EXISTS ${SVTHEVCENC_LIB}
     OR NOT EXISTS ${SVTAV1ENC_LIB}
     OR NOT EXISTS ${DAV1D_LIB})
    message(FATAL_ERROR "Could not find expected codec libraries")
  endif()
endif()

set(AVCODEC_LIB ${VPL_DEP_DIR}/lib/libavcodec.a)
set(AVUTIL_LIB ${VPL_DEP_DIR}/lib/libavutil.a)
set(AVFILTER_LIB ${VPL_DEP_DIR}/lib/libavfilter.a)
set(SWSCALE_LIB ${VPL_DEP_DIR}/lib/libswscale.a)

if(NOT EXISTS ${AVCODEC_LIB}
   OR NOT EXISTS ${AVUTIL_LIB}
   OR NOT EXISTS ${AVFILTER_LIB}
   OR NOT EXISTS ${SWSCALE_LIB})
  message(FATAL_ERROR "Could not find expected FFmpeg libraries")
endif()

set_property(TARGET ${TARGET} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                                       ${VPL_DEP_DIR}/include)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Add basic FFmpeg and codec libs
if(WIN32)
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    target_link_libraries(
      ${TARGET} INTERFACE ${AVFILTER_LIB} ${AVCODEC_LIB} ${AVUTIL_LIB}
                          ${SWSCALE_LIB} Threads::Threads)
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_link_libraries(
      ${TARGET}
      INTERFACE ${AVFILTER_LIB}
                ${AVCODEC_LIB}
                ${AVUTIL_LIB}
                ${SWSCALE_LIB}
                ${SVTHEVCENC_LIB}
                ${SVTAV1ENC_LIB}
                ${DAV1D_LIB}
                Threads::Threads)
  endif()
else()
  if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    target_link_libraries(
      ${TARGET} INTERFACE ${AVFILTER_LIB} ${AVCODEC_LIB} ${AVUTIL_LIB}
                          ${SWSCALE_LIB} Threads::Threads ${CMAKE_DL_LIBS})
  elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    target_link_libraries(
      ${TARGET}
      INTERFACE ${AVFILTER_LIB}
                ${AVCODEC_LIB}
                ${AVUTIL_LIB}
                ${SWSCALE_LIB}
                ${SVTHEVCENC_LIB}
                ${SVTAV1ENC_LIB}
                ${DAV1D_LIB}
                Threads::Threads
                ${CMAKE_DL_LIBS})
  endif()
endif()

# Get Mingw info, common to all cases if OS is Windows
if(WIN32)
  if(NOT DEFINED ENV{MSYS_ROOT})
    message(STATUS "MSYS_ROOT not defined, assuming C:/tools/msys64")
    set(MSYS_ROOT C:/tools/msys64)
  else()
    file(TO_CMAKE_PATH "$ENV{MSYS_ROOT}" MSYS_ROOT)
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

  set(MINGW_PATH ${MSYS_ROOT}/${mingw_name})

  execute_process(COMMAND ${MINGW_PATH}/bin/gcc.exe --version
                  OUTPUT_VARIABLE gcc_version_text)
  string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" gcc_version ${gcc_version_text})
endif()

# Set AVC encoder lib name
if(BUILD_GPL_X264)
  set(H264_ENC_LIB ${VPL_DEP_DIR}/lib/libx264.a)
  if(NOT EXISTS ${H264_ENC_LIB})
    message(FATAL_ERROR "Could not find x264 libraries")
  else()
    message(STATUS "Building with GPL x264 for AVC implementation")
  endif()
elseif(BUILD_OPENH264)
  set(H264_ENC_LIB ${VPL_DEP_DIR}/lib/libopenh264.a)
  if(NOT EXISTS ${H264_ENC_LIB})
    message(FATAL_ERROR "Could not find openh264 libraries")
  else()
    message(STATUS "Building with openH264 for AVC implementation")
  endif()
endif()

# Add AVC encoder libs
target_link_libraries(${TARGET} INTERFACE ${H264_ENC_LIB})

if(WIN32)
  # openH264 lib dependencies
  if(BUILD_OPENH264)
    findmingwlib(STDCPP_LIB ${MINGW_PATH} lib/gcc/${gcc_name}/${gcc_version}
                 libstdc++.a)
    if(NOT EXISTS ${STDCPP_LIB})
      message(
        FATAL_ERROR "Could not find expected MinGW stdc++ runtime libraries")
    endif()

    # Add openH264 dependencies
    target_link_libraries(${TARGET} INTERFACE ${STDCPP_LIB})
  endif()

  # Common libs from mingw
  findmingwlib(GCC_LIB ${MINGW_PATH} lib/gcc/${gcc_name}/${gcc_version}
               libgcc.a)
  findmingwlib(GCC_EH_LIB ${MINGW_PATH} lib/gcc/${gcc_name}/${gcc_version}
               libgcc_eh.a)

  findmingwlib(BCRYPT_LIB ${MINGW_PATH} ${gcc_name}/lib libbcrypt.a)
  findmingwlib(MINGW32_LIB ${MINGW_PATH} ${gcc_name}/lib libmingw32.a)
  findmingwlib(MINGWEX_LIB ${MINGW_PATH} ${gcc_name}/lib libmingwex.a)
  findmingwlib(PTHREAD_IMPLIB ${MINGW_PATH} ${gcc_name}/lib libpthread.dll.a)

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
  if(NOT EXISTS ${PTHREAD_IMPLIB})
    message(FATAL_ERROR "Could not find expected MinGW runtime libraries")
  endif()

  # Add mingw libs
  target_link_libraries(
    ${TARGET} INTERFACE ${GCC_LIB} ${GCC_EH_LIB} ${BCRYPT_LIB} ${MINGW32_LIB}
                        ${MINGWEX_LIB} ${PTHREAD_IMPLIB})

  # libwinpthread dll
  set(PTHREAD_DLL ${MINGW_PATH}/bin/libwinpthread-1.dll)

  if(NOT EXISTS ${PTHREAD_DLL})
    message(
      FATAL_ERROR "Could not find expected MinGW PThread runtime libraries")
  endif()

  install(
    PROGRAMS ${PTHREAD_DLL}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime)
  # copy pthread dll to output directory so bins work without setting paths
  add_custom_target(mingw-dependencies)
  add_dependencies(${TARGET} mingw-dependencies)
  add_custom_command(
    TARGET mingw-dependencies
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
  add_custom_command(
    TARGET mingw-dependencies
    COMMAND ${CMAKE_COMMAND} -E copy ${PTHREAD_DLL}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)

  # openH264 dll dependencies
  if(BUILD_OPENH264)
    set(STDCPP_DLL ${MINGW_PATH}/bin/libstdc++-6.dll)

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
      set(GCC_SEH_DLL ${MINGW_PATH}/bin/libgcc_s_seh-1.dll)
    elseif("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
      set(GCC_SEH_DLL ${MINGW_PATH}/bin/libgcc_s_dw2-1.dll)
    endif()

    if(NOT EXISTS ${STDCPP_DLL})
      message(FATAL_ERROR "Could not find expected MinGW stdc++ runtime Dll")
    endif()
    if(NOT EXISTS ${GCC_SEH_DLL})
      message(
        FATAL_ERROR "Could not find expected MinGW GCC SEH runtime libraries")
    endif()

    install(
      PROGRAMS ${STDCPP_DLL} ${GCC_SEH_DLL}
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      COMPONENT runtime)

    add_custom_command(
      TARGET mingw-dependencies
      COMMAND ${CMAKE_COMMAND} -E copy ${STDCPP_DLL} ${GCC_SEH_DLL}
              ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/)
  endif()
endif()
